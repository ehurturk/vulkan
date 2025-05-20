#include "application.h"

#include "platform/platform.h"

typedef struct application_state {
    b8 running;
    platform_state_t* p_state;
} application_state_t;

static application_state_t* astate;

b8 application_create(application_config_t* cfg) { astate->running = FALSE; }

b8 application_run() {}