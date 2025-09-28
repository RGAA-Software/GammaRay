//
// Created by RGAA on 2024/1/25.
//

#include "plugin_net_event_router.h"
#include <memory>
#include <iostream>
#include <thread>
#include <QApplication>
#include "rd_app.h"
#include "rd_context.h"
#include "rd_statistics.h"
#include "tc_message.pb.h"
#include "plugin_manager.h"
#include "app/app_manager.h"
#include "app/app_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "app_global_messages.h"
#include "settings/rd_settings.h"
#include "network/net_message_maker.h"
#include "app/win/win_event_replayer.h"
#include "tc_common_new/process_util.h"
#include "tc_render_panel_message.pb.h"
#include "app/win/win_desktop_manager.h"
#include "tc_encoder_new/encoder_messages.h"
#include "tc_message_new/rp_proto_converter.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_capture_new/capture_message_maker.h"
#include "plugin_interface/gr_video_encoder_plugin.h"
#include "plugin_interface/gr_monitor_capture_plugin.h"

namespace tc {

    PluginNetEventRouter::PluginNetEventRouter(const std::shared_ptr<RdApplication>& app) {
        this->app_ = app;
        this->context_ = app->GetContext();
        this->plugin_manager_ = app->GetPluginManager();
        this->settings_ = RdSettings::Instance();
        this->statistics_ = RdStatistics::Instance();

        msg_notifier_ = this->app_->GetContext()->GetMessageNotifier();
        msg_listener_ = this->app_->GetContext()->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<CaptureMonitorInfoMessage>([=, this](const CaptureMonitorInfoMessage& msg) {
            if (auto plugin = plugin_manager_->GetEventsReplayerPlugin(); plugin) {
                plugin->UpdateCaptureMonitorInfo(msg);
            }
        });
    }

    void PluginNetEventRouter::ProcessClientConnectedEvent(const std::shared_ptr<GrPluginClientConnectedEvent>& event) {
        // has no effects in plugin mode
        context_->SendAppMessage(MsgInsertIDR {});
        context_->SendAppMessage(MsgRefreshScreen{});
        LOGI("New connection established!");

        // tell all plugins that a client connected
        plugin_manager_->VisitAllPlugins([=](GrPluginInterface* plugin) {
            plugin->OnNewClientConnected(event->visitor_device_id_, event->stream_id_, event->conn_type_);
        });

        // tell encoder plugins to insert an I Frame
        plugin_manager_->VisitEncoderPlugins([=, this](GrVideoEncoderPlugin* plugin) {
            plugin->InsertIdr();
        });

        // notify
        context_->SendAppMessage(MsgClientConnected {
            .conn_id_ = event->conn_id_,
            .conn_type_ = event->conn_type_,
            .stream_id_ = event->stream_id_,
            .visitor_device_id_ = event->visitor_device_id_,
            .begin_timestamp_ = event->begin_timestamp_,
        });

        // report it
        ReportClientConnected(event);
    }

    void PluginNetEventRouter::ProcessClientDisConnectedEvent(const std::shared_ptr<GrPluginClientDisConnectedEvent>& event) {
        MsgClientDisconnected msg{};
        msg.conn_id_ = event->conn_id_;
        msg.visitor_device_id_ = event->visitor_device_id_;
        msg.stream_id_ = event->stream_id_;
        msg.end_timestamp_ = event->end_timestamp_;
        msg.duration_ = event->duration_;
        context_->SendAppMessage(msg);

        // tell all plugins that a client disconnected
        plugin_manager_->VisitAllPlugins([=](GrPluginInterface* plugin) {
            plugin->OnClientDisconnected(event->visitor_device_id_, event->stream_id_);
        });

        // report it
        ReportClientDisConnected(event);
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
            .virtual_desktop_bound_rectangle_info_ = plugin->GetVirtualDesktopBoundRectangleInfo()
        };
      
        // to event replayer
        //win_event_replayer_->UpdateCaptureMonitorInfo(cm_msg);
        if (auto erp_plugin = plugin_manager_->GetEventsReplayerPlugin(); erp_plugin) {
            erp_plugin->UpdateCaptureMonitorInfo(cm_msg);
        }

        // to other listeners
        msg_notifier_->SendAppMessage(cm_msg);
    }

    void PluginNetEventRouter::ProcessNetEvent(const std::shared_ptr<GrPluginNetClientEvent>& event) {
        if (event->is_proto_ && event->message_) {
            auto msg = std::make_shared<Message>();
            auto parse_res = msg->ParsePartialFromArray(event->message_->CStr(), event->message_->Size());
            if (!parse_res) {
                std::cout << "PluginNetEventRouter HandleMessage parse error" << std::endl;
                return;
            }

            // notify to all plugins
            plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
                plugin->OnMessage(msg);
            });

            // notify to panel
            context_->PostTask([=, this]() {
                tcrp::RpMessage msg;
                msg.set_type(tcrp::kRpRawRenderMessage);
                auto sub = msg.mutable_raw_render_msg();
                sub->set_msg(event->message_->AsString());
                auto buffer = RpProtoAsData(&msg);
                app_->PostPanelMessage(buffer);
            });

            switch (msg->type()) {
                case kHello: {
                    this->ProcessHelloEvent(std::move(msg));
                    if (event->nt_plugin_type_ == NetPluginType::kUdpKcp) {
                        this->SyncInfoToUdpPlugin(event->socket_fd_, msg->device_id(), msg->stream_id());
                    }
                    break;
                }
                case kAck: {
                    this->ProcessAck(event, msg);
                    break;
                }
                case kHeartBeat: {
                    ProcessHeartBeat(std::move(msg));
                    if (event->nt_plugin_type_ == NetPluginType::kUdpKcp) {
                        this->SyncInfoToUdpPlugin(event->socket_fd_, msg->device_id(), msg->stream_id());
                    }
                    break;
                }
                case MessageType::kClientStatistics: {
                    ProcessClientStatistics(std::move(msg));
                    break;
                }
                case kClipboardInfo: {
                    const auto& sub = msg->clipboard_info();
                    LOGI("Clipboard msg: {}", sub.msg());
                    LOGI("Clipboard files: {}", sub.files_size());
                    for (const auto& file : sub.files()) {
                        LOGI("File: {}", file.full_path());
                    }
                    break;
                }
                case kClipboardInfoResp: {
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
                case kReqCtrlAltDelete: {
                    ProcessCtrlAltDelete(std::move(msg));
                    break;
                }
                // file transmit start
                case MessageType::kFileOperationEvent:
                case MessageType::kFileTransDataPacket:
                case MessageType::kFileTransSaveFileException: {
                    //auto file_trans_plugin = plugin_manager_->GetFileTransferPlugin();
                    //if (file_trans_plugin) {
                    //    file_trans_plugin->OnMessage(msg);
                    //}
                    break;
                }
                // file transmit end
                case MessageType::kClipboardReqAtBegin:
                case MessageType::kClipboardReqAtEnd:
                case MessageType::kClipboardReqBuffer:
                case MessageType::kClipboardRespBuffer: {
                    //if (auto plugin = plugin_manager_->GetClipboardPlugin(); plugin) {
                    //    plugin->OnMessage(msg);
                    //}
                    break;
                }
                case MessageType::kUpdateDesktop: {
                    ProcessUpdateDesktop();
                    break;
                }
                case MessageType::kHardUpdateDesktop: {
                    ProcessHardUpdateDesktop();
                    break;
                }
                case kSwitchFullColorMode: {
                    ProcessSwitchFullColorMode(std::move(msg));
                    break;
                }
                case kStartMediaRecordClientSide: {
                    ProcessStartMediaRecordClientSide();
                    break;
                }
                case kStopMediaRecordClientSide: {
                    ProcessStopMediaRecordClientSide();
                    break;
                }
                case kModifyFps: {
                    ProcessModifyFps(std::move(msg));
                    break;
                }
//                case kFocusOutEvent: {
//                    ProcessFocusOutEvent();
//                    break;
//                }
//                case kExitControlledEnd: {
//                    ProcessExitControlledEnd();
//                    break;
//                }
                case kStopRender: {
                    ProcessUtil::KillProcess(GetCurrentProcessId());
                }
                case kLockDevice: {
                    context_->SendAppMessage(MsgPanelStreamLockScreen{});
                }
                default: {
                   
                }
            }
        } else {

        }
    }

    void PluginNetEventRouter::ProcessHelloEvent(std::shared_ptr<Message>&& msg) {
        const auto& hello = msg->hello();
        auto event = MsgClientHello();
        event.device_id_ = msg->device_id();
        event.stream_id_ = msg->stream_id();
        event.enable_audio_ = hello.enable_audio();
        event.enable_video_ = hello.enable_video();
        event.enable_controller = hello.enable_controller();
        event.client_type_ = hello.client_type();
        event.device_name_ = hello.device_name();
        app_->GetContext()->SendAppMessage(event);

        auto e = std::make_shared<MsgClientHello>(event);
        plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
            plugin->DispatchAppEvent(e);
        });
    }

    void PluginNetEventRouter::ProcessMouseEvent(std::shared_ptr<Message>&& msg) {
        if (settings_->app_.IsGlobalReplayMode()) {
            if (settings_->can_be_operated_) {
                //win_event_replayer_->HandleMessage(msg);
            }
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

            //LOGI("window rect:{},{},{}x{}, x:{}, y: {}", rect.left, rect.top, app_width, app_height, (int)x, (int)y);
            auto mouse_event_msg = CaptureMessageMaker::MakeMouseEventMessage(hwnd_ptr, (int)x, (int)y,
                                                                  mouse_event.button(), mouse_event.data(),
                                                                  mouse_event.delta_x(), mouse_event.delta_y(),
                                                                  mouse_event.pressed(), mouse_event.released());
            auto msg_str = CaptureMessageMaker::ConvertMessageToString(mouse_event_msg);

            //2. post it
            PostIpcMessage(msg_str);
        }
    }

    void PluginNetEventRouter::ProcessKeyboardEvent(std::shared_ptr<Message>&& msg) {
        bool global_events = settings_->app_.event_replay_mode_ == TargetApplication::EventReplayMode::kGlobal;
        if (global_events) {
            if (settings_->can_be_operated_) {
                //win_event_replayer_->HandleMessage(msg);
            }
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

    void PluginNetEventRouter::ProcessClientStatistics(std::shared_ptr<Message>&& msg) {
        auto& cst = msg->client_statistics();
        statistics_->CopyDecodeDurations(cst.decode_durations());
        statistics_->CopyClientVideoRecvGaps(cst.video_recv_gaps());
        statistics_->client_fps_video_recv_ = cst.fps_video_recv();
        statistics_->client_fps_render_ = cst.fps_render();
        statistics_->client_recv_media_data_ = cst.recv_media_data();
        statistics_->render_width_ = cst.render_width();
        statistics_->render_height_ = cst.render_height();
    }

    void PluginNetEventRouter::ProcessHeartBeat(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto& hb = msg->heartbeat();
            auto proto_msg = NetMessageMaker::MakeOnHeartBeatMsg(app_, hb.index(), hb.timestamp());
            app_->PostNetMessage(proto_msg);
        });

        auto event = std::make_shared<MsgClientHeartbeat>();
        event->device_id_ = msg->device_id();
        event->stream_id_ = msg->stream_id();
        event->hb_index_ = msg->heartbeat().index();
        event->timestamp_ = msg->heartbeat().timestamp();
        plugin_manager_->VisitAllPlugins([=, this](GrPluginInterface* plugin) {
            plugin->DispatchAppEvent(event);
        });
    }

    void PluginNetEventRouter::ProcessClipboardInfo(std::shared_ptr<Message>&& msg) {
        //if (auto plugin = plugin_manager_->GetClipboardPlugin(); plugin) {
        //    plugin->OnMessage(msg);
        //}
    }

    void PluginNetEventRouter::ProcessSwitchMonitor(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto sm = msg->switch_monitor();
            auto capture_plugin = app_->GetWorkingMonitorCapturePlugin();
            if (!capture_plugin) {
                return;
            }
            capture_plugin->SetCaptureMonitor(sm.name());
            //plugin->SendCapturingMonitorMessage();

            auto encoder_plugins = app_->GetWorkingVideoEncoderPlugins();
            for (const auto& [k, encoder_plugin] : encoder_plugins) {
                if (encoder_plugin) {
                    encoder_plugin->InsertIdr();
                }
            }

            auto cm_msg = CaptureMonitorInfoMessage {
                .monitors_ = capture_plugin->GetCaptureMonitorInfo(),
                .capturing_monitor_name_ = capture_plugin->GetCapturingMonitorName(),
            };
            //msg_notifier_->SendAppMessage(msg);
            //win_event_replayer_->UpdateCaptureMonitorInfo(cm_msg);
            if (auto erp_plugin = plugin_manager_->GetEventsReplayerPlugin(); erp_plugin) {
                erp_plugin->UpdateCaptureMonitorInfo(cm_msg);
            }

            int mon_index = 0;
            auto mon_index_res = capture_plugin->GetMonIndexByName(sm.name());
            if (mon_index_res.has_value()) {
                mon_index = mon_index_res.value();
            }
            auto proto_msg = NetMessageMaker::MakeMonitorSwitched(sm.name(), mon_index);
            app_->PostNetMessage(proto_msg);
        });
    }

    void PluginNetEventRouter::ProcessSwitchWorkMode(std::shared_ptr<Message>&& msg) {
        app_->PostGlobalTask([=, this]() {
            auto wm = msg->work_mode();
            auto plugin = app_->GetWorkingMonitorCapturePlugin();
            if (!plugin) {
                LOGE("Working monitor capture is empty!");
                return;
            }
            if (wm.mode() == SwitchWorkMode::kWork) {
                plugin->SetCaptureFps(30);
            } else if (wm.mode() == SwitchWorkMode::kGame) {
                plugin->SetCaptureFps(60);
            }
        });
    }

    void PluginNetEventRouter::ProcessSwitchFullColorMode(std::shared_ptr<Message>&& msg) {
        auto sw = msg->switch_full_color_mode();
        this->settings_->SetFullColorMode(sw.enable());
    }

    void PluginNetEventRouter::ProcessStartMediaRecordClientSide() {
        auto encoder_plugins = app_->GetWorkingVideoEncoderPlugins();
        for (const auto& [k, encoder_plugin] : encoder_plugins) {
            if (encoder_plugin) {
                encoder_plugin->InsertIdr();
                encoder_plugin->SetClientSideMediaRecording(true);
            }
        }
    }

    void PluginNetEventRouter::ProcessStopMediaRecordClientSide() {
        auto encoder_plugins = app_->GetWorkingVideoEncoderPlugins();
        for (const auto& [k, encoder_plugin] : encoder_plugins) {
            if (encoder_plugin) {
                encoder_plugin->SetClientSideMediaRecording(false);
            }
        }
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

    void PluginNetEventRouter::ProcessEncodedAudioFrameEvent(const std::shared_ptr<Data>& data, int samples, int channels, int bits, int frame_size) {
        auto net_msg = NetMessageMaker::MakeAudioFrameMsg(data, samples, channels, bits, frame_size);
        //statistics_->AppendMediaBytes(net_msg.size());
        app_->PostNetMessage(net_msg);
    }

    void PluginNetEventRouter::ProcessCtrlAltDelete(std::shared_ptr<Message>&& msg) {
        app_->ReqCtrlAltDelete(msg->device_id(), msg->stream_id());
    }

    void PluginNetEventRouter::ProcessUpdateDesktop() {
        if (context_) {
            context_->SendAppMessage(MsgRefreshScreen{});
        }
    }

    void PluginNetEventRouter::ProcessHardUpdateDesktop() {
        auto desk_manager = app_->GetDesktopManager();
        if (!desk_manager) {
            return;
        }
        desk_manager->UpdateDesktop();
    }

    void PluginNetEventRouter::SyncInfoToUdpPlugin(int64_t socket_fd, const std::string& device_id, const std::string& stream_id) {
        auto udp_plugin = plugin_manager_->GetUdpPlugin();
        if (!udp_plugin) {
            return;
        }
        udp_plugin->SyncInfo(NetSyncInfo {
            .socket_fd_ = socket_fd,
            .device_id_ = device_id,
            .stream_id_ = stream_id
        });
    }

    void PluginNetEventRouter::ProcessRtcReportEvent(const std::shared_ptr<GrPluginRtcReportEvent>& event) {

    }

    void PluginNetEventRouter::ReportClientConnected(const std::shared_ptr<GrPluginClientConnectedEvent>& event) {
        app_->PostGlobalTask([=, this]() {
            tcrp::RpMessage msg;
            msg.set_type(tcrp::kRpClientConnected);
            auto sub = msg.mutable_client_connected();
            sub->set_conn_id(event->conn_id_);
            sub->set_stream_id(event->stream_id_);
            sub->set_conn_type(event->conn_type_);
            sub->set_visitor_device_id(event->visitor_device_id_);
            sub->set_begin_timestamp(event->begin_timestamp_);
            auto buffer = RpProtoAsData(&msg);
            app_->PostPanelMessage(buffer);
        });
    }

    void PluginNetEventRouter::ReportClientDisConnected(const std::shared_ptr<GrPluginClientDisConnectedEvent>& event) {
        app_->PostGlobalTask([=, this]() {
            tcrp::RpMessage msg;
            msg.set_type(tcrp::kRpClientDisConnected);
            auto sub = msg.mutable_client_disconnected();
            sub->set_conn_id(event->conn_id_);
            sub->set_stream_id(event->stream_id_);
            sub->set_visitor_device_id(event->visitor_device_id_);
            sub->set_end_timestamp(event->end_timestamp_);
            sub->set_duration(event->duration_);
            auto buffer = RpProtoAsData(&msg);
            app_->PostPanelMessage(buffer);
        });
    }

    // client -> render 修改帧率
    void PluginNetEventRouter::ProcessModifyFps(std::shared_ptr<Message>&& msg) {
        auto mf = msg->modify_fps();
        int fps = mf.fps();
        if (context_) {
            context_->SendAppMessage(MsgModifyFps{.fps_ = fps});
        }
    }

//    void PluginNetEventRouter::ProcessFocusOutEvent() {
//        win_event_replayer_->HandleFocusOutEvent();
//    }

//    void PluginNetEventRouter::ProcessExitControlledEnd() {
//        LOGI("recv exit controlled end msg, render will exit and restart.");
//        //win_event_replayer_->SimulateCtrlWinShiftB();
//        //exit(0);
//    }

    void PluginNetEventRouter::ProcessAck(const std::shared_ptr<GrPluginNetClientEvent>& ev, const std::shared_ptr<Message>& m) {
        auto sub = m->ack();
        auto ack = std::make_shared<NetMessageAck>();
        ack->send_time_ = sub.send_time();
        ack->resp_time_ = sub.resp_time();
        ack->ch_type_ = ev->nt_channel_type_;
        ack->msg_type_ = m->type();
        ev->from_plugin_->OnMessageAck(ack);
    }
}