//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_EVENTS_H
#define GAMMARAY_GR_PLUGIN_EVENTS_H

#include <string>
#include <memory>
#include <any>
#include "tc_common_new/time_util.h"

// from plugins ---> exe

namespace tc
{

    class Data;
    class Image;

    enum class ClientPluginEventType {
        kPluginUnknownType,

        // test begin
        kPluginTestEvent,
        // test end

        // 显示通知消息
        kPluginNotifyMsgEvent,
    };

    class ClientPluginBaseEvent {
    public:
        ClientPluginBaseEvent() {
            created_timestamp_ = TimeUtil::GetCurrentTimestamp();
        }
        virtual ~ClientPluginBaseEvent() = default;
    public:
        std::string plugin_name_;
        ClientPluginEventType event_type_{ClientPluginEventType::kPluginUnknownType};
        std::any extra_;
        uint64_t created_timestamp_ = 0;
    };

    ///
    class ClientPluginTestEvent : public ClientPluginBaseEvent {
    public:
        ClientPluginTestEvent() : ClientPluginBaseEvent() {
            event_type_ = ClientPluginEventType::kPluginTestEvent;
        }
    public:
        std::string message_;
    };


    class ClientPluginNotifyMsgEvent : public ClientPluginBaseEvent {
    public:
        ClientPluginNotifyMsgEvent() : ClientPluginBaseEvent() {
            event_type_ = ClientPluginEventType::kPluginNotifyMsgEvent;
        }
    public:
        std::string title_;
        std::string message_;
    };

}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
