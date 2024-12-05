//
// Created by RGAA on 2024-03-30.
//

#include "gr_render_controller.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"
#include "gr_settings.h"
#include "tc_service_message.pb.h"
#include "gr_application.h"

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
        auto resp = Response<bool, uint32_t>::Make(false, 0, "");

        QString current_path = QCoreApplication::applicationDirPath();
        QString work_dir = current_path;
        current_path = current_path.append("/").append(kGammaRayServerName);

        auto args = GrSettings::Instance()->GetArgs();
        QStringList arg_list;
        for (auto& arg : args) {
            arg_list << arg.c_str();
        }

        std::stringstream ss;
        std::for_each(arg_list.begin(), arg_list.end(), [&](const auto &item) {
            ss << item.toStdString();
        });
        LOGI("param: {}", ss.str());

        //
        tc::ServiceMessage srv_msg;
        srv_msg.set_type(ServiceMessageType::kStartServer);
        auto sub = srv_msg.mutable_start_server();
        sub->set_work_dir(work_dir.toStdString());
        sub->set_app_path(current_path.toStdString());
        for (auto& arg : args) {
            sub->add_args(arg);
        }
        app_->PostMessage2Service(srv_msg.SerializeAsString());
        return true;
    }

    bool GrRenderController::StopServer() {
        tc::ServiceMessage srv_msg;
        srv_msg.set_type(ServiceMessageType::kStopServer);
        auto sub = srv_msg.mutable_stop_server();
        app_->PostMessage2Service(srv_msg.SerializeAsString());
        return true;
    }

    bool GrRenderController::ReStart() {
        tc::ServiceMessage srv_msg;
        srv_msg.set_type(ServiceMessageType::kRestartServer);
        auto sub = srv_msg.mutable_restart_server();
        app_->PostMessage2Service(srv_msg.SerializeAsString());
        return true;
    }

    void GrRenderController::Exit() {

    }

}