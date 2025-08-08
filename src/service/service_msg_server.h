//
// Created by RGAA on 29/11/2024.
//

#ifndef GAMMARAY_SERVICE_MSG_SERVER_H
#define GAMMARAY_SERVICE_MSG_SERVER_H

#include <string>
#include <asio2/websocket/ws_server.hpp>
#include <asio2/asio2.hpp>
#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{

    class GrService;
    class ServiceContext;
    class RenderManager;

    class SessionWrapper {
    public:
        uint64_t socket_fd_ = 0;
        std::shared_ptr<asio2::ws_session> session_;
        std::string from_ = "";
    };

    class ServiceMsgServer {
    public:
        explicit ServiceMsgServer(const std::shared_ptr<ServiceContext>& context, const std::shared_ptr<RenderManager>& rm);
        void Init(const std::shared_ptr<GrService>& service);
        void Start();
        void ParseMessage(const std::shared_ptr<SessionWrapper>& sw, std::string_view data);
        void PostBinaryMessage(const std::string& msg);

    private:
        void ProcessStartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        void ProcessStopRender();
        void ProcessRestartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        void ProcessHeartBeat(int64_t index);
        void ProcessCtrlAltDelete();

    private:
        std::shared_ptr<RenderManager> render_manager_ = nullptr;
        std::shared_ptr<asio2::ws_server> server_ = nullptr;
        tc::ConcurrentHashMap<uint64_t, std::shared_ptr<SessionWrapper>> sessions_;
        std::shared_ptr<ServiceContext> context_ = nullptr;
        std::shared_ptr<GrService> service_ = nullptr;
        std::string service_path_ = "/service/message";
    };

}

#endif //GAMMARAY_SERVICE_MSG_SERVER_H
