// #include <core/application.hpp>
// #include <core/logger.hpp>

// int main() {
//     Core::Logger::getInstance().initialize();

// #ifdef BUILD_DEBUG
//     LOG_INFO("Building in debug mode...");
// #elif BUILD_RELEASE
//     LOG_INFO("Building in release mode...");
// #endif

//     const Core::ApplicationConfig cfg{.width = 1280, .height = 720, .name = "Vulkan Test",
//     .backend = Renderer::RendererBackendType::Vulkan}; Core::Application &app =
//     Core::Application::getInstance();

//     if (!app.create(cfg)) {
//         LOG_FATAL("Failed to create application");
//         return -1;
//     }

//     if (!app.run()) {
//         LOG_FATAL("Application failed to run");
//         return -1;
//     }

//     app.shutdown();
//     Core::Logger::getInstance().shutdown();

//     return 0;
// }

#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

CREATE_CONTEXT(playground_context) {
    for (const auto& arg : playground_context.arguments()) {
        LOG_INFO("Arg: {}", arg);
    }

    return 1;
}