#ifndef VECTOR
#define VECTOR

#include "defines.h"

typedef struct {
    void *data;
    uint size;
    uint max_capacity;
} vector_t;

vector_t *vector_init();
void vector_push_back(vector_t *vec, void *data);
void vector_push_at_index(vector_t *vec, void *data, int idx);
void vector_remove_elem(vector_t *vec, void *data);
void vector_remove_at_index(vector_t *vec, uint idx);
void vector_free(vector_t *vec);

#endif /* VECTOR */
