#include "context.hpp"

namespace Platform {
WindowsPlatformContext::WindowsPlatformContext(HINSTANCE hInstance,
                                               HINSTANCE hPrevInstance,
                                               PSTR lpCmdLine,
                                               INT nCmdShow) {
    throw std::runtime_error("Windows platform is not supported yet!");
}
}  // namespace Platform