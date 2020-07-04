#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "Gauss.h"
#include "NeuralFieldModel.h"

bool ParseConfigFile(ConfigMap_t& map, const std::string& fileName) {
    std::ifstream in(fileName, std::ios::in);
    if (!in.is_open()) {
        LOGE << "Unable to load Amari Model Config File " << fileName;
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

        if (line.find("h = ") == 0) {
            sscanf(line.c_str(), "h = %lf\n", &fval);
            map["h"] = fval;

        }
        else if (line.find("k = ") == 0) {
            sscanf(line.c_str(), "k = %lf\n", &fval);
            map["k"] = fval;

        }
        else if (line.find("K = ") == 0) {
            sscanf(line.c_str(), "K = %lf\n", &fval);
            map["K_"] = fval;

        }
        else if (line.find("m = ") == 0) {
            sscanf(line.c_str(), "m = %lf\n", &fval);
            map["m"] = fval;

        }
        else if (line.find("M = ") == 0) {
            sscanf(line.c_str(), "M = %lf\n", &fval);
            map["M_"] = fval;

        }
        else if (line.find("mode = ") == 0) {
            sscanf(line.c_str(), "mode = %s\n", sval);
            if (strcmp(sval, "wrap") == 0) {
                map["mode"] = MODE_WRAP;

            }
            else if (strcmp(sval, "reflect") == 0) {
                map["mode"] = MODE_REFLECT;

            }
            else if (strcmp(sval, "mirror") == 0) {
                map["mode"] = MODE_MIRROR;
            }

        }
        else if (line.find("size = ") == 0) {
            sscanf(line.c_str(), "size = %d\n", &ival);
            map["size"] = ival;
        }
    }

    return true;
}

NeuralFieldModel::NeuralFieldModel()
    : h(-0.1)
    , k(0.05)
    , K_(0.125)
    , m(0.025)
    , M_(0.065)
    , mode(MODE_REFLECT) {
}

bool NeuralFieldModel::init(const ConfigMap_t& configMap) {
    this->h = configMap.at("h");
    this->k = configMap.at("k");
    this->K_ = configMap.at("K_");
    this->m = configMap.at("m");
    this->M_ = configMap.at("M_");
    this->mode = (KernelMode)static_cast<int>(configMap.at("mode"));
    this->size = (size_t)configMap.at("size");

    sigma_k = 1.0 / sqrtf(2.0 * k);
    sigma_m = 1.0 / sqrtf(2.0 * m);
    pi_k = K_ * M_PI / k;
    pi_m = M_ * M_PI / m;

    excitement_kernel = std::move(KernelGuard_t(kernel_create(sigma_k, mode), kernel_free));
    inhibition_kernel = std::move(KernelGuard_t(kernel_create(sigma_m, mode), kernel_free));

    stimulus = std::move(MatrixGuard_t(matrix_allocate(size, size), matrix_free));
    activity = std::move(MatrixGuard_t(matrix_allocate(size, size), matrix_free));
    excitement = std::move(MatrixGuard_t(matrix_allocate(size, size), matrix_free));
    inhibition = std::move(MatrixGuard_t(matrix_allocate(size, size), matrix_free));

    restart();

    return true;
}

void NeuralFieldModel::restart() {
    matrix_random_f(stimulus.get());
    matrix_scalar_mul(stimulus.get(), -h);

    matrix_scalar_set(activity.get(), h);
    matrix_scalar_set(excitement.get(), 0.0);
    matrix_scalar_set(inhibition.get(), 0.0);
}

void NeuralFieldModel::release() {
    stimulus.release();
    activity.release();
    excitement.release();
    inhibition.release();

    excitement_kernel.release();
    inhibition_kernel.release();
}

void NeuralFieldModel::stimulate() {
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

void NeuralFieldModel::set_activity(size_t x, size_t y, float a) {
    matrix_set(activity.get(), y, x, a);
}
