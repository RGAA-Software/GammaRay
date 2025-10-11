//
// Created by RGAA on 17/05/2025.
//

#ifndef GAMMARAY_CT_DASHBOARD_CLIENT_H
#define GAMMARAY_CT_DASHBOARD_CLIENT_H

#include <memory>
#include <asio2/websocket/wss_client.hpp>

namespace tc
{
    class GrContext;

    // Between Panel <-> Spvr
    class GrSpvrClient {
    public:
        explicit GrSpvrClient(const std::shared_ptr<GrContext>& ctx,
                              const std::string& host,
                              int port,
                              const std::string& device_id,
                              const std::string& appkey);
        void Start();
        bool IsStarted();

    private:
        void ParseMessage(std::string_view data);

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<asio2::wss_client> client_ = nullptr;
        std::string host_;
        int port_ = 0;
        std::string device_id_;
        std::string appkey_;
    };

}

#endif //GAMMARAY_CT_DASHBOARD_CLIENT_H
