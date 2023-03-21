#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"
#include "NeuralFieldModel.h"

bool NeuralFieldModel::Init(const NeuralFieldModelParams& params) {
    this->h = params.at("h");
    this->k = params.at("k");
    this->K_ = params.at("Kp");
    this->m = params.at("m");
    this->M_ = params.at("Mp");
    // this->mode = (KernelMode)static_cast<int>(params.at("mode"));
    this->size = (size_t)params.at("size");

    sigma_k = 1.0 / sqrtf(2.0 * k);
    sigma_m = 1.0 / sqrtf(2.0 * m);
    pi_k = K_ * M_PI / k;
    pi_m = M_ * M_PI / m;

    excitement_kernel = KernelGuard_t(kernel_create(sigma_k, mode), kernel_free);
    inhibition_kernel = KernelGuard_t(kernel_create(sigma_m, mode), kernel_free);

    stimulus = MatrixGuard_t(matrix_allocate(size, size), matrix_free);
    activity = MatrixGuard_t(matrix_allocate(size, size), matrix_free);
    excitement = MatrixGuard_t(matrix_allocate(size, size), matrix_free);
    inhibition = MatrixGuard_t(matrix_allocate(size, size), matrix_free);
    temp = MatrixGuard_t(matrix_allocate(size, size), matrix_free);

    Restart();

    return true;
}

void NeuralFieldModel::Restart() {
    matrix_random_f(stimulus.get());
    matrix_scalar_mul(stimulus.get(), -h);

    matrix_scalar_set(activity.get(), h);
    matrix_scalar_set(excitement.get(), 0.0);
    matrix_scalar_set(inhibition.get(), 0.0);
}

void NeuralFieldModel::Release() {
    stimulus.release();
    activity.release();
    excitement.release();
    inhibition.release();

    excitement_kernel.release();
    inhibition_kernel.release();
}

void NeuralFieldModel::Stimulate() {
    matrix_heaviside(activity.get());

    kernel_apply_to_matrix(excitement.get(), activity.get(), temp.get(), excitement_kernel.get());
    matrix_scalar_mul(excitement.get(), pi_k);

    kernel_apply_to_matrix(inhibition.get(), activity.get(), temp.get(), inhibition_kernel.get());
    matrix_scalar_mul(inhibition.get(), pi_m);

    matrix_scalar_set(activity.get(), h);
    matrix_add(activity.get(), excitement.get());
    matrix_sub(activity.get(), inhibition.get());
    matrix_add(activity.get(), stimulus.get());
}

void NeuralFieldModel::SetActivity(size_t x, size_t y, float a) {
    matrix_set(activity.get(), y, x, a);
}
