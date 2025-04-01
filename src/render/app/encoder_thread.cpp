//
// Created by RGAA on 2023-12-24.
//

#include "encoder_thread.h"

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
#include "settings/rd_settings.h"
#include "render/rd_statistics.h"
#include <d3d11.h>
#include <wrl/client.h>
#include "tc_common_new/win32/d3d_render.h"
#include "tc_common_new/win32/d3d_debug_helper.h"
#include "plugins/plugin_manager.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_video_encoder_plugin.h"
#include "video_frame_carrier.h"
#include "rd_app.h"

#define DEBUG_FILE 0
#define DEBUG_SAVE_D3D11TEXTURE_TO_FILE 0

namespace tc
{
#if DEBUG_SAVE_D3D11TEXTURE_TO_FILE
    std::shared_ptr<D3DRender> g_render;
#endif
    std::shared_ptr<EncoderThread> EncoderThread::Make(const std::shared_ptr<RdApplication>& app) {
        return std::make_shared<EncoderThread>(app);
    }

    EncoderThread::EncoderThread(const std::shared_ptr<RdApplication>& app) {
        app_ = app;
        context_ = app->GetContext();
        settings_ = RdSettings::Instance();
        plugin_manager_ = context_->GetPluginManager();
        enc_thread_ = Thread::Make("encoder_thread", 5);
        enc_thread_->Poll();

        msg_listener_ = context_->CreateMessageListener();
        msg_listener_->Listen<MsgInsertKeyFrame>([=, this](const MsgInsertKeyFrame& msg) {
            // plugins: InsertIdr
            plugin_manager_->VisitEncoderPlugins([=](GrVideoEncoderPlugin* plugin) {
                plugin->InsertIdr();
            });
        });
    }

    void EncoderThread::Encode(const std::shared_ptr<Image>& image, uint64_t frame_index) {
//        if (frame_width_ != image->width || frame_height_ != image->height || !video_encoder_) {
//            auto settings = RdSettings::Instance();
//            if (video_encoder_) {
//                video_encoder_->Exit();
//                video_encoder_.reset();
//            }
//            tc::EncoderConfig encoder_config;
//            encoder_config.width = image->width;
//            encoder_config.height = image->height;
//            encoder_config.codec_type = settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kH264 ? tc::EVideoCodecType::kH264 : tc::EVideoCodecType::kHEVC;
//            encoder_config.gop_size = -1;
//            encoder_config.fps = 60;
//            encoder_config.bitrate = settings->encoder_.bitrate_ * 1000000;
//            EncoderFeature encoder_feature{-1, 0};
//            video_encoder_ = VideoEncoderFactory::CreateEncoder(context_->GetMessageNotifier(),
//                                                                encoder_feature,
//                                                                settings_->encoder_.encoder_select_type_,
//                                                                encoder_config,
//                                                                ECreateEncoderName::kFFmpeg);
//            if (!video_encoder_) {
//                LOGE("Create encoder failed, width: {}, height: {}, select type : {}, encoder name: {}",
//                     image->width, image->height, (int)settings->encoder_.encoder_select_type_, (int)settings->encoder_.encoder_name_);
//                return;
//            }
//
//            static uint64_t write_buffer = 0;
//            video_encoder_->RegisterEncodeCallback([=, this](const std::shared_ptr<Image>& frame, uint64_t frame_index, bool key) {
//                LOGI("Encoded: frame size:{}, frame index: {}, key frame: {}", frame->data->Size(), frame_index, key);
//                if (debug_file_) {
//                    debug_file_->Write(write_buffer, frame->data);
//                    write_buffer += frame->data->Size();
//                }
//
//                //
//                MsgVideoFrameEncoded msg {
//                    .frame_width_ = static_cast<uint32_t>(frame->width),
//                    .frame_height_ = static_cast<uint32_t>(frame->height),
//                    .frame_format_ = (uint32_t )settings->encoder_.encoder_format_,
//                    .frame_index_ = frame_index,
//                    .key_frame_ = key,
//                    .image_ = frame,
//                };
//                context_->SendAppMessage(msg);
//            });
//
//            frame_width_ = image->width;
//            frame_height_ = image->height;
//        }
//
//        enc_thread_->Post(SimpleThreadTask::Make([=, this]() {
//            auto beg = TimeUtil::GetCurrentTimestamp();
//            video_encoder_->Encode(image, frame_index);
//            auto end = TimeUtil::GetCurrentTimestamp();
//            auto diff = end - beg;
//            RdStatistics::Instance()->AppendEncodeDuration(diff);
//        }));
    }

    void EncoderThread::Encode(const CaptureVideoFrame& cap_video_msg) {
        PostEncTask([=, this]() {
            auto settings = RdSettings::Instance();
            auto frame_index = cap_video_msg.frame_index_;
#if DEBUG_SAVE_D3D11TEXTURE_TO_FILE
            Microsoft::WRL::ComPtr<ID3D11Texture2D> shared_texture;
        if(g_render) {
            shared_texture = g_render->OpenSharedTexture(reinterpret_cast<HANDLE>(handle));
            if(!shared_texture) {
                g_render = nullptr;
            }
        }

        if(!g_render) {
            g_render = D3DRender::Create(reinterpret_cast<HANDLE>(handle), shared_texture.ReleaseAndGetAddressOf());
        }

        if(shared_texture) {
            CopyID3D11Texture2D(shared_texture);
        }
#endif
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
            //frame_width_ != cap_video_msg.frame_width_ || frame_height_ != cap_video_msg.frame_height_

            auto target_encoder_plugin = GetEncoderForMonitor(monitor_name);
            if (frame_meta_info_changed || encoder_format_ != settings->encoder_.encoder_format_ || !target_encoder_plugin) {
                if (target_encoder_plugin) {
                    // todo : Test it!
                    target_encoder_plugin->Exit(monitor_name);
                }
                tc::EncoderConfig encoder_config;
                if (settings_->encoder_.encode_res_type_ == Encoder::EncodeResolutionType::kOrigin) {
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
                    encoder_config.frame_resize = true;
                }
                encoder_config.codec_type = settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kH264 ? tc::EVideoCodecType::kH264 : tc::EVideoCodecType::kHEVC;
                encoder_config.enable_adaptive_quantization = true;
                encoder_config.gop_size = -1;
                encoder_config.quality_preset = 1;
                encoder_config.fps = -1;
                //encoder_config.fps = 60;
                encoder_config.multi_pass = tc::ENvdiaEncMultiPass::kMultiPassDisabled;
                encoder_config.rate_control_mode = tc::ERateControlMode::kRateControlModeCbr;
                encoder_config.sample_desc_count = 1;
                encoder_config.supports_intra_refresh = true;
                encoder_config.texture_format = cap_video_msg.frame_format_;
                encoder_config.bitrate = settings->encoder_.bitrate_ * 1000000;
                encoder_config.adapter_uid_ = cap_video_msg.adapter_uid_;

                PrintEncoderConfig(encoder_config);

                // generate d3d device/context
                if (!app_->GetD3DDevice() || !app_->GetD3DContext()) {
                    if (!app_->GenerateD3DDevice(cap_video_msg.adapter_uid_)) {
                        LOGE("Generate D3DDevice failed!");
                        return;
                    }
                }
                else {
                    LOGI("We use d3d device from capture.");
                }

                encoder_config.d3d11_device_ = app_->GetD3DDevice();
                encoder_config.d3d11_device_context_ = app_->GetD3DContext();

                // all plugins
                plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
                    plugin->d3d11_device_ = app_->GetD3DDevice();
                    plugin->d3d11_device_context_ = app_->GetD3DContext();
                });

                // video frame carrier
                auto frame_carrier = GetFrameCarrier(monitor_name);
                if (frame_carrier != nullptr) {
                    frame_carrier->Exit();
                    frame_carriers_.erase(monitor_name);
                    frame_carrier = nullptr;
                }
                if (encoder_config.frame_resize) {
                    frame_carrier = std::make_shared<VideoFrameCarrier>(context_, app_->GetD3DDevice(), app_->GetD3DContext(), cap_video_msg.adapter_uid_,
                                                                         true, encoder_config.encode_width, encoder_config.encode_height);
                }
                else {
                    frame_carrier = std::make_shared<VideoFrameCarrier>(context_, app_->GetD3DDevice(), app_->GetD3DContext(),
                                                                         cap_video_msg.adapter_uid_,false, -1, -1);
                }
                frame_carriers_[monitor_name] = frame_carrier;
                LOGI("Create frame carrier for monitor: {}", monitor_name);

                // plugins: Create encoder plugin
                // To use FFmpeg encoder if mocking video stream or to implement the hardware encoder to encode raw frame(RGBA)
                bool is_mocking = settings_->capture_.mock_video_;
                auto nvenc_encoder = plugin_manager_->GetNvencEncoderPlugin();
                if (!is_mocking && nvenc_encoder && nvenc_encoder->IsPluginEnabled() && nvenc_encoder->Init(encoder_config, monitor_name)) {
                    target_encoder_plugin = nvenc_encoder;
                } else {
                    LOGW("Init NVENC failed, will try AMF.");
                    auto amf_encoder = plugin_manager_->GetAmfEncoderPlugin();
                    if (!is_mocking && amf_encoder && amf_encoder->IsPluginEnabled() && amf_encoder->Init(encoder_config, monitor_name)) {
                        target_encoder_plugin = amf_encoder;
                    } else {
                        LOGW("Init AMF failed, will try FFmpeg.");
                        auto ffmpeg_encoder = plugin_manager_->GetFFmpegEncoderPlugin();
                        if (ffmpeg_encoder && ffmpeg_encoder->IsPluginEnabled() && ffmpeg_encoder->Init(encoder_config, monitor_name)) {
                            target_encoder_plugin = ffmpeg_encoder;
                        } else {
                            LOGE("Init FFmpeg failed, we can't encode frame in this machine!");
                            return;
                        }
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

                encoder_format_ = settings->encoder_.encoder_format_;
                last_video_frames_[monitor_name] = cap_video_msg;
            }

            auto frame_carrier = GetFrameCarrier(monitor_name);
            if (frame_carrier == nullptr) {
                LOGI("Don't have frame carrier for monitor: {}", monitor_name);
                return;
            }
            // from texture
            if (cap_video_msg.handle_ > 0 && frame_carrier) {
                // copy shared texture
                auto beg = TimeUtil::GetCurrentTimestamp();
                auto target_texture = frame_carrier->CopyTexture(cap_video_msg.handle_, frame_index);
                if (target_texture == nullptr) {
                    LOGI("Don't have target texture, frame carrier copies texture failed!");
                    return;
                }

                //video_encoder_->Encode(target_texture, frame_index);
                bool can_encode_texture = false;
                if (target_encoder_plugin && target_encoder_plugin->CanEncodeTexture()) {
                    can_encode_texture = true;
                    target_encoder_plugin->Encode(target_texture, frame_index, cap_video_msg);
                }

                if (!can_encode_texture /*|| other configs*/) {
                    //Todo: TEST
                    //TimeDuration td("Measure Map Raw Texture");
                    D3D11_TEXTURE2D_DESC desc;
                    target_texture->GetDesc(&desc);
                    auto rgba_cbk = [=, this](const std::shared_ptr<Image> &image) {
                        // callback in Enc thread
                        context_->PostStreamPluginTask([=, this]() {
                            plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                                plugin->OnRawVideoFrameRgba(monitor_name, image);
                            });
                        });
                    };
                    auto yuv_cbk = [=, this](const std::shared_ptr<Image> &image) {
                        // callback in YUV converter thread
                        if (target_encoder_plugin && !can_encode_texture) {
                            PostEncTask([=, this]() {
                                target_encoder_plugin->Encode(image, frame_index, cap_video_msg);
                            });
                        }
                        context_->PostStreamPluginTask([=, this]() {
                            plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                                plugin->OnRawVideoFrameYuv(monitor_name, image);
                            });
                        });
                    };
                    frame_carrier->MapRawTexture(target_texture, desc.Format, (int) desc.Height, rgba_cbk, yuv_cbk);
                }

                auto end = TimeUtil::GetCurrentTimestamp();
                auto diff = end - beg;
                RdStatistics::Instance()->AppendEncodeDuration(diff);
            }
            else {
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

    std::shared_ptr<VideoFrameCarrier> EncoderThread::GetFrameCarrier(const std::string& monitor_name) {
        if (frame_carriers_.contains(monitor_name)) {
            return frame_carriers_[monitor_name];
        }
        return nullptr;
    }

    std::map<std::string, GrVideoEncoderPlugin*> EncoderThread::GetWorkingVideoEncoderPlugins() {
        return encoder_plugins_;
    }

    bool EncoderThread::HasEncoderForMonitor(const std::string& monitor_name) {
        return GetEncoderForMonitor(monitor_name) != nullptr;
    }

    GrVideoEncoderPlugin* EncoderThread::GetEncoderForMonitor(const std::string& monitor_name) {
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
        LOGI("***************************************************");
    }

}