#ifndef RENDERER
#define RENDERER

#define RENDERER_BACKEND_VK

typedef enum {
    RENDERER_VK = 0,
    RENDERER_GL = 1,
} RENDERER_BACKEND;

typedef struct {
    RENDERER_BACKEND backend;
    /* other properties */
} renderer_state_t;

typedef struct {
    void *internal_state;
    renderer_state_t state;
} renderer_t;

void init_renderer(renderer_t *renderer);
void destroy_renderer(renderer_t *renderer);

#endif /* RENDERER */
