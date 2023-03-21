#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "Gauss.h"
#include "GraphicsLogger.h"
#include "Shader.h"
#include "FrameBufferWrapper.h"
#include "PlainTextureRenderer.h"
#include "TextureRenderer.h"

const size_t g_bitsPerPixel = 4;

/*****************************************************************************
 * TextureRenderer
 ****************************************************************************/
const std::filesystem::path g_vertexShaderPath = "texture.vert";
const std::filesystem::path g_fragmentShaderPath = "texture.frag";

const std::filesystem::path g_vertexBlurShaderPath = "texture-blur.vert";
const std::filesystem::path g_fragmentBlurShaderPath = "texture-blur.frag";

TextureRenderer::~TextureRenderer() {
    Release();
}

bool TextureRenderer::Init(const std::filesystem::path& moduleDataDir, size_t textureSize) {
    SetBlur(1.0);
	
    if (!InitTextures(textureSize)) {
        LOGE << "Failed to create textures for neural field renderer";
        return false;
    }

    // Init framebuffer
    if (!frameBuffer.Init()) {
        LOGE << "Failed to create framebuffer for neural field renderer";
        return false;
    }

    // Init shader program
	program.p = Shader::CreateProgramFromFiles(
			(moduleDataDir / g_vertexShaderPath).string(),
			(moduleDataDir / g_fragmentShaderPath).string());
    if (!program.p) {
        LOGE << "Failed to create shader program for neural field renderer";
        return false;
    }

    // Init blur shader program
	blurProgram.p = Shader::CreateProgramFromFiles(
			(moduleDataDir / g_vertexBlurShaderPath).string(),
			(moduleDataDir / g_fragmentBlurShaderPath).string());
    if (!blurProgram.p) {
        LOGE << "Failed to create blur shader program for neural field renderer";
        return false;
    }

    blurProgram.uBlurDir = glGetUniformLocation(blurProgram.p, "blur_dir");
    blurProgram.uBlurKernelTex = glGetUniformLocation(blurProgram.p, "blur_kernel_tex");

    // Init renderers
    if (!screenRenderer.Init(program.p)) {
        LOGE << "Failed to create screen texture renderer";
        return false;
    }

    if (!blurPreRenderer.Init(blurProgram.p)) {
        LOGE << "Failed to create texture blur pre-renderer";
        return false;
    }

    return true;
}

bool TextureRenderer::InitTextures(size_t newSize) {
    ReleaseTextures();

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
    tex = TextureGuard_t(texture_alloc(size, g_bitsPerPixel == 3 ? TextureFormat::RGB : TextureFormat::RGBA),
        texture_free);

    return true;
}

void TextureRenderer::ReleaseTextures() {
    tex.reset();

    if (texture) {
        glDeleteTextures(1, &texture); LOGOPENGLERROR();
        texture = 0;
    }
    if (blurTextureInter) {
        glDeleteTextures(1, &blurTextureInter); LOGOPENGLERROR();
        blurTextureInter = 0;
    }
}

void TextureRenderer::Release() {
    ReleaseTextures();

    glDeleteProgram(program.p); LOGOPENGLERROR();
    glDeleteProgram(blurProgram.p); LOGOPENGLERROR();

    glDeleteTextures(1, &blurKernelTexture); LOGOPENGLERROR();
}

void TextureRenderer::Render(const hmm_mat4& mvp) {
    if (useBlur) {
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
        glUniform1i(blurProgram.uBlurKernelTex, 2); LOGOPENGLERROR();
        glUniform1i(blurProgram.uBlurDir, static_cast<int>(BlurDirection::VerticalBlur)); LOGOPENGLERROR();

        frameBuffer.Bind();
        blurPreRenderer.Render();
        frameBuffer.Unbind();

        // Step 2: Horizontal blur blurTextureInter->texture
        frameBuffer.SetTexColorBuffer(texture);
        blurPreRenderer.SetTexture(blurTextureInter); // blurTextureInter is source (1)

        glUseProgram(blurProgram.p); LOGOPENGLERROR();
        glUniform1i(blurProgram.uBlurKernelTex, 2); LOGOPENGLERROR();
        glUniform1i(blurProgram.uBlurDir, static_cast<int>(BlurDirection::HorizontalBlur)); LOGOPENGLERROR();

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

void TextureRenderer::Resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
    screenRenderer.Resize(w, h);
}

void TextureRenderer::UpdateTexture(matrix_t* m) {
    texture_copy_matrix(tex.get(), m);

    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->size, tex->size, GL_RGBA,
                    GL_UNSIGNED_BYTE, (const GLubyte *)tex->data); LOGOPENGLERROR();
}

void TextureRenderer::SetBlur(double blur) {
    blurSigma = blur;
    if (blurSigma > 0.0) {
        SetUseBlur(true);

        blurKernel = KernelGuard_t(kernel_create(blurSigma, MODE_WRAP), kernel_free);

        InitBlurKernelTex();

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

void TextureRenderer::InitBlurKernelTex() {
    // Cast array for doubles to array of floats
    // to send array to OpenGL texture
    std::vector<float> data(blurKernel->size);
    for (size_t i = 0; i < blurKernel->size; i++) {
        data[i] = static_cast<float>(blurKernel->data[i]);
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
