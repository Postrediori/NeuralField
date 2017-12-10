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

static const GLfloat Background[] = {0.00f, 0.00f, 0.00f, 1.00f};
static const GLfloat Foreground[] = {0.50f, 0.50f, 1.00f, 1.00f};
static const GLfloat Outline[] = {1.00f, 1.00f, 1.00f, 1.00f};
static const GLfloat TextColor[] = {1.00f, 1.00f, 1.00f, 1.00f};

static const int Width = 512;
static const int Height = 512;
static const int TimerInterval = 10;
static const float TextureBlurDelta = 0.1f;

static const char FontFile[] = "data/font.ttf";
static const char ConfigFile[] = "data/amari.conf";
static const char VertexShader[] = "data/plane.vert";
static const char FragmentShader[]  = "data/plane.frag";

static const float XMin = -1.f;
static const float XMax =  1.f;
static const float YMin = -1.f;
static const float YMax =  1.f;


/*****************************************************************************
 * Main variables
 ****************************************************************************/
static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    
bool gKeys[255];
bool gFullscreen = false;
bool gGamemode = true;
bool gShowHelp;
int gWindowWidth, gWindowHeight, gSize;
float gScaleX, gScaleY;

FPSCounter gFPSCounter;

glm::mat4 gModel, gView, gProjection;

FontRenderer fr;
FontAtlas* a24;

AmariModel* gAmariModel  = NULL;
AmariRender* gAmariRender = NULL;

ShaderFiles gContourProgram;
ContourPlot* gContourLines;
ContourPlot* gContourFill;
ContourPlot* gContourParallel;
//ContourPlot* gContourParallelFill;

enum RenderMode {
    RENDER_TEXTURE,
    RENDER_CONTOUR,
    RENDER_PARALLEL,

    RENDER_MAXIMAL
} gRenderMode = RENDER_CONTOUR;


static const char* const RenderModeLabels[] = {
    "Texture", "Contour", "Contour (Parallel)"
};

/*****************************************************************************
 * Graphics functions
 ****************************************************************************/
bool Init() {
    srand(time(0));
    
    plog::init(plog::debug, &consoleAppender);
    
    LOGI << "OpenGL Renderer : " << glGetString(GL_RENDERER);
    LOGI << "OpenGL Vendor : " << glGetString(GL_VENDOR);
    LOGI << "OpenGL Version : " << glGetString(GL_VERSION);
    LOGI << "GLSL Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION);
    LOGI << "GLEW Version : " << glewGetString(GLEW_VERSION);

    // Init GLEW
    glewExperimental = GL_TRUE;

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        LOGE << "GL Loading Error: " << glewGetErrorString(err);
        return false;
    }

    // Init scene
    gWindowWidth  = Width;
    gWindowHeight = Height;
    gFullscreen = false;
    gShowHelp = true;

    // Init font
    if (!fr.init()) {
        LOGE << "Unable to initialize FreeType Font Loader";
        return false;
    }
    if (!fr.load(FontFile)) {
        return false;
    }
    a24 = fr.createAtlas(24);
    if (!a24) {
        LOGE << "Unable to create typeset";
        return false;
    }

    // Init MVP matrices
    gModel = glm::mat4(1.0f);
    gView = glm::mat4(1.0f);
    gProjection = glm::ortho(XMin, XMax, YMin, YMax);

    // Init model
    gAmariModel = new AmariModel();
    if (!gAmariModel->init(ConfigFile)) {
        LOGE << "Unable to load Amari Model Config from file " << ConfigFile;
        return false;
    }

    // Init render
    gAmariRender = new AmariRender();
    if (!gAmariRender->init(gAmariModel->size)) {
        LOGE << "Unable to init Amari Model Renderer";
        return false;
    }
    gAmariRender->resize(Width, Height);
    gAmariRender->update_texture(gAmariModel->activity, gAmariModel->data_size);

    // Init contour lines
    if (!gContourProgram.load(VertexShader, FragmentShader)) {
        LOGE << "Unable to create shader for contour lines";
        return false;
    }

    gContourLines = new ContourLine(gContourProgram.program());
    if (!gContourLines->init(gAmariModel->activity,
        gAmariModel->size-1, gAmariModel->size-1,
        XMin, XMax, YMin, YMax, 1.f)) {
        LOGE << "Unable to create Contour Lines";
        return false;
    }

    gContourFill = new ContourFill(gContourProgram.program());
    if (!gContourFill->init(gAmariModel->activity,
        gAmariModel->size-1, gAmariModel->size-1,
        XMin, XMax, YMin, YMax, 1.f)) {
        LOGE << "Unable to create Filled Contour";
        return false;
    }

    gContourParallel = new ContourParallel(gContourProgram.program());
    if (!gContourParallel->init(gAmariModel->activity,
        gAmariModel->size-1, gAmariModel->size-1,
        XMin, XMax, YMin, YMax, 1.f)) {
        LOGE << "Unable to create Parallel Contour";
        return false;
    }

    //gContourParallelFill = new ContourParallelFill(gContourProgram.program());
    //if (gContourParallelFill->init(gAmariModel->activity,
    //    gAmariModel->size-1, gAmariModel->size-1,
    //    XMin, XMax, YMin, YMax, 1.f) != 0) {
    //    return -1;
    //}

    // Set up OpenGL
    glEnable(GL_TEXTURE_2D); LOGOPENGLERROR();
    glShadeModel(GL_SMOOTH); LOGOPENGLERROR();

    glClearColor(Background[0], Background[1], Background[2], Background[3]); LOGOPENGLERROR();
    glClearDepth(1.); LOGOPENGLERROR();

    glEnable(GL_MULTISAMPLE_ARB); LOGOPENGLERROR();

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); LOGOPENGLERROR();

    return true;
}

void Deinit() {
    if (gAmariRender) {
        gAmariRender->release();
        delete gAmariRender;
        gAmariRender = NULL;
    }

    if (gAmariModel) {
        gAmariModel->release();
        delete gAmariModel;
        gAmariModel = NULL;
    }

    if (gContourLines) {
        delete gContourLines;
        gContourLines = NULL;
    }

    if (gContourFill) {
        delete gContourFill;
        gContourFill = NULL;
    }

    if (gContourParallel) {
        delete gContourParallel;
        gContourParallel = NULL;
    }

    //if (gContourParallelFill) {
    //    delete gContourParallelFill;
    //    gContourParallelFill = NULL;
    //}

    // gContourProgram.release();
}

/*****************************************************************************
 * GLUT Callback functions
 ****************************************************************************/
void Display() {
    glClear(GL_COLOR_BUFFER_BIT); LOGOPENGLERROR();

    glm::mat4 mvp = gProjection * gView * gModel;

    if (gRenderMode == RENDER_TEXTURE) {
        gAmariRender->render(mvp);

    } else if (gRenderMode == RENDER_CONTOUR) {
        static const float zoom = 1.f;
        static const glm::vec2 offset = glm::vec2(0.f, 0.f);

        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        gContourFill->render(mvp, zoom, offset, Foreground);
        
        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        gContourLines->render(mvp, zoom, offset, Outline);

    } else if (gRenderMode == RENDER_PARALLEL) {
        static const float zoom = 1.f;
        static const glm::vec2 offset = glm::vec2(0.f, 0.f);
        
        //glPolygonOffset(1, 0);
        //glEnable(GL_POLYGON_OFFSET_FILL);
        //gContourParallelFill->render(mvp, zoom, offset, Foreground);
        //
        //glPolygonOffset(0, 0);
        //glDisable(GL_POLYGON_OFFSET_FILL);
        gContourParallel->render(mvp, zoom, offset, Outline);
    }

    fr.renderStart();
    fr.renderColor(TextColor);
    if (gShowHelp) {
        fr.renderText(a24, -1+8*gScaleX, -1+200*gScaleY, gScaleX, gScaleY, "1-3 ... Choose plot mode");
        fr.renderText(a24, -1+8*gScaleX, -1+175*gScaleY, gScaleX, gScaleY, "B ... Turn texture blur on/off");
        fr.renderText(a24, -1+8*gScaleX, -1+150*gScaleY, gScaleX, gScaleY, "LMB ... Activate model");
        fr.renderText(a24, -1+8*gScaleX, -1+125*gScaleY, gScaleX, gScaleY, "RMB ... Clear model");
        fr.renderText(a24, -1+8*gScaleX, -1+100*gScaleY, gScaleX, gScaleY, "F2 ... Show/hide help");
        fr.renderText(a24, -1+8*gScaleX, -1+75*gScaleY,  gScaleX, gScaleY, "F1 ... Fullscreen on/off");
        fr.renderText(a24, -1+8*gScaleX, -1+50*gScaleY,  gScaleX, gScaleY, "Display Mode : %s", RenderModeLabels[gRenderMode]);
    }
    fr.renderText(a24, -1+8*gScaleX, -1+25*gScaleY, gScaleX, gScaleY, "FPS : %.1f", gFPSCounter.fps);
    fr.renderEnd();
    
    glutSwapBuffers();
    gFPSCounter.update(glutGet(GLUT_ELAPSED_TIME));
}

void Reshape(GLint w, GLint h) {
    glViewport(0, 0, w, h); LOGOPENGLERROR();

    gWindowWidth = w;
    gWindowHeight = h;

    gScaleX = 2.f / (float)gWindowWidth;
    gScaleY = 2.f / (float)gWindowHeight;

    if (w > h) {
        gSize = h;
    } else {
        gSize = w;
    }

    gAmariRender->resize(w, h);
    gContourLines->resize(w, h);
    gContourFill->resize(w, h);
    gContourParallel->resize(w, h);
    //gContourParallelFill->resize(w, h);
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;

    case 'b':
    case 'B':
        gAmariRender->use_blur = !gAmariRender->use_blur;
        if (gAmariRender->use_blur) {
            LOGI << "Turned Blur On";
        } else {
            LOGI << "Turned Blur Off";
        }
        break;

    case '+':
        gAmariRender->blur_sigma += TextureBlurDelta;
        if (gAmariRender->blur_sigma>0.f) {
            gAmariRender->use_blur = true;
        }
        LOGI << "Blur Sigma " << gAmariRender->blur_sigma;
        break;

    case '-':
        gAmariRender->blur_sigma -= TextureBlurDelta;
        if (gAmariRender->blur_sigma<0.f) {
            gAmariRender->blur_sigma = 0.f;
            gAmariRender->use_blur = false;
            LOGI << "Turned Blur Off";
        } else {
            LOGI << "Blur Sigma " << gAmariRender->blur_sigma;
        }
        break;

    case '1':
        gRenderMode = RENDER_TEXTURE;
        break;

    case '2':
        gRenderMode = RENDER_CONTOUR;
        break;

    case '3':
        gRenderMode = RENDER_PARALLEL;
        break;

    case ' ':
        gAmariModel->restart();
        LOGI << "Cleared Model";
        break;
    }
}

void SpecialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_F1:
        gFullscreen = !gFullscreen;
        if (gFullscreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(Width, Height);
        }
        break;

    case GLUT_KEY_F2:
        gShowHelp = !gShowHelp;
        break;

    default:
        gKeys[key] = true;
    }
}

void SpecialKeysUp(int key, int x, int y) {
    gKeys[key] = false;
}

void MouseButton(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) {
        return;
    }
    if (button == GLUT_LEFT_BUTTON) {
        int cx, cy;

        if (gWindowWidth > gWindowHeight) {
            cx = x - (gWindowWidth - gWindowHeight) / 2;
            cy = y;
        } else {
            cx = x;
            cy = y - (gWindowHeight - gWindowWidth) / 2;
        }

        if (cx<0 || cy<0 || cx>gSize || cy>gSize) {
            return;
        }

        int n, m;
        n = (int)((float)cx/(float)gSize * gAmariModel->size);
        m = (int)((1.f - (float)cy/(float)gSize) * gAmariModel->size);

        gAmariModel->set_activity(n, m, 1.f);

        LOGI << "Set Activity at [" << n << "," << m << "]";

    } else if (button == GLUT_RIGHT_BUTTON) {
        gAmariModel->restart();

    }
}

void Idle() {
    gAmariModel->stimulate();

    if (gRenderMode == RENDER_TEXTURE) {
        gAmariRender->update_texture(gAmariModel->activity, gAmariModel->data_size);

    } else if (gRenderMode == RENDER_CONTOUR) {
        if (gContourLines) {
            gContourLines->release();
            gContourLines->init(gAmariModel->activity,
                gAmariModel->size-1, gAmariModel->size-1,
                XMin, XMax, YMin, YMax, 0.f);
        }

        if (gContourFill) {
            gContourFill->release();
            gContourFill->init(gAmariModel->activity,
                gAmariModel->size-1, gAmariModel->size-1,
                XMin, XMax, YMin, YMax, 0.f);
        }

    } else if (gRenderMode == RENDER_PARALLEL) {
        if (gContourParallel) {
            gContourParallel->release();
            gContourParallel->init(gAmariModel->activity,
                gAmariModel->size-1, gAmariModel->size-1,
                XMin, XMax, YMin, YMax, 0.f);
        }

        //if (gContourParallelFill) {
        //    gContourParallelFill->release();
        //    gContourParallelFill->init(gAmariModel->activity,
        //        gAmariModel->size-1, gAmariModel->size-1,
        //        XMin, XMax, YMin, YMax, 0.f);
        //}
    }

    glutPostRedisplay();
}

/*****************************************************************************
 * Main program
 ****************************************************************************/
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(Width, Height);
    glutCreateWindow("glutAmari");

    atexit(Deinit);
    if (!Init()) {
        LOGE << "Init Failed";
        return -1;
    }

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys);
    glutSpecialUpFunc(SpecialKeysUp);
    glutMouseFunc(MouseButton);
    glutIdleFunc(Idle);

    glutMainLoop();

    return 0;
}

