//
// Created by RGAA on 2024/4/10.
//

#include "gr_settings.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/log.h"
#include "tc_common_new/base64.h"
#include "util/dxgi_mon_detector.h"
#include "tc_common_new/win32/audio_device_helper.h"
#include <sstream>
#include <QApplication>

namespace tc
{

    void GrSettings::Load() {
        sp_ = SharedPreference::Instance();
        version_ = "V 1.1.9";
        udp_server_port_ = 21034;

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

        network_type_ = sp_->Get(kStNetworkType, "websocket");
        http_server_port_ = sp_->GetInt(kStHttpPort, 20368);
        ws_server_port_ = sp_->GetInt(kStWsPort, 20369);
        network_listen_port_ = sp_->GetInt(kStNetworkListenPort, 20371);

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
        ss << "network_type_: " << network_type_ << std::endl;
        ss << "http_server_port_: " << http_server_port_ << std::endl;
        ss << "ws_server_port_: " << ws_server_port_ << std::endl;
        ss << "network_listen_port_: " << network_listen_port_ << std::endl;
        ss << "capture_monitor_: " << GetCaptureMonitor() << std::endl;
        ss << "capture_audio_device_: " << capture_audio_device_ << std::endl;
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
        args.push_back(std::format("--{}={}", kStNetworkType, network_type_));
        args.push_back(std::format("--{}={}", kStNetworkListenPort, network_listen_port_));
        args.push_back(std::format("--{}={}", kStCaptureMonitor, Base64::Base64Encode(capture_monitor_)));
        args.push_back(std::format("--{}={}", kStCaptureAudioDevice, Base64::Base64Encode(capture_audio_device_)));
        args.push_back(std::format("--{}={}", kStAppGamePath, "desktop"));
        args.push_back(std::format("--{}={}", kStAppGameArgs, ""));
        args.push_back(std::format("--{}={}", kStDebugBlock, false));
        args.push_back(std::format("--{}={}", kStMockVideo, false));
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

}
