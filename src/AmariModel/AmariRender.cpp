#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "Shader.h"
#include "AmariRender.h"

static const size_t g_bitsPerPixel = 4;

/*****************************************************************************
 * AmariRender
 ****************************************************************************/
static const GLsizei g_quadVerticesCount = 4;
static const GLfloat g_quadVertices[] = {
    -1.0f, -1.0f, 0.f, 0.f,
    -1.0f,  1.0f, 0.f, 1.f,
    1.0f, -1.0f, 1.f, 0.f,
    1.0f,  1.0f, 1.f, 1.f,
};

static const char g_vertexShader110[] = 
    "#version 110\n"
    "attribute vec2 coord;"
    "attribute vec2 tex_coord;"
    "varying vec2 xy_coord;"
    "uniform vec2 iRes;"
    "uniform mat4 mvp;"
    "void main(void){"
    "    xy_coord=coord.xy;"
    "    if (iRes.x>iRes.y){"
    "        xy_coord.x*=iRes.y/iRes.x;"
    "    }else{"
    "        xy_coord.y*=iRes.x/iRes.y;"
    "    }"
    "    gl_Position=mvp*vec4(xy_coord,0.,1.);"
    "    xy_coord=tex_coord.xy;"
    "}";

static const char g_fragmentShader110[] =
    "#version 110\n"
    "varying vec2 xy_coord;"
    "uniform sampler2D tex;"
    "const vec4 col0=vec4(.5,.5,1.,1.);"
    "void main(void){"
    "   gl_FragColor=texture2D(tex,xy_coord)*col0;"
    "}";

static const char g_vertexShader130[] = 
    "#version 130\n"
    "in vec2 coord;"
    "in vec2 tex_coord;"
    "out vec2 xy_coord;"
    "uniform vec2 iRes;"
    "uniform mat4 mvp;"
    "void main(void){"
    "    xy_coord=coord.xy;"
    "    if (iRes.x>iRes.y)"
    "        xy_coord.x*=iRes.y/iRes.x;"
    "    else"
    "        xy_coord.y*=iRes.x/iRes.y;"
    "    gl_Position=mvp*vec4(xy_coord,0.,1.);"
    "    xy_coord=tex_coord.xy;"
    "}";

static const char g_fragmentShader130[] =
    "#version 130\n"
    "in vec2 xy_coord;"
    "out vec4 frag_color;"
    "uniform sampler2D tex;"
    "const vec4 col0=vec4(.5,.5,1.,1.);"
    "void main(void){"
    "   frag_color=texture(tex,xy_coord)*col0;"
    "   frag_color.a=1.;"
    "}";

AmariRender::AmariRender()
 : use_blur(true) {
    set_blur(1.0);
}

AmariRender::~AmariRender() {
    release();
}

bool AmariRender::init(size_t size) {
    this->size = size;
    
    GLuint genbuf[1];

    // Init textures
    glGenTextures(1, genbuf); LOGOPENGLERROR();
    texture = genbuf[0];
    if (!texture) {
        LOGE << "Unable to initialize texture for Amari Renderer";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); LOGOPENGLERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL); LOGOPENGLERROR();

    // Init VBO
    glGenBuffers(1, genbuf); LOGOPENGLERROR();
    vbo = genbuf[0];
    if (!vbo) {
        LOGE << "Unable to initialize VBO for Amari Renderer";
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quadVertices), g_quadVertices, GL_STATIC_DRAW); LOGOPENGLERROR();

    // Init shader
    if (!Shader::createProgramSource(program, vertex, fragment, g_vertexShader110, g_fragmentShader110)) {
        LOGE << "Unable to load shader for Amari Renderer";
        return false;
    }

    aCoord = glGetAttribLocation(program, "coord");
    aTexCoord = glGetAttribLocation(program, "tex_coord");
    uTex = glGetUniformLocation(program, "tex");
    uResolution = glGetUniformLocation(program, "iRes");
    uMVP = glGetUniformLocation(program, "mvp");
    if (aCoord == -1 || aTexCoord == -1 || uTex == -1
        || uResolution == -1 || uMVP == -1) {
        LOGE << "Invalid shader program";
        return false;
    }

    // Allocate memory
    tex = TextureGuard_t(texture_alloc(size, g_bitsPerPixel), texture_free);

    return true;
}

void AmariRender::release() {
    tex.reset();

    Shader::releaseProgram(program, vertex, fragment);
    
    glDeleteTextures(1, &texture); LOGOPENGLERROR();
    glDeleteBuffers(1, &vbo); LOGOPENGLERROR();
}

void AmariRender::render(const glm::mat4& mvp) {
    glUseProgram(program); LOGOPENGLERROR();

    glActiveTexture(GL_TEXTURE0); LOGOPENGLERROR();
    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform2f(uResolution, (GLfloat)w, (GLfloat)h); LOGOPENGLERROR();
    glUniform1i(uTex, 0); LOGOPENGLERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glEnableVertexAttribArray(aCoord); LOGOPENGLERROR();
    glVertexAttribPointer(aCoord, 2, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat)*4, 0); LOGOPENGLERROR();
    glEnableVertexAttribArray(aTexCoord); LOGOPENGLERROR();
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat)*4, (void *)(sizeof(GLfloat)*2)); LOGOPENGLERROR();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, g_quadVerticesCount); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
}

void AmariRender::resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
}

void AmariRender::update_texture(matrix_t* m) {
    texture_copy_matrix(tex.get(), m);

    if (use_blur) {
        kernel_apply_to_texture(tex.get(), blur_kernel.get());
    }

    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->size, tex->size, GL_RGBA,
                    GL_UNSIGNED_BYTE, (const GLubyte *)tex->data); LOGOPENGLERROR();
}

void AmariRender::set_blur(double blur) {
    blur_sigma = blur;
    if (blur_sigma > 0.0) {
        blur_kernel = KernelGuard_t(kernel_create(blur_sigma, MODE_WRAP), kernel_free);
    }
}

void AmariRender::add_blur(double dblur) {
    double new_blur_sigma = blur_sigma + dblur;

    if (new_blur_sigma > 0.0) {
        use_blur = true;
        LOGI << "Blur Sigma " << new_blur_sigma;
    } else if (new_blur_sigma < 0.0) {
        use_blur = false;
        LOGI << "Turned Blur Off";
    }

    set_blur(new_blur_sigma);
}
