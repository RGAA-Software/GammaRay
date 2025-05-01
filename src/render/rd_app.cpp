//
// Created by RGAA on 2023-12-16.
//

#include "rd_app.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "rd_context.h"
#include "tc_capture_new/desktop_capture.h"
#include "tc_capture_new/desktop_capture_factory.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/time_util.h"
#include "tc_encoder_new/video_encoder_factory.h"
#include "tc_capture_new/capture_message.h"
#include "tc_capture_new/capture_message_maker.h"
#include "tc_capture_new/audio_capture_factory.h"
#include "app/app_manager.h"
#include "app/app_manager_factory.h"
#include "app/app_messages.h"
#include "settings/rd_settings.h"
#include "render_panel/network/ws_panel_server.h"
#include "ipc/host_ipc_manager.h"
#include "app/encoder_thread.h"
#include "network/net_message_maker.h"
#include "tc_message.pb.h"
#include "app/app_timer.h"
#include "tc_opus_codec_new/opus_codec.h"
#include "network/message_processor.h"
#include "network/ws_panel_client.h"
#include "network/network_factory.h"
#include "network/server_cast.h"
#include "app/app_shared_info.h"
#include "app/win/dx_address_loader.h"
#include "tc_common_new/win32/win_helper.h"
#include "tc_common_new/fft_32.h"
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
#include "app/win/d3d11_wrapper.h"

namespace tc
{

    std::shared_ptr<RdApplication> RdApplication::Make(const AppParams& args) {
        // By OS
        // Windows
        return std::make_shared<WinApplication>(args);
        // Linux
    }

    RdApplication::RdApplication(const AppParams& args) {
        auto settings = RdSettings::Instance();
        settings_ = settings;
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
        control_thread_ = Thread::Make("control", 16);
        control_thread_->Poll();
        // desktop capture
        if (settings_->capture_.mock_video_) {
            LOGI("Use mocking video plugin.");
            data_provider_plugin = plugin_manager_->GetMockVideoStreamPlugin();
        }
        else {
            if (settings_->capture_.IsVideoHook()) {
                LOGI("Use hook.");
            }
            else {
                LOGI("Use dda capture plugin.");
                monitor_capture_plugin_ = plugin_manager_->GetDDACapturePlugin();
            }
        }

        if (settings_->capture_.enable_video_) {
            if (settings_->capture_.capture_video_type_ == Capture::CaptureVideoType::kVideoHook) {
                StartProcessWithHook();
            } else if (settings_->capture_.capture_video_type_ == Capture::CaptureVideoType::kCaptureScreen) {
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

        return qapp_->exec();
    }

    void RdApplication::InitAppTimer() {
        app_timer_ = std::make_shared<AppTimer>(context_);
        app_timer_->StartTimers();
    }

    void RdApplication::InitMessages() {
        msg_listener_->Listen<MsgBeforeInject>([=, this](const MsgBeforeInject& msg) {
            if (settings_->capture_.IsVideoHook()) {
                this->WriteBoostUpInfoForPid(msg.pid_);
            }
        });

        msg_listener_->Listen<MsgObsInjected>([=, this](const MsgObsInjected& msg) {

        });

        msg_listener_->Listen<MsgTimer16>([=, this](const MsgTimer16& msg) {
            this->PostGlobalTask([=, this]() {
                this->ReportAudioSpectrum();
            });
        });

        msg_listener_->Listen<MsgTimer100>([=, this](const MsgTimer100& msg) {

        });

        msg_listener_->Listen<MsgTimer1000>([=, this](const MsgTimer1000& msg) {
            statistics_->IncreaseRunningTime();

            auto plugin_manager = context_->GetPluginManager();
            plugin_manager->On1Second();
        });

        msg_listener_->Listen<MsgGamepadState>([=, this](const MsgGamepadState& state) {
            this->ProcessGamepadState(state);
        });

        msg_listener_->Listen<MsgClientConnected>([=, this](const MsgClientConnected& msg) {
            this->PostGlobalTask([=, this]() {
                if (msg.client_size_ == 1) {
                    //
                }
            });
        });

        msg_listener_->Listen<MsgHello>([=, this](const MsgHello& msg) {
            this->PostGlobalTask([=, this]() {
                if (msg.enable_controller) {
                    InitVigemController();
                }

                // send configuration back to client
                this->SendConfigurationBack();
            });
        });

        msg_listener_->Listen<MsgClientDisconnected>([=, this](const MsgClientDisconnected& msg) {
            this->PostGlobalTask([=, this]() {
                if (msg.client_size_ <= 0) {
                    ReleaseVigemController();
                }
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
                    int cpy_size = 120;
                    if (fft_left_.size() < cpy_size || fft_right_.size() < cpy_size) {
                        return;
                    }
                    if (statistics_->left_spectrum_.size() != cpy_size) {
                        statistics_->left_spectrum_.resize(cpy_size);
                        statistics_->right_spectrum_.resize(cpy_size);
                    }
                    memcpy(statistics_->left_spectrum_.data(), fft_left_.data(), sizeof(double)*cpy_size);
                    memcpy(statistics_->right_spectrum_.data(), fft_right_.data(), sizeof(double)*cpy_size);
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
        if (settings_->capture_.IsVideoHook()) {
            PostNetMessage(msg);
        }
    }

    void RdApplication::PostNetMessage(const std::string& msg) {
        plugin_manager_->VisitNetPlugins([=](GrNetPlugin* plugin) {
            plugin->PostProtoMessage(msg, false);
        });
    }

    void RdApplication::StartProcessWithHook() {
        msg_listener_->Listen<MsgVideoFrameEncoded>([=, this](const MsgVideoFrameEncoded& msg) {
            auto net_msg = NetMessageMaker::MakeVideoFrameMsg([=]() -> tc::VideoType {
                return (Encoder::EncoderFormat)msg.frame_format_ == Encoder::EncoderFormat::kH264 ? tc::VideoType::kNetH264 : tc::VideoType::kNetHevc;
            } (), msg.image_->data, msg.frame_index_, msg.frame_width_, msg.frame_height_, msg.key_frame_, msg.monitor_name_,
            msg.monitor_left_, msg.monitor_top_, msg.monitor_right_, msg.monitor_bottom_);

            if (settings_->app_.debug_enabled_) {
                if (!debug_encode_file_) {
                    debug_encode_file_ = File::OpenForWriteB("1.debug_after_encode.h264");
                }
                debug_encode_file_->Append(msg.image_->data->AsString());
                LOGI("encoded frame callback, size: {}x{}, buffer size: {}", msg.frame_width_, msg.frame_height_, msg.image_->data->Size());
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
    }

    void RdApplication::StartProcessWithScreenCapture() {
        msg_listener_->Listen<CaptureVideoFrame>([=, this](const CaptureVideoFrame& msg) {
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

            // plugins: SharedTexture
            if (msg.handle_ > 0) {
                context_->PostStreamPluginTask([=, this]() {
                    plugin_manager_->VisitStreamPlugins([=](GrStreamPlugin *plugin) {
                        plugin->OnRawVideoFrameSharedTexture(msg.handle_);
                    });
                });
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
            //LOGI("Not have client, return...");
            return;
        }

//        LOGI("Frame ws ipc pass from shm: adapter uid: {}, type: {}, frame index: {}, frame_width: {}, frame_height: {}, buffer size: {}",
//             msg->adapter_uid_, (int)msg->type_, msg->frame_index_, msg->frame_width_, msg->frame_height_,  0);
        encoder_thread_->Encode(*msg);
    }

    bool RdApplication::HasConnectedPeer() {
        bool has_working_net_plugin = false;
        plugin_manager_->VisitNetPlugins([&](GrNetPlugin* plugin) {
            if (plugin->IsWorking() && plugin->ConnectedClientSize() > 0) {
                has_working_net_plugin = true;
            }
        });
        return has_working_net_plugin;
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

    void RdApplication::InitVigemController() {
        vigem_controller_ = VigemController::Make();
        if (!vigem_controller_->Connect()) {
            return;
        }
        vigem_controller_->AllocController();
    }

    void RdApplication::ReleaseVigemController() {
        if (vigem_controller_) {
            vigem_controller_->Exit();
            vigem_controller_.reset();
        }
    }

    void RdApplication::ProcessGamepadState(const tc::MsgGamepadState &state) {
        if (!vigem_controller_) {
            return;
        }
        control_thread_->Post(SimpleThreadTask::Make([=, this]() {
            vigem_controller_->SendGamepadState(0,state.state_);
        }));
    }

    void RdApplication::ReportAudioSpectrum() {
        auto st = RdStatistics::Instance();
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kServerAudioSpectrum);
        auto sas = msg->mutable_server_audio_spectrum();
        sas->set_samples(st->audio_samples_);
        sas->set_bits(st->audio_bits_);
        sas->set_channels(st->audio_channels_);
        sas->mutable_left_spectrum()->Add(st->left_spectrum_.begin(), st->left_spectrum_.end());
        sas->mutable_right_spectrum()->Add(st->right_spectrum_.begin(), st->right_spectrum_.end());
        auto net_msg = msg->SerializeAsString();
        if (ws_panel_client_) {
            ws_panel_client_->PostNetMessage(net_msg);
        }

        // audio spectrum
        PostNetMessage(net_msg);
    }

    void RdApplication::SendClipboardMessage(const std::string& msg) {
        tc::Message m;
        m.set_type(tc::kClipboardInfo);
        m.mutable_clipboard_info()->set_msg(msg);
        PostNetMessage(m.SerializeAsString());
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
            monitors_info->Add(std::move(info));
        }
        config->set_capturing_monitor_name(capturing_name);

        //
        PostNetMessage(m.SerializeAsString());
    }

    void RdApplication::RequestRestartMe() {
        tc::Message m;
        m.set_type(tc::kRestartServer);
        m.mutable_restart_server()->set_reason("restart");
        ws_panel_client_->PostNetMessage(m.SerializeAsString());
    }

    void RdApplication::ResetMonitorResolution(const std::string& name, int w, int h) {
        DEVMODE dm;
        dm.dmSize = sizeof(dm);
        dm.dmPelsWidth = w;
        dm.dmPelsHeight = h;
        dm.dmBitsPerPel = 32;
        dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        auto deviceName = StringExt::ToWString(name);//L"\\\\.\\DISPLAY1";
        LONG result = ChangeDisplaySettingsExW(deviceName.c_str(), &dm, nullptr, CDS_FULLSCREEN, nullptr);
        bool ok = result == DISP_CHANGE_SUCCESSFUL;

        tc::Message m;
        m.set_type(tc::kChangeMonitorResolutionResult);
        auto r = m.mutable_change_monitor_resolution_result();
        r->set_monitor_name(name);
        r->set_result(ok);
        PostNetMessage(m.SerializeAsString());
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
                LOGI("Adapter Index:{} Name: {}", adapter_index, StringExt::ToUTF8(desc.Description).c_str());
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