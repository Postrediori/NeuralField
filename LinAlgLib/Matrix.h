#pragma once

#include <functional>

struct matrix_t {
    size_t rows;
    size_t cols;
    double* data;
};

typedef std::unique_ptr<matrix_t, std::function<void(matrix_t*)>> MatrixGuard_t;

matrix_t* matrix_allocate(size_t rows, size_t cols);
void matrix_free(matrix_t* m);

matrix_t* matrix_scalar_set(matrix_t* a, double h);
matrix_t* matrix_scalar_add(matrix_t* a, double h);
matrix_t* matrix_scalar_mul(matrix_t* a, double h);

matrix_t* matrix_add(matrix_t* a, matrix_t* b);
matrix_t* matrix_sub(matrix_t* a, matrix_t* b);

matrix_t* matrix_heaviside(matrix_t* a);
matrix_t* matrix_random_f(matrix_t* a);
