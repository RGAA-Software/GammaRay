//
// Created RGAA on 15/11/2024.
//

#include "event_replayer_plugin.h"
#include <QApplication>
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "win_event_replayer.h"
#include "render/plugins/plugin_ids.h"
#include "tc_common_new/process_util.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_plugin_context.h"

void* GetInstance() {
    static tc::EventReplayerPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string EventReplayerPlugin::GetPluginId() {
        return kEventReplayerPluginId;
    }

    std::string EventReplayerPlugin::GetPluginName() {
        return "Events Replayer";
    }

    std::string EventReplayerPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t EventReplayerPlugin::GetVersionCode() {
        return 110;
    }

    std::string EventReplayerPlugin::GetPluginDescription() {
        return plugin_desc_;
    }

    void EventReplayerPlugin::On1Second() {
        GrPluginInterface::On1Second();

    }
    
    bool EventReplayerPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);

        if (!IsPluginEnabled()) {
            return true;
        }
        root_widget_->hide();

        replayer_ = std::make_shared<WinEventReplayer>();

        return true;
    }

    void EventReplayerPlugin::OnMessage(std::shared_ptr<Message> msg) {
        GrPluginInterface::OnMessage(msg);
        auto stream_id = msg->stream_id();
        if (msg->type() == MessageType::kMouseEvent) {
            ProcessMouseEvent(msg);
        }
        else if (msg->type() == MessageType::kKeyEvent) {
            ProcessKeyboardEvent(msg);
        }
        else if (msg->type() == MessageType::kFocusOutEvent) {
            if (replayer_) {
                replayer_->HandleFocusOutEvent();
            }
        }
        else if (msg->type() == MessageType::kExitControlledEnd) {
            LOGI("recv exit controlled end msg, render will exit and restart.");
            if (replayer_) {
                replayer_->SimulateCtrlWinShiftB();
                plugin_context_->PostDelayTask([]() {
                    ProcessUtil::KillProcess(QApplication::applicationPid());
                }, 100);
            }
        }
    }

    void EventReplayerPlugin::ProcessMouseEvent(std::shared_ptr<Message> msg) {
        // global / inner
        if (replayer_) {
            replayer_->HandleMouseEvent(msg->mouse_event());
        }
    }

    void EventReplayerPlugin::ProcessKeyboardEvent(std::shared_ptr<Message> msg) {
        // global / inner
        if (replayer_) {
            replayer_->HandleKeyEvent(msg->key_event());
        }
    }

    void EventReplayerPlugin::OnClientDisconnected(const std::string &visitor_device_id, const std::string &stream_id) {

    }

    void EventReplayerPlugin::UpdateCaptureMonitorInfo(const CaptureMonitorInfoMessage& msg) {
        if (replayer_) {
            replayer_->UpdateCaptureMonitorInfo(msg);
        }
    }

}
