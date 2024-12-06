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

    void FrameResizerPlugin::On1Second() {

    }

    ComPtr<ID3D11Texture2D> FrameResizerPlugin::Process(ID3D11Texture2D* input, int target_width, int target_height) {
        if (!input) {
            return nullptr;
        }
        D3D11_TEXTURE2D_DESC desc;
        input->GetDesc(&desc);
        if (!frame_render_ && d3d11_device_ && d3d11_device_context_) {
            frame_render_ = std::make_shared<FrameRender>(d3d11_device_.Get(), d3d11_device_context_.Get());
            auto hr = frame_render_->Prepare({target_width, target_height}, {(LONG)desc.Width, (LONG)desc.Height}, desc.Format);
            if (hr != S_OK) {
                LOGE("Prepare for frame render failed!");
                return nullptr;
            }
        }

        auto resize_ctx = frame_render_->GetD3D11DeviceContext();
        auto pre_texture = frame_render_->GetSrcTexture();
        resize_ctx->CopyResource(pre_texture, input);

        //DebugOutDDS(pre_texture, "2.dds");

        frame_render_->Draw();
        auto final_texture = frame_render_->GetFinalTexture();
        return final_texture;
    }


}
