#pragma once

using NeuralFieldModelParams = std::map<std::string, double>;

class NeuralFieldModel {
public:
    NeuralFieldModel() = default;

    bool Init(const NeuralFieldModelParams& params);

    void Release();

    void Restart();
    void Stimulate();

    void SetActivity(size_t x, size_t y, float a);

public:
    size_t size = 0;
    size_t data_size = 0;

    double h = -0.1;
    double k = 0.05, K_ = 0.125;
    double sigma_k = 0.0;
    double pi_k = 0.0;

    double m = 0.025, M_ = 0.065;
    double sigma_m = 0.0;
    double pi_m = 0.0;

    KernelMode mode = MODE_REFLECT;

    KernelGuard_t excitement_kernel;
    KernelGuard_t inhibition_kernel;

    MatrixGuard_t stimulus;
    MatrixGuard_t activity;
    MatrixGuard_t excitement;
    MatrixGuard_t inhibition;
    MatrixGuard_t temp;
};
