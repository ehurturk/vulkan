#ifndef GAME_TYPES
#define GAME_TYPES

#include "core/application.h"

typedef struct game {
    application_config_t config;

    void (*initialize)();   /* Called on game initialization */
    void (*update)(f32 dt); /* Called on game update on each frame */
    void (*render)(f32 dt); /* */
    void (*shutdown)();
} game_t;

#endif /* GAME_TYPES */
