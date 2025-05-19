#include "window.h"

#include <stdlib.h>

#include "core/logger.h"
#include "platform.h"

window_t* window_init(i32 w, i32 h, const char* name, b8 resizable, b8 fullscreen) {
    LOG_INFO("Creating window %s...", name);
    window_t* window = malloc(sizeof(window_t));
    window->name = name;
    window->width = w;
    window->height = h;
    window->resizable = resizable;
    window->handle = NULL;
    window->fullscreen = fullscreen;

    if (!glfwInit()) {
        LOG_FATAL("Failed to initialize GFLFW.");
        return NULL;
    }
    return window;
}

b8 window_create(window_t* win) {
    /* TODO: For different renderer backends, use GLFW_OPENGL */
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, win->resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWwindow* _handle;
    if (!(win->fullscreen)) {
        _handle = glfwCreateWindow(win->width, win->height, win->name, NULL, NULL);
    } else {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        _handle = glfwCreateWindow(win->width, win->height, win->name, primary, NULL);
    }

    if (_handle == NULL) {
        LOG_FATAL("Failed to create GLFW window.");
        platform_free(win, FALSE);
        return FALSE;
    }
    win->handle = _handle;
    glfwSetWindowUserPointer(win->handle, win);
    return TRUE;
}

void window_poll_events() { glfwPollEvents(); }

b8 window_should_close(window_t* win) { return glfwWindowShouldClose(win->handle); }

void window_shutdown(window_t* win) {
    if (win == NULL) {
        LOG_ERROR("Window is NULL.");
        return;
    }
    if (win->handle == NULL) {
        LOG_ERROR("Can't destory uninitialized window.");
        return;
    }

    LOG_INFO("Destroying window %s...", win->name);
    glfwDestroyWindow(win->handle);
    win->handle = NULL;
    glfwTerminate();
}
