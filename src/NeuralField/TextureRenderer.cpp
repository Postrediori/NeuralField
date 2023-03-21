#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"
#include "GraphicsLogger.h"
#include "GraphicsResource.h"
#include "Shader.h"
#include "NeuralFieldModel.h"
#include "PlainTextureRenderer.h"
#include "TextureRenderer.h"

const size_t g_bitsPerPixel = 4;

const std::filesystem::path g_vertexShaderPath = "texture.vert";
const std::filesystem::path g_fragmentShaderPath = "texture.frag";

/*****************************************************************************
 * TextureRenderer
 ****************************************************************************/
TextureRenderer::~TextureRenderer() {
    Release();
}

bool TextureRenderer::Init(NeuralFieldModel* model, const std::filesystem::path& moduleDataDir, size_t textureSize) {
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
    if (!screenRenderer.Init(program.get())) {
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

    glBindTexture(GL_TEXTURE_2D, texture.get()); LOGOPENGLERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); LOGOPENGLERROR();

    screenRenderer.SetTexture(texture.get());

    // Create base texture
    glBindTexture(GL_TEXTURE_2D, texture.get()); LOGOPENGLERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size, size, 0, GL_RED,
        GL_FLOAT, nullptr); LOGOPENGLERROR();

    // Allocate memory
    tex = MatrixGuard_t(matrix_allocate(size, size), matrix_free);
    tempTex = MatrixGuard_t(matrix_allocate(size, size), matrix_free);

    return true;
}

void TextureRenderer::ReleaseTextures() {
    tex.reset();
    tempTex.reset();

    texture.reset();
}

void TextureRenderer::Release() {
    ReleaseTextures();
}

void TextureRenderer::Render(const hmm_mat4& mvp) {
    screenRenderer.SetMvp(mvp);
    screenRenderer.Render();
}

void TextureRenderer::Resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
    screenRenderer.Resize(w, h);
}

void TextureRenderer::UpdateTexture(matrix_t* m) {
    {
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

    glBindTexture(GL_TEXTURE_2D, texture.get()); LOGOPENGLERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RED,
                    GL_FLOAT, static_cast<const GLfloat *>(tex->data)); LOGOPENGLERROR();
}

void TextureRenderer::SetBlur(double blur) {
    blurSigma = blur;
    if (blurSigma > 0.0) {
        SetUseBlur(true);

        blurKernel = KernelGuard_t(kernel_create(blurSigma, MODE_WRAP), kernel_free);

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
