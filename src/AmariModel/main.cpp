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
#include "GlFormatter.h"

const int Width = 512;
const int Height = 512;

const char Title[] = "Amari Model of Neural Field";

/*****************************************************************************
 * Main variables
 ****************************************************************************/
static plog::ConsoleAppender<plog::GlFormatter> consoleAppender;

bool gFullscreen = false;

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
    
    // Init scene
    gFullscreen = false;

    if (!gContext.Init()) {
        LOGE << "Unable to initialize context";
        return false;
    }

    gContext.Resize(Width, Height);

    return true;
}

void Deinit() {
    gContext.Release();
}

void Error(int /*error*/, const char* description) {
    LOGE << "Error: " << description;
}

/*****************************************************************************
 * GLUT Callback functions
 ****************************************************************************/
void Display() {
    glClear(GL_COLOR_BUFFER_BIT); LOGOPENGLERROR();

    gContext.Render();
}

void Reshape(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height); LOGOPENGLERROR();

    gContext.Resize(width, height);
}

void Keyboard(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        case GLFW_KEY_F1:
            gFullscreen = !gFullscreen;
            if (gFullscreen) {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0,
                    mode->width, mode->height, mode->refreshRate);
            }
            else {
                glfwSetWindowMonitor(window, nullptr, 0, 0,
                    Width, Height, GLFW_DONT_CARE);
            }
            break;

        case GLFW_KEY_F2:
            gContext.showHelp_ = !gContext.showHelp_;
            break;

        case GLFW_KEY_B:
            gContext.SwitchBlur();
            break;

        case GLFW_KEY_EQUAL:
            gContext.IncreaseBlur();
            break;

        case GLFW_KEY_MINUS:
            gContext.DecreaseBlur();
            break;

        case GLFW_KEY_1:
            gContext.SetRenderMode(RENDER_TEXTURE);
            break;

        case GLFW_KEY_2:
            gContext.SetRenderMode(RENDER_CONTOUR);
            break;

        case GLFW_KEY_3:
            gContext.SetRenderMode(RENDER_PARALLEL);
            break;

        case GLFW_KEY_SPACE:
            gContext.Restart();
            break;
        }
    }
}


void Mouse(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_1) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            gContext.SetActivity(x, y);
        }
        else if (button == GLFW_MOUSE_BUTTON_2) {
            gContext.Restart();
        }
    }
}

void Update(GLFWwindow* window) {
    gContext.Update();
}


/*****************************************************************************
 * Main program
 ****************************************************************************/
int main(int /*argc*/, char** /*argv*/) {
    int status = EXIT_SUCCESS;
    plog::init(plog::debug, &consoleAppender);
    
    glfwSetErrorCallback(Error);

    if (!glfwInit()) {
        LOGE << "Failed to load GLFW";
        return EXIT_FAILURE;
    }

    LOGI << "Init window context with OpenGL 2.0";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    auto window = glfwCreateWindow(Width, Height, Title, nullptr, nullptr);
    if (!window) {
        LOGE << "Unable to Create OpenGL 2.0 Context";
        status = EXIT_FAILURE;
        goto finish;
    }

    glfwSetKeyCallback(window, Keyboard);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    glfwSetMouseButtonCallback(window, Mouse);
    glfwSetWindowSizeCallback(window, Reshape);

    glfwMakeContextCurrent(window);
    gladLoadGL();

    if (!Init()) {
        LOGE << "Initialization failed";
        status = EXIT_FAILURE;
        goto finish;
    }

    while (!glfwWindowShouldClose(window)) {
        Display();

        Update(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

finish:
    Deinit();
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();

    return status;
}
