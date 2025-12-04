//
// Created by RGAA on 17/05/2025.
//

#ifndef GAMMARAY_CT_DASHBOARD_CLIENT_H
#define GAMMARAY_CT_DASHBOARD_CLIENT_H

#include <memory>
#include <asio2/websocket/wss_client.hpp>

namespace tc
{
    class ClientContext;
    class ThunderSdk;

    class CtSpvrClient {
    public:
        explicit CtSpvrClient(const std::shared_ptr<ThunderSdk>& sdk,
                              const std::shared_ptr<ClientContext>& ctx,
                              const std::string& host,
                              int port,
                              const std::string& device_id,
                              const std::string& remote_device_id,
                              const std::string& appkey);
        void Start();
        void Exit() const;

    private:
        void ParseMessage(const std::string& data);
        bool IsAlive() const;
        void Hello();
        void Heartbeat();

    private:
        std::shared_ptr<ThunderSdk> sdk_;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<asio2::wss_client> client_ = nullptr;
        std::string host_;
        int port_ = 0;
        std::string device_id_;
        std::string remote_device_id_;
        std::string appkey_;
        int64_t hb_index_ = 0;
    };

}

#endif //GAMMARAY_CT_DASHBOARD_CLIENT_H
