#include "stdafx.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "AmariRender.h"

static const size_t BitsPerPixel = 4;

/*****************************************************************************
 * AmariRender
 ****************************************************************************/
static const size_t QuadVerticesCount = 4;
static const GLfloat QuadVertices[] = {
    -1.0f, -1.0f, 0.f, 0.f,
    -1.0f,  1.0f, 0.f, 1.f,
    1.0f, -1.0f, 1.f, 0.f,
    1.0f,  1.0f, 1.f, 1.f,
};

static const char vertex_src[] = 
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

static const char fragment_src[] =
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
 : use_blur(true), blur_sigma(1.f), tex_data(NULL) {
    //
}

AmariRender::~AmariRender() {
    //
}

bool AmariRender::init(size_t size) {
    GLuint genbuf[1];

    // Allocate mem
    tex_size = size;
    tex_data_size = size * size;
    tex_data = new GLubyte[tex_data_size * BitsPerPixel];
    memset(tex_data, 0, sizeof(GLubyte) * BitsPerPixel * tex_data_size);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_size, tex_size, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL); LOGOPENGLERROR();

    // Init VBO
    glGenBuffers(1, genbuf); LOGOPENGLERROR();
    vbo = genbuf[0];
    if (!vbo) {
        LOGE << "Unable to initialize VBO for Amari Renderer";
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW); LOGOPENGLERROR();

    // Init shader
    if (!program.load(vertex_src, fragment_src)) {
        LOGE << "Unable to load shader for Amari Renderer";
        return false;
    }

    aCoord = program.attrib("coord");
    aTexCoord = program.attrib("tex_coord");
    uTex = program.uniform("tex");
    uResolution = program.uniform("iRes");
    uMVP = program.uniform("mvp");

    return true;
}

void AmariRender::release() {
    if (tex_data) {
        delete[] tex_data;
        tex_data = NULL;
    }

    glDeleteTextures(1, &texture); LOGOPENGLERROR();
    glDeleteBuffers(1, &vbo); LOGOPENGLERROR();

    // program.release();
}

void AmariRender::render(const glm::mat4& mvp) {
    glActiveTexture(GL_TEXTURE0); LOGOPENGLERROR();
    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();

    glUseProgram(program.program()); LOGOPENGLERROR();

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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, QuadVerticesCount); LOGOPENGLERROR();
}

void AmariRender::resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
}

void AmariRender::update_texture(const float data[], const size_t size) {
#pragma omp parallel for
    for (int idx=0; idx<size; idx++) {
        GLubyte k = (data[idx]>0.f) ? 0xFF : 0x00;
        tex_data[BitsPerPixel*idx  ] = tex_data[BitsPerPixel*idx+1] = tex_data[BitsPerPixel*idx+2] = k;
        if (BitsPerPixel>3) {
            tex_data[BitsPerPixel*idx+3] = 0xFF;
        }
    }

    if (this->use_blur) {
        gaussian_filter(this->tex_data, this->tex_size, BitsPerPixel, this->blur_sigma);
    }

    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_size, tex_size, GL_RGBA,
                    GL_UNSIGNED_BYTE, tex_data); LOGOPENGLERROR();
}
