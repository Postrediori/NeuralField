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

AmariModel::~AmariModel() {
    //
}

bool AmariModel::init(const char* config_file) {
    if (!load_config(config_file)) {
        return false;
    }

    sigma_k = 1.0 / sqrtf(2.0 * k);
    sigma_m = 1.0 / sqrtf(2.0 * m);
    pi_k = K_ * M_PI / k;
    pi_m = M_ * M_PI / m;

    excitement_kernel = kernel_create(sigma_k);
    inhibition_kernel = kernel_create(sigma_m);

    stimulus = matrix_allocate(size, size);
    activity = matrix_allocate(size, size);
    excitement = matrix_allocate(size, size);
    inhibition = matrix_allocate(size, size);

    restart();

    return true;
}

void AmariModel::restart() {
    matrix_random_f(stimulus);
    matrix_scalar_mul(stimulus, -h);

    matrix_scalar_set(activity, h);
    matrix_scalar_set(excitement, 0.0);
    matrix_scalar_set(inhibition, 0.0);
}

void AmariModel::release() {
    matrix_free(stimulus);
    matrix_free(activity);
    matrix_free(excitement);
    matrix_free(inhibition);
    stimulus = nullptr;
    activity = nullptr;
    excitement = nullptr;
    inhibition = nullptr;

    kernel_free(excitement_kernel);
    kernel_free(inhibition_kernel);
    excitement_kernel = nullptr;
    inhibition_kernel = nullptr;
}

void AmariModel::stimulate() {
    matrix_heaviside(activity);

    kernel_apply_to_matrix(excitement, activity, excitement_kernel, mode);
    matrix_scalar_mul(excitement, pi_k);

    kernel_apply_to_matrix(inhibition, activity, inhibition_kernel, mode);
    matrix_scalar_mul(inhibition, pi_m);

    matrix_scalar_set(activity, h);
    matrix_add(activity, excitement);
    matrix_sub(activity, inhibition);
    matrix_add(activity, stimulus);
}

bool AmariModel::load_config(const char* config_file) {
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
        if (line[0]=='/' || line[0]=='\n' || line[0]=='\r') {
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
    activity->data[y * activity->cols + x] = a;
}
