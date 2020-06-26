#pragma once

class NeuralFieldModel {
public:
    NeuralFieldModel();

    bool init(const std::string& config_file);
    bool load_config(const std::string& config_file);

    void release();

    void restart();
    void stimulate();

    void set_activity(size_t x, size_t y, float a);
    
public:
    size_t size;
    size_t data_size;

    double h;
    double k, K_;
    double sigma_k;
    double pi_k;

    double m, M_;
    double sigma_m;
    double pi_m;

    KernelMode mode;

    KernelGuard_t excitement_kernel;
    KernelGuard_t inhibition_kernel;

    MatrixGuard_t stimulus;
    MatrixGuard_t activity;
    MatrixGuard_t excitement;
    MatrixGuard_t inhibition;
};
