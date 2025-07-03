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
        // logo point / 1 pixel
        {
            auto file = File::OpenForReadB(":/resources/ic_logo_point.png");
            auto data = file->ReadAll();
            logo_image_ = Image::MakeByCompressedImage(data);
        }

        // logo src
        {
            QImage image;
            image.load(":/resources/ic_logo_src.png");
            LOGI("logo src image, size: {}x{}, channels:{}, data size: {}", image.width(), image.height(), (int)image.format(), image.sizeInBytes());
            for (int h = 0; h < image.height(); h++) {
                for (int w = 0; w < image.width(); w++) {
                    auto r = image.pixel(w, h);
                    if (qRed(r) == 0) {
                        logo_points_.emplace_back(w, h);
                    }
                }
            }
        }

        // big logo src
        {
            QImage image;
            image.load(":/resources/ic_logo_src_big.png");
            LOGI("logo src image, size: {}x{}, channels:{}, data size: {}", image.width(), image.height(), (int)image.format(), image.sizeInBytes());
            for (int h = 0; h < image.height(); h++) {
                for (int w = 0; w < image.width(); w++) {
                    auto r = image.pixel(w, h);
                    if (qRed(r) == 0) {
                        big_logo_points_.emplace_back(w, h);
                    }
                }
            }
        }

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
        frame_carrier = std::make_shared<VideoFrameCarrier>(this,
                                                            params.d3d_device_,
                                                            params.d3d_device_context_,
                                                            params.adapter_uid_,
                                                            params.mon_name_,
                                                            params.enable_full_color_mode_);
        frame_carriers_[params.mon_name_] = frame_carrier;
        LOGI("Create frame carrier for monitor: {},", params.mon_name_);
        return true;
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

    bool FrameCarrierPlugin::MapRawTexture(const std::string& mon_name, const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture, DXGI_FORMAT format, int height,
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

    std::shared_ptr<Image> FrameCarrierPlugin::GetLogoImage() {
        return logo_image_;
    }

    std::vector<QPoint> FrameCarrierPlugin::GetLogoPoints() {
        return logo_points_;
    }

    std::vector<QPoint> FrameCarrierPlugin::GetBigLogoPoints() {
        return big_logo_points_;
    }

}
