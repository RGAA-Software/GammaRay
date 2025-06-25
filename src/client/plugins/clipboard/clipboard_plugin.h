//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_CLIENT_CLIPBOARD_PLUGIN_H
#define GAMMARAY_CLIENT_CLIPBOARD_PLUGIN_H

#include "plugin_interface/ct_plugin_interface.h"
#include <map>

namespace tc
{

    class ClipboardManager;

    class ClientClipboardPlugin : public ClientPluginInterface {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        bool OnCreate(const tc::ClientPluginParam& param) override;
        void On1Second() override;
        void OnMessage(std::shared_ptr<Message> msg) override;
        void DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) override;

        // from local
        void OnClipboardUpdated();

        bool IsClipboardEnabled();

    private:
        void OnRequestFileBuffer(std::shared_ptr<Message> msg);

    private:
        std::shared_ptr<ClipboardManager> clipboard_mgr_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();


#endif //GAMMARAY_UDP_PLUGIN_H
