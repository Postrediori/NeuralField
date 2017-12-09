#include "stdafx.h"
#include "Gauss.h"

/*****************************************************************************
 * Gaussian filter
 ****************************************************************************/
inline float gfunc(float x, float sigma) {
    float s = sigma * sigma;
    return expf(-.5f * x * x / s);
}

size_t normalize_index(int p, size_t i_size, KernelMode mode) {
    int n = p;

    switch (mode){
    case MODE_WRAP:
        if (n<0) n = i_size - ((-n) % i_size);
        if (n>=i_size) n %= i_size;
        break;

    case MODE_REFLECT:
        while (n<0 || n>=i_size) {
            if (n==-1) n = 0;
            if (n<-1) n = 1;
            if (n==i_size) n = i_size - 2;
            if (n>i_size) n = i_size - 3;
        }
        break;

    case MODE_MIRROR:
        while (n<0 || n>=i_size) {
            if (n<0) n = -n;
            if (n>=i_size) n = 2 * i_size - n;
        }
        break;
    }

    return (size_t)n;
}

void create_kernel(float sigma, float** kernel, size_t* size) {
    int lw = (int)(4.f * sigma + .5f);
    (*size) = lw * 2 + 1;
    (*kernel) = new float[(*size)];

    int k_size = (*size);
    float *k = (*kernel);

    int i;
    float s, f;

    s = 1.f;
    k[lw] = 1.f;
    for (i=1; i<=lw; i++) {
        f = gfunc((float)(i), sigma);
        k[lw-i] = f;
        k[lw+i] = f;
        s += f * 2.f;
    }

    for (i=0; i<k_size; i++) {
        k[i] /= s;
    }
}

void apply_filter(float src[], float dst[], size_t i_size, float k[], size_t k_size, KernelMode mode) {
    int k2 = k_size / 2;

    size_t max_size = i_size * i_size;
    float* tmp = new float[max_size];
    
    int i, j;
    float d;

    int l;
    
#pragma omp parallel for private(i,j,d) shared(src,tmp,k)
    for (l=0; l<max_size; l++) {
        i = l % i_size;
        j = l / i_size;

        d = 0.f;

        for (int n=0; n<k_size; n++) {
            int p = i + n - k2;
            p = normalize_index(p, i_size, mode);
            d += src[p+j*i_size] * k[n];
        }

        tmp[i+j*i_size] = d;
    }
    
#pragma omp parallel for private(i,j,d) shared(tmp,dst,k)
    for (l=0; l<max_size; l++) {
        i = l % i_size;
        j = l / i_size;

        d = 0.f;

        for (int n=0; n<k_size; n++) {
            int p = i + n - k2;
            p = normalize_index(p, i_size, mode);
            d += tmp[j+p*i_size] * k[n];
        }

        dst[j+i*i_size] = d;
    }

    delete[] tmp;
}

void gaussian_filter(float a[], float b[], size_t i_size, float sigma, KernelMode mode) {
    size_t k_size;
    float* kernel;
    create_kernel(sigma, &kernel, &k_size);

    apply_filter(a, b, i_size, kernel, k_size, mode);

    delete[] kernel;
}

void apply_filter(unsigned char* texture, size_t tex_size, unsigned char BitsPerPixel,
                  float k[], size_t k_size, KernelMode mode) {
    int k2 = k_size / 2;
    int i, j, l;
    float r, g, b;

    size_t pixels_count = tex_size * tex_size;
    unsigned char* tmp = new unsigned char[pixels_count * BitsPerPixel];
    
#pragma omp parallel for private(i,j,r,g,b) shared(tmp,texture,k)
    for (l=0; l<pixels_count; l++) {
        j = l / tex_size;
        i = l % tex_size;

        r = g = b = 0.f;

        for (int n=0; n<k_size; n++) {
            int p = i + n - k2;
            p = normalize_index(p, tex_size, mode);

            r += texture[(p+j*tex_size)*BitsPerPixel+0] * k[n];
            g += texture[(p+j*tex_size)*BitsPerPixel+1] * k[n];
            b += texture[(p+j*tex_size)*BitsPerPixel+2] * k[n];
        }

        tmp[(i+j*tex_size)*BitsPerPixel+0] = r;
        tmp[(i+j*tex_size)*BitsPerPixel+1] = g;
        tmp[(i+j*tex_size)*BitsPerPixel+2] = b;
    }
    
#pragma omp parallel for private(i,j,r,g,b) shared(tmp,texture,k)
    for (l=0; l<pixels_count; l++) {
        j = l / tex_size;
        i = l % tex_size;

        r = g = b = 0.0;

        for (int n=0; n<k_size; n++) {
            int p = i + n - k2;
            p = normalize_index(p, tex_size, mode);

            r += tmp[(j+p*tex_size)*BitsPerPixel+0] * k[n];
            g += tmp[(j+p*tex_size)*BitsPerPixel+1] * k[n];
            b += tmp[(j+p*tex_size)*BitsPerPixel+2] * k[n];
        }

        texture[(j+i*tex_size)*BitsPerPixel+0] = r;
        texture[(j+i*tex_size)*BitsPerPixel+1] = g;
        texture[(j+i*tex_size)*BitsPerPixel+2] = b;
    }

    delete[] tmp;
}

void gaussian_filter(unsigned char* texture, size_t tex_size, unsigned char BitsPerPixel,
                     float sigma, KernelMode mode) {
    size_t k_size;
    float* kernel;
    create_kernel(sigma, &kernel, &k_size);

    apply_filter(texture, tex_size, BitsPerPixel, kernel, k_size, mode);

    delete[] kernel;
}
