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

static const float g_UiWidth = 250.0f;
static const float g_UiMargin = 0.0f;


NeuralFieldContext::NeuralFieldContext()
    : renderMode_(RENDER_TEXTURE)
    , program_(0) {
}

NeuralFieldContext::~NeuralFieldContext() {
    Release();
}

bool NeuralFieldContext::Init() {
    showUi_ = true;
    
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
    mvp_ = glm::ortho(g_area.xmin, g_area.xmax, g_area.ymin, g_area.ymax);

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
    static const float zoom = 1.f;
    static const glm::vec2 offset(0.f, 0.f);

    if (renderMode_ == RENDER_TEXTURE) {
        renderer_.render(mvp_);

    } else if (renderMode_ == RENDER_CONTOUR) {
        contourLines_->render(mvp_, zoom, offset, g_outline);

    } else if (renderMode_ == RENDER_PARALLEL) {
        contourParallel_->render(mvp_, zoom, offset, g_outline);

    } else if (renderMode_ == RENDER_FILL) {
        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourFill_->render(mvp_, zoom, offset, g_foreground);

        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourLines_->render(mvp_, zoom, offset, g_outline);

    } else if (renderMode_ == RENDER_PARALLEL_FILL) {
        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourParallelFill_->render(mvp_, zoom, offset, g_foreground);

        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourParallel_->render(mvp_, zoom, offset, g_outline);
    }

    if (showUi_) {
        this->RenderUi();
    }
}

void NeuralFieldContext::RenderUi() {
    ImVec2 uiSize = ImVec2(g_UiWidth, windowHeight_ - g_UiMargin * 2);

    ImGui::SetNextWindowPos(ImVec2(g_UiMargin, g_UiMargin), ImGuiCond_Always);
    ImGui::SetNextWindowSize(uiSize, ImGuiCond_Always);

    ImGui::Begin("Neural Field", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Text("Rendering mode:");
    for (int idx = 0; idx < RenderMode::RENDER_MODES; idx++) {
        ImGui::RadioButton(g_renderModeLabels[idx], (int *)&this->renderMode_, (int)idx);
        if (idx == 0) {
            ImGui::SameLine();
            if (ImGui::Checkbox("Texture Blur", (bool*)&this->textureBlur_)) {
                renderer_.use_blur = this->textureBlur_;
            }
        }
    }

    ImGui::Separator();

    ImGui::Text("User Guide:");
    ImGui::BulletText("F1 to on/off fullscreen mode.");
    ImGui::BulletText("F2 to show/hide UI.");
    ImGui::BulletText("RMB to Clear model.");
    ImGui::BulletText("LMB to Activate model in a point.");
    ImGui::BulletText("B to toggle texture blur on/off.");
    ImGui::BulletText("F2 to show/hide UI.");

    ImGui::Separator();

    ImGui::Text("FPS Counter: %.1f", fps_);

    ImGui::End();
}

void NeuralFieldContext::Resize(int w, int h) {
    windowWidth_ = w;
    windowHeight_ = h;

    //scaleX_ = 2.f / (float)windowWidth_;
    //scaleY_ = 2.f / (float)windowHeight_;

    int newW = w - g_UiWidth - g_UiMargin * 2;
    double newScale = 2.0 / (double)(newW);
    double newLeft = g_area.xmin - g_UiWidth * newScale;
    mvp_ = glm::ortho(newLeft, g_area.xmax, g_area.ymin, g_area.ymax);

    renderer_.resize(newW, h);
    contourLines_->resize(newW, h);
    contourFill_->resize(newW, h);
    contourParallel_->resize(newW, h);
    contourParallelFill_->resize(newW, h);
}

void NeuralFieldContext::SetActivity(int x, int y) {
    int cx = 0, cy = 0;

    int w = windowWidth_ - g_UiWidth;
    int h = windowHeight_;

    if (w > h) {
        cx = x - (w - h) / 2 - g_UiWidth;
        cy = y;
    } else {
        cx = x - g_UiWidth;
        cy = y - (h - w) / 2;
    }

    int size = (w > h) ? h : w;

    if (cx<0 || cy<0 || cx>size || cy>size) {
        return;
    }

    int n = (int)((float)cx/(float)size * model_.size);
    int m = (int)((1.f - (float)cy/(float)size) * model_.size);

    model_.set_activity(n, m, 1.f);

    LOGI << "Set Activity at [" << n << "," << m << "]";
}

void NeuralFieldContext::Restart() {
    model_.restart();
    LOGI << "Reset Model";
}

void NeuralFieldContext::Update(double t) {
    static double lastTime = 0.0;
    static double lastFpsTime = 0.0;
    double currentTime = t;
    double dt = currentTime - lastTime;
    lastTime = currentTime;

    if (currentTime - lastFpsTime > 1.0) {
        fps_ = ImGui::GetIO().Framerate;
        lastFpsTime = currentTime;
    }

    model_.stimulate(/* dt */);

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
    textureBlur_ = !textureBlur_;
    renderer_.use_blur = textureBlur_;
    if (textureBlur_) {
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
    showUi_ = !showUi_;
}
