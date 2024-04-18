//
// Created by hy on 2024/4/10.
//

#include "gr_settings.h"

namespace tc
{

    void GrSettings::Load() {
        http_server_port_ = 20368;
        ws_server_port_ = 20369;
        stream_server_port_ = 20371;
        udp_server_port_ = 21034;
    }

}
