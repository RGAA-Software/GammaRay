//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/ct_plugin_interface.h"
#include <map>

namespace tc
{

    class MediaRecordPlugin : public ClientPluginInterface {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        bool OnCreate(const tc::ClientPluginParam& param) override;
        void On1Second() override;
        void OnMessage(std::shared_ptr<Message> msg) override;
        void DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) override;

    private:
        std::map<std::string, QLabel*> previewers_;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::MediaRecordPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H
