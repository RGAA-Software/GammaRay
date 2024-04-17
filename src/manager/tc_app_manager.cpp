//
// Created by RGAA on 2024-03-30.
//

#include "tc_app_manager.h"
#include "tc_app_info.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"

#include <QCoreApplication>

namespace tc
{

    AppManager::AppManager(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
    }

    Response<bool, uint32_t> AppManager::Start(const std::string& args) {
        auto resp = Response<bool, uint32_t>::Make(false, 0, "");

        QString current_path = QCoreApplication::applicationDirPath();
        QString work_dir = current_path + "/publish";
        current_path = current_path.append("/publish/tc_application.exe");

        auto process = std::make_shared<QProcess>();
        process->setWorkingDirectory(work_dir);
        process->start(current_path);
        auto pid = process->processId();

        auto app_info = std::make_shared<RunningAppInfo>();
        app_info->pid_ = pid;
        app_info->process_ = process;
        running_apps_.Insert(pid, app_info);

        resp.ok_ = true;
        resp.value_ = pid;

        return resp;
    }

    Response<bool, bool> AppManager::Stop(uint32_t pid) {
        auto resp = Response<bool, bool>::Make(false, false, "");
        return resp;
    }

}