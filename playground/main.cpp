#include <core/application.hpp>
#include <core/logger.hpp>

int main() {
    Core::Logger::getInstance().initialize();

#ifdef BUILD_DEBUG
    LOG_INFO("Building in debug mode...");
#elif BUILD_RELEASE
    LOG_INFO("Building in release mode...");
#endif

    Core::ApplicationConfig cfg{.width = 1280, .height = 720, .name = "Vulkan Test"};
    Core::Application &app = Core::Application::getInstance();

    if (!app.create(cfg)) {
        LOG_FATAL("Failed to create application");
        return -1;
    }

    if (!app.run()) {
        LOG_FATAL("Application failed to run");
        return -1;
    }

    app.shutdown();
    Core::Logger::getInstance().shutdown();

    return 0;
}