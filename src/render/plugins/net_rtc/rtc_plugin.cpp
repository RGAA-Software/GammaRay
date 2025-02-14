//
// Created RGAA on 15/11/2024.
//

#include "rtc_plugin.h"
#include "render/plugins/plugin_ids.h"
#include "video_source_mock.h"

namespace tc
{

    std::string RtcPlugin::GetPluginId() {
        return kNetRtcPluginId;
    }

    std::string RtcPlugin::GetPluginName() {
        return "RTC Plugin";
    }

    std::string RtcPlugin::GetVersionName() {
        return "1.0.2";
    }

    uint32_t RtcPlugin::GetVersionCode() {
        // test //
        auto mock = std::make_shared<VideoSourceMock>();
        // test //
        return 102;
    }

    void RtcPlugin::OnProtoMessage(const std::string& msg) {

    }

}
