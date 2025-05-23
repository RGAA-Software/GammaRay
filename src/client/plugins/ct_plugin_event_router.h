//
// Created by RGAA on 23/05/2025.
//

#ifndef GAMMARAY_CT_PLUGIN_EVENT_ROUTER_H
#define GAMMARAY_CT_PLUGIN_EVENT_ROUTER_H

#include <memory>
#include <string>

namespace tc
{

    class ClientContext;
    class ClientPluginBaseEvent;
    class MessageNotifier;

    class ClientPluginEventRouter {
    public:
        explicit ClientPluginEventRouter(const std::shared_ptr<ClientContext>& app);
        void ProcessPluginEvent(const std::shared_ptr<ClientPluginBaseEvent>& event);

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
    };

}

#endif //GAMMARAY_CT_PLUGIN_EVENT_ROUTER_H
