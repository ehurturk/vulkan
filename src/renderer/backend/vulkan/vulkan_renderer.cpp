#include "vulkan_renderer.hpp"
#include <algorithm>
#include <set>
#include "core/logger.hpp"
#include "core/assert.hpp"
#include "../renderer.hpp"
#include "defines.hpp"
#include "platform/window.hpp"

#include <vulkan/vulkan_core.h>

#define VULKAN_CHECK(x)                                                   \
    do {                                                                  \
        VkResult _r = (x);                                                \
        if (_r != VK_SUCCESS) {                                           \
            LOG_FATAL("Vulkan error: VkResult={}", static_cast<int>(_r)); \
            ASSERT(false);                                                \
        }                                                                 \
    } while (0)

namespace Renderer {

static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
              VkDebugUtilsMessageTypeFlagsEXT,
              const VkDebugUtilsMessengerCallbackDataEXT* data,
              void*) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOG_ERROR("[Vulkan] {}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG_WARN("[Vulkan] {}", data->pMessage);
    } else {
        LOG_INFO("[Vulkan] {}", data->pMessage);
    }
    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
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
                                          VkDebugUtilsMessengerEXT debugMessenger,
                                          const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VulkanRenderer::VulkanRenderer(Platform::Window* window)
    : m_Window(window),
      m_vkState(std::make_unique<VkState>()),
      m_ValidationLayers{"VK_LAYER_KHRONOS_validation"},
#ifdef __PLATFORM_MACOS__
      m_DeviceExtensions{"VK_KHR_portability_subset", VK_KHR_SWAPCHAIN_EXTENSION_NAME},
#endif
      m_Device(VK_NULL_HANDLE),
      m_PhysicalDevice(VK_NULL_HANDLE),
      m_GraphicsQueue(VK_NULL_HANDLE),
      m_Surface(VK_NULL_HANDLE) {
    LOG_WARN("VULKAN IS INITIALIZED!");
}

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

void VulkanRenderer::initialize(const RendererConfig& cfg) {
    m_vkState->validation = cfg.enableValidation;
    create_instance();
    if (m_vkState->validation)
        setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_swapchain();

    LOG_INFO("Vulkan instance created.");
}

void VulkanRenderer::shutdown() {
    if (!m_vkState)
        return;
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

    vkDestroyDevice(m_Device, nullptr);

    if (m_vkState->validation && m_vkState->debugMessenger) {
        DestroyDebugUtilsMessengerEXT(m_vkState->instance, m_vkState->debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_vkState->instance, m_Surface, nullptr);

    vkDestroyInstance(m_vkState->instance, nullptr);

    m_vkState->instance = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;
    m_vkState->debugMessenger = VK_NULL_HANDLE;
}

void VulkanRenderer::create_instance() {
    if (m_vkState->validation && !check_validation_layer_support()) {
        LOG_FATAL("[VulkanRenderer]: Validation layers requested but not available!");
        throw std::runtime_error("Validation layers requested but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CC Engine";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.pEngineName = "CC Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();

    ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_vkState->validation) {
        ci.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        ci.ppEnabledLayerNames = m_ValidationLayers.data();

        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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

bool VulkanRenderer::check_validation_layer_support() {
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

void VulkanRenderer::create_surface() {
    m_Surface = m_Window->createSurface(m_vkState->instance);
}

VulkanRenderer::SwapchainSupportDetails VulkanRenderer::querySwapChainSupport(
    VkPhysicalDevice device) {
    SwapchainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    U32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount,
                                             details.formats.data());
    }

    U32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}

void VulkanRenderer::create_swapchain() {
    SwapchainSupportDetails swapchainSup = querySwapChainSupport(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSup.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSup.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSup.capabilities);

    U32 imageCount = swapchainSup.capabilities.minImageCount + 1;

    if (swapchainSup.capabilities.maxImageCount > 0 &&
        imageCount > swapchainSup.capabilities.maxImageCount) {
        imageCount = swapchainSup.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = find_queue_families(m_PhysicalDevice);
    U32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
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

    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;
}

void VulkanRenderer::create_logical_device() {
    QueueFamilyIndices indices = find_queue_families(m_PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<U32> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                         indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (U32 queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
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

    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
}

void VulkanRenderer::pick_physical_device() {
    U32 deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vkState->instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        LOG_FATAL("Failed to find GPUs with Vulkan support!");
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

VulkanRenderer::QueueFamilyIndices VulkanRenderer::find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    U32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.is_complete())
            break;
        i++;
    }

    return indices;
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

// Settle for any GPU
// A physical device is suitable if extensions are supported and has a swapchain support for format
// capabilities and present capabilities.
bool VulkanRenderer::is_physical_device_suitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = find_queue_families(device);
    bool extensionsSupported = check_device_extension_support(device);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport = querySwapChainSupport(device);
        swapChainAdequate =
            !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }
    return indices.is_complete() && extensionsSupported && swapChainAdequate;
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions() {
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

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<U32>::max()) {
        return capabilities.currentExtent;
    }

    const Platform::Window::Extent extent = m_Window->getExtentPixel();

    VkExtent2D ext = {extent.width, extent.height};
    ext.width =
        std::clamp(ext.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    ext.height = std::clamp(ext.height, capabilities.minImageExtent.height,
                            capabilities.maxImageExtent.height);
    return ext;
}

void VulkanRenderer::setup_debug_messenger() {
    VkDebugUtilsMessengerCreateInfoEXT dbg{};
    dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbg.pfnUserCallback = DebugCallback;
    dbg.pNext = nullptr;

    VULKAN_CHECK(CreateDebugUtilsMessengerEXT(m_vkState->instance, &dbg, nullptr,
                                              &m_vkState->debugMessenger));
}

}  // namespace Renderer
