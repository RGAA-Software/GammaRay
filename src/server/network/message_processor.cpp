//
// Created by RGAA on 2024/1/25.
//

#include "message_processor.h"
#include <memory>
#include <iostream>
#include "app.h"
#include "settings/settings.h"
#include "app/win/control_event_replayer_win.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_capture_new/capture_message_maker.h"
#include "app_global_messages.h"
#include "app/app_manager.h"
#include "tc_common_new/win32/process_helper.h"
#include "app/app_messages.h"
#include "context.h"
#include "statistics.h"
#include "ws_media_router.h"

namespace tc {

    MessageProcessor::MessageProcessor(const std::shared_ptr<Application>& app) {
        this->app_ = app;
        this->settings_ = Settings::Instance();
        this->statistics_ = Statistics::Instance();
        control_event_replayer_win_ = std::make_shared<ControlEventReplayerWin>();
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
            case kHeartBeat:
                break;
            case kVideoFrame:
                break;
            case kAudioFrame:
                break;
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
            case kCursorInfoSync:
                break;
            case kCaptureStatistics:
                break;
            case kServerAudioSpectrum:
                break;
            case kOnlineGames:
                break;
            case MessageType_INT_MIN_SENTINEL_DO_NOT_USE_:
                break;
            case MessageType_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
    }

    void MessageProcessor::ProcessHelloEvent(const std::shared_ptr<WsMediaRouter>& router, std::shared_ptr<Message>&& msg) {
        const auto& hello = msg->hello();
        router->audio_only_ = hello.only_audio();
    }

    void MessageProcessor::ProcessMouseEvent(std::shared_ptr<Message>&& msg) {
        bool global_events = settings_->app_.event_replay_mode_ == TargetApplication::EventReplayMode::kGlobal;
        if (global_events) {
            control_event_replayer_win_->HandleMessage(msg);
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
            control_event_replayer_win_->HandleMessage(msg);
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
}