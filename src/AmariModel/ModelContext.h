#pragma once

enum RenderMode {
    RENDER_TEXTURE,
    RENDER_CONTOUR,
    RENDER_PARALLEL,

    RENDER_MAXIMAL
};

class AmariModelContext {
public:
    AmariModelContext();
    ~AmariModelContext();
    
    bool Init();
    void Release();
    
    void Render();
    void Resize(int w, int h);
    void SetActivity(int x, int y);
    void Restart();
    void Update();
    
    void SetRenderMode(RenderMode mode);
    void SwitchBlur();
    void IncreaseBlur();
    void DecreaseBlur();
    
public:
    int windowWidth_, windowHeight_, size_;
    float scaleX_, scaleY_;

    RenderMode renderMode_;

    GLuint program_, vertex_, fragment_;
    
    FPSCounter fpsCounter_;
    
    Math::mat4f mvp_;
    
    FontRenderer fr_;
    FontHandle_t a24;
    
    AmariModel amariModel_;
    AmariRender amariRender_;

    ContourPlotGuard_t contourLines_;
    ContourPlotGuard_t contourFill_;
    ContourPlotGuard_t contourParallel_;
    
    bool showHelp_;
};
