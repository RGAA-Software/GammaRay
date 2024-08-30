//
// Created by RGAA on 2024/1/25.
//

#ifndef TC_APPLICATION_MESSAGE_PROCESSOR_H
#define TC_APPLICATION_MESSAGE_PROCESSOR_H

#include <string>
#include <memory>
#include <string_view>
#include "tc_message.pb.h"

namespace tc
{
    class WinEventReplayer;
    class Application;
    class Settings;
    class Data;
    class Statistics;
    class WsMediaRouter;
    class MessageListener;

    class MessageProcessor {
    public :
        explicit MessageProcessor(const std::shared_ptr<Application>& app);
        void HandleMessage(const std::shared_ptr<WsMediaRouter>& router, std::string_view message_str);

    private:
        void ProcessHelloEvent(const std::shared_ptr<WsMediaRouter>& router, std::shared_ptr<Message>&& msg);
        void ProcessMouseEvent(std::shared_ptr<Message>&& msg);
        void ProcessKeyboardEvent(std::shared_ptr<Message>&& msg);
#if ENABLE_SHM
        void PostIpcMessage(std::shared_ptr<Data>&& data);
#endif
        void PostIpcMessage(const std::string& msg);
        void ProcessGamepadState(std::shared_ptr<Message>&& msg);
        void ProcessClientStatistics(std::shared_ptr<Message>&& msg);
        void ProcessHeartBeat(std::shared_ptr<Message>&& msg);
        void ProcessClipboardInfo(std::shared_ptr<Message>&& msg);
        void ProcessSwitchMonitor(std::shared_ptr<Message>&& msg);
        void ProcessSwitchWorkMode(std::shared_ptr<Message>&& msg);
        void ProcessChangeMonitorResolution(std::shared_ptr<Message>&& msg);

    private:
        Settings* settings_ = nullptr;
        Statistics* statistics_ = nullptr;
        std::shared_ptr<Application> app_ = nullptr;
        std::shared_ptr<WinEventReplayer> win_event_replayer_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };
}

#endif //TC_APPLICATION_MESSAGE_PROCESSOR_H
