//
// Created by RGAA on 26/11/2024.
//

#ifndef GAMMARAY_GR_DATA_PROVIDER_PLUGIN_H
#define GAMMARAY_GR_DATA_PROVIDER_PLUGIN_H

#include "gr_plugin_interface.h"

namespace tc
{

    class GrDataProviderPlugin : public GrPluginInterface {
    public:
        GrDataProviderPlugin();
        ~GrDataProviderPlugin() override;
        bool OnCreate(const tc::GrPluginParam& param) override;
        bool OnDestroy() override;
        virtual void StartProviding();
        virtual void StopProviding();

    };

}

#endif //GAMMARAY_GR_DATA_PROVIDER_PLUGIN_H
