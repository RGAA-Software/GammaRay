//
// Created by hy on 19/11/2024.
//

#ifndef GAMMARAY_GR_ENCODER_PLUGIN_H
#define GAMMARAY_GR_ENCODER_PLUGIN_H

#include <d3d11.h>
#include "gr_plugin_interface.h"
#include <mutex>

namespace tc
{
    class Image;

    class GrEncoderPlugin : public GrPluginInterface {
    public:
        GrEncoderPlugin();
        ~GrEncoderPlugin() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        virtual bool OnDestroy() override;
        void InsertIdr() override;

        virtual void Encode(uint64_t handle, uint64_t frame_index);
        virtual void Encode(ID3D11Texture2D* tex2d);
        virtual void Encode(const std::shared_ptr<Image>& i420_data, uint64_t frame_index);

    protected:
        int refresh_rate_ = 60;
        uint32_t out_width_ = 0;
        uint32_t out_height_ = 0;
        int gop_size_ = 60;
        int bitrate_ = 10000000; // 10Mbps
        bool insert_idr_ = false;
        std::atomic_bool init_success_ = false;
    };

}

#endif //GAMMARAY_GR_ENCODER_PLUGIN_H
