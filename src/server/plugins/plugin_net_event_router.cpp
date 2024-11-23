//
// Created by RGAA on 2024/1/25.
//

#include "plugin_net_event_router.h"
#include <memory>
#include <iostream>
#include "app.h"
#include "settings/settings.h"
#include "app/win/win_event_replayer.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_capture_new/capture_message_maker.h"
#include "app_global_messages.h"
#include "app/app_manager.h"
#include "tc_common_new/win32/process_helper.h"
#include "app/app_messages.h"
#include "context.h"
#include "statistics.h"
#include "network/net_message_maker.h"
#include "app/clipboard_manager.h"
#include "tc_capture_new/desktop_capture.h"
#include "tc_message.pb.h"
#include "tc_encoder_new/encoder_messages.h"
#include "plugin_manager.h"
#include "plugin_interface/gr_encoder_plugin.h"
#include "plugin_interface/gr_monitor_capture_plugin.h"

namespace tc {

    PluginNetEventRouter::PluginNetEventRouter(const std::shared_ptr<Application>& app) {
        this->app_ = app;
        this->context_ = app->GetContext();
        this->plugin_manager_ = app->GetPluginManager();
        this->settings_ = Settings::Instance();
        this->statistics_ = Statistics::Instance();
        win_event_replayer_ = std::make_shared<WinEventReplayer>();
        msg_notifier_ = this->app_->GetContext()->GetMessageNotifier();
        msg_listener_ = this->app_->GetContext()->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<CaptureMonitorInfoMessage>([=, this](const CaptureMonitorInfoMessage& msg) {
            win_event_replayer_->UpdateCaptureMonitorInfo(msg);
        });
    }

    void PluginNetEventRouter::ProcessClientConnectedEvent(const std::shared_ptr<GrPluginClientConnectedEvent>& event) {
        // has no effects in plugin mode
        context_->SendAppMessage(MsgInsertIDR {});
        context_->SendAppMessage(RefreshScreenMessage {});
        //
        plugin_manager_->VisitEncoderPlugins([=, this](GrEncoderPlugin* plugin) {
            plugin->InsertIdr();
        });
    }

    void PluginNetEventRouter::ProcessClientDisConnectedEvent(const std::shared_ptr<GrPluginClientDisConnectedEvent>& event) {

    }

    void PluginNetEventRouter::ProcessCapturingMonitorInfoEvent(const std::shared_ptr<GrPluginCapturingMonitorInfoEvent>& event) {
        auto plugin = app_->GetWorkingMonitorCapturePlugin();
        if (!plugin) {
            LOGE("ProcessCapturingMonitorInfoEvent failed, plugin is null.");
            return;
        }
        auto cm_msg = CaptureMonitorInfoMessage {
            .monitors_ = plugin->GetCaptureMonitorInfo(),
            .capturing_monitor_name_ = plugin->GetCapturingMonitorName(),
        };
        win_event_replayer_->UpdateCaptureMonitorInfo(cm_msg);
    }

    void PluginNetEventRouter::ProcessNetEvent(const std::shared_ptr<GrPluginNetClientEvent>& event) {
        if (event->is_proto_) {
            auto msg = std::make_shared<Message>();
            auto parse_res = msg->ParseFromString(event->message_);
            if (!parse_res) {
                std::cout << "PluginNetEventRouter HandleMessage parse error" << std::endl;
                return;
            }
            switch (msg->type()) {
                case kHello: {
                    this->ProcessHelloEvent(std::move(msg));
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
        } else {

        }
    }

    void PluginNetEventRouter::ProcessHelloEvent(std::shared_ptr<Message>&& msg) {
        const auto& hello = msg->hello();
        // todo: 
        //router->enable_audio_ = hello.enable_audio();
        //router->enable_video_ = hello.enable_video();
        app_->GetContext()->SendAppMessage(MsgHello {
            .enable_audio_ = hello.enable_audio(),
            .enable_video_ = hello.enable_video(),
            .enable_controller = hello.enable_controller(),
        });
    }

    void PluginNetEventRouter::ProcessMouseEvent(std::shared_ptr<Message>&& msg) {
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

    void PluginNetEventRouter::ProcessKeyboardEvent(std::shared_ptr<Message>&& msg) {
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

    void PluginNetEventRouter::PostIpcMessage(const std::string& msg) {
        auto task_msg = AppMessageMaker::MakeTaskMessage([=, this]() mutable {
            this->app_->PostIpcMessage(msg);
        });
        app_->PostGlobalAppMessage(std::move(task_msg));
    }

    void PluginNetEventRouter::ProcessGamepadState(std::shared_ptr<Message>&& msg) {
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

    void PluginNetEventRouter::ProcessClientStatistics(std::shared_ptr<Message>&& msg) {
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

    void PluginNetEventRouter::ProcessHeartBeat(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto proto_msg = NetMessageMaker::MakeOnHeartBeatMsg(msg->heartbeat().index());
            app_->PostNetMessage(proto_msg);
        });
    }

    void PluginNetEventRouter::ProcessClipboardInfo(std::shared_ptr<Message>&& msg) {
        auto info = msg->clipboard_info();
        auto clipboard_mgr = app_->GetClipboardManager();
        clipboard_mgr->UpdateRemoteInfo(QString::fromStdString(info.msg()));
    }

    void PluginNetEventRouter::ProcessSwitchMonitor(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto sm = msg->switch_monitor();
            auto plugin = app_->GetWorkingMonitorCapturePlugin();
            if (plugin) {
                plugin->SetCaptureMonitor(sm.index(), sm.name());
                //plugin->SendCapturingMonitorMessage();

                auto cm_msg = CaptureMonitorInfoMessage {
                    .monitors_ = plugin->GetCaptureMonitorInfo(),
                    .capturing_monitor_name_ = plugin->GetCapturingMonitorName(),
                };
                //msg_notifier_->SendAppMessage(msg);
                win_event_replayer_->UpdateCaptureMonitorInfo(cm_msg);

                auto proto_msg = NetMessageMaker::MakeMonitorSwitched(sm.index(), sm.name());
                app_->PostNetMessage(proto_msg);
            }
        });
    }

    void PluginNetEventRouter::ProcessSwitchWorkMode(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto wm = msg->work_mode();
            auto plugin = app_->GetWorkingMonitorCapturePlugin();
            if (wm.mode() == SwitchWorkMode::kWork) {
                plugin->SetCaptureFps(30);
            } else if (wm.mode() == SwitchWorkMode::kGame) {
                plugin->SetCaptureFps(60);
            }
        });
    }

    void PluginNetEventRouter::ProcessChangeMonitorResolution(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto cmr = msg->change_monitor_resolution();
            app_->ResetMonitorResolution(cmr.monitor_name(), cmr.target_width(), cmr.target_height());
        });
    }

    void PluginNetEventRouter::ProcessInsertKeyFrame(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            app_->SendAppMessage(MsgInsertKeyFrame{});
        });
    }
}