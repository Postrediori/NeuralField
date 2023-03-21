#pragma once

class PlainTextureRenderer {
public:
    PlainTextureRenderer() = default;
    ~PlainTextureRenderer();

    bool Init(GLuint program);
    void Release();
    void Resize(int newWidth, int newHeight);
    void Render();

    void AdjustViewport();

    void SetTexture(GLuint t);
    void SetMvp(const hmm_mat4& mvp);

private:
    int width = 0, height = 0;

    GLuint texture = 0;

    GLuint program = 0;
    GLint uRes = -1, uMvp = -1, uTex = -1;

    GLuint vao = 0;
    GLuint vbo = 0, indVbo = 0;

    hmm_mat4 mvp;
};
