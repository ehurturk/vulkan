#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <filesystem>

namespace Renderer::Vulkan {

namespace fs = std::filesystem;

class Texture {
public:
    Texture() = default;
    ~Texture();

    // TODO: methods for creating a texture
    static Texture load_from_path(fs::path path);

private:
    VkImage image { VK_NULL_HANDLE };
    VmaAllocation allocation { VK_NULL_HANDLE };
    VkImageView view { VK_NULL_HANDLE };
    VkSampler sampler { VK_NULL_HANDLE }; // TODO: reference a global sampler?
};

} // namespace Renderer::Vulkan
