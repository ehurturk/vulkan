#ifndef DESTACK_ALLOCATOR
#define DESTACK_ALLOCATOR

#include "defines.h"

/*
 * A double ended stack allocator for better memory
 * allocation, avoids fragmentation as much as possible.
 */

typedef struct destack_allocator {
    void* end;
    void* start;
} destack_allocator_t;

API destack_allocator_t* destack_init();
API void destack_destroy(destack_allocator_t* ds);

#endif /* DESTACK_ALLOCATOR */
