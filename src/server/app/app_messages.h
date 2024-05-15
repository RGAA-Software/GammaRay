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

    class MsgVideoFrameEncoded {
    public:
        uint32_t frame_width_ = 0;
        uint32_t frame_height_ = 0;
        uint32_t frame_format_ = 0;
        uint64_t frame_index_ = 0;
        bool key_frame_ = false;
        std::shared_ptr<Image> image_ = nullptr;
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
}

#endif //TC_APPLICATION_APP_MESSAGES_H
