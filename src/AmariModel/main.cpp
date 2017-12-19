#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "FreeType.h"
#include "AmariModel.h"
#include "AmariRender.h"
#include "ContourPlot.h"
#include "ModelContext.h"

/*****************************************************************************
 * Main variables
 ****************************************************************************/
static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;

bool gKeys[255];
bool gFullscreen = false;
bool gGamemode = true;

AmariModelContext gContext;

/*****************************************************************************
 * Graphics functions
 ****************************************************************************/
bool Init() {
    srand(time(0));
    
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
    gFullscreen = false;

    if (!gContext.Init()) {
        LOGE << "Unable to initialize context";
        return false;
    }

    return true;
}

/*****************************************************************************
 * GLUT Callback functions
 ****************************************************************************/
void Display() {
    glClear(GL_COLOR_BUFFER_BIT); LOGOPENGLERROR();

    gContext.Render();
    
    glutSwapBuffers();
}

void Reshape(GLint w, GLint h) {
    glViewport(0, 0, w, h); LOGOPENGLERROR();

    gContext.Resize(w, h);
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;

    case 'b':
    case 'B':
        gContext.SwitchBlur();
        break;

    case '+':
        gContext.IncreaseBlur();
        break;

    case '-':
        gContext.DecreaseBlur();
        break;

    case '1':
        gContext.SetRenderMode(RENDER_TEXTURE);
        break;

    case '2':
        gContext.SetRenderMode(RENDER_CONTOUR);
        break;

    case '3':
        gContext.SetRenderMode(RENDER_PARALLEL);
        break;

    case ' ':
        gContext.Restart();
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
        gContext.showHelp_ = !gContext.showHelp_;
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
        gContext.SetActivity(x, y);
    } else if (button == GLUT_RIGHT_BUTTON) {
        gContext.Restart();
    }
}

void Idle() {
    gContext.Update();
    glutPostRedisplay();
}

/*****************************************************************************
 * Main program
 ****************************************************************************/
int main(int argc, char* argv[]) {
    plog::init(plog::debug, &consoleAppender);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(Width, Height);
    glutCreateWindow("glutAmari");

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
