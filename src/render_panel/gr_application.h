//
// Created by RGAA on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_GR_APPLICATION_H
#define TC_SERVER_STEAM_GR_APPLICATION_H

#include <memory>
#include <QTimer>
#include <QObject>

namespace tc
{

    class GrContext;
    class WSServer;
    class UdpBroadcaster;
    class GrRenderController;
    class GrSettings;
    class GrSystemMonitor;
    class FileTransferChannel;
    class GrServiceClient;
    class WsSigClient;
    class SigSdkContext;
    class MessageListener;

    class GrApplication : public QObject, public std::enable_shared_from_this<GrApplication> {
    public:

        GrApplication();
        ~GrApplication() override;

        void Init();
        void Exit();

        std::shared_ptr<GrContext> GetContext() { return context_; }
        std::shared_ptr<WSServer> GetWSServer() { return ws_server_; }
        std::shared_ptr<GrServiceClient> GetServiceClient() { return service_client_; }
        bool PostMessage2Service(const std::string& msg);
        void RequestNewClientId(bool force_update);

    private:
        void RefreshSigServerSettings();
        void RegisterMessageListener();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<WSServer> ws_server_ = nullptr;
        std::shared_ptr<UdpBroadcaster> udp_broadcaster_ = nullptr;
        std::shared_ptr<GrSystemMonitor> sys_monitor_ = nullptr;
        std::shared_ptr<FileTransferChannel> file_transfer_ = nullptr;
        std::shared_ptr<GrServiceClient> service_client_ = nullptr;
        std::shared_ptr<WsSigClient> sig_client_ = nullptr;
        QTimer* timer_ = nullptr;
        GrSettings* settings_ = nullptr;
        std::shared_ptr<SigSdkContext> sig_sdk_ctx_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_GR_APPLICATION_H
