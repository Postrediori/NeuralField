#pragma once

enum KernelMode : int {
    MODE_WRAP = 0,
    MODE_REFLECT = 1,
    MODE_MIRROR = 2
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
matrix_t* kernel_filter_matrix(matrix_t* dst, matrix_t* src, double sigma, KernelMode mode);
