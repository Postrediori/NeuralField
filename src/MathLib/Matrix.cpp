#include "stdafx.h"
#include "Matrix.h"

#ifdef _MSC_VER
double drand48() {
    return ((double)rand() / (double)RAND_MAX);
}
#endif

/*****************************************************************************
 * Memory allocation
 ****************************************************************************/

matrix_t* matrix_allocate(size_t rows, size_t cols) {
    matrix_t* m = new matrix_t;
    if (!m) {
        LOGE << "MATRIX ALLOCATION ERROR";
        return nullptr;
    }
    m->rows = rows;
    m->cols = cols;
    m->data = new double[rows * cols];
    return m;
}

void matrix_free(matrix_t* m) {
    if (!m) {
        LOGE << "MATRIX NULL ERROR";
        return;
    }
    if (m->data) {
        delete[] m->data;
    }
    delete m;
}

matrix_t* matrix_set(matrix_t* a, size_t row, size_t col, double val) {
    if (a == nullptr) {
        LOGE << "Matrix NULL Error";
        return nullptr;
    }
    if (a->rows <= row) {
        LOGE << "Matrix ROWS Error";
        return a;
    }
    if (a->cols <= col) {
        LOGE << "Matrix COLS Error";
        return a;
    }
    a->data[row * a->cols + col] = val;
    return a;
}

/*****************************************************************************
 * Matrix algebra
 ****************************************************************************/
matrix_t* matrix_scalar_set(matrix_t* a, double h) {
    if (a == nullptr) {
        LOGE << "Matrix NULL Error";
        return nullptr;
    }
#pragma omp parallel for
    for (size_t idx = 0; idx < a->rows * a->cols; idx++) {
        a->data[idx] = h;
    }
    return a;
}

matrix_t* matrix_scalar_add(matrix_t* a, double h) {
    if (a == nullptr) {
        LOGE << "Matrix NULL Error";
        return a;
    }
#pragma omp parallel for
    for (size_t idx = 0; idx < a->rows * a->cols; idx++) {
        a->data[idx] += h;
    }
    return a;
}

matrix_t* matrix_scalar_mul(matrix_t* a, double h) {
    if (a == nullptr) {
        LOGE << "Matrix NULL Error";
        return a;
    }
#pragma omp parallel for
    for (size_t idx = 0; idx < a->rows * a->cols; idx++) {
        a->data[idx] *= h;
    }
    return a;
}

matrix_t* matrix_add(matrix_t* a, matrix_t* b) {
    if (a == nullptr || b == nullptr) {
        LOGE << "Matrix NULL Error";
        return a;
    }
    if (a->rows != b->rows) {
        LOGE << "Matrix ROWS Error";
        return a;
    }
    if (a->cols != b->cols) {
        LOGE << "Matrix COLUMNS Error";
        return a;
    }
#pragma omp parallel for
    for (size_t idx = 0; idx < a->rows * a->cols; idx++) {
        a->data[idx] += b->data[idx];
    }
    return a;
}

matrix_t* matrix_sub(matrix_t* a, matrix_t* b) {
    if (a == nullptr || b == nullptr) {
        LOGE << "Matrix NULL Error";
        return a;
    }
    if (a->rows != b->rows) {
        LOGE << "Matrix ROWS Error";
        return a;
    }
    if (a->cols != b->cols) {
        LOGE << "Matrix COLUMNS Error";
        return a;
    }
#pragma omp parallel for
    for (size_t idx = 0; idx < a->rows * a->cols; idx++) {
        a->data[idx] -= b->data[idx];
    }
    return a;
}

matrix_t* matrix_heaviside(matrix_t* a) {
    if (a == nullptr) {
        LOGE << "Matrix NULL Error";
        return a;
    }
#pragma omp parallel for
    for (size_t idx = 0; idx < a->rows * a->cols; idx++) {
        a->data[idx] = (a->data[idx] > 0.0) ? 1.0 : 0.0;
    }
    return a;
}

matrix_t* matrix_random_f(matrix_t* a) {
    if (a == nullptr) {
        LOGE << "Matrix NULL Error";
        return a;
    }
#pragma omp parallel for
    for (size_t idx = 0; idx < a->rows * a->cols; idx++) {
        a->data[idx] = drand48();
    }
    return a;
}
