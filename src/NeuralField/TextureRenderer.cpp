#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "MathUtils.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "Shader.h"
#include "FrameBufferWrapper.h"
#include "TextureRenderer.h"

static const size_t g_bitsPerPixel = 4;

/*****************************************************************************
 * TextureRenderer
 ****************************************************************************/
static const std::vector<glm::vec4> g_quadVertices = {
    {-1.0f, -1.0f, 0.f, 0.f},
    {-1.0f,  1.0f, 0.f, 1.f},
    {1.0f, -1.0f, 1.f, 0.f},
    {1.0f,  1.0f, 1.f, 1.f},
};

static const std::vector<GLuint> g_quadIndices = {
    0, 1, 2,
    2, 1, 3,
};

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

        program.uTex = glGetUniformLocation(program.p, "tex");
        program.uResolution = glGetUniformLocation(program.p, "iRes");
        program.uMVP = glGetUniformLocation(program.p, "mvp");
    }

    // Init blur shader program
    {
        if (!Shader::createProgram(blurProgram.p, g_vertexBlurShaderPath, g_fragmentBlurShaderPath)) {
            LOGE << "Failed to create blur shader program for neural field renderer";
            return false;
        }

        blurProgram.uTex = glGetUniformLocation(blurProgram.p, "tex");
        blurProgram.uBlurDir = glGetUniformLocation(blurProgram.p, "blur_dir");
        blurProgram.uBlurKernelTex = glGetUniformLocation(blurProgram.p, "blur_kernel_tex");
    }

    // Create vertex buffer
    glGenBuffers(1, &vbo); LOGOPENGLERROR();
    glGenBuffers(1, &indVbo); LOGOPENGLERROR();
    if (!vbo || !indVbo) {
        LOGE << "Unable to initialize vertex buffers";
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quadVertices[0]) * g_quadVertices.size(),
        g_quadVertices.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_quadIndices[0]) * g_quadIndices.size(),
        g_quadIndices.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    // Create VAO
    {
        glGenVertexArrays(1, &vao); LOGOPENGLERROR();
        if (!vao) {
            LOGE << "Failed to create vertex array object";
            return false;
        }
        glBindVertexArray(vao); LOGOPENGLERROR();

        GLint aCoord = glGetAttribLocation(program.p, "coord");
        GLint aTexCoord = glGetAttribLocation(program.p, "tex_coord");

        glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indVbo);

        glEnableVertexAttribArray(aCoord); LOGOPENGLERROR();
        glVertexAttribPointer(aCoord, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 4, 0); LOGOPENGLERROR();

        glEnableVertexAttribArray(aTexCoord); LOGOPENGLERROR();
        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 4, (void *)(sizeof(GLfloat) * 2)); LOGOPENGLERROR();

        glBindVertexArray(0); LOGOPENGLERROR();
    }

    {
        glGenVertexArrays(1, &blurVao); LOGOPENGLERROR();
        if (!blurVao) {
            LOGE << "Failed to create vertex array object";
            return false;
        }
        glBindVertexArray(blurVao); LOGOPENGLERROR();

        GLint aCoord = glGetAttribLocation(blurProgram.p, "coord");
        GLint aTexCoord = glGetAttribLocation(blurProgram.p, "tex_coord");

        glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indVbo);

        glEnableVertexAttribArray(aCoord); LOGOPENGLERROR();
        glVertexAttribPointer(aCoord, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 4, 0); LOGOPENGLERROR();

        glEnableVertexAttribArray(aTexCoord); LOGOPENGLERROR();
        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE,
            sizeof(GLfloat) * 4, (void *)(sizeof(GLfloat) * 2)); LOGOPENGLERROR();

        glBindVertexArray(0); LOGOPENGLERROR();
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
    
    glDeleteVertexArrays(1, &vao); LOGOPENGLERROR();
    glDeleteVertexArrays(1, &blurVao); LOGOPENGLERROR();

    glDeleteBuffers(1, &vbo); LOGOPENGLERROR();
    glDeleteBuffers(1, &indVbo); LOGOPENGLERROR();

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
        glUseProgram(blurProgram.p); LOGOPENGLERROR();
        glBindVertexArray(blurVao); LOGOPENGLERROR();

        glActiveTexture(GL_TEXTURE0); LOGOPENGLERROR();
        glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();

        glActiveTexture(GL_TEXTURE1); LOGOPENGLERROR();
        glBindTexture(GL_TEXTURE_2D, blurTextureInter); LOGOPENGLERROR();

        glActiveTexture(GL_TEXTURE2); LOGOPENGLERROR();
        glBindTexture(GL_TEXTURE_1D, blurKernelTexture); LOGOPENGLERROR();

        glUniform1i(blurProgram.uBlurKernelTex, 2); LOGOPENGLERROR();

        // Step 1: Vertical blur texture->blurTextureInter
        frameBuffer.SetTexColorBuffer(blurTextureInter);
        glUniform1i(blurProgram.uTex, 0); LOGOPENGLERROR(); // texture is source (0)
        glUniform1i(blurProgram.uBlurDir, 1); LOGOPENGLERROR(); // Vertical blur

        frameBuffer.Bind();
        glDrawElements(GL_TRIANGLES, (GLsizei)g_quadIndices.size(), GL_UNSIGNED_INT, nullptr); LOGOPENGLERROR();
        frameBuffer.Unbind();

        // Step 2: Horizontal blur blurTextureInter->blurTextureFinal
        frameBuffer.SetTexColorBuffer(texture);
        glUniform1i(blurProgram.uTex, 1); LOGOPENGLERROR(); // blurTextureInter is source (1)
        glUniform1i(blurProgram.uBlurDir, 2); LOGOPENGLERROR(); // Horizontal blur

        frameBuffer.Bind();
        glDrawElements(GL_TRIANGLES, (GLsizei)g_quadIndices.size(), GL_UNSIGNED_INT, nullptr); LOGOPENGLERROR();
        frameBuffer.Unbind();

        // Step 3: Render texture

        // Restore viewport
        glViewport(viewport[0], viewport[1],
            viewport[2], viewport[3]);
    }

    glUseProgram(program.p); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glActiveTexture(GL_TEXTURE0); LOGOPENGLERROR();
    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();

    glUniformMatrix4fv(program.uMVP, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform2f(program.uResolution, (GLfloat)w, (GLfloat)h); LOGOPENGLERROR();
    glUniform1i(program.uTex, 0); LOGOPENGLERROR();

    glDrawElements(GL_TRIANGLES, (GLsizei)g_quadIndices.size(), GL_UNSIGNED_INT, nullptr); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
    glBindVertexArray(0); LOGOPENGLERROR();
}

void TextureRenderer::resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
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
        use_blur = true;
        LOGI << "Blur Sigma " << new_blur_sigma;
    }
    else {
        use_blur = false;
        LOGI << "Turned Blur Off";
    }

    set_blur(new_blur_sigma);
}
