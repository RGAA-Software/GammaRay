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

        // local clipboard updated
        kPluginClipboardEvent,

        // network event with protobuf
        kPluginNetworkEvent,

        // file transfer begin
        kPluginFileTransBeginEvent,

        // file transfer end
        kPluginFileTransferEndEvent,

        // remote clipboard resp
        kPluginRemoteClipboardResp,
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
        std::function<void()> clicked_cbk_ {nullptr};
    };

    // kPluginClipboardEvent
    class ClientPluginClipboardEvent : public ClientPluginBaseEvent {
    public:
        ClientPluginClipboardEvent() : ClientPluginBaseEvent() {
            event_type_ = ClientPluginEventType::kPluginClipboardEvent;
        }
    public:
        // text or files
        ClipboardType type_;
        // text mode
        std::string text_msg_;
        // file mode
        std::vector<ClipboardFile> cp_files_;
    };

    // kPluginNetworkEvent
    class ClientPluginNetworkEvent : public ClientPluginBaseEvent {
    public:
        ClientPluginNetworkEvent() : ClientPluginBaseEvent() {
            event_type_ = ClientPluginEventType::kPluginNetworkEvent;
        }
    public:
        // sent message via media channel ?
        bool media_channel_ = false;
        std::shared_ptr<Data> buf_ = nullptr;
    };

    // file transfer begin
    //kPluginFileTransBeginEvent,
    class ClientPluginFileTransferBeginEvent : public ClientPluginBaseEvent {
    public:
        ClientPluginFileTransferBeginEvent() {
            event_type_ = ClientPluginEventType::kPluginFileTransBeginEvent;
        }
    public:
        std::string task_id_;
        std::string file_path_;
        std::string direction_;
    };

    // file transfer end
    //kPluginFileTransferEndEvent,
    class ClientPluginFileTransferEndEvent : public ClientPluginBaseEvent {
    public:
        ClientPluginFileTransferEndEvent() {
            event_type_ = ClientPluginEventType::kPluginFileTransferEndEvent;
        }
    public:
        std::string task_id_;
        std::string file_path_;
        std::string direction_;
        bool success_;
    };

    // kPluginRemoteClipboardResp
    class ClientPluginRemoteClipboardResp : public ClientPluginBaseEvent {
    public:
        ClientPluginRemoteClipboardResp() : ClientPluginBaseEvent() {
            event_type_ = ClientPluginEventType::kPluginRemoteClipboardResp;
        }
    public:
        int content_type_{0};
        std::string remote_info_;
    };
}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
