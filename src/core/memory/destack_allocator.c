#include "destack_allocator.h"

#include <memory.h>
#include <stdlib.h>

destack_allocator_t* destack_init() {
    destack_allocator_t* ds = malloc(sizeof(destack_allocator_t));
    ds->start = NULL;
    ds->end = NULL;
}

void destack_destroy(destack_allocator_t* ds) {
    /* TODO: free end & start? */
    ds->start = NULL;
    ds->end = NULL;
    free(ds);
}
