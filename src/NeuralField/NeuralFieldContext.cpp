#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "MathUtils.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "Shader.h"
#include "NeuralFieldModel.h"
#include "TextureRenderer.h"
#include "ContourPlot.h"
#include "ContourLine.h"
#include "ContourFill.h"
#include "ContourParallel.h"
#include "ContourParallelFill.h"
#include "QuadRenderer.h"
#include "NeuralFieldContext.h"


static const std::array<GLfloat, 4> g_background = {0.15f, 0.15f, 0.15f, 1.00f};
static const std::array<GLfloat, 4> g_foreground = {0.50f, 0.50f, 1.00f, 1.00f};
static const std::array<GLfloat, 4> g_outline = {1.00f, 1.00f, 1.00f, 1.00f};

static const std::vector<std::string> g_renderModeLabels = {
    "Texture",
    "Contour",
    "Contour Parallel",
    "Fill",
    "Filled Parallel"
};

static const std::string g_configFile = "data/amari.conf";
static const std::string g_vertexShader = "data/plane.vert";
static const std::string g_fragmentShader = "data/plane.frag";

static const area_t g_area = {-1.0, 1.0, -1.0, 1.0};

static const float g_textureBlurDelta = 0.1f;

static const float g_UiWidth = 250.0f;

NeuralFieldContext::NeuralFieldContext() {
}

NeuralFieldContext::~NeuralFieldContext() {
    Release();
}

bool NeuralFieldContext::Init() {
    // Init MVP matrices
    mvp_ = glm::ortho(g_area.xmin, g_area.xmax, g_area.ymin, g_area.ymax);

    // Init model
    if (!ParseConfigFile(modelConfig_, g_configFile)) {
        LOGE << "Unable to load Model Config from file " << g_configFile;
        return false;
    }

    model_.init(modelConfig_);

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

    quad_.Init(program_, { g_area.xmin, g_area.xmax, g_area.ymin, g_area.ymax });

    if (!contourLines_.init(program_)) {
        LOGE << "Unable to create Contour Lines";
        return false;
    }

    if (!contourFill_.init(program_)) {
        LOGE << "Unable to create Filled Contour";
        return false;
    }

    if (!contourParallel_.init(program_)) {
        LOGE << "Unable to create Parallel Contour Lines";
        return false;
    }

    if (!contourParallelFill_.init(program_)) {
        LOGE << "Unable to create Parallel Contour Lines";
        return false;
    }

    contourLines_.update(model_.activity.get(), g_area, 1.0);
    contourFill_.update(model_.activity.get(), g_area, 1.0);
    contourParallelFill_.update(model_.activity.get(), g_area, 1.0);
    contourParallel_.update(model_.activity.get(), g_area, 1.0);
    
    // Set up OpenGL
    glClearColor(0.0, 0.0, 0.0, 1.0); LOGOPENGLERROR();
    glClearDepth(1.); LOGOPENGLERROR();
    
    return true;
}

void NeuralFieldContext::Release() {
    if (program_) {
        glDeleteProgram(program_); LOGOPENGLERROR();
        program_ = 0;
    }
}

void NeuralFieldContext::Render() {
    static const float zoom = 1.f;
    static const glm::vec2 offset(0.f, 0.f);

    if (renderMode_ == RenderMode::Texture) {
        renderer_.render(mvp_);
    }
    else if (renderMode_ == RenderMode::Contour) {
        quad_.Render(mvp_, zoom, offset, g_background);
        contourLines_.render(mvp_, zoom, offset, g_outline);
    }
    else if (renderMode_ == RenderMode::ContourParallel) {
        quad_.Render(mvp_, zoom, offset, g_background);
        contourParallel_.render(mvp_, zoom, offset, g_outline);
    }
    else if (renderMode_ == RenderMode::Fill) {
        quad_.Render(mvp_, zoom, offset, g_background);

        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourFill_.render(mvp_, zoom, offset, g_foreground);

        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourLines_.render(mvp_, zoom, offset, g_outline);
    }
    else if (renderMode_ == RenderMode::FillParallel) {
        quad_.Render(mvp_, zoom, offset, g_background);

        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourParallelFill_.render(mvp_, zoom, offset, g_foreground);

        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourParallel_.render(mvp_, zoom, offset, g_outline);
    }

    if (showUi_) {
        this->RenderUi();
    }
}

void NeuralFieldContext::RenderUi() {
    ImVec2 uiSize = ImVec2(g_UiWidth, windowHeight_);

    ImGui::SetNextWindowPos(ImVec2(0.0, 0.0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(uiSize, ImGuiCond_Always);

    ImGui::Begin("Neural Field", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Text("Rendering mode:");
    for (int idx = 0; idx < static_cast<int>(RenderMode::RenderModesCount); idx++) {
        ImGui::RadioButton(g_renderModeLabels[idx].c_str(), (int *)&this->renderMode_, (int)idx);
        if (idx == 0) {
            ImGui::SameLine();
            if (ImGui::Checkbox("Texture Blur", (bool *)&this->textureBlur_)) {
                renderer_.use_blur = this->textureBlur_;
            }
        }
    }

    ImGui::Separator();

    static const std::map<std::string, int> g_ModelSizes = {
        {"128x128", 128},
        {"256x256", 256},
        {"512x512", 512}
    };
    static int gModelSize = static_cast<int>(modelConfig_["size"]);
    ImGui::Text("Model size:");
    for (const auto& s : g_ModelSizes) {
        if (ImGui::RadioButton(s.first.c_str(), &gModelSize, s.second)) {
            modelConfig_["size"] = gModelSize;
            renderer_.initTexture(gModelSize);
            model_.init(modelConfig_);
        }
        ImGui::SameLine();
    }

    ImGui::Separator();

    static const std::map<std::string, int> g_ModelModes = {
        {"wrap", static_cast<int>(KernelMode::MODE_WRAP)},
        {"reflect", static_cast<int>(KernelMode::MODE_REFLECT)},
        {"mirror", static_cast<int>(KernelMode::MODE_MIRROR)}
    };
    static int gModelMode = static_cast<int>(modelConfig_["mode"]);
    ImGui::Text("Border mode:");
    for (const auto& s : g_ModelModes) {
        if (ImGui::RadioButton(s.first.c_str(), &gModelMode, s.second)) {
            modelConfig_["mode"] = gModelMode;
            model_.init(modelConfig_);
        }
        ImGui::SameLine();
    }

    ImGui::Separator();

    ImGui::Text("Model params:");

    static float gModelH = -0.2;
    if (ImGui::SliderFloat("h", &gModelH, -0.3f, 0.0f)) {
        modelConfig_["h"] = gModelH;
        model_.init(modelConfig_);
    }

    static float gModelM = 0.065;
    if (ImGui::SliderFloat("M", &gModelM, 0.05f, 0.07)) {
        modelConfig_["M_"] = gModelM;
        model_.init(modelConfig_);
    }

    ImGui::Separator();

    ImGui::Text("User Guide:");
    ImGui::BulletText("F1 to on/off fullscreen mode.");
    ImGui::BulletText("RMB to Clear model.");
    ImGui::BulletText("LMB to Activate model in a point.");
    ImGui::BulletText("B to toggle texture blur on/off.");

    ImGui::Separator();

    ImGui::Text("FPS Counter: %.1f", fps_);

    ImGui::End();
}

void NeuralFieldContext::Resize(int w, int h) {
    windowWidth_ = w;
    windowHeight_ = h;

    int newW = windowWidth_ - g_UiWidth;
    double newScale = 2.0 / (double)(newW);
    double newLeft = g_area.xmin - g_UiWidth * newScale;
    mvp_ = glm::ortho(newLeft, g_area.xmax, g_area.ymin, g_area.ymax);

    quad_.Resize(newW, h);
    renderer_.resize(newW, h);
    contourLines_.resize(newW, h);
    contourFill_.resize(newW, h);
    contourParallel_.resize(newW, h);
    contourParallelFill_.resize(newW, h);
}

void NeuralFieldContext::SetActivity(int x, int y) {
    int cx = 0, cy = 0;

    int newX = x - g_UiWidth;

    int w = windowWidth_ - g_UiWidth;
    int h = windowHeight_;

    if (w > h) {
        cx = newX - (w - h) / 2;
        cy = y;
    }
    else {
        cx = newX;
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
    (void)dt;

    if (currentTime - lastFpsTime > 1.0) {
        fps_ = ImGui::GetIO().Framerate;
        lastFpsTime = currentTime;
    }

    model_.stimulate(/* dt */);

    switch (renderMode_) {
    case RenderMode::Texture:
        renderer_.update_texture(model_.activity.get());
        break;

    case RenderMode::Contour:
        contourLines_.update(model_.activity.get(), g_area, 0.0);
        break;

    case RenderMode::ContourParallel:
        contourParallel_.update(model_.activity.get(), g_area, 0.0);
        break;

    case RenderMode::Fill:
        contourLines_.update(model_.activity.get(), g_area, 0.0);
        contourFill_.update(model_.activity.get(), g_area, 0.0);
        break;

    case RenderMode::FillParallel:
        contourParallel_.update(model_.activity.get(), g_area, 0.0);
        contourParallelFill_.update(model_.activity.get(), g_area, 0.0);
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
    }
    else {
        LOGI << "Turned Blur Off";
    }
}

void NeuralFieldContext::IncreaseBlur() {
    renderer_.add_blur(g_textureBlurDelta);
}

void NeuralFieldContext::DecreaseBlur() {
    renderer_.add_blur(-g_textureBlurDelta);
}
