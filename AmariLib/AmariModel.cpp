#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"
#include "AmariModel.h"

inline float frand1000() {
     return (float)(rand() % 1000) / 1000.f;
}

/*****************************************************************************
 * Amari model
 ****************************************************************************/
AmariModel::AmariModel()
    : h(-0.1)
    , k(0.05)
    , K_(0.125)
    , m(0.025)
    , M_(0.065)
    , mode(MODE_REFLECT) {
}

bool AmariModel::init(const std::string& config_file) {
    if (!load_config(config_file)) {
        return false;
    }

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

    restart();

    return true;
}

void AmariModel::restart() {
    matrix_random_f(stimulus.get());
    matrix_scalar_mul(stimulus.get(), -h);

    matrix_scalar_set(activity.get(), h);
    matrix_scalar_set(excitement.get(), 0.0);
    matrix_scalar_set(inhibition.get(), 0.0);
}

void AmariModel::release() {
    stimulus.release();
    activity.release();
    excitement.release();
    inhibition.release();

    excitement_kernel.release();
    inhibition_kernel.release();
}

void AmariModel::stimulate() {
    matrix_heaviside(activity.get());

    kernel_apply_to_matrix(excitement.get(), activity.get(), excitement_kernel.get());
    matrix_scalar_mul(excitement.get(), pi_k);

    kernel_apply_to_matrix(inhibition.get(), activity.get(), inhibition_kernel.get());
    matrix_scalar_mul(inhibition.get(), pi_m);

    matrix_scalar_set(activity.get(), h);
    matrix_add(activity.get(), excitement.get());
    matrix_sub(activity.get(), inhibition.get());
    matrix_add(activity.get(), stimulus.get());
}

bool AmariModel::load_config(const std::string& config_file) {
    std::ifstream in(config_file, std::ios::in);
    if (!in) {
        LOGE << "Unable to load Amari Model Config File " << config_file;
        return false;
    }

    int ival;
    double fval;
    char sval[256];

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line.find("#") == 0) {
            continue;
        }

        if (line.find("h = ")==0) {
            sscanf(line.c_str(), "h = %lf\n", &fval);
            this->h = fval;

        } else if (line.find("k = ")==0) {
            sscanf(line.c_str(), "k = %lf\n", &fval);
            this->k = fval;

        } else if (line.find("K = ")==0) {
            sscanf(line.c_str(), "K = %lf\n", &fval);
            this->K_ = fval;

        } else if (line.find("m = ")==0) {
            sscanf(line.c_str(), "m = %lf\n", &fval);
            this->m = fval;

        } else if (line.find("M = ")==0) {
            sscanf(line.c_str(), "M = %lf\n", &fval);
            this->M_ = fval;

        } else if (line.find("mode = ")==0) {
            sscanf(line.c_str(), "mode = %s\n", &sval);
            if (strcmp(sval, "wrap")==0) {
                this->mode = MODE_WRAP;
                
            } else if (strcmp(sval, "reflect")==0) {
                this->mode = MODE_REFLECT;
                
            } else if (strcmp(sval, "mirror")==0) {
                this->mode = MODE_MIRROR;
            }

        } else if (line.find("size = ")==0) {
            sscanf(line.c_str(), "size = %d\n", &ival);
            this->size = ival;
        }
    }
    
    in.close();
    
    return true;
}

void AmariModel::set_activity(size_t x, size_t y, float a) {
    matrix_set(activity.get(), y, x, a);
}
