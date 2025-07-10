//
// Created by RGAA on 10/07/2025.
//

#include "stream_messages.h"
#include "json/json.hpp"

using namespace nlohmann;

namespace tc
{

    std::string GrSmRestartRender::AsJson() {
        json obj;
        return obj.dump();
    }

    std::string GrSmLockScreen::AsJson() {
        json obj;
        return obj.dump();
    }

    std::string GrSmRestartDevice::AsJson() {
        json obj;
        return obj.dump();
    }

    std::string GrSmShutdownDevice::AsJson() {
        json obj;
        return obj.dump();
    }
}