#include "renderer/backend/vulkan/vulkan_context.hpp"
#include "renderer/backend/vulkan/vulkan_utils.hpp"
#include "platform/window/window.hpp"

#include "core/assert.hpp"
#include "core/logger.hpp"

#include <vulkan/vulkan_core.h>

namespace Renderer::Vulkan {

static VKAPI_ATTR VkBool32 VKAPI_CALL _DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data, void*) {
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

static VkResult _CreateDebugUtilsMessengerEXT(VkInstance instance,
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

static void _DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VulkanContext::VulkanContext(Platform::Window& window, VulkanConfiguration config)
    : m_Window(window)
    , m_Config(config) {
    create_instance();
    create_surface();
}

VulkanContext::~VulkanContext() { destroy(); }

void VulkanContext::create_instance() {
    ASSERT_MSG(m_Config.ValidationLayersEnabled && !check_validation_layer_support(),
        "[VulkanRenderer]: Validation layers requested but not available!");
    m_Config.VkApiVersion = m_Config.VkApiVersion;

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "VGE";
    appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.apiVersion = m_Config.VkApiVersion;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceInfo {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.pNext = nullptr;

    auto instance_extensions = get_required_extensions();

    instanceInfo.enabledExtensionCount = static_cast<U32>(instance_extensions.size());
    instanceInfo.ppEnabledExtensionNames = instance_extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (m_Config.ValidationLayersEnabled) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        instanceInfo.ppEnabledLayerNames = m_ValidationLayers.data();

        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = _DebugCallback;
        instanceInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    } else {
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = nullptr;
    }

#ifdef __PLATFORM_MACOS__
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VULKAN_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_Instance));

    if (m_Config.ValidationLayersEnabled) {
        setup_debug_messenger();
    }
}

void VulkanContext::create_surface() {
    ASSERT_MSG(m_Instance != VK_NULL_HANDLE, "[VulkanContext]: VkInstance is null!");
    m_Surface = m_Window.createSurface(*this);
}

void VulkanContext::destroy() {
    // TODO: deletion queue
    if (m_Config.ValidationLayersEnabled && m_DebugMessenger != VK_NULL_HANDLE) {
        _DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }

    m_Window.destroySurface(*this);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanContext::setup_debug_messenger() {
    VkDebugUtilsMessengerCreateInfoEXT dbg {};
    dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbg.pfnUserCallback = _DebugCallback;
    dbg.pNext = nullptr;

    VULKAN_CHECK(_CreateDebugUtilsMessengerEXT(m_Instance, &dbg, nullptr, &m_DebugMessenger));
}

bool VulkanContext::check_validation_layer_support() {
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

std::vector<const char*> VulkanContext::get_required_extensions() const {
    auto extensions = m_Window.getRequiredInstanceExtensions();

#ifdef __PLATFORM_MACOS__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

    if (m_Config.ValidationLayersEnabled) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

} // namespace Renderer::Vulkan
