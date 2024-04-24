#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

#include "tc_message.pb.h"

namespace tc
{

    // can't connect or not installed
    class MsgViGEmState {
    public:
        bool ok_ = false;
    };

    // install the ViGEm
    class MsgInstallViGEm {
    public:

    };

    //
    class MsgServerAlive {
    public:
        bool alive_ = false;
    };

    // capture statistics
    class MsgCaptureStatistics {
    public:
        std::shared_ptr<tc::Message> msg_ = nullptr;
        tc::CaptureStatistics statistics_;
    };

    // timer 100ms
    class MsgGrTimer100 {
    public:
    };

}

#endif // APP_MESSAGES_H
