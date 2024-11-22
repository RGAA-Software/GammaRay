//
// Created by hy on 22/11/2024.
//

#ifndef GAMMARAY_GR_DATA_PROVIDER_PLUGIN_H
#define GAMMARAY_GR_DATA_PROVIDER_PLUGIN_H

#include "gr_plugin_interface.h"

namespace tc
{
    class Image;
    class Data;

    class GrDataProviderPlugin : public GrPluginInterface {
    public:
        GrDataProviderPlugin();

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;

        void StartProviding();

    };
}

#endif //GAMMARAY_GR_DATA_PROVIDER_PLUGIN_H
