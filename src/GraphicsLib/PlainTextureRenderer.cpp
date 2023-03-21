#include "stdafx.h"
#include "GraphicsLogger.h"
#include "PlainTextureRenderer.h"


const hmm_vec4 PlaneBounds = { -1.0f, 1.0f, -1.0f, 1.0f };

const std::vector<hmm_vec4> g_quadVertices = {
    {-1.0f, -1.0f, 0.f, 0.f},
    {-1.0f,  1.0f, 0.f, 1.f},
    {1.0f, -1.0f, 1.f, 0.f},
    {1.0f,  1.0f, 1.f, 1.f},
};

const std::vector<GLuint> g_quadIndices = {
    0, 1, 2,
    2, 1, 3,
};

/*****************************************************************************
 * PlainTextureRenderer
 ****************************************************************************/
PlainTextureRenderer::~PlainTextureRenderer() {
    Release();
}

bool PlainTextureRenderer::Init(GLuint p) {
    program = p;

    uRes = glGetUniformLocation(program, "iRes"); LOGOPENGLERROR();
    uMvp = glGetUniformLocation(program, "mvp"); LOGOPENGLERROR();
    uTex = glGetUniformLocation(program, "tex"); LOGOPENGLERROR();

    mvp = HMM_Orthographic(PlaneBounds.X, PlaneBounds.Y, PlaneBounds.Z, PlaneBounds.W, 1.f, -1.f);

    // Init VAO
    glGenVertexArrays(1, &vao); LOGOPENGLERROR();
    if (!vao) {
        LOGE << "Failed to create VAO for planar texture";
        return false;
    }
    glBindVertexArray(vao); LOGOPENGLERROR();

    // Init VBO
    glGenBuffers(1, &vbo); LOGOPENGLERROR();
    if (!vbo) {
        LOGE << "Failed to create VBO for planar texture";
        return false;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quadVertices[0]) * g_quadVertices.size(),
        g_quadVertices.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    // Init indices VBO
    glGenBuffers(1, &indVbo);
    if (!indVbo) {
        LOGE << "Failed to create indices VBO";
        return false;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_quadIndices[0]) * g_quadIndices.size(),
        g_quadIndices.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    // Setup VAO
    GLint aCoord = glGetAttribLocation(program, "coord"); LOGOPENGLERROR();
    GLint aTexCoord = glGetAttribLocation(program, "tex_coord"); LOGOPENGLERROR();

    glEnableVertexAttribArray(aCoord); LOGOPENGLERROR();
    glVertexAttribPointer(aCoord, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, (void *)(0)); LOGOPENGLERROR();

    glEnableVertexAttribArray(aTexCoord); LOGOPENGLERROR();
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, (void *)(sizeof(GLfloat) * 2)); LOGOPENGLERROR();

    glBindVertexArray(0); LOGOPENGLERROR();

    return true;
}

void PlainTextureRenderer::SetTexture(GLuint t) {
    texture = t;
}

void PlainTextureRenderer::SetMvp(const hmm_mat4& newMvp) {
    mvp = newMvp;
}

void PlainTextureRenderer::Release() {
    if (vao) {
        glDeleteVertexArrays(1, &vao); LOGOPENGLERROR();
        vao = 0;
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo); LOGOPENGLERROR();
        vbo = 0;
    }
}

void PlainTextureRenderer::Resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
}

void PlainTextureRenderer::AdjustViewport() {
    glViewport(0, 0, width, height); LOGOPENGLERROR();
}

void PlainTextureRenderer::Render() {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glActiveTexture(GL_TEXTURE0); LOGOPENGLERROR();
    glBindTexture(GL_TEXTURE_2D, texture); LOGOPENGLERROR();

    glUniformMatrix4fv(uMvp, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp)); LOGOPENGLERROR();
    glUniform2f(uRes, static_cast<GLfloat>(width), static_cast<GLfloat>(height)); LOGOPENGLERROR();
    glUniform1i(uTex, 0); LOGOPENGLERROR();

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_quadIndices.size()), GL_UNSIGNED_INT, nullptr); LOGOPENGLERROR();

    glBindVertexArray(0); LOGOPENGLERROR();
    glUseProgram(0); LOGOPENGLERROR();
}
