//
// Created by hy on 2024/3/15.
//

#include "server_cast.h"
#include "tc_common_new/log.h"
#include "context.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

namespace tc
{

    std::shared_ptr<ServerCast> ServerCast::Make(std::shared_ptr<Context> &app) {
        return std::make_shared<ServerCast>(app);
    }

    ServerCast::ServerCast(const std::shared_ptr<Context> &app) : app_(app) {

    }

    void ServerCast::Start() {
        std::string_view host = "0.0.0.0";
        std::string_view port = "8022";

        udp_cast_ = std::make_shared<asio2::udp_cast>();

        udp_cast_->bind_recv([&](asio::ip::udp::endpoint &endpoint, std::string_view data) {
            printf("recv : %s %u %zu %.*s\n",
                   endpoint.address().to_string().c_str(), endpoint.port(),
                   data.size(), (int) data.size(), data.data());

            //udp_cast_->async_send(endpoint, data);
        }).bind_start([&]() {
            printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }).bind_stop([&]() {
            printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }).bind_init([&]() {
            //// Join the multicast group.
            udp_cast_->socket().set_option(boost::asio::socket_base::broadcast(true));
            udp_cast_->socket().set_option(
            	asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
        });

        udp_cast_->start(host, port);
    }

    void ServerCast::CastMessage(const std::string &msg) {
        udp_cast_->async_send("255.255.255.255", "8030", msg);
        //udp_cast_->async_send("239.255.0.1", "8030", msg);

        //udp_cast_->send("127.0.0.1", 8022, "sent to self...");

        //asio::ip::udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 8020);
        //udp_cast_->async_send(ep, "ddddd");
        //LOGI("Cast....");
    }

    void ServerCast::Stop() {

    }

}
