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

namespace tc
{

    class ClientContext;
    class BaseWorkspace;
    class ClientPluginInterface;
    class ClientPluginEventRouter;
    class MediaRecordPluginClient;
    class ClientClipboardPlugin;

    class ClientPluginManager {
    public:
        static std::shared_ptr<ClientPluginManager> Make(const std::shared_ptr<BaseWorkspace>& ws);

        explicit ClientPluginManager(const std::shared_ptr<BaseWorkspace>& ws);

        void LoadAllPlugins();
        void RegisterPluginEventsCallback();
        void ReleaseAllPlugins();
        void ReleasePlugin(const std::string& name);

        ClientPluginInterface* GetPluginById(const std::string& id);
        MediaRecordPluginClient* GetMediaRecordPlugin();
        ClientClipboardPlugin* GetClipboardPlugin();
        ClientPluginInterface* GetFileTransferPlugin();

        void VisitAllPlugins(const std::function<void(ClientPluginInterface*)>&& visitor);
        void DumpPluginInfo();

        void On1Second();

    private:
        std::shared_ptr<BaseWorkspace> ws_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::map<std::string, ClientPluginInterface*> plugins_;
        std::shared_ptr<ClientPluginEventRouter> evt_router_ = nullptr;
    };

}

#endif //GAMMARAY_PLUGIN_MANAGER_H
