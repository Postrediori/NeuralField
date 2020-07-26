#pragma once

class QuadRenderer {
public:
    ~QuadRenderer();

    bool Init(GLuint p, const std::array<double, 4>& area,
        const std::array<float, 4>& color);
    void Release();
    void Render(const glm::mat4& mvp, float zoom, const glm::vec2& offset);
    void Resize(int w, int h);
    void SetColor(const std::array<float, 4> newColor);

private:
    int width = 0, height = 0;
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei verticesCount = 0;
    GLint uMvp = -1, uColor = -1, uZoom = -1, uOffset = -1, uRes = -1;
    std::array<float, 4> color;
};
