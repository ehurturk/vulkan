#include <core/assert.h>
#include <core/logger.h>
#include <platform/platform.h>
#include <platform/window.h>

// NOTE: This file is a playground that tests the engine library functionality.

int main() {
  WARN("Hello there!");
  FATAL("Oopps!");
  ASSERT(10 < 2);

  window_t *win = window_init(600, 800, "Test Vulkan", FALSE);
  while (win->running) {
    window_poll_events();
  }

  window_shutdown(win);
}
