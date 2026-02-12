//
// Created by RGAA on 17/05/2025.
//

#ifndef GAMMARAY_CT_DASHBOARD_CLIENT_H
#define GAMMARAY_CT_DASHBOARD_CLIENT_H

#include <memory>
#include <asio2/websocket/wss_client.hpp>
#include "tc_common_new/concurrent_type.h"

namespace tc
{
    class GrContext;
    class GrSettings;
    class MessageListener;
    class SysInfo;

    // Between Panel <-> Spvr
    class GrSpvrClient {
    public:
        explicit GrSpvrClient(const std::shared_ptr<GrContext>& ctx,
                              const std::string& host,
                              int port,
                              const std::string& device_id);
        void Start();
        void Stop();
        bool IsStarted();
        bool IsActive();
        void PostBinMessage(const std::string& m);
        bool IsAlive() const;

    private:
        void ParseMessage(const std::string& data);
        void Hello();
        void Heartbeat();

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<asio2::wss_client> client_ = nullptr;
        std::string host_;
        int port_ = 0;
        std::string device_id_;
        std::string appkey_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::atomic_int64_t hb_idx_ = 0;
        int64_t last_received_timestamp_ = 0;
        Mutex<std::shared_ptr<SysInfo>> sys_info_;
    };

}

#endif //GAMMARAY_CT_DASHBOARD_CLIENT_H
