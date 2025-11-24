//
// Created by RGAA on 19/11/2024.
//

#ifndef GAMMARAY_GR_STREAM_PLUGIN_H
#define GAMMARAY_GR_STREAM_PLUGIN_H

#include "gr_plugin_interface.h"

namespace tc
{

    class GrStreamPlugin : public GrPluginInterface {
    public:
        GrStreamPlugin();
        ~GrStreamPlugin() override;

        // video
        void OnVideoEncoderCreated(const std::string& mon_name, const GrPluginEncodedVideoType& type, int width, int height) override;
        // data: encode video frame, h264/h265/...
        void OnEncodedVideoFrame(const std::string& mon_name,
                                 const GrPluginEncodedVideoType& video_type,
                                 const std::shared_ptr<Data>& data,
                                 uint64_t frame_index,
                                 int frame_width,
                                 int frame_height,
                                 bool key) override;
        // raw video frame
        // handle: D3D Shared texture handle
        void OnRawVideoFrameSharedTexture(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, uint64_t handle, int64_t adapter_id, uint64_t frame_format) override {}
        // raw video frame in rgba format
        // image: Raw image
        void OnRawVideoFrameRgba(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, const std::shared_ptr<Image>& image) override {}
        // raw video frame in yuv(I420) format
        // image: Raw image
        void OnRawVideoFrameYuv(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, const std::shared_ptr<Image>& image) override {}

        // audio
        void OnRawAudioData(const std::shared_ptr<Data>& data, int samples, int channels, int bits) override;
        void OnSplitRawAudioData(const std::shared_ptr<Data>& left_ch_data,
                                         const std::shared_ptr<Data>& right_ch_data,
                                         int samples, int channels, int bits) override;
        void OnSplitFFTAudioData(const std::vector<double>& left_fft, const std::vector<double>& right_fft) override;

    };

}

#endif //GAMMARAY_GR_STREAM_PLUGIN_H
