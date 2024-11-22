//
// Created by RGAA on 22/11/2024.
//

#include "gr_data_provider_plugin.h"

namespace tc
{

    GrDataProviderPlugin::GrDataProviderPlugin() {

    }

    bool GrDataProviderPlugin::OnCreate(const tc::GrPluginParam &param) {
        return true;
    }

    bool GrDataProviderPlugin::OnDestroy() {
        return true;
    }

    void GrDataProviderPlugin::StartProviding() {

    }

}