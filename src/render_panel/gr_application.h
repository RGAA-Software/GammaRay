//
// Created by RGAA on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_GR_APPLICATION_H
#define TC_SERVER_STEAM_GR_APPLICATION_H

#include <memory>
#include <QTimer>
#include <QObject>
#include <QAbstractNativeEventFilter>
#include "tc_common_new/message_notifier.h"

namespace tc
{

    class Data;
    class Thread;
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
    class MgrDeviceOperator;
    class WinMessageLoop;
    class GrConnectedManager;
    class GrBaseStreamMessage;
    class GrRenderMsgProcessor;
    class ClipboardManager;
    class GrGuardStarter;
    class PanelCompanion;

    class GrApplication : public QObject, public QAbstractNativeEventFilter, public std::enable_shared_from_this<GrApplication> {
    public:

        explicit GrApplication(QWidget* main_window, bool run_automatically);
        ~GrApplication() override;

        bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

        void Init();
        void Exit();

        std::shared_ptr<GrContext> GetContext() { return context_; }
        std::shared_ptr<WsPanelServer> GetWsPanelServer() { return ws_panel_server_; }
        std::shared_ptr<GrServiceClient> GetServiceClient() { return service_client_; }
        std::shared_ptr<GrAccountManager> GetAccountManager() { return account_mgr_; }
        std::shared_ptr<GrRenderMsgProcessor> GetRenderMsgProcessor() { return rd_msg_processor_; }
        std::shared_ptr<ClipboardManager> GetClipboardManager() { return clipboard_mgr_; }
        bool IsServiceConnected();
        // panel -> service
        // msg: protobuf message
        bool PostMessage2Service(const std::string& msg);
        bool IsRendererConnected();
        // panel -> render
        // msg: protobuf message
        bool PostMessage2Renderer(std::shared_ptr<Data> msg);
        void RequestNewClientId(bool force_update);
        std::shared_ptr<MessageNotifier> GetMessageNotifier();

        // manager client
        std::shared_ptr<MgrClientSdk> GetManagerClient();
        // device operator
        std::shared_ptr<MgrDeviceOperator> GetDeviceOperator();

        // 1. device id is empty ?
        // 2. device id & password paired ?
        bool CheckLocalDeviceInfoWithPopup();

        // compare local safety password and password in pr server
        // refresh server if they are not equal
        // 1. when the app starts and has pr server info
        // 2. when pr server info is obtained
        void UpdateServerSecurityPasswordIfNeeded();

        // send the message to remote render in json format
        bool PostMessage2RemoteRender(const std::shared_ptr<GrBaseStreamMessage>& msg);

        // companion for private logics
        PanelCompanion* GetCompanion();

        std::string GetAppkey();

    private:
        void RefreshSigServerSettings();
        void RegisterMessageListener();
        void RegisterFirewall();

        // if there isn't a security password, will pop up a dialog for you to input it
        void CheckSecurityPassword();

        // windows messages looping
        void StartWindowsMessagesLooping();

        // load panel companion
        void LoadPanelCompanion();

    private:
        QWidget* main_window_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<WsPanelServer> ws_panel_server_ = nullptr;
        //std::shared_ptr<UdpBroadcaster> udp_broadcaster_ = nullptr;
        std::shared_ptr<GrSystemMonitor> sys_monitor_ = nullptr;
        std::shared_ptr<GrServiceClient> service_client_ = nullptr;
        QTimer* timer_ = nullptr;
        GrSettings* settings_ = nullptr;
        std::shared_ptr<MgrClientSdk> mgr_client_sdk_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        std::shared_ptr<GrAccountManager> account_mgr_ = nullptr;
        // window messages looping
        std::shared_ptr<Thread> win_msg_thread_ = nullptr;
        std::shared_ptr<WinMessageLoop> win_msg_loop_ = nullptr;
        // listen connections info and show the info panel
        std::shared_ptr<GrConnectedManager> gr_connected_manager_ = nullptr;

        // render messages processor
        // message from render -> panel
        std::shared_ptr<GrRenderMsgProcessor> rd_msg_processor_ = nullptr;

        // clipboard manager
        std::shared_ptr<ClipboardManager> clipboard_mgr_ = nullptr;

        // guard starter
        std::shared_ptr<GrGuardStarter> guard_starter_ = nullptr;

        // is started by OS when logon?
        bool run_automatically_ = false;

        // panel companion
        PanelCompanion* companion_ = nullptr;

    };

    extern std::shared_ptr<GrApplication> grApp;

}

#endif //TC_SERVER_STEAM_GR_APPLICATION_H
