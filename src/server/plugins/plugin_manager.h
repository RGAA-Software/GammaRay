//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_PLUGIN_MANAGER_H
#define GAMMARAY_PLUGIN_MANAGER_H

#include <memory>
#include <string>
#include <map>
#include <QLibrary>
#include "tc_common_new/concurrent_hashmap.h"
#include "plugin_ids.h"

namespace tc
{

    class Context;
    class Application;
    class PluginEventRouter;
    class GrPluginInterface;
    class GrStreamPlugin;
    class GrEncoderPlugin;
    class GrNetPlugin;
    class GrMonitorCapturePlugin;
    class GrDataProviderPlugin;

    class PluginManager {
    public:
        static std::shared_ptr<PluginManager> Make(const std::shared_ptr<Application>& app);

        explicit PluginManager(const std::shared_ptr<Application>& app);

        void LoadAllPlugins();
        void RegisterPluginEventsCallback();
        void ReleaseAllPlugins();
        void ReleasePlugin(const std::string& name);

        GrPluginInterface* GetPluginById(const std::string& id);
        GrEncoderPlugin* GetFFmpegEncoderPlugin();
        GrEncoderPlugin* GetNvencEncoderPlugin();
        GrEncoderPlugin* GetAmfEncoderPlugin();
        GrMonitorCapturePlugin* GetDDACapturePlugin();
        GrDataProviderPlugin* GetMockVideoStreamPlugin();

        void VisitAllPlugins(const std::function<void(GrPluginInterface*)>&& visitor);
        void VisitStreamPlugins(const std::function<void(GrStreamPlugin*)>&& visitor);
        void VisitUtilPlugins(const std::function<void(GrPluginInterface*)>&& visitor);
        void VisitEncoderPlugins(const std::function<void(GrEncoderPlugin*)>&& visitor);
        void VisitNetPlugins(const std::function<void(GrNetPlugin*)>&& visitor);
        void DumpPluginInfo();

        void On1Second();

    private:
        std::shared_ptr<Application> app_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;
        std::map<std::string, GrPluginInterface*> plugins_;
        std::map<std::string, QLibrary*> libs_;
        std::shared_ptr<PluginEventRouter> evt_router_ = nullptr;
    };

}

#endif //GAMMARAY_PLUGIN_MANAGER_H
