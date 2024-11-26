//
// Created by RGAA on 26/11/2024.
//

#include "gr_data_provider_plugin.h"

namespace tc
{

    GrDataProviderPlugin::GrDataProviderPlugin() {

    }

    GrDataProviderPlugin::~GrDataProviderPlugin() {

    }

    bool GrDataProviderPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrPluginInterface::OnCreate(param);
        return true;
    }

    bool GrDataProviderPlugin::OnDestroy() {
        return true;
    }

    void GrDataProviderPlugin::StartProviding() {

    }

    void GrDataProviderPlugin::StopProviding() {

    }

}