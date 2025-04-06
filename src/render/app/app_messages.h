//
// Created by RGAA on 2023/12/25.
//

#ifndef TC_APPLICATION_APP_MESSAGES_H
#define TC_APPLICATION_APP_MESSAGES_H

#include <memory>

#include "tc_steam_manager_new/steam_entities.h"
#include "tc_controller/vigem/vigem_defs.h"

namespace tc
{

    class Data;
    class Image;

    class AppBaseEvent {
    public:
        virtual ~AppBaseEvent() = default;
    };

    class MsgVideoFrameEncoded {
    public:
        uint32_t frame_width_ = 0;
        uint32_t frame_height_ = 0;
        uint32_t frame_format_ = 0;
        uint64_t frame_index_ = 0;
        bool key_frame_ = false;
        std::shared_ptr<Image> image_ = nullptr;
        std::string monitor_name_;
        int monitor_left_ = 0;
        int monitor_top_ = 0;
        int monitor_right_ = 0;
        int monitor_bottom_ = 0;
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
        int client_size_ = -1;
    };

    //
    class MsgClientDisconnected {
    public:
        int client_size_ = -1;
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


    //: public AppBaseEvent
}

#endif //TC_APPLICATION_APP_MESSAGES_H
