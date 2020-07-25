#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "MathUtils.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "Shader.h"
#include "FrameBufferWrapper.h"
#include "PlainTextureRenderer.h"
#include "TextureRenderer.h"

static const size_t g_bitsPerPixel = 4;

/*****************************************************************************
 * TextureRenderer
 ****************************************************************************/
static const std::string g_vertexShaderPath = "data/texture.vert";
static const std::string g_fragmentShaderPath = "data/texture.frag";

static const std::string g_vertexBlurShaderPath = "data/texture-blur.vert";
static const std::string g_fragmentBlurShaderPath = "data/texture-blur.frag";

TextureRenderer::TextureRenderer()
 : use_blur(true) {

    set_blur(1.0);
}

TextureRenderer::~TextureRenderer() {
    release();
}

bool TextureRenderer::init(size_t textureSize) {
    if (!initTexture(textureSize)) {
        LOGE << "Failed to create textures for neural field renderer";
        return false;
    }

    // Init framebuffer
    if (!frameBuffer.Init()) {
        LOGE << "Failed to create framebuffer for neural field renderer";
        return false;
    }

    // Init shader program
    {
        if (!Shader::createProgram(program.p, g_vertexShaderPath, g_fragmentShaderPath)) {
            LOGE << "Failed to create shader program for neural field renderer";
            return false;
        }
    }

    // Init blur shader program
    {
        if (!Shader::createProgram(blurProgram.p, g_vertexBlurShaderPath, g_fragmentBlurShaderPath)) {
            LOGE << "Failed to create blur shader program for neural field renderer";
            return false;
        }

        blurProgram.uBlurDir = glGetUniformLocation(blurProgram.p, "blur_dir");
        blurProgram.uBlurKernelTex = glGetUniformLocation(blurProgram.p, "blur_kernel_tex");
    }

    if (!screenRenderer.Init(program.p)) {
        return false;
    }

    if (!blurPreRenderer.Init(blurProgram.p)) {
        return false;
    }

    return true;
}

bool TextureRenderer::initTexture(size_t newSize) {
    releaseTextures();

    size = newSize;

    // Main texture
    glGenTextures(1, &texture); LOGOPENGLERROR();
    if (!texture) {
        LOGE << "Failed to create texture for neural field renderer";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); LOGOPENGLERROR();

    // Blur texture
    glGenTextures(1, &blurTextureInter); LOGOPENGLERROR();
    if (!blurTextureInter) {
        LOGE << "Failed to create temporary texture for neural field renderer";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, blurTextureInter); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); LOGOPENGLERROR();

    // Create base texture
    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, nullptr); LOGOPENGLERROR();

    // Create blur textures
    glBindTexture(GL_TEXTURE_2D, blurTextureInter); LOGOPENGLERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, nullptr); LOGOPENGLERROR();

    // Allocate memory
    tex = TextureGuard_t(texture_alloc(size, g_bitsPerPixel), texture_free);

    return true;
}

void TextureRenderer::releaseTextures() {
    if (texture) {
        glDeleteTextures(1, &texture); LOGOPENGLERROR();
        texture = 0;
    }
    if (blurTextureInter) {
        glDeleteTextures(1, &blurTextureInter); LOGOPENGLERROR();
        blurTextureInter = 0;
    }
}

void TextureRenderer::release() {
    tex.reset();
    releaseTextures();

    glDeleteProgram(program.p); LOGOPENGLERROR();
    glDeleteProgram(blurProgram.p); LOGOPENGLERROR();

    glDeleteTextures(1, &blurKernelTexture); LOGOPENGLERROR();
}

void TextureRenderer::render(const glm::mat4& mvp) {
    if (use_blur) {
        // Render blured version to texture

        // Save viewport
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        // Set viewport of framebuffer
        glViewport(0, 0, (GLsizei)size, (GLsizei)size);

        // Initialize blur shader program
        glActiveTexture(GL_TEXTURE2); LOGOPENGLERROR();
        glBindTexture(GL_TEXTURE_1D, blurKernelTexture); LOGOPENGLERROR();

        // Step 1: Vertical blur texture->blurTextureInter
        frameBuffer.SetTexColorBuffer(blurTextureInter);
        blurPreRenderer.SetTexture(texture); // texture is source (0)

        glUseProgram(blurProgram.p); LOGOPENGLERROR();
        glUniform1i(blurProgram.uBlurDir, 1); LOGOPENGLERROR(); // Vertical blur
        glUniform1i(blurProgram.uBlurKernelTex, 2); LOGOPENGLERROR();

        frameBuffer.Bind();
        blurPreRenderer.Render();
        frameBuffer.Unbind();

        // Step 2: Horizontal blur blurTextureInter->blurTextureFinal
        frameBuffer.SetTexColorBuffer(texture);
        blurPreRenderer.SetTexture(blurTextureInter); // blurTextureInter is source (1)

        glUseProgram(blurProgram.p); LOGOPENGLERROR();
        glUniform1i(blurProgram.uBlurKernelTex, 2); LOGOPENGLERROR();
        glUniform1i(blurProgram.uBlurDir, 2); LOGOPENGLERROR(); // Horizontal blur

        frameBuffer.Bind();
        blurPreRenderer.Render();
        frameBuffer.Unbind();

        // Step 3: Render texture

        // Restore viewport
        glViewport(viewport[0], viewport[1],
            viewport[2], viewport[3]);
    }

    screenRenderer.SetMvp(mvp);
    screenRenderer.SetTexture(texture);
    screenRenderer.Render();
}

void TextureRenderer::resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
    screenRenderer.Resize(w, h);
}

void TextureRenderer::update_texture(matrix_t* m) {
    texture_copy_matrix(tex.get(), m);

    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->size, tex->size, GL_RGBA,
                    GL_UNSIGNED_BYTE, (const GLubyte *)tex->data); LOGOPENGLERROR();
}

void TextureRenderer::set_blur(double blur) {
    blur_sigma = blur;
    if (blur_sigma > 0.0) {
        blur_kernel = KernelGuard_t(kernel_create(blur_sigma, MODE_WRAP), kernel_free);

        this->initBlurKernelTex();

#ifndef NDEBUG
        std::stringstream s;
        for (size_t t = 0; t < blur_kernel->size; t++) {
            s << blur_kernel->data[t] << ", ";
        }
        LOGD << "Blur kernel = [ " << s.str() << " ]";
#endif
    }
}

void TextureRenderer::initBlurKernelTex() {
    // Cast array for doubles to array of floats
    // to send array to OpenGL texture
    std::vector<float> data(blur_kernel->size);
    for (size_t i = 0; i < blur_kernel->size; i++) {
        data[i] = static_cast<float>(blur_kernel->data[i]);
    }

    // Remove existing texture
    if (blurKernelTexture) {
        glDeleteTextures(1, &blurKernelTexture); LOGOPENGLERROR();
        blurKernelTexture = 0;
    }

    // Create new texture
    glGenTextures(1, &blurKernelTexture); LOGOPENGLERROR();
    if (!blurKernelTexture) {
        LOGE << "Failed to create blur kernel texture";
        return;
    }

    glBindTexture(GL_TEXTURE_1D, blurKernelTexture); LOGOPENGLERROR();
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, data.size(), 0, GL_RED,
        GL_FLOAT, data.data());

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT); LOGOPENGLERROR();
}

void TextureRenderer::add_blur(double dblur) {
    double new_blur_sigma = blur_sigma + dblur;

    if (new_blur_sigma > 0.0) {
        this->setUseBlur(true);
    }
    else {
        this->setUseBlur(false);
        new_blur_sigma = 0.0;
    }
    LOGI << "Blur Sigma = " << new_blur_sigma;

    set_blur(new_blur_sigma);
}

void TextureRenderer::setUseBlur(bool newUseBlur) {
    this->use_blur = newUseBlur;
    if (this->use_blur) {
        LOGI << "Turned Blur On";
    }
    else {
        LOGI << "Turned Blur Off";
    }
}
