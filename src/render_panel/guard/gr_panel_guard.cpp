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

namespace tc
{

    const auto kGammaRayName = "GammaRay.exe";

    GrPanelGuard::GrPanelGuard(const std::shared_ptr<GrGuardContext>& ctx) {
        context_ = ctx;
    }

    void GrPanelGuard::Start() {
        thread_ = Thread::Make("panel_guard", 1024);
        thread_->Poll();

        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgGrGuardTimer5S>([=, this](const MsgGrGuardTimer5S& msg) {
            thread_->Post([this]() {
                if (!CheckPanelState()) {
                    StartPanel();
                }
            });
        });
    }

    void GrPanelGuard::Exit() {

    }

    bool GrPanelGuard::CheckPanelState() {
        auto processes = ProcessHelper::GetProcessList(false);
        for (const auto& pi : processes) {
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

}