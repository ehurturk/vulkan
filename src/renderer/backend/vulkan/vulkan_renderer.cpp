#include "vulkan_renderer.hpp"
#include "core/logger.hpp"
#include "core/assert.hpp"
#include "../renderer.hpp"

#include <GLFW/glfw3.h>

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

static PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerEXT = nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = nullptr;

VulkanRenderer::VulkanRenderer() : m_vkState(std::make_unique<VkState>()) {}
VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

void VulkanRenderer::initialize(const RendererConfig& cfg) {
    m_vkState->validation = cfg.enableValidation;
    create_instance(m_vkState->validation);
    if (m_vkState->validation)
        setup_debug_messenger();
    LOG_INFO("Vulkan instance created.");
}

void VulkanRenderer::shutdown() {
    if (!m_vkState)
        return;
    if (m_vkState->validation && m_vkState->debugMessenger) {
        destroy_debug_messenger();
        m_vkState->debugMessenger = VK_NULL_HANDLE;
    }
    if (m_vkState->instance) {
        vkDestroyInstance(m_vkState->instance, nullptr);
        m_vkState->instance = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::create_instance(bool enableValidation) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CC Engine";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.pEngineName = "CC Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pNext = nullptr;

    uint32_t glfwCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwCount);

    std::vector<const char*> extensions(glfwExts, glfwExts + glfwCount);

#ifdef __PLATFORM_MACOS__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    if (enableValidation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";
    std::vector<const char*> layers;
    if (enableValidation) {
        // In production you’d enumerate and verify presence.
        layers.push_back(kValidationLayer);
    }

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ci.pApplicationInfo = &appInfo;
    ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.data();
    ci.enabledLayerCount = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames = layers.data();
    ci.pNext = nullptr;

#ifdef __PLATFORM_MACOS__
    ci.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VULKAN_CHECK(vkCreateInstance(&ci, nullptr, &m_vkState->instance));

    if (enableValidation) {
        pfnCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_vkState->instance, "vkCreateDebugUtilsMessengerEXT"));
        pfnDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_vkState->instance, "vkDestroyDebugUtilsMessengerEXT"));
    }
}

void VulkanRenderer::setup_debug_messenger() {
    if (!pfnCreateDebugUtilsMessengerEXT) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT dbg{};
    dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbg.pfnUserCallback = DebugCallback;
    dbg.pNext = nullptr;

    VULKAN_CHECK(pfnCreateDebugUtilsMessengerEXT(m_vkState->instance, &dbg, nullptr,
                                                 &m_vkState->debugMessenger));
}

void VulkanRenderer::destroy_debug_messenger() {
    if (pfnDestroyDebugUtilsMessengerEXT && m_vkState->debugMessenger) {
        pfnDestroyDebugUtilsMessengerEXT(m_vkState->instance, m_vkState->debugMessenger, nullptr);
    }
}

}  // namespace Renderer
