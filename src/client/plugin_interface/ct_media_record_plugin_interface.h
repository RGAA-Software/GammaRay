#pragma once

#include "ct_plugin_interface.h"

namespace tc
{

    class MediaRecordPluginClientInterface : public ClientPluginInterface {
    public:
        virtual void StartRecord() {};
        virtual void EndRecord() {};
    };

}