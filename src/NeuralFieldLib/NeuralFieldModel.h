#pragma once

using NeuralFieldModelParams = std::map<std::string, double>;

class NeuralFieldModel {
public:
    NeuralFieldModel() = default;
    ~NeuralFieldModel();

#ifdef USE_OPENCL
    bool InitOpenCLContext(cl_platform_id platformId, cl_device_id device,
        cl_context context, cl_command_queue commandQueue);
#endif
    bool Init(const NeuralFieldModelParams& params);

    void Release();

    void Restart();
    void Stimulate();

    void SetActivity(size_t x, size_t y, float a);

#ifdef USE_OPENCL
    bool IsEnabledOpenCL() const { return isEnabledOpenCL; }

    void CalcHeaviside(cl_mem memDst, cl_uint matrixSize);
    void GaussianBlur(cl_mem memDst, cl_mem memSrc, cl_mem memTmp, cl_uint matrixSize, cl_mem memKernel, cl_uint kernelSize);
#endif

#ifdef USE_OPENCL
private:
    // OpenCL
    bool InitOpenCLObjects();
    void ReleaseOpenCLObjects();

    bool InitOpenCLBuffers();
    void ReleaseOpenCLBuffers();

    void CalcExcitementMatrix();
    void CalcInhibitionMatrix();
    void CalcActivity();
#endif

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

#ifdef USE_OPENCL
    cl_platform_id platformId = 0;
    cl_device_id device = 0;
    cl_context context = 0;
    cl_command_queue commandQueue = 0;

    cl_mem memActivityMatrix = 0;
    cl_mem memTempMatrix = 0;
#endif

#ifdef USE_OPENCL
private:
    // OpenCL context and objects
    bool isEnabledOpenCL = false;

    cl_mem memExcitementMatrix = 0;
    cl_mem memInhibitionMatrix = 0;
    cl_mem memStimulusMatrix = 0;

    cl_mem memExcitementKernel = 0;
    cl_mem memInhibitionKernel = 0;

    cl_program heavisideProgram = 0;
    cl_kernel heavisideKernel = 0;

    cl_program gaussianBlurProgram = 0;
    cl_kernel gaussianBlurKernel = 0;

    cl_program stimulationProgram = 0;
    cl_kernel stimulationKernel = 0;
#endif
};
