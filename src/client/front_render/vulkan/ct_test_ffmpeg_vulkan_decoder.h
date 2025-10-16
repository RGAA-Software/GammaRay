#pragma once
#include <functional>
#include <optional>
#include <memory>
extern "C" {
	#include <libavcodec/codec.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/opt.h>
	#include <libavutil/log.h>
	#include <libavutil/pixdesc.h>
	#include <libavutil/hwcontext.h>
	#include <libavutil/avassert.h>
}

namespace tc {

    using TestPostAVFrameCallbackFuncType = std::function<void(AVFrame* frame)>;

    class TestFFmpegVulkanDecoder {
    public:
        static std::shared_ptr<TestFFmpegVulkanDecoder> Make();
        TestFFmpegVulkanDecoder();
        ~TestFFmpegVulkanDecoder() ;

        AVCodec* test_hevc_decoder_ = nullptr;
        AVCodecContext* test_hevc_video_decoder_ctx_ = nullptr;
        TestPostAVFrameCallbackFuncType test_hevc_post_av_frame_callback_func_ = nullptr;

        bool InitTestHevcDecoder();
        bool OpenTestHevcDecoder();
        std::optional<AVFrame*> GetDecodeTestHevcYuv444Frame();
        void FreeTestHevcYuv444Frame(AVFrame* frame);
        void SetHwDeviceCtx(AVBufferRef* hw_device_ctx);
       
        static const uint8_t k_HEVCRExt8_444TestFrame[];
    };

}