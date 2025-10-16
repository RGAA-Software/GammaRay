#pragma once
#include <memory>
#include <qobject.h>


namespace tc {

    class PlVulkan;
    class TestVulkanVideoWidget;
    class TestFFmpegVulkanDecoder;

    class VulkanChecker : public QObject {
    public:
        static std::shared_ptr<VulkanChecker> Make();
        VulkanChecker();
        ~VulkanChecker();

        bool TestDecodeAndRenderHevcYuv444Frame();
    private:
        std::shared_ptr<TestVulkanVideoWidget> render_widget_ = nullptr;
        std::shared_ptr<PlVulkan> pl_vulkan_ = nullptr;
        std::shared_ptr<TestFFmpegVulkanDecoder> ffmpeg_vulkan_decoder_ = nullptr;
    };


}