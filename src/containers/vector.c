#include "vector.h"
#include "core/assert.h"
#include <stdlib.h>

#define VECTOR_INITIAL_CAPACITY 10

vector_t* vector_init() {
    vector_t *vec = malloc(sizeof(vector_t));
    ASSERT(vec != NULL);
    vec->max_capacity = VECTOR_INITIAL_CAPACITY;
    vec->size = 0;
    vec->data = malloc(sizeof(void *)); // FIXME: this will always return 8 bytes!!!!
    ASSERT(vec->data != NULL);
    return vec;
}

void vector_push_back(vector_t* vec, void* data) {
    if (vec->size >= vec->max_capacity) {
        /* resize */
        vec->max_capacity *= 2; /* NOTE: may change this to += 10 */
        vec->data = realloc(vec->data, vec->max_capacity);
        ASSERT_MSG(vec->data != NULL, "There was a problem when resizing the vector");
    }
    // vec->data[vec->size-1] = NULL;
}

void vector_push_at_index(vector_t* vec, void* data, int idx) {

}

void vector_remove_elem(vector_t* vec, void* data) {

}

void vector_remove_at_index(vector_t* vec, uint idx) {

}

void vector_free(vector_t* vec) {

}