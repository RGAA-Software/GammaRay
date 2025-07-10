//
// Created by RGAA on 10/07/2025.
//

#include "stream_messages.h"
#include "json/json.hpp"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"

using namespace nlohmann;

namespace tc
{

    std::string GrSmRestartRender::AsJson() {
        json obj;
        obj["type"] = "restart_render";
        obj["from_device"] = grApp->GetContext()->GetDeviceIdOrIpAddress();
        return obj.dump();
    }

    std::string GrSmLockScreen::AsJson() {
        json obj;
        obj["type"] = "lock_screen";
        obj["from_device"] = grApp->GetContext()->GetDeviceIdOrIpAddress();
        return obj.dump();
    }

    std::string GrSmRestartDevice::AsJson() {
        json obj;
        obj["type"] = "restart_device";
        obj["from_device"] = grApp->GetContext()->GetDeviceIdOrIpAddress();
        return obj.dump();
    }

    std::string GrSmShutdownDevice::AsJson() {
        json obj;
        obj["type"] = "shutdown_device";
        obj["from_device"] = grApp->GetContext()->GetDeviceIdOrIpAddress();
        return obj.dump();
    }
}