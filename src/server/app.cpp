//
// Created by RGAA on 2023-12-16.
//

#include "app.h"

// MUST put it here before windows.h
#include "context.h"
#include "tc_capture_new/desktop_capture.h"
#include "tc_capture_new/desktop_capture_factory.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/time_ext.h"
#include "tc_encoder_new/video_encoder_factory.h"
#include "tc_capture_new/capture_message.h"
#include "tc_capture_new/capture_message_maker.h"
#include "tc_capture_new/audio_capture_factory.h"
#include "app/app_manager.h"
#include "app/app_manager_factory.h"
#include "app/app_messages.h"
#include "settings/settings.h"
#include "network/ws_server.h"
#include "ipc/host_ipc_manager.h"
#include "app/encoder_thread.h"
#include "network/net_message_maker.h"
#include "tc_message.pb.h"
#include "app/app_timer.h"
#include "tc_opus_codec_new/opus_codec.h"
#include "network/message_processor.h"
#include "network/ws_client.h"
#include "network/network_factory.h"
#include "network/server_cast.h"
#include "app/app_shared_info.h"
#include "app/win/dx_address_loader.h"
#include "tc_common_new/win32/win_helper.h"
#include "tc_common_new/fft_32.h"
#include "tc_controller/vigem/vigem_controller.h"
#include "tc_controller/vigem_driver_manager.h"
#include "statistics.h"

namespace tc
{

    std::shared_ptr<Application> Application::Make(const AppParams& args) {
        // By OS
        // Windows
        return std::make_shared<WinApplication>(args);
        // Linux
    }

    Application::Application(const AppParams& args) {
        auto settings = Settings::Instance();
        settings_ = settings;
    }

    Application::~Application() {
        LOGI("Application dtor");
    }

    int Application::Run() {
        // presets
        WinHelper::DontCareDPI();

        statistics_ = Statistics::Instance();

        // context
        context_ = std::make_shared<Context>();

        // websocket server
        connection_ = NetworkFactory::MakeConnection(shared_from_this());
        connection_->Start();

        ws_client_ = std::make_shared<WSClient>(context_);
        ws_client_->Start();

        // app manager
        app_manager_ = AppManagerFactory::Make(context_);
        // encoder in thread
        encoder_thread_ = EncoderThread::Make(context_);
        // event bus listener
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        // app shared info
        app_shared_info_ = AppSharedInfo::Make(context_);
        // audio cache
        audio_cache_ = Data::Make(nullptr, 1024*16);

        // app timer
        InitAppTimer();
        // messages
        InitMessages();
        // global audio capture
        if (settings_->capture_.enable_audio_) {
            InitGlobalAudioCapture();
        }

        // init webrtc
        //InitWebRtc();
        // vigem control thread
        control_thread_ = Thread::Make("control", 16);
        control_thread_->Poll();
        // desktop capture
        auto target_monitor = settings_->capture_.capture_monitor_;
        desktop_capture_ = DesktopCaptureFactory::Make(context_->GetMessageNotifier(), target_monitor);

        if (settings_->capture_.enable_video_) {
            if (settings_->capture_.capture_video_type_ == Capture::CaptureVideoType::kVideoHook) {
                StartProcessWithHook();
            } else if (settings_->capture_.capture_video_type_ == Capture::CaptureVideoType::kCaptureScreen) {
                StartProcessWithScreenCapture();
            }
        }

        while (!exit_app_) {
            std::unique_lock<std::mutex> guard(app_msg_cond_mtx_);
            app_msg_cond_.wait(guard, [=, this]() -> bool {
                return exit_app_ || !app_messages_.Empty();
            });

            if (exit_app_) {
                LOGI("Exit app....");
                break;
            }

            auto msg = app_messages_.Front();
            if (msg->task_) {
                msg->task_();
            }
            app_messages_.Pop();
        }

        return 0;
    }

    void Application::InitAppTimer() {
        app_timer_ = std::make_shared<AppTimer>(context_);
        app_timer_->StartTimers();
    }

    void Application::InitMessages() {
        msg_listener_->Listen<MsgBeforeInject>([=, this](const MsgBeforeInject& msg) {
            if (settings_->capture_.IsVideoHook()) {
                this->WriteBoostUpInfoForPid(msg.pid_);
            }
        });

        msg_listener_->Listen<MsgObsInjected>([=, this](const MsgObsInjected& msg) {
#if ENABLE_SHM
            this->InitHostIpcManager(msg.pid_);
#endif
        });

        msg_listener_->Listen<MsgTimer16>([=, this](const MsgTimer16& msg) {
            this->PostGlobalTask([=, this]() {
                this->ReportAudioSpectrum();
            });
        });

        msg_listener_->Listen<MsgTimer100>([=, this](const MsgTimer100& msg) {

        });

        msg_listener_->Listen<MsgTimer1000>([=, this](const MsgTimer1000& msg) {
            statistics_->TickFps();
            statistics_->IncreaseRunningTime();
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
            });
        });

        msg_listener_->Listen<MsgClientDisconnected>([=, this](const MsgClientDisconnected& msg) {
            this->PostGlobalTask([=, this]() {
                if (msg.client_size_ <= 0) {
                    ReleaseVigemController();
                }
            });
        });

    }

    void Application::InitGlobalAudioCapture() {
        if (settings_->capture_.capture_audio_type_ != Capture::CaptureAudioType::kAudioGlobal) {
            return;
        }

        audio_capture_ = AudioCaptureFactory::Make();
        audio_capture_->RegisterFormatCallback([=, this](int samples, int channels, int bits) {
            LOGI("audio format, samples: {}, channels: {}, bits: {}", samples, channels, bits);
            auto stat = Statistics::Instance();
            stat->audio_samples_ = samples;
            stat->audio_channels_ = channels;
            stat->audio_bits_ = bits;
            opus_encoder_ = std::make_shared<OpusAudioEncoder>(samples, channels, bits, OPUS_APPLICATION_AUDIO);
            if (opus_encoder_->valid()) {
                opus_encoder_->SetComplexity(8);
            }
        });

        audio_capture_->RegisterSplitDataCallback([=, this](const tc::DataPtr& left, const tc::DataPtr& right) {
            std::vector<double> fft_left;
            std::vector<double> fft_right;
            FFT32::DoFFT(fft_left, left, 960);
            FFT32::DoFFT(fft_right, right, 960);
            int cpy_size = 120;
            if (fft_left.size() < cpy_size || fft_right.size() < cpy_size) {
                return;
            }
            if (statistics_->left_spectrum_.size() != cpy_size) {
                statistics_->left_spectrum_.resize(cpy_size);
                statistics_->right_spectrum_.resize(cpy_size);
            }
            memcpy(statistics_->left_spectrum_.data(), fft_left.data(), sizeof(double)*cpy_size);
            memcpy(statistics_->right_spectrum_.data(), fft_right.data(), sizeof(double)*cpy_size);
        });

        audio_capture_->RegisterDataCallback([=, this](const tc::DataPtr& data) {
            if (debug_opus_decoder_) {
                static auto pcm_file = File::OpenForWriteB("1.origin.pcm");
                pcm_file->Append((char*)data->DataAddr(), data->Size());
            }
            if (!HasConnectedPeer()) {
                return;
            }

            audio_cache_->Append(data->DataAddr(), data->Size());
            // 2 or 6
            if (++audio_callback_count_ < 2) {
                return;
            }

            {
                auto current_time = TimeExt::GetCurrentTimestamp();
                if (last_post_audio_time_ == 0) {
                    last_post_audio_time_ = current_time;
                }
                auto diff = current_time - last_post_audio_time_;
                last_post_audio_time_ = current_time;
                statistics_->AppendAudioFrameGap(diff);
            }

            //int frame_size = data->Size() / 2 / 2;
            int frame_size = audio_cache_->Offset()/2/2;
            auto encoded_frames = opus_encoder_->Encode(audio_cache_->CStr(), audio_cache_->Offset(), frame_size);
            for (const auto& ef : encoded_frames) {
                auto encoded_data = Data::Make((char*)ef.data(), ef.size());
                auto net_msg = NetMessageMaker::MakeAudioFrameMsg(encoded_data, opus_encoder_->SampleRate(),
                                                                  opus_encoder_->Channels(), opus_encoder_->Bits(), frame_size);
                connection_->PostAudioMessage(net_msg);

                if (debug_opus_decoder_) {
                    if (!opus_decoder_) {
                        opus_decoder_ = std::make_shared<OpusAudioDecoder>(opus_encoder_->SampleRate(), opus_encoder_->Channels());
                    }
                    std::vector<unsigned char> buffer(ef.begin(), ef.end());
                    auto pcm_data = opus_decoder_->Decode(buffer, frame_size, false);
                    static auto pcm_file = File::OpenForWriteB("1.test.pcm");
                    pcm_file->Append((char*)pcm_data.data(), pcm_data.size()*2);
                }
            }

            audio_cache_->Reset();
            audio_callback_count_ = 0;
        });

        audio_capture_thread_ = std::make_shared<Thread>([=, this]() {
            audio_capture_->Prepare();
            audio_capture_->StartRecording();
            LOGI("Start audio recording...");
        }, "global audio capture", false);
    }

    void Application::InitWebRtc() {

    }

    void Application::PostGlobalAppMessage(std::shared_ptr<AppMessage>&& msg) {
        app_messages_.Push(std::move(msg));
        app_msg_cond_.notify_one();
    }

    void Application::PostGlobalTask(std::function<void()>&& task) {
        PostGlobalAppMessage(AppMessageMaker::MakeTaskMessage(std::move(task)));
    }

    void Application::PostIpcMessage(std::shared_ptr<Data>&& msg) {
#if ENABLE_SHM
        host_ipc_managers_.ApplyAll([m = std::move(msg)](const auto& k, const std::shared_ptr<HostIpcManager>& ipc_mgr) {
            if (ipc_mgr->Ready()) {
                ipc_mgr->Send(m);
            }
        });
#endif
    }

    void Application::PostIpcMessage(const std::string& msg) {
        if (settings_->capture_.IsVideoHook()) {
            connection_->PostIpcMessage(msg);
        }
    }

    void Application::StartProcessWithHook() {
        msg_listener_->Listen<MsgVideoFrameEncoded>([=, this](const MsgVideoFrameEncoded& msg) {
            auto net_msg = NetMessageMaker::MakeVideoFrameMsg([=]() -> tc::VideoType {
                return (Encoder::EncoderFormat)msg.frame_format_ == Encoder::EncoderFormat::kH264 ? tc::VideoType::kNetH264 : tc::VideoType::kNetHevc;
            } (), msg.image_->data, msg.frame_index_, msg.frame_width_, msg.frame_height_, msg.key_frame_, msg.monitor_index_, msg.monitor_name_,
            msg.monitor_left_, msg.monitor_top_, msg.monitor_right_, msg.monitor_bottom_);

            if (settings_->app_.debug_enabled_) {
                if (!debug_encode_file_) {
                    debug_encode_file_ = File::OpenForWriteB("1.debug_after_encode.h264");
                }
                debug_encode_file_->Append(msg.image_->data->AsString());
                LOGI("encoded frame callback, size: {}x{}, buffer size: {}", msg.frame_width_, msg.frame_height_, msg.image_->data->Size());
            }
            connection_->PostVideoMessage(net_msg);
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

#if ENABLE_SHM
        // only one, easyhook start & hook
        InitHostIpcManager(settings_->transmission_.listening_port_);
#endif
        fn_start_process();
    }

    void Application::StartProcessWithScreenCapture() {
        msg_listener_->Listen<MsgVideoFrameEncoded>([=, this](const MsgVideoFrameEncoded& msg) {
            auto video_type = [=]() -> tc::VideoType {
                return (Encoder::EncoderFormat)msg.frame_format_ == Encoder::EncoderFormat::kH264 ? tc::VideoType::kNetH264 : tc::VideoType::kNetHevc;
            } ();
            auto net_msg = NetMessageMaker::MakeVideoFrameMsg(video_type, msg.image_->data,msg.frame_index_, msg.frame_width_,
                                                              msg.frame_height_, msg.key_frame_, msg.monitor_index_, msg.monitor_name_,
                                                              msg.monitor_left_, msg.monitor_top_, msg.monitor_right_, msg.monitor_bottom_);
            //LOGI("monitor: {} - {} , ({},{} {},{})",
            //     msg.monitor_index_, msg.monitor_name_, msg.monitor_left_, msg.monitor_top_, msg.monitor_right_, msg.monitor_bottom_);
            connection_->PostVideoMessage(net_msg);
            statistics_->fps_video_encode_->Tick();
        });

        msg_listener_->Listen<CaptureVideoFrame>([=, this](const CaptureVideoFrame& msg) {
            if (!HasConnectedPeer()) {
                return;
            }
            if (!connection_ || connection_->OnlyAudioClient()) {
                return;
            }

            // calculate gaps between 2 captured frames.
            {
                auto current_time = TimeExt::GetCurrentTimestamp();
                if (last_capture_screen_time_ == 0) {
                    last_capture_screen_time_ = current_time;
                }
                auto gap = current_time - last_capture_screen_time_;
                last_capture_screen_time_ = current_time;
                statistics_->AppendFrameGap(gap);
                statistics_->capture_width_ = msg.frame_width_;
                statistics_->capture_height_ = msg.frame_height_;
            }

            // to encode
            encoder_thread_->Encode(msg);
        });

        msg_listener_->Listen<CaptureCursorBitmap>([=, this](const CaptureCursorBitmap& cursor_msg) {
            auto net_msg = NetMessageMaker::MakeCursorInfoSyncMsg(cursor_msg.x_, cursor_msg.y_, cursor_msg.hotspot_x_,
                                                                  cursor_msg.hotspot_y_, cursor_msg.width_, cursor_msg.height_,
                                                                  cursor_msg.visible_, cursor_msg.data_);
            connection_->PostVideoMessage(net_msg);
        });

        if(desktop_capture_) {
            desktop_capture_->StartCapture();
        }
        app_manager_->StartProcess();
    }

#if ENABLE_SHM
    void Application::InitHostIpcManager(uint32_t pid) {
        auto host_ipc_manager = HostIpcManager::Make(pid);
        if (!host_ipc_manager->Ready()) {
            LOGE("HostIpcManager not ready, InitHostIpcManager failed.");
            return;
        }
        host_ipc_manager->RegisterVideoFrameCallback([=, this](const std::shared_ptr<CaptureVideoFrame>& msg, const std::shared_ptr<Data>& buffer) {
            if (this->connection_->GetConnectionPeerCount() <= 0) {
                //LOGI("Not have client, return...");
                return;
            }
            //LOGI("Frame pass from shm: adapter uid: {}, type: {}, frame index: {}, frame_width: {}, frame_height: {}, buffer size: {}",
            //     msg->adapter_uid_, msg->type, msg->frame_index_, msg->frame_width_, msg->frame_height_, buffer?buffer->Size() : 0);
            if (msg->handle_ == 0) {
                encoder_thread_->Encode(buffer, (int)msg->frame_width_, (int)msg->frame_height_, msg->frame_index_);
            }
            else {
                encoder_thread_->Encode(msg->adapter_uid_, msg->handle_, (int) msg->frame_width_,
                                        (int) msg->frame_height_, (int) msg->frame_format_, msg->frame_index_);
            }
        });
        host_ipc_manager->WaitForMessage();
        host_ipc_managers_.Insert(pid, host_ipc_manager);
    }
#endif

    void Application::OnIpcVideoFrame(const std::shared_ptr<CaptureVideoFrame>& msg) {
        if (!HasConnectedPeer()) {
            //LOGI("Not have client, return...");
            return;
        }

        //LOGI("Frame ws ipc pass from shm: adapter uid: {}, type: {}, frame index: {}, frame_width: {}, frame_height: {}, buffer size: {}",
        //     msg->adapter_uid_, (int)msg->type_, msg->frame_index_, msg->frame_width_, msg->frame_height_,  0);
        encoder_thread_->Encode(*msg);
    }

    bool Application::HasConnectedPeer() {
        return connection_ && connection_->GetConnectionPeerCount() > 0;
    }

    void Application::WriteBoostUpInfoForPid(uint32_t pid) {
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

    void Application::InitVigemController() {
        vigem_controller_ = VigemController::Make();
        if (!vigem_controller_->Connect()) {
            return;
        }
        vigem_controller_->AllocController();
    }

    void Application::ReleaseVigemController() {
        if (vigem_controller_) {
            vigem_controller_->Exit();
            vigem_controller_.reset();
        }
    }

    void Application::ProcessGamepadState(const tc::MsgGamepadState &state) {
        if (!vigem_controller_) {
            return;
        }
        control_thread_->Post(SimpleThreadTask::Make([=, this]() {
            vigem_controller_->SendGamepadState(0,state.state_);
        }));
    }

    void Application::ReportAudioSpectrum() {
        auto msg = NetMessageMaker::MakeServerAudioSpectrumMsg();
        if (ws_client_) {
            ws_client_->PostNetMessage(msg);
        }

        // audio spectrum
        if (connection_) {
            connection_->PostAudioMessage(msg);
        }
    }

    void Application::Exit() {
        if (app_shared_info_) {
            app_shared_info_->Exit();
        }
        if (audio_capture_) {
            audio_capture_->Pause();
            audio_capture_->Stop();
        }
        if (audio_capture_thread_ && audio_capture_thread_->IsJoinable()) {
            audio_capture_thread_->Join();
        }
        if (connection_) {
            connection_->Exit();
        }
        if (app_manager_) {
            app_manager_->Exit();
        }
        if (encoder_thread_) {
            encoder_thread_->Exit();
        }

        exit_app_ = true;
        app_msg_cond_.notify_one();
    }

    // ------------------------------------------------------ //
    // Windows
    WinApplication::WinApplication(const AppParams& args)
        : Application(args) {

    }

    WinApplication::~WinApplication() {
        Application::~Application();
    }

    int WinApplication::Run() {
        if(!tc::SetDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            LOGE("SetDpiAwarenessContext error");
        }
        LoadDxAddress();
        Application::Run();
        return 0;
    }

    void WinApplication::Exit() {
        Application::Exit();
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