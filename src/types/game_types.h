#ifndef GAME_TYPES
#define GAME_TYPES

#include "defines.h"

typedef struct game {
    void (*init_game)();
    void (*update_game)(f32 dt);
    void (*render_game)(f32 dt);
    void (*shutdown_game)();
} game_t;

#endif /* GAME_TYPES */
