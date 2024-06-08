//
// Created by RGAA on 2024-03-30.
//

#include "gr_server_manager.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"
#include "gr_settings.h"

#include <QCoreApplication>

namespace tc
{

    ServerManager::ServerManager(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
    }

    ServerManager::~ServerManager() {
        Exit();
    }

    Response<bool, uint32_t> ServerManager::StartServer() {
        auto resp = Response<bool, uint32_t>::Make(false, 0, "");

        QString current_path = QCoreApplication::applicationDirPath();
        QString work_dir = current_path;
        current_path = current_path.append("/GammaRayServer.exe");

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

        if (running_srv_ && running_srv_->process_) {
            running_srv_->process_->kill();
            running_srv_->process_ = nullptr;
        }

        auto process = std::make_shared<QProcess>();
        process->setWorkingDirectory(work_dir);
        process->start(current_path, arg_list);
        auto pid = process->processId();

        auto app_info = std::make_shared<RunningAppInfo>();
        app_info->pid_ = pid;
        app_info->process_ = process;
        running_srv_ = app_info;

        resp.ok_ = true;
        resp.value_ = pid;

        return resp;
    }

    Response<bool, bool> ServerManager::StopServer() {
        auto resp = Response<bool, bool>::Make(false, false, "");
        if (running_srv_ && running_srv_->process_) {
            running_srv_->process_->kill();
            running_srv_->process_ = nullptr;
        }
        resp.ok_ = true;
        resp.value_ = true;
        return resp;
    }

    Response<bool, uint32_t> ServerManager::ReStart() {
        auto resp = Response<bool, uint32_t>::Make(false, 0);
        return resp;
    }

    void ServerManager::Exit() {
        if (running_srv_ && running_srv_->process_) {
            running_srv_->process_->kill();
            running_srv_->process_->waitForFinished();
            running_srv_->process_ = nullptr;
        }
    }

}