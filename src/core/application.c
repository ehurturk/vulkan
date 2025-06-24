#include "application.h"
#include "defines.h"
#include "logger.h"
#include "platform/platform.h"

/* Total 24 bytes */
typedef struct application_state {
    platform_state_t* p_state; /* 8 bytes */
    application_config_t* cfg; /* 8 bytes */
    b8 running;                /* 1 byte */
    b8 suspended;              /* 1 byte */
    char _pad[6];              /* 6 bytes */
} application_state_t;

static b8 application_initialized = FALSE;
static application_state_t astate;

b8 application_create(application_config_t* cfg) {
    if (application_initialized) {
        LOG_ERROR(
            "One application is already initialized. Can't create more than one application.");
        return FALSE;
    }
    astate.running = FALSE;
    astate.suspended = FALSE;
    astate.cfg = cfg;
    astate.p_state = platform_allocate(sizeof(platform_state_t), TRUE);
    return TRUE;
}

b8 application_run() {
    while (platform_should_run(astate.p_state)) {
        platform_dispatch_messages(astate.p_state);
        /* update */
        /* render */
        /* swap front & back buffers */
    }
    return TRUE;
}

b8 application_shutdown() {}
