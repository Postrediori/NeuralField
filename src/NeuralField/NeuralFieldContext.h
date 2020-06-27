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

    void ToggleUi();

private:
    void RenderUi();

private:
    int windowWidth_, windowHeight_;
    //float scaleX_, scaleY_;

    RenderMode renderMode_;

    GLuint program_;
    
    glm::mat4 mvp_;
    
    FontRenderer fr_;
    FontHandle_t a24;
    
    NeuralFieldModel model_;
    TextureRenderer renderer_;

    ContourPlotGuard_t contourLines_;
    ContourPlotGuard_t contourFill_;
    ContourPlotGuard_t contourParallel_;
    ContourPlotGuard_t contourParallelFill_;
    
    bool showUi_;

    float fps_;

    bool textureBlur_ = true;
};
