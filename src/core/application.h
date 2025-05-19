#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "defines.h"

typedef struct application_config_t {
    /* app width & height */
    i32 width;
    i32 height;
    /* application name */
    const char* name;
} application_config_t;

API b8 application_create(application_config_t* cfg);
API b8 application_run();

#endif