#pragma once

enum class BlurDirection : int {
    VerticalBlur = 1,
    HorizontalBlur = 2,
};

class TextureRenderer {
public:
    TextureRenderer() = default;
    ~TextureRenderer();

    bool Init(NeuralFieldModel* model, const std::filesystem::path& moduleDataDir, size_t size);
    bool InitTextures(size_t size);

    void Render(const hmm_mat4& mvp);
    void Resize(unsigned int w, unsigned int h);

    void UpdateTexture(matrix_t* m);

    void SetBlur(double blur);
    void AddBlur(double dblur);

    void SetUseBlur(bool newUseBlur);

private:
    void ReleaseTextures();
    void Release();

private:
    unsigned int w = 0, h = 0;
    size_t size = 0;

    bool useBlur = false;
    double blurSigma = 0.0;
    KernelGuard_t blurKernel;

    MatrixGuard_t tex, tempTex;
    GraphicsUtils::unique_texture texture;

    PlainTextureRenderer screenRenderer;

    GraphicsUtils::unique_program program;

    NeuralFieldModel* model_ = nullptr;
};
