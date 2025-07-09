//
// Created by RGAA on 2024/1/25.
//

#ifndef TC_APPLICATION_NET_EVENT_ROUTER_H
#define TC_APPLICATION_NET_EVENT_ROUTER_H

#include <string>
#include <memory>
#include <string_view>
#include "plugin_interface/gr_plugin_events.h"

namespace tc
{
    class WinEventReplayer;
    class RdApplication;
    class RdSettings;
    class Data;
    class RdStatistics;
    class MessageListener;
    class MessageNotifier;
    class Message;
    class RdContext;
    class PluginManager;

    class PluginNetEventRouter {
    public :
        explicit PluginNetEventRouter(const std::shared_ptr<RdApplication>& app);
        void ProcessNetEvent(const std::shared_ptr<GrPluginNetClientEvent>& event);
        void ProcessClientConnectedEvent(const std::shared_ptr<GrPluginClientConnectedEvent>& event);
        void ProcessClientDisConnectedEvent(const std::shared_ptr<GrPluginClientDisConnectedEvent>& event);
        void ProcessCapturingMonitorInfoEvent(const std::shared_ptr<GrPluginCapturingMonitorInfoEvent>& event);
        void ProcessEncodedAudioFrameEvent(const std::shared_ptr<Data>& data, int samples, int channels, int bits, int frame_size);
        void ProcessRtcReportEvent(const std::shared_ptr<GrPluginRtcReportEvent>& event);

    private:
        void ProcessHelloEvent(std::shared_ptr<Message>&& msg);
        void ProcessMouseEvent(std::shared_ptr<Message>&& msg);
        void ProcessKeyboardEvent(std::shared_ptr<Message>&& msg);
        void PostIpcMessage(const std::string& msg);
        void ProcessGamepadState(std::shared_ptr<Message>&& msg);
        void ProcessClientStatistics(std::shared_ptr<Message>&& msg);
        void ProcessHeartBeat(std::shared_ptr<Message>&& msg);
        void ProcessClipboardInfo(std::shared_ptr<Message>&& msg);
        void ProcessSwitchMonitor(std::shared_ptr<Message>&& msg);
        void ProcessSwitchWorkMode(std::shared_ptr<Message>&& msg);
        void ProcessChangeMonitorResolution(std::shared_ptr<Message>&& msg);
        void ProcessInsertKeyFrame(std::shared_ptr<Message>&& msg);
        void ProcessCtrlAltDelete(std::shared_ptr<Message>&& msg);
        // 刷新桌面
        void ProcessUpdateDesktop();
        void ProcessHardUpdateDesktop();
        // 全彩模式
        void ProcessSwitchFullColorMode(std::shared_ptr<Message>&& msg);

        void ProcessStartMediaRecordClientSide();

        void ProcessStopMediaRecordClientSide();

        // client -> render 修改帧率
        void ProcessModifyFps(std::shared_ptr<Message>&& msg);

        // client -> render 窗口失焦
        void ProcessFocusOutEvent();

        // client -> render 退出
        void ProcessExitControlledEnd();

        void SyncInfoToUdpPlugin(int64_t socket_fd, const std::string& device_id, const std::string& stream_id);

        // report client connect/disconnect state
        void ReportClientConnected(const std::shared_ptr<GrPluginClientConnectedEvent>& event);
        void ReportClientDisConnected(const std::shared_ptr<GrPluginClientDisConnectedEvent>& event);

    private:
        RdSettings* settings_ = nullptr;
        RdStatistics* statistics_ = nullptr;
        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<WinEventReplayer> win_event_replayer_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<PluginManager> plugin_manager_ = nullptr;
        std::shared_ptr<MessageNotifier> msg_notifier_ = nullptr;
    };
}

#endif //TC_APPLICATION_MESSAGE_PROCESSOR_H
