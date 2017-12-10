#include "stdafx.h"

#include "GlUtils.h"
#include "FreeType.h"
#include "AmariModel.h"
#include "AmariRender.h"
#include "ContourPlot.h"
#include "ContourParallel.h"
//#include "ContourParallelFill.h"
#include "ContourLine.h"
#include "ContourFill.h"

#include "ModelContext.h"


static const GLfloat g_background[] = {0.00f, 0.00f, 0.00f, 1.00f};
static const GLfloat g_foreground[] = {0.50f, 0.50f, 1.00f, 1.00f};
static const GLfloat g_outline[] = {1.00f, 1.00f, 1.00f, 1.00f};
static const GLfloat g_textColor[] = {1.00f, 1.00f, 1.00f, 1.00f};

static const FontSize_t g_fontSize = 24;

static const char g_fontFile[] = "data/font.ttf";
static const char g_configFile[] = "data/amari.conf";
static const char g_vertexShader[] = "data/plane.vert";
static const char g_fragmentShader[]  = "data/plane.frag";

/* static const float XMin = -1.f;
static const float XMax =  1.f;
static const float YMin = -1.f;
static const float YMax =  1.f; */
static const area_t g_area = {-1.0, 1.0, -1.0, 1.0};

static const int g_timerInterval = 10;
static const float g_textureBlurDelta = 0.1f;


AmariModelContext::AmariModelContext()
    : renderMode_(RENDER_CONTOUR) {
}

AmariModelContext::~AmariModelContext() {
}

bool AmariModelContext::Init() {
    showHelp_ = true;
    windowWidth_ = Width;
    windowHeight_ = Height;
    
    // Init font
    if (!fr_.init()) {
        LOGE << "Unable to initialize FreeType Font Loader";
        return false;
    }
    if (!fr_.load(g_fontFile)) {
        return false;
    }
    fr_.createAtlas(g_fontSize);
    //a24_.reset(fr_.createAtlas(g_fontSize));
    //if (!a24_) {
    //    LOGE << "Unable to create typeset";
    //    return false;
    //}

    // Init MVP matrices
    model_ = glm::mat4(1.0f);
    view_ = glm::mat4(1.0f);
    projection_ = glm::ortho(g_area.xmin, g_area.xmax, g_area.ymin, g_area.ymax);

    // Init model
    if (!amariModel_.init(g_configFile)) {
        LOGE << "Unable to load Amari Model Config from file " << g_configFile;
        return false;
    }

    // Init render
    if (!amariRender_.init(amariModel_.size)) {
        LOGE << "Unable to init Amari Model Renderer";
        return false;
    }
    
    amariRender_.resize(Width, Height);
    amariRender_.update_texture(amariModel_.activity.get());

    // Init contour lines
    if (!contourProgram_.load(g_vertexShader, g_fragmentShader)) {
        LOGE << "Unable to create shader for contour lines";
        return false;
    }

    contourLines_.reset(new ContourLine(contourProgram_.program()));
    if (!contourLines_->init()) {
        LOGE << "Unable to create Contour Lines";
        return false;
    }


    contourFill_.reset(new ContourFill(contourProgram_.program()));
    if (!contourFill_->init()) {
        LOGE << "Unable to create Filled Contour";
        return false;
    }

    contourParallel_.reset(new ContourParallel(contourProgram_.program()));
    if (!contourParallel_->init()) {
        LOGE << "Unable to create Parallel Contour";
        return false;
    }

    contourLines_->update(amariModel_.activity.get(), g_area, 1.0);
    contourFill_->update(amariModel_.activity.get(), g_area, 1.0);
    contourParallel_->update(amariModel_.activity.get(), g_area, 1.0);

    //gContourParallelFill = new ContourParallelFill(gContourProgram.program());
    //if (gContourParallelFill->init(gAmariModel->activity,
    //    gAmariModel->size-1, gAmariModel->size-1,
    //    XMin, XMax, YMin, YMax, 1.f) != 0) {
    //    return -1;
    //}
    
    // Set up OpenGL
    glEnable(GL_TEXTURE_2D); LOGOPENGLERROR();
    glShadeModel(GL_SMOOTH); LOGOPENGLERROR();

    glClearColor(g_background[0], g_background[1],
                 g_background[2], g_background[3]); LOGOPENGLERROR();
    glClearDepth(1.); LOGOPENGLERROR();

    glEnable(GL_MULTISAMPLE_ARB); LOGOPENGLERROR();

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); LOGOPENGLERROR();
    
    return true;
}

void AmariModelContext::Render() {
    glm::mat4 mvp = projection_ * view_ * model_;

    if (renderMode_ == RENDER_TEXTURE) {
        amariRender_.render(mvp);

    } else if (renderMode_ == RENDER_CONTOUR) {
        static const float zoom = 1.f;
        static const glm::vec2 offset = glm::vec2(0.f, 0.f);

        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourFill_->render(mvp, zoom, offset, g_foreground);
        
        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourLines_->render(mvp, zoom, offset, g_outline);

    } else if (renderMode_ == RENDER_PARALLEL) {
        static const float zoom = 1.f;
        static const glm::vec2 offset = glm::vec2(0.f, 0.f);
        
        //glPolygonOffset(1, 0);
        //glEnable(GL_POLYGON_OFFSET_FILL);
        //gContourParallelFill->render(mvp, zoom, offset, Foreground);
        //
        //glPolygonOffset(0, 0);
        //glDisable(GL_POLYGON_OFFSET_FILL);
        contourParallel_->render(mvp, zoom, offset, g_outline);
    }

    fr_.renderStart();
    fr_.renderColor(g_textColor);
    if (showHelp_) {
        fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+200*scaleY_, scaleX_, scaleY_}, "1-3 ... Choose plot mode");
        fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+175*scaleY_, scaleX_, scaleY_}, "B ... Turn texture blur on/off");
        fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+150*scaleY_, scaleX_, scaleY_}, "LMB ... Activate model");
        fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+125*scaleY_, scaleX_, scaleY_}, "RMB ... Clear model");
        fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+100*scaleY_, scaleX_, scaleY_}, "F2 ... Show/hide help");
        fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+75*scaleY_,  scaleX_, scaleY_}, "F1 ... Fullscreen on/off");
        
        std::stringstream str;
        str << "Display Mode : " << g_renderModeLabels[renderMode_];
        fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+50*scaleY_,  scaleX_, scaleY_}, str.str());
    }
    
    std::stringstream str;
    str << "FPS : " << fpsCounter_.fps;
    fr_.renderText(g_fontSize, {-1+8*scaleX_, -1+25*scaleY_, scaleX_, scaleY_}, str.str());
    
    fr_.renderEnd();
    
    fpsCounter_.update(glutGet(GLUT_ELAPSED_TIME));
}

void AmariModelContext::Resize(int w, int h) {
    windowWidth_ = w;
    windowHeight_ = h;

    scaleX_ = 2.f / (float)windowWidth_;
    scaleY_ = 2.f / (float)windowHeight_;

    if (w > h) {
        size_ = h;
    } else {
        size_ = w;
    }

    amariRender_.resize(w, h);
    contourLines_->resize(w, h);
    contourFill_->resize(w, h);
    contourParallel_->resize(w, h);
    //contourParallelFill_->resize(w, h);
}

void AmariModelContext::SetActivity(int x, int y) {
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
    n = (int)((float)cx/(float)size_ * amariModel_.size);
    m = (int)((1.f - (float)cy/(float)size_) * amariModel_.size);

    amariModel_.set_activity(n, m, 1.f);

    LOGI << "Set Activity at [" << n << "," << m << "]";
}

void AmariModelContext::Restart() {
    amariModel_.restart();
    LOGI << "Reset Model";
}

void AmariModelContext::Update() {
    amariModel_.stimulate();

    if (renderMode_ == RENDER_TEXTURE) {
        amariRender_.update_texture(amariModel_.activity.get());

    } else if (renderMode_ == RENDER_CONTOUR) {
        if (contourLines_) {
            contourLines_->update(amariModel_.activity.get(), g_area, 0.0);
        }

        if (contourFill_) {
            contourFill_->update(amariModel_.activity.get(), g_area, 0.0);
        }

    } else if (renderMode_ == RENDER_PARALLEL) {
        if (contourParallel_) {
            contourParallel_->update(amariModel_.activity.get(), g_area, 0.0);
        }

        //if (gContourParallelFill) {
        //    gContourParallelFill->release();
        //    gContourParallelFill->init(gAmariModel->activity,
        //        gAmariModel->size-1, gAmariModel->size-1,
        //        XMin, XMax, YMin, YMax, 0.f);
        //}
    }
}

void AmariModelContext::SetRenderMode(RenderMode mode) {
    renderMode_ = mode;
}

void AmariModelContext::SwitchBlur() {
    amariRender_.use_blur = !amariRender_.use_blur;
    if (amariRender_.use_blur) {
        LOGI << "Turned Blur On";
    } else {
        LOGI << "Turned Blur Off";
    }
}

void AmariModelContext::IncreaseBlur() {
    amariRender_.blur_sigma += g_textureBlurDelta;
    if (amariRender_.blur_sigma>0.f) {
        amariRender_.use_blur = true;
    }
    LOGI << "Blur Sigma " << amariRender_.blur_sigma;
}

void AmariModelContext::DecreaseBlur() {
    amariRender_.blur_sigma -= g_textureBlurDelta;
    if (amariRender_.blur_sigma<0.f) {
        amariRender_.blur_sigma = 0.f;
        amariRender_.use_blur = false;
        LOGI << "Turned Blur Off";
    } else {
        LOGI << "Blur Sigma " << amariRender_.blur_sigma;
    }
}
