#pragma once

enum class BlurDirection : int {
    VerticalBlur = 1,
    HorizontalBlur = 2,
};

class TextureRenderer {
public:
    TextureRenderer() = default;
    ~TextureRenderer();

    bool Init(const std::filesystem::path& moduleDataDir, size_t size);
    bool InitTextures(size_t size);

    void Render(const hmm_mat4& mvp);
    void Resize(unsigned int w, unsigned int h);

    void UpdateTexture(matrix_t* m);

    void SetBlur(double blur);
    void AddBlur(double dblur);

    void SetUseBlur(bool newUseBlur);

private:
    void InitBlurKernelTex();

    void ReleaseTextures();
    void Release();

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
