//
// Created by RGAA on 2024/4/10.
//

#include "gr_settings.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/log.h"
#include "tc_common_new/base64.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/uuid.h"
#include "tc_common_new/win32/dxgi_mon_detector.h"
#include "tc_common_new/win32/audio_device_helper.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/http_client.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/md5.h"
#include "gr_app_messages.h"
#include "app_config.h"
#include <sstream>
#include <QApplication>
#include <qstandardpaths.h>

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
        encoder_format_ = sp_->Get(kStEncoderFormat, "h264");
        encoder_bitrate_ = sp_->Get(kStEncoderBitrate, "10");
        encoder_fps_ = sp_->Get(kStEncoderFPS, "60");
        encoder_resolution_type_ = sp_->Get(kStEncoderResolutionType, "origin");
        encoder_width_ = sp_->Get(kStEncoderWidth, "1280");
        encoder_height_ = sp_->Get(kStEncoderHeight, "720");

        capture_audio_ = sp_->Get(kStCaptureAudio, "true");
        capture_audio_type_ = sp_->Get(kStCaptureAudioType, "global");
        capture_video_ = sp_->Get(kStCaptureVideo, "true");
        capture_video_type_ = sp_->Get(kStCaptureVideoType, "global");
        capture_audio_device_ = sp_->Get(kStCaptureAudioDevice, "");

        panel_srv_port_ = sp_->GetInt(kStPanelListeningPort, 20369);
        http_server_port_ = panel_srv_port_;
        render_srv_port_ = sp_->GetInt(kStNetworkListenPort, 20371);
        network_listening_ip_ = sp_->Get(kStListeningIp, "");
        websocket_enabled_ = sp_->Get(kStWebSocketEnabled, kStTrue);
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

        // spvr server
        spvr_server_host_ = sp_->Get(kStSpvrServerHost, "");
        spvr_server_port_ = sp_->Get(kStSpvrServerPort, "");
        // signaling
        sig_server_address_ = sp_->Get(kStSigServerAddress, "");
        sig_server_port_ = sp_->Get(kStSigServerPort, "");
        // coturn
        coturn_server_address_ = sp_->Get(kStCoturnAddress, "");
        coturn_server_port_ = sp_->Get(kStCoturnPort, "");
        // id server
        profile_server_host_ = sp_->Get(kStProfileServerHost, "");
        profile_server_port_ = sp_->Get(kStProfileServerPort, "");
        // relay server
        relay_server_host_ = sp_->Get(kStRelayServerHost, "");
        relay_server_port_ = sp_->Get(kStRelayServerPort, "");

        device_id_ = sp_->Get(kStDeviceId, "");
        device_random_pwd_ = sp_->Get(kStDeviceRandomPwd, "");
        device_safety_pwd_md5_ = sp_->Get(kStDeviceSafetyPwd, "");
    }

    void GrSettings::Dump() {
        std::stringstream ss;
        ss << "---------------------GrSettings Begin---------------------" << std::endl;
        ss << "log_file_: " << log_file_ << std::endl;
        ss << "encoder_select_type_: " << encoder_select_type_ << std::endl;
        ss << "encoder_name_: " << encoder_name_ << std::endl;
        ss << "encoder_format_: " << encoder_format_ << std::endl;
        ss << "encoder_bitrate_: " << encoder_bitrate_ << std::endl;
        ss << "encoder_resolution_type_: " << encoder_resolution_type_ << std::endl;
        ss << "encoder_width_: " << encoder_width_ << std::endl;
        ss << "encoder_height_: " << encoder_height_ << std::endl;
        ss << "capture_audio_: " << capture_audio_ << std::endl;
        ss << "capture_audio_type_: " << capture_audio_type_ << std::endl;
        ss << "capture_video_: " << capture_video_ << std::endl;
        ss << "capture_video_type: " << capture_video_type_ << std::endl;
        ss << "http_server_port_: " << http_server_port_ << std::endl;
        ss << "panel_srv_port_: " << panel_srv_port_ << std::endl;
        ss << "render_srv_port_: " << render_srv_port_ << std::endl;
        ss << "websocket_enabled_:" << websocket_enabled_ << std::endl;
        ss << "webrtc_enabled_:" << webrtc_enabled_ << std::endl;
        ss << "capture_audio_device_: " << capture_audio_device_ << std::endl;
        ss << "sig_server_address_: " << sig_server_address_ << std::endl;
        ss << "sig_server_port_: " << sig_server_port_ << std::endl;
        ss << "coturn_server_address_: " << coturn_server_address_ << std::endl;
        ss << "coturn_server_port_: " << coturn_server_port_ << std::endl;
        ss << "device_id_: " << device_id_ << std::endl;
        ss << "device_random_pwd_: " << device_random_pwd_ << std::endl;
        ss << "device_safety_pwd_md5_: " << device_safety_pwd_md5_ << std::endl;
        ss << "udp_listen_port_:" << udp_listen_port_ << std::endl;
        ss << "relay host: " << relay_server_host_ << std::endl;
        ss << "relay port: " << relay_server_port_ << std::endl;
        ss << "spvr server host: " << spvr_server_host_ << std::endl;
        ss << "spvr server port: " << spvr_server_port_ << std::endl;
        ss << "pr server host: " << profile_server_host_ << std::endl;
        ss << "pr server port: " << profile_server_port_ << std::endl;
        ss << "---------------------GrSettings End-----------------------" << std::endl;
        LOGI("\n {}", ss.str());
    }

    std::vector<std::string> GrSettings::GetArgs() const {
        std::vector<std::string> args;
        args.push_back(std::format("--{}={}", kStLogFile, log_file_));
        args.push_back(std::format("--{}={}", kStEncoderSelectType, encoder_select_type_));
        args.push_back(std::format("--{}={}", kStEncoderName, encoder_name_));
        args.push_back(std::format("--{}={}", kStEncoderFormat, encoder_format_));
        args.push_back(std::format("--{}={}", kStEncoderBitrate, encoder_bitrate_));
        args.push_back(std::format("--{}={}", kStEncoderFPS, encoder_fps_));
        args.push_back(std::format("--{}={}", kStEncoderResolutionType, encoder_resolution_type_));
        args.push_back(std::format("--{}={}", kStEncoderWidth, encoder_width_));
        args.push_back(std::format("--{}={}", kStEncoderHeight, encoder_height_));
        args.push_back(std::format("--{}={}", kStCaptureAudio, capture_audio_));
        args.push_back(std::format("--{}={}", kStCaptureAudioType, capture_audio_type_));
        args.push_back(std::format("--{}={}", kStCaptureVideo, capture_video_));
        args.push_back(std::format("--{}={}", kStCaptureVideoType, capture_video_type_));
        args.push_back(std::format("--{}={}", kStWebSocketEnabled, websocket_enabled_));
        args.push_back(std::format("--{}={}", kStNetworkListenPort, render_srv_port_));
        args.push_back(std::format("--{}={}", kStWebRTCEnabled, webrtc_enabled_));
        args.push_back(std::format("--{}={}", kStUdpKcpEnabled, udp_kcp_enabled_));
        args.push_back(std::format("--{}={}", kStUdpListenPort, udp_listen_port_));
        args.push_back(std::format("--{}={}", kStCaptureAudioDevice, Base64::Base64Encode(capture_audio_device_)));
        args.push_back(std::format("--{}={}", kStAppGamePath, "desktop"));
        args.push_back(std::format("--{}={}", kStAppGameArgs, ""));
        args.push_back(std::format("--{}={}", kStDebugBlock, false));
        args.push_back(std::format("--{}={}", kStMockVideo, false));
        args.push_back(std::format("--{}={}", kStSigServerAddress, sig_server_address_));
        args.push_back(std::format("--{}={}", kStSigServerPort, sig_server_port_));
        args.push_back(std::format("--{}={}", kStCoturnAddress, coturn_server_address_));
        args.push_back(std::format("--{}={}", kStCoturnPort, coturn_server_port_));
        args.push_back(std::format("--{}={}", kStDeviceId, device_id_));
        args.push_back(std::format("--{}={}", kStDeviceRandomPwd, device_random_pwd_));
        args.push_back(std::format("--{}={}", kStDeviceSafetyPwd, device_safety_pwd_md5_));
        args.push_back(std::format("--panel_server_port={}", this->http_server_port_));
        args.push_back(std::format("--{}={}", kStRelayServerHost, relay_server_host_));
        args.push_back(std::format("--{}={}", kStRelayServerPort, relay_server_port_));
        return args;
    }

    void GrSettings::SetEnableResResize(bool enabled) {
        if (enabled) {
            encoder_resolution_type_ = kResTypeResize;
        } else {
            encoder_resolution_type_ = kResTypeOrigin;
        }
        sp_->Put(kStEncoderResolutionType, encoder_resolution_type_);
    }

    void GrSettings::SetBitrate(int br) {
        encoder_bitrate_ = std::to_string(br);
        sp_->Put(kStEncoderBitrate, encoder_bitrate_);
    }

    void GrSettings::SetFPS(int fps) {
        encoder_fps_ = std::to_string(fps);
        sp_->Put(kStEncoderFPS, encoder_fps_);
    }

    void GrSettings::SetResWidth(int width) {
        encoder_width_ = std::to_string(width);
        sp_->Put(kStEncoderWidth, encoder_width_);
    }

    void GrSettings::SetResHeight(int height) {
        encoder_height_ = std::to_string(height);
        sp_->Put(kStEncoderHeight, encoder_height_);
    }

    void GrSettings::SetEncoderFormat(int idx) {
        // h264
        if (idx == 0) {
            sp_->Put(kStEncoderFormat, "h264");
        } else if (idx == 1) {
            sp_->Put(kStEncoderFormat, "h265");
        }
    }

    void GrSettings::SetCaptureVideo(bool enabled) {
        capture_video_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStCaptureVideo, capture_video_);
    }

    void GrSettings::SetCaptureAudio(bool enabled) {
        capture_audio_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStCaptureAudio, capture_audio_);
    }

    void GrSettings::SetCaptureAudioDeviceId(const std::string& name) {
        capture_audio_device_ = name;
        sp_->Put(kStCaptureAudioDevice, capture_audio_device_);
    }

    bool GrSettings::IsEncoderResTypeOrigin() const {
        return encoder_resolution_type_ == kResTypeOrigin;
    }

    void GrSettings::SetFileTransferFolder(const std::string& path) {
        file_transfer_folder_ = path;
        sp_->Put(kStFileTransferFolder, path);
    }

    std::string GrSettings::GetCaptureMonitor() const {
        return sp_->Get(kStCaptureMonitor, "");
    }

    void GrSettings::SetListeningIp(const std::string& ip) {
        network_listening_ip_ = ip;
        sp_->Put(kStListeningIp, ip);
    }

    void GrSettings::SetWebSocketEnabled(bool enabled) {
        websocket_enabled_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStWebSocketEnabled, websocket_enabled_);
    }

    void GrSettings::SetWebRTCEnabled(bool enabled) {
        webrtc_enabled_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStWebRTCEnabled, webrtc_enabled_);
    }

    void GrSettings::SetUdpKcpEnabled(bool enabled) {
        udp_kcp_enabled_ = enabled ? kStTrue : kStFalse;
        sp_->Put(kStUdpKcpEnabled, udp_kcp_enabled_);
    }

    void GrSettings::SetSigServerAddress(const std::string& address) {
        sig_server_address_ = address;
        sp_->Put(kStSigServerAddress, address);
    }

    void GrSettings::SetSigServerPort(const std::string& port) {
        sig_server_port_ = port;
        sp_->Put(kStSigServerPort, port);
    }

    void GrSettings::SetCoturnServerAddress(const std::string& address) {
        coturn_server_address_ = address;
        sp_->Put(kStCoturnAddress, address);
    }

    void GrSettings::SetCoturnServerPort(const std::string& port) {
        coturn_server_port_ = port;
        sp_->Put(kStCoturnPort, port);
    }

    void GrSettings::SetDeviceId(const std::string& id) {
        device_id_ = id;
        sp_->Put(kStDeviceId, id);
    }

    void GrSettings::SetDeviceRandomPwd(const std::string& pwd) {
        device_random_pwd_ = pwd;
        sp_->Put(kStDeviceRandomPwd, pwd);
    }

    void GrSettings::SetDeviceSafetyPwd(const std::string& pwd) {
        device_safety_pwd_md5_ = pwd;
        sp_->Put(kStDeviceSafetyPwd, pwd);
    }

    void GrSettings::SetPanelListeningPort(int port) {
        panel_srv_port_ = port;
        sp_->Put(kStPanelListeningPort, std::to_string(port));
    }

    void GrSettings::SetProfileServerHost(const std::string& host) {
        profile_server_host_ = host;
        sp_->Put(kStProfileServerHost, host);
    }

    void GrSettings::SetProfileServerPort(const std::string& port) {
        profile_server_port_ = port;
        sp_->Put(kStProfileServerPort, port);
    }

    void GrSettings::SetRelayServerHost(const std::string& host) {
        relay_server_host_ = host;
        sp_->Put(kStRelayServerHost, host);
    }

    void GrSettings::SetRelayServerPort(const std::string& port) {
        relay_server_port_ = port;
        sp_->Put(kStRelayServerPort, port);
    }

    void GrSettings::SetSpvrServerHost(const std::string& host) {
        spvr_server_host_ = host;
        sp_->Put(kStSpvrServerHost, host);
    }

    void GrSettings::SetSpvrServerPort(const std::string& port) {
        spvr_server_port_ = port;
        sp_->Put(kStSpvrServerPort, port);
    }

    void GrSettings::SetScreenRecordingPath(const std::string& path) {
        screen_recording_path_ = path;
        sp_->Put(kStScreenRecordingPath, path);
    }

    std::string GrSettings::GetScreenRecordingPath() const {
        return sp_->Get(kStScreenRecordingPath, "");
    }
}
