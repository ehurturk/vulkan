#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

#include <platform/unix/platform.hpp>
#include "core/application.hpp"

class MyApp : public Core::Application {
   public:
    bool initialize(Platform::Window* window) override {
        (void)window;
        return true;
    }

    void update(float deltaTime) override { (void)deltaTime; }

    void render() override {}

    void cleanup() override {}
};

ENTRYPOINT(platform) {
    MyApp app;

    platform.initialize();
    platform.run(&app);
    platform.terminate();

    return 1;
}
