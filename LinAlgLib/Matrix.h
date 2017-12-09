#pragma once

void matrix_scalar_set(float a[], size_t i_size, float h);
void matrix_scalar_add(float a[], size_t i_size, float h);
void matrix_scalar_mul(float a[], size_t i_size, float h);
void matrix_add(float a[], float b[], size_t i_size);
void matrix_sub(float a[], float b[], size_t i_size);

void matrix_heaviside(float a[], size_t i_size);
void matrix_random_f(float a[], size_t i_size);
