//
// Created by RGAA on 9/09/2025.
//

#ifndef GAMMARAYPREMIUM_CT_PANEL_API_H
#define GAMMARAYPREMIUM_CT_PANEL_API_H

#include "tc_common_new/log.h"
#include "tc_common_new/http_client.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_3rdparty/expt/expected.h"

#include "render_panel/network/apis.h"

using namespace nlohmann;

namespace tc
{

    using HttpRespCode = int;

    class CtPanelApi {
    public:
        static Result<bool, HttpRespCode> StopRender(const std::string& host, int port);
    };

}

#endif //GAMMARAYPREMIUM_CT_PANEL_API_H
