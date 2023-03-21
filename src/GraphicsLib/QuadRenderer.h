#pragma once

class QuadRenderer {
public:
    QuadRenderer() = default;
    ~QuadRenderer();

    bool Init(GLuint p, const hmm_vec4& area, const FloatColor& color);
    void Release();
    void Render(const hmm_mat4& mvp, float zoom, const hmm_vec2& offset);
    void Resize(int w, int h);
    void SetColor(const FloatColor& newColor);

private:
    int width = 0, height = 0;
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei verticesCount = 0;
    GLint uMvp = -1, uColor = -1, uZoom = -1, uOffset = -1, uRes = -1;
    FloatColor color;
};
