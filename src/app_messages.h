#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

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
}

#endif // APP_MESSAGES_H
