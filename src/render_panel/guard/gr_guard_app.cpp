//
// Created by RGAA on 28/07/2025.
//

#include "gr_guard_app.h"
#include "gr_guard_context.h"
#include "gr_panel_guard.h"

namespace tc
{

    GrGuardApp::GrGuardApp(const std::shared_ptr<GrGuardContext>& ctx) {
        context_ = ctx;
        panel_guard_ = std::make_shared<GrPanelGuard>(context_);
        panel_guard_->Start();
    }

}