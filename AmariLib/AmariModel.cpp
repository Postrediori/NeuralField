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
    : h(-0.1f)
    , k(0.05f)
    , K_(0.125f)
    , m(0.025f)
    , M_(0.065f)
    , mode(MODE_REFLECT) {
}

AmariModel::~AmariModel() {
    //
}

bool AmariModel::init(const char* config_file) {
    if (!load_config(config_file)) {
        return false;
    }

    sigma_k = 1.f / sqrtf(2.f * this->k);
    sigma_m = 1.f / sqrtf(2.f * this->m);
    pi_k = this->K_ * M_PI / this->k;
    pi_m = this->M_ * M_PI / this->m;

    create_kernel(sigma_k, &excitement_kernel, &excitement_kernel_size);
    create_kernel(sigma_m, &inhibition_kernel, &inhibition_kernel_size);

    this->stimulus = new float[this->data_size];
    this->activity = new float[this->data_size];
    this->excitement = new float[this->data_size];
    this->inhibition = new float[this->data_size];

    this->restart();

    return true;
}

void AmariModel::restart() {
    matrix_scalar_set(this->stimulus, this->size, -this->h);

    for (int i=0; i<this->data_size; i++) {
        this->stimulus[i] *= frand1000();
    }

    matrix_scalar_set(this->activity,   this->size, this->h);
    matrix_scalar_set(this->excitement, this->size, 0.0f);
    matrix_scalar_set(this->inhibition, this->size, 0.0f);
}

void AmariModel::release() {
    delete[] this->stimulus;
    delete[] this->activity;
    delete[] this->excitement;
    delete[] this->inhibition;

    delete[] this->excitement_kernel;
    delete[] this->inhibition_kernel;
}

void AmariModel::stimulate() {
    matrix_heaviside(this->activity, this->size);

    apply_filter(this->activity, this->excitement, this->size,
                 excitement_kernel, excitement_kernel_size, mode);
    matrix_scalar_mul(this->excitement, this->size, this->pi_k);

    apply_filter(this->activity, this->inhibition, this->size,
                 inhibition_kernel, inhibition_kernel_size, mode);
    matrix_scalar_mul(this->inhibition, this->size, this->pi_m);

    matrix_scalar_set(this->activity, this->size, this->h);
    matrix_add(this->activity, this->excitement, this->size);
    matrix_sub(this->activity, this->inhibition, this->size);
    matrix_add(this->activity, this->stimulus, this->size);
}

bool AmariModel::load_config(const char* config_file) {
    std::ifstream in(config_file, std::ios::in);
    if (!in) {
        LOGE << "Unable to load AMari Model Config File " << config_file;
        return false;
    }

    int ival;
    float fval;
    char sval[256];

    std::string line;
    while (std::getline(in, line)) {
        if (line[0]=='/' || line[0]=='\n' || line[0]=='\r') {
            continue;
        }

        if (strncmp(line.c_str(), "h = ", 4)==0) {
            sscanf(line.c_str(), "h = %f\n", &fval);
            this->h = fval;

        } else if (strncmp(line.c_str(), "k = ", 4)==0) {
            sscanf(line.c_str(), "k = %f\n", &fval);
            this->k = fval;

        } else if (strncmp(line.c_str(), "K = ", 4)==0) {
            sscanf(line.c_str(), "K = %f\n", &fval);
            this->K_ = fval;

        } else if (strncmp(line.c_str(), "m = ", 4)==0) {
            sscanf(line.c_str(), "m = %f\n", &fval);
            this->m = fval;

        } else if (strncmp(line.c_str(), "M = ", 4)==0) {
            sscanf(line.c_str(), "M = %f\n", &fval);
            this->M_ = fval;

        } else if (strncmp(line.c_str(), "mode = ", 7)==0) {
            sscanf(line.c_str(), "mode = %s\n", &sval);
            if (strcmp(sval, "wrap")==0) {
                this->mode = MODE_WRAP;
            } else if (strcmp(sval, "reflect")==0) {
                this->mode = MODE_REFLECT;
            } else if (strcmp(sval, "mirror")==0) {
                this->mode = MODE_MIRROR;
            }

        } else if (strncmp(line.c_str(), "size = ", 7)==0) {
            sscanf(line.c_str(), "size = %d\n", &ival);
            this->size = ival;
            this->data_size = this->size * this->size;
        }
    }
    
    in.close();
    
    return true;
}

void AmariModel::set_activity(size_t x, size_t y, float a) {
    this->activity[y * this->size + x] = a;
}
