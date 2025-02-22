//
// Created RGAA on 15/11/2024.
//

#include "udp_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "plugin_interface/gr_plugin_events.h"

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
        udp_listen_port_ = (int)GetConfigIntParam("udp-listen-port");
        auto config_listen_port = (int)GetConfigIntParam("listen-port");
        if (config_listen_port > 0) {
            udp_listen_port_ = config_listen_port;
        }
        LOGI("Listen port: {}", udp_listen_port_);
        StartInternal();
        return true;
    }

    bool UdpPlugin::OnDestroy() {
        return true;
    }

    void UdpPlugin::PostProtoMessage(const std::string& msg) {
        sessions_.VisitAll([=, this](int64_t socket_fd, const std::shared_ptr<UdpSession>& us) {
            us->sess_->async_send(msg, [](std::size_t bytes_sent) {

            });
        });
    }

    bool UdpPlugin::PostTargetStreamProtoMessage(const std::string& stream_id, const std::string& msg) {
        bool found_target_stream = false;
        sessions_.VisitAll([=, &found_target_stream](int64_t socket_fd, const std::shared_ptr<UdpSession>& us) {
            if (us->stream_id_ == stream_id) {
                found_target_stream = true;
                us->sess_->async_send(msg, [](std::size_t bytes_sent) {

                });
            }
        });
        return found_target_stream;
    }

    void UdpPlugin::StartInternal() {
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::udp_session> &sess_ptr) -> int64_t {
            auto& s = sess_ptr->socket();
            return (int64_t)s.native_handle();
        };

        server_ = std::make_shared<asio2::udp_server>();
        server_->bind_recv([=, this](std::shared_ptr<asio2::udp_session>& session_ptr, std::string_view data) {
            auto msg = std::string(data.data(), data.size());
            auto socket_fd = fn_get_socket_fd(session_ptr);
            this->OnClientEventCame(true, socket_fd, NetPluginType::kUdpKcp, msg);

        }).bind_connect([=, this](std::shared_ptr<asio2::udp_session>& session_ptr) {
            auto socket_fd = fn_get_socket_fd(session_ptr);
            auto udp_sess = std::make_shared<UdpSession>();

            udp_sess->socket_fd_ = socket_fd;
            udp_sess->sess_ = session_ptr;
            sessions_.Insert(socket_fd, udp_sess);

            NotifyMediaClientConnected();
            LOGI("client enter : {} {} ; {} {}",
                   session_ptr->remote_address().c_str(), session_ptr->remote_port(),
                   session_ptr->local_address().c_str(), session_ptr->local_port());

        }).bind_disconnect([=, this](auto &session_ptr) {
            auto socket_fd = fn_get_socket_fd(session_ptr);
            if (sessions_.HasKey(socket_fd)) {
                sessions_.Remove(socket_fd);
            }

            NotifyMediaClientDisConnected();
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

        });

        server_->start("0.0.0.0", udp_listen_port_, asio2::use_kcp);
    }

    void UdpPlugin::NotifyMediaClientConnected() {
        auto event = std::make_shared<GrPluginClientConnectedEvent>();
        this->CallbackEvent(event);
    }

    void UdpPlugin::NotifyMediaClientDisConnected() {
        auto event = std::make_shared<GrPluginClientDisConnectedEvent>();
        this->CallbackEvent(event);
    }

    int UdpPlugin::ConnectedClientSize() {
        return (int)sessions_.Size();
    }

    bool UdpPlugin::IsOnlyAudioClients() {
        return false;
    }

    bool UdpPlugin::IsWorking() {
        return ConnectedClientSize() > 0;
    }

    void UdpPlugin::SyncInfo(const NetSyncInfo& info) {
        GrNetPlugin::SyncInfo(info);
        sessions_.VisitAll([=, this](int64_t fd, std::shared_ptr<UdpSession>& sess) {
            if (info.socket_fd_ == fd) {
                sess->device_id_ = info.device_id_;
                sess->stream_id_ = info.stream_id_;
            }
        });
    }

}
