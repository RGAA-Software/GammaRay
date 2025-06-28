//
// Created RGAA on 15/11/2024.
//

#include "frame_carrier_plugin.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "video_frame_carrier.h"
#include "render/plugins/plugin_ids.h"
#include "plugin_interface/gr_plugin_events.h"

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
        // release the old one
        auto frame_carrier = GetFrameCarrier(params.mon_name_);
        if (frame_carrier != nullptr) {
            frame_carrier->Exit();
            frame_carriers_.erase(params.mon_name_);
            frame_carrier = nullptr;
        }

        // create a new one
        if (params.frame_resize_) {
            frame_carrier = std::make_shared<VideoFrameCarrier>(this,
                                                                params.d3d_device_,
                                                                params.d3d_device_context_,
                                                                params.adapter_uid_,
                                                                params.mon_name_,
                                                                params.frame_resize_,
                                                                params.encode_width_,
                                                                params.encode_height_,
                                                                params.full_color_mode_);
        }
        else {
            frame_carrier = std::make_shared<VideoFrameCarrier>(this,
                                                                params.d3d_device_,
                                                                params.d3d_device_context_,
                                                                params.adapter_uid_,
                                                                params.mon_name_,
                                                                false,
                                                                -1,
                                                                -1,
                                                                params.full_color_mode_);
        }
        frame_carriers_[params.mon_name_] = frame_carrier;
        LOGI("Create frame carrier for monitor: {}, resize?: {}", params.mon_name_, params.frame_resize_);

        //
        auto file = File::OpenForReadB("ic_logo_point.png");
        auto data = file->ReadAll();
        logo_image_ = Image::MakeByCompressedImage(data);

        return true;
    }

    GrFrameProcessorPlugin* FrameCarrierPlugin::GetFrameResizePlugin(const std::string& mon_name) {
        if (carrier_params_.contains(mon_name)) {
            return carrier_params_[mon_name].frame_resize_plugin_;
        }
        return nullptr;
    }

    std::shared_ptr<GrCarriedFrame> FrameCarrierPlugin::CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index) {
        auto frame_carrier = GetFrameCarrier(mon_name);
        if (!frame_carrier) {
            LOGE("Can't find frame carrier for monitor: {}", mon_name);
            return nullptr;
        }
        auto texture_2d = frame_carrier->CopyTexture(mon_name, handle, frame_index);
        return std::make_shared<GrCarriedFrame>(GrCarriedFrame {
            .mon_name_ = mon_name,
            .frame_index_ = frame_index,
            .texture_ = texture_2d,
        });
    }

    std::shared_ptr<VideoFrameCarrier> FrameCarrierPlugin::GetFrameCarrier(const std::string& monitor_name) {
        if (frame_carriers_.contains(monitor_name)) {
            return frame_carriers_[monitor_name];
        }
        return nullptr;
    }

    bool FrameCarrierPlugin::MapRawTexture(const std::string& mon_name, ID3D11Texture2D* texture, DXGI_FORMAT format, int height,
                       std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                       std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
        if (auto frame_carrier = GetFrameCarrier(mon_name); frame_carrier) {
            return frame_carrier->MapRawTexture(texture, format, height, std::move(rgba_cbk), std::move(yuv_cbk));
        }
        return false;
    }

    bool FrameCarrierPlugin::ConvertRawImage(const std::string& mon_name, const std::shared_ptr<Image> image,
                         std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                         std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
        if (auto frame_carrier = GetFrameCarrier(mon_name); frame_carrier) {
            return frame_carrier->ConvertRawImage(image, std::move(rgba_cbk), std::move(yuv_cbk));
        }
        return false;
    }

    std::optional<GrFrameResizeInfo> FrameCarrierPlugin::GetFrameResizeInfo(const std::string &mon_name) {
        if (auto frame_carrier = GetFrameCarrier(mon_name); frame_carrier) {
            return GrFrameResizeInfo {
                .mon_name_ = mon_name,
                .resize_width_ = frame_carrier->GetResizeWidth(),
                .resize_height_ = frame_carrier->GetResizeHeight(),
            };
        }
        return std::nullopt;
    }

    std::shared_ptr<Image> FrameCarrierPlugin::GetLogoImage() {
        return logo_image_;
    }

}
