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
#include <sstream>
#include <QApplication>

namespace tc
{

    void GrSettings::Load() {
        sp_ = SharedPreference::Instance();
        version_ = "V 1.1.9";

        log_file_ = sp_->Get(kStLogFile, "true");
        encoder_select_type_ = sp_->Get(kStEncoderSelectType, "auto");
        encoder_name_ = sp_->Get(kStEncoderName, "nvenc");
        encoder_format_ = sp_->Get(kStEncoderFormat, "h264");
        encoder_bitrate_ = sp_->Get(kStEncoderBitrate, "20");
        encoder_resolution_type_ = sp_->Get(kStEncoderResolutionType, "origin");
        encoder_width_ = sp_->Get(kStEncoderWidth, "1280");
        encoder_height_ = sp_->Get(kStEncoderHeight, "720");

        capture_audio_ = sp_->Get(kStCaptureAudio, "true");
        capture_audio_type_ = sp_->Get(kStCaptureAudioType, "global");
        capture_video_ = sp_->Get(kStCaptureVideo, "true");
        capture_video_type_ = sp_->Get(kStCaptureVideoType, "global");
        capture_monitor_ = sp_->Get(kStCaptureMonitor, "");
        capture_audio_device_ = sp_->Get(kStCaptureAudioDevice, "");

        ws_server_port_ = sp_->GetInt(kStWsPort, 20369);
        http_server_port_ = ws_server_port_;
        network_listening_port_ = sp_->GetInt(kStNetworkListenPort, 20371);
        network_listening_ip_ = sp_->Get(kStListeningIp, "");
        websocket_enabled_ = sp_->Get(kStWebSocketEnabled, kStTrue);
        webrtc_enabled_ = sp_->Get(kStWebRTCEnabled, kStTrue);
        udp_listen_port_ = sp_->GetInt(kStUdpListenPort, 20381);

        file_transfer_folder_ = sp_->Get(kStFileTransferFolder, "");
        if (file_transfer_folder_.empty()) {
            file_transfer_folder_ = qApp->applicationDirPath().toStdString();
        }

        // default
        LOGI("Load from db, capture monitor is: {}", capture_monitor_);
        if (capture_monitor_.empty()) {
            auto adapters = DxgiMonitorDetector::Instance()->GetAdapters();
            for (const auto& adapter : adapters) {
                if (adapter.primary) {
                    LOGI("Use primary monitor: {}", adapter.display_name);
                    SetCaptureMonitor(adapter.display_name);
                }
            }
        }

        if (capture_audio_device_.empty()) {
            auto audio_devices = AudioDeviceHelper::DetectAudioDevices();
            for (const auto& dev : audio_devices) {
                if (dev.default_device_) {
                    SetCaptureAudioDeviceId(dev.id_);
                }
            }
        }

        sig_server_address_ = sp_->Get(kStSigServerAddress, "");
        sig_server_port_ = sp_->Get(kStSigServerPort, "");
        coturn_server_address_ = sp_->Get(kStCoturnAddress, "");
        coturn_server_port_ = sp_->Get(kStCoturnPort, "");

        client_id_ = sp_->Get(kStClientId, "");
        client_random_pwd_ = sp_->Get(kStClientRandomPwd, "");

        device_id_ = sp_->Get(kStDeviceId, "");
        if (device_id_.empty()) {
            auto hardware = Hardware::Instance();
            hardware->Detect(false, true, false);
            device_id_ = MD5::Hex(hardware->GetHardwareDescription());
            sp_->Put(kStDeviceId, device_id_);
        }
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
        ss << "ws_server_port_: " << ws_server_port_ << std::endl;
        ss << "network_listening_port_: " << network_listening_port_ << std::endl;
        ss << "websocket_enabled_:" << websocket_enabled_ << std::endl;
        ss << "webrtc_enabled_:" << webrtc_enabled_ << std::endl;
        ss << "capture_monitor_: " << GetCaptureMonitor() << std::endl;
        ss << "capture_audio_device_: " << capture_audio_device_ << std::endl;
        ss << "sig_server_address_: " << sig_server_address_ << std::endl;
        ss << "sig_server_port_: " << sig_server_port_ << std::endl;
        ss << "coturn_server_address_: " << coturn_server_address_ << std::endl;
        ss << "coturn_server_port_: " << coturn_server_port_ << std::endl;
        ss << "client_id_: " << client_id_ << std::endl;
        ss << "device_id_: " << device_id_ << std::endl;
        ss << "client_random_pwd_: " << client_random_pwd_ << std::endl;
        ss << "udp_listen_port_:" << udp_listen_port_ << std::endl;
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
        args.push_back(std::format("--{}={}", kStEncoderResolutionType, encoder_resolution_type_));
        args.push_back(std::format("--{}={}", kStEncoderWidth, encoder_width_));
        args.push_back(std::format("--{}={}", kStEncoderHeight, encoder_height_));
        args.push_back(std::format("--{}={}", kStCaptureAudio, capture_audio_));
        args.push_back(std::format("--{}={}", kStCaptureAudioType, capture_audio_type_));
        args.push_back(std::format("--{}={}", kStCaptureVideo, capture_video_));
        args.push_back(std::format("--{}={}", kStCaptureVideoType, capture_video_type_));
        args.push_back(std::format("--{}={}", kStWebSocketEnabled, websocket_enabled_));
        args.push_back(std::format("--{}={}", kStWebRTCEnabled, webrtc_enabled_));
        args.push_back(std::format("--{}={}", kStNetworkListenPort, network_listening_port_));
        args.push_back(std::format("--{}={}", kStUdpListenPort, udp_listen_port_));
        args.push_back(std::format("--{}={}", kStCaptureMonitor, Base64::Base64Encode(capture_monitor_)));
        args.push_back(std::format("--{}={}", kStCaptureAudioDevice, Base64::Base64Encode(capture_audio_device_)));
        args.push_back(std::format("--{}={}", kStAppGamePath, "desktop"));
        args.push_back(std::format("--{}={}", kStAppGameArgs, ""));
        args.push_back(std::format("--{}={}", kStDebugBlock, false));
        args.push_back(std::format("--{}={}", kStMockVideo, false));
        args.push_back(std::format("--{}={}", kStSigServerAddress, sig_server_address_));
        args.push_back(std::format("--{}={}", kStSigServerPort, sig_server_port_));
        args.push_back(std::format("--{}={}", kStCoturnAddress, coturn_server_address_));
        args.push_back(std::format("--{}={}", kStCoturnPort, coturn_server_port_));
        args.push_back(std::format("--{}={}", kStClientId, client_id_));
        args.push_back(std::format("--{}={}", kStClientRandomPwd, client_random_pwd_));
        args.push_back(std::format("--panel_server_port={}", this->http_server_port_));
        args.push_back(std::format("--{}={}", kStDeviceId, device_id_));
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

    void GrSettings::SetCaptureMonitor(const std::string& name) {
        capture_monitor_ = name;
        sp_->Put(kStCaptureMonitor, capture_monitor_);
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

    void GrSettings::SetClientId(const std::string& id) {
        client_id_ = id;
        sp_->Put(kStClientId, id);
    }

    void GrSettings::SetClientRandomPwd(const std::string& pwd) {
        client_random_pwd_ = pwd;
        sp_->Put(kStClientRandomPwd, pwd);
    }

}
