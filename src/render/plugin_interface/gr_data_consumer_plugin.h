//
// Created by RGAA on 23/02/2025.
//

#ifndef GAMMARAY_GR_DATA_CONSUMER_PLUGIN_H
#define GAMMARAY_GR_DATA_CONSUMER_PLUGIN_H

#include "gr_plugin_interface.h"

namespace tc
{

    class Message;

    class GrDataConsumerPlugin : public GrPluginInterface {
    public:
        virtual void OnMessage(const std::string& msg) = 0;
        virtual void OnMessage(const std::shared_ptr<Message>& msg) = 0;
    };

}

#endif //GAMMARAY_GR_DATA_CONSUMER_PLUGIN_H
