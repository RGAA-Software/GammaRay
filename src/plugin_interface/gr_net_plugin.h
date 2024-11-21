//
// Created by RGAA on 21/11/2024.
//

#ifndef GAMMARAY_GR_NET_PLUGIN_H
#define GAMMARAY_GR_NET_PLUGIN_H

#include "gr_plugin_interface.h"

namespace tc
{

    class GrNetPlugin : public GrPluginInterface {
    public:
        GrNetPlugin();
        virtual ~GrNetPlugin() override;

        // to see format detail in tc_message_new/tc_message.proto
        // such as : message VideoFrame { ... }
        // you can send it to any clients
        virtual void OnProtoMessage(const std::string& msg);

        //
        void CallbackClientEvent(bool is_proto, const std::string& msg);

    };

}

#endif //GAMMARAY_GR_NET_PLUGIN_H
