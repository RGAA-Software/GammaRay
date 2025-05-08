//
// Created by RGAA on 19/11/2024.
//

#ifndef GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H
#define GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H

#include <d3d11.h>
#include <mutex>
#include <map>
#include <optional>
#include "gr_plugin_interface.h"
#include "tc_encoder_new/encoder_config.h"

namespace tc
{
    class Image;

    // dynamic working capture information
    class WorkingEncoderInfo {
    public:
        // monitor name or hook capturing
        std::string target_name_;
        int32_t fps_ = 0;
        // NVENC / AMF / SOFTWARE
        std::string encoder_name_;
        // max 180
        std::vector<int32_t> encode_durations_;
    };
    using WorkingEncoderInfoPtr = std::shared_ptr<WorkingEncoderInfo>;

    class GrVideoEncoderPlugin : public GrPluginInterface {
    public:
        GrVideoEncoderPlugin();
        ~GrVideoEncoderPlugin() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void InsertIdr() override;

        virtual bool CanEncodeTexture();
        virtual bool HasEncoderForMonitor(const std::string& monitor_name) = 0;
        virtual bool Init(const EncoderConfig& config, const std::string& monitor_name);
        virtual void Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, const std::any& extra);
        virtual void Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra);
        virtual void Exit(const std::string& monitor_name);
        virtual void ExitAll();
        // encoding information for monitors/hook
        virtual std::map<std::string, WorkingEncoderInfoPtr> GetWorkingCapturesInfo() = 0;

        
        std::optional<EncoderConfig> GetEncoderConfig(const std::string& monitor_name);

    public:
        int refresh_rate_ = 60;
        uint32_t out_width_ = 0;
        uint32_t out_height_ = 0;
        int gop_size_ = 60;
        int bitrate_ = 10000000; // 10Mbps
        bool insert_idr_ = false;
        std::map<std::string, EncoderConfig> encoder_configs_;
    };

}

#endif //GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H
