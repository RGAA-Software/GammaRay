//
// Created by RGAA on 21/11/2024.
//

#include "gr_net_plugin.h"

namespace tc
{

    GrNetPlugin::GrNetPlugin() {
        plugin_type_ = GrPluginType::kNet;
    }

    GrNetPlugin::~GrNetPlugin() {

    }

    void GrNetPlugin::OnProtoMessage(const std::string& msg) {

    }

}
