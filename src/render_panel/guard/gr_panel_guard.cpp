//
// Created by RGAA on 28/07/2025.
//

#include "gr_panel_guard.h"
#include <QApplication>
#include "tc_common_new/thread.h"
#include "gr_guard_context.h"
#include "gr_guard_messages.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/log.h"

namespace tc
{

    const auto kGammaRayName = "GammaRay.exe";
    const auto kGammaRaySysInfoName = "gr_sysinfo.exe";

    GrPanelGuard::GrPanelGuard(const std::shared_ptr<GrGuardContext>& ctx) {
        context_ = ctx;
    }

    void GrPanelGuard::Start() {
        thread_ = Thread::Make("panel_guard", 1024);
        thread_->Poll();

        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgGrGuardTimer5S>([=, this](const MsgGrGuardTimer5S& msg) {
            thread_->Post([this]() {
                auto pss = ProcessHelper::GetProcessList(false);
                if (!CheckPanelState(pss)) {
                    StartPanel();
                }

                if (!CheckSysInfoState(pss)) {
                    LOGI("**Start gr_sysinfo.exe.");
                    StartSysInfo();
                }
            });
        });

        thread_->Post([this]() {
            auto pss = ProcessHelper::GetProcessList(false);
            if (!CheckSysInfoState(pss)) {
                LOGI("--Start gr_sysinfo.exe.");
                StartSysInfo();
            }
        });
    }

    void GrPanelGuard::Exit() {
        if (thread_) {
            thread_->Exit();
        }
    }

    bool GrPanelGuard::CheckPanelState(const std::vector<std::shared_ptr<ProcessInfo>>& pss) {
        for (const auto& pi : pss) {
            if (pi->exe_full_path_.find(kGammaRayName) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    void GrPanelGuard::StartPanel() {
        auto cmdline = QCoreApplication::applicationDirPath() + "/" + kGammaRayName;
        ProcessUtil::StartProcess(cmdline.toStdString(), {}, true, false);
        //ProcessUtil::StartProcessInWorkDir(QCoreApplication::applicationDirPath().toStdString(), cmdline.toStdString(), {});
    }

    bool GrPanelGuard::CheckSysInfoState(const std::vector<std::shared_ptr<ProcessInfo>>& pss) {
        for (const auto& pi : pss) {
            if (pi->exe_full_path_.find(kGammaRaySysInfoName) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    void GrPanelGuard::StartSysInfo() {
        auto cmdline = QCoreApplication::applicationDirPath() + "/" + kGammaRaySysInfoName;
        ProcessUtil::StartProcess(cmdline.toStdString(), {}, true, false);
    }

}