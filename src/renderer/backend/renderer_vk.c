#include "defines.h"
#include "renderer.h"
#include "core/assert.h"
#include "core/logger.h"

#include <_string.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#ifdef RENDERER_BACKEND_VK

#define MAX_EXTENSION_LEN 50

typedef struct {
    VkInstance instance;
} vk_internal_state_t;

void vk_create_instance(renderer_t *renderer);

void init_renderer(renderer_t *renderer) {
    ASSERT(renderer != NULL);
    vk_create_instance(renderer);
    ASSERT_MSG(TRUE, "Renderer backend initialization works!");
}

void vk_create_instance(renderer_t *renderer) {
    ASSERT(renderer != NULL);

    renderer->internal_state = malloc(sizeof(vk_internal_state_t));
    vk_internal_state_t *is = renderer->internal_state;

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Vulkan";
    app_info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;
    app_info.pNext = NULL;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    u32 glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    create_info.enabledLayerCount = 0;
    create_info.ppEnabledExtensionNames = glfw_extensions;

#ifdef PLATFORM_APPLE
    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    glfw_extension_count++;
    const char *extra_extensions[glfw_extension_count];
    for (int i = 0; i < glfw_extension_count - 1; i++) {
        extra_extensions[i] = glfw_extensions[i];
    }
    extra_extensions[glfw_extension_count - 1] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    create_info.ppEnabledExtensionNames = extra_extensions;
#endif

    create_info.enabledExtensionCount = glfw_extension_count;

    VkResult res = vkCreateInstance(&create_info, NULL, &is->instance);
    if (res != VK_SUCCESS) {
        ASSERT_MSG(FALSE, "Failed to create VK Instance");
        LOG_FATAL("VkResult is: %d", res);
    }
}

void destroy_renderer(renderer_t *renderer) {
    ASSERT(renderer != NULL);
    vkDestroyInstance(((vk_internal_state_t *)renderer->internal_state)->instance, NULL);
    free(((vk_internal_state_t *)renderer->internal_state));
}

#endif