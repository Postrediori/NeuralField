#include "stdafx.h"
#include "Matrix.h"

/*****************************************************************************
 * Matrix algebra
 ****************************************************************************/
void matrix_scalar_set(float a[], size_t i_size, float h) {
#pragma omp parallel for
    for (size_t idx = 0; idx < i_size * i_size; idx++) {
        a[idx] = h;
    }
}

void matrix_scalar_add(float a[], size_t i_size, float h) {
#pragma omp parallel for
    for (size_t idx = 0; idx < i_size * i_size; idx++) {
        a[idx] += h;
    }
}

void matrix_scalar_mul(float a[], size_t i_size, float h) {
#pragma omp parallel for
    for (size_t idx = 0; idx < i_size * i_size; idx++) {
        a[idx] *= h;
    }
}

void matrix_add(float a[], float b[], size_t i_size) {
#pragma omp parallel for
    for (size_t idx = 0; idx < i_size * i_size; idx++) {
        a[idx] += b[idx];
    }
}

void matrix_sub(float a[], float b[], size_t i_size) {
#pragma omp parallel for
    for (size_t idx = 0; idx < i_size * i_size; idx++) {
        a[idx] -= b[idx];
    }
}

void matrix_heaviside(float a[], size_t i_size) {
#pragma omp parallel for
    for (size_t idx = 0; idx < i_size * i_size; idx++) {
        a[idx] = (a[idx] > 0.f) ? 1.f : 0.f;
    }
}

void matrix_random_f(float a[], size_t i_size) {
#pragma omp parallel for
    for (size_t idx = 0; idx < i_size * i_size; idx++) {
        a[idx] = (float)(rand() % 1000) / 1000.f;
    }
}

