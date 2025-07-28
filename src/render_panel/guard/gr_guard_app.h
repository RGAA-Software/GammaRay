//
// Created by RGAA on 28/07/2025.
//

#ifndef GAMMARAYPREMIUM_GR_GUARD_APP_H
#define GAMMARAYPREMIUM_GR_GUARD_APP_H

#include <memory>

namespace tc
{

    class GrGuardContext;
    class GrPanelGuard;

    class GrGuardApp {
    public:
        explicit GrGuardApp(const std::shared_ptr<GrGuardContext>& ctx);

    private:
        std::shared_ptr<GrGuardContext> context_ = nullptr;
        std::shared_ptr<GrPanelGuard> panel_guard_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_GR_GUARD_APP_H
