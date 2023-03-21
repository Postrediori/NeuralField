#pragma once

enum class RenderMode : int {
    Texture,
    Contour,
    Fill
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

    void RegisterCallbacks();

private:
    GLFWwindow* window_ = nullptr;
    int windowWidth_ = 0, windowHeight_ = 0;

    RenderMode renderMode_ = RenderMode::Fill;

    GraphicsUtils::unique_program program_;

    hmm_mat4 mvp_;

    NeuralFieldModel model_;
    TextureRenderer renderer_;

    ContourLine contourLines_;
    ContourFill contourFill_;

    bool isFullscreen_ = false;
    struct {
        int XPos, YPos;
        int Width, Height;
    } savedWindowInfo_ = {0, 0, 0, 0};

    float fps_ = 0.0f;
    uint64_t averageIteration_{ 0 };

    bool textureBlur_ = true;

    NeuralFieldModelParams modelConfig_;

    QuadRenderer quad_;
};
