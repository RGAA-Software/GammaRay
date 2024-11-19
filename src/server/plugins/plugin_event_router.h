//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_PLUGIN_EVENT_ROUTER_H
#define GAMMARAY_PLUGIN_EVENT_ROUTER_H

#include <memory>

namespace tc
{

    class Context;
    class GrPluginBaseEvent;

    class PluginEventRouter {
    public:
        explicit PluginEventRouter(const std::shared_ptr<Context>& ctx);

        void ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event);

    };

}

#endif //GAMMARAY_PLUGIN_EVENT_ROUTER_H