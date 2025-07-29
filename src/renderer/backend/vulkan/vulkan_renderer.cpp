#include "vulkan_renderer.hpp"
#include "../../../core/logger.hpp"
#include "../../../core/assert.hpp"
#include "../renderer.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstring>

namespace Renderer {

VulkanRenderer::VulkanRenderer() : m_state(std::make_unique<InternalState>()) {}

VulkanRenderer::~VulkanRenderer() { destroy(); }

void VulkanRenderer::initialize(Renderer *renderer) {
    ASSERT(renderer != nullptr);
    m_renderer = renderer;
    createInstance();
    ASSERT_MSG(true, "Renderer backend initialization works!");
}

void VulkanRenderer::destroy() {
    if (m_state && m_state->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_state->instance, nullptr);
        m_state->instance = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::handleExtensions(std::vector<const char *> &extensions) {
    u32 glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    extensions.clear();
    extensions.reserve(glfwExtensionCount + 1);

    for (u32 i = 0; i < glfwExtensionCount; ++i) {
        extensions.push_back(glfwExtensions[i]);
    }

#ifdef PLATFORM_APPLE
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    LOG_INFO("{}Available extensions:", Colors::YELLOW);
    for (const auto &ext : availableExtensions) {
        LOG_INFO("\t{}{}", Colors::YELLOW, ext.extensionName);
    }

    for (const auto *requiredExt : extensions) {
        auto it = std::find_if(availableExtensions.begin(), availableExtensions.end(),
                               [requiredExt](const VkExtensionProperties &ext) {
                                   return std::strcmp(ext.extensionName, requiredExt) == 0;
                               });

        if (it == availableExtensions.end()) {
            LOG_FATAL("GLFW Extension {} is not supported!", requiredExt);
            ASSERT(false);
        }
    }

    LOG_INFO("All GLFW extensions are supported!");
}

void VulkanRenderer::createInstance() {
    ASSERT(m_renderer != nullptr);
    ASSERT(m_state != nullptr);

    m_state->appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                        .pNext = nullptr,
                        .pApplicationName = "Hello Vulkan",
                        .applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
                        .pEngineName = "CC Engine",
                        .engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
                        .apiVersion = VK_API_VERSION_1_0};

    std::vector<const char *> extensions;
    handleExtensions(extensions);

    m_state->createInfo = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                           .pNext = nullptr,
                           .flags = 0,
                           .pApplicationInfo = &m_state->appInfo,
                           .enabledLayerCount = 0,
                           .ppEnabledLayerNames = nullptr,
                           .enabledExtensionCount = static_cast<u32>(extensions.size()),
                           .ppEnabledExtensionNames = extensions.data()};

#ifdef PLATFORM_APPLE
    m_state->createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkResult result = vkCreateInstance(&m_state->createInfo, nullptr, &m_state->instance);
    if (result != VK_SUCCESS) {
        LOG_FATAL("VkResult is: {}", static_cast<i32>(result));
        ASSERT_MSG(false, "Failed to create VK Instance");
    }
}

}