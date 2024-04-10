//
// Created by hy on 2024/4/10.
//

#include "settings.h"

namespace tc
{

    void Settings::Load() {
        http_server_port_ = 20368;
        ws_server_port_ = 20369;
        udp_server_port_ = 21034;
    }

}
