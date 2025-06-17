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
        std::string conn_type_;
        std::string visitor_device_id_;
        int64_t begin_timestamp_{0};
    };

    //
    class MsgClientDisconnected {
    public:
        std::string visitor_device_id_;
        int64_t end_timestamp_{0};
        int64_t duration_{0};
    };

    // Hello message
    class MsgHello {
    public:
        bool enable_audio_ = false;
        bool enable_video_ = false;
        bool enable_controller = false;
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

    // public AppBaseEvent
    class MsgClipboardUpdate : public AppBaseEvent {
    public:
        MsgClipboardUpdate() {
            type_ = EType::kClipboardUpdate;
        }
        HWND hwnd_ = nullptr;
    };

    // 当监听到显示变更的windows消息,然后分发到 dda capture plugin
    class MsgDisplayDeviceChange : public AppBaseEvent {
    public:
        MsgDisplayDeviceChange() {
            type_ = EType::kDisplayDeviceChange;
        }
    };

    // numbers of connected clients
    class MsgConnectedClientCount : public AppBaseEvent {
    public:
        MsgConnectedClientCount() {
            type_ = EType::kConnectedClientCount;
        }
    public:
        int connected_client_count_ = 0;
    };
}

#endif //TC_APPLICATION_APP_MESSAGES_H
