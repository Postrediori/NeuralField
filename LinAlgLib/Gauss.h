#pragma once

enum KernelMode {
    MODE_WRAP,
    MODE_REFLECT,
    MODE_MIRROR
};

float gfunc(float x, float sigma);
size_t normalize_index(int p, size_t i_size, KernelMode mode);
void create_kernel(float sigma, float** kernel, size_t* size);

void apply_filter(float src[], float dst[], size_t i_size, float k[], size_t k_size, KernelMode mode);
void gaussian_filter(float a[], float b[], size_t i_size, float sigma, KernelMode mode = MODE_WRAP);

void apply_filter(unsigned char* texture, size_t tex_size, unsigned char BitsPerPixel,
    float k[], size_t k_size, KernelMode mode);
void gaussian_filter(unsigned char* texture, size_t tex_size, unsigned char BitsPerPixel,
    float sigma, KernelMode mode = MODE_WRAP);
