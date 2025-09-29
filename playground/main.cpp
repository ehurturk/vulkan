#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

#include <platform/unix/platform.hpp>
#include "core/application.hpp"

class MyApp : public Core::Application {
   public:
    bool initialize(Platform::Window* window) override { return true; }

    void update(float deltaTime) override {}

    void render() override {}

    void cleanup() override {}
};

ENTRYPOINT(playground) {
    for (const auto& arg : playground.arguments()) {
        LOG_INFO("Arg: {}", arg);
    }

    Platform::UnixPlatform platform(playground, Platform::UnixType::MACOS);

    MyApp app;

    platform.initialize();
    platform.run(&app);
    platform.terminate();

    return 1;
}