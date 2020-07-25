#pragma once

enum class BlurDirection : int {
    VerticalBlur = 1,
    HorizontalBlur = 2,
};

class TextureRenderer {
public:
    TextureRenderer();
    ~TextureRenderer();

    bool init(size_t size);
    bool initTextures(size_t size);

    void render(const glm::mat4& mvp);
    void resize(unsigned int w, unsigned int h);

    void updateTexture(matrix_t* m);

    void setBlur(double blur);
    void addBlur(double dblur);

    void setUseBlur(bool newUseBlur);

private:
    void initBlurKernelTex();

    void releaseTextures();
    void release();

private:
    unsigned int w = 0, h = 0;
    size_t size = 0;

    bool useBlur = false;
    double blurSigma = 0.0;
    KernelGuard_t blurKernel;
    GLuint blurKernelTexture = 0;

    TextureGuard_t tex;
    GLuint texture = 0;
    GLuint blurTextureInter = 0;

    FrameBufferWrapper frameBuffer;

    PlainTextureRenderer blurPreRenderer;
    PlainTextureRenderer screenRenderer;

    struct {
        GLuint p = 0;
    } program;

    struct {
        GLuint p = 0;
        GLint uBlurDir = -1;
        GLint uBlurKernelTex = -1;
    } blurProgram;
};
