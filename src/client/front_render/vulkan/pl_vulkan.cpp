#include "pl_vulkan.h"
#include <set>
#include <qdebug.h>
#include <qsize.h>
#include <qrect.h>
#include <SDL.h>

#define PL_LIBAV_IMPLEMENTATION 0
#include <libplacebo/utils/libav.h>

#include <SDL_vulkan.h>

#include <libavutil/hwcontext_vulkan.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h> 

#include "tc_common_new/log.h"

#define LOG_ERROR(msg) qCritical() << msg
// 或者： #define LOG_ERROR(msg) std::cerr << msg << std::endl


#define POPULATE_FUNCTION(name) \
    fn_##name = (PFN_##name)m_PlVkInstance->get_proc_addr(m_PlVkInstance->instance, #name); \
    if (fn_##name == nullptr) { \
        LOGE("Missing required Vulkan function:  {}", #name);\
        return false; \
    }

// Keep these in sync with hwcontext_vulkan.c
static const char* k_OptionalDeviceExtensions[] = {
    /* Misc or required by other extensions */
    //VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
    VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
    VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
    VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME,
    VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
    VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME,

    /* Imports/exports */
    VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
    VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
#ifdef Q_OS_WIN32
    VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
#endif

    /* Video encoding/decoding */
    VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME,
#if LIBAVCODEC_VERSION_MAJOR >= 61
    VK_KHR_VIDEO_DECODE_AV1_EXTENSION_NAME, // FFmpeg 7.0 uses the official Khronos AV1 extension
#else
    "VK_MESA_video_decode_av1", // FFmpeg 6.1 uses the Mesa AV1 extension
#endif
};

static void plLogCallback(void* priv, enum pl_log_level level, const char* msg) {
    qDebug().noquote() << "[libplacebo]" << msg;
    LOGI("[libplacebo] {}", msg);
}

namespace tc { 

    std::shared_ptr<PlVulkan> PlVulkan::Make() {
        return std::make_shared<PlVulkan>();
    }

	PlVulkan::PlVulkan() {
	
	}

	PlVulkan::~PlVulkan() {
        for (auto& pair : textures_) {
            auto texture_array = pair.second;
            for (int i = 0; i < texture_array.size(); i++) {
                if (m_Vulkan && m_Vulkan->gpu && texture_array[i]) {
                    pl_tex_destroy(m_Vulkan->gpu, &texture_array[i]);
                }
            }
        }

        for (auto& pair : vulkan_renderers_) {
            auto render = pair.second;
            if (render) {
                pl_renderer_destroy(&render);
            }
        }

        for (auto& pair : vulkan_swapchains_) {
            auto swapchain = pair.second;
            if (swapchain) {
                pl_swapchain_destroy(&swapchain);
            }
        }

        if (m_Vulkan) {
            pl_vulkan_destroy(&m_Vulkan);
        }

        if (fn_vkDestroySurfaceKHR) {
            for (auto& pair : vulkan_surfaces_) {
                auto surface = pair.second;
                if (surface) {
                    fn_vkDestroySurfaceKHR(m_PlVkInstance->instance, surface, nullptr);
                }
            }
        }

        if (m_HwDeviceCtx != nullptr) {
            av_buffer_unref(&m_HwDeviceCtx);
        }

        if (m_PlVkInstance) {
            pl_vk_inst_destroy(&m_PlVkInstance);
        }

        // m_Log must always be the last object destroyed
        if (m_Log) {
            pl_log_destroy(&m_Log);
        }
	}

    bool PlVulkan::CreatePlVulkanInstance() {
        // 创建 Vulkan 实例参数
        pl_vk_inst_params vkInstParams = pl_vk_inst_default_params;
        {
            vkInstParams.debug_extra = !!qEnvironmentVariableIntValue("PLVK_DEBUG_EXTRA");
            vkInstParams.debug = vkInstParams.debug_extra || !!qEnvironmentVariableIntValue("PLVK_DEBUG");
        }

        // 用系统函数获取 vkGetInstanceProcAddr
        vkInstParams.get_proc_addr = (PFN_vkGetInstanceProcAddr)vkGetInstanceProcAddr;

        // 自己指定需要的扩展
        static const char* instanceExtensions[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        };
        vkInstParams.extensions = instanceExtensions;
        vkInstParams.num_extensions = sizeof(instanceExtensions) / sizeof(instanceExtensions[0]);

        struct pl_log_params lparams {};
        lparams.log_cb = plLogCallback;                // or set to a callback if you want colored output
        lparams.log_level = PL_LOG_DEBUG;
        m_Log = pl_log_create(PL_API_VER, &lparams);
        if (!m_Log) {
            LOGW("pl_log_create failed");
            return false;
        }

        // 创建 libplacebo 的 Vulkan 实例
        m_PlVkInstance = pl_vk_inst_create(m_Log, &vkInstParams);
        if (!m_PlVkInstance) {
            LOGW("pl_vk_inst_create() failed");
            return false;
        }

        // 获取 Vulkan 函数指针
        POPULATE_FUNCTION(vkDestroySurfaceKHR);
        POPULATE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties2);
        POPULATE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
        POPULATE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
        POPULATE_FUNCTION(vkEnumeratePhysicalDevices);
        POPULATE_FUNCTION(vkGetPhysicalDeviceProperties);
        POPULATE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
        POPULATE_FUNCTION(vkEnumerateDeviceExtensionProperties);
        return true;
    }

    bool PlVulkan::CreateWin32SurfaceFromHwnd(uintptr_t game_view_ptr, HWND hwnd) {

        std::array<pl_tex, PL_MAX_PLANES> texture = {0};
        textures_[game_view_ptr] = texture;

        // 用 Win32 API 创建 surface
        VkWin32SurfaceCreateInfoKHR sci{};
        sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        sci.hinstance = GetModuleHandle(nullptr);
        sci.hwnd = hwnd;

        VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
        vulkan_surfaces_[game_view_ptr] = vk_surface;

        if (vkCreateWin32SurfaceKHR(m_PlVkInstance->instance, &sci, nullptr, &vulkan_surfaces_[game_view_ptr]) != VK_SUCCESS) {
            LOGE("vkCreateWin32SurfaceKHR failed");
            return false;
        }

        LOGI("Vulkan surface created successfully!");
        return true;
    }

    // 与surface关联
    bool PlVulkan::CreateSwapchain(uintptr_t game_view_ptr) {
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;  // 这种模式也被称为 立即模式 或 无垂直同步模式

        pl_vulkan_swapchain_params vkSwapchainParams = {};
        vkSwapchainParams.surface = vulkan_surfaces_[game_view_ptr];
        vkSwapchainParams.present_mode = presentMode;
        vkSwapchainParams.swapchain_depth = 1; // No queued frames
#if PL_API_VER >= 338
        vkSwapchainParams.disable_10bit_sdr = true; // Some drivers don't dither 10-bit SDR output correctly
#endif
        vulkan_swapchains_[game_view_ptr] = pl_vulkan_create_swapchain(m_Vulkan, &vkSwapchainParams);
        if (vulkan_swapchains_[game_view_ptr] == nullptr) {
            LOGE("pl_vulkan_create_swapchain() failed");
            return false;
        }

        return true;
    }

    bool PlVulkan::CreatePlRender(uintptr_t game_view_ptr) {
        vulkan_renderers_[game_view_ptr] = pl_renderer_create(m_Log, m_Vulkan->gpu);
        if (vulkan_renderers_[game_view_ptr] == nullptr) {
            LOGE("pl_renderer_create() failed");
            return false;
        }
        return true;
    }

    bool PlVulkan::InitAVHWDeviceContext() {
        m_HwDeviceCtx = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_VULKAN);
        if (m_HwDeviceCtx == nullptr) {
            LOGE("av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_VULKAN) failed");
            return false;
        }

        auto hwDeviceContext = ((AVHWDeviceContext*)m_HwDeviceCtx->data);
        hwDeviceContext->user_opaque = this; // Used by lockQueue()/unlockQueue()

        auto vkDeviceContext = (AVVulkanDeviceContext*)((AVHWDeviceContext*)m_HwDeviceCtx->data)->hwctx;
        vkDeviceContext->get_proc_addr = m_PlVkInstance->get_proc_addr;
        vkDeviceContext->inst = m_PlVkInstance->instance;
        vkDeviceContext->phys_dev = m_Vulkan->phys_device;
        vkDeviceContext->act_dev = m_Vulkan->device;
        vkDeviceContext->device_features = *m_Vulkan->features;
        vkDeviceContext->enabled_inst_extensions = m_PlVkInstance->extensions;
        vkDeviceContext->nb_enabled_inst_extensions = m_PlVkInstance->num_extensions;
        vkDeviceContext->enabled_dev_extensions = m_Vulkan->extensions;
        vkDeviceContext->nb_enabled_dev_extensions = m_Vulkan->num_extensions;
#if LIBAVUTIL_VERSION_INT > AV_VERSION_INT(58, 9, 100)
        vkDeviceContext->lock_queue = lockQueue;
        vkDeviceContext->unlock_queue = unlockQueue;
#endif
        // Populate the device queues for decoding this video format
        populateQueues(/*params->videoFormat*/ -1); // LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(59, 34, 100) 不用videoFormat参数

        int err = av_hwdevice_ctx_init(m_HwDeviceCtx);
        if (err < 0) {
            LOGE("av_hwdevice_ctx_init() failed: {}", err);
            return false;
        }

        return true;
    }

    bool PlVulkan::Initialize(uintptr_t game_view_ptr, HWND hwnd) {
        
        if (!CreatePlVulkanInstance()) {
            LOGE("CreatePlVulkanInstance() failed");
            return false;
        }

        if (!CreateWin32SurfaceFromHwnd(game_view_ptr, hwnd)) {
            LOGE("CreateWin32SurfaceFromHwnd() failed");
            return false;
        }

        if (!ChooseVulkanDevice(game_view_ptr, nullptr, false)) {
            LOGE("ChooseVulkanDevice() failed");
            return false;
        }

        if (!CreateSwapchain(game_view_ptr)) {
            LOGE("CreateSwapchain() failed");
            return false;
        }

        if (!CreatePlRender(game_view_ptr)) {
            LOGE("CreatePlRender() failed");
            return false;
        }

        if (!InitAVHWDeviceContext()) {
            LOGE("InitAVHWDeviceContext() failed");
            return false;
        }

        LOGI("Vulkan initialized successfully!");
        return true;
    }

    bool PlVulkan::ChooseVulkanDevice(uintptr_t game_view_ptr, PDECODER_PARAMETERS params, bool hdrOutputRequired)
    {
        /*
        * 暂时不用支持HDR输出
        * HDR（High Dynamic Range，高动态范围）输出是音视频领域的一项重要技术，它通过扩展亮度和色彩范围，带来更加逼真和沉浸式的视觉体验。
        */
        hdrOutputRequired = false;

        uint32_t physicalDeviceCount = 0;
        fn_vkEnumeratePhysicalDevices(m_PlVkInstance->instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        fn_vkEnumeratePhysicalDevices(m_PlVkInstance->instance, &physicalDeviceCount, physicalDevices.data());

        std::set<uint32_t> devicesTried;
        VkPhysicalDeviceProperties deviceProps;

        if (physicalDeviceCount == 0) {
            LOGE("No Vulkan devices found!");
            return false;
        }

        // First, try the first device in the list to support device selection layers
        // that put the user's preferred GPU in the first slot.
        fn_vkGetPhysicalDeviceProperties(physicalDevices[0], &deviceProps);
        if (tryInitializeDevice(game_view_ptr, physicalDevices[0], &deviceProps, params, hdrOutputRequired)) {
            return true;
        }
        devicesTried.emplace(0);

        // Next, we'll try to match an integrated GPU, since we want to minimize
        // power consumption and inter-GPU copies.
        for (uint32_t i = 0; i < physicalDeviceCount; i++) {
            // Skip devices we've already tried
            if (devicesTried.find(i) != devicesTried.end()) {
                continue;
            }

            VkPhysicalDeviceProperties deviceProps;
            fn_vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProps);
            if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
                if (tryInitializeDevice(game_view_ptr, physicalDevices[i], &deviceProps, params, hdrOutputRequired)) {
                    return true;
                }
                devicesTried.emplace(i);
            }
        }

        // Next, we'll try to match a discrete GPU.
        for (uint32_t i = 0; i < physicalDeviceCount; i++) {
            // Skip devices we've already tried
            if (devicesTried.find(i) != devicesTried.end()) {
                continue;
            }

            VkPhysicalDeviceProperties deviceProps;
            fn_vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProps);
            if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                if (tryInitializeDevice(game_view_ptr, physicalDevices[i], &deviceProps, params, hdrOutputRequired)) {
                    return true;
                }
                devicesTried.emplace(i);
            }
        }

        // Finally, we'll try matching any non-software device.
        for (uint32_t i = 0; i < physicalDeviceCount; i++) {
            // Skip devices we've already tried
            if (devicesTried.find(i) != devicesTried.end()) {
                continue;
            }

            VkPhysicalDeviceProperties deviceProps;
            fn_vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProps);
            if (tryInitializeDevice(game_view_ptr, physicalDevices[i], &deviceProps, params, hdrOutputRequired)) {
                return true;
            }
            devicesTried.emplace(i);
        }

        LOGE("No suitable {}Vulkan devices found!", hdrOutputRequired ? "HDR-capable " : "");
        return false;
    }

    bool PlVulkan::tryInitializeDevice(uintptr_t game_view_ptr, VkPhysicalDevice device, VkPhysicalDeviceProperties* deviceProps, PDECODER_PARAMETERS decoderParams, bool hdrOutputRequired)
    {

        // Check the Vulkan API version first to ensure it meets libplacebo's minimum
        if (deviceProps->apiVersion < PL_VK_MIN_VERSION) {
            LOGE("Vulkan device {} does not meet minimum Vulkan version", deviceProps->deviceName);
            return false;
        }

#ifdef Q_OS_WIN32
        // Intel's Windows drivers seem to have interoperability issues as of FFmpeg 7.0.1
        // when using Vulkan Video decoding. Since they also expose HEVC REXT profiles using
        // D3D11VA, let's reject them here so we can select a different Vulkan device or
        // just allow D3D11VA to take over.
        if (m_HwAccelBackend && deviceProps->vendorID == 0x8086 && !qEnvironmentVariableIntValue("PLVK_ALLOW_INTEL")) {
            LOGE("Skipping Intel GPU for Vulkan Video due to broken drivers");
            return false;
        }
#endif

        // If we're acting as the decoder backend, we need a physical device with Vulkan video support
        if (m_HwAccelBackend) {
            const char* videoDecodeExtension;
            // 这里直接使用265测试
            videoDecodeExtension = VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME;
            if (!isExtensionSupportedByPhysicalDevice(game_view_ptr, device, videoDecodeExtension)) {
                LOGE("Vulkan device {} does not support {}", deviceProps->deviceName, videoDecodeExtension);
                return false;
            }
        }

        if (!isSurfacePresentationSupportedByPhysicalDevice(game_view_ptr, device)) {
            LOGE("Vulkan device {} does not support presenting on window surface", deviceProps->deviceName);
            return false;
        }

        if (hdrOutputRequired && !isColorSpaceSupportedByPhysicalDevice(game_view_ptr, device, VK_COLOR_SPACE_HDR10_ST2084_EXT)) {
            LOGE("Vulkan device {} does not support HDR10 (ST.2084 PQ)", deviceProps->deviceName);
            return false;
        }

        // Avoid software GPUs
        if (deviceProps->deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU /*&& qgetenv("PLVK_ALLOW_SOFTWARE") != "1"*/) {
            LOGE("Vulkan device {} is a a (probably slow) software renderer. Set PLVK_ALLOW_SOFTWARE=1 to allow using this device.", deviceProps->deviceName);
            return false;
        }

        pl_vulkan_params vkParams = pl_vulkan_default_params;
        vkParams.instance = m_PlVkInstance->instance;
        vkParams.get_proc_addr = m_PlVkInstance->get_proc_addr;
        vkParams.surface = vulkan_surfaces_[game_view_ptr];
        vkParams.device = device;
        vkParams.opt_extensions = k_OptionalDeviceExtensions;
        vkParams.num_opt_extensions = SDL_arraysize(k_OptionalDeviceExtensions);
        vkParams.extra_queues = m_HwAccelBackend ? VK_QUEUE_FLAG_BITS_MAX_ENUM : 0;
        m_Vulkan = pl_vulkan_create(m_Log, &vkParams);
        if (m_Vulkan == nullptr) {
            LOGE("pl_vulkan_create() failed for {}", deviceProps->deviceName);
            return false;
        }

        LOGI("Vulkan device chosen: {}", deviceProps->deviceName);
        return true;
    }

    bool PlVulkan::isExtensionSupportedByPhysicalDevice(uintptr_t game_view_ptr, VkPhysicalDevice device, const char* extensionName) {
        uint32_t extensionCount = 0;
        fn_vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        fn_vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

        for (const VkExtensionProperties& extension : extensions) {
            if (strcmp(extension.extensionName, extensionName) == 0) {
                return true;
            }
        }
        return false;
    }

    bool PlVulkan::isColorSpaceSupportedByPhysicalDevice(uintptr_t game_view_ptr, VkPhysicalDevice device, VkColorSpaceKHR colorSpace)
    {
        uint32_t formatCount = 0;
        fn_vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_surfaces_[game_view_ptr], &formatCount, nullptr);

        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        fn_vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_surfaces_[game_view_ptr], &formatCount, formats.data());

        for (uint32_t i = 0; i < formatCount; i++) {
            if (formats[i].colorSpace == colorSpace) {
                return true;
            }
        }
        return false;
    }

    bool PlVulkan::isSurfacePresentationSupportedByPhysicalDevice(uintptr_t game_view_ptr, VkPhysicalDevice device)
    {
        uint32_t queueFamilyCount = 0;
        fn_vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, nullptr);

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            VkBool32 supported = VK_FALSE;
            if (fn_vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkan_surfaces_[game_view_ptr], &supported) == VK_SUCCESS && supported == VK_TRUE) {
                return true;
            }
        }

        return false;
    }


    void PlVulkan::lockQueue(struct AVHWDeviceContext* dev_ctx, uint32_t queue_family, uint32_t index)
    {
        auto me = (PlVulkan*)dev_ctx->user_opaque;
        me->m_Vulkan->lock_queue(me->m_Vulkan, queue_family, index);
    }

    void PlVulkan::unlockQueue(struct AVHWDeviceContext* dev_ctx, uint32_t queue_family, uint32_t index)
    {
        auto me = (PlVulkan*)dev_ctx->user_opaque;
        me->m_Vulkan->unlock_queue(me->m_Vulkan, queue_family, index);
    }

    bool PlVulkan::populateQueues(int videoFormat)
    {
        auto vkDeviceContext = (AVVulkanDeviceContext*)((AVHWDeviceContext*)m_HwDeviceCtx->data)->hwctx;

        uint32_t queueFamilyCount = 0;
        fn_vkGetPhysicalDeviceQueueFamilyProperties2(m_Vulkan->phys_device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount);
        std::vector<VkQueueFamilyVideoPropertiesKHR> queueFamilyVideoProps(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            queueFamilyVideoProps[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR;
            queueFamilies[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
            queueFamilies[i].pNext = &queueFamilyVideoProps[i];
        }

        fn_vkGetPhysicalDeviceQueueFamilyProperties2(m_Vulkan->phys_device, &queueFamilyCount, queueFamilies.data());

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(59, 34, 100)
        Q_UNUSED(videoFormat);

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            vkDeviceContext->qf[i].idx = i;
            vkDeviceContext->qf[i].num = queueFamilies[i].queueFamilyProperties.queueCount;
            vkDeviceContext->qf[i].flags = (VkQueueFlagBits)queueFamilies[i].queueFamilyProperties.queueFlags;
            vkDeviceContext->qf[i].video_caps = (VkVideoCodecOperationFlagBitsKHR)queueFamilyVideoProps[i].videoCodecOperations;
        }
        vkDeviceContext->nb_qf = queueFamilyCount;
#else
        vkDeviceContext->queue_family_index = m_Vulkan->queue_graphics.index;
        vkDeviceContext->nb_graphics_queues = m_Vulkan->queue_graphics.count;
        vkDeviceContext->queue_family_tx_index = m_Vulkan->queue_transfer.index;
        vkDeviceContext->nb_tx_queues = m_Vulkan->queue_transfer.count;
        vkDeviceContext->queue_family_comp_index = m_Vulkan->queue_compute.index;
        vkDeviceContext->nb_comp_queues = m_Vulkan->queue_compute.count;

        // Select a video decode queue that is capable of decoding our chosen format
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFamilyProperties.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
                if (videoFormat & VIDEO_FORMAT_MASK_H264) {
                    if (queueFamilyVideoProps[i].videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
                        vkDeviceContext->queue_family_decode_index = i;
                        vkDeviceContext->nb_decode_queues = queueFamilies[i].queueFamilyProperties.queueCount;
                        break;
                    }
                }
                else if (videoFormat & VIDEO_FORMAT_MASK_H265) {
                    if (queueFamilyVideoProps[i].videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR) {
                        vkDeviceContext->queue_family_decode_index = i;
                        vkDeviceContext->nb_decode_queues = queueFamilies[i].queueFamilyProperties.queueCount;
                        break;
                    }
                }
                else if (videoFormat & VIDEO_FORMAT_MASK_AV1) {
#if LIBAVCODEC_VERSION_MAJOR >= 61
                    // VK_KHR_video_decode_av1 added VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR to check for AV1
                    // decoding support on this queue. Since FFmpeg 6.1 used the older Mesa-specific AV1 extension,
                    // we'll just assume all video decode queues on this device support AV1 (since we checked that
                    // the physical device supports it earlier.
                    if (queueFamilyVideoProps[i].videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR)
#endif
                    {
                        vkDeviceContext->queue_family_decode_index = i;
                        vkDeviceContext->nb_decode_queues = queueFamilies[i].queueFamilyProperties.queueCount;
                        break;
                    }
                }
                else {
                    SDL_assert(false);
                }
            }
        }

        if (vkDeviceContext->queue_family_decode_index < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "Unable to find compatible video decode queue!");
            return false;
        }
#endif

        return true;
    }

    bool PlVulkan::prepareDecoderContext(AVCodecContext* context, AVDictionary**) {
        if (m_HwAccelBackend) {
            LOGI("Using Vulkan video decoding");
            context->hw_device_ctx = av_buffer_ref(m_HwDeviceCtx);
        }
        else {
            LOGI("Using Vulkan renderer");
        }

        return true;
    }

    bool PlVulkan::RenderFrame(uintptr_t game_view_ptr, AVFrame* frame) {
  
        const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get((AVPixelFormat) frame->format);
        if (!desc) { // 这里需要判断下,因为如果desc是空 会导致在libplacebo库 崩溃
            return false;
        }

        pl_frame mappedFrame;
        pl_frame targetFrame;

        // 1) Map AVFrame -> pl_frame (检查返回)
        if (!mapAvFrameToPlacebo(game_view_ptr, frame, &mappedFrame)) {
            LOGE("mapAvFrameToPlacebo failed");
            return false;
        }

        // 2) Start swapchain frame (必须有，不能跳过)
        pl_swapchain_frame sw_frame;
        if (!pl_swapchain_start_frame(vulkan_swapchains_[game_view_ptr], &sw_frame)) {
            LOGE("pl_swapchain_start_frame failed (window occluded?)");
            pl_unmap_avframe(m_Vulkan->gpu, &mappedFrame);
            return false;
        }

        // 3) Build targetFrame from swapchain frame
        pl_frame_from_swapchain(&targetFrame, &sw_frame);

        // 4) Ensure overlays are empty (you removed overlays); avoid leaving garbage
        targetFrame.num_overlays = 0;
        targetFrame.overlays = nullptr;

        // 6) Render
        if (!pl_render_image(vulkan_renderers_[game_view_ptr], &mappedFrame, &targetFrame, &pl_render_fast_params)) {
            LOGE("pl_render_image() failed");
            // still fallthrough and submit
            return false;
        }

        // 7) Submit and (on Windows) swap buffers
        if (!pl_swapchain_submit_frame(vulkan_swapchains_[game_view_ptr])) {
            LOGE("pl_swapchain_submit_frame() failed");
            // handle recreate if necessary
            return false;
        }

#ifdef Q_OS_WIN32
        pl_swapchain_swap_buffers(vulkan_swapchains_[game_view_ptr]);
#endif
        // 8) Unmap source frame
        pl_unmap_avframe(m_Vulkan->gpu, &mappedFrame);
        return true;
    }

    bool PlVulkan::mapAvFrameToPlacebo(uintptr_t game_view_ptr, const AVFrame* frame, pl_frame* mappedFrame)
    {
        pl_avframe_params mapParams = {};
        mapParams.frame = frame;
        mapParams.tex = textures_[game_view_ptr].data();
        if (!pl_map_avframe_ex(m_Vulkan->gpu, mappedFrame, &mapParams)) {   
            LOGE("pl_map_avframe_ex() failed");
            return false;
        }

        // libplacebo assumes a minimum luminance value of 0 means the actual value was unknown.
        // Since we assume the host values are correct, we use the PL_COLOR_HDR_BLACK constant to
        // indicate infinite contrast.
        //
        // NB: We also have to check that the AVFrame actually had metadata in the first place,
        // because libplacebo may infer metadata if the frame didn't have any.
        if (av_frame_get_side_data(frame, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA) && !mappedFrame->color.hdr.min_luma) {
            mappedFrame->color.hdr.min_luma = PL_COLOR_HDR_BLACK;
        }

        // HACK: AMF AV1 encoding on the host PC does not set full color range properly in the
        // bitstream data, so libplacebo incorrectly renders the content as limited range.
        //
        // As a workaround, set full range manually in the mapped frame ourselves.
        mappedFrame->repr.levels = PL_COLOR_LEVELS_FULL;

        mappedFrame->repr = pl_color_repr_uhdtv;    //  关键代码,支持更宽的色域
        mappedFrame->color = pl_color_space_bt709;  //  关键代码
        return true;
    }

    bool PlVulkan::CreateRenderComponent(uintptr_t game_view_ptr, HWND hwnd) {
        if (!CreateWin32SurfaceFromHwnd(game_view_ptr, hwnd)) {
            LOGE("CreateWin32SurfaceFromHwnd failed");
            return false;
        }

        if (!CreateSwapchain(game_view_ptr)) {
            LOGE("CreateSwapchain failed");
            return false;
        }

        if (!CreatePlRender(game_view_ptr)) {
            LOGE("CreatePlRender failed");
            return false;
        }
        LOGI("CreateRenderComponent success");
        return true;
    }
}