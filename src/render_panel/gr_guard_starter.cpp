//
// Created by RGAA on 28/07/2025.
//

#include "gr_guard_starter.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/win32/process_helper.h"
#include "tc_common_new/process_util.h"

namespace tc
{

    static const std::string kGammaRayGuardName = "GammaRayGuard.exe";

    GrGuardStarter::GrGuardStarter(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;

        msg_listener_ = ctx->ObtainMessageListener();
        msg_listener_->Listen<MsgGrTimer5S>([=, this](const MsgGrTimer5S& msg) {
            context_->PostTask([=, this]() {
                if (!this->CheckGuardState()) {
                    this->StartGuard();
                }
            });
        });
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
        auto exe_path = context_->GetCurrentExeFolder() + "/" + kGammaRayGuardName;
        ProcessUtil::StartProcess(exe_path, {}, true, false);
    }

}