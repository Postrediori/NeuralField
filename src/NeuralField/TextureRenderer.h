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

private:
    void initBlurKernelTex();

    void releaseTextures();
    
public:
    unsigned int w, h;
    size_t size;

    bool use_blur;
    double blur_sigma;
    KernelGuard_t blur_kernel;
    GLuint blurKernelTexture = 0;

    TextureGuard_t tex;
    GLuint texture;
    GLuint blurTextureInter = 0;

    GLuint vao, blurVao;
    GLuint vbo, indVbo;

    FrameBufferWrapper frameBuffer;

    struct {
        GLuint p = 0;
        GLint uMVP = -1, uResolution = -1, uTex = -1;
    } program;

    struct {
        GLuint p = 0;
        GLint uTex = -1;
        GLint uBlurDir = -1;
        GLint uBlurKernelTex = -1;
    } blurProgram;
};
