#ifndef WINDOW_H_
#define WINDOW_H_

#include <GLFW/glfw3.h>

#include "defines.h"

typedef struct window {
  GLFWwindow* _window;
  i32 height;
  i32 width;
  b8 resizable;
  b8 running;
  const char* name;
} window_t;

window_t* window_init(i32 w, i32 h, const char* name, b8 resizable);
void window_poll_events();
void window_shutdown(window_t* win);

#endif
