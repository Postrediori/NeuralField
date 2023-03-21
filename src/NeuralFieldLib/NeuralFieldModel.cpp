#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"
#ifdef USE_OPENCL
#include "ParallelUtils.h"
#endif
#include "NeuralFieldModel.h"

#ifdef USE_OPENCL

/*
 * Heaviside kernel
 */
static const std::string HeavisideKernelName = "HeavisideMatrix";
static const std::string HeavisideKernelSource = R"opencl(
__kernel void HeavisideMatrix(__global float *m)
{
    size_t idx = get_global_id(0);
    m[idx] = (m[idx] > 0.0f) ? 1.0f : 0.0f;
}
)opencl";

/*
 * Stimulation kernel
 */
static const std::string StimulationKernelName = "Stimulation";
static const std::string StimulationKernelSource = R"opencl(
__kernel void Stimulation(__global const float *e, __global const float *i, __global const float *s,
    float h, float pik, float pim, __global float *out)
{
    size_t idx = get_global_id(0);
    out[idx] = h + e[idx] * pik - i[idx] * pim + s[idx];
}
)opencl";

/*
 * Gaussian blur kenel
 */
static const std::string GaussianBlurKernelName = "GaussianBlur";
static const std::string GaussianBlurKernelSource = R"opencl(
#define HORIZONTAL_BLUR 0
#define VERTICAL_BLUR 1

int wrap_index(int n, uint size)
{
    if (n < 0) {
        return (n + size);
    }
    else if (n >= size) {
        return (n % size);
    }
    return n;
}

__kernel void GaussianBlur(__global const float *m, __global const float *k,
                     uint nmatrix, uint nkernel, int dir, __global float *out)
{
    size_t idx = get_global_id(0);

    int row = idx / nmatrix;
    int col = idx % nmatrix;
    int ofs = row * nmatrix;

    uint k2 = nkernel / 2;
    float sum = 0.f;

    if (dir == HORIZONTAL_BLUR) {
        for (uint j = 1; j < k2; j++)
        {
            int col1 = col + j;
            int col2 = col - j;
        
            col1 = wrap_index(col1, nmatrix);
            col2 = wrap_index(col2, nmatrix);

            sum += (m[col1 + ofs] + m[col2 + ofs]) * k[k2 - j];
        }
    }
    else if (dir == VERTICAL_BLUR) {
        for (uint j = 1; j < k2; j++)
        {
            int row1 = row + j;
            int row2 = row - j;
        
            row1 = wrap_index(row1, nmatrix);
            row2 = wrap_index(row2, nmatrix);

            sum += (m[col + row1 * nmatrix] + m[col + row2 * nmatrix]) * k[k2 - j];
        }
    }
    sum += m[idx] * k[k2];

    out[idx] = sum;
}
)opencl";

#endif /* USE_OPENCL */

NeuralFieldModel::~NeuralFieldModel() {
#ifdef USE_OPENCL
    ReleaseOpenCLObjects();
#endif
}

#ifdef USE_OPENCL
bool NeuralFieldModel::InitOpenCLContext(cl_platform_id platformId, cl_device_id device,
    cl_context context, cl_command_queue commandQueue) {
    this->platformId = platformId;
    this->device = device;
    this->context = context;
    this->commandQueue = commandQueue;

    if (InitOpenCLObjects()) {
        LOGI << "Successfully created OpenCL objects for neural field simulation";
        isEnabledOpenCL = true;
    }
    else {
        LOGI << "Failed to init OpenCL programs for neural field simulation";
        isEnabledOpenCL = false;
    }

    return isEnabledOpenCL;
}
#endif

bool NeuralFieldModel::Init(const NeuralFieldModelParams& params) {
    if (params.find("h") != params.end()) {
        this->h = params.at("h");
    }
    if (params.find("k") != params.end()) {
        this->k = params.at("k");
    }
    if (params.find("Kp") != params.end()) {
        this->K_ = params.at("Kp");
    }
    if (params.find("m") != params.end()) {
        this->m = params.at("m");
    }
    if (params.find("Mp") != params.end()) {
        this->M_ = params.at("Mp");
    }
    if (params.find("mode") != params.end()) {
        this->mode = static_cast<KernelMode>(params.at("mode"));
    }
    if (params.find("size") != params.end()) {
        this->size = static_cast<size_t>(params.at("size"));
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
    temp = MatrixGuard_t(matrix_allocate(size, size), matrix_free);

#ifdef USE_OPENCL
    if (isEnabledOpenCL) {
        ReleaseOpenCLBuffers();
        if (!InitOpenCLBuffers()) {
            LOGE << "Error initialising OpenCL buffers. Disable OpenCL calculations";
            isEnabledOpenCL = false;
        }
    }
#endif

    Restart();

    return true;
}

void NeuralFieldModel::Restart() {
    matrix_random_f(stimulus.get());
    matrix_scalar_mul(stimulus.get(), -h);

    matrix_scalar_set(activity.get(), h);
    matrix_scalar_set(excitement.get(), 0.0);
    matrix_scalar_set(inhibition.get(), 0.0);

#ifdef USE_OPENCL
    if (isEnabledOpenCL) {
        cl_int status = CL_SUCCESS;

        status |= clEnqueueWriteBuffer(commandQueue, memStimulusMatrix, CL_FALSE, 0,
            sizeof(cl_float) * stimulus->dataSize, stimulus->data, 0, NULL, NULL);

        if (status != CL_SUCCESS) {
            LOGE << "Failed to setup OpenCL buffer queue : " << ParallelUtils::GetOpenCLError(status);
            LOGE << "Disable OpenCL";
            isEnabledOpenCL = false;
        }
    }
#endif
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
#ifdef USE_OPENCL
    if (!isEnabledOpenCL)
#endif
    {
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
#ifdef USE_OPENCL
    else {
        cl_int status;

        // Load activity matrix to OpenCL memory buffer
        status = clEnqueueWriteBuffer(commandQueue, memActivityMatrix, CL_FALSE, 0,
            sizeof(cl_float) * activity->dataSize, activity->data, 0, NULL, NULL);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to setup OpenCL buffer queue : " << ParallelUtils::GetOpenCLError(status);
            return;
        }

        // Activity = Heaviside(Activity)
        CalcHeaviside(memActivityMatrix, this->size);

        // Excitement = GaussianBlur(Activity, ExcitementKernel)
        CalcExcitementMatrix();

        // Inhibition = GaussianBlur(Activity, InhibitionKernel)
        CalcInhibitionMatrix();

        // Activity = h + Excitement * piK - Inhibition * piM + Stimulus
        CalcActivity();

        // Synchronous/blocking read of results
        status = clEnqueueReadBuffer(commandQueue, memActivityMatrix, CL_TRUE, 0,
            sizeof(cl_float) * activity->dataSize, activity->data, 0, NULL, NULL);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to read result buffer after OpenCL kernel run : " << ParallelUtils::GetOpenCLError(status);
        }
    }
#endif
}

void NeuralFieldModel::SetActivity(size_t x, size_t y, float a) {
    matrix_set(activity.get(), y, x, a);
}

#ifdef USE_OPENCL

bool NeuralFieldModel::InitOpenCLObjects() {

    // --------------------------------------------------------
    // Create the heaviside program
    if (!ParallelUtils::CreateProgram(context, device,
        HeavisideKernelName, HeavisideKernelSource,
        &heavisideProgram, &heavisideKernel)) {
        LOGE << "Failed to create matrix Heaviside program";
        return false;
    }

    // --------------------------------------------------------
    // Create the stimulation program
    if (!ParallelUtils::CreateProgram(context, device,
        StimulationKernelName, StimulationKernelSource,
        &stimulationProgram, &stimulationKernel)) {
        LOGE << "Failed to create model stimulation program";
        return false;
    }

    // --------------------------------------------------------
    // Create the gaussian blur program
    if (!ParallelUtils::CreateProgram(context, device,
        GaussianBlurKernelName, GaussianBlurKernelSource,
        &gaussianBlurProgram, &gaussianBlurKernel)) {
        LOGE << "Failed to create Gaussian blur program";
        return false;
    }

    return true;
}

void NeuralFieldModel::ReleaseOpenCLObjects() {
    ReleaseOpenCLBuffers();

    if (heavisideProgram) {
        clReleaseProgram(heavisideProgram);
        heavisideProgram = 0;
    }

    if (heavisideKernel) {
        clReleaseKernel(heavisideKernel);
        heavisideKernel = 0;
    }

    if (gaussianBlurProgram) {
        clReleaseProgram(gaussianBlurProgram);
        gaussianBlurProgram = 0;
    }

    if (gaussianBlurKernel) {
        clReleaseKernel(gaussianBlurKernel);
        gaussianBlurKernel = 0;
    }

    if (stimulationProgram) {
        clReleaseProgram(stimulationProgram);
        stimulationProgram = 0;
    }

    if (stimulationKernel) {
        clReleaseKernel(stimulationKernel);
        stimulationKernel = 0;
    }
}

bool NeuralFieldModel::InitOpenCLBuffers() {
    cl_int status, callStatus;

    // --------------------------------------------------------
    // Allocate the OpenCL buffer memory objects
    status = CL_SUCCESS;

    memExcitementMatrix = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * activity->dataSize, NULL, &callStatus);
    status |= callStatus;

    memInhibitionMatrix = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * activity->dataSize, NULL, &callStatus);
    status |= callStatus;

    memStimulusMatrix = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * activity->dataSize, NULL, &callStatus);
    status |= callStatus;

    memActivityMatrix = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * activity->dataSize, NULL, &callStatus);
    status |= callStatus;

    memTempMatrix = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * activity->dataSize, NULL, &callStatus);
    status |= callStatus;

    memInhibitionKernel = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * inhibition_kernel->size, NULL, &callStatus);
    status |= callStatus;

    memExcitementKernel = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * excitement_kernel->size, NULL, &callStatus);
    status |= callStatus;

    if (status != CL_SUCCESS) {
        LOGE << "Failed to create OpenCL memory buffer : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    // --------------------------------------------------------
    // Load kernels into memory objects

    status = CL_SUCCESS;

    status |= clEnqueueWriteBuffer(commandQueue, memExcitementKernel, CL_FALSE, 0,
        sizeof(cl_float) * excitement_kernel->size, excitement_kernel->data, 0, NULL, NULL);

    status |= clEnqueueWriteBuffer(commandQueue, memInhibitionKernel, CL_FALSE, 0,
        sizeof(cl_float) * inhibition_kernel->size, inhibition_kernel->data, 0, NULL, NULL);

    if (status != CL_SUCCESS) {
        LOGE << "Failed to load kernels into OpenCL memory : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    return true;
}

void NeuralFieldModel::ReleaseOpenCLBuffers() {
    clReleaseMemObject(memExcitementMatrix);
    clReleaseMemObject(memInhibitionMatrix);
    clReleaseMemObject(memStimulusMatrix);
    clReleaseMemObject(memActivityMatrix);
    clReleaseMemObject(memTempMatrix);

    clReleaseMemObject(memInhibitionKernel);
    clReleaseMemObject(memExcitementKernel);
}

void NeuralFieldModel::CalcHeaviside(cl_mem memDst, cl_uint matrixSize) {
    cl_int status = CL_SUCCESS;

    const size_t globalWorkSize = matrixSize * matrixSize;
    const size_t localWorkSize = 64;

    {
        // --------------------------------------------------------
        // Set the Argument values

        status = clSetKernelArg(heavisideKernel, 0, sizeof(cl_mem), (void*)&memDst);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to setup OpenCL program arguments : " << ParallelUtils::GetOpenCLError(status);
            return;
        }

        // --------------------------------------------------------
        // Start Core sequence

        // Compute: launch kernel
        status = clEnqueueNDRangeKernel(commandQueue, heavisideKernel, 1, NULL, &globalWorkSize,
            &localWorkSize, 0, NULL, NULL);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to launch OpenCL kernel : " << ParallelUtils::GetOpenCLError(status);
            return;
        }
    }
}

void NeuralFieldModel::CalcExcitementMatrix() {
    GaussianBlur(memExcitementMatrix, memActivityMatrix, memTempMatrix, this->size,
        memExcitementKernel, excitement_kernel->size);
}

void NeuralFieldModel::CalcInhibitionMatrix() {
    GaussianBlur(memInhibitionMatrix, memActivityMatrix, memTempMatrix, this->size,
        memInhibitionKernel, inhibition_kernel->size);
}

void NeuralFieldModel::GaussianBlur(cl_mem memDst, cl_mem memSrc, cl_mem memTmp, cl_uint matrixSize, cl_mem memKernel, cl_uint kernelSize) {
    cl_int status;

    const size_t globalWorkSize = matrixSize * matrixSize;
    const size_t localWorkSize = 64;

    cl_int blurDirection = 0;
    cl_mem src = memSrc;
    cl_mem dst = memTmp;

    do {
        // --------------------------------------------------------
        // Set the Argument values

        status = CL_SUCCESS;

        status |= clSetKernelArg(gaussianBlurKernel, 0, sizeof(cl_mem), (void*)&src);
        status |= clSetKernelArg(gaussianBlurKernel, 1, sizeof(cl_mem), (void*)&memKernel);
        status |= clSetKernelArg(gaussianBlurKernel, 2, sizeof(cl_uint), (void*)&matrixSize);
        status |= clSetKernelArg(gaussianBlurKernel, 3, sizeof(cl_uint), (void*)&kernelSize);
        status |= clSetKernelArg(gaussianBlurKernel, 4, sizeof(cl_int), (void*)&blurDirection);
        status |= clSetKernelArg(gaussianBlurKernel, 5, sizeof(cl_mem), (void*)&dst);

        if (status != CL_SUCCESS) {
            LOGE << "Failed to setup OpenCL program arguments : " << ParallelUtils::GetOpenCLError(status);
            break;
        }

        // --------------------------------------------------------
        // Start Core sequence

        // Compute: launch kernel
        status = clEnqueueNDRangeKernel(commandQueue, gaussianBlurKernel, 1, NULL, &globalWorkSize,
            &localWorkSize, 0, NULL, NULL);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to launch OpenCL kernel : " << ParallelUtils::GetOpenCLError(status);
            break;
        }

        // Next matrix
        src = dst;
        dst = memDst;
    } while (++blurDirection < 2);
}

void NeuralFieldModel::CalcActivity() {
    cl_int status;

    const size_t globalWorkSize = activity->dataSize;
    const size_t localWorkSize = 64;

    {
        float fh = float(h);
        float fpi_k = float(pi_k);
        float fpi_m = float(pi_m);

        // --------------------------------------------------------
        // Set the Argument values

        status = CL_SUCCESS;

        status |= clSetKernelArg(stimulationKernel, 0, sizeof(cl_mem), (void*)&memExcitementMatrix);
        status |= clSetKernelArg(stimulationKernel, 1, sizeof(cl_mem), (void*)&memInhibitionMatrix);
        status |= clSetKernelArg(stimulationKernel, 2, sizeof(cl_mem), (void*)&memStimulusMatrix);
        status |= clSetKernelArg(stimulationKernel, 3, sizeof(cl_float), (void*)&fh);
        status |= clSetKernelArg(stimulationKernel, 4, sizeof(cl_float), (void*)&fpi_k);
        status |= clSetKernelArg(stimulationKernel, 5, sizeof(cl_float), (void*)&fpi_m);
        status |= clSetKernelArg(stimulationKernel, 6, sizeof(cl_mem), (void*)&memActivityMatrix);

        if (status != CL_SUCCESS) {
            LOGE << "Failed to setup OpenCL program arguments : " << ParallelUtils::GetOpenCLError(status);
            return;
        }

        // --------------------------------------------------------
        // Start Core sequence

        // Compute: launch kernel
        status = clEnqueueNDRangeKernel(commandQueue, stimulationKernel, 1, NULL, &globalWorkSize,
            &localWorkSize, 0, NULL, NULL);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to launch OpenCL kernel : " << ParallelUtils::GetOpenCLError(status);
            return;
        }
    }
}
#endif /* USE_OPENCL */