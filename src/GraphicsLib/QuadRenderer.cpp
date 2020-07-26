#include "stdafx.h"
#include "GlUtils.h"
#include "QuadRenderer.h"

QuadRenderer::~QuadRenderer() {
    Release();
}

bool QuadRenderer::Init(GLuint p, const std::array<double, 4>& area,
    const std::array<GLfloat, 4>& quadColor) {
    program = p;

    this->SetColor(quadColor);

    uMvp = glGetUniformLocation(program, "mvp"); LOGOPENGLERROR();
    uColor = glGetUniformLocation(program, "color"); LOGOPENGLERROR();
    uZoom = glGetUniformLocation(program, "zoom"); LOGOPENGLERROR();
    uOffset = glGetUniformLocation(program, "ofs"); LOGOPENGLERROR();
    uRes = glGetUniformLocation(program, "res"); LOGOPENGLERROR();

    glGenVertexArrays(1, &vao); LOGOPENGLERROR();
    if (!vao) {
        return false;
    }
    glBindVertexArray(vao); LOGOPENGLERROR();

    glGenBuffers(1, &vbo); LOGOPENGLERROR();
    if (!vbo) {
        return false;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();

    std::vector<glm::vec2> vertices = {
        {area[0], area[2]},
        {area[0], area[3]},
        {area[1], area[2]},

        {area[1], area[2]},
        {area[0], area[3]},
        {area[1], area[3]},
    };
    verticesCount = vertices.size();

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * verticesCount,
        vertices.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    GLint aCoord = glGetAttribLocation(program, "coord"); LOGOPENGLERROR();

    glEnableVertexAttribArray(aCoord); LOGOPENGLERROR();
    glVertexAttribPointer(aCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); LOGOPENGLERROR();

    glBindVertexArray(0); LOGOPENGLERROR();

    return true;
}

void QuadRenderer::Release() {
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
}

void QuadRenderer::Render(const glm::mat4& mvp, float zoom, const glm::vec2& offset) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glUniformMatrix4fv(uMvp, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform1f(uZoom, zoom); LOGOPENGLERROR();
    glUniform2fv(uOffset, 1, glm::value_ptr(offset)); LOGOPENGLERROR();
    glUniform4fv(uColor, 1, color.data()); LOGOPENGLERROR();
    glUniform2f(uRes, static_cast<GLfloat>(width), static_cast<GLfloat>(height)); LOGOPENGLERROR();

    glDrawArrays(GL_TRIANGLES, 0, verticesCount); LOGOPENGLERROR();

    glBindVertexArray(0); LOGOPENGLERROR();
    glUseProgram(0); LOGOPENGLERROR();
}

void QuadRenderer::Resize(int w, int h) {
    width = w;
    height = h;
}

void QuadRenderer::SetColor(const std::array<float, 4> newColor) {
    color = newColor;
}
