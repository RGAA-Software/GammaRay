#pragma once
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
        PlVulkan();
		~PlVulkan();

        bool CreatePlVulkanInstance();
        bool CreateWin32SurfaceFromHwnd(HWND hwnd);
        bool CreateSwapchain();
        bool CreatePlRender();
        bool InitAVHWDeviceContext();

        bool Initialize(HWND hwnd);
        bool ChooseVulkanDevice(PDECODER_PARAMETERS params, bool hdrOutputRequired);
        bool tryInitializeDevice(VkPhysicalDevice device, VkPhysicalDeviceProperties* deviceProps, PDECODER_PARAMETERS decoderParams, bool hdrOutputRequired);
        bool isExtensionSupportedByPhysicalDevice(VkPhysicalDevice device, const char* extensionName);
        bool isColorSpaceSupportedByPhysicalDevice(VkPhysicalDevice device, VkColorSpaceKHR colorSpace);
        bool isSurfacePresentationSupportedByPhysicalDevice(VkPhysicalDevice device);

        bool populateQueues(int videoFormat);

        bool prepareDecoderContext(AVCodecContext* context, AVDictionary**);

        void renderFrame(AVFrame* frame);

        bool mapAvFrameToPlacebo(const AVFrame* frame, pl_frame* mappedFrame);

        static void lockQueue(AVHWDeviceContext* dev_ctx, uint32_t queue_family, uint32_t index);
        static void unlockQueue(AVHWDeviceContext* dev_ctx, uint32_t queue_family, uint32_t index);



	public:
        SDL_Window* m_Window = nullptr;

        // The libplacebo rendering state
        pl_log m_Log = nullptr;
        pl_vk_inst m_PlVkInstance = nullptr;
        VkSurfaceKHR m_VkSurface = VK_NULL_HANDLE;
        pl_vulkan m_Vulkan = nullptr;
        pl_swapchain m_Swapchain = nullptr;
        pl_renderer m_Renderer = nullptr;
        pl_tex m_Textures[PL_MAX_PLANES] = {};
        pl_color_space m_LastColorspace = {};

        // Pending swapchain state shared between waitToRender(), renderFrame(), and cleanupRenderContext()
        pl_swapchain_frame m_SwapchainFrame = {};

        bool m_HwAccelBackend = true;

        // Device context used for hwaccel decoders
        AVBufferRef* m_HwDeviceCtx = nullptr;

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