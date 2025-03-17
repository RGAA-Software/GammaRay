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
#include "tc_3rdparty/json/json.hpp"
#include "tc_common_new/message_notifier.h"
#include "gr_app_messages.h"
#include <sstream>
#include <QApplication>

using namespace nlohmann;

namespace tc
{

    void GrSettings::Init(const std::shared_ptr<MessageNotifier>& notifier) {
        notifier_ = notifier;
    }

    void GrSettings::Load() {
        sp_ = SharedPreference::Instance();
        version_ = "V 1.1.9";

        log_file_ = sp_->Get(kStLogFile, "true");
        encoder_select_type_ = sp_->Get(kStEncoderSelectType, "auto");
        encoder_name_ = sp_->Get(kStEncoderName, "nvenc");
        encoder_format_ = sp_->Get(kStEncoderFormat, "h264");
        encoder_bitrate_ = sp_->Get(kStEncoderBitrate, "5");
        encoder_resolution_type_ = sp_->Get(kStEncoderResolutionType, "origin");
        encoder_width_ = sp_->Get(kStEncoderWidth, "1280");
        encoder_height_ = sp_->Get(kStEncoderHeight, "720");

        capture_audio_ = sp_->Get(kStCaptureAudio, "true");
        capture_audio_type_ = sp_->Get(kStCaptureAudioType, "global");
        capture_video_ = sp_->Get(kStCaptureVideo, "true");
        capture_video_type_ = sp_->Get(kStCaptureVideoType, "global");
        capture_audio_device_ = sp_->Get(kStCaptureAudioDevice, "");

        panel_listen_port_ = sp_->GetInt(kStPanelListeningPort, 20369);
        http_server_port_ = panel_listen_port_;
        network_listening_port_ = sp_->GetInt(kStNetworkListenPort, 20371);
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
        profile_server_host_ = sp_->Get(kStIDServerHost, "");
        profile_server_port_ = sp_->Get(kStIDServerPort, "");
        // relay server
        relay_server_host_ = sp_->Get(kStRelayServerHost, "");
        relay_server_port_ = sp_->Get(kStRelayServerPort, "");

        device_id_ = sp_->Get(kStDeviceId, "");
        device_random_pwd_ = sp_->Get(kStDeviceRandomPwd, "");
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
        ss << "panel_listen_port_: " << panel_listen_port_ << std::endl;
        ss << "network_listening_port_: " << network_listening_port_ << std::endl;
        ss << "websocket_enabled_:" << websocket_enabled_ << std::endl;
        ss << "webrtc_enabled_:" << webrtc_enabled_ << std::endl;
        ss << "capture_audio_device_: " << capture_audio_device_ << std::endl;
        ss << "sig_server_address_: " << sig_server_address_ << std::endl;
        ss << "sig_server_port_: " << sig_server_port_ << std::endl;
        ss << "coturn_server_address_: " << coturn_server_address_ << std::endl;
        ss << "coturn_server_port_: " << coturn_server_port_ << std::endl;
        ss << "device_id_: " << device_id_ << std::endl;
        ss << "device_random_pwd_: " << device_random_pwd_ << std::endl;
        ss << "udp_listen_port_:" << udp_listen_port_ << std::endl;
        ss << "relay host: " << relay_server_host_ << std::endl;
        ss << "relay port: " << relay_server_port_ << std::endl;
        ss << "spvr server host: " << spvr_server_host_ << std::endl;
        ss << "spvr server port: " << spvr_server_port_ << std::endl;
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
        args.push_back(std::format("--{}={}", kStNetworkListenPort, network_listening_port_));
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

    void GrSettings::SetPanelListeningPort(int port) {
        panel_listen_port_ = port;
        sp_->Put(kStPanelListeningPort, std::to_string(port));
    }

    void GrSettings::SetIdServerHost(const std::string& host) {
        profile_server_host_ = host;
        sp_->Put(kStIDServerHost, host);
    }

    void GrSettings::SetIdServerPort(const std::string& port) {
        profile_server_port_ = port;
        sp_->Put(kStIDServerPort, port);
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

    bool GrSettings::VerifyOnlineServers() {
        if (spvr_server_host_.empty() || spvr_server_port_.empty()
            || relay_server_host_.empty() || relay_server_port_.empty()
            || profile_server_host_.empty() || profile_server_port_.empty()) {
            return false;
        }
        // check spvr
        bool ok = CanPingServer(spvr_server_host_, spvr_server_port_);
        if (!ok) {
            LOGE("Spvr is not online: {} {} ", spvr_server_host_, spvr_server_port_);
            return false;
        }

        // check relay
        ok = CanPingServer(relay_server_host_, relay_server_port_);
        if (!ok) {
            LOGE("Relay is not online: {} {} ", relay_server_host_, relay_server_port_);
            return false;
        }

        // check profile
        ok = CanPingServer(profile_server_host_, profile_server_port_);
        if (!ok) {
            LOGE("Profile is not online: {} {} ", profile_server_host_, profile_server_port_);
            return false;
        }

        return true;
    }

    bool GrSettings::RequestOnlineServers() {
        auto client =
                HttpClient::Make(std::format("{}:{}", spvr_server_host_, spvr_server_port_), "/get/online/servers", 3);
        auto resp = client->Request();
        if (resp.status != 200 || resp.body.empty()) {
            LOGE("Request new device failed.");
            return false;
        }

        try {
            auto obj = json::parse(resp.body);
            if (obj["code"].get<int>() != 200) {
                return false;
            }

            auto data = obj["data"];
            if (!data.is_array()) {
                return false;
            }

            bool settings_changed = false;
            for (const auto& item : data) {
                auto srv_type = item["server_type"].get<std::string>();
                auto srv_name = item["server_name"].get<std::string>();
                auto srv_id = item["server_id"].get<std::string>();
                auto srv_w3c_ip = item["w3c_ip"].get<std::string>();
                auto srv_local_ip = item["local_ip"].get<std::string>();
                auto srv_working_port = item["working_port"].get<std::string>();
                auto srv_grpc_port = item["grpc_port"].get<std::string>();
                if (srv_type == "0") {
                    // relay server
                    bool ok = this->CanPingServer(srv_w3c_ip, srv_working_port);
                    LOGI("Ping relay server result: {}", ok);
                    // save to db
                    if (ok) {
                        this->SetRelayServerHost(srv_w3c_ip);
                        this->SetRelayServerPort(srv_working_port);
                        settings_changed = true;
                    }
                }
                else if (srv_type == "1") {
                    // profile server ; check it
                    bool ok = this->CanPingServer(srv_w3c_ip, srv_working_port);
                    LOGI("Ping profile server result: {}", ok);
                    // save to db
                    if (ok) {
                        this->SetIdServerHost(srv_w3c_ip);
                        this->SetIdServerPort(srv_working_port);
                        settings_changed = true;
                    }
                }

                LOGI("--online server : {}, type: {}", srv_name, srv_type);
                LOGI("----srv w3c ip: {}", srv_w3c_ip);
                LOGI("----srv local ip: {}", srv_local_ip);
                LOGI("----srv id: {}", srv_id);
                LOGI("----srv working port: {}", srv_working_port);
                LOGI("----srv grpc port: {}", srv_grpc_port);
            }

            if (settings_changed) {
                notifier_->SendAppMessage(MsgSettingsChanged {});
            }

            return true;
        } catch(std::exception& e) {
            LOGE("RequestNewDevice failed: {}, message: {}", e.what(), resp.body);
            return false;
        }
    }

    bool GrSettings::CanPingServer(const std::string& host, const std::string& port) {
        auto client =
                HttpClient::Make(std::format("{}:{}", spvr_server_host_, spvr_server_port_), "/ping", 2);
        auto resp = client->Request();
        if (resp.status != 200 || resp.body.empty()) {
            LOGE("Request new device failed.");
            return false;
        }

        try {
            auto obj = json::parse(resp.body);
            if (obj["code"].get<int>() != 200) {
                return false;
            }

//            auto data = obj["data"].get<std::string>();
//            return data == "Pong";
// !! payload info !!
            return true;
        } catch (...) {
            return false;
        }
    }

    ///verify/device/info
    DeviceVerifyResult GrSettings::VerifyDeviceInfo() {
        if (device_id_.empty() || device_random_pwd_.empty()) {
            return DeviceVerifyResult::kVfEmptyDeviceId;
        }
        if (profile_server_host_.empty() || profile_server_port_.empty()) {
            return DeviceVerifyResult::kVfEmptyServerHost;
        }
        auto client =
                HttpClient::Make(std::format("{}:{}", profile_server_port_, profile_server_port_), "/verify/device/info", 2);
        auto resp = client->Request();
        if (resp.status != 200 || resp.body.empty()) {
            LOGE("Request new device failed.");
            return DeviceVerifyResult::kVfNetworkFailed;
        }

        try {
            auto obj = json::parse(resp.body);
            if (obj["code"].get<int>() != 200) {
                return DeviceVerifyResult::kVfResponseFailed;
            }

            return DeviceVerifyResult::kVfSuccess;
        } catch(...) {
            return DeviceVerifyResult::kVfParseJsonFailed;
        }
    }
}
