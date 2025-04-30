#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

#include "devices/stream_item.h"

namespace tc
{
    class Message;
    class CaptureStatistics;
    class ServerAudioSpectrum;
    class GrSettings;
    class PtPluginsInfo;

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

    class MsgGrTimer5S {
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

    // Settings changed
    class MsgSettingsChanged {
    public:
        GrSettings* settings_ = nullptr;
    };

    // Client id requested
    class MsgRequestedNewDevice {
    public:
        std::string device_id_;
        std::string device_random_pwd_;
        bool force_update_{false};
    };

    // Sync Settings to Render
    class MsgSyncSettingsToRender {
    public:
    };

    // Verify failed!
    // Request new device id - pair
    class MsgForceRequestDeviceId {
    public:

    };

    class StreamItemAdded {
    public:
        StreamItem item_;
    };

    class StreamItemUpdated {
    public:
        StreamItem item_;
    };

    // Close workspace
    class ClearWorkspace {
    public:
        StreamItem item_;
    };

    // reported plugins info
    class MsgPluginsInfo {
    public:
        std::shared_ptr<tc::PtPluginsInfo> plugins_info_;
    };

}

#endif // APP_MESSAGES_H
