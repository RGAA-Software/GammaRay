//
// Created by RGAA on 28/07/2025.
//

#include "gr_guard_starter.h"
#include <QApplication>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <shellapi.h>
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/auto_start.h"

namespace tc
{

    static const std::string kGammaRayGuardName = "GammaRayGuard.exe";

    GrGuardStarter::GrGuardStarter(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;

        auto fn_start = [=, this]() {
            context_->PostTask([=, this]() {
                if (!this->CheckGuardState()) {
                    this->StartGuard();
                }
            });
        };

        msg_listener_ = ctx->ObtainMessageListener();
        msg_listener_->Listen<MsgGrTimer5S>([=, this](const MsgGrTimer5S& msg) {
            fn_start();
        });

        fn_start();
    }

    bool GrGuardStarter::CheckGuardState() {
        auto processes = ProcessHelper::GetProcessList(false);
        for (const auto& pi : processes) {
            if (pi->exe_full_path_.find(kGammaRayGuardName) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    void GrGuardStarter::StartGuard() {
        auto cmdline = context_->GetCurrentExeFolder() + "/" + kGammaRayGuardName;
        ProcessUtil::RunAsAdminWithShell(QString::fromStdString(cmdline).toStdWString().c_str());
    }

}