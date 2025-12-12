//
// Created by RGAA on 12/12/2025.
//

#ifndef GAMMARAYPREMIUM_GR_SPVR_MANAGER_H
#define GAMMARAYPREMIUM_GR_SPVR_MANAGER_H

#include <memory>
#include <optional>
#include "tc_spvr_client/spvr_api.h"

namespace tc
{

    class GrContext;
    class GrSettings;

    class GrSpvrManager {
    public:
        explicit GrSpvrManager(const std::shared_ptr<GrContext>& context);
        std::optional<spvr::AliveConnections> QueryAliveConnections(bool show_err_dialog) const;
        std::optional<spvr::AvailableNewConnection> QueryNewConnection(bool show_err_dialog) const;

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_GR_SPVR_MANAGER_H