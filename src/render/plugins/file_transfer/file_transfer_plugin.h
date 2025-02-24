//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_RTC_PLUGIN_H
#define GAMMARAY_RTC_PLUGIN_H

#include "plugin_interface/gr_data_consumer_plugin.h"

namespace tc
{

    class Message;

    class FileTransferPlugin : public GrDataConsumerPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        void OnMessage(const std::string& msg) override;
        void OnMessage(const std::shared_ptr<tc::Message>& msg) override;

    private:
        void PostTargetStreamMessage(const std::string& stream_id, const std::string& msg);

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H
