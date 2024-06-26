//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_UDP_BROADCASTER_H
#define TC_SERVER_STEAM_UDP_BROADCASTER_H

#include <memory>
#include <string>
#include <QUdpSocket>
#include <QByteArray>

namespace tc
{
    class GrContext;

    class UdpBroadcaster : public QObject {
    public:

        static std::shared_ptr<UdpBroadcaster> Make(const std::shared_ptr<GrContext>& ctx);

        explicit UdpBroadcaster(const std::shared_ptr<GrContext>& ctx);

        void Broadcast(const std::string& msg);
        void Exit();

    private:

        std::shared_ptr<GrContext> context_ = nullptr;
        QUdpSocket* udp_socket_ = nullptr;
    };
}

#endif //TC_SERVER_STEAM_UDP_BROADCASTER_H
