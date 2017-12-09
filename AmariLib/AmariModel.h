#pragma once

#include "Gauss.h"

class AmariModel {
public:
    AmariModel();
    ~AmariModel();

    bool init(const char* config_file);
    bool load_config(const char* config_file);

    void release();

    void restart();
    void stimulate();

    void set_activity(size_t x, size_t y, float a);
    
public:
    size_t size;
    size_t data_size;

    float h;
    float k, K_;
    float sigma_k;
    float pi_k;

    float m, M_;
    float sigma_m;
    float pi_m;

    KernelMode mode;

    size_t excitement_kernel_size;
    float* excitement_kernel;

    size_t inhibition_kernel_size;
    float* inhibition_kernel;

    float* stimulus;
    float* activity;
    float* excitement;
    float* inhibition;
};
