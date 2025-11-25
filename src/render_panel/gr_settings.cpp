//
// Created by RGAA on 2024/4/10.
//

#include "gr_settings.h"
#include <sstream>
#include <QApplication>
#include "version_config.h"
#include "gr_app_messages.h"
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/uuid.h"
#include "tc_common_new/base64.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/win32/dxgi_mon_detector.h"
#include "tc_common_new/win32/audio_device_helper.h"

namespace tc
{

    void GrSettings::Init(const std::shared_ptr<MessageNotifier>& notifier) {
        notifier_ = notifier;
    }

    void GrSettings::Load() {
        sp_ = SharedPreference::Instance();
        version_ = std::format("V {}", PROJECT_VERSION);

        log_file_ = sp_->Get(kStLogFile, "true");
        encoder_select_type_ = sp_->Get(kStEncoderSelectType, "auto");
        encoder_name_ = sp_->Get(kStEncoderName, "nvenc");
        //encoder_format_ = sp_->Get(kStEncoderFormat, "h264");
        //encoder_bitrate_ = sp_->Get(kStEncoderBitrate, "10");
        //encoder_fps_ = sp_->Get(kStEncoderFPS, "60");
        //encoder_resolution_type_ = sp_->Get(kStEncoderResolutionType, "origin");
        //encoder_width_ = sp_->Get(kStEncoderWidth, "1280");
        //encoder_height_ = sp_->Get(kStEncoderHeight, "720");

        capture_audio_type_ = sp_->Get(kStCaptureAudioType, "global");
        capture_video_ = sp_->Get(kStCaptureVideo, "true");
        capture_video_type_ = sp_->Get(kStCaptureVideoType, "global");
        capture_audio_device_ = sp_->Get(kStCaptureAudioDevice, "");

        network_listening_ip_ = sp_->Get(kStListeningIp, "");
        webrtc_enabled_ = sp_->Get(kStWebRTCEnabled, kStTrue);
        udp_listen_port_ = sp_->GetInt(kStUdpListenPort, 20381);
        udp_kcp_enabled_ = sp_->Get(kStUdpKcpEnabled, kStTrue);

        file_transfer_folder_ = sp_->Get(kStFileTransferFolder, "");
        if (file_transfer_folder_.empty()) {
            file_transfer_folder_ = qApp->applicationDirPath().toStdString();
        }

        if (capture_audio_device_.empty()) {
            auto audio_devices = AudioDeviceHelper::DetectAudioDevices();
            for (const auto& dev : audio_devices) {
                if (dev.default_device_) {
                    SetCaptureAudioDeviceId(dev.id_);
                }
            }
        }
    }

    void GrSettings::Dump() {
        std::stringstream ss;
        ss << "---------------------GrSettings Begin---------------------" << std::endl;
        ss << "log_file_: " << log_file_ << std::endl;
        ss << "encoder_select_type_: " << encoder_select_type_ << std::endl;
        ss << "encoder_name_: " << encoder_name_ << std::endl;
        ss << "encoder_format_: " << GetEncoderFormat() << std::endl;
        ss << "encoder_bitrate_: " << GetBitrate() << std::endl;
        ss << "res resize enabled ? : " << IsResResizeEnabled() << std::endl;
        ss << "encoder_width_: " << GetResWidth() << std::endl;
        ss << "encoder_height_: " << GetResHeight() << std::endl;
        ss << "capture_audio_: " << IsCaptureAudioEnabled() << std::endl;
        ss << "capture_audio_type_: " << capture_audio_type_ << std::endl;
        ss << "capture_video_: " << capture_video_ << std::endl;
        ss << "capture_video_type: " << capture_video_type_ << std::endl;
        ss << "panel_srv_port_: " << GetPanelServerPort() << std::endl;
        ss << "render_srv_port_: " << GetRenderServerPort() << std::endl;
        ss << "websocket_enabled_:" << IsWebSocketEnabled() << std::endl;
        ss << "webrtc_enabled_:" << webrtc_enabled_ << std::endl;
        ss << "capture_audio_device_: " << capture_audio_device_ << std::endl;
        ss << "device_id_: " << GetDeviceId() << std::endl;
        ss << "device_random_pwd: " << GetDeviceRandomPwd() << std::endl;
        ss << "device_security_pwd_md5: " << GetDeviceSecurityPwd() << std::endl;
        ss << "udp_listen_port_:" << udp_listen_port_ << std::endl;
        ss << "relay host: " << GetRelayServerHost() << std::endl;
        ss << "relay port: " << GetRelayServerPort() << std::endl;
        ss << "spvr server host: " << GetSpvrServerHost() << std::endl;
        ss << "spvr server port: " << GetSpvrServerPort() << std::endl;
        ss << "---------------------GrSettings End-----------------------" << std::endl;
        LOGI("\n {}", ss.str());
    }

    void GrSettings::ClearData() {
        this->SetDeviceId("");
        this->SetDeviceRandomPwd("");
        this->SetDeviceSecurityPwd("");
        this->SetSpvrServerHost("");
        this->SetSpvrServerPort("");
        this->SetRelayServerHost("");
        this->SetRelayServerPort("");
        this->SetSpvrAccessInfo("");
    }

    void GrSettings::SetEnableResResize(bool enabled) {
        sp_->Put(kStEncoderResResize, enabled ? kStTrue : kStFalse);
    }

    bool GrSettings::IsResResizeEnabled() {
        auto value = sp_->Get(kStEncoderResResize);
        return !value.empty() && value == kStTrue;
    }

    void GrSettings::SetBitrate(int br) {
        sp_->Put(kStEncoderBitrate, std::to_string(br));
    }

    int GrSettings::GetBitrate() {
        auto value = std::atoi(sp_->Get(kStEncoderBitrate).c_str());
        return value > 0 ? value : 10;
    }

    void GrSettings::SetFPS(int fps) {
        sp_->Put(kStEncoderFPS, std::to_string(fps));
    }

    int GrSettings::GetFPS() {
        auto value = std::atoi(sp_->Get(kStEncoderFPS).c_str());
        return value > 0 ? value : 60;
    }

    void GrSettings::SetResWidth(int width) {
        sp_->Put(kStEncoderWidth, std::to_string(width));
    }

    int GrSettings::GetResWidth() {
        auto value = std::atoi(sp_->Get(kStEncoderWidth).c_str());
        return value > 0 ? value : 1280;
    }

    void GrSettings::SetResHeight(int height) {
        sp_->Put(kStEncoderHeight, std::to_string(height));
    }

    int GrSettings::GetResHeight() {
        auto value = std::atoi(sp_->Get(kStEncoderHeight).c_str());
        return value > 0 ? value : 720;
    }

    void GrSettings::SetEncoderFormat(int idx) {
        // h264
        if (idx == 0) {
            sp_->Put(kStEncoderFormat, "h264");
        } else if (idx == 1) {
            sp_->Put(kStEncoderFormat, "h265");
        }
    }

    std::string GrSettings::GetEncoderFormat() {
        auto value = sp_->Get(kStEncoderFormat);
        return value.empty() ? "h264" : value;
    }

    void GrSettings::SetCaptureVideo(bool enabled) {
        capture_video_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStCaptureVideo, capture_video_);
    }

    void GrSettings::SetCaptureAudio(bool enabled) {
        sp_->Put(kStCaptureAudio, enabled ? kStTrue : kStFalse);
    }

    bool GrSettings::IsCaptureAudioEnabled() {
        auto value = sp_->Get(kStCaptureAudio);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetCaptureAudioDeviceId(const std::string& name) {
        capture_audio_device_ = name;
        sp_->Put(kStCaptureAudioDevice, capture_audio_device_);
    }

    void GrSettings::SetFileTransferFolder(const std::string& path) {
        file_transfer_folder_ = path;
        sp_->Put(kStFileTransferFolder, path);
    }

    void GrSettings::SetListeningIp(const std::string& ip) {
        network_listening_ip_ = ip;
        sp_->Put(kStListeningIp, ip);
    }

    void GrSettings::SetWebSocketEnabled(bool enabled) {
        sp_->Put(kStWebSocketEnabled, enabled ? kStTrue : kStFalse);
    }

    bool GrSettings::IsWebSocketEnabled() {
        auto value = sp_->Get(kStWebSocketEnabled);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetWebRTCEnabled(bool enabled) {
        webrtc_enabled_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStWebRTCEnabled, webrtc_enabled_);
    }

    void GrSettings::SetUdpKcpEnabled(bool enabled) {
        udp_kcp_enabled_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStUdpKcpEnabled, udp_kcp_enabled_);
    }

    // Device ID // Set
    void GrSettings::SetDeviceId(const std::string& id) {
        sp_->Put(kStDeviceId, id);
    }

    // Device ID // Get
    std::string GrSettings::GetDeviceId() {
        return sp_->Get(kStDeviceId, "");
    }

    // Device Random Pwd // Set
    void GrSettings::SetDeviceRandomPwd(const std::string& pwd) {
        sp_->Put(kStDeviceRandomPwd, pwd);
    }

    // Device Random Pwd // Get
    std::string GrSettings::GetDeviceRandomPwd() {
        return sp_->Get(kStDeviceRandomPwd, "");
    }

    // Device Security Pwd // Set
    void GrSettings::SetDeviceSecurityPwd(const std::string& pwd) {
        sp_->Put(kStDeviceSafetyPwd, pwd);
    }

    // Device Security Pwd // Set
    std::string GrSettings::GetDeviceSecurityPwd() {
        return sp_->Get(kStDeviceSafetyPwd, "");
    }

    // Panel Server Port // Set
    void GrSettings::SetPanelServerPort(int port) {
        sp_->Put(kStPanelListeningPort, std::to_string(port));
    }

    // Panel Server Port // Get
    int GrSettings::GetPanelServerPort() {
        auto value = std::atoi(sp_->Get(kStPanelListeningPort, "").c_str());
        return value > 0 ? value : 20369;
    }

    // Render Server Port // Set
    void GrSettings::SetRenderServerPort(int port) {
        sp_->Put(kStNetworkListenPort, std::to_string(port));
    }

    // Render Server Port // Get
    int GrSettings::GetRenderServerPort() {
        auto value = std::atoi(sp_->Get(kStNetworkListenPort, "").c_str());
        return value > 0 ? value : 20371;
    }

    // Relay
    // Host
    void GrSettings::SetRelayServerHost(const std::string& host) {
        sp_->Put(kStRelayServerHost, host);
    }

    std::string GrSettings::GetRelayServerHost() {
        return sp_->Get(kStRelayServerHost, "");
    }

    // Relay
    // Port
    void GrSettings::SetRelayServerPort(const std::string& port) {
        sp_->Put(kStRelayServerPort, port);
    }

    int GrSettings::GetRelayServerPort() {
        return std::atoi(sp_->Get(kStRelayServerPort, "").c_str());
    }

    // Relay
    bool GrSettings::HasRelayServerConfig() {
        return !GetRelayServerHost().empty() && GetRelayServerPort() > 0;
    }

    // Spvr
    // Set Host
    void GrSettings::SetSpvrServerHost(const std::string& host) {
        sp_->Put(kStSpvrServerHost, host);
    }

    // Spvr
    // Get Host
    std::string GrSettings::GetSpvrServerHost() {
        return sp_->Get(kStSpvrServerHost, "");
    }

    // Spvr
    // Set Port
    void GrSettings::SetSpvrServerPort(const std::string& port) {
        sp_->Put(kStSpvrServerPort, port);
    }

    // Spvr
    // Get Port
    int GrSettings::GetSpvrServerPort() {
        return std::atoi(sp_->Get(kStSpvrServerPort, "").c_str());
    }

    bool GrSettings::HasSpvrServerConfig() {
        return !GetSpvrServerHost().empty() && GetSpvrServerPort() > 0;
    }

    void GrSettings::SetScreenRecordingPath(const std::string& path) {
        sp_->Put(kStScreenRecordingPath, path);
    }

    std::string GrSettings::GetScreenRecordingPath() const {
        return sp_->Get(kStScreenRecordingPath, "");
    }

    // show max window
    void GrSettings::SetShowingMaxWindow(bool enable) {
        sp_->Put(kStShowMaxWindow, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsMaxWindowEnabled() {
        return sp_->Get(kStShowMaxWindow) == kStTrue;
    }

    void GrSettings::SetMaxNumOfScreen(const std::string& num) {
        sp_->Put(kStMaxNumOfScreen, num);
    }

    std::string GrSettings::GetMaxNumOfScreen() {
        auto value = sp_->Get(kStMaxNumOfScreen);
        return value.empty() ? "4" : value;
    }

    void GrSettings::SetDisplayClientLogo(int enable) {
        sp_->Put(kStDisplayClientLogo, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsClientLogoDisplaying() {
        auto value = sp_->Get(kStDisplayClientLogo);
        return !value.empty() && value == kStTrue;
    }

    // can be operated
    // Settings->Security Settings
    void GrSettings::SetCanBeOperated(bool enable) {
        sp_->Put(kStCanBeOperated, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsBeingOperatedEnabled() {
        auto value = sp_->Get(kStCanBeOperated);
        return value.empty() || value == kStTrue;
    }

    // use ssl connection
    // Settings->Security Settings
    void GrSettings::SetUsingSSLConnection(bool enable) {
        sp_->Put(kStSSLConnection, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsSSLConnectionEnabled() {
        auto value = sp_->Get(kStSSLConnection);
        return value.empty() || value == kStTrue;
    }

    // record visit history
    // Settings->Security Settings
    void GrSettings::SetRecordingVisitHistory(bool enable) {
        sp_->Put(kStRecordVisitHistory, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsVisitHistoryEnabled() {
        auto value = sp_->Get(kStRecordVisitHistory);
        return value.empty() || value == kStTrue;
    }

    // record file transfer history
    // Settings->Security Settings
    void GrSettings::SetRecordingFileTransferHistory(bool enable) {
        sp_->Put(kStRecordFileTransferHistory, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsFileTransferHistoryEnabled() {
        auto value = sp_->Get(kStRecordFileTransferHistory);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetDisconnectAutoLockScreen(bool enable) {
        sp_->Put(kStDisconnectAutoLockScreen, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsDisconnectAutoLockScreenEnabled() {
        auto value = sp_->Get(kStDisconnectAutoLockScreen);
        return !value.empty() && value == kStTrue;
    }

    void GrSettings::SetRelayEnabled(bool enabled) {
        sp_->Put(kStRelayEnabled, enabled ? kStTrue : kStFalse);
    }

    bool GrSettings::IsRelayEnabled() {
        auto value = sp_->Get(kStRelayEnabled);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetDevelopModeEnabled(bool enable) {
        sp_->Put(kStDevelopMode, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsDevelopMode() {
        auto value = sp_->Get(kStDevelopMode);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetFileTransferEnabled(bool enable) {
        sp_->Put(kStFileTransferEnabled, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsFileTransferEnabled() {
        auto value = sp_->Get(kStFileTransferEnabled);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetColorfulTitleBar(bool enable) {
        sp_->Put(kStColorfulTitlebar, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsColorfulTitleBarEnabled() {
        auto value = sp_->Get(kStColorfulTitlebar);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetDisplayRandomPwd(bool enable) {
        sp_->Put(kStDisplayRandomPwd, enable ? kStTrue : kStFalse);
    }

    bool GrSettings::IsDisplayRandomPwd() {
        auto value = sp_->Get(kStDisplayRandomPwd);
        return value.empty() || value == kStTrue;
    }

    void GrSettings::SetPreferDecoder(const std::string& decoder) {
        sp_->Put(kStPreferDecoder, decoder);
    }

    std::string GrSettings::GetPreferDecoder() {
        return sp_->Get(kStPreferDecoder, "Auto");
    }

    void GrSettings::SetSpvrAccessInfo(const std::string& info) {
        sp_->Put(kStSpvrAccessInfo, info);
    }

    std::string GrSettings::GetSpvrAccessInfo() {
        return sp_->Get(kStSpvrAccessInfo, "");
    }

    void GrSettings::SetSkinName(const std::string& name) {
        sp_->Put(kStSkinName, name);
    }

    std::string GrSettings::GetSkinName() {
        return sp_->Get(kStSkinName, "");
    }

    std::string GrSettings::GetGrDataPath() {
        return gr_data_path_;
    }

    std::string GrSettings::GetGrDataCachePath() {
        return gr_data_path_ + "/cache";
    }

}
