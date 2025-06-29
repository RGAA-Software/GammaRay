//
// Created by RGAA on 23/05/2025.
//

#ifndef GAMMARAY_CT_PLUGIN_EVENT_ROUTER_H
#define GAMMARAY_CT_PLUGIN_EVENT_ROUTER_H

#include <memory>
#include <string>

namespace tc
{

    class Settings;
    class ThunderSdk;
    class ClientContext;
    class BaseWorkspace;
    class MessageNotifier;
    class ClientPluginBaseEvent;

    class ClientPluginEventRouter {
    public:
        explicit ClientPluginEventRouter(const std::shared_ptr<BaseWorkspace>& ws);
        void ProcessPluginEvent(const std::shared_ptr<ClientPluginBaseEvent>& event);

    private:
        Settings* settings_ = nullptr;
        std::shared_ptr<BaseWorkspace> ws_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<ThunderSdk> thunder_sdk_ = nullptr;
    };

}

#endif //GAMMARAY_CT_PLUGIN_EVENT_ROUTER_H
