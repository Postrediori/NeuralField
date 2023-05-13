#pragma once

class QuadRenderer {
public:
    QuadRenderer() = default;

    bool Init(GLuint program, const HMM_Vec4& area, const FloatColor& color);
    void Render(const HMM_Mat4& mvp, float zoom, const HMM_Vec2& offset);
    void Resize(int w, int h);
    void SetColor(const FloatColor& newColor);

private:
    int width = 0, height = 0;
    GLuint program = 0;
    GLint uMvp = -1, uColor = -1, uZoom = -1, uOffset = -1, uRes = -1;
    GraphicsUtils::unique_vertex_array vao;
    GraphicsUtils::unique_buffer vbo;
    GLsizei verticesCount = 0;
    FloatColor color;
};
