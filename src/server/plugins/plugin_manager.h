//
// Created by hy on 15/11/2024.
//

#ifndef GAMMARAY_PLUGIN_MANAGER_H
#define GAMMARAY_PLUGIN_MANAGER_H

#include <memory>
#include <string>
#include <map>
#include <QLibrary>
#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{

    class Context;
    class PluginEventRouter;
    class GrPluginInterface;
    class GrStreamPlugin;
    class GrEncoderPlugin;

    class PluginManager {
    public:
        static std::shared_ptr<PluginManager> Make(const std::shared_ptr<Context>& ctx);

        explicit PluginManager(const std::shared_ptr<Context>& ctx);

        void LoadAllPlugins();
        void RegisterPluginEventsCallback();
        void ReleaseAllPlugins();
        void ReleasePlugin(const std::string& name);

        GrPluginInterface* GetPluginByName(const std::string& name);
        GrEncoderPlugin* GetFFmpegEncoderPlugin();
        GrEncoderPlugin* GetNvencEncoderPlugin();
        GrEncoderPlugin* GetAmfEncoderPlugin();

        void VisitAllPlugins(const std::function<void(GrPluginInterface*)>&& visitor);
        void VisitStreamPlugins(const std::function<void(GrStreamPlugin*)>&& visitor);
        void VisitUtilPlugins(const std::function<void(GrPluginInterface*)>&& visitor);
        void VisitEncoderPlugins(const std::function<void(GrEncoderPlugin*)>&& visitor);
        void DumpPluginInfo();

        void On1Second();

    private:
        std::shared_ptr<Context> context_ = nullptr;
        tc::ConcurrentHashMap<std::string, GrPluginInterface*> plugins_;
        std::map<std::string, QLibrary*> libs_;
        std::shared_ptr<PluginEventRouter> evt_router_ = nullptr;
        GrEncoderPlugin* working_encoder_plugin_ = nullptr;
    };

}

#endif //GAMMARAY_PLUGIN_MANAGER_H
