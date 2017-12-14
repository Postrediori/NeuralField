#include "stdafx.h"
#include "Texture.h"
#include "Matrix.h"
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

    switch (mode){
    case MODE_WRAP:
        if (n<0) {
            n = i_size - ((-n) % i_size);
        }
        if (n>=i_size) {
            n %= i_size;
        }
        break;

    case MODE_REFLECT:
        while (n<0 || n>=i_size) {
            if (n==-1) {
                n = 0;
            }
            if (n<-1) {
                n = 1;
            }
            if (n==i_size) {
                n = i_size - 2;
            }
            if (n>i_size) {
                n = i_size - 3;
            }
        }
        break;

    case MODE_MIRROR:
        while (n<0 || n>=i_size) {
            if (n<0) {
                n = -n;
            }
            if (n>=i_size) {
                n = 2 * i_size - n;
            }
        }
        break;
    }

    return (size_t)n;
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
    for (int i=1; i <= lw; i++) {
        double f = gfunc((double)(i), sigma);
        k->data[lw-i] = f;
        k->data[lw+i] = f;
        s += f * 2.0;
    }

    for (int i=0; i < k_size; i++) {
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
    
    int k2 = k->size / 2;
    
//#pragma omp parallel for private(i,j,d) shared(src,tmp,k)
    for (int j = 0; j < dst->cols; ++j) {
        for (int i = 0; i < dst->rows; ++i) {
            double d = 0.0;

            for (int n = 0; n < k->size; n++) {
                int p = i + n - k2;
                p = normalize_index(p, dst->cols, k->mode);
                d += src->data[p + j * src->cols] * k->data[n];
            }

            tmp->data[i + j * tmp->cols] = d;
        }
    }
    
//#pragma omp parallel for private(i,j,d) shared(tmp,dst,k)
    for (int j = 0; j < dst->cols; ++j) {
        for (int i = 0; i < dst->rows; ++i) {
            double d = 0.0;

            for (int n = 0; n < k->size; ++n) {
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

texture_t* kernel_apply_to_texture(texture_t* t, kernel_t* k) {
    if (t == nullptr || k == nullptr) {
        LOGE << "kernel null error";
        return t;
    }
    
    texture_t* tmp = texture_alloc(t->size, t->bpp);
    if (!tmp) {
        return t;
    }
    
    int k2 = k->size / 2;
    
//#pragma omp parallel for private(i,j,r,g,b) shared(tmp,texture,k)
    for (int j = 0; j < t->size; ++j) {
        for (int i = 0; i < t->size; ++i) {
            double r = 0.0, g = 0.0, b = 0.0;

            for (int n = 0; n < k->size; n++) {
                int p = i + n - k2;
                p = normalize_index(p, t->size, k->mode);

                r += t->data[(p+j*t->size)*t->bpp+0] * k->data[n];
                g += t->data[(p+j*t->size)*t->bpp+1] * k->data[n];
                b += t->data[(p+j*t->size)*t->bpp+2] * k->data[n];
            }

            tmp->data[(i+j*t->size)*t->bpp+0] = r;
            tmp->data[(i+j*t->size)*t->bpp+1] = g;
            tmp->data[(i+j*t->size)*t->bpp+2] = b;
        }
    }
    
//#pragma omp parallel for private(i,j,r,g,b) shared(tmp,texture,k)
    for (int j = 0; j < t->size; ++j) {
        for (int i = 0; i < t->size; ++i) {
            double r = 0.0, g = 0.0, b = 0.0;

            for (int n = 0; n < k->size; ++n) {
                int p = i + n - k2;
                p = normalize_index(p, tmp->size, k->mode);

                r += tmp->data[(j+p*tmp->size)*tmp->bpp+0] * k->data[n];
                g += tmp->data[(j+p*tmp->size)*tmp->bpp+1] * k->data[n];
                b += tmp->data[(j+p*tmp->size)*tmp->bpp+2] * k->data[n];
            }

            t->data[(j+i*t->size)*t->bpp+0] = r;
            t->data[(j+i*t->size)*t->bpp+1] = g;
            t->data[(j+i*t->size)*t->bpp+2] = b;
        }
    }

    texture_free(tmp);
    
    return t;
}

texture_t* kernel_filter_texture(texture_t* t, double sigma, KernelMode mode) {
    if (t == nullptr) {
        LOGE << "texture null error";
        return nullptr;
    }
    kernel_t* k = kernel_create(sigma, mode);
    if (!k) {
        return t;
    }

    kernel_apply_to_texture(t, k);

    kernel_free(k);
    
    return t;
}
