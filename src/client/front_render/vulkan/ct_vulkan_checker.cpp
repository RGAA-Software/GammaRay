#include "ct_vulkan_checker.h"
#include "tc_common_new/log.h"
#include "ct_test_ffmpeg_vulkan_decoder.h"
#include "ct_test_vulkan_video_widget.h"
#include "pl_vulkan.h"

namespace tc {

    std::shared_ptr<VulkanChecker> VulkanChecker::Make() {
        return std::make_shared<VulkanChecker>();
    }

    VulkanChecker::VulkanChecker() : QObject(nullptr) {
    
    }

    VulkanChecker::~VulkanChecker() {
    
    }

    bool VulkanChecker::TestDecodeAndRenderHevcYuv444Frame() {
        
        render_widget_ = std::make_shared<TestVulkanVideoWidget>();
        pl_vulkan_ = PlVulkan::Make();

        HWND render_hwnd = reinterpret_cast<HWND>(render_widget_->winId());

        bool res = pl_vulkan_->Initialize(render_hwnd);
        if (!res) {
            LOGW("vulkan init error");
            return false;
        }
        
        ffmpeg_vulkan_decoder_ = TestFFmpegVulkanDecoder::Make();
        res = ffmpeg_vulkan_decoder_->InitTestHevcDecoder();
        if (!res) {
            LOGW("ffmpeg init decoder error");
            return false;
        }

        AVBufferRef* hw_device_ctx = pl_vulkan_->GetHwDeviceCtx();
        ffmpeg_vulkan_decoder_->SetHwDeviceCtx(hw_device_ctx);
        res = ffmpeg_vulkan_decoder_->OpenTestHevcDecoder();
        if (!res) {
            LOGW("ffmpeg open decoder error");
            return false;
        }

        auto frame_res = ffmpeg_vulkan_decoder_->GetDecodeTestHevcYuv444Frame();
        if (!frame_res.has_value()) {
            qDebug() << "Failed to get decoded frame";
            return false;
        }
        AVFrame* frame = frame_res.value();
        if (AV_PIX_FMT_VULKAN != frame->format) {
            qDebug() << "frame format is not AV_PIX_FMT_VULKAN ";
            return false;
        }
        res = pl_vulkan_->renderFrame(frame);
        ffmpeg_vulkan_decoder_->FreeTestHevcYuv444Frame(frame);

        return res;
    }
}