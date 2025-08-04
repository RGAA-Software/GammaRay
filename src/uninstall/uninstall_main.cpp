//
// Created by RGAA on 22/01/2025.
//

#include <QApplication>
#include <QProcess>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <thread>
#include <chrono>
#include <windows.h>
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/log.h"
#include "service/service_manager.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "gflags/gflags.h"

using namespace tc;

static const std::string kGammaRayName = "GammaRay.exe";
static const std::string kGammaRayGuardName = "GammaRayGuard.exe";
static const std::string kGammaRayRenderName = "GammaRayRender.exe";
static const std::string kGammaRayClientInner = "GammaRayClientInner.exe";
static const std::string kGammaRayService = "GammaRayService.exe";

DEFINE_string(option, "null", "uninstall/stop");

int main(int argc, char** argv) {

    QApplication app(argc, argv);

    gflags::ParseCommandLineFlags(&argc, &argv, true);

    auto service_manager = ServiceManager::Make();
    service_manager->Init("GammaRayService", "", "GammaRat Service", "** GammaRay Service **");
    
    if (FLAGS_option == "uninstall") {
        service_manager->Remove(true);    
        std::thread task_thread =  std::thread([=]() {
           
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            auto processes = tc::ProcessHelper::GetProcessList(false);
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayGuardName) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    tc::ProcessHelper::CloseProcess(process->pid_);
                    break;
                }
            }
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayClientInner) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    tc::ProcessHelper::CloseProcess(process->pid_);
                    break;
                }
            }
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayRenderName) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    tc::ProcessHelper::CloseProcess(process->pid_);
                    break;
                }
            }
            for (auto& process : processes) {
                if (process->exe_full_path_.find(kGammaRayName) != std::string::npos) {
                    LOGI("Kill exe: {}", process->exe_full_path_);
                    tc::ProcessHelper::CloseProcess(process->pid_);
                }
            }
        });

        if (task_thread.joinable()) {
            task_thread.join();
        }

        QString path = QCoreApplication::applicationDirPath();
        path += "/shadow_deleter.exe";
        auto process = new QProcess();
        QStringList args;
        args.append(path);
        process->startDetached(path, args);
        qApp->exit(0);
    }
    else if (FLAGS_option == "stop") {
        service_manager->Remove(false);
    }
    else if (FLAGS_option == "null") {
        return 0;
    }
    else {
        return 0;
    }
    return 0;
}
