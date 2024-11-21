//
// Created by RGAA on 21/11/2024.
//

#ifndef GAMMARAY_PLUGIN_STREAM_EVENT_ROUTER_H
#define GAMMARAY_PLUGIN_STREAM_EVENT_ROUTER_H

#include <memory>
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    class Context;
    class PluginManager;
    class Statistics;

    class PluginStreamEventRouter {
    public:
        explicit PluginStreamEventRouter(const std::shared_ptr<Context>& ctx);

        void ProcessEncodedVideoFrameEvent(const std::shared_ptr<GrPluginEncodedVideoFrameEvent>& event);

    private:
        Statistics* statistics_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
    };

}

#endif //GAMMARAY_PLUGIN_STREAM_EVENT_ROUTER_H
