//
// Created RGAA on 15/11/2024.
//

#include "frame_carrier_plugin.h"
#include "plugin_interface/gr_plugin_events.h"1
#include "tc_common_new/file.h"
#include "render/plugins/plugin_ids.h"

void* GetInstance() {
    static tc::FrameCarrierPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string FrameCarrierPlugin::GetPluginId() {
        return kFrameCarrierPluginId;
    }

    std::string FrameCarrierPlugin::GetPluginName() {
        return "Frame Carrier";
    }

    std::string FrameCarrierPlugin::GetVersionName() {
        return plugin_version_name_;
    }

    uint32_t FrameCarrierPlugin::GetVersionCode() {
        return plugin_version_code_;
    }

    std::string FrameCarrierPlugin::GetPluginDescription() {
        return "Frame carrier";
    }

    void FrameCarrierPlugin::On1Second() {

    }

    bool FrameCarrierPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrPluginInterface::OnCreate(param);
        return true;
    }

    bool FrameCarrierPlugin::OnDestroy() {
        return true;
    }

    bool FrameCarrierPlugin::InitFrameCarrier(const tc::GrCarrierParams &params) {
        GrFrameCarrierPlugin::InitFrameCarrier(params);
        return false;
    }

    std::shared_ptr<GrCarriedFrame> FrameCarrierPlugin::CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index) {
        return nullptr;
    }

}
