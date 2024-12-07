//
// Created by RGAA on 19/11/2024.
//

#ifndef GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H
#define GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H

#include <d3d11.h>
#include <mutex>
#include "gr_plugin_interface.h"
#include "tc_encoder_new/encoder_config.h"

namespace tc
{
    class Image;

    class GrVideoEncoderPlugin : public GrPluginInterface {
    public:
        GrVideoEncoderPlugin();
        ~GrVideoEncoderPlugin() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void InsertIdr() override;

        virtual bool CanEncodeTexture();
        virtual bool Init(const EncoderConfig& config);
        virtual void Encode(ID3D11Texture2D* tex2d, uint64_t frame_index, std::any extra);
        virtual void Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, std::any extra);
        virtual void Exit();

        EncoderConfig GetEncoderConfig();

    public:
        int refresh_rate_ = 60;
        uint32_t input_frame_width_ = 0;
        uint32_t input_frame_height_ = 0;
        uint32_t out_width_ = 0;
        uint32_t out_height_ = 0;
        int gop_size_ = 60;
        int bitrate_ = 10000000; // 10Mbps
        bool insert_idr_ = false;
        std::atomic_bool init_success_ = false;
        EncoderConfig encoder_config_;
    };

}

#endif //GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H
