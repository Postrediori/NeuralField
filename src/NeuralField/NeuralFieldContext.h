#pragma once

enum class RenderMode : int {
    Texture,
    Contour,
    Fill
};

struct WindowDimensions {
    int X, Y;
    int Width, Height;
};

class NeuralFieldContext {
public:
    NeuralFieldContext() = default;
    ~NeuralFieldContext();

    bool Init(GLFWwindow* window, int argc, const char* argv[]);

    void Display();
    void Update();

    static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void ReshapeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, int button, int action, int mods);

private:
#ifdef USE_OPENCL
    bool InitOpenCLContext();
    void ReleaseOpenCLContext();

    std::string GetOpenCLStatus() const;
#endif

    void Restart();
    void SetActivity(int x, int y);

    void Resize(int w, int h);

    void SetRenderMode(RenderMode mode);

    void SwitchBlur();
    void IncreaseBlur();
    void DecreaseBlur();

    void RenderUi();
    void Release();

    void Keyboard(int key, int scancode, int action, int mods);
    void Mouse(int button, int action, int mods);

    int ParseArgs(int argc, const char* argv[]);
    int ShowUsage(const std::string& cmd);

    void RegisterCallbacks();

private:
    GLFWwindow* window_ = nullptr;
    int windowWidth_ = 0, windowHeight_ = 0;


    RenderMode renderMode_ = RenderMode::Fill;

    GraphicsUtils::unique_program program_;

    HMM_Mat4 mvp_;

    NeuralFieldModel model_;
    int modelSize_;
    int modelMode_;
    float modelH_;
    float modelM_;

    TextureRenderer renderer_;

    ContourLine contourLines_;
    ContourFill contourFill_;

    bool isFullscreen_ = false;
    WindowDimensions savedWindowInfo_ = { 0, 0, 0, 0 };

    float fps_ = 0.0f;
    double lastFpsTime_ = 0.0;
    uint64_t averageIteration_{ 0 };
    uint64_t iterationsTime_ = 0;
    int simulationsCounter_ = 0;

    bool textureBlur_ = true;

    NeuralFieldModelParams modelConfig_;

    QuadRenderer quad_;

#ifdef USE_OPENCL
    // OpenCL context
    bool isEnabledOpenCL = false; // If load of OpenCL was successfull
    size_t openClPlatformNum = 1;
    size_t openClDeviceNum = 1;
    std::string openClStatusStr;

    cl_platform_id platformId = nullptr;
    cl_device_id device = nullptr;
    cl_context context = nullptr;

    cl_command_queue commandQueue = nullptr;
#endif
};
