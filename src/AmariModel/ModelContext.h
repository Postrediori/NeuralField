#pragma once

#include "FreeType.h"
#include "AmariModel.h"
#include "AmariRender.h"
#include "ContourPlot.h"

static const int Width = 512;
static const int Height = 512;


enum RenderMode {
    RENDER_TEXTURE,
    RENDER_CONTOUR,
    RENDER_PARALLEL,

    RENDER_MAXIMAL
};

static const char* const g_renderModeLabels[] = {
    "Texture", "Contour", "Contour (Parallel)"
};

class AmariModelContext {
public:
    AmariModelContext();
    ~AmariModelContext();
    
    bool Init();
    
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
    
    FPSCounter fpsCounter_;
        
    glm::mat4 model_, view_, projection_;
    
    FontRenderer fr_;
    
    AmariModel amariModel_;
    AmariRender amariRender_;
    
    ShaderFiles contourProgram_;
    ContourPlotGuard_t contourLines_;
    ContourPlotGuard_t contourFill_;
    ContourPlotGuard_t contourParallel_;
    // ContourPlotGuard_t contourParallelFill_;
    
    bool showHelp_;
};
