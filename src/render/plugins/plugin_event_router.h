//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_PLUGIN_EVENT_ROUTER_H
#define GAMMARAY_PLUGIN_EVENT_ROUTER_H

#include <memory>

namespace tc
{

    class RdContext;
    class RdApplication;
    class RdStatistics;
    class GrPluginBaseEvent;
    class PluginManager;
    class PluginStreamEventRouter;
    class PluginNetEventRouter;
    class MessageNotifier;
    class GrPluginFileTransferBegin;
    class GrPluginFileTransferEnd;
    class GrPluginRemoteClipboardResp;

    class PluginEventRouter {
    public:
        explicit PluginEventRouter(const std::shared_ptr<RdApplication>& app);
        void ProcessPluginEvent(const std::shared_ptr<GrPluginBaseEvent>& event);

    private:
        void SendAnswerSdpToRemote(const std::shared_ptr<GrPluginBaseEvent>& event);
        void SendIceToRemote(const std::shared_ptr<GrPluginBaseEvent>& event);
        void ReportFileTransferBegin(const std::shared_ptr<GrPluginFileTransferBegin>& event);
        void ReportFileTransferEnd(const std::shared_ptr<GrPluginFileTransferEnd>& event);
        void ReportRemoteClipboardResp(const std::shared_ptr<GrPluginRemoteClipboardResp>& event);

    private:
        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        std::shared_ptr<PluginStreamEventRouter> stream_event_router_ = nullptr;
        std::shared_ptr<PluginNetEventRouter> net_event_router_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
        RdStatistics* stat_ = nullptr;

    };

}

#endif //GAMMARAY_PLUGIN_EVENT_ROUTER_H
