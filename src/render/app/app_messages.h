//
// Created by RGAA on 2023/12/25.
//

#ifndef TC_APPLICATION_APP_MESSAGES_H
#define TC_APPLICATION_APP_MESSAGES_H

#include <memory>

#include "tc_steam_manager_new/steam_entities.h"
#include "tc_controller/vigem/vigem_defs.h"
#include "tc_common_new/image.h"

namespace tc
{

    class Data;
    class Image;

    class AppBaseEvent {
    public:
        enum class EType {
            kUnknown,
            kClipboardUpdate,
            kDisplayDeviceChange,
            kConnectedClientCount,
            kClipboardEvent,
            kClientHello,
            kClientHeartbeat,
            kClientDisconnected,
        };
        EType type_ = EType::kUnknown;
    public:
        virtual ~AppBaseEvent() = default;
    };

    class MsgVideoFrameEncoded {
    public:
        uint32_t frame_width_ = 0;
        uint32_t frame_height_ = 0;
        uint32_t frame_encode_type_ = 0;
        uint64_t frame_index_ = 0;
        bool key_frame_ = false;
        std::shared_ptr<Data> data_ = nullptr;
        std::string monitor_name_;
        int monitor_left_ = 0;
        int monitor_top_ = 0;
        int monitor_right_ = 0;
        int monitor_bottom_ = 0;

        RawImageType frame_image_format_ = RawImageType::kI420;
    };

    class MsgAudioFrameEncoded {
    public:
        uint64_t frame_index_ = 0;
        uint32_t sample_ = 0;
        uint32_t channel_ = 0;
        uint32_t format_ = 0;
        std::shared_ptr<Data> audio_ = nullptr;
    };

    class MsgTimer1000 {
    public:
    };

    class MsgTimer2000 {
    public:
    };

    class MsgTimer5000 {
    public:
    };

    class MsgTimer500 {
    public:
    };

    class MsgTimer100 {
    public:
    };

    class MsgTimer16 {
    public:
    };

    //
    class MsgBeforeInject {
    public:
        SteamAppPtr steam_app_;
        uint32_t pid_{};
    };

    class MsgObsInjected {
    public:
        SteamAppPtr steam_app_;
        uint32_t pid_{};
    };

    //
    class MsgGamepadState {
    public:
        int index_{};
        XInputGamepadState state_{};
    };

    //
    class MsgClientConnected {
    public:
        std::string conn_id_;
        std::string conn_type_;
        std::string stream_id_;
        std::string visitor_device_id_;
        int64_t begin_timestamp_{0};
    };

    // render -> plugins
    class MsgClientDisconnected : public AppBaseEvent {
    public:
        MsgClientDisconnected() {
            type_ = EType::kClientDisconnected;
        }
    public:
        std::string conn_id_;
        std::string visitor_device_id_;
        std::string stream_id_;
        int64_t end_timestamp_{0};
        int64_t duration_{0};
    };

    // Hello message from clients
    class MsgClientHello : public AppBaseEvent {
    public:
        MsgClientHello() : AppBaseEvent() {
            type_ = EType::kClientHello;
        }
    public:
        std::string device_id_;
        std::string stream_id_;
        bool enable_audio_ = false;
        bool enable_video_ = false;
        bool enable_controller = false;
        // see: ClientType in tc_messages.proto
        int client_type_ = 100; // 100 is Unknown
        std::string device_name_;
    };

    // Heartbeat from clients
    class MsgClientHeartbeat : public AppBaseEvent {
    public:
        MsgClientHeartbeat() : AppBaseEvent() {
            type_ = EType::kClientHeartbeat;
        }
    public:
        std::string device_id_;
        std::string stream_id_;
        int64_t hb_index_ = 0;
        int64_t timestamp_ = 0;
    };

    class ClipboardMessage {
    public:
        int type_ = 0;
        std::string msg_;
    };

    // Insert key frame
    class MsgInsertKeyFrame {
    public:
    };

    //
    class MsgRenderConnected2Service {
    public:
    };

    class MsgRefreshScreen {
    public:
    };

    class MsgReCreateRefresher {
    public:
    };

    class MsgModifyFps {
    public:
        int fps_ = 30;
    };

    // 当监听到显示变更的windows消息,然后分发到 dda capture plugin
    class MsgDisplayDeviceChange : public AppBaseEvent {
    public:
        MsgDisplayDeviceChange() {
            type_ = EType::kDisplayDeviceChange;
        }
    };

    // numbers of connected clients
    // render -> plugins
    class MsgConnectedClientCount : public AppBaseEvent {
    public:
        MsgConnectedClientCount() {
            type_ = EType::kConnectedClientCount;
        }
    public:
        int connected_client_count_ = 0;
    };

    // Clipboard message
    enum class MsgClipboardType {
        kText,
        kFiles,
    };

    class MsgClipboardFile {
    public:
        std::string file_name_;
        std::string full_path_;
        int64_t total_size_ = 0;
        std::string ref_path_;
    };

    // render panel -> ipc -> render -> plugins
    class MsgClipboardEvent : public AppBaseEvent {
    public:
        MsgClipboardEvent() : AppBaseEvent() {
            type_ = EType::kClipboardEvent;
        }
    public:
        // text or files
        MsgClipboardType clipboard_type_;
        // text mode
        std::string text_msg_;
        // file mode
        std::vector<MsgClipboardFile> files_;
    };

}

#endif //TC_APPLICATION_APP_MESSAGES_H
