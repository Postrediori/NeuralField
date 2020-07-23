#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "Gauss.h"

/*****************************************************************************
 * Gaussian filter
 ****************************************************************************/
static double gfunc(double x, double sigma) {
    double s = sigma * sigma;
    return exp(-0.5 * x * x / s);
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
    k->data = new double[size];
    return k;
}

void kernel_free(kernel_t* k) {
    if (!k) {
        LOGE << "kernel null error";
        return;
    }
    if (k->data) {
        delete[] k->data;
    }
    delete k;
}

kernel_t* kernel_create(double sigma, KernelMode mode) {
    int lw = (int)(4.0 * sigma + 0.5);
    int k_size = lw * 2 + 1;
    if (k_size <= 0) {
        LOGE << "kernel invalid sigma error";
        return nullptr;
    }
    
    kernel_t* k = kernel_alloc(k_size);
    k->mode = mode;

    double s = 1.0;
    k->data[lw] = 1.0;
    for (int i = 1; i <= lw; i++) {
        double f = gfunc((double)(i), sigma);
        k->data[lw-i] = f;
        k->data[lw+i] = f;
        s += f * 2.0;
    }

    for (int i = 0; i < k_size; i++) {
        k->data[i] /= s;
    }
    
    return k;
}

matrix_t* kernel_apply_to_matrix(matrix_t* dst, matrix_t* src, kernel_t* k) {
    if (dst == nullptr || src == nullptr || k == nullptr) {
        LOGE << "kernel null error";;
        return nullptr;
    }
    
    if (dst->rows != src->rows) {
        LOGE << "kernel rows mismatch";
        return dst;
    }
    
    if (dst->cols != src->cols) {
        LOGE << "kernel cols mismatch";
        return dst;
    }
    
    matrix_t* tmp = matrix_allocate(src->rows, src->cols);
    if (!tmp) {
        return dst;
    }
    
    size_t k2 = k->size / 2;
    
#pragma omp parallel for
    for (int j = 0; j < static_cast<int>(dst->cols); ++j) {
        for (int i = 0; i < static_cast<int>(dst->rows); ++i) {
            double d = 0.0;

            for (int n = 0; n < static_cast<int>(k->size); n++) {
                int p = i + n - k2;
                p = normalize_index(p, dst->cols, k->mode);
                d += src->data[p + j * src->cols] * k->data[n];
            }

            tmp->data[i + j * tmp->cols] = d;
        }
    }
    
#pragma omp parallel for
    for (int j = 0; j < static_cast<int>(dst->cols); ++j) {
        for (int i = 0; i < static_cast<int>(dst->rows); ++i) {
            double d = 0.0;

            for (int n = 0; n < static_cast<int>(k->size); ++n) {
                int p = i + n - k2;
                p = normalize_index(p, dst->rows, k->mode);
                d += tmp->data[j + p * tmp->cols] * k->data[n];
            }

            dst->data[j + i * dst->cols] = d;
        }
    }

    matrix_free(tmp);
    
    return dst;
}

matrix_t* kernel_filter_matrix(matrix_t* dst, matrix_t* src, double sigma, KernelMode mode) {
    if (dst == nullptr || src == nullptr) {
        LOGE << "kernel null error";
        return dst;
    }
    kernel_t* k = kernel_create(sigma, mode);
    if (!k) {
        return dst;
    }
    
    kernel_apply_to_matrix(dst, src, k);

    kernel_free(k);
    
    return dst;
}
