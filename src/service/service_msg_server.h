//
// Created by RGAA on 29/11/2024.
//

#ifndef GAMMARAY_SERVICE_MSG_SERVER_H
#define GAMMARAY_SERVICE_MSG_SERVER_H

#include <string>
#include <asio2/websocket/ws_server.hpp>

namespace tc
{

    class GrService;
    class ServiceContext;

    class ServiceMsgServer {
    public:
        ServiceMsgServer(const std::shared_ptr<ServiceContext>& context);
        void Init(const std::shared_ptr<GrService>& service);
        void Start();
        void ParseMessage(std::string_view data);
        void PostBinaryMessage(const std::string& msg);

    private:
        std::shared_ptr<asio2::ws_server> server_ = nullptr;
        std::shared_ptr<asio2::ws_session> session_ = nullptr;
        std::shared_ptr<ServiceContext> context_ = nullptr;
        std::shared_ptr<GrService> service_ = nullptr;
    };

}

#endif //GAMMARAY_SERVICE_MSG_SERVER_H
