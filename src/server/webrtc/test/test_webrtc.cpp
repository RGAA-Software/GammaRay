//
// Created by RGAA on 2024/3/4.
//

#include <thread>
#include <chrono>

#include "tc_encoder_new/video_encoder_factory.h"
#include "tc_encoder_new/ffmpeg_video_encoder.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"
#include "webrtc_server_impl.h"

using namespace tc;
int main(int argc, char** argv) {
//
    auto srv = WebRtcServerImpl::Make();
    srv->Init(WebRtcServerParam{});

#if 0
    tc::EncoderConfig encoder_config;
    encoder_config.width = 1024;
    encoder_config.height = 768;
    encoder_config.codec_type = tc::EVideoCodecType::kH264;
    encoder_config.gop_size = 60;
    encoder_config.fps = 60;
    encoder_config.bitrate = 25 * 1000000;

    auto video_encoder = std::make_shared<FFmpegVideoEncoder>(nullptr, EncoderFeature{-1, 0});
    if(!video_encoder->Initialize(encoder_config)) {
        printf("FFmpegVideoEncoder Initialize error\n");
        return -1;
    }

    video_encoder->RegisterEncodeCallback([](const std::shared_ptr<Image>& frame, uint64_t frame_index, bool key) {
        LOGI("Encoded frame idx: {}, key: {}, buffer size: {}", frame_index, key, frame->GetData()->Size());
    });

    while (true) {
         int size = encoder_config.width * encoder_config.height * 1.5;
         auto data = Data::Make(nullptr, size);
         auto image = Image::Make(data, encoder_config.width, encoder_config.height);
         static int frame_idx = 0;
        video_encoder->Encode(image, frame_idx++);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
#endif

    while (true) {
        MsgVideoFrameEncoded msg;
        msg.frame_width_ = 1024;
        msg.frame_height_ = 1024;
        msg.frame_format_ = 0;
        msg.image_ = Image::Make(nullptr, 1024, 1024, 3);
        srv->OnImageEncoded(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}