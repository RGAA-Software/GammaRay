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

        // to see format details in tc_message_new/tc_message.proto
        // such as : message VideoFrame { ... }
        // you can send it to any clients
        //                       -> client 1
        // Renderer Messages ->  -> client 2
        //                       -> client 3
        virtual void OnProtoMessage(const std::string& msg);

        // to a specific stream
        virtual bool OnTargetStreamMessage(const std::string& stream_id, const std::string& msg);

        // messages from remote(client) -> this plugin -> process it
        // client 1 ->
        // client 2 ->  -> Renderer
        // client 3 ->
        void CallbackClientEvent(bool is_proto, const std::string& msg);

        virtual bool IsOnlyAudioClients();

        virtual int ConnectedClientSize();

    };

}

#endif //GAMMARAY_GR_NET_PLUGIN_H
