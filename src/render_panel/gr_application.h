//
// Created by RGAA on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_GR_APPLICATION_H
#define TC_SERVER_STEAM_GR_APPLICATION_H

#include <memory>
#include <QTimer>
#include <QObject>
#include "tc_common_new/message_notifier.h"

namespace tc
{

    class GrContext;
    class WsPanelServer;
    class UdpBroadcaster;
    class GrRenderController;
    class GrSettings;
    class GrSystemMonitor;
    class GrServiceClient;
    class WsSigClient;
    class MgrClientSdk;
    class MessageListener;
    class GrAccountManager;

    class GrApplication : public QObject, public std::enable_shared_from_this<GrApplication> {
    public:

        GrApplication();
        ~GrApplication() override;

        void Init();
        void Exit();

        std::shared_ptr<GrContext> GetContext() { return context_; }
        std::shared_ptr<WsPanelServer> GetWsPanelServer() { return ws_panel_server_; }
        std::shared_ptr<GrServiceClient> GetServiceClient() { return service_client_; }
        std::shared_ptr<GrAccountManager> GetAccountManager() { return account_mgr_; }
        bool PostMessage2Service(const std::string& msg);
        bool PostMessage2Renderer(const std::string& msg);
        void RequestNewClientId(bool force_update);
        std::shared_ptr<MessageNotifier> GetMessageNotifier();

        // 1. device id is empty ?
        // 2. device id & password paired ?
        bool CheckLocalDeviceInfoWithPopup();

    private:
        void RefreshSigServerSettings();
        void RegisterMessageListener();
        void RegisterFirewall();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<WsPanelServer> ws_panel_server_ = nullptr;
        //std::shared_ptr<UdpBroadcaster> udp_broadcaster_ = nullptr;
        std::shared_ptr<GrSystemMonitor> sys_monitor_ = nullptr;
        std::shared_ptr<GrServiceClient> service_client_ = nullptr;
        std::shared_ptr<WsSigClient> sig_client_ = nullptr;
        QTimer* timer_ = nullptr;
        GrSettings* settings_ = nullptr;
        std::shared_ptr<MgrClientSdk> mgr_client_sdk_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<GrAccountManager> account_mgr_ = nullptr;
    };

    extern std::shared_ptr<GrApplication> grApp;

}

#endif //TC_SERVER_STEAM_GR_APPLICATION_H
