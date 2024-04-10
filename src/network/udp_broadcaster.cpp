//
// Created by RGAA on 2024-03-30.
//

#include "udp_broadcaster.h"
#include "tc_common_new/log.h"
#include "tc_3rdparty/json/json.hpp"
#include "settings.h"

namespace tc
{

    std::shared_ptr<UdpBroadcaster> UdpBroadcaster::Make(const std::shared_ptr<Context>& ctx) {
        return std::make_shared<UdpBroadcaster>(ctx);
    }

    UdpBroadcaster::UdpBroadcaster(const std::shared_ptr<Context>& ctx) : QObject(nullptr) {
        context_ = ctx;
        udp_socket_ = new QUdpSocket(this);
        udp_socket_->bind(QHostAddress::AnyIPv4, 21034);
    }

    void UdpBroadcaster::Broadcast(const std::string& msg) {
        QHostAddress broadcastAddress("255.255.255.255");
        quint16 broadcastPort = Settings::Instance()->udp_server_port_;
        QByteArray data = msg.c_str();
        udp_socket_->writeDatagram(data, QHostAddress::Broadcast, broadcastPort);
        LOGI("Udp broadcast: {}", msg);
    }

    void UdpBroadcaster::Exit() {

    }

}