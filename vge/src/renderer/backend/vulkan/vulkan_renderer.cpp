#include "renderer/backend/vulkan/vulkan_renderer.hpp"
#include <algorithm>
#include <numeric>
#include <set>
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "core/logger.hpp"
#include "core/assert.hpp"
#include "renderer/backend/renderer.hpp"
#include "renderer/backend/shader_loader.hpp"
#include "renderer/backend/vulkan/vulkan_utils.hpp"
#include "defines.hpp"
#include "platform/window/window.hpp"
#include "scene/camera.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan_core.h>

#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../extern/stb_image.h"

#define VULKAN_CHECK(x)                                                                            \
    do {                                                                                           \
        VkResult _r = (x);                                                                         \
        if (_r != VK_SUCCESS) {                                                                    \
            CORE_LOG_FATAL("Vulkan error: VkResult={}", static_cast<int>(_r));                     \
            ASSERT(false);                                                                         \
        }                                                                                          \
    } while (0)

namespace Renderer::Vulkan {

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*) {
    const char* msg = data->pMessage;
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        CORE_LOG_ERROR("[Vulkan] {}", msg);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        CORE_LOG_WARN("[Vulkan] {}", msg);
    } else {
        CORE_LOG_INFO("[Vulkan] {}", msg);
    }
    (void)msg;
    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VkSurfaceFormatKHR choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

// Choose a VkPresentModeKHR to base a swapchain's present mode to.
// Settle for a VK_PRESENT_MODE_KHR (triple-buffering without hard vsync).
// If no available present modes support VK_PRESENT_MODE_MAILBOX_KHR, settle
// for a VK_PRESENT_MODE_FIFO_KHR (strong vsync present mode).
static VkPresentModeKHR choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& availablePresentModes,
    const VkPresentModeKHR preferred_present_mode = VK_PRESENT_MODE_MAILBOX_KHR) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == preferred_present_mode)
            return availablePresentMode;
    }

    CORE_LOG_WARN("No available present modes support VK_PRESENT_MODE_MAILBOX_KHR, falling back to "
                  "{}!",
        vkb::to_string(availablePresentModes[0]));

    return availablePresentModes[0];
}

[[maybe_unused]] static bool has_stencil_component(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VulkanRenderer::VulkanRenderer(Platform::Window* window)
    : m_Window(window)
    , m_CurrentFrame(0)
    , m_vkState(std::make_unique<VkState>())
    , m_ValidationLayers { "VK_LAYER_KHRONOS_validation" }
    ,
#ifdef __PLATFORM_MACOS__
    m_DeviceExtensions { "VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME }
    ,
#endif
    m_SwapchainImageFormat()
    , m_SwapchainExtent()
    , m_Device(VK_NULL_HANDLE)
    , m_PhysicalDevice(VK_NULL_HANDLE)
    , m_Allocator {}
    , m_GraphicsQueue(VK_NULL_HANDLE)
    , m_PresentQueue(VK_NULL_HANDLE)
    , m_Surface(VK_NULL_HANDLE)
    , m_Swapchain(VK_NULL_HANDLE)
    , m_RenderPass(VK_NULL_HANDLE)
    , m_PipelineLayout(VK_NULL_HANDLE)
    , m_GraphicsPipeline(VK_NULL_HANDLE)
    , m_DescriptorSetLayout(VK_NULL_HANDLE)
    , m_CommandPool(VK_NULL_HANDLE)
    , m_CommandBuffers {}
    , m_VertexBuffer(VK_NULL_HANDLE)
    , m_VertexBufferMemory(VK_NULL_HANDLE)
    , m_IndexBuffer(VK_NULL_HANDLE)
    , m_IndexBufferMemory(VK_NULL_HANDLE)
    , m_DescriptorPool(VK_NULL_HANDLE)
    , m_TextureSampler(VK_NULL_HANDLE)
    , m_TextureImage(VK_NULL_HANDLE)
    , m_TextureImageMemory(VK_NULL_HANDLE)
    , m_TextureImageView(VK_NULL_HANDLE)
    , m_DepthImage(VK_NULL_HANDLE)
    , m_DepthImageAllocation {}
    , m_DepthImageMemory(VK_NULL_HANDLE)
    , m_DepthImageView(VK_NULL_HANDLE)
    , m_InFlightFences {} {
    CORE_LOG_INFO("VULKAN IS INITIALIZED!");
}

VulkanRenderer::~VulkanRenderer() {
    // FIXME: Leak! call shutdown (but avoid double freeing in shutdown)
}

void VulkanRenderer::initialize(const RendererConfig& cfg) {
    m_vkState->validation = cfg.enableValidation;
    create_instance();
    if (m_vkState->validation)
        setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_memory_allocator();
    create_swapchain();
    create_swapchain_image_views();
    create_render_pass();
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_commandpool();
    create_depth_resources();
    create_framebuffers();
    create_texture_sampler();
    load_model(MODEL_PATH);
    // load_model("../../../../assets/models/DamagedHelmet/DamagedHelmet.gltf");
    // load_model("../../../../assets/models/rungholt/rungholt.obj");
    create_vertex_buffer();
    create_index_buffer();
    setup_game_objects();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffers();
    create_sync_objects();

    CORE_LOG_INFO("Vulkan instance created.");
}

void VulkanRenderer::shutdown() {
    if (!m_vkState || !m_Device || !m_PipelineLayout || !m_Swapchain || !m_Surface)
        return;

    vkDeviceWaitIdle(m_Device);

    cleanup_swapchain();

    vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

    for (auto& game_object : m_GameObjects) {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(m_Device, game_object.uniformBuffers[i], nullptr);
            vkFreeMemory(m_Device, game_object.uniformBufferMemories[i], nullptr);
        }
    }

    // Descriptor sets are automatically destroyed when the descriptor pool is destroyed
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);

    vkDestroyImageView(m_Device, m_TextureImageView, nullptr);
    vkDestroyImage(m_Device, m_TextureImage, nullptr);
    vkFreeMemory(m_Device, m_TextureImageMemory, nullptr);

    for (auto& material : m_LoadedModel.materials) {
        if (material.diffuseTextureView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_Device, material.diffuseTextureView, nullptr);
        }
        if (material.diffuseTexture != VK_NULL_HANDLE) {
            vkDestroyImage(m_Device, material.diffuseTexture, nullptr);
        }
        if (material.diffuseTextureMemory != VK_NULL_HANDLE) {
            vkFreeMemory(m_Device, material.diffuseTextureMemory, nullptr);
        }
    }

    vkDestroySampler(m_Device, m_TextureSampler, nullptr);

    vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);

    vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
    vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);

    vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

    for (int i = 0; i < static_cast<int>(m_SwapchainImages.size()); i++) {
        vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

    vmaDestroyAllocator(m_Allocator);
    vkDestroyDevice(m_Device, nullptr);

    if (m_vkState->validation && m_vkState->debugMessenger) {
        DestroyDebugUtilsMessengerEXT(m_vkState->instance, m_vkState->debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_vkState->instance, m_Surface, nullptr);
    vkDestroyInstance(m_vkState->instance, nullptr);
}

void VulkanRenderer::draw_frame(RenderContext context) {
    // Outline of a frame:
    // 1) wait for the previous frame to finish
    // 2) Acquire an image from the swapchain
    // 3) Record command buffer that draws the scene onto the image
    // 4) Submit the recorded command buffer
    // 5) Present the swapchain image

    m_ImageAvailableSemaphores.resize(m_SwapchainImages.size());
    m_RenderFinishedSemaphores.resize(m_SwapchainImages.size());

    // wait for the frame's fence (i.e. wait until the rendered image is submitted to the graphics
    // queue)
    vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

    U32 imageIdx;
    vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX,
        m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIdx);

    // Remove all previous commands from the command buffer
    vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    // Record the draw commands to the current frame's command buffer for the image imageIdx
    record_draw_commands(m_CommandBuffers[m_CurrentFrame], imageIdx);

    update_uniform_buffer(m_CurrentFrame, context);

    // block until this is ready
    std::array<VkSemaphore, 1> waitSemaphores { m_ImageAvailableSemaphores[m_CurrentFrame] };
    // signal when render finished
    std::array<VkSemaphore, 1> signalSemaphores { m_RenderFinishedSemaphores[imageIdx] };
    // Wait in the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT pipeline stage
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1; // recorded only 1 command buffer
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

    // Submit the command buffer to the graphics queue
    // Signal fence when the draw commands in the command buffer are executed.
    // This provides CPU-GPU synchronization.
    VULKAN_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]));

    // Now put the image rendered into the visible window (e.g. present it)
    // wait on the signalSemaphore for this, as the draw commands MUST finish
    // before the image is presented into the swapchain.
    std::array<VkSwapchainKHR, 1> swapchains { m_Swapchain };
    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = signalSemaphores.size();
    presentInfo.pWaitSemaphores = signalSemaphores.data();
    presentInfo.swapchainCount = swapchains.size();
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = &imageIdx;
    presentInfo.pResults = nullptr;
    presentInfo.pNext = nullptr;

    // Submit the present information to the presentation queue
    vkQueuePresentKHR(m_PresentQueue, &presentInfo);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::create_instance() {
    if (m_vkState->validation && !check_validation_layer_support()) {
        CORE_LOG_FATAL("[VulkanRenderer]: Validation layers requested but not available!");
        throw std::runtime_error("Validation layers requested but not available!");
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CC Engine";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.pEngineName = "CC Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();

    ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (m_vkState->validation) {
        ci.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        ci.ppEnabledLayerNames = m_ValidationLayers.data();

        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        ci.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    } else {
        ci.enabledLayerCount = 0;
        ci.pNext = nullptr;
    }

#ifdef __PLATFORM_MACOS__
    ci.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VULKAN_CHECK(vkCreateInstance(&ci, nullptr, &m_vkState->instance));
}

void VulkanRenderer::setup_debug_messenger() {
    VkDebugUtilsMessengerCreateInfoEXT dbg {};
    dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbg.pfnUserCallback = DebugCallback;
    dbg.pNext = nullptr;

    VULKAN_CHECK(CreateDebugUtilsMessengerEXT(
        m_vkState->instance, &dbg, nullptr, &m_vkState->debugMessenger));
}

void VulkanRenderer::create_surface() { m_Surface = m_Window->createSurface(m_vkState->instance); }

void VulkanRenderer::pick_physical_device() {
    U32 deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vkState->instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        CORE_LOG_FATAL("Failed to find GPUs with Vulkan support!");
        ASSERT(false);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vkState->instance, &deviceCount, devices.data());

    // Select the first physical device
    for (const auto& device : devices) {
        if (is_physical_device_suitable(device)) {
            m_PhysicalDevice = device;
            break;
        }
    }

    ASSERT_MSG(m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU!");
}

void VulkanRenderer::create_logical_device() {
    auto [graphicsFamily, presentFamily] = find_queue_families(m_PhysicalDevice);

    std::set<U32> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos {};
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    float queuePriority = 1.0f;
    for (U32 queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<U32>(m_DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

    if (m_vkState->validation) {
        createInfo.enabledLayerCount = static_cast<U32>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VULKAN_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

    vkGetDeviceQueue(m_Device, graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, presentFamily.value(), 0, &m_PresentQueue);
}

void VulkanRenderer::create_memory_allocator() {
    VmaVulkanFunctions vkFunctions {};
    vkFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vkFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
    vkFunctions.vkCreateImage = &vkCreateImage;

    VmaAllocatorCreateInfo createInfo {};
    createInfo.instance = m_vkState->instance;
    createInfo.device = m_Device;
    createInfo.physicalDevice = m_PhysicalDevice;
    createInfo.vulkanApiVersion = VK_API_VERSION_1_2; // TODO: store the api version somewhere.
    createInfo.pVulkanFunctions = &vkFunctions;

    VULKAN_CHECK(vmaCreateAllocator(&createInfo, &m_Allocator));
}

void VulkanRenderer::create_swapchain() {
    SwapchainSupportDetails swapchainSup = query_swap_chain_support(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapchainSup.formats);
    VkPresentModeKHR presentMode = choose_swap_present_mode(swapchainSup.presentModes);
    VkExtent2D extent = choose_swap_extent(swapchainSup.capabilities);

    U32 imageCount = swapchainSup.capabilities.minImageCount + 1;

    if (swapchainSup.capabilities.maxImageCount > 0
        && imageCount > swapchainSup.capabilities.maxImageCount) {
        imageCount = swapchainSup.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices queueIndices = find_queue_families(m_PhysicalDevice);
    U32 queueFamilyIndices[]
        = { queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() };

    if (queueIndices.graphicsFamily != queueIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapchainSup.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VULKAN_CHECK(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain));

    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
    m_SwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data());
    CORE_LOG_INFO("Swapchain created with {} images", imageCount);

    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;
}

void VulkanRenderer::create_swapchain_image_views() {
    m_SwapchainImageViews.resize(m_SwapchainImages.size());
    for (size_t i = 0; i < m_SwapchainImages.size(); i++) {
        m_SwapchainImageViews[i] = create_image_view(
            m_SwapchainImages[i], m_SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

// Creates a descriptor set layout object.
// Initializes bindings for the descriptor
// set layout.
void VulkanRenderer::create_descriptor_set_layout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const std::array<VkDescriptorSetLayoutBinding, 2> descriptor_set_layout_bindings {
        uboLayoutBinding, samplerLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo ci {};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount = static_cast<U32>(descriptor_set_layout_bindings.size());
    ci.pBindings = descriptor_set_layout_bindings.data();

    VULKAN_CHECK(vkCreateDescriptorSetLayout(m_Device, &ci, nullptr, &m_DescriptorSetLayout));
}

void VulkanRenderer::create_graphics_pipeline() {
    // TODO: abstract away the filepath thing, especially for resources such as shaders/textures.
    auto shader = ShaderLoader::read_file("../../../../assets/shaders/triangle.spv");

    // Shader modules && stage creations
    VkShaderModule shaderModule = create_shader_module(shader);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = shaderModule;
    vertShaderStageInfo.pName = "vertMain";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = shaderModule;
    fragShaderStageInfo.pName = "fragMain";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // Fixed functions:

    // Vertex Input
    auto bindingDescription = Vertex::get_binding_description();
    auto attributeDescriptions = Vertex::get_attribute_description();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount
        = static_cast<U32>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports & Scissors
    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth and Stencil Testing
    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    // Color Blending
    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates
        = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Pipeline Layout
    // We specify that the pipeline layout has a SINGLE Descriptor Set Layout.
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    VULKAN_CHECK(
        vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_PipelineLayout;
    pipelineInfo.renderPass = m_RenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VULKAN_CHECK(vkCreateGraphicsPipelines(
        m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));

    vkDestroyShaderModule(m_Device, shaderModule, nullptr);
}

void VulkanRenderer::create_render_pass() {
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = m_SwapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // final image layout is going to be used for presenting
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = find_depth_format();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // final image layout is going to be used for storing depth and stencil information
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency subpassDependency {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask
        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.dstAccessMask
        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDependency;

    VULKAN_CHECK(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass));
}

void VulkanRenderer::create_framebuffers() {
    m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());
    for (size_t i = 0; i < m_SwapchainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = { m_SwapchainImageViews[i], m_DepthImageView };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<U32>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_SwapchainExtent.width;
        framebufferInfo.height = m_SwapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i])
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void VulkanRenderer::create_commandpool() {
    QueueFamilyIndices queueFamilyIndices = find_queue_families(m_PhysicalDevice);

    VkCommandPoolCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.pNext = nullptr;
    // Record the commands for graphics, thus use the graphics queue family
    createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VULKAN_CHECK(vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool));
}

void VulkanRenderer::create_depth_resources() {
    VkFormat depthFormat = find_depth_format();

    // create_image(m_SwapchainExtent.width, m_SwapchainExtent.height, 1, depthFormat,
    //    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
    VmaAllocationCreateInfo allocCI {};
    allocCI.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    allocCI.usage = VMA_MEMORY_USAGE_AUTO;

    VkImageCreateInfo depthImageCI {};
    depthImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageCI.imageType = VK_IMAGE_TYPE_2D;
    depthImageCI.arrayLayers = 1;
    depthImageCI.mipLevels = 1;
    depthImageCI.extent.depth = 1;
    depthImageCI.extent.width = m_SwapchainExtent.width;
    depthImageCI.extent.height = m_SwapchainExtent.height;
    depthImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageCI.flags = 0;
    depthImageCI.format = depthFormat;
    depthImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vmaCreateImage(
        m_Allocator, &depthImageCI, &allocCI, &m_DepthImage, &m_DepthImageAllocation, nullptr);

    m_DepthImageView = create_image_view(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void VulkanRenderer::create_texture_sampler() {
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    VULKAN_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_TextureSampler));
}

void VulkanRenderer::process_node(aiNode* node, const aiScene* scene) {
    m_LoadedModel.meshes.reserve(node->mNumMeshes);

    for (size_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_LoadedModel.meshes.push_back(process_mesh(mesh, scene));
    }

    for (size_t i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}

Mesh VulkanRenderer::process_mesh(aiMesh* mesh, const aiScene* scene) {
    Mesh result {};
    std::unordered_map<Vertex, U32> uniqueVertices {};

    result.vertices.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; i++) {
        result.vertices.emplace_back(
            glm::vec3 { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z },
            mesh->mColors[0]
                ? glm::vec3 { mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b }
                : glm::vec3 { 1.0f, 1.0f, 1.0f },
            mesh->mTextureCoords[0]
                ? glm::vec2 { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }
                : glm::vec2 { 0.0f, 0.0f });
    }

    // TODO: reserve for here?
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        result.indices.insert(
            result.indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
    }

    result.materialIndex = mesh->mMaterialIndex;

    return result;
}

void VulkanRenderer::process_materials(const aiScene* scene) {
    m_LoadedModel.materials.resize(scene->mNumMaterials);

    for (size_t i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        Material& mat = m_LoadedModel.materials[i];

        mat.materialIndex = i;

        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        mat.name = name.C_Str();

        CORE_LOG_INFO("[VulkanRenderer]: Processing material {}: {}", i, mat.name);

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString texturePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);

            std::string fullPath = m_LoadedModel.directory + "/" + texturePath.C_Str();

            if (size_t fchar_idx = fullPath.find("\\"); fchar_idx != std::string::npos) {
                fullPath.replace(
                    fullPath.begin() + fchar_idx, fullPath.begin() + fchar_idx + 1, "/");
            }

            CORE_LOG_INFO("[VulkanRenderer]:\tLoading diffuse texture: {}", fullPath);

            load_texture(
                fullPath, mat.diffuseTexture, mat.diffuseTextureMemory, mat.diffuseTextureView);
        } else {
            CORE_LOG_WARN("[VulkanRenderer]:\tNo diffuse texture found for material {}", mat.name);
            create_default_texture(
                mat.diffuseTexture, mat.diffuseTextureMemory, mat.diffuseTextureView);
        }
    }
}

void VulkanRenderer::create_default_texture(
    VkImage& image, VkDeviceMemory& imageMemory, VkImageView& imageView) {
    const int texWidth = 1;
    const int texHeight = 1;
    unsigned char pixels[4] = { 255, 255, 255, 255 }; // White

    VkDeviceSize imageSize = 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, 4);
    vkUnmapMemory(m_Device, stagingBufferMemory);

    create_image(texWidth, texHeight, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

    transition_image_layout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

    copy_buffer_to_image(stagingBuffer, image, texWidth, texHeight);

    transition_image_layout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

    imageView = create_image_view(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VulkanRenderer::load_texture(const std::string& path, VkImage& textureImage,
    VkDeviceMemory& textureMemory, VkImageView& textureView) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels) {
        CORE_LOG_WARN(
            "[VulkanRenderer]: Failed to load texture: {}. Creating a default texture...", path);
        create_default_texture(textureImage, textureMemory, textureView);
        return;
    }

    U32 mipLevels = static_cast<U32>(glm::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
        stagingBufferMemory);

    // Copy pixel data to staging buffer
    void* data;
    vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_Device, stagingBufferMemory);

    stbi_image_free(pixels);

    // Create the actual image in device local memory
    // Image has an initial layout of UNDEFINED, therefore its layout has to be transitioned to a
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout.
    create_image(texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureMemory);

    // Transition image layout and copy from staging buffer
    transition_image_layout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

    copy_buffer_to_image(
        stagingBuffer, textureImage, static_cast<U32>(texWidth), static_cast<U32>(texHeight));

    // transition_image_layout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
    //                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

    // Cleanup staging buffer
    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

    generate_mipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

    // Create image view
    textureView = create_image_view(
        textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

void VulkanRenderer::generate_mipmaps(
    VkImage image, VkFormat imageFormat, U32 width, U32 height, U32 mipLevels) {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures
            & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        CORE_LOG_FATAL("[VulkanRenderer]: Texture image format does not support linear blitting!");
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    I32 mipWidth = width;
    I32 mipHeight = height;

    for (U32 i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1]
            = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    end_single_time_commands(commandBuffer);
}

void VulkanRenderer::load_model(std::string_view path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.data(),
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals
            | aiProcess_JoinIdenticalVertices | aiProcess_MakeLeftHanded
            | aiProcess_FlipWindingOrder);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        CORE_LOG_FATAL(
            "[VulkanRenderer::load_model()]: Assimp error: {}", importer.GetErrorString());
        return;
    }

    m_LoadedModel.directory = path.substr(0, path.find_last_of('/'));
    CORE_LOG_INFO("[VulkanRenderer]: Loading model: {}", path);
    CORE_LOG_INFO("[VulkanRenderer]: Model has {} materials", scene->mNumMaterials);
    CORE_LOG_INFO("[VulkanRenderer]: Model has {} meshes", scene->mNumMeshes);
    CORE_LOG_INFO("[VulkanRenderer]: Model has {} textures", scene->mNumTextures);

    process_materials(scene);

    process_node(scene->mRootNode, scene);

    CORE_LOG_INFO("[VulkanRenderer]: Model loaded successfully");
    CORE_LOG_INFO("[VulkanRenderer]: Total Vertices: {}",
        std::accumulate(m_LoadedModel.meshes.begin(), m_LoadedModel.meshes.end(), 0,
            [](int sum, const Mesh& m) { return sum + m.vertices.size(); }));
}

void VulkanRenderer::create_vertex_buffer() {
    size_t total_vertices
        = std::accumulate(m_LoadedModel.meshes.begin(), m_LoadedModel.meshes.end(), 0,
            [](int sum, const Mesh& m) { return sum + m.vertices.size(); });

    VkDeviceSize bufferSize = sizeof(Vertex) * total_vertices;

    if (bufferSize == 0) {
        CORE_LOG_ERROR("[VulkanRenderer]: No vertices to create buffer for.");
        throw std::runtime_error("No vertices to create buffer for.");
    }

    // Create a staging buffer to use it as a source in memory transfer operation
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer,
        stagingBufferMemory);

    void* data;

    vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    VkDeviceSize currentOffset = 0;
    for (auto& mesh : m_LoadedModel.meshes) {
        // Store offset information in the mesh
        mesh.vertexOffset = currentOffset;
        mesh.vertexCount = static_cast<U32>(mesh.vertices.size());
        mesh.vertexOffsetIndex = static_cast<I32>(currentOffset / sizeof(Vertex));

        // Copy this mesh's vertices
        size_t meshSize = mesh.vertices.size() * sizeof(Vertex);
        memcpy(static_cast<char*>(data) + currentOffset, mesh.vertices.data(), meshSize);

        currentOffset += meshSize;
    }
    vkUnmapMemory(m_Device, stagingBufferMemory);

    // Make the vertex buffer a transfer destination for the memory transfer
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

    copy_buffer(stagingBuffer, m_VertexBuffer, bufferSize);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::create_index_buffer() {
    size_t total_indices = std::accumulate(m_LoadedModel.meshes.begin(), m_LoadedModel.meshes.end(),
        0, [](int sum, const Mesh& m) { return sum + m.indices.size(); });

    VkDeviceSize bufferSize = sizeof(U32) * total_indices;

    if (bufferSize == 0) {
        CORE_LOG_ERROR("[VulkanRenderer]: No indices to create buffer for!");
        throw std::runtime_error("No indices to create buffer for!");
    }

    // Create a staging buffer to use it as a source in memory transfer operation
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer,
        stagingBufferMemory);

    void* data;

    vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    VkDeviceSize currentOffset = 0;
    for (auto& mesh : m_LoadedModel.meshes) {
        // Store offset information in the mesh
        mesh.indexOffset = currentOffset;
        mesh.indexCount = static_cast<U32>(mesh.indices.size());
        mesh.firstIndex = static_cast<U32>(currentOffset / sizeof(U32));

        // Copy this mesh's indices
        size_t meshSize = mesh.indices.size() * sizeof(U32);
        memcpy(static_cast<char*>(data) + currentOffset, mesh.indices.data(), meshSize);

        currentOffset += meshSize;
    }

    vkUnmapMemory(m_Device, stagingBufferMemory);

    // Make the vertex buffer a transfer destination for the memory transfer
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

    copy_buffer(stagingBuffer, m_IndexBuffer, bufferSize);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::setup_game_objects() {
    for (size_t i = 0; i < m_LoadedModel.meshes.size(); i++) {
        GameObject obj {};
        obj.meshIndex = static_cast<U32>(i);
        obj.materialIndex = m_LoadedModel.meshes[i].materialIndex;
        obj.position = { 0.0f, 0.0f, 0.0f };
        obj.rotation = { 0.0f, 0.0f, 0.0f };
        obj.scale = { 1.0f, 1.0f, 1.0f };

        // TODO: convert this to emplace_back
        m_GameObjects.push_back(obj);
    }

    CORE_LOG_INFO("[VulkanRenderer]: Created {} game objects from model", m_GameObjects.size());
}

void VulkanRenderer::create_uniform_buffers() {
    for (auto& game_object : m_GameObjects) {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);

            create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                game_object.uniformBuffers[i], game_object.uniformBufferMemories[i]);

            vkMapMemory(m_Device, game_object.uniformBufferMemories[i], 0, bufferSize, 0,
                &game_object.uniformBuffersMapped[i]);
        }
    }
}

// TODO: what happens when we add more game objects? how do we resize the descriptor pool?
void VulkanRenderer::create_descriptor_pool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<U32>(m_GameObjects.size() * MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<U32>(m_GameObjects.size() * MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // ?
    poolInfo.poolSizeCount = static_cast<U32>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<U32>(m_GameObjects.size() * MAX_FRAMES_IN_FLIGHT);

    VULKAN_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool));
}

// Create descriptor sets for each MAX_FRAMES_IN_FLIGHT.
void VulkanRenderer::create_descriptor_sets() {
    for (auto& game_object : m_GameObjects) {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

        // Allocate descriptor sets from the descriptor pool (m_DescriptorPool).
        // We will have MAX_FRAMES_IN_FLIGHT number of descriptor sets (a descriptor
        // set belongs to each frame).
        VkDescriptorSetAllocateInfo allocate_info {};
        allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool = m_DescriptorPool;
        allocate_info.descriptorSetCount = static_cast<U32>(layouts.size());
        allocate_info.pSetLayouts = layouts.data();

        VULKAN_CHECK(
            vkAllocateDescriptorSets(m_Device, &allocate_info, game_object.descriptorSets.data()));

        // Populate every descriptor for our uniform buffers
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            // References the actual Uniform Buffer (VkBuffer) we are using.
            // Offset 0 means read from the beginning of the buffer.
            // This actually gives the buffer information a descriptor set needs.
            VkDescriptorBufferInfo bufferInfo {};
            bufferInfo.buffer = game_object.uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            const Material& material = m_LoadedModel.materials[game_object.materialIndex];

            VkDescriptorImageInfo imageInfo {};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = material.diffuseTextureView;
            imageInfo.sampler = m_TextureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites {};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = game_object.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = game_object.descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_Device, static_cast<U32>(descriptorWrites.size()),
                descriptorWrites.data(), 0, nullptr);
        }
    }
}

void VulkanRenderer::create_command_buffers() {
    VkCommandBufferAllocateInfo allocateInfo {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_CommandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandBufferCount = m_CommandBuffers.size();

    VULKAN_CHECK(vkAllocateCommandBuffers(m_Device, &allocateInfo, m_CommandBuffers.data()));
}

void VulkanRenderer::create_sync_objects() {
    m_ImageAvailableSemaphores.resize(m_SwapchainImages.size());
    m_RenderFinishedSemaphores.resize(m_SwapchainImages.size());

    VkSemaphoreCreateInfo semaCI {};
    semaCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCI {};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VULKAN_CHECK(vkCreateFence(m_Device, &fenceCI, nullptr, &m_InFlightFences[i]));
    }

    for (int i = 0; i < (int)m_SwapchainImages.size(); i++) {
        VULKAN_CHECK(vkCreateSemaphore(m_Device, &semaCI, nullptr, &m_ImageAvailableSemaphores[i]));
        VULKAN_CHECK(vkCreateSemaphore(m_Device, &semaCI, nullptr, &m_RenderFinishedSemaphores[i]));
    }
}

VkShaderModule VulkanRenderer::create_shader_module(const std::vector<char>& code) const {
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(char);
    createInfo.pCode = reinterpret_cast<const U32*>(code.data());

    VkShaderModule shaderModule;
    VULKAN_CHECK(vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

void VulkanRenderer::cleanup_swapchain() {
    vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
    // vkDestroyImage(m_Device, m_DepthImage, nullptr);
    // vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);

    vmaDestroyImage(m_Allocator, m_DepthImage, m_DepthImageAllocation);

    for (const auto& framebuffer : m_SwapchainFramebuffers) {
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    }

    for (auto imageView : m_SwapchainImageViews) {
        vkDestroyImageView(m_Device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
}

void VulkanRenderer::recreate_swapchain() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        m_Window->waitForEvents();
        auto [frame_width, frame_height] = m_Window->getFramebufferSize();
        width = frame_width;
        height = frame_height;
    }

    vkDeviceWaitIdle(m_Device);

    create_swapchain();
    create_swapchain_image_views();
    create_depth_resources();
    create_framebuffers();
}

// Record a command buffer for image id IMAGE_IDX to be drawn
void VulkanRenderer::record_draw_commands(VkCommandBuffer commandBuffer, U32 image_idx) const {
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    // Begin writing to the command buffer
    VULKAN_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    {
        // Initialize Info for Render Pass

        // NOTE: order of clear values should be the same as order of attachments.
        std::array<VkClearValue, 2> clearValues {};
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_RenderPass;
        renderPassBeginInfo.framebuffer = m_SwapchainFramebuffers[image_idx];
        // render area offest & extent:
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = m_SwapchainExtent;
        // background clear color:
        renderPassBeginInfo.clearValueCount = static_cast<U32>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        // Begin Render Pass:
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 1) Bind graphics pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

        // 2) Bind vertex & index buffers (shared by all objects)
        VkBuffer vertexBuffers[] = { m_VertexBuffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

        // 3) Submit Viewport Details
        VkViewport viewport {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapchainExtent.width);
        viewport.height = static_cast<float>(m_SwapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // 4) Set Scissor
        VkRect2D scissor {};
        scissor.offset = { 0, 0 };
        scissor.extent = m_SwapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // 5) Bind Descriptor Set
        // NOTE: descriptor sets are not unique to graphics pipelines:
        //    - We have to specify whether we bind descriptors to graphics or compute pipeline
        // This will bind the current descriptor set for the current frame (as we have
        // MAX_FRAMES_IN_FLIGHT) amount of descriptor sets.
        // Draw each object with its own descriptor set
        for (const auto& game_object : m_GameObjects) {
            const Mesh& mesh = m_LoadedModel.meshes[game_object.meshIndex];

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_PipelineLayout, 0, 1, &game_object.descriptorSets[m_CurrentFrame], 0, nullptr);

            // 6) Draw Indexed
            vkCmdDrawIndexed(
                commandBuffer, mesh.indexCount, 1, mesh.firstIndex, mesh.vertexOffsetIndex, 0);
        }
        // 7) End Render Pass
        vkCmdEndRenderPass(commandBuffer);
    }

    // End recording of the command buffer
    VULKAN_CHECK(vkEndCommandBuffer(commandBuffer));
}

void VulkanRenderer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const {
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.flags = 0;

    VmaAllocationCreateInfo allocationInfo {};
    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VULKAN_CHECK(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memReqs {};
    vkGetBufferMemoryRequirements(m_Device, buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = find_memory_type(memReqs.memoryTypeBits, properties);

    VULKAN_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory));

    vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
}

VkCommandBuffer VulkanRenderer::begin_single_time_commands() {
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanRenderer::end_single_time_commands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue);

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void VulkanRenderer::copy_buffer(VkBuffer src, VkBuffer dest, VkDeviceSize size) {
    // Memory transfer operations are executed using command buffers - like drawing commands
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkBufferCopy copyRegion {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);

    end_single_time_commands(commandBuffer);
}

void VulkanRenderer::update_uniform_buffer(U32 imageIdx, RenderContext context) {
    glm::mat4 view = context.camera->get_view_matrix();
    glm::mat4 proj
        = context.camera->get_projection_matrix(m_SwapchainExtent.width, m_SwapchainExtent.height);

    proj[1][1] *= -1; // flip Y position for Vulkan (TODO)

    for (auto& gameObject : m_GameObjects) {
        glm::mat4 model = gameObject.get_model_matrix();

        UniformBufferObject ubo { .model = model, .view = view, .proj = proj };

        // Copy the UBO data to the mapped memory
        memcpy(gameObject.uniformBuffersMapped[m_CurrentFrame], &ubo, sizeof(ubo));
    }
}

VkImageView VulkanRenderer::create_image_view(
    VkImage image, VkFormat format, VkImageAspectFlags flags, U32 mipLevels) {
    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = flags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

void VulkanRenderer::create_image(U32 width, U32 height, U32 mipLevels, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
    VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    // image is going to only be used by one
    // queue family (graphics + transfer)
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VULKAN_CHECK(vkCreateImage(m_Device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_Device, image, imageMemory, 0);
}

void VulkanRenderer::transition_image_layout(VkImage image, VkFormat format,
    VkImageLayout oldLayout, VkImageLayout newLayout, U32 mipLevels) {
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    // We are not using the barrier to transfer queue family ownership, so make queue families
    // be ignored.
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    end_single_time_commands(commandBuffer);
}

void VulkanRenderer::copy_buffer_to_image(VkBuffer buffer, VkImage image, U32 width, U32 height) {
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(
        commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    end_single_time_commands(commandBuffer);
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions() const {
    auto extensions = m_Window->getRequiredInstanceExtensions();

#ifdef __PLATFORM_MACOS__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

    if (m_vkState->validation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

// Settle for any GPU
// A physical device is suitable if extensions are supported and has a swapchain support for
// format capabilities and present capabilities.
bool VulkanRenderer::is_physical_device_suitable(VkPhysicalDevice device) {
    QueueFamilyIndices queueIndices = find_queue_families(device);
    bool extensionsSupported = check_device_extension_support(device);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport = query_swap_chain_support(device);
        swapChainAdequate
            = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return queueIndices.is_complete() && extensionsSupported && swapChainAdequate
        && supportedFeatures.samplerAnisotropy;
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices queueIndices;

    U32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueIndices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

        if (presentSupport) {
            queueIndices.presentFamily = i;
        }

        if (queueIndices.is_complete())
            break;
        i++;
    }

    return queueIndices;
}

bool VulkanRenderer::check_device_extension_support(VkPhysicalDevice device) {
    U32 extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_exts(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_exts.data());

    std::set<std::string> required_exts(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

    for (const auto& ext : available_exts) {
        required_exts.erase(ext.extensionName);
    }

    return required_exts.empty();
}

VulkanRenderer::SwapchainSupportDetails VulkanRenderer::query_swap_chain_support(
    VkPhysicalDevice device) const {
    SwapchainSupportDetails details {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    U32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, m_Surface, &formatCount, details.formats.data());
    }

    U32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, m_Surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkExtent2D VulkanRenderer::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<U32>::max()) {
        return capabilities.currentExtent;
    }

    const Platform::Window::Extent extent = m_Window->getExtentPixel();

    VkExtent2D ext = { extent.width, extent.height };
    ext.width = std::clamp(
        ext.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    ext.height = std::clamp(
        ext.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return ext;
}

bool VulkanRenderer::check_validation_layer_support() const {
    U32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layer_name : m_ValidationLayers) {
        bool layerFound = false;

        for (const auto& layerprops : availableLayers) {
            if (strcmp(layer_name, layerprops.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }
    return true;
}

// TODO: move down
U32 VulkanRenderer::find_memory_type(U32 typeFilter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProps {};
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProps);

    for (U32 i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i))
            && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("ERR: Failed to find suitable memory type!");
}

VkFormat VulkanRenderer::find_supported_format(std::span<const VkFormat> candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features) const {
    for (auto& format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR
            && (props.linearTilingFeatures & features) == features) {
            return format;
        }

        if (tiling == VK_IMAGE_TILING_OPTIMAL
            && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    CORE_LOG_FATAL("[VulkanRenderer]: Failed to find a supported format!");
    throw std::runtime_error("Failed to find a supported format!");
}

VkFormat VulkanRenderer::find_depth_format() const {
    constexpr std::array<VkFormat, 3> formats
        = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    return find_supported_format(
        formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace Renderer::Vulkan
