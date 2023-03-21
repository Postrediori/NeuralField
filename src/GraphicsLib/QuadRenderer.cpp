#include "stdafx.h"
#include "GraphicsUtils.h"
#include "GraphicsLogger.h"
#include "GraphicsResource.h"
#include "QuadRenderer.h"

bool QuadRenderer::Init(GLuint program, const hmm_vec4& area, const FloatColor& quadColor) {
    this->program = program;

    this->SetColor(quadColor);

    uMvp = glGetUniformLocation(program, "mvp"); LOGOPENGLERROR();
    uColor = glGetUniformLocation(program, "color"); LOGOPENGLERROR();
    uZoom = glGetUniformLocation(program, "zoom"); LOGOPENGLERROR();
    uOffset = glGetUniformLocation(program, "ofs"); LOGOPENGLERROR();
    uRes = glGetUniformLocation(program, "res"); LOGOPENGLERROR();

    glGenVertexArrays(1, vao.put()); LOGOPENGLERROR();
    if (!vao) {
        return false;
    }
    glBindVertexArray(vao.get()); LOGOPENGLERROR();

    glGenBuffers(1, vbo.put()); LOGOPENGLERROR();
    if (!vbo) {
        return false;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo.get()); LOGOPENGLERROR();

    const std::vector<hmm_vec2> vertices = {
        {area.Elements[0], area.Elements[2]},
        {area.Elements[0], area.Elements[3]},
        {area.Elements[1], area.Elements[2]},

        {area.Elements[1], area.Elements[2]},
        {area.Elements[0], area.Elements[3]},
        {area.Elements[1], area.Elements[3]},
    };
    verticesCount = vertices.size();

    glBufferData(GL_ARRAY_BUFFER, sizeof(hmm_vec2) * verticesCount,
        vertices.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    GLint aCoord = glGetAttribLocation(program, "coord"); LOGOPENGLERROR();

    glEnableVertexAttribArray(aCoord); LOGOPENGLERROR();
    glVertexAttribPointer(aCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); LOGOPENGLERROR();

    glBindVertexArray(0); LOGOPENGLERROR();

    return true;
}

void QuadRenderer::Render(const hmm_mat4& mvp, float zoom, const hmm_vec2& offset) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao.get()); LOGOPENGLERROR();

    glUniformMatrix4fv(uMvp, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp)); LOGOPENGLERROR();
    glUniform1f(uZoom, zoom); LOGOPENGLERROR();
    glUniform2fv(uOffset, 1, reinterpret_cast<const GLfloat*>(&offset)); LOGOPENGLERROR();
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

void QuadRenderer::SetColor(const FloatColor& newColor) {
    color = newColor;
}
