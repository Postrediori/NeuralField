#pragma once

class PlainTextureRenderer {
public:
    PlainTextureRenderer() = default;

    bool Init(GLuint program);
    void Resize(int newWidth, int newHeight);
    void Render();

    void AdjustViewport();

    void SetTexture(GLuint t);
    void SetMvp(const HMM_Mat4& mvp);

private:
    int width = 0, height = 0;

    GLuint texture = 0;

    GLuint program = 0;
    GLint uRes = -1, uMvp = -1, uTex = -1;

    HMM_Mat4 mvp;

    GraphicsUtils::unique_vertex_array vao;
    GraphicsUtils::unique_buffer vbo;
    GraphicsUtils::unique_buffer indVbo;
};
