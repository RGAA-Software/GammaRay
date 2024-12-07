//
// Created RGAA on 15/11/2024.
//

#include "udp_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "tc_common_new/log.h"

namespace tc
{

    std::string UdpPlugin::GetPluginId() {
        return kNetUdpPluginId;
    }

    std::string UdpPlugin::GetPluginName() {
        return "UDP Plugin";
    }

    std::string UdpPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t UdpPlugin::GetVersionCode() {
        return 110;
    }

    bool UdpPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrNetPlugin::OnCreate(param);
        StartInternal();
        return true;
    }

    bool UdpPlugin::OnDestroy() {
        return true;
    }

    void UdpPlugin::OnProtoMessage(const std::string& msg) {
        if (session_ && session_->is_started()) {
            session_->async_send(msg);
            LOGI("send msg: {}bytes", msg.size());
        }
    }

    void UdpPlugin::StartInternal() {
        server_ = std::make_shared<asio2::udp_server>();
        server_->bind_recv([](std::shared_ptr<asio2::udp_session> &session_ptr, std::string_view data) {
            LOGI("recv : {} {}", data.size(), (int) data.size(), data.data());


        }).bind_connect([this](auto &session_ptr) {
            session_ = session_ptr;
            LOGI("client enter : {} {} ; {} {}",
                   session_ptr->remote_address().c_str(), session_ptr->remote_port(),
                   session_ptr->local_address().c_str(), session_ptr->local_port());
        }).bind_disconnect([this](auto &session_ptr) {
            session_ = nullptr;
            LOGI("client leave : {} {} {}",
                   session_ptr->remote_address().c_str(), session_ptr->remote_port(),
                   asio2::last_error_msg().c_str());
        }).bind_handshake([](auto &session_ptr) {
            if (asio2::get_last_error())
                LOGI("client handshake failure : {} {} {} {}",
                       session_ptr->remote_address().c_str(), session_ptr->remote_port(),
                       asio2::last_error_val(), asio2::last_error_msg().c_str());
            else
                LOGI("client handshake success : {} {}",
                       session_ptr->remote_address().c_str(), session_ptr->remote_port());
        }).bind_start([&]() {
            if (asio2::get_last_error())
                LOGI("start udp server kcp failure : {} {}",
                       asio2::last_error_val(), asio2::last_error_msg().c_str());
            else
                LOGI("start udp server kcp success : {} {}",
                       server_->listen_address().c_str(), server_->listen_port());
        }).bind_stop([&]() {
            LOGI("stop udp server kcp : {} {}",
                   asio2::last_error_val(), asio2::last_error_msg().c_str());
        }).bind_init([&]() {
            //// Join the multicast group. you can set this option in the on_init(_fire_init) function.
            //server.acceptor().set_option(
            //	// for ipv6, the host must be a ipv6 address like 0::0
            //	asio::ip::multicast::join_group(asio::ip::make_address("ff31::8000:1234")));
            //	// for ipv4, the host must be a ipv4 address like 0.0.0.0
            //	//asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
        });

        // to use kcp, the last param must be : asio2::use_kcp
        server_->start("0.0.0.0", 20400, asio2::use_kcp);
    }

}
