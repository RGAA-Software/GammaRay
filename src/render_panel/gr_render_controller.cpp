//
// Created by RGAA on 2024-03-30.
//

#include "gr_render_controller.h"
#include "gr_settings.h"
#include "gr_application.h"
#include "tc_service_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/base64.h"
#include "tc_common_new/string_util.h"
#include "translator/tc_translator.h"
#include <QCoreApplication>

namespace tc
{

    GrRenderController::GrRenderController(const std::shared_ptr<GrApplication>& app) {
        app_ = app;
        context_ = app_->GetContext();
    }

    GrRenderController::~GrRenderController() {
        Exit();
    }

    bool GrRenderController::StartServer() {
        auto args = this->GetArgs();
        LOGI("StartServer Params:");
        QStringList arg_list;
        for (auto& arg : args) {
            arg_list << arg.c_str();
            LOGI("{}", arg);
        }

        //
        tc::ServiceMessage srv_msg;
        srv_msg.set_type(ServiceMessageType::kSrvStartServer);
        auto sub = srv_msg.mutable_start_server();
        sub->set_work_dir(GetWorkDir().toStdString());
        sub->set_app_path(GetAppPath().toStdString());
        for (auto& arg : args) {
            sub->add_args(arg);
        }
        app_->PostMessage2Service(srv_msg.SerializeAsString());
        return true;
    }

    bool GrRenderController::StopServer() {
        tc::ServiceMessage srv_msg;
        srv_msg.set_type(ServiceMessageType::kSrvStopServer);
        auto sub = srv_msg.mutable_stop_server();
        app_->PostMessage2Service(srv_msg.SerializeAsString());
        return true;
    }

    bool GrRenderController::ReStart() {
        tc::ServiceMessage srv_msg;
        srv_msg.set_type(ServiceMessageType::kSrvRestartServer);
        auto sub = srv_msg.mutable_restart_server();
        sub->set_work_dir(GetWorkDir().toStdString());
        sub->set_app_path(GetAppPath().toStdString());
        auto args = this->GetArgs();
        LOGI("Restart args:");
        for (auto& arg : args) {
            sub->add_args(arg);
            LOGI("{}", arg);
        }
        app_->PostMessage2Service(srv_msg.SerializeAsString());
        return true;
    }

    void GrRenderController::Exit() {

    }

    QString GrRenderController::GetWorkDir() {
        return QCoreApplication::applicationDirPath();
    }

    QString GrRenderController::GetAppPath() {
        QString current_path = QCoreApplication::applicationDirPath();
        current_path = current_path.append("/").append(kGammaRayRenderName);
        return current_path;
    }

    std::vector<std::string> GrRenderController::GetArgs() {
        auto settings = GrSettings::Instance();
        std::vector<std::string> args;
        args.push_back(std::format("--app_mode={}", "desktop"));
        args.push_back(std::format("--{}={}", kStEncoderSelectType, settings->encoder_select_type_));
        args.push_back(std::format("--{}={}", kStEncoderName, settings->encoder_name_));
        args.push_back(std::format("--{}={}", kStEncoderFormat, settings->GetEncoderFormat()));
        args.push_back(std::format("--{}={}", kStEncoderBitrate, settings->GetBitrate()));
        args.push_back(std::format("--{}={}", kStEncoderFPS, settings->GetFPS()));
        args.push_back(std::format("--encoder_resolution_type={}", (settings->IsResResizeEnabled() ? kResTypeResize : kResTypeOrigin)));
        args.push_back(std::format("--{}={}", kStEncoderWidth, settings->GetResWidth()));
        args.push_back(std::format("--{}={}", kStEncoderHeight, settings->GetResHeight()));
        args.push_back(std::format("--{}={}", kStCaptureAudio, settings->IsCaptureAudioEnabled()));
        args.push_back(std::format("--{}={}", kStCaptureAudioType, settings->capture_audio_type_));
        args.push_back(std::format("--{}={}", kStCaptureVideo, settings->capture_video_));
        args.push_back(std::format("--{}={}", kStCaptureVideoType, settings->capture_video_type_));
        args.push_back(std::format("--{}={}", kStWebSocketEnabled, settings->IsWebSocketEnabled()));
        args.push_back(std::format("--{}={}", kStNetworkListenPort, settings->GetRenderServerPort()));
        args.push_back(std::format("--{}={}", kStWebRTCEnabled, settings->webrtc_enabled_));
        args.push_back(std::format("--{}={}", kStUdpKcpEnabled, settings->udp_kcp_enabled_));
        args.push_back(std::format("--{}={}", kStUdpListenPort, settings->udp_listen_port_));
        args.push_back(std::format("--{}={}", kStCaptureAudioDevice, Base64::Base64Encode(settings->capture_audio_device_)));
        args.push_back(std::format("--{}={}", kStAppGamePath, ""));
        args.push_back(std::format("--{}={}", kStAppGameArgs, ""));
        args.push_back(std::format("--{}={}", kStDebugBlock, false));
        args.push_back(std::format("--{}={}", kStMockVideo, false));
        args.push_back(std::format("--{}={}", kStDeviceId, settings->GetDeviceId()));
        args.push_back(std::format("--{}={}", kStDeviceRandomPwd, settings->GetDeviceRandomPwd()));
        args.push_back(std::format("--{}={}", kStDeviceSafetyPwd, settings->GetDeviceSecurityPwd()));
        args.push_back(std::format("--panel_server_port={}", settings->GetPanelServerPort()));
        args.push_back(std::format("--{}={}", kStRelayServerHost, settings->GetRelayServerHost()));
        args.push_back(std::format("--{}={}", kStRelayServerPort, settings->GetRelayServerPort()));
        args.push_back(std::format("--{}={}", kStCanBeOperated, settings->IsBeingOperatedEnabled()));
        args.push_back(std::format("--{}={}", kStRelayEnabled, settings->IsRelayEnabled()));
        args.push_back(std::format("--language={}", (int)tcTrMgr()->GetSelectedLanguage()));
        args.push_back(std::format("--{}={}", kStLogFile, settings->log_file_));
        args.push_back(std::format("--appkey={}", grApp->GetAppkey()));
        return args;
    }

}