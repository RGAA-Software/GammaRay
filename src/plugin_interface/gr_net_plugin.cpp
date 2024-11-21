//
// Created by RGAA on 21/11/2024.
//

#include "gr_net_plugin.h"
#include "gr_plugin_events.h"

namespace tc
{

    GrNetPlugin::GrNetPlugin() {
        plugin_type_ = GrPluginType::kNet;
    }

    GrNetPlugin::~GrNetPlugin() {

    }

    void GrNetPlugin::OnProtoMessage(const std::string& msg) {

    }

    void GrNetPlugin::CallbackClientEvent(bool is_proto, const std::string& msg) {
        auto event = std::make_shared<GrPluginNetClientEvent>();
        event->is_proto_ = is_proto;
        event->message_ = msg;
        CallbackEvent(event);
    }

}
