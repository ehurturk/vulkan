#include <platform/core/PlatformEntryPoint.hpp>
#include <core/logger.hpp>

#include <platform/unix/platform.hpp>
#include "core/application.hpp"

class MyApp : public Core::Application {
   public:
    bool initialize(Platform::Window* window) override {
        LOG_INFO("[MyApp]: Initializing my app!");
        m_Window = window;
        return true;
    }

    void update(float deltaTime) override {
        (void)deltaTime;
        m_Window->setTitle("FPS: " + std::to_string(getFPS()) +
                           ", MS: " + std::to_string(getFrameTime()));
    }

    void render() override {}

    void cleanup() override {}

   private:
    Platform::Window* m_Window;
};

ENTRYPOINT(platform) {
    MyApp app;

    platform.initialize();
    platform.run(&app);
    platform.terminate();

    return 1;
}
