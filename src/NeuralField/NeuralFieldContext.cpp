#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "MathUtils.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "Shader.h"
#include "FreeType.h"
#include "NeuralFieldModel.h"
#include "TextureRenderer.h"
#include "ContourPlot.h"
#include "ContourLine.h"
#include "ContourFill.h"
#include "ContourParallel.h"
#include "ContourParallelFill.h"
#include "NeuralFieldContext.h"


static const GLfloat g_background[] = {0.00f, 0.00f, 0.00f, 1.00f};
static const GLfloat g_foreground[] = {0.50f, 0.50f, 1.00f, 1.00f};
static const GLfloat g_outline[] = {1.00f, 1.00f, 1.00f, 1.00f};
static const GLfloat g_textColor[] = {1.00f, 1.00f, 1.00f, 1.00f};

static const FontSize_t g_fontSize = 24;

static const char* const g_renderModeLabels[RENDER_MODES] = {
    "Texture", "Contour", "Contour Parallel", "Fill", "Filled Parallel"
};

static const char g_fontFile[] = "data/font.ttf";
static const char g_configFile[] = "data/amari.conf";
static const char g_vertexShader[] = "data/plane.vert";
static const char g_fragmentShader[]  = "data/plane.frag";

static const area_t g_area = {-1.0, 1.0, -1.0, 1.0};

static const int g_timerInterval = 10;
static const float g_textureBlurDelta = 0.1f;


NeuralFieldContext::NeuralFieldContext()
    : renderMode_(RENDER_CONTOUR)
    , program_(0) {
}

NeuralFieldContext::~NeuralFieldContext() {
    Release();
}

bool NeuralFieldContext::Init() {
    showHelp_ = true;
    
    // Init font
    if (!fr_.init()) {
        LOGE << "Unable to initialize FreeType Font Loader";
        return false;
    }
    if (!fr_.load(g_fontFile)) {
        return false;
    }
    a24 = fr_.createAtlas(g_fontSize);

    // Init MVP matrices
    mvp_ = Math::GetOrthoProjection(g_area.xmin, g_area.xmax, g_area.ymin, g_area.ymax);

    // Init model
    if (!model_.init(g_configFile)) {
        LOGE << "Unable to load Amari Model Config from file " << g_configFile;
        return false;
    }

    // Init render
    if (!renderer_.init(model_.size)) {
        LOGE << "Unable to init Amari Model Renderer";
        return false;
    }
    
    renderer_.update_texture(model_.activity.get());

    // Init contour lines
    if (!Shader::createProgram(program_, g_vertexShader, g_fragmentShader)) {
        LOGE << "Unable to create shader for contour lines";
        return false;
    }

    contourLines_.reset(new ContourLine(program_));
    if (!contourLines_->init()) {
        LOGE << "Unable to create Contour Lines";
        return false;
    }

    contourFill_.reset(new ContourFill(program_));
    if (!contourFill_->init()) {
        LOGE << "Unable to create Filled Contour";
        return false;
    }

    contourParallel_.reset(new ContourParallel(program_));
    if (!contourParallel_->init()) {
        LOGE << "Unable to create Parallel Contour Lines";
        return false;
    }

    contourParallelFill_.reset(new ContourParallelFill(program_));
    if (!contourParallelFill_->init()) {
        LOGE << "Unable to create Parallel Contour Lines";
        return false;
    }

    contourLines_->update(model_.activity.get(), g_area, 1.0);
    contourFill_->update(model_.activity.get(), g_area, 1.0);
    contourParallelFill_->update(model_.activity.get(), g_area, 1.0);
    contourParallel_->update(model_.activity.get(), g_area, 1.0);
    
    // Set up OpenGL
    glClearColor(g_background[0], g_background[1],
                 g_background[2], g_background[3]); LOGOPENGLERROR();
    glClearDepth(1.); LOGOPENGLERROR();
    
    return true;
}

void NeuralFieldContext::Release() {
    if (program_) {
        glDeleteProgram(program_);
        program_ = 0;
    }
}

void NeuralFieldContext::Render() {
    if (renderMode_ == RENDER_TEXTURE) {
        renderer_.render(mvp_);

    } else if (renderMode_ == RENDER_CONTOUR) {
        static const float zoom = 1.f;
        static const Math::vec2f offset(0.f, 0.f);

        contourLines_->render(mvp_, zoom, offset, g_outline);

    } else if (renderMode_ == RENDER_PARALLEL) {
        static const float zoom = 1.f;
        static const Math::vec2f offset(0.f, 0.f);

        contourParallel_->render(mvp_, zoom, offset, g_outline);

    } else if (renderMode_ == RENDER_FILL) {
        static const float zoom = 1.f;
        static const Math::vec2f offset(0.f, 0.f);

        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourFill_->render(mvp_, zoom, offset, g_foreground);

        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourLines_->render(mvp_, zoom, offset, g_outline);

    } else if (renderMode_ == RENDER_PARALLEL_FILL) {
        static const float zoom = 1.f;
        static const Math::vec2f offset(0.f, 0.f);

        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourParallelFill_->render(mvp_, zoom, offset, g_foreground);

        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourParallel_->render(mvp_, zoom, offset, g_outline);
    }

    if (showHelp_) {
        fr_.renderStart();
        fr_.renderColor(g_textColor);

        fr_.renderText(a24, {-1+8*scaleX_, -1+200*scaleY_, scaleX_, scaleY_}, "1-5 ... Choose plot mode");
        fr_.renderText(a24, {-1+8*scaleX_, -1+175*scaleY_, scaleX_, scaleY_}, "B ... Turn texture blur on/off");
        fr_.renderText(a24, {-1+8*scaleX_, -1+150*scaleY_, scaleX_, scaleY_}, "LMB ... Activate model");
        fr_.renderText(a24, {-1+8*scaleX_, -1+125*scaleY_, scaleX_, scaleY_}, "RMB ... Clear model");
        fr_.renderText(a24, {-1+8*scaleX_, -1+100*scaleY_, scaleX_, scaleY_}, "F2 ... Show/hide help");
        fr_.renderText(a24, {-1+8*scaleX_, -1+75*scaleY_,  scaleX_, scaleY_}, "F1 ... Fullscreen on/off");
        
        std::stringstream str;
        str << "Display Mode : " << g_renderModeLabels[renderMode_];
        fr_.renderText(a24, {-1+8*scaleX_, -1+50*scaleY_,  scaleX_, scaleY_}, str.str());

        str.str(std::string());
        str << "FPS : " << fpsCounter_.fps;
        fr_.renderText(a24, {-1+8*scaleX_, -1+25*scaleY_, scaleX_, scaleY_}, str.str());

        fr_.renderEnd();
    }
    
    fpsCounter_.update(glfwGetTime());
}

void NeuralFieldContext::Resize(int w, int h) {
    windowWidth_ = w;
    windowHeight_ = h;

    scaleX_ = 2.f / (float)windowWidth_;
    scaleY_ = 2.f / (float)windowHeight_;

    if (w > h) {
        size_ = h;
    } else {
        size_ = w;
    }

    renderer_.resize(w, h);
    contourLines_->resize(w, h);
    contourFill_->resize(w, h);
    contourParallel_->resize(w, h);
    contourParallelFill_->resize(w, h);
}

void NeuralFieldContext::SetActivity(int x, int y) {
    int cx, cy;

    if (windowWidth_ > windowHeight_) {
        cx = x - (windowWidth_ - windowHeight_) / 2;
        cy = y;
    } else {
        cx = x;
        cy = y - (windowHeight_ - windowWidth_) / 2;
    }

    if (cx<0 || cy<0 || cx>size_ || cy>size_) {
        return;
    }

    int n, m;
    n = (int)((float)cx/(float)size_ * model_.size);
    m = (int)((1.f - (float)cy/(float)size_) * model_.size);

    model_.set_activity(n, m, 1.f);

    LOGI << "Set Activity at [" << n << "," << m << "]";
}

void NeuralFieldContext::Restart() {
    model_.restart();
    LOGI << "Reset Model";
}

void NeuralFieldContext::Update() {
    model_.stimulate();

    switch (renderMode_) {
    case RENDER_TEXTURE:
        renderer_.update_texture(model_.activity.get());
        break;

    case RENDER_CONTOUR:
        if (contourLines_) {
            contourLines_->update(model_.activity.get(), g_area, 0.0);
        }
        break;

    case RENDER_PARALLEL:
        if (contourParallel_) {
            contourParallel_->update(model_.activity.get(), g_area, 0.0);
        }
        break;

    case RENDER_FILL:
        if (contourLines_) {
            contourLines_->update(model_.activity.get(), g_area, 0.0);
        }

        if (contourFill_) {
            contourFill_->update(model_.activity.get(), g_area, 0.0);
        }
        break;

    case RENDER_PARALLEL_FILL:
        if (contourParallel_) {
            contourParallel_->update(model_.activity.get(), g_area, 0.0);
        }

        if (contourParallelFill_) {
            contourParallelFill_->update(model_.activity.get(), g_area, 0.0);
        }
        break;

    default:
        break;
    }
}

void NeuralFieldContext::SetRenderMode(RenderMode mode) {
    renderMode_ = mode;
}

void NeuralFieldContext::SwitchBlur() {
    renderer_.use_blur = !renderer_.use_blur;
    if (renderer_.use_blur) {
        LOGI << "Turned Blur On";
    } else {
        LOGI << "Turned Blur Off";
    }
}

void NeuralFieldContext::IncreaseBlur() {
    renderer_.add_blur(g_textureBlurDelta);
}

void NeuralFieldContext::DecreaseBlur() {
    renderer_.add_blur(-g_textureBlurDelta);
}

void NeuralFieldContext::ToggleUi() {
    showHelp_ = !showHelp_;
}
