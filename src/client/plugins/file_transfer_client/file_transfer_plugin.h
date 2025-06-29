//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "client/plugin_interface/ct_plugin_interface.h"
#include <map>

namespace tc
{

    class FileTransInterface;

    class FileTransferPlugin : public ClientPluginInterface {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void ShowRootWidget() override;
        void HideRootWidget() override;
        bool OnCreate(const tc::ClientPluginParam& param) override;
        void On1Second() override;
        void OnMessage(std::shared_ptr<Message> msg) override;
        void DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) override;

    private:
        std::shared_ptr<FileTransInterface> file_trans_interface_ = nullptr;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H
