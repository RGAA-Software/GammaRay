//
// Created by hy on 15/11/2024.
//

#ifndef GAMMARAY_PLUGIN_MANAGER_H
#define GAMMARAY_PLUGIN_MANAGER_H

#include <memory>
#include <string>
#include <map>
#include <QLibrary>

namespace tc
{

    class Context;
    class GrPluginInterface;
    class PluginEventRouter;

    class PluginManager {
    public:
        static std::shared_ptr<PluginManager> Make(const std::shared_ptr<Context>& ctx);

        explicit PluginManager(const std::shared_ptr<Context>& ctx);

        void LoadAllPlugins();
        void RegisterPluginEventsCallback();
        void ReleaseAllPlugins();
        void ReleasePlugin(const std::string& name);

        GrPluginInterface* GetPluginByName(const std::string& name);
        void VisitPlugins(const std::function<void(GrPluginInterface*)>&& visitor);
        void DumpPluginInfo();

        void On1Second();

    private:
        std::shared_ptr<Context> context_ = nullptr;
        std::map<std::string, GrPluginInterface*> plugins_;
        std::map<std::string, QLibrary*> libs_;
        std::shared_ptr<PluginEventRouter> evt_router_ = nullptr;
    };

}

#endif //GAMMARAY_PLUGIN_MANAGER_H
