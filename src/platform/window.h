#ifndef WINDOW_H_
#define WINDOW_H_

#include <GLFW/glfw3.h>

#include "defines.h"

typedef struct window {
    GLFWwindow* handle; /* 8 bytes */
    i32 height;         /* 4 bytes */
    i32 width;          /* 4 bytes */
    b8 resizable;       /* 1 byte */
    b8 fullscreen;      /* 1 byte */
    const char* name;   /* 8 bytes */
} window_t;

window_t* window_init(i32 w, i32 h, const char* name, b8 resizable, b8 fullscreen);
b8 window_create(window_t* win);
void window_poll_events();
void window_shutdown(window_t* win);

b8 window_should_close(window_t* win);

#endif
