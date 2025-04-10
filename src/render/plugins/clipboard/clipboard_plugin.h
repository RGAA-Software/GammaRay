//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_CLIPBOARD_PLUGIN_H
#define GAMMARAY_CLIPBOARD_PLUGIN_H

#include "plugin_interface/gr_data_consumer_plugin.h"

namespace tc
{

    class Data;
    class ClipboardManager;
    class CpVirtualFile;

    class ClipboardPlugin : public GrDataConsumerPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;
        void On1Second() override;

        bool OnCreate(const tc::GrPluginParam& param) override;
        bool OnDestroy() override;

        void OnMessage(const std::string &msg) override;
        void OnMessage(const std::shared_ptr<Message> &msg) override;
        void DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) override;

    private:
        std::shared_ptr<ClipboardManager> clipboard_mgr_ = nullptr;
        CpVirtualFile* virtual_file_ = nullptr;
        IDataObject* data_object_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H
