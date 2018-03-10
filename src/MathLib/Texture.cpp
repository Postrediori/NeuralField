#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"

texture_t* texture_alloc(size_t size, size_t bpp) {
    texture_t* t = new texture_t;
    if (!t) {
        LOGE << "texture alloc error";
        return nullptr;
    }
    
    t->size = size;
    t->bpp = bpp;
    
    t->data = new uint8_t[size * size * bpp];
    
    return t;
}

void texture_free(texture_t* t) {
    if (!t) {
        LOGE << "texture null error";
        return;
    }
    if (t->data) {
        delete[] t->data;
    }
    delete t;
}

texture_t* texture_copy_matrix(texture_t* t, matrix_t* m) {
    if (t == nullptr || m == nullptr) {
        LOGE << "texture null error";
        return t;
    }
    
    if (t->size != m->rows) {
        LOGE << "texture rows error";
        return t;
    }
    
    if (t->size != m->cols) {
        LOGE << "texture cols error";
        return t;
    }
    
    if (t->bpp != 3 && t->bpp != 4) {
        LOGE << "texture bpp error";
        return t;
    } 
    
#pragma omp parallel for
    for (size_t idx = 0; idx < t->size * t->size; ++idx) {
        uint8_t k = m->data[idx] > 0.0 ? 0xff : 0x00;
        t->data[idx * t->bpp + 0] = k;
        t->data[idx * t->bpp + 1] = k;
        t->data[idx * t->bpp + 2] = k;
        if (t->bpp == 4) {
            t->data[idx * t->bpp + 3] = 0xff;
        }
    }
    
    return t;
}
