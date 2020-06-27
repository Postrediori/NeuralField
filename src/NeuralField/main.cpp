#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "MathUtils.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "FreeType.h"
#include "NeuralFieldModel.h"
#include "TextureRenderer.h"
#include "ContourPlot.h"
#include "NeuralFieldContext.h"
#include "GlFormatter.h"
#include "ScopeGuard.h"

static const int Width = 800;
static const int Height = 600;

static const std::string Title = "Model of Planar Neural Field";


/*****************************************************************************
 * Graphics functions
 ****************************************************************************/
bool Init(NeuralFieldContext& context) {
    srand(time(0));

    LOGI << "OpenGL Renderer : " << glGetString(GL_RENDERER);
    LOGI << "OpenGL Vendor : " << glGetString(GL_VENDOR);
    LOGI << "OpenGL Version : " << glGetString(GL_VERSION);
    LOGI << "GLSL Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION);

    // Setup of ImGui visual style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 0.0f;

    // Init simulation
    if (!context.Init()) {
        LOGE << "Unable to initialize context";
        return false;
    }

    context.Resize(Width, Height);

    return true;
}

void Error(int /*error*/, const char* description) {
    LOGE << "Error: " << description;
}

/*****************************************************************************
 * GLUT Callback functions
 ****************************************************************************/
void Display(NeuralFieldContext& context) {
    glClear(GL_COLOR_BUFFER_BIT); LOGOPENGLERROR();

    context.Render();
}

void Reshape(GLFWwindow* window, int width, int height) {
    void* p = glfwGetWindowUserPointer(window);
    assert(p);
    NeuralFieldContext* context = (NeuralFieldContext *)p;

    glViewport(0, 0, width, height); LOGOPENGLERROR();

    context->Resize(width, height);
}

void Keyboard(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    static bool gFullscreen = false;
    static int gSavedXPos = 0, gSavedYPos = 0;
    static int gSavedWidth = 0, gSavedHeight = 0;

    void* p = glfwGetWindowUserPointer(window);
    assert(p);
    NeuralFieldContext* context = (NeuralFieldContext *)p;

    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        case GLFW_KEY_F1:
            gFullscreen = !gFullscreen;
            if (gFullscreen) {
                glfwGetWindowPos(window, &gSavedXPos, &gSavedYPos);
                glfwGetWindowSize(window, &gSavedWidth, &gSavedHeight);

                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0,
                    mode->width, mode->height, mode->refreshRate);
            }
            else {
                glfwSetWindowMonitor(window, nullptr, gSavedXPos, gSavedYPos,
                    gSavedWidth, gSavedHeight, GLFW_DONT_CARE);
            }
            break;

        case GLFW_KEY_F2:
            context->ToggleUi();
            break;

        case GLFW_KEY_B:
            context->SwitchBlur();
            break;

        case GLFW_KEY_EQUAL:
            context->IncreaseBlur();
            break;

        case GLFW_KEY_MINUS:
            context->DecreaseBlur();
            break;

        case GLFW_KEY_1:
            context->SetRenderMode(RENDER_TEXTURE);
            break;

        case GLFW_KEY_2:
            context->SetRenderMode(RENDER_CONTOUR);
            break;

        case GLFW_KEY_3:
            context->SetRenderMode(RENDER_PARALLEL);
            break;

        case GLFW_KEY_4:
            context->SetRenderMode(RENDER_FILL);
            break;

        case GLFW_KEY_5:
            context->SetRenderMode(RENDER_PARALLEL_FILL);
            break;

        case GLFW_KEY_SPACE:
            context->Restart();
            break;
        }
    }
}


void Mouse(GLFWwindow* window, int button, int action, int /*mods*/) {
    void* p = glfwGetWindowUserPointer(window);
    assert(p);
    NeuralFieldContext* context = (NeuralFieldContext *)p;

    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_1) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            context->SetActivity(x, y);
        }
        else if (button == GLFW_MOUSE_BUTTON_2) {
            context->Restart();
        }
    }
}

void Update(NeuralFieldContext& context) {
    context.Update(glfwGetTime());
}


/*****************************************************************************
 * Main program
 ****************************************************************************/
int main(int /*argc*/, char** /*argv*/) {

    static plog::ConsoleAppender<plog::GlFormatter> consoleAppender;
#ifdef NDEBUG
    plog::init(plog::info, &consoleAppender);
#else
    plog::init(plog::debug, &consoleAppender);
#endif

    glfwSetErrorCallback(Error);

    if (!glfwInit()) {
        LOGE << "Failed to load GLFW";
        return EXIT_FAILURE;
    }
    ScopeGuard glfwGuard([]() {
        glfwTerminate();
        LOGD << "Cleanup : GLFW context";
    });

    LOGI << "Init window context with OpenGL 3.3";
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    auto window = glfwCreateWindow(Width, Height, Title.c_str(), nullptr, nullptr);
    if (!window) {
        LOGE << "Unable to Create OpenGL 3.3 Context";
        return EXIT_FAILURE;
    }
    ScopeGuard windowGuard([window]() {
        glfwDestroyWindow(window);
        LOGD << "Cleanup : GLFW window";
    });

    glfwSetKeyCallback(window, Keyboard);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    glfwSetMouseButtonCallback(window, Mouse);
    glfwSetWindowSizeCallback(window, Reshape);

    glfwMakeContextCurrent(window);
    gladLoadGL();

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // Disable .ini

    static const char* gGlslVersion = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(gGlslVersion);

    ScopeGuard imGuiContextGuard([]() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            LOGD << "Cleanup : ImGui";
        });

    NeuralFieldContext gContext;
    if (!Init(gContext)) {
        LOGE << "Initialization failed";
        return EXIT_FAILURE;
    }
    glfwSetWindowUserPointer(window, (void *)(&gContext));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Display(gContext);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Update(gContext);

        glfwSwapBuffers(window);
    }

    // Cleanup is done by scope guards

    return EXIT_SUCCESS;
}
