//
// Created by RGAA on 2023-12-16.
//

#include "rd_app.h"
#include <windows.h>
#include "rd_context.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_util.h"
#include "tc_common_new/time_util.h"
#include "tc_encoder_new/video_encoder_factory.h"
#include "tc_capture_new/capture_message.h"
#include "tc_capture_new/capture_message_maker.h"
//#include "hook_capture/audio_capture_factory.h"
//#include "hook_capture/desktop_capture.h"
//#include "hook_capture/desktop_capture_factory.h"
#include "app/app_manager.h"
#include "app/app_manager_factory.h"
#include "app/app_messages.h"
#include "settings/rd_settings.h"
#include "render_panel/network/ws_panel_server.h"
#include "app/encoder_thread.h"
#include "network/net_message_maker.h"
#include "tc_message.pb.h"
#include "tc_render_panel_message.pb.h"
#include "app/app_timer.h"
#include "tc_opus_codec_new/opus_codec.h"
#include "network/ws_panel_client.h"
#include "network/server_cast.h"
#include "app/app_shared_info.h"
#include "app/win/dx_address_loader.h"
#include "tc_common_new/win32/win_helper.h"
#include "tc_common_new/fft_32.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/shared_preference.h"
#include "tc_controller/vigem/vigem_controller.h"
#include "tc_controller/vigem_driver_manager.h"
#include "rd_statistics.h"
#include "network/render_service_client.h"
#include "plugins/plugin_manager.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_net_plugin.h"
#include "plugin_interface/gr_monitor_capture_plugin.h"
#include "plugin_interface/gr_data_provider_plugin.h"
#include "plugin_interface/gr_audio_encoder_plugin.h"
#include "tc_service_message.pb.h"
#include "app/monitor_refresher.h"
#include "app/win/win_desktop_manager.h"
#include "tc_common_new/win32/d3d11_wrapper.h"
#include "tc_message_new/proto_converter.h"
#include "tc_message_new/rp_proto_converter.h"

namespace tc
{

    std::shared_ptr<RdApplication> rdApp;

    std::shared_ptr<RdApplication> RdApplication::Make(const AppParams& args) {
        // By OS
        // Windows
        return std::make_shared<WinApplication>(args);
        // Linux
    }

    RdApplication::RdApplication(const AppParams& args) {
        auto settings = RdSettings::Instance();
        settings_ = settings;

        // debug
        // MessageBoxA(0, "", "debug", 0);
    }

    RdApplication::~RdApplication() {
        LOGI("RdApplication dtor");
    }

    void RdApplication::Init(int argc, char** argv) {
        qapp_ = std::make_shared<QApplication>(argc, argv);

        // sp
        sp_ = SharedPreference::Instance();
        auto path = qapp_->applicationDirPath() + "/gr_data";
        std::string sp_name = std::format("gammaray_render_{}.dat", settings_->transmission_.listening_port_);
        sp_->Init(path.toStdString(), sp_name);
    }

    int RdApplication::Run() {
        statistics_ = RdStatistics::Instance();

        // context
        context_ = std::make_shared<RdContext>();
        context_->Init();

        plugin_manager_ = PluginManager::Make(shared_from_this());
        context_->SetPluginManager(plugin_manager_);

        plugin_manager_->LoadAllPlugins();
        plugin_manager_->RegisterPluginEventsCallback();
        plugin_manager_->DumpPluginInfo();

        statistics_->SetApplication(shared_from_this());
        statistics_->StartMonitor();

        ws_panel_client_ = std::make_shared<WsPanelClient>(context_);
        ws_panel_client_->Start();

        // app manager
        app_manager_ = AppManagerFactory::Make(context_);
        // encoder in thread
        encoder_thread_ = EncoderThread::Make(shared_from_this());
        // event bus listener
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        // app shared info
        app_shared_info_ = AppSharedInfo::Make(context_);

        // app timer
        InitAppTimer();
        // messages
        InitMessages();
        // global audio capture
        if (settings_->capture_.enable_audio_) {
            InitAudioCapture();
        }

        // vigem control thread
        //control_thread_ = Thread::Make("control", 16);
        //control_thread_->Poll();
        // desktop capture
        if (settings_->capture_.mock_video_) {
            LOGI("Use mocking video plugin.");
            data_provider_plugin = plugin_manager_->GetMockVideoStreamPlugin();
        }
        else {
            if (settings_->capture_.IsVideoInnerCapture()) {
                LOGI("Use inner capture.");
            }
            else {
                monitor_capture_plugin_ = plugin_manager_->GetDDACapturePlugin();
                LOGI("Use capture fps: {}", settings_->encoder_.fps_);
                if (monitor_capture_plugin_ && monitor_capture_plugin_->IsPluginEnabled()) {
                    LOGI("Use dda capture plugin.");
                    monitor_capture_plugin_->SetCaptureFps(settings_->encoder_.fps_);
                    monitor_capture_plugin_->SetCaptureInitFailedCallback([=, this]() { // 当DDA初始化有异常发生时候, 切换为GDI
                        monitor_capture_plugin_->StopCapturing();
                        monitor_capture_plugin_->DisablePlugin();
                        LOGI("Don't use DDA, will switch to GDI.");
                        SwitchGdiCapture();
                        monitor_capture_plugin_->StartCapturing();
                    });
                }
                else {
                    LOGI("Don't use DDA, will switch to GDI.");
                    SwitchGdiCapture();
                }
            }
        }

        if (settings_->capture_.enable_video_) {
            if (settings_->capture_.capture_video_type_ == Capture::CaptureVideoType::kVideoInner) {
                StartProcessWithHook();
            }
            else if (settings_->capture_.capture_video_type_ == Capture::CaptureVideoType::kCaptureScreen) {
                StartProcessWithScreenCapture();
            }
        }

        // connect to service
        service_client_ = std::make_shared<RenderServiceClient>(shared_from_this());
        service_client_->Start();

        // monitor refresher
        monitor_refresher_ = std::make_shared<MonitorRefresher>(context_, nullptr);

        // desktop manager
        desktop_mgr_ = WinDesktopManager::Make(context_);

        rdApp = shared_from_this();

        return qapp_->exec();
    }

    void RdApplication::InitAppTimer() {
        app_timer_ = std::make_shared<AppTimer>(context_);
        app_timer_->StartTimers();
    }

    void RdApplication::InitMessages() {
        msg_listener_->Listen<MsgBeforeInject>([=, this](const MsgBeforeInject& msg) {
            if (settings_->capture_.IsVideoInnerCapture()) {
                this->WriteBoostUpInfoForPid(msg.pid_);
            }
        });

        msg_listener_->Listen<MsgObsInjected>([=, this](const MsgObsInjected& msg) {

        });

        msg_listener_->Listen<MsgTimer16>([=, this](const MsgTimer16& msg) {
            this->PostGlobalTask([=, this]() {
                this->SendAudioSpectrumMessage();
            });
        });

        msg_listener_->Listen<MsgTimer100>([=, this](const MsgTimer100& msg) {
            this->PostGlobalTask([=, this]() {
                // If you want a much smoother spectrum, report it quicker, post it in MsgTimer16 callback
                this->ReportAudioSpectrum2Panel();
            });
        });

        msg_listener_->Listen<MsgTimer1000>([=, this](const MsgTimer1000& msg) {
            statistics_->IncreaseRunningTime();

            auto plugin_manager = context_->GetPluginManager();
            plugin_manager->On1Second();
        });

        msg_listener_->Listen<MsgClientConnected>([=, this](const MsgClientConnected& msg) {
            this->PostGlobalTask([=, this]() {

            });
        });

        msg_listener_->Listen<MsgClientHello>([=, this](const MsgClientHello& msg) {
            this->PostGlobalTask([=, this]() {
                // send configuration back to client
                this->SendConfigurationBack();
            });
        });

        msg_listener_->Listen<MsgClientDisconnected>([=, this](const MsgClientDisconnected& msg) {
            this->PostGlobalTask([=, this]() {

            });
        });

        msg_listener_->Listen<ClipboardMessage>([=, this](const ClipboardMessage& msg) {
            this->PostGlobalTask([=, this]() {
                SendClipboardMessage(msg.msg_);
            });
        });

        // DDA Init failed
        msg_listener_->Listen<CaptureInitFailedMessage>([=, this](const CaptureInitFailedMessage& msg) {
            this->PostGlobalTask([=, this]() {
                statistics_->IncreaseDDAFailedCount();
                // tell UI process to restart me
                RequestRestartMe();
            });
        });

        // CaptureMonitorInfoMessage
        msg_listener_->Listen<CaptureMonitorInfoMessage>([=, this](const CaptureMonitorInfoMessage& msg) {
            this->PostGlobalTask([=, this]() {
                SendConfigurationBack();
            });
        });

        msg_listener_->Listen<MsgReCreateRefresher>([=, this](const MsgReCreateRefresher& msg) {
            context_->PostUITask([=, this]() {
                monitor_refresher_ = std::make_shared<MonitorRefresher>(context_, nullptr);
            });
        });

        msg_listener_->Listen<MsgModifyFps>([=, this](const MsgModifyFps& msg) {
            if (monitor_capture_plugin_) {
                settings_->encoder_.fps_ = msg.fps_;
                monitor_capture_plugin_->SetCaptureFps(msg.fps_);
            }
        });

        // request from Remote Panel's context menu or same function
        msg_listener_->Listen<MsgPanelStreamLockScreen>([=, this](const MsgPanelStreamLockScreen& msg) {
            LOGI(" ** Panel request LockScreen from device: {}", msg.from_device_);
            Hardware::LockScreen();
        });

        // request from Remote Panel's context menu or same function
        msg_listener_->Listen<MsgPanelStreamRestartDevice>([=, this](const MsgPanelStreamRestartDevice& msg) {
            LOGI(" ** Panel request RestartDevice from device: {}", msg.from_device_);
            Hardware::RestartDevice();
        });

        // request from Remote Panel's context menu or same function
        msg_listener_->Listen<MsgPanelStreamShutdownDevice>([=, this](const MsgPanelStreamShutdownDevice& msg) {
            LOGI(" ** Panel request ShutdownDevice from device: {}", msg.from_device_);
            Hardware::ShutdownDevice();
        });

    }

    void RdApplication::InitAudioCapture() {
        if (settings_->capture_.capture_audio_type_ != Capture::CaptureAudioType::kAudioGlobal) {
            return;
        }

        audio_capture_plugin_ = plugin_manager_->GetAudioCapturePlugin();
        audio_encoder_plugin_ = plugin_manager_->GetAudioEncoderPlugin();
        if (!audio_capture_plugin_ || !audio_encoder_plugin_) {
            return;
        }

        msg_listener_->Listen<CaptureAudioFrame>([=, this] (const CaptureAudioFrame& frame) {
            if (!HasConnectedPeer()) {
                return;
            }

            int samples = (int)frame.samples_;
            int channels = (int)frame.channels_;
            int bits = (int)frame.bits_;

            if (frame.full_data_) {
                audio_encoder_plugin_->Encode(frame.full_data_, samples, channels, bits);

                auto stat = RdStatistics::Instance();
                stat->audio_samples_ = samples;
                stat->audio_channels_ = channels;
                stat->audio_bits_ = bits;

                // plugins
                {
                    auto data = frame.full_data_;
                    context_->PostStreamPluginTask([=, this]() {
                        plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                            plugin->OnRawAudioData(data, samples, channels, bits);
                        });
                    });
                }
                // statistics
                {
                    auto current_time = TimeUtil::GetCurrentTimestamp();
                    if (last_post_audio_time_ == 0) {
                        last_post_audio_time_ = current_time;
                    }
                    auto diff = current_time - last_post_audio_time_;
                    last_post_audio_time_ = current_time;
                    statistics_->AppendAudioFrameGap(diff);
                }
            }
            else if (frame.left_ch_data_ && frame.right_ch_data_) {
                PostGlobalTask([=, this]() {
                    auto bytes = 960;
                    auto single_bytes = bytes/2;
                    if (fft_left_.size() != single_bytes) {
                        fft_left_.resize(single_bytes);
                    }
                    if (fft_right_.size() != single_bytes) {
                        fft_right_.resize(single_bytes);
                    }
                    FFT32::DoFFT(fft_left_, frame.left_ch_data_, 960, true);
                    FFT32::DoFFT(fft_right_, frame.right_ch_data_, 960, true);
                    int cpy_size = 150;
                    if (fft_left_.size() < cpy_size || fft_right_.size() < cpy_size) {
                        return;
                    }

                    statistics_->CopyLeftSpectrum(fft_left_, cpy_size);
                    statistics_->CopyRightSpectrum(fft_right_, cpy_size);
                });

                context_->PostStreamPluginTask([=, this]() {
                    plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                        plugin->OnSplitRawAudioData(frame.left_ch_data_, frame.right_ch_data_, samples, channels, bits);
                        plugin->OnSplitFFTAudioData(fft_left_, fft_right_);
                    });
                });
            }
        });

        audio_capture_thread_ = std::make_shared<Thread>([=, this]() {
            audio_capture_plugin_->StartProviding();
        }, "global audio capture", false);
    }

    void RdApplication::PostGlobalAppMessage(std::shared_ptr<AppMessage>&& msg) {
        QMetaObject::invokeMethod(this, [m = std::move(msg)]() {
            if (m->task_) {
                m->task_();
            }
        });
    }

    void RdApplication::PostGlobalTask(std::function<void()>&& task) {
        PostGlobalAppMessage(AppMessageMaker::MakeTaskMessage(std::move(task)));
    }

    void RdApplication::PostIpcMessage(std::shared_ptr<Data>&& msg) {

    }

    void RdApplication::PostIpcMessage(const std::string& msg) {
        if (settings_->capture_.IsVideoInnerCapture()) {
            PostNetMessage(Data::From(msg));
        }
    }

    void RdApplication::PostNetMessage(std::shared_ptr<Data> msg) {
        if (!msg) {
            return;
        }
        plugin_manager_->VisitNetPlugins([=](GrNetPlugin* plugin) {
            plugin->PostProtoMessage(msg, true);
        });
    }

    void RdApplication::StartProcessWithHook() {
#if 0   // to do, 目前用不到暂时注释掉
        msg_listener_->Listen<MsgVideoFrameEncoded>([=, this](const MsgVideoFrameEncoded& msg) {
            auto net_msg = NetMessageMaker::MakeVideoFrameMsg([=]() -> tc::VideoType {
                return (Encoder::EncoderFormat)msg.frame_encode_type_ == Encoder::EncoderFormat::kH264 ? tc::VideoType::kNetH264 : tc::VideoType::kNetHevc;
            } (), msg.data_, msg.frame_index_, msg.frame_width_, msg.frame_height_, msg.key_frame_, msg.monitor_name_,
            msg.monitor_left_, msg.monitor_top_, msg.monitor_right_, msg.monitor_bottom_);

            if (settings_->app_.debug_enabled_) {
                if (!debug_encode_file_) {
                    debug_encode_file_ = File::OpenForWriteB("1.debug_after_encode.h264");
                }
                debug_encode_file_->Append(msg.data_->AsString());
                LOGI("encoded frame callback, size: {}x{}, buffer size: {}", msg.frame_width_, msg.frame_height_, msg.data_->Size());
            }
            PostNetMessage(net_msg);
        });

        auto fn_start_process = [=, this]() {
            bool ok = app_manager_->StartProcessWithHook();
            if (!ok) {
                LOGE("StartProcessWithHook failed.");
            }
        };

        bool is_steam_app = settings_->app_.IsSteamUrl();
        // steam app
        if (is_steam_app) {
            fn_start_process();
            return;
        }
        fn_start_process();
#endif
    }

    void RdApplication::StartProcessWithScreenCapture() {
        msg_listener_->Listen<CaptureVideoFrame>([=, this](const CaptureVideoFrame& msg) {
            // todo: RtcLocal process
            //

            if (!HasConnectedPeer()) {
                return;
            }
            bool only_audio_clients = true;
            plugin_manager_->VisitNetPlugins([&](GrNetPlugin* plugin) {
                if (plugin->IsWorking() && !plugin->IsOnlyAudioClients()) {
                    only_audio_clients = false;
                }
            });
            if (only_audio_clients) {
                LOGI("Only audio clients, ignore video frame.");
                return;
            }

            // calculate gaps between 2 captured frames.
            //{
            //    auto current_time = TimeUtil::GetCurrentTimestamp();
            //    if (last_capture_screen_time_ == 0) {
            //        last_capture_screen_time_ = current_time;
            //    }
            //    auto gap = current_time - last_capture_screen_time_;
            //    last_capture_screen_time_ = current_time;
            //    statistics_->AppendFrameGap(gap);
            //}

            // to encode
            encoder_thread_->Encode(msg);
        });

        msg_listener_->Listen<CaptureCursorBitmap>([=, this](const CaptureCursorBitmap& cursor_msg) {
            auto net_msg = NetMessageMaker::MakeCursorInfoSyncMsg(cursor_msg.x_, cursor_msg.y_, cursor_msg.hotspot_x_,
                                                                  cursor_msg.hotspot_y_, cursor_msg.width_, cursor_msg.height_,
                                                                  cursor_msg.visible_, cursor_msg.data_, cursor_msg.type_);
            PostNetMessage(net_msg);
        });

        if (monitor_capture_plugin_) {
            monitor_capture_plugin_->StartCapturing();
            //monitor_capture_plugin_->SetCaptureMonitor("");
        }
        if (data_provider_plugin) {
            data_provider_plugin->StartProviding();
        }
        app_manager_->StartProcess();
    }

    void RdApplication::OnIpcVideoFrame(const std::shared_ptr<CaptureVideoFrame>& msg) {
        if (!HasConnectedPeer()) {
            return;
        }
        encoder_thread_->Encode(*msg);
    }

    bool RdApplication::HasConnectedPeer() {
        return plugin_manager_->GetTotalConnectedClientsCount();
    }

    void RdApplication::WriteBoostUpInfoForPid(uint32_t pid) {
        if (!app_shared_message_) {
            LOGE("Don't have app_shared_message_");
            return;
        }
        auto shm_name = std::format("application_shm_{}", pid);
        std::string shm_buffer;
        shm_buffer.resize(sizeof(AppSharedMessage));
        memcpy(shm_buffer.data(), app_shared_message_.get(), sizeof(AppSharedMessage));
        app_shared_info_->WriteData(shm_name, shm_buffer);
    }

    void RdApplication::SendAudioSpectrumMessage() {
        auto st = RdStatistics::Instance();
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kRendererAudioSpectrum);
        auto sas = msg->mutable_renderer_audio_spectrum();
        sas->set_samples(st->audio_samples_);
        sas->set_bits(st->audio_bits_);
        sas->set_channels(st->audio_channels_);
        auto left_spectrum = st->GetLeftSpectrum();
        auto right_spectrum = st->GetRightSpectrum();
        sas->mutable_left_spectrum()->Add(left_spectrum.begin(), left_spectrum.end());
        sas->mutable_right_spectrum()->Add(right_spectrum.begin(), right_spectrum.end());
        auto net_msg = ProtoAsData(msg);
//        if (ws_panel_client_) {
//            ws_panel_client_->PostNetMessage(net_msg);
//        }

        // audio spectrum
        PostNetMessage(net_msg);
    }

    void RdApplication::ReportAudioSpectrum2Panel() {
        auto st = RdStatistics::Instance();
        auto msg = std::make_shared<tcrp::RpMessage>();
        msg->set_type(tcrp::kRpServerAudioSpectrum);
        auto sas = msg->mutable_renderer_audio_spectrum();
        sas->set_samples(st->audio_samples_);
        sas->set_bits(st->audio_bits_);
        sas->set_channels(st->audio_channels_);
        auto left_spectrum = st->GetLeftSpectrum();
        auto right_spectrum = st->GetRightSpectrum();
        sas->mutable_left_spectrum()->Add(left_spectrum.begin(), left_spectrum.end());
        sas->mutable_right_spectrum()->Add(right_spectrum.begin(), right_spectrum.end());
        auto buffer = RpProtoAsData(msg);
        PostPanelMessage(buffer);
    }

    void RdApplication::SendClipboardMessage(const std::string& msg) {
        tc::Message m;
        m.set_type(tc::kClipboardInfo);
        m.mutable_clipboard_info()->set_msg(msg);
        auto buffer = ProtoAsData(&m);
        PostNetMessage(buffer);
    }

    void RdApplication::SendConfigurationBack() {
        if (!monitor_capture_plugin_) {
            LOGE("SendConfigurationBack failed, working monitor capture plugin is null.");
            return;
        }
        tc::Message m;
        m.set_type(tc::kServerConfiguration);
        auto config = m.mutable_config();
        // screen info
        auto monitors_info = config->mutable_monitors_info();
        auto capturing_name = monitor_capture_plugin_->GetCapturingMonitorName();
        auto monitors = monitor_capture_plugin_->GetCaptureMonitorInfo();
        LOGI("Will send configuration back, monitor size: {}", monitors.size());
        for (int i = 0; i < monitors.size(); i++) {
            auto monitor = monitors[i];
            MonitorInfo info;
            info.set_name(monitor.name_);
            for (const auto& res : monitor.supported_res_) {
                MonitorResolution mr;
                mr.set_width(res.width_);
                mr.set_height(res.height_);
                info.mutable_resolutions()->Add(std::move(mr));
            }
            info.set_current_width(monitor.Width());
            info.set_current_height(monitor.Height());
            monitors_info->Add(std::move(info));
        }
        LOGI("Will send configuration back, fps: {}", settings_->encoder_.fps_);
        config->set_fps(settings_->encoder_.fps_);
        config->set_capturing_monitor_name(capturing_name);
        config->set_file_transfer_enabled(settings_->file_transfer_enabled_);
        config->set_audio_enabled(settings_->audio_enabled_);
        config->set_can_be_operated(settings_->can_be_operated_);
        //
        auto buffer = ProtoAsData(&m);
        PostNetMessage(buffer);
    }

    void RdApplication::RequestRestartMe() {
        tcrp::RpMessage m;
        m.set_type(tcrp::kRpRestartServer);
        m.mutable_restart_server()->set_reason("restart");
        auto buffer = RpProtoAsData(&m);
        ws_panel_client_->PostNetMessage(buffer);
    }

    void RdApplication::ResetMonitorResolution(const std::string& name, int w, int h) {
        DEVMODE dm;
        dm.dmSize = sizeof(dm);
        dm.dmPelsWidth = w;
        dm.dmPelsHeight = h;
        dm.dmBitsPerPel = 32;
        dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        auto deviceName = StringUtil::ToWString(name);//L"\\\\.\\DISPLAY1";
        LONG result = ChangeDisplaySettingsExW(deviceName.c_str(), &dm, nullptr, CDS_FULLSCREEN, nullptr);
        bool ok = result == DISP_CHANGE_SUCCESSFUL;

        tc::Message m;
        m.set_type(tc::kChangeMonitorResolutionResult);
        auto r = m.mutable_change_monitor_resolution_result();
        r->set_monitor_name(name);
        r->set_result(ok);
        auto buffer = ProtoAsData(&m);
        PostNetMessage(buffer);
    }

    std::shared_ptr<PluginManager> RdApplication::GetPluginManager() {
        return plugin_manager_;
    }

    tc::GrMonitorCapturePlugin* RdApplication::GetWorkingMonitorCapturePlugin() {
        return monitor_capture_plugin_;
    }

    std::map<std::string, GrVideoEncoderPlugin*> RdApplication::GetWorkingVideoEncoderPlugins() {
        if (encoder_thread_) {
            return encoder_thread_->GetWorkingVideoEncoderPlugins();
        }
        return {};
    }

    bool RdApplication::GenerateD3DDevice(uint64_t adapter_uid) {
        LOGI("GenerateD3DDevice, adapter_uid = {}", adapter_uid);
        if (d3d11_devices_.contains(adapter_uid)) {
            d3d11_devices_[adapter_uid]->Release();
            d3d11_devices_.erase(adapter_uid);
        }

        auto new_device_wrapper = std::make_shared<D3D11DeviceWrapper>();

        ComPtr<IDXGIFactory1> factory1;
        ComPtr<IDXGIAdapter1> adapter;
        DXGI_ADAPTER_DESC desc;
        HRESULT res = NULL;
        int adapter_index = 0;
        res = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(factory1.GetAddressOf()));
        if (res != S_OK) {
            LOGE("CreateDXGIFactory1 failed");
            return false;
        }
        while (true) {
            res = factory1->EnumAdapters1(adapter_index, adapter.GetAddressOf());
            if (res != S_OK) {
                LOGE("EnumAdapters1 index:{} failed\n", adapter_index);
                return false;
            }
            D3D_FEATURE_LEVEL featureLevel;

            adapter->GetDesc(&desc);
            if (adapter_uid == desc.AdapterLuid.LowPart || adapter_uid == -1 /* todo: try to find hardware.*/) {
                LOGI("Adapter Index:{} Name: {}", adapter_index, StringUtil::ToUTF8(desc.Description).c_str());
                LOGI("find adapter");
                break;
            }
            ++adapter_index;
        }

        D3D_FEATURE_LEVEL featureLevel;
        res = D3D11CreateDevice(adapter.Get(),
                                D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                                D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                nullptr, 0, D3D11_SDK_VERSION,
                                &new_device_wrapper->d3d11_device_, &featureLevel, &new_device_wrapper->d3d11_device_context_);

        if (res != S_OK || !new_device_wrapper->d3d11_device_) {
            LOGE("D3D11CreateDevice failed: {}", res);
            return false;
        } else {
            LOGI("D3D11CreateDevice mDevice = {}", (void *) new_device_wrapper->d3d11_device_.Get());
            d3d11_devices_[adapter_uid] = new_device_wrapper;
            return true;
        }
    }

    ComPtr<ID3D11Device> RdApplication::GetD3DDevice(uint64_t adapter_uid) {
        if (!d3d11_devices_.contains(adapter_uid)) {
            return nullptr;
        }
        return d3d11_devices_[adapter_uid]->d3d11_device_;
    }

    ComPtr<ID3D11DeviceContext> RdApplication::GetD3DContext(uint64_t adapter_uid) {
        if (!d3d11_devices_.contains(adapter_uid)) {
            return nullptr;
        }
        return d3d11_devices_[adapter_uid]->d3d11_device_context_;
    }

    void RdApplication::ReqCtrlAltDelete(const std::string& device_id, const std::string& stream_id) {
        if (!service_client_ || !service_client_->IsAlive()) {
            return;
        }
        tc::ServiceMessage m;
        m.set_type(ServiceMessageType::kSrvReqCtrlAltDelete);
        m.mutable_req_ctrl_alt_delete()->set_req_device_id(device_id);
        m.mutable_req_ctrl_alt_delete()->set_req_stream_id(stream_id);
        service_client_->PostNetMessage(m.SerializeAsString());
    }

    std::shared_ptr<WinDesktopManager> RdApplication::GetDesktopManager() {
        return desktop_mgr_;
    }

    void RdApplication::SwitchGdiCapture() {
        if (monitor_capture_plugin_) {
            monitor_capture_plugin_->StopCapturing();
        }
        monitor_capture_plugin_ = plugin_manager_->GetGdiCapturePlugin();
        monitor_capture_plugin_->SetCaptureFps(settings_->encoder_.fps_);
        //monitor_capture_plugin_->EnablePlugin();
        LOGI("Use gdi capture plugin.");
    }

    void RdApplication::PostPanelMessage(std::shared_ptr<Data> msg) {
        if (ws_panel_client_ && msg) {
            ws_panel_client_->PostNetMessage(msg);
        }
    }

    void RdApplication::Exit() {
        if (app_shared_info_) {
            app_shared_info_->Exit();
        }
        if (audio_capture_thread_ && audio_capture_thread_->IsJoinable()) {
            audio_capture_thread_->Join();
        }
        if (app_manager_) {
            app_manager_->Exit();
        }
        if (encoder_thread_) {
            encoder_thread_->Exit();
        }

        exit_app_ = true;
    }

    // ------------------------------------------------------ //
    // Windows
    WinApplication::WinApplication(const AppParams& args)
        : RdApplication(args) {

    }

    WinApplication::~WinApplication() {
        RdApplication::~RdApplication();
    }

    int WinApplication::Run() {
        LoadDxAddress();
        RdApplication::Run();
        return 0;
    }

    void WinApplication::Exit() {
        RdApplication::Exit();
    }

    static std::function<BOOL(DWORD)> s_ctrl_handler;
    static BOOL ConsoleHandler(DWORD signal) {
        if (s_ctrl_handler) {
            return s_ctrl_handler(signal);
        }
        return FALSE;
    }

    void WinApplication::CaptureControlC() {
        s_ctrl_handler = [this](DWORD signal) -> BOOL {
            if (signal == CTRL_C_EVENT) {
                std::cout << "CTRL+C detected, localVar value is " << "\n";
                this->Exit();
                return TRUE;
            }
            return FALSE;
        };
        if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
            LOGE("ERROR: Could not set control handler");
        }
    }

    void WinApplication::LoadDxAddress() {
        app_shared_message_ = DxAddressLoader::LoadDxAddress();
        if (app_shared_message_) {
            app_shared_message_->ipc_port_ = settings_->transmission_.listening_port_;
            app_shared_message_->self_size_ = sizeof(AppSharedMessage);
            app_shared_message_->enable_hook_events_ = 1;
        } else {
            LOGE("LoadDxAddress failed.");
        }
    }

    // Windows
    // ------------------------------------------------------ //
}