//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_UDP_PLUGIN_H
#define GAMMARAY_UDP_PLUGIN_H

#include "plugin_interface/gr_net_plugin.h"
#include <asio2/udp/udp_server.hpp>

namespace tc
{

    class UdpPlugin : public GrNetPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void OnProtoMessage(const std::string &msg) override;

    private:
        void StartInternal();

    private:
        std::shared_ptr<asio2::udp_server> server_ = nullptr;
        std::shared_ptr<asio2::udp_session> session_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::UdpPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H
