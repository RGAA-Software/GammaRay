//
// Created by RGAA on 2024-03-30.
//

#include "udp_broadcaster.h"
#include "tc_common_new/log.h"
#include "tc_3rdparty/json/json.hpp"
#include "render_panel/gr_settings.h"

namespace tc
{

    std::shared_ptr<UdpBroadcaster> UdpBroadcaster::Make(const std::shared_ptr<GrContext>& ctx) {
        return std::make_shared<UdpBroadcaster>(ctx);
    }

    UdpBroadcaster::UdpBroadcaster(const std::shared_ptr<GrContext>& ctx) : QObject(nullptr) {
        context_ = ctx;
        udp_socket_ = new QUdpSocket(this);
        udp_socket_->bind(QHostAddress::AnyIPv4, 21034);
    }

    void UdpBroadcaster::Broadcast(const std::string& msg) {
        QHostAddress broadcastAddress("255.255.255.255");
        quint16 broadcastPort = GrSettings::Instance()->udp_listen_port_;
        QByteArray data = msg.c_str();
        udp_socket_->writeDatagram(data, QHostAddress::Broadcast, broadcastPort);
        //LOGI("Udp broadcast: {}", msg);
    }

    void UdpBroadcaster::Exit() {

    }

}