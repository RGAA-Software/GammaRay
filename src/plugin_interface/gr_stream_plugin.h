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
        virtual void OnVideoEncoderCreated(const GrPluginEncodedVideoType& type, int width, int height);
        // data: encode video frame, h264/h265/...
        virtual void OnEncodedVideoFrame(const GrPluginEncodedVideoType& video_type,
                                         const std::shared_ptr<Data>& data,
                                         uint64_t frame_index,
                                         int frame_width,
                                         int frame_height,
                                         bool key);
        // raw video frame
        // handle: D3D Shared texture handle
        virtual void OnRawVideoFrameSharedTexture(uint64_t handle);
        // raw video frame in rgba format
        // image: Raw image
        virtual void OnRawVideoFrameRgba(const std::shared_ptr<Image>& image);
        // raw video frame in yuv(I420) format
        // image: Raw image
        virtual void OnRawVideoFrameYuv(const std::shared_ptr<Image>& image);

        // audio
        virtual void OnRawAudioData(const std::shared_ptr<Data>& data, int samples, int channels, int bits);
        virtual void OnSplitRawAudioData(const std::shared_ptr<Data>& left_ch_data,
                                         const std::shared_ptr<Data>& right_ch_data,
                                         int samples, int channels, int bits);
        virtual void OnSplitFFTAudioData(const std::vector<double>& left_fft, const std::vector<double>& right_fft);

    };

}

#endif //GAMMARAY_GR_STREAM_PLUGIN_H
