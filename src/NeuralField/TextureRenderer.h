#pragma once

class TextureRenderer {
public:
    TextureRenderer();
    ~TextureRenderer();

    bool init(size_t size);
    bool initTexture(size_t size);
    void release();

    void render(const glm::mat4& mvp);
    void resize(unsigned int w, unsigned int h);

    void update_texture(matrix_t* m);

    void set_blur(double blur);
    void add_blur(double dblur);

    void setUseBlur(bool newUseBlur);

private:
    void initBlurKernelTex();

    void releaseTextures();

private:
    unsigned int w, h;
    size_t size;

    bool use_blur;
    double blur_sigma;
    KernelGuard_t blur_kernel;
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
