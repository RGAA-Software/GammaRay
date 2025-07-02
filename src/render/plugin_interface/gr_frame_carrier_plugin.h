//
// Created by RGAA on 27/06/2025.
//

#ifndef GAMMARAY_GR_FRAME_CARRIER_PLUGIN_H
#define GAMMARAY_GR_FRAME_CARRIER_PLUGIN_H

#include <memory>
#include <optional>
#include "gr_plugin_interface.h"

namespace tc
{

    //
    class GrFrameProcessorPlugin;

    // Params
    class GrCarrierParams {
    public:
        std::string mon_name_;
        bool frame_resize_ = false;
        Microsoft::WRL::ComPtr<ID3D11Device> d3d_device_ = nullptr;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d_device_context_ = nullptr;
        int64_t adapter_uid_ = 0;
        int encode_width_ = 0;
        int encode_height_ = 0;
        bool enable_full_color_mode_ = false;
        GrFrameProcessorPlugin* frame_resize_plugin_ = nullptr;
    };

    // After copied
    class GrCarriedFrame {
    public:
        std::string mon_name_;
        uint64_t frame_index_ = 0;
#ifdef WIN32
        ID3D11Texture2D* texture_ = nullptr;
#endif
    };

    // Resize Info
    class GrFrameResizeInfo {
    public:
        std::string mon_name_;
        int resize_width_ = 0;
        int resize_height_ = 0;
    };

    class GrFrameCarrierPlugin : public GrPluginInterface {
    public:
        // Create frame carrier
        virtual bool InitFrameCarrier(const GrCarrierParams& params) {
            carrier_params_[params.mon_name_] = params;
            return false;
        }

        // DDA / GDI / Hook -> Handle -> OpenShared -> Copy it
        virtual std::shared_ptr<GrCarriedFrame> CopyTexture(const std::string& mon_name, uint64_t handle, uint64_t frame_index) {
            return nullptr;
        }

        // Map Texture from GPU -> CPU
        virtual bool MapRawTexture(const std::string& mon_name, ID3D11Texture2D* texture, DXGI_FORMAT format, int height,
                           std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                           std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
            return false;
        }

        // RGBA -> YUV
        virtual bool ConvertRawImage(const std::string& mon_name, const std::shared_ptr<Image> image,
                             std::function<void(const std::shared_ptr<Image>&)>&& rgba_cbk,
                             std::function<void(const std::shared_ptr<Image>&)>&& yuv_cbk) {
            return false;
        }

        virtual std::optional<GrFrameResizeInfo> GetFrameResizeInfo(const std::string& mon_name) {
            return std::nullopt;
        }

    protected:
        std::map<std::string, GrCarrierParams> carrier_params_;
    };

}

#endif //GAMMARAY_GR_FRAME_CARRIER_PLUGIN_H
