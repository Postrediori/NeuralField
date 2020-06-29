#pragma once

enum RenderMode : int {
    RENDER_TEXTURE = 0,
    RENDER_CONTOUR = 1,
    RENDER_PARALLEL = 2,
    RENDER_FILL = 3,
    RENDER_PARALLEL_FILL = 4,

    RENDER_MODES = 5
};

class NeuralFieldContext {
public:
    NeuralFieldContext();
    ~NeuralFieldContext();
    
    bool Init();
    void Release();
    
    void Render();
    void Resize(int w, int h);
    void SetActivity(int x, int y);
    void Restart();
    void Update(double t);
    
    void SetRenderMode(RenderMode mode);
    void SwitchBlur();
    void IncreaseBlur();
    void DecreaseBlur();

private:
    void RenderUi();

private:
    int windowWidth_ = 0, windowHeight_ = 0;

    RenderMode renderMode_ = RenderMode::RENDER_TEXTURE;

    GLuint program_ = 0;
    
    glm::mat4 mvp_;
    
    NeuralFieldModel model_;
    TextureRenderer renderer_;

    ContourLine contourLines_;
    ContourFill contourFill_;
    ContourParallel contourParallel_;
    ContourParallelFill contourParallelFill_;
    
    bool showUi_ = true;

    float fps_ = 0.0f;

    bool textureBlur_ = true;

    ConfigMap_t modelConfig_;

    QuadRenderer quad_;
};
