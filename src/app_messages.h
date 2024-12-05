#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

namespace tc
{
    class Message;
    class CaptureStatistics;
    class ServerAudioSpectrum;

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

    //
    class MsgServiceAlive {
    public:
        bool alive_ = false;
    };

    // capture statistics
    class MsgCaptureStatistics {
    public:
        std::shared_ptr<tc::Message> msg_ = nullptr;
        std::shared_ptr<tc::CaptureStatistics> statistics_ = nullptr;
    };

    class MsgServerAudioSpectrum {
    public:
        std::shared_ptr<tc::Message> msg_ = nullptr;
        std::shared_ptr<tc::ServerAudioSpectrum> spectrum_ = nullptr;
    };

    // timer 100ms
    class MsgGrTimer100 {
    public:
    };

    class MsgGrTimer1S {
    public:
    };

    // running game ids
    class MsgRunningGameIds {
    public:
        std::vector<uint64_t> game_ids_;
    };

    class AppMsgRestartServer {
    public:

    };

    // connected to service
    class MsgConnectedToService {
    public:

    };

}

#endif // APP_MESSAGES_H
