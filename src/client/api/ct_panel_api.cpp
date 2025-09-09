//
// Created by RGAA on 9/09/2025.
//

#include "ct_panel_api.h"

namespace tc
{

    Result<bool, HttpRespCode> CtPanelApi::StopRender(const std::string& host, int port) {
        auto client = HttpClient::Make(host, port, kPathStopServer, 3000);
        auto resp = client->Request({});
        if (resp.status != 200 || resp.body.empty()) {
            LOGE("Request new device failed : {}", resp.status);
            return TRError(resp.status);
        }

        try {
            //LOGI("==> RelayDevice body: {}", resp.body);
            auto obj = json::parse(resp.body);
            auto code = obj["code"].get<int>();
            if (code == 200) {
                return true;
            }
            else {
                return TcErr(code);
            }
        } catch(std::exception& e) {
            LOGE("GetRelayDeviceInfo Exception: {}", e.what());
            return TcErr(-1);
        }
    }

}