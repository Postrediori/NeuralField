#include "stdafx.h"
#include "Matrix.h"
#include "Texture.h"
#include "MathUtils.h"
#include "Gauss.h"
#include "GlUtils.h"
#include "NeuralFieldModel.h"
#include "FrameBufferWrapper.h"
#include "PlainTextureRenderer.h"
#include "TextureRenderer.h"
#include "ContourPlot.h"
#include "ContourLine.h"
#include "ContourFill.h"
#include "ContourParallel.h"
#include "ContourParallelFill.h"
#include "QuadRenderer.h"
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

void Display(NeuralFieldContext& context) {
    glClear(GL_COLOR_BUFFER_BIT); LOGOPENGLERROR();

    context.Render();
}

void Update(NeuralFieldContext& context) {
    context.Update(glfwGetTime());
}


/*****************************************************************************
 * GLFW Callback functions
 ****************************************************************************/

void Error(int /*error*/, const char* description) {
    LOGE << "Error: " << description;
}

void Reshape(GLFWwindow* window, int width, int height) {
    void* p = glfwGetWindowUserPointer(window);
    assert(p);
    NeuralFieldContext* context = static_cast<NeuralFieldContext *>(p);

    glViewport(0, 0, width, height); LOGOPENGLERROR();

    context->Resize(width, height);
}

void Keyboard(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    static bool gFullscreen = false;
    static int gSavedXPos = 0, gSavedYPos = 0;
    static int gSavedWidth = 0, gSavedHeight = 0;

    void* p = glfwGetWindowUserPointer(window);
    assert(p);
    NeuralFieldContext* context = static_cast<NeuralFieldContext *>(p);

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
            context->SetRenderMode(RenderMode::Texture);
            break;

        case GLFW_KEY_2:
            context->SetRenderMode(RenderMode::Contour);
            break;

        case GLFW_KEY_3:
            context->SetRenderMode(RenderMode::ContourParallel);
            break;

        case GLFW_KEY_4:
            context->SetRenderMode(RenderMode::Fill);
            break;

        case GLFW_KEY_5:
            context->SetRenderMode(RenderMode::FillParallel);
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
    NeuralFieldContext* context = static_cast<NeuralFieldContext *>(p);

    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_1) {
            double x = 0.0, y = 0.0;
            glfwGetCursorPos(window, &x, &y);
            context->SetActivity(x, y);
        }
        else if (button == GLFW_MOUSE_BUTTON_2) {
            context->Restart();
        }
    }
}


/*****************************************************************************
 * GUI Functions
 ****************************************************************************/

void GuiInit(GLFWwindow* window) {
    assert(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // Disable .ini

    static const std::string glslVersion = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion.c_str());
}

void GuiTerminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void GuiStartFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
    ScopeGuard glfwGuard([]() { glfwTerminate(); });

    LOGI << "Init window context with OpenGL 3.3";
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window(
        glfwCreateWindow(Width, Height, Title.c_str(), nullptr, nullptr),
        &glfwDestroyWindow);
    if (!window) {
        LOGE << "Unable to Create OpenGL 3.3 Context";
        return EXIT_FAILURE;
    };

    glfwSetKeyCallback(window.get(), Keyboard);
    glfwSetInputMode(window.get(), GLFW_STICKY_KEYS, GLFW_TRUE);

    glfwSetMouseButtonCallback(window.get(), Mouse);
    glfwSetWindowSizeCallback(window.get(), Reshape);

    glfwMakeContextCurrent(window.get());
    gladLoadGL();

    // Setup ImGui
    GuiInit(window.get());
    ScopeGuard imGuiContextGuard([]() { GuiTerminate(); });

    NeuralFieldContext gContext;
    if (!Init(gContext)) {
        LOGE << "Initialization failed";
        return EXIT_FAILURE;
    }
    glfwSetWindowUserPointer(window.get(), static_cast<void *>(&gContext));

    while (!glfwWindowShouldClose(window.get())) {
        glfwPollEvents();

        // Start ImGui frame
        GuiStartFrame();

        Display(gContext);

        // Render ImGui
        GuiRender();

        Update(gContext);

        glfwSwapBuffers(window.get());
    }

    return EXIT_SUCCESS;
}
