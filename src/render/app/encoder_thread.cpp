//
// Created by RGAA on 2023-12-24.
//

#include "encoder_thread.h"
#include <d3d11.h>
#include <wrl/client.h>
#include "rd_app.h"
#include "rd_context.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/file.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/message_notifier.h"
#include "tc_encoder_new/video_encoder_factory.h"
#include "tc_encoder_new/video_encoder.h"
#include "tc_encoder_new/ffmpeg_video_encoder.h"
#include "tc_encoder_new/nvenc_video_encoder.h"
#include "settings/rd_settings.h"
#include "app/app_messages.h"
#include "render/rd_statistics.h"
#include "tc_common_new/win32/d3d_render.h"
#include "tc_common_new/win32/d3d_debug_helper.h"
#include "plugins/plugin_manager.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_video_encoder_plugin.h"
#include "plugin_interface/gr_frame_carrier_plugin.h"
#include "plugin_interface/gr_frame_processor_plugin.h"

#define DEBUG_FILE 0

namespace tc
{

    std::shared_ptr<EncoderThread> EncoderThread::Make(const std::shared_ptr<RdApplication>& app) {
        return std::make_shared<EncoderThread>(app);
    }

    EncoderThread::EncoderThread(const std::shared_ptr<RdApplication>& app) {
        app_ = app;
        stat_ = RdStatistics::Instance();
        context_ = app->GetContext();
        settings_ = RdSettings::Instance();
        plugin_manager_ = context_->GetPluginManager();
        enc_thread_ = Thread::Make("encoder_thread", 5);
        enc_thread_->Poll();

        // frame carrier
        frame_carrier_plugin_ = plugin_manager_->GetFrameCarrierPlugin();

        msg_listener_ = context_->CreateMessageListener();
        msg_listener_->Listen<MsgInsertKeyFrame>([=, this](const MsgInsertKeyFrame& msg) {
            // plugins: InsertIdr
            plugin_manager_->VisitEncoderPlugins([=](GrVideoEncoderPlugin* plugin) {
                plugin->InsertIdr();
            });
        });
    }

    void EncoderThread::Encode(const CaptureVideoFrame& cap_video_msg) {
        if (!frame_carrier_plugin_) {
            return;
        }
        PostEncTask([=, this]() {

            auto adapter_uid = cap_video_msg.adapter_uid_;

            // plugins: SharedTexture
            if (cap_video_msg.handle_ > 0) {

                // all plugins   // to do 这里要不要考虑adapter_uid不合法的情况 
                //plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
                //    plugin->d3d11_devices_[adapter_uid] = app_->GetD3DDevice(adapter_uid);
                //    plugin->d3d11_devices_context_[adapter_uid] = app_->GetD3DContext(adapter_uid);
                //});

                context_->PostStreamPluginTask([=, this]() {
                    plugin_manager_->VisitAllPlugins([=](GrPluginInterface* plugin) {
                        plugin->OnRawVideoFrameSharedTexture(cap_video_msg.display_name_,
                                                             cap_video_msg.frame_index_,
                                                             cap_video_msg.frame_width_,
                                                             cap_video_msg.frame_height_,
                                                             cap_video_msg.handle_,
                                                             cap_video_msg.adapter_uid_,
                                                             cap_video_msg.frame_format_
                                                            );
                    });
                });
            }

            auto settings = RdSettings::Instance();
            auto frame_index = cap_video_msg.frame_index_;
            //auto adapter_uid = cap_video_msg.adapter_uid_;
            auto monitor_name = std::string(cap_video_msg.display_name_);
            bool frame_meta_info_changed = [&]() {
                auto last_video_frame_exists = last_video_frames_.contains(monitor_name);
                if (!last_video_frame_exists) {
                    return true;
                }
                auto last_video_frame = last_video_frames_[monitor_name];
                if (last_video_frame == std::nullopt) {
                    return true;
                }
                return last_video_frame.value().frame_width_ != cap_video_msg.frame_width_
                    || last_video_frame.value().frame_height_ != cap_video_msg.frame_height_;
            }();

            bool full_color_mode_changed = false;
            auto target_encoder_plugin = GetEncoderPluginForMonitor(monitor_name);
            if (target_encoder_plugin) {
                auto encoder_config_res = target_encoder_plugin->GetEncoderConfig(monitor_name);
                if (encoder_config_res.has_value()) {
                    const auto selected_encoder_config = encoder_config_res.value();
                    if (selected_encoder_config.enable_full_color_mode_ != settings_->EnableFullColorMode() ) {
                        full_color_mode_changed = true;
                        LOGI("full_color_mode_changed!!!");
                    }
                } else {
                
                    LOGI("EncoderThread encoder_config_res no value");
                }
            }
            else {
                LOGI("EncoderThread target_encoder_plugin is nullptr");
            }

            if (full_color_mode_changed || frame_meta_info_changed || encoder_format_ != settings->encoder_.encoder_format_
                || !target_encoder_plugin || !target_encoder_plugin->IsPluginEnabled()) {
                if (target_encoder_plugin) {
                    // todo : Test it!
                    target_encoder_plugin->Exit(monitor_name);
                    target_encoder_plugin = nullptr;
                }
                tc::EncoderConfig encoder_config;
                bool is_gdi_capture = plugin_manager_->IsGDIMonitorCapturePlugin(app_->GetWorkingMonitorCapturePlugin());
                if (settings_->encoder_.encode_res_type_ == Encoder::EncodeResolutionType::kOrigin || is_gdi_capture) {
                    encoder_config.width = cap_video_msg.frame_width_;
                    encoder_config.height = cap_video_msg.frame_height_;
                    encoder_config.encode_width = cap_video_msg.frame_width_;
                    encoder_config.encode_height = cap_video_msg.frame_height_;
                    encoder_config.frame_resize = false;
                } else {
                    encoder_config.width = settings_->encoder_.encode_width_;
                    encoder_config.height = settings_->encoder_.encode_height_;
                    encoder_config.encode_width = settings_->encoder_.encode_width_;
                    encoder_config.encode_height = settings_->encoder_.encode_height_;
                    // resize will be enabled when dda capture working
                    encoder_config.frame_resize = true;
                }

                // 如果选择了全彩模式, 则强制使用HEVC, 因为 大多数264硬件解码器不支持全彩
                if (settings_->EnableFullColorMode()) {
                    settings->encoder_.encoder_format_ = Encoder::EncoderFormat::kHEVC;
                    LOGI("full color mode, use HEVC");
                }

                encoder_config.codec_type = settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kH264 ? tc::EVideoCodecType::kH264 : tc::EVideoCodecType::kHEVC;
                encoder_config.enable_adaptive_quantization = true;
                encoder_config.gop_size = -1;
                encoder_config.quality_preset = 1;
                // MUST have a value > 0
                encoder_config.fps = settings_->encoder_.fps_;
                if (encoder_config.fps < 15 || encoder_config.fps > 120) {
                    encoder_config.fps = 60;
                }
                encoder_config.multi_pass = tc::ENvdiaEncMultiPass::kMultiPassDisabled;
                encoder_config.rate_control_mode = tc::ERateControlMode::kRateControlModeCbr;
                encoder_config.sample_desc_count = 1;
                encoder_config.supports_intra_refresh = true;
                encoder_config.texture_format = cap_video_msg.frame_format_;
                encoder_config.bitrate = settings->encoder_.bitrate_ * 1000000;
                encoder_config.adapter_uid_ = cap_video_msg.adapter_uid_;
                encoder_config.enable_full_color_mode_ = settings_->EnableFullColorMode();

                PrintEncoderConfig(encoder_config);

                // generate d3d device/context
                if (!app_->GetD3DDevice(adapter_uid) || !app_->GetD3DContext(adapter_uid)) {
                    if (!app_->GenerateD3DDevice(adapter_uid)) {
                        LOGE("Generate D3DDevice failed!");
                        return;
                    }
                }
                else {
                    LOGI("We use d3d device from capture.");
                }

                // all plugins
                plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
                    plugin->d3d11_devices_[adapter_uid] = app_->GetD3DDevice(adapter_uid);
                    plugin->d3d11_devices_context_[adapter_uid] = app_->GetD3DContext(adapter_uid);
                });

                // video frame carrier
                auto r = frame_carrier_plugin_->InitFrameCarrier(GrCarrierParams {
                    .mon_name_ = monitor_name,
                    .d3d_device_ = app_->GetD3DDevice(adapter_uid),
                    .d3d_device_context_ = app_->GetD3DContext(adapter_uid),
                    .adapter_uid_ = cap_video_msg.adapter_uid_,
                    .enable_full_color_mode_ = encoder_config.enable_full_color_mode_,
                });
                if (!r) {
                    LOGE("Init Frame Carrier failed");
                }

                // plugins: Create encoder plugin
                // To use FFmpeg encoder if mocking video stream or to implement the hardware encoder to encode raw frame(RGBA)
                bool is_mocking = settings_->capture_.mock_video_;
               
                auto select_encoder_with_capability_func = [=, &target_encoder_plugin](tc::GrVideoEncoderPlugin* encoder_plugin, const std::string& monitor_name) {
                    if (!encoder_config.enable_full_color_mode_) {
                        target_encoder_plugin = encoder_plugin;
                    }
                    else {
                        auto cap_res = encoder_plugin->GetEncoderCapability(monitor_name);
                        if (cap_res.has_value()) {
                            auto cap = cap_res.value();
                            if (tc::EVideoCodecType::kH264 == encoder_config.codec_type) {
                                if (cap.support_h264_yuv444_) {
                                    target_encoder_plugin = encoder_plugin;
                                }
                            }
                            else if (tc::EVideoCodecType::kHEVC == encoder_config.codec_type) {
                                if (cap.support_hevc_yuv444_) {
                                    target_encoder_plugin = encoder_plugin;
                                }
                            }
                        }
                    }
                };
                
                // to do: GDI 采集的时候 显示器的名字是自定义的，需要测试下
                if (!target_encoder_plugin) {
                    auto nvenc_encoder_plugin = plugin_manager_->GetNvencEncoderPlugin();
                    if (!is_mocking && nvenc_encoder_plugin && nvenc_encoder_plugin->IsPluginEnabled() && nvenc_encoder_plugin->Init(encoder_config, monitor_name)) {
                        select_encoder_with_capability_func(nvenc_encoder_plugin, monitor_name);
                    }

                    if (!target_encoder_plugin) {
                        LOGW("Init NVENC failed, will try AMF.");
                        auto amf_encoder_plugin = plugin_manager_->GetAmfEncoderPlugin();
                        if (!is_mocking && amf_encoder_plugin && amf_encoder_plugin->IsPluginEnabled() && amf_encoder_plugin->Init(encoder_config, monitor_name)) {
                            select_encoder_with_capability_func(amf_encoder_plugin, monitor_name);
                        }
                    }

                    auto ffmpeg_encoder_plugin = plugin_manager_->GetFFmpegEncoderPlugin();
                    if (!target_encoder_plugin) {
                        LOGW("Init AMF failed, will try FFmpeg(kNvEnc).");
                        // 让ffmpeg尝试硬编码初始化 
                        encoder_config.Hardware = EHardwareEncoder::kNvEnc;
                        if (ffmpeg_encoder_plugin && ffmpeg_encoder_plugin->IsPluginEnabled() && ffmpeg_encoder_plugin->Init(encoder_config, monitor_name)) {
                            select_encoder_with_capability_func(ffmpeg_encoder_plugin, monitor_name);
                        }
                    }

                    if (!target_encoder_plugin) {
                        LOGW("Init FFmpeg(kNvEnc) failed, will try FFmpeg(kAmf).");
                        // 让ffmpeg尝试硬编码初始化 
                        encoder_config.Hardware = EHardwareEncoder::kAmf;
                        if (ffmpeg_encoder_plugin && ffmpeg_encoder_plugin->IsPluginEnabled() && ffmpeg_encoder_plugin->Init(encoder_config, monitor_name)) {
                            select_encoder_with_capability_func(ffmpeg_encoder_plugin, monitor_name);
                        }
                    }

                    if (!target_encoder_plugin) {
                        LOGW("Init FFmpeg(kAmf) failed, will try FFmpeg(kNone).");
                        //让ffmpeg尝试软件编码初始化
                        encoder_config.Hardware = EHardwareEncoder::kNone;
                        if (ffmpeg_encoder_plugin && ffmpeg_encoder_plugin->IsPluginEnabled() && ffmpeg_encoder_plugin->Init(encoder_config, monitor_name)) {
                            select_encoder_with_capability_func(ffmpeg_encoder_plugin, monitor_name);
                        }
                    }

                    if (!target_encoder_plugin) {
                        LOGW("Init FFmpeg(kAmf) failed, will try FFmpeg(kNone). without capability!");
                        //让ffmpeg尝试软件编码初始化
                        encoder_config.Hardware = EHardwareEncoder::kNone;
                        if (ffmpeg_encoder_plugin && ffmpeg_encoder_plugin->IsPluginEnabled() && ffmpeg_encoder_plugin->Init(encoder_config, monitor_name)) {
                            target_encoder_plugin = ffmpeg_encoder_plugin;
                        }
                    }

                    if (!target_encoder_plugin) {
                        LOGE("Init FFmpeg failed, we can't encode frame in this machine!");
                        return;
                    }
                }

                encoder_plugins_[monitor_name] = target_encoder_plugin;
                LOGI("Finally, we use encoder plugin: {}, version: {} for monitor: {}",
                     target_encoder_plugin->GetPluginName(), target_encoder_plugin->GetVersionName(), monitor_name);

                auto video_type = [=]() -> GrPluginEncodedVideoType {
                    if (settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kH264) {
                        return GrPluginEncodedVideoType::kH264;
                    } else if (settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kHEVC) {
                        return GrPluginEncodedVideoType::kH265;
                    } else {
                        return GrPluginEncodedVideoType::kH264;
                    }
                } ();

                // plugins: VideoEncoderCreated
                context_->PostStreamPluginTask([=, this]() {
                    plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                        plugin->OnVideoEncoderCreated(video_type, encoder_config.width, encoder_config.height);
                    });
                });

                stat_->video_encoder_format_ = settings->encoder_.encoder_format_;

                encoder_format_ = settings->encoder_.encoder_format_;
                last_video_frames_[monitor_name] = cap_video_msg;
            }

            // from texture handle
            if (cap_video_msg.handle_ > 0 && frame_carrier_plugin_) {
                // 1. copy shared texture
                auto beg = TimeUtil::GetCurrentTimestamp();
                auto cp_result = frame_carrier_plugin_->CopyTexture(monitor_name, cap_video_msg.handle_, frame_index);
                if (cp_result->texture_ == nullptr) {
                    LOGI("Don't have target texture, frame carrier copies texture failed!");
                    return;
                }

                ComPtr<ID3D11Texture2D> target_texture = cp_result->texture_;
                // 2. resize ?
                if (auto opt_config = target_encoder_plugin->GetEncoderConfig(monitor_name);
                    opt_config.has_value() && opt_config.value().frame_resize) {
                    auto config = opt_config.value();
                    if (auto resize_plugin = plugin_manager_->GetFrameResizePlugin(); resize_plugin) {
                        auto t = resize_plugin->Process(cp_result->texture_, adapter_uid, monitor_name, config.encode_width, config.encode_height);
                        if (t) {
                            target_texture = t;
                        }
                        else {
                            LOGE("Resize failed!");
                            return;
                        }
                    }
                }

                auto copy_texture_end = TimeUtil::GetCurrentTimestamp();
                auto diff_copy_texture = copy_texture_end - beg;
                stat_->CaptureInfo(monitor_name)->AppendCopyTextureDuration((int32_t)diff_copy_texture);

                //video_encoder_->Encode(target_texture, frame_index);
                bool can_encode_texture = false;
                if (target_encoder_plugin && target_encoder_plugin->CanEncodeTexture()) {
                    can_encode_texture = true;
                    // plugins: EncodeTexture
                    target_encoder_plugin->Encode(target_texture, frame_index, cap_video_msg);
                }

                // TODO: Add Texture Mapping duration
                if (!can_encode_texture /*|| other configs*/) {
                    //Todo: TEST
                    //TimeDuration td("Measure Map Raw Texture");

                    auto beg_map_texture = TimeUtil::GetCurrentTimestamp();

                    D3D11_TEXTURE2D_DESC desc;
                    target_texture->GetDesc(&desc);
                    auto rgba_cbk = [=, this](const std::shared_ptr<Image> &image) {
                        // callback in Enc thread
                        context_->PostStreamPluginTask([=, this]() {
                            plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
                                plugin->OnRawVideoFrameRgba(monitor_name, cap_video_msg.frame_index_, cap_video_msg.frame_width_, cap_video_msg.frame_height_, image);
                            });
                        });
                    };
                    auto yuv_cbk = [=, this](const std::shared_ptr<Image> &image) {
                        // notify yuv
                        context_->PostStreamPluginTask([=, this]() {
                            plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface *plugin) {
                                plugin->OnRawVideoFrameYuv(monitor_name, cap_video_msg.frame_index_, cap_video_msg.frame_width_, cap_video_msg.frame_height_, image);
                            });
                        });

                        // calculate used time
                        auto end_map_cvt_texture = TimeUtil::GetCurrentTimestamp();
                        auto diff_map_cvt_texture = end_map_cvt_texture - beg_map_texture;
                        stat_->CaptureInfo(monitor_name)->AppendMapCvtTextureDuration((int32_t)diff_map_cvt_texture);

                        // callback in YUV converter thread
                        if (target_encoder_plugin && !can_encode_texture) {
                            PostEncTask([=, this]() {
                                target_encoder_plugin->Encode(image, frame_index, cap_video_msg);
                            });
                        }
                    };
                    // map the texture from GPU -> CPU
                    frame_carrier_plugin_->MapRawTexture(monitor_name, target_texture, desc.Format, (int)desc.Height, rgba_cbk, yuv_cbk);
                }

                auto end = TimeUtil::GetCurrentTimestamp();
                auto diff = end - beg;
                //RdStatistics::Instance()->AppendEncodeDuration(diff);
            }
            else {
#if 0
                context_->PostStreamPluginTask([=, this]() {
                    plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                        plugin->OnRawVideoFrameRgba(monitor_name, cap_video_msg.raw_image_);
                    });
                });

                // todo: convert to YUV

                // raw video frame
                if (target_encoder_plugin) {
                    target_encoder_plugin->Encode(cap_video_msg.raw_image_, frame_index, cap_video_msg);
                }
#endif

                auto beg_map_texture = TimeUtil::GetCurrentTimestamp();

                auto rgba_cbk = [=, this](const std::shared_ptr<Image>& image) {
                    // callback in Enc thread
                    context_->PostStreamPluginTask([=, this]() {
                        plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin* plugin) {
                            plugin->OnRawVideoFrameRgba(monitor_name, cap_video_msg.frame_index_, cap_video_msg.frame_width_, cap_video_msg.frame_height_, image);
                        });
                    });
                };

                auto yuv_cbk = [=, this](const std::shared_ptr<Image>& image) {
                    // calculate used time
                    auto end_map_cvt_texture = TimeUtil::GetCurrentTimestamp();
                    auto diff_map_cvt_texture = end_map_cvt_texture - beg_map_texture;
                    stat_->CaptureInfo(monitor_name)->AppendMapCvtTextureDuration((int32_t)diff_map_cvt_texture);

                    // callback in YUV converter thread
                    if (target_encoder_plugin) {
                        PostEncTask([=, this]() {
                            target_encoder_plugin->Encode(image, frame_index, cap_video_msg);
                        });
                    }
                    context_->PostStreamPluginTask([=, this]() {
                        plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin* plugin) {
                            plugin->OnRawVideoFrameYuv(monitor_name, cap_video_msg.frame_index_, cap_video_msg.frame_width_, cap_video_msg.frame_height_, image);
                        });
                    });
                };
                frame_carrier_plugin_->ConvertRawImage(monitor_name, cap_video_msg.raw_image_, rgba_cbk, yuv_cbk);
            }
        });
    }

    void EncoderThread::Exit() {
        if (enc_thread_) {
            enc_thread_->Exit();
        }
    }

    void EncoderThread::PostEncTask(std::function<void()>&& task) {
        enc_thread_->Post(std::move(task));
    }

    std::map<std::string, GrVideoEncoderPlugin*> EncoderThread::GetWorkingVideoEncoderPlugins() {
        return encoder_plugins_;
    }

    bool EncoderThread::HasEncoderForMonitor(const std::string& monitor_name) {
        return GetEncoderPluginForMonitor(monitor_name) != nullptr;
    }

    GrVideoEncoderPlugin* EncoderThread::GetEncoderPluginForMonitor(const std::string& monitor_name) {
        for (const auto& [name, plugin] : encoder_plugins_) {
            if (name == monitor_name) {
                return plugin;
            }
        }
        return nullptr;
    }

    void EncoderThread::PrintEncoderConfig(const tc::EncoderConfig& config) {
        LOGI("---------------------------------------------------");
        LOGI("Encoder configs:");
        LOGI("width x height:{}x{}", config.width, config.height);
        LOGI("gop size: {}", config.gop_size);
        LOGI("gop bitrate: {}", config.bitrate);
        LOGI("enable full color: {}", config.enable_full_color_mode_);
        LOGI("encoder codec_type: {}", static_cast<int>(config.codec_type));
        LOGI("***************************************************");
    }

}