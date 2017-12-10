#pragma once

#include "Matrix.h"
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

    double h;
    double k, K_;
    double sigma_k;
    double pi_k;

    double m, M_;
    double sigma_m;
    double pi_m;

    KernelMode mode;

    //size_t excitement_kernel_size;
    //float* excitement_kernel;
    kernel_t* excitement_kernel;

    //size_t inhibition_kernel_size;
    //float* inhibition_kernel;
    kernel_t* inhibition_kernel;

    //float* stimulus;
    //float* activity;
    //float* excitement;
    //float* inhibition;
    matrix_t* stimulus;
    matrix_t* activity;
    matrix_t* excitement;
    matrix_t* inhibition;
};
