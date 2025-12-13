#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

#include <map>
#include <QVariantMap>
#include "notify/notify_defs.h"
#include "tc_spvr_client/spvr_stream.h"

namespace tcrp
{
    class RpMessage;
    class RpCaptureStatistics;
    class RpPluginsInfo;
    class RpServerAudioSpectrum;
    class RpConnectedClientInfo;
    class RpRemoteClipboardResp;
}

namespace tc
{
    class Message;
    class GrSettings;
    class SysInfo;

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
        std::shared_ptr<tcrp::RpMessage> msg_ = nullptr;
        std::shared_ptr<tcrp::RpCaptureStatistics> statistics_ = nullptr;
    };

    class MsgServerAudioSpectrum {
    public:
        std::shared_ptr<tcrp::RpMessage> msg_ = nullptr;
        std::shared_ptr<tcrp::RpServerAudioSpectrum> spectrum_ = nullptr;
    };

    // timer 100ms
    class MsgGrTimer100 {
    public:
    };

    class MsgGrTimer1S {
    public:
    };

    class MsgGrTimer2S {
    public:
    };

    class MsgGrTimer5S {
    public:
    };

    class MsgGrTimer10H {
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
        bool force_update_device_id_ = false;
    };

    // Client id requested
    class MsgRequestedNewDevice {
    public:
        std::string device_id_;
        std::string device_random_pwd_;
        bool force_update_{false};
    };

    // Random password updated
    class MsgRandomPasswordUpdated {
    public:
        std::string device_id_;
        std::string device_random_pwd_;
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
        std::shared_ptr<spvr::SpvrStream> item_;
        bool auto_start_ = false;
    };

    class StreamItemUpdated {
    public:
        std::shared_ptr<spvr::SpvrStream> item_;
    };

    // Close workspace
    class ClearWorkspace {
    public:
        std::shared_ptr<spvr::SpvrStream> item_;
    };

    // reported plugins info
    class MsgPluginsInfo {
    public:
        std::shared_ptr<tcrp::RpPluginsInfo> plugins_info_;
    };

    // remote peer info
    class MsgRemotePeerInfo {
    public:
        // from which stream
        std::string stream_id_;
        std::string desktop_name_;
        std::string os_version_;
    };

    // client connected to panel
    class MsgClientConnectedPanel {
    public:
        std::string stream_id_;
        //tccp::CpSessionType
        int sess_type_{-1};
    };

    // translate
    class MsgLanguageChanged {
    public:
        // LanguageKind
        int language_kind_ = 3;
    };

    // security password updated
    class MsgSecurityPasswordUpdated {
    public:
        std::string security_password_;
    };

    // develop mode update
    class MsgDevelopModeUpdated {
    public:
        bool enabled_ = false;
    };

    // exit all programs
    class MsgForceStopAllPrograms {
    public:
        bool uninstall_service_ = false;
    };

    // notification clicked
    class MsgNotificationClicked {
    public:
        NotifyItem data_;
    };

    // update connected clients info
    class MsgUpdateConnectedClientsInfo {
    public:
        std::vector<std::shared_ptr<tcrp::RpConnectedClientInfo>> clients_info_;
    };

    // remote clipboard resp
    class MsgRemoteClipboardResp {
    public:
        std::string text_msg_;
    };

    // one client disconnect
    class MsgOneClientDisconnect {
    public:
        
    };

    // clear program data
    class MsgForceClearProgramData {
    public:
    };

    class MsgHWInfo {
    public:
        std::shared_ptr<SysInfo> sys_info_ = nullptr;
    };

    // spvr access info
    class StNetworkSpvrAccessInfo;
    class MsgSpvrAccessInfo {
    public:
        std::map<std::string, std::shared_ptr<StNetworkSpvrAccessInfo>> access_info_;
    };

    // user logged in
    class MsgUserLoggedIn {
    public:

    };

    // user logged out
    class MsgUserLoggedOut {
    public:

    };

    // check for updates
    class MsgCheckUpdate {
    public:

    };

    // no available connection
    class MsgNoAvailableConnection {
    public:
        std::string stream_id_;
    };

}

#endif // APP_MESSAGES_H
