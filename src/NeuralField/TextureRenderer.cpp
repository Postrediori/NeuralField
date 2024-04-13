#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"
#include "GraphicsLogger.h"
#include "GraphicsResource.h"
#include "Shader.h"
#ifdef USE_OPENCL
#include "ParallelUtils.h"
#endif
#include "NeuralFieldModel.h"
#include "PlainTextureRenderer.h"
#include "TextureRenderer.h"

const std::filesystem::path g_vertexShaderPath = "texture.vert";
const std::filesystem::path g_fragmentShaderPath = "texture.frag";

/*****************************************************************************
 * TextureRenderer
 ****************************************************************************/
TextureRenderer::~TextureRenderer() {
    Release();
}

bool TextureRenderer::Init(NeuralFieldModel* model, const std::filesystem::path& moduleDataDir) {
    this->model_ = model;

    SetBlur(1.0);

    if (!InitTextures(model_->size)) {
        LOGE << "Failed to create textures for neural field renderer";
        return false;
    }

    // Init shader program
    auto vertexShaderPath = moduleDataDir / g_vertexShaderPath;
    auto fragmentShaderPath = moduleDataDir / g_fragmentShaderPath;
    program.reset(Shader::CreateProgramFromFiles(vertexShaderPath.string(), fragmentShaderPath.string()));
    if (!program) {
        LOGE << "Failed to create shader program for neural field renderer";
        return false;
    }

    // Init renderers
    if (!screenRenderer.Init(static_cast<GLuint>(program))) {
        LOGE << "Failed to create screen texture renderer";
        return false;
    }

    return true;
}

bool TextureRenderer::InitTextures(size_t newSize) {
    ReleaseTextures();

    size = newSize;

    // Main texture
    glGenTextures(1, texture.put()); LOGOPENGLERROR();
    if (!texture) {
        LOGE << "Failed to create texture for neural field renderer";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture)); LOGOPENGLERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); LOGOPENGLERROR();

    screenRenderer.SetTexture(static_cast<GLuint>(texture));

    // Create base texture
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture)); LOGOPENGLERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size, size, 0, GL_RED,
        GL_FLOAT, nullptr); LOGOPENGLERROR();

    // Allocate memory
    tex = MatrixGuard_t(matrix_allocate(size, size), matrix_free);
    tempTex = MatrixGuard_t(matrix_allocate(size, size), matrix_free);

#ifdef USE_OPENCL
    // Init OpenCL
    if (isEnabledOpenCL) {
        cl_int status, callStatus;

        status = CL_SUCCESS;

        memTextureBuffer = clCreateBuffer(model_->context, CL_MEM_READ_WRITE, sizeof(cl_float) * size * size, NULL, &callStatus);
        status |= callStatus;

        if (status != CL_SUCCESS) {
            LOGE << "Failed to create OpenCL memory buffer : " << ParallelUtils::GetOpenCLError(status);
            isEnabledOpenCL = false;
        }
    }
#endif

    return true;
}

void TextureRenderer::ReleaseTextures() {
    tex.reset();
    tempTex.reset();

    texture.reset();

#ifdef USE_OPENCL
    if (isEnabledOpenCL && memTextureBuffer) {
        clReleaseMemObject(memTextureBuffer);
        memTextureBuffer = 0;
    }
#endif
}

#ifdef USE_OPENCL
void TextureRenderer::ReleaseOpenCLBuffers() {
    if (memBlurKernel) {
        clReleaseMemObject(memBlurKernel);
        memBlurKernel = 0;
    }
}
#endif

void TextureRenderer::Release() {
    ReleaseTextures();
#ifdef USE_OPENCL
    if (isEnabledOpenCL) {
        ReleaseOpenCLBuffers();
    }
#endif
}

void TextureRenderer::Render(const HMM_Mat4& mvp) {
    screenRenderer.SetMvp(mvp);
    screenRenderer.Render();
}

void TextureRenderer::Resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
    screenRenderer.Resize(w, h);
}

void TextureRenderer::UpdateTexture() {
#ifdef USE_OPENCL
    if (!isEnabledOpenCL) {
#else
    {
#endif
        matrix_t* m = model_->activity.get();

        matrix_scalar_set(tempTex.get(), 0.0);
        matrix_add(tempTex.get(), m);

        matrix_heaviside(tempTex.get());

        matrix_scalar_set(tex.get(), 0.0);
        matrix_add(tex.get(), tempTex.get());

        if (useBlur) {
            kernel_apply_to_matrix(tex.get(), tex.get(), tempTex.get(), blurKernel.get());
        }
    }
#ifdef USE_OPENCL
    else {
        cl_int status;

        status = clEnqueueCopyBuffer(model_->commandQueue, model_->memActivityMatrix, memTextureBuffer,
            0, 0, sizeof(cl_float) * this->size * this->size, 0, nullptr, nullptr);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to copy OpenCL memory buffer : " << ParallelUtils::GetOpenCLError(status);
            return;
        }

        model_->CalcHeaviside(memTextureBuffer, this->size);

        if (useBlur) {
            model_->GaussianBlur(memTextureBuffer, memTextureBuffer, model_->memTempMatrix, this->size,
                this->memBlurKernel, this->blurKernel->size);
        }

        // Read the results
        status = clEnqueueReadBuffer(model_->commandQueue, memTextureBuffer, CL_TRUE, 0,
            sizeof(cl_float) * tex->dataSize, tex->data, 0, NULL, NULL);
        if (status != CL_SUCCESS) {
            LOGE << "Failed to read result buffer after OpenCL kernel run : " << ParallelUtils::GetOpenCLError(status);
            return;
        }
    }
#endif

    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture)); LOGOPENGLERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RED,
                    GL_FLOAT, static_cast<const GLfloat *>(tex->data)); LOGOPENGLERROR();
}

void TextureRenderer::SetBlur(double blur) {
    blurSigma = blur;
    if (blurSigma > 0.0) {
        SetUseBlur(true);

        blurKernel = KernelGuard_t(kernel_create(blurSigma, MODE_WRAP), kernel_free);

#ifdef USE_OPENCL
        if (isEnabledOpenCL) {
            cl_int status, callStatus;

            ReleaseOpenCLBuffers();

            // --------------------------------------------------------
            // Allocate the OpenCL buffer memory object for blur kernel
            status = CL_SUCCESS;
            memBlurKernel = clCreateBuffer(model_->context, CL_MEM_READ_ONLY, sizeof(cl_float) * blurKernel->size, NULL, &callStatus);
            status |= callStatus;

            if (status != CL_SUCCESS) {
                LOGE << "Failed to create memory buffer for texture blur kernel: " << ParallelUtils::GetOpenCLError(status);
                return;
            }

            // --------------------------------------------------------
            // Load kernel into memory object
            status = CL_SUCCESS;

            status |= clEnqueueWriteBuffer(model_->commandQueue, memBlurKernel, CL_FALSE, 0,
                sizeof(cl_float) * blurKernel->size, blurKernel->data, 0, NULL, NULL);

            if (status != CL_SUCCESS) {
                LOGE << "Failed to load kernels into OpenCL memory : " << ParallelUtils::GetOpenCLError(status);
                return;
            }
        }
#endif

#ifndef NDEBUG
        std::stringstream s;
        for (size_t t = 0; t < blurKernel->size; t++) {
            s << blurKernel->data[t] << ", ";
        }
        LOGD << "Blur kernel = [ " << s.str() << " ]";
#endif
    }
    else {
        SetUseBlur(false);
    }
}

void TextureRenderer::AddBlur(double dblur) {
    double new_blur_sigma = blurSigma + dblur;

    if (new_blur_sigma > 0.0) {
        SetUseBlur(true);
    }
    else {
        SetUseBlur(false);
        new_blur_sigma = 0.0;
    }
    LOGI << "Blur Sigma = " << new_blur_sigma;

    SetBlur(new_blur_sigma);
}

void TextureRenderer::SetUseBlur(bool newUseBlur) {
    useBlur = newUseBlur;
    if (useBlur) {
        LOGI << "Turned Blur On";
    }
    else {
        LOGI << "Turned Blur Off";
    }
}
