//
// Created by RGAA on 2024/1/25.
//

#include "message_processor.h"
#include <memory>
#include <iostream>
#include "rd_app.h"
#include "settings/rd_settings.h"
#include "app/win/win_event_replayer.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_capture_new/capture_message_maker.h"
#include "app_global_messages.h"
#include "app/app_manager.h"
#include "tc_common_new/win32/process_helper.h"
#include "app/app_messages.h"
#include "rd_context.h"
#include "rd_statistics.h"
#include "ws_media_router.h"
#include "net_message_maker.h"
#include "app/clipboard_manager.h"
#include "tc_capture_new/desktop_capture.h"

namespace tc {

    MessageProcessor::MessageProcessor(const std::shared_ptr<RdApplication>& app) {
        this->app_ = app;
        this->settings_ = RdSettings::Instance();
        this->statistics_ = RdStatistics::Instance();
        win_event_replayer_ = std::make_shared<WinEventReplayer>();

        msg_listener_ = this->app_->GetContext()->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<CaptureMonitorInfoMessage>([=, this](const CaptureMonitorInfoMessage& msg) {
            win_event_replayer_->UpdateCaptureMonitorInfo(msg);
        });
    }

    void MessageProcessor::HandleMessage(const std::shared_ptr<WsMediaRouter>& router, const std::string_view message_str) {
        auto msg = std::make_shared<Message>();
        auto parse_res = msg->ParseFromArray(message_str.data(), message_str.size());
        if(!parse_res) {
            std::cout << "MessageProcessor HandleMessage parse error" << std::endl;
            return;
        }
        switch (msg->type()) {
            case kHello: {
                this->ProcessHelloEvent(router, std::move(msg));
                break;
            }
            case kAck:
                break;
            case kHeartBeat: {
                ProcessHeartBeat(std::move(msg));
                break;
            }
            case MessageType::kKeyEvent: {
                ProcessKeyboardEvent(std::move(msg));
                break;
            }
            case MessageType::kMouseEvent: {
                ProcessMouseEvent(std::move(msg));
                break;
            }
            case MessageType::kGamepadState: {
                ProcessGamepadState(std::move(msg));
                break;
            }
            case MessageType::kClientStatistics: {
                ProcessClientStatistics(std::move(msg));
                break;
            }
            case kClipboardInfo: {
                ProcessClipboardInfo(std::move(msg));
                break;
            }
            case kSwitchMonitor: {
                ProcessSwitchMonitor(std::move(msg));
                break;
            }
            case kSwitchWorkMode: {
                ProcessSwitchWorkMode(std::move(msg));
                break;
            }
            case kChangeMonitorResolution: {
                ProcessChangeMonitorResolution(std::move(msg));
                break;
            }
            case kInsertKeyFrame: {
                ProcessInsertKeyFrame(std::move(msg));
                break;
            }
        }
    }

    void MessageProcessor::ProcessHelloEvent(const std::shared_ptr<WsMediaRouter>& router, std::shared_ptr<Message>&& msg) {
        const auto& hello = msg->hello();
        router->enable_audio_ = hello.enable_audio();
        router->enable_video_ = hello.enable_video();
        app_->GetContext()->SendAppMessage(MsgHello {
            .enable_audio_ = hello.enable_audio(),
            .enable_video_ = hello.enable_video(),
            .enable_controller = hello.enable_controller(),
        });
    }

    void MessageProcessor::ProcessMouseEvent(std::shared_ptr<Message>&& msg) {
        if (settings_->app_.IsGlobalReplayMode()) {
            win_event_replayer_->HandleMessage(msg);
        } else {
            //1. convert to ipc message
            const auto& mouse_event = msg->mouse_event();
            auto hwnd = this->app_->GetAppManager()->GetWindowHandle();
            auto hwnd_ptr = reinterpret_cast<uint64_t>(hwnd);
            RECT rect{0,0,0,0};
            if (!ProcessHelper::GetWindowPositionByHwnd((HWND)hwnd, rect)) {
                LOGE("GetWindowPositionByHwnd failed for HWND: {}", hwnd_ptr);
                return;
            }

            int app_width = rect.right - rect.left;
            int app_height = rect.bottom - rect.top;

            auto x = rect.left + app_width * mouse_event.x_ratio();
            auto y = rect.top + app_height * mouse_event.y_ratio();

            LOGI("window rect:{},{},{}x{}, x:{}, y: {}", rect.left, rect.top, app_width, app_height, (int)x, (int)y);
            auto msg = CaptureMessageMaker::MakeMouseEventMessage(hwnd_ptr, (int)x, (int)y,
                                                                  mouse_event.button(), mouse_event.data(),
                                                                  mouse_event.delta_x(), mouse_event.delta_y(),
                                                                  mouse_event.pressed(), mouse_event.released());
            auto msg_str = CaptureMessageMaker::ConvertMessageToString(msg);

            //2. post it
            PostIpcMessage(msg_str);
        }
    }

    void MessageProcessor::ProcessKeyboardEvent(std::shared_ptr<Message>&& msg) {
        bool global_events = settings_->app_.event_replay_mode_ == TargetApplication::EventReplayMode::kGlobal;
        if (global_events) {
            win_event_replayer_->HandleMessage(msg);
        } else {
            // 1. convert to ipc message
            const auto& key_event = msg->key_event();
            auto hwnd = this->app_->GetAppManager()->GetWindowHandle();
            auto hwnd_ptr = reinterpret_cast<uint64_t>(hwnd);

            auto keyboard_msg = CaptureMessageMaker::MakeKeyboardEventMessage(hwnd_ptr, key_event.key_code(), key_event.down(),
                key_event.num_lock_status(), key_event.caps_lock_status());
            auto msg_str = CaptureMessageMaker::ConvertMessageToString(keyboard_msg);
            // 2. post it
            PostIpcMessage(msg_str);
        }
    }

#if ENABLE_SHM
    void MessageProcessor::PostIpcMessage(std::shared_ptr<Data>&& msg_data) {
        auto task_msg = AppMessageMaker::MakeTaskMessage([this, msg = std::move(msg_data)]() mutable {
            this->app_->PostIpcMessage(std::move(msg));
        });
        app_->PostGlobalAppMessage(std::move(task_msg));
    }
#endif

    void MessageProcessor::PostIpcMessage(const std::string& msg) {
        auto task_msg = AppMessageMaker::MakeTaskMessage([=, this]() mutable {
            this->app_->PostIpcMessage(msg);
        });
        app_->PostGlobalAppMessage(std::move(task_msg));
    }

    void MessageProcessor::ProcessGamepadState(std::shared_ptr<Message>&& msg) {
        const auto& gamepad_state = msg->gamepad_state();
        // convert to XINPUT_STATE
//        LOGI("----Gamepad state----");
//        LOGI("button: {:x}, left trigger: {}, right trigger: {}", gamepad_state.buttons(), gamepad_state.left_trigger(), gamepad_state.right_trigger());
//        LOGI("Left thumb: {},{}", gamepad_state.thumb_lx(), gamepad_state.thumb_ly());
//        LOGI("Right thumb: {},{}", gamepad_state.thumb_rx(), gamepad_state.thumb_ry());

        MsgGamepadState msg_ctrl_state{};
        msg_ctrl_state.state_.wButtons = gamepad_state.buttons();
        msg_ctrl_state.state_.bLeftTrigger = gamepad_state.left_trigger();
        msg_ctrl_state.state_.bRightTrigger = gamepad_state.right_trigger();
        msg_ctrl_state.state_.sThumbLX = gamepad_state.thumb_lx();
        msg_ctrl_state.state_.sThumbLY = gamepad_state.thumb_ly();
        msg_ctrl_state.state_.sThumbRX = gamepad_state.thumb_rx();
        msg_ctrl_state.state_.sThumbRY = gamepad_state.thumb_ry();
        //app_->GetContext()->SendAppMessage(msg_ctrl_state);
        app_->ProcessGamepadState(msg_ctrl_state);
    }

    void MessageProcessor::ProcessClientStatistics(std::shared_ptr<Message>&& msg) {
        auto& cst = msg->client_statistics();
        statistics_->decode_durations_.clear();
        statistics_->decode_durations_.insert(statistics_->decode_durations_.begin(),
                                              cst.decode_durations().begin(),
                                              cst.decode_durations().end());
        statistics_->client_video_recv_gaps_.clear();
        statistics_->client_video_recv_gaps_.insert(statistics_->client_video_recv_gaps_.begin(),
                                                    cst.video_recv_gaps().begin(),
                                                    cst.video_recv_gaps().end());
        statistics_->client_fps_video_recv_ = cst.fps_video_recv();
        statistics_->client_fps_render_ = cst.fps_render();
        statistics_->client_recv_media_data_ = cst.recv_media_data();
        statistics_->render_width_ = cst.render_width();
        statistics_->render_height_ = cst.render_height();
    }

    void MessageProcessor::ProcessHeartBeat(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto proto_msg = NetMessageMaker::MakeOnHeartBeatMsg(msg->heartbeat().index());
            app_->PostNetMessage(proto_msg);
        });
    }

    void MessageProcessor::ProcessClipboardInfo(std::shared_ptr<Message>&& msg) {
        auto info = msg->clipboard_info();
        auto clipboard_mgr = app_->GetClipboardManager();
        clipboard_mgr->UpdateRemoteInfo(std::move(msg));
    }

    void MessageProcessor::ProcessSwitchMonitor(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto sm = msg->switch_monitor();
            auto dc = app_->GetDesktopCapture();
            // !!! dc is null in plugin mode !!!
            // todo: NOT WORK IN PLUGIN MODE
            dc->SetCaptureMonitor(/*sm.index()*/0, sm.name());
            dc->SendCapturingMonitorMessage();

            auto proto_msg = NetMessageMaker::MakeMonitorSwitched(sm.name());
            app_->PostNetMessage(proto_msg);
        });
    }

    void MessageProcessor::ProcessSwitchWorkMode(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto wm = msg->work_mode();
            auto dc = app_->GetDesktopCapture();
            // !!! dc is null in plugin mode !!!
            if (wm.mode() == SwitchWorkMode::kWork) {
                dc->SetCaptureFps(30);
            } else if (wm.mode() == SwitchWorkMode::kGame) {
                dc->SetCaptureFps(60);
            }
        });
    }

    void MessageProcessor::ProcessChangeMonitorResolution(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto cmr = msg->change_monitor_resolution();
            app_->ResetMonitorResolution(cmr.monitor_name(), cmr.target_width(), cmr.target_height());
        });
    }

    void MessageProcessor::ProcessInsertKeyFrame(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            app_->SendAppMessage(MsgInsertKeyFrame{});
        });
    }
}