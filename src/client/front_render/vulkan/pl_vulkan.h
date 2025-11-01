#pragma once
#include <memory>
#include <vector>
#include <map>
#include <array>
#include <cstdint>
#include <Windows.h>

#include <SDL.h>
#undef main

#ifdef Q_OS_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <libplacebo/log.h>
#include <libplacebo/renderer.h>
#include <libplacebo/vulkan.h>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/hwcontext.h>
    #include <libavutil/pixdesc.h>
    #include <libavutil/hwcontext_vulkan.h>
}

namespace tc { 

    class DECODER_PARAMETERS {
    public:
        enum AVCodecID codec_id;      // H.264 / HEVC / AV1
        int profile;
        int level;
        int bit_depth;

        //int width;
        //int height;

        enum AVPixelFormat hw_format; // e.g. AV_PIX_FMT_VULKAN
        enum AVPixelFormat sw_format; // e.g. AV_PIX_FMT_YUV444P

        // Vulkan context (from FFmpeg Vulkan hwctx)
        VkDevice device;
        VkPhysicalDevice phys_device;
        VkQueue queue;

        // chroma info
        int chroma_subsample_w;
        int chroma_subsample_h;

        // keep reference to frames_ctx
        AVBufferRef* hw_frames_ctx;
    };

    using PDECODER_PARAMETERS = DECODER_PARAMETERS*;

	class PlVulkan {
	public:
        static std::shared_ptr<PlVulkan> Make();
        PlVulkan();
		~PlVulkan();

        bool CreatePlVulkanInstance();
        bool CreateWin32SurfaceFromHwnd(uintptr_t game_view_ptr, HWND hwnd);
        bool CreateSwapchain(uintptr_t game_view_ptr);
        bool CreatePlRender(uintptr_t game_view_ptr);
        bool InitAVHWDeviceContext();

        bool Initialize(uintptr_t game_view_ptr, HWND hwnd);

        bool CreateRenderComponent(uintptr_t game_view_ptr, HWND hwnd);

        bool ChooseVulkanDevice(uintptr_t game_view_ptr, PDECODER_PARAMETERS params, bool hdrOutputRequired);
        bool tryInitializeDevice(uintptr_t game_view_ptr, VkPhysicalDevice device, VkPhysicalDeviceProperties* deviceProps, PDECODER_PARAMETERS decoderParams, bool hdrOutputRequired);
        bool isExtensionSupportedByPhysicalDevice(uintptr_t game_view_ptr, VkPhysicalDevice device, const char* extensionName);
        bool isColorSpaceSupportedByPhysicalDevice(uintptr_t game_view_ptr, VkPhysicalDevice device, VkColorSpaceKHR colorSpace);
        bool isSurfacePresentationSupportedByPhysicalDevice(uintptr_t game_view_ptr, VkPhysicalDevice device);

        bool populateQueues(int videoFormat);

        bool prepareDecoderContext(AVCodecContext* context, AVDictionary**);
        AVBufferRef* GetHwDeviceCtx() { 
            return m_HwDeviceCtx; 
        }

        bool RenderFrame(uintptr_t game_view_ptr, AVFrame* frame);

        bool mapAvFrameToPlacebo(uintptr_t game_view_ptr, const AVFrame* frame, pl_frame* mappedFrame);

        static void lockQueue(AVHWDeviceContext* dev_ctx, uint32_t queue_family, uint32_t index);
        static void unlockQueue(AVHWDeviceContext* dev_ctx, uint32_t queue_family, uint32_t index);
	public:
        // The libplacebo rendering state
        pl_log m_Log = nullptr;
        pl_vk_inst m_PlVkInstance = nullptr;
        pl_vulkan m_Vulkan = nullptr;

        bool m_HwAccelBackend = true;

        // Device context used for hwaccel decoders
        AVBufferRef* m_HwDeviceCtx = nullptr;

        //创建多个交换链等 以支持渲染多个窗口
        std::map<uintptr_t, VkSurfaceKHR> vulkan_surfaces_;
        std::map<uintptr_t, pl_swapchain> vulkan_swapchains_;
        std::map<uintptr_t, pl_renderer> vulkan_renderers_;
        std::map<uintptr_t, std::array<pl_tex, PL_MAX_PLANES>> textures_; // 使用 std::array<pl_tex, PL_MAX_PLANES> 更容易管理

        // Vulkan functions we call directly
        PFN_vkDestroySurfaceKHR fn_vkDestroySurfaceKHR = nullptr;
        PFN_vkGetPhysicalDeviceQueueFamilyProperties2 fn_vkGetPhysicalDeviceQueueFamilyProperties2 = nullptr;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fn_vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fn_vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
        PFN_vkEnumeratePhysicalDevices fn_vkEnumeratePhysicalDevices = nullptr;
        PFN_vkGetPhysicalDeviceProperties fn_vkGetPhysicalDeviceProperties = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceSupportKHR fn_vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
        PFN_vkEnumerateDeviceExtensionProperties fn_vkEnumerateDeviceExtensionProperties = nullptr;
	};
}