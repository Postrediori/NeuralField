#pragma once

#include <stddef.h>
#include "Matrix.h"

struct texture_t {
    size_t size;
    size_t bpp;
    uint8_t* data;
};

texture_t* texture_alloc(size_t size, size_t bpp);
void texture_free(texture_t* t);

texture_t* texture_copy_matrix(texture_t* t, matrix_t* m);
