#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"
#include "GraphicsUtils.h"
#include "GraphicsLogger.h"
#include "GraphicsResource.h"
#include "Shader.h"
#include "NeuralFieldModel.h"
#ifdef USE_OPENCL
#include "ParallelUtils.h"
#endif
#include "PlainTextureRenderer.h"
#include "TextureRenderer.h"
#include "ContourPlot.h"
#include "ContourLine.h"
#include "ContourFill.h"
#include "QuadRenderer.h"
#include "ResourceFinder.h"
#include "NeuralFieldContext.h"

const FloatColor g_background = {0.15f, 0.15f, 0.15f, 1.00f};
const FloatColor g_foreground = {0.50f, 0.50f, 1.00f, 1.00f};
const FloatColor g_outline = {1.00f, 1.00f, 1.00f, 1.00f};

const std::filesystem::path g_configFile = "amari.conf";
const std::filesystem::path g_vertexShader = "plane.vert";
const std::filesystem::path g_fragmentShader = "plane.frag";

const hmm_vec4 g_area = {-1.f, 1.f, -1.f, 1.f};

const float g_textureBlurDelta = 0.1f;

const float g_UiWidth = 250.0f;

NeuralFieldContext::~NeuralFieldContext() {
    Release();
}

bool NeuralFieldContext::Init(GLFWwindow* window, int argc, const char* argv[]) {
    srand(time(0));

    std::filesystem::path moduleDataDir;
    if (!Utils::ResourceFinder::GetDataDirectory(argv[0], moduleDataDir)) {
        LOGE << "Unable to find data directory";
        return false;
    }

    LOGI << "OpenGL Renderer : " << glGetString(GL_RENDERER);
    LOGI << "OpenGL Vendor : " << glGetString(GL_VENDOR);
    LOGI << "OpenGL Version : " << glGetString(GL_VERSION);
    LOGI << "GLSL Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION);
#ifdef USE_OPENMP
    LOGI << "OpenMP Support : " << "Yes";
#else
    LOGI << "OpenMP Support : " << "No";
#endif
#ifdef USE_OPENCL
    LOGI << "OpenCL Support : " << "Yes";
    LOGI << "OpenCL Version : " << CL_TARGET_OPENCL_VERSION;
#else
    LOGI << "OpenCL Support : " << "No";
#endif

    window_ = window;
    glfwSetWindowUserPointer(window_, static_cast<void *>(this));

    // Init MVP matrices
    mvp_ = HMM_Orthographic(g_area.X, g_area.Y, g_area.Z, g_area.W, 1.f, -1.f);

    constexpr float DefaultH = -0.1;
    constexpr float DefaultK = 0.05;
    constexpr float DefaultKp = 0.125;
    constexpr float DefaultM = 0.025;
    constexpr float DefaultMp = 0.0625;
    constexpr int DefaultSize = 256;

    // Init model
    auto configFilePath = (moduleDataDir / g_configFile).string();
    INIReader reader(configFilePath.c_str());
    if (reader.ParseError() == 0) {
        modelConfig_["h"] = reader.GetFloat("", "h", DefaultH);
        modelConfig_["k"] = reader.GetFloat("", "k", DefaultK);
        modelConfig_["Kp"] = reader.GetFloat("", "Kp", DefaultKp);
        modelConfig_["m"] = reader.GetFloat("", "m", DefaultM);
        modelConfig_["Mp"] = reader.GetFloat("", "Mp", DefaultMp);
        modelConfig_["size"] = reader.GetInteger("", "size", DefaultSize);
    }
    else {
        LOGE << "Unable to load Model Config from file " << configFilePath;
        LOGI << "Model will use default params instead";
        modelConfig_["h"] = DefaultH;
        modelConfig_["k"] = DefaultK;
        modelConfig_["Kp"] = DefaultKp;
        modelConfig_["m"] = DefaultM;
        modelConfig_["Mp"] = DefaultMp;
        modelConfig_["size"] = DefaultSize;
    }

    // Parse arguments
    if (ParseArgs(argc, argv) != EXIT_SUCCESS) {
        // Invalid arguments
        return false;
    }

#ifdef USE_OPENCL
    // Init OpenCL
    isEnabledOpenCL = InitOpenCLContext();

    isEnabledOpenCL = isEnabledOpenCL && model_.InitOpenCLContext(platformId, device, context, commandQueue);
#endif

    if (!model_.Init(modelConfig_)) {
        LOGE << "Unable to init neural field model";
        return false;
    }

#ifdef USE_OPENCL
    isEnabledOpenCL = isEnabledOpenCL && model_.IsEnabledOpenCL();
    if (isEnabledOpenCL) {
        LOGI << "Successfully loaded OpenCL. Will use OpenCL mode by default";
    }
    else {
        LOGI << "Unable to init OpenCL. Will use only CPU mode";
    }

    renderer_.SetEnabledOpenCL(isEnabledOpenCL);
#endif

    // Init render
    if (!renderer_.Init(&model_, moduleDataDir)) {
        LOGE << "Unable to init Amari Model Renderer";
        return false;
    }

#ifdef USE_OPENCL
    isEnabledOpenCL = isEnabledOpenCL && renderer_.GetEnabledOpenCL();
#endif

    renderer_.UpdateTexture();

    // Init contour lines
    auto vertexShaderFile = (moduleDataDir / g_vertexShader).string();
    auto fragmentShaderFile = (moduleDataDir / g_fragmentShader).string();
    program_.reset(Shader::CreateProgramFromFiles(vertexShaderFile, fragmentShaderFile));
    if (!program_) {
        LOGE << "Unable to create shader for contour lines";
        return false;
    }

    quad_.Init(program_.get(), { g_area.X, g_area.Y, g_area.Z, g_area.W }, g_background);

    if (!contourLines_.Init(program_.get())) {
        LOGE << "Unable to create Contour Lines";
        return false;
    }

    if (!contourFill_.Init(program_.get())) {
        LOGE << "Unable to create Filled Contour";
        return false;
    }

    contourLines_.Update(model_.activity.get(), g_area, 1.0);
    contourFill_.Update(model_.activity.get(), g_area, 1.0);

    // Initial resize
    glfwGetWindowSize(window_, &windowWidth_, &windowHeight_);
    this->Resize(windowWidth_, windowHeight_);

    RegisterCallbacks();

    // Set up OpenGL
    glClearColor(0.f, 0.f, 0.f, 1.0); LOGOPENGLERROR();
    glClearDepth(1.); LOGOPENGLERROR();

    return true;
}

void NeuralFieldContext::Release() {
#ifdef USE_OPENCL
    ReleaseOpenCLContext();
#endif
}

void NeuralFieldContext::Display() {
    glClear(GL_COLOR_BUFFER_BIT); LOGOPENGLERROR();

    constexpr float zoom = 1.f;
    static const hmm_vec2 offset = {0.f, 0.f};

    switch (renderMode_) {
    case RenderMode::Texture:
        renderer_.Render(mvp_);
        break;

    case RenderMode::Contour:
        quad_.Render(mvp_, zoom, offset);
        contourLines_.Render(mvp_, zoom, offset, g_outline);
        break;

    case RenderMode::Fill:
        quad_.Render(mvp_, zoom, offset);

        glPolygonOffset(1, 0); LOGOPENGLERROR();
        glEnable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourFill_.Render(mvp_, zoom, offset, g_foreground);

        glPolygonOffset(0, 0); LOGOPENGLERROR();
        glDisable(GL_POLYGON_OFFSET_FILL); LOGOPENGLERROR();
        contourLines_.Render(mvp_, zoom, offset, g_outline);
        break;
    }

    this->RenderUi();
}

void NeuralFieldContext::RenderUi() {
    ImVec2 uiSize = ImVec2(g_UiWidth, windowHeight_);

    ImGui::SetNextWindowPos(ImVec2(0.0, 0.0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(uiSize, ImGuiCond_Always);

    ImGui::Begin("Neural Field", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    static const std::vector<std::tuple<std::string, RenderMode>> g_RenderModeLabels = {
        {"Texture", RenderMode::Texture},
        {"Contour", RenderMode::Contour},
        {"Fill", RenderMode::Fill},
    };

    ImGui::Text("Rendering mode:");

    bool firstItemFlag = true;
    for (const auto& info : g_RenderModeLabels) {
        std::string name;
        RenderMode mode;
        std::tie(name, mode) = info;
        ImGui::RadioButton(name.c_str(), (int *)&this->renderMode_, (int)mode);
        if (firstItemFlag) {
            ImGui::SameLine();
            if (ImGui::Checkbox("Texture Blur", (bool *)&this->textureBlur_)) {
                renderer_.SetUseBlur(this->textureBlur_);
            }
            firstItemFlag = false;
        }
    }

    ImGui::Separator();

    static const std::vector<std::tuple<std::string, int>> g_ModelSizes = {
        {"128x128", 128},
        {"256x256", 256},
        {"512x512", 512},
        {"1024x1024", 1024}
    };
    static int gModelSize = static_cast<int>(modelConfig_["size"]);
    int k = 0;
    ImGui::Text("Model size:");
    for (const auto& s : g_ModelSizes) {
        if (k>0) {
            if (k%2==1) {
                ImGui::SameLine();
            }
        }
        k++;

        if (ImGui::RadioButton(std::get<0>(s).c_str(), &gModelSize, std::get<1>(s))) {
            modelConfig_["size"] = gModelSize;
            renderer_.InitTextures(gModelSize);
            model_.Init(modelConfig_);
        }
    }

    ImGui::Separator();

    static const std::map<std::string, int> g_ModelModes = {
        {"wrap", static_cast<int>(KernelMode::MODE_WRAP)},
        {"reflect", static_cast<int>(KernelMode::MODE_REFLECT)},
        {"mirror", static_cast<int>(KernelMode::MODE_MIRROR)}
    };
    static int gModelMode = static_cast<int>(modelConfig_["mode"]);
    firstItemFlag = true;
    ImGui::Text("Border mode:");
    for (const auto& s : g_ModelModes) {
        if (firstItemFlag) {
            firstItemFlag = false;
        }
        else {
            ImGui::SameLine();
        }
        if (ImGui::RadioButton(s.first.c_str(), &gModelMode, s.second)) {
            modelConfig_["mode"] = gModelMode;
            model_.Init(modelConfig_);
        }
    }

    ImGui::Separator();

    ImGui::Text("Model params:");

    static float gModelH = -0.2;
    if (ImGui::SliderFloat("h", &gModelH, -0.3f, 0.0f)) {
        modelConfig_["h"] = gModelH;
        model_.Init(modelConfig_);
    }

    static float gModelM = 0.065;
    if (ImGui::SliderFloat("M", &gModelM, 0.05f, 0.07)) {
        modelConfig_["M_"] = gModelM;
        model_.Init(modelConfig_);
    }

#ifdef USE_OPENCL
    ImGui::Separator();

    bool bUseOpenCL = isEnabledOpenCL;
    if (ImGui::Checkbox("Use OpenCL", &bUseOpenCL)) {
        // Don't change the value, this is only for indication
        bUseOpenCL = isEnabledOpenCL;
    }
    if (ImGui::IsItemHovered()) {
        std::string str = this->GetOpenCLStatus();
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(str.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
#endif

    ImGui::Separator();

    ImGui::Text("User Guide:");
    ImGui::BulletText("F1 to on/off fullscreen mode.");
    ImGui::BulletText("RMB to Clear model.");
    ImGui::BulletText("LMB to Activate model in a point.");
    ImGui::BulletText("B to toggle texture blur on/off.");

    ImGui::Separator();

    ImGui::Text("Iterations average (ms): %ld", averageIteration_);
    ImGui::Text("FPS Counter: %.1f", fps_);

    ImGui::End();
}

void NeuralFieldContext::Resize(int w, int h) {
    windowWidth_ = w;
    windowHeight_ = h;

    glViewport(0, 0, windowWidth_, windowHeight_); LOGOPENGLERROR();

    int newW = windowWidth_ - static_cast<int>(g_UiWidth);
    float newScale = 2.0 / static_cast<float>(newW);
    float newLeft = g_area.X - g_UiWidth * newScale;
    mvp_ = HMM_Orthographic(newLeft, g_area.Y, g_area.Z, g_area.W, 1.f, -1.f);

    quad_.Resize(newW, h);
    renderer_.Resize(newW, h);
    contourLines_.Resize(newW, h);
    contourFill_.Resize(newW, h);
}

void NeuralFieldContext::SetActivity(int x, int y) {
    int cx = 0, cy = 0;

    int newX = x - static_cast<int>(g_UiWidth);

    int w = windowWidth_ - static_cast<int>(g_UiWidth);
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

    size_t n = static_cast<size_t>((static_cast<double>(cx) / size) * model_.size);
    size_t m = static_cast<size_t>((1.0 - static_cast<double>(cy) / size) * model_.size);

    model_.SetActivity(n, m, 1.f);

    LOGI << "Set Activity at [" << n << "," << m << "]";
}

void NeuralFieldContext::Restart() {
    model_.Restart();
    LOGI << "Reset Model";
}

void NeuralFieldContext::Update() {
    static double lastFpsTime = 0.0;
    double currentTime = glfwGetTime();

    static uint64_t iterationsTime = 0;
    static int simulationsCounter = 0;

    if (currentTime - lastFpsTime > 1.0) {
        fps_ = ImGui::GetIO().Framerate;
        lastFpsTime = currentTime;

        if (simulationsCounter != 0) {
            averageIteration_ = iterationsTime / simulationsCounter;

            LOGI << "Stimulation Step Avg Time (microseconds) = " << averageIteration_;

            iterationsTime = 0;
            simulationsCounter = 0;
        }
    }

    {
        auto stimulationStepStart = std::chrono::high_resolution_clock::now();

        model_.Stimulate();

        auto stimulationStepEnd = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stimulationStepEnd - stimulationStepStart);

        simulationsCounter++;
        iterationsTime += duration.count();
    }

    constexpr double ContourThreshold = 0.0;

    switch (renderMode_) {
    case RenderMode::Texture:
        renderer_.UpdateTexture();
        break;

    case RenderMode::Contour:
        contourLines_.Update(model_.activity.get(), g_area, ContourThreshold);
        break;

    case RenderMode::Fill:
        contourLines_.Update(model_.activity.get(), g_area, ContourThreshold);
        contourFill_.Update(model_.activity.get(), g_area, ContourThreshold);
        break;
    }
}

void NeuralFieldContext::SetRenderMode(RenderMode mode) {
    renderMode_ = mode;
}

void NeuralFieldContext::SwitchBlur() {
    textureBlur_ = !textureBlur_;
    renderer_.SetUseBlur(textureBlur_);
}

void NeuralFieldContext::IncreaseBlur() {
    renderer_.AddBlur(g_textureBlurDelta);
}

void NeuralFieldContext::DecreaseBlur() {
    renderer_.AddBlur(-g_textureBlurDelta);
}

void NeuralFieldContext::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto p = glfwGetWindowUserPointer(window);
    assert(p);
    auto context = static_cast<NeuralFieldContext *>(p);

    context->Keyboard(key, scancode, action, mods);
}

void NeuralFieldContext::ReshapeCallback(GLFWwindow* window, int width, int height) {
    auto p = glfwGetWindowUserPointer(window);
    assert(p);
    auto context = static_cast<NeuralFieldContext *>(p);

    context->Resize(width, height);
}

void NeuralFieldContext::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
    auto p = glfwGetWindowUserPointer(window);
    assert(p);
    auto context = static_cast<NeuralFieldContext *>(p);

    context->Mouse(button, action, mods);
}

void NeuralFieldContext::Keyboard(int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
            break;

        case GLFW_KEY_F1:
            isFullscreen_ = !isFullscreen_;
            if (isFullscreen_) {
                glfwGetWindowPos(window_, &savedWindowInfo_.XPos, &savedWindowInfo_.YPos);
                glfwGetWindowSize(window_, &savedWindowInfo_.Width, &savedWindowInfo_.Height);

                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window_, monitor, 0, 0,
                    mode->width, mode->height, mode->refreshRate);
            }
            else {
                glfwSetWindowMonitor(window_, nullptr, savedWindowInfo_.XPos, savedWindowInfo_.YPos,
                    savedWindowInfo_.Width, savedWindowInfo_.Height, GLFW_DONT_CARE);
            }
            break;

        case GLFW_KEY_B:
            textureBlur_ = !textureBlur_;
            renderer_.SetUseBlur(textureBlur_);
            break;

        case GLFW_KEY_EQUAL:
            IncreaseBlur();
            break;

        case GLFW_KEY_MINUS:
            DecreaseBlur();
            break;

        case GLFW_KEY_1:
            SetRenderMode(RenderMode::Texture);
            break;

        case GLFW_KEY_2:
            SetRenderMode(RenderMode::Contour);
            break;

        case GLFW_KEY_3:
            SetRenderMode(RenderMode::Fill);
            break;

        case GLFW_KEY_SPACE:
            Restart();
            break;
        }
    }
}

void NeuralFieldContext::Mouse(int button, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_1) {
            double x = 0.0, y = 0.0;
            glfwGetCursorPos(window_, &x, &y);
            SetActivity(x, y);
        }
        else if (button == GLFW_MOUSE_BUTTON_2) {
            Restart();
        }
    }
}

#ifdef USE_OPENCL
bool NeuralFieldContext::InitOpenCLContext() {
    // --------------------------------------------------------
    // Variables

    cl_int status{ 0 };

    // --------------------------------------------------------
    // Get all platforms
    cl_uint numPlatforms{ 0 };
    status = clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to get number of platforms : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    LOGI << "This system provides " << numPlatforms << " platforms";

    std::vector<cl_platform_id> platformIds(numPlatforms);
    status = clGetPlatformIDs(numPlatforms, platformIds.data(), nullptr);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to get platform ids : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    // --------------------------------------------------------
    // Check that selected platform exists
    if (openClPlatformNum > platformIds.size()) {
        LOGE << "Platform " << openClPlatformNum << " was not found in this system";
        return false;
    }

    platformId = platformIds[openClPlatformNum - 1];

    // --------------------------------------------------------
    // Get all devices
    cl_uint numDevices{ 0 };
    status = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to get number of OpenCL deivces : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    LOGI << "Platform " << openClPlatformNum << " provides " << numDevices << " devices";

    std::vector<cl_device_id> deviceIds(numDevices);
    status = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, numDevices, deviceIds.data(), nullptr);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to get deivce ids : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    // --------------------------------------------------------
    // Check that selected device exists
    if (openClDeviceNum > deviceIds.size()) {
        LOGE << "OpenCL device " << openClPlatformNum << " is not found in this system";
        return false;
    }

    device = deviceIds[openClDeviceNum - 1];

    // Create the context
    context = clCreateContext(0, 1, &device, NULL, NULL, &status);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to create OpenCL context : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    // Create a command-queue
    commandQueue = clCreateCommandQueue(context, device, 0, &status);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to create OpenCL command queue : " << ParallelUtils::GetOpenCLError(status);
        return false;
    }

    // Get OpenCL status
    std::stringstream s;
    s << "OpenCL Platform Info : " << std::endl << ParallelUtils::GetPlatformInfo(platformId)
        << "Device Info :" << std::endl << ParallelUtils::GetDeviceInfo(device);
    openClStatusStr = s.str();

    LOGI << openClStatusStr;

    return true;
}

void NeuralFieldContext::ReleaseOpenCLContext() {
    if (context) {
        clReleaseContext(context);
        context = 0;
    }

    if (commandQueue) {
        clReleaseCommandQueue(commandQueue);
        commandQueue = 0;
    }
}

std::string NeuralFieldContext::GetOpenCLStatus() const {
    if (!isEnabledOpenCL) {
        return "OpenCL is not available";
    }
    return openClStatusStr;
}
#endif /* USE_OPENCL */

int NeuralFieldContext::ParseArgs(int argc, const char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "-h" || arg == "--help") {
            return ShowUsage(argv[0]);
        }
#ifdef USE_OPENCL
        else if (arg == "-p" || arg == "--platform") {
            if (i + 1 < argc) {
                int k = atoi(argv[++i]);
                if (k < 1) {
                    LOGE << "OpenCL platform ID must be greater than 0";
                    return false;
                }
                openClPlatformNum = static_cast<size_t>(k);
                LOGI << "Set OpenCL platform to " << openClPlatformNum;
            }
            else {
                LOGE << "--platform option requres one argument";
                return EXIT_FAILURE;
            }
        }
        else if (arg == "-d" || arg == "--device") {
            if (i + 1 < argc) {
                int k = atoi(argv[++i]);
                if (k < 1) {
                    LOGE << "OpenCL device ID must be greater than 0";
                    return false;
                }
                openClDeviceNum = static_cast<size_t>(k);
                LOGI << "Set OpenCL device to " << openClDeviceNum;
            }
            else {
                LOGE << "--device option requres one argument";
                return EXIT_FAILURE;
            }
        }
#endif /* USE_OPENCL */
    }

    return EXIT_SUCCESS;
}

int NeuralFieldContext::ShowUsage(const std::string& cmd) {
    std::cout << "Usage: " << cmd << " <option(s)>" << std::endl
        << "Options:" << std::endl
        << "\t-h,--help\t\tShow this help message" << std::endl;
#ifdef USE_OPENCL
    std::cout << "\t-p,--platform NUM\t\tSpecify OpenCL platform by ID" << std::endl
        << "\t-d,--device NUM\t\tSpecify OpenCL device by ID" << std::endl;
#endif
    return EXIT_FAILURE;
}

void NeuralFieldContext::RegisterCallbacks() {
    glfwSetKeyCallback(window_, NeuralFieldContext::KeyboardCallback);
    glfwSetInputMode(window_, GLFW_STICKY_KEYS, GLFW_TRUE);

    glfwSetMouseButtonCallback(window_, NeuralFieldContext::MouseCallback);
    glfwSetWindowSizeCallback(window_, NeuralFieldContext::ReshapeCallback);
}
