#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"

/*****************************************************************************
 * Gaussian filter
 ****************************************************************************/
float gfunc(float x, float sigma) {
    float s = sigma * sigma;
    return expf(-0.5 * x * x / s);
}

static size_t normalize_index(int p, size_t i_size, KernelMode mode) {
    int n = p;
    int size = static_cast<int>(i_size);

    switch (mode){
    case MODE_WRAP:
        if (n < 0) {
            n = size - ((-n) % size);
        }
        if (n >= size) {
            n %= size;
        }
        break;

    case MODE_REFLECT:
        while (n < 0 || n >= size) {
            if (n == -1) {
                n = 0;
            }
            if (n < -1) {
                n = 1;
            }
            if (n == size) {
                n = size - 2;
            }
            if (n > size) {
                n = size - 3;
            }
        }
        break;

    case MODE_MIRROR:
        while (n < 0 || n >= size) {
            if (n < 0) {
                n = -n;
            }
            if (n == size) {
                n = 2 * size - n - 1;
            }
            if (n > size) {
                n = 2 * size - n;
            }
        }
        break;
    }

    return static_cast<size_t>(n);
}

kernel_t* kernel_alloc(size_t size) {
    kernel_t* k = new kernel_t;
    if (!k) {
        LOGE << "kernel alloc error";
        return nullptr;
    }
    k->size = size;
    k->data = new float[size];
    return k;
}

void kernel_free(kernel_t* k) {
    assert(k);
    assert(k->data);
    if (k->data) {
        delete[] k->data;
    }
    delete k;
}

kernel_t* kernel_create(float sigma, KernelMode mode) {
    int lw = (int)(4.0 * sigma + 0.5);
    int k_size = lw * 2 + 1;
    if (k_size <= 0) {
        LOGE << "kernel invalid sigma error";
        return nullptr;
    }
    
    kernel_t* k = kernel_alloc(k_size);
    k->mode = mode;

    float s = 1.0;
    k->data[lw] = 1.0;
    for (int i = 1; i <= lw; i++) {
        float f = gfunc((float)(i), sigma);
        k->data[lw-i] = f;
        k->data[lw+i] = f;
        s += f * 2.0;
    }

    for (int i = 0; i < k_size; i++) {
        k->data[i] /= s;
    }
    
    return k;
}

matrix_t* kernel_apply_to_matrix(matrix_t* dst, matrix_t* src, matrix_t* tmp, kernel_t* k) {
    assert(dst);
    assert(dst->data);
    assert(src);
    assert(src->data);
    assert(k);
    assert(k->data);
    
    if (dst->rows != src->rows) {
        LOGE << "kernel rows mismatch";
        return dst;
    }
    
    if (dst->cols != src->cols) {
        LOGE << "kernel cols mismatch";
        return dst;
    }
    
    size_t k2 = k->size / 2;
    
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
    for (int j = 0; j < static_cast<int>(dst->cols); ++j) {
        for (int i = 0; i < static_cast<int>(dst->rows); ++i) {
            float d = 0.0;

            for (int n = 0; n < static_cast<int>(k->size); n++) {
                int p = i + n - k2;
                p = normalize_index(p, dst->cols, k->mode);
                d += src->data[p + j * src->cols] * k->data[n];
            }

            tmp->data[i + j * tmp->cols] = d;
        }
    }
    
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
    for (int j = 0; j < static_cast<int>(dst->cols); ++j) {
        for (int i = 0; i < static_cast<int>(dst->rows); ++i) {
            float d = 0.0;

            for (int n = 0; n < static_cast<int>(k->size); ++n) {
                int p = i + n - k2;
                p = normalize_index(p, dst->rows, k->mode);
                d += tmp->data[j + p * tmp->cols] * k->data[n];
            }

            dst->data[j + i * dst->cols] = d;
        }
    }
    
    return dst;
}

matrix_t* kernel_filter_matrix(matrix_t* dst, matrix_t* src, float sigma, KernelMode mode) {
    assert(dst);
    assert(dst->data);
    assert(src);
    assert(src->data);

    kernel_t* k = kernel_create(sigma, mode);
    if (!k) {
        return dst;
    }
    
    matrix_t* tmp = matrix_allocate(src->rows, src->cols);

    kernel_apply_to_matrix(dst, src, tmp, k);

    matrix_free(tmp);

    kernel_free(k);
    
    return dst;
}
