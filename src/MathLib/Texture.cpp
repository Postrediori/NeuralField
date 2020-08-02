#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"

texture_t* texture_alloc(size_t size, TextureFormat format) {
    texture_t* t = new texture_t;
    if (!t) {
        LOGE << "texture alloc error";
        return nullptr;
    }
    
    t->size = size;
    t->format = format;
    t->bpp = static_cast<size_t>(format);
    t->pixelCount = size * size;
    t->dataSize = t->pixelCount * t->bpp;
    
    t->data = new uint8_t[t->dataSize];
    
    return t;
}

void texture_free(texture_t* t) {
    assert(t);
    assert(t->data);

    if (t->data) {
        delete[] t->data;
    }
    delete t;
}

texture_t* texture_copy_matrix(texture_t* t, matrix_t* m) {
    assert(t);
    assert(t->data);
    assert(m);
    assert(m->data);
    
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
    for (int idx = 0; idx < static_cast<int>(t->pixelCount); ++idx) {
        uint8_t k = m->data[idx] > 0.0 ? 0xff : 0x00;
        t->data[idx * t->bpp + 0] = k;
        t->data[idx * t->bpp + 1] = k;
        t->data[idx * t->bpp + 2] = k;
    }

    if (t->format == TextureFormat::RGBA) {
#pragma omp parallel for
        for (int idx = 0; idx < static_cast<int>(t->pixelCount); ++idx) {
            t->data[idx * t->bpp + 3] = 0xff;
        }
    }
    
    return t;
}
