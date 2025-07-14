//
// Created RGAA on 15/11/2024.
//

#include "event_replayer_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "render/plugins/plugin_ids.h"
#include "tc_message.pb.h"

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

        return true;
    }

    void EventReplayerPlugin::OnMessage(std::shared_ptr<Message> msg) {
        GrPluginInterface::OnMessage(msg);
        auto stream_id = msg->stream_id();

    }

    void EventReplayerPlugin::OnClientDisconnected(const std::string &visitor_device_id, const std::string &stream_id) {

    }

}
