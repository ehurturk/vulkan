#include "window.h"

#include "core/logger.h"
#include "platform.h"

window_t* window_init(i32 w, i32 h, const char* name, b8 resizable) {
  INFO("Creating window %s...", name);
  window_t* window = platform_allocate(sizeof(window_t), TRUE);
  window->name = name;
  window->width = w;
  window->height = h;
  window->resizable = resizable;
  window->_window = glfwCreateWindow(w, h, name, NULL, NULL);
  return window;
}

void window_poll_events() { glfwPollEvents(); }

void window_shutdown(window_t* win) {
  INFO("Destroying window %s...", win->name);
  glfwDestroyWindow(win->_window);
}
