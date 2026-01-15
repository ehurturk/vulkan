#include "renderer/backend/vulkan/vulkan_device.hpp"
#include "renderer/backend/vulkan/vulkan_buffer.hpp"
#include "renderer/backend/vulkan/vulkan_utils.hpp"
#include "renderer/backend/vulkan/vulkan_context.hpp"

#include "core/logger.hpp"
#include "core/assert.hpp"

#include <vulkan/vulkan_core.h>

#include <set>

namespace Renderer::Vulkan {

VulkanDevice::VulkanDevice(VulkanContext& context, VkSurfaceKHR surface)
    : m_Context(context)
    , m_Surface(surface) {
    pick_physical_device();
    create_device();
    create_memory_allocator();
}

VulkanDevice::~VulkanDevice() {
    if (m_Device == VK_NULL_HANDLE)
        CORE_LOG_FATAL("[VulkanDevice]: VkDevice is NULL.");

    if (m_PhysicalDevice == VK_NULL_HANDLE)
        CORE_LOG_FATAL("[VulkanDevice]: VkPhysicalDevice is NULL.");

    if (m_Allocator == VK_NULL_HANDLE)
        CORE_LOG_FATAL("[VulkanDevice]: VmaAllocator is NULL.");

    CORE_LOG_INFO("[VulkanDevice]: Destroying VulkanDevice...");
    vmaDestroyAllocator(m_Allocator);
    vkDestroyDevice(m_Device, nullptr);
}

// Buffer VulkanDevice::create_buffer() const {  }

QueueFamilyIndices VulkanDevice::find_queue_families(VkPhysicalDevice pd) const {
    QueueFamilyIndices queueIndices;

    U32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueIndices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, m_Surface, &presentSupport);

        if (presentSupport) {
            queueIndices.presentFamily = i;
        }

        if (queueIndices.is_complete())
            break;
        i++;
    }

    return queueIndices;
}

SwapchainSupportDetails VulkanDevice::query_swap_chain_support(VkPhysicalDevice device) const {
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

bool VulkanDevice::is_physical_device_suitable(VkPhysicalDevice pd) const {
    QueueFamilyIndices queueIndices = find_queue_families(pd);
    bool extensionsSupported = check_physical_device_extension_support(pd);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport = query_swap_chain_support(pd);
        swapChainAdequate
            = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(pd, &supportedFeatures);

    return queueIndices.is_complete() && extensionsSupported && swapChainAdequate
        && supportedFeatures.samplerAnisotropy;
}

void VulkanDevice::pick_physical_device() {
    U32 deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Context.vk_instance(), &deviceCount, nullptr);
    if (deviceCount == 0) {
        CORE_LOG_FATAL("Failed to find GPUs with Vulkan support!");
        ASSERT(false);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Context.vk_instance(), &deviceCount, devices.data());

    // Select the first physical device
    for (const auto& device : devices) {
        if (is_physical_device_suitable(device)) {
            m_PhysicalDevice = device;
            break;
        }
    }

    ASSERT_MSG(m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU!");
}

void VulkanDevice::create_device() {
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
#ifdef BUILD_DEBUG
    createInfo.enabledLayerCount = static_cast<U32>(m_ValidationLayers.size());
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif

    VULKAN_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

    vkGetDeviceQueue(m_Device, graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, presentFamily.value(), 0, &m_PresentQueue);
}

void VulkanDevice::create_memory_allocator() {
    VmaVulkanFunctions vkFunctions {};
    vkFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vkFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
    vkFunctions.vkCreateImage = &vkCreateImage;

    VmaAllocatorCreateInfo createInfo {};
    createInfo.instance
        = m_Context.vk_instance(); // TODO: probably should be retrieved via call to VulkanContext
    createInfo.device = m_Device;
    createInfo.physicalDevice = m_PhysicalDevice;
    createInfo.vulkanApiVersion = m_Context.vk_version();
    createInfo.pVulkanFunctions = &vkFunctions;

    VULKAN_CHECK(vmaCreateAllocator(&createInfo, &m_Allocator));
}

} // namespace Renderer::Vulkan
