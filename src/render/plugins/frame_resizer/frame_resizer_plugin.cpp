//
// Created RGAA on 15/11/2024.
//

#include "frame_resizer_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "render/plugins/plugin_ids.h"
#include "frame_render.h"
#include "tc_common_new/log.h"

static void* GetInstance() {
    static tc::FrameResizerPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{
    FrameResizerPlugin::FrameResizerPlugin() {
        processor_type_ = FrameProcessorType::kPrev;
    }

    std::string FrameResizerPlugin::GetPluginId() {
        return kFrameResizerPluginId;
    }

    std::string FrameResizerPlugin::GetPluginName() {
        return "Frame Resizer Plugin";
    }

    std::string FrameResizerPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t FrameResizerPlugin::GetVersionCode() {
        return 110;
    }

    std::string FrameResizerPlugin::GetPluginDescription() {
        return "Resize frame to target size before encoding";
    }

    void FrameResizerPlugin::On1Second() {

    }

    ComPtr<ID3D11Texture2D> FrameResizerPlugin::Process(ID3D11Texture2D* input, uint64_t adapter_uid, const std::string& monitor_name, int target_width, int target_height) {
        if (!input) {
            return nullptr;
        }
        D3D11_TEXTURE2D_DESC desc;
        input->GetDesc(&desc);
        if (!frame_renders_.contains(monitor_name) && d3d11_devices_[adapter_uid] && d3d11_devices_context_[adapter_uid]) {
            auto frame_render = std::make_shared<FrameRender>(d3d11_devices_[adapter_uid].Get(), d3d11_devices_context_[adapter_uid].Get());
            auto hr = frame_render->Prepare({target_width, target_height}, {(LONG)desc.Width, (LONG)desc.Height}, desc.Format);
            if (hr != S_OK) {
                LOGE("Prepare for frame render failed!");
                return nullptr;
            }
            frame_renders_[monitor_name] = frame_render;
        }

        auto frame_render = frame_renders_[monitor_name];

        auto resize_ctx = frame_render->GetD3D11DeviceContext();
        auto pre_texture = frame_render->GetSrcTexture();
        resize_ctx->CopyResource(pre_texture, input);

        //DebugOutDDS(pre_texture, "2.dds");

        frame_render->Draw();
        auto final_texture = frame_render->GetFinalTexture();
        return final_texture;
    }


}
