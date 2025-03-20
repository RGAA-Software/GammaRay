//
// Created by RGAA on 21/11/2024.
//

#ifndef GAMMARAY_PLUGIN_STREAM_EVENT_ROUTER_H
#define GAMMARAY_PLUGIN_STREAM_EVENT_ROUTER_H

#include <memory>
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{

    class RdContext;
    class RdApplication;
    class PluginManager;
    class RdStatistics;

    class PluginStreamEventRouter {
    public:
        explicit PluginStreamEventRouter(const std::shared_ptr<RdApplication>& app);

        void ProcessEncodedVideoFrameEvent(const std::shared_ptr<GrPluginEncodedVideoFrameEvent>& event);

    private:
        RdStatistics* statistics_ = nullptr;
        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
    };

}

#endif //GAMMARAY_PLUGIN_STREAM_EVENT_ROUTER_H
