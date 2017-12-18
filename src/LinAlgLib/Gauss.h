#pragma once

#include <functional>
#include "Texture.h"

enum KernelMode {
    MODE_WRAP,
    MODE_REFLECT,
    MODE_MIRROR
};

struct kernel_t {
    size_t size;
    double sigma;
    KernelMode mode;
    double* data;
};

typedef std::unique_ptr<kernel_t, std::function<void(kernel_t*)>> KernelGuard_t;

kernel_t* kernel_alloc(size_t size);
void kernel_free(kernel_t* k);

kernel_t* kernel_create(double sigma, KernelMode mode);

matrix_t* kernel_apply_to_matrix(matrix_t* dst, matrix_t* src, kernel_t* k);
matrix_t* kernel_filter_matrix(matrix_t* dst, matrix_t* src, KernelMode mode);

texture_t* kernel_apply_to_texture(texture_t* t, kernel_t* k);
texture_t* kernel_filter_texture(texture_t* t, double sigma, KernelMode mode);
