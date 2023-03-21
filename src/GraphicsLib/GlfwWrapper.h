#pragma once

struct GlfwWrapper {
    GlfwWrapper() = default;
    ~GlfwWrapper();
    
    int Init(const std::string& title, int width, int height);
    void Release();
    
    GLFWwindow* GetWindow() const;
    
    static void ErrorCallback(int error, const char* description);
    
    GLFWwindow* window_ = nullptr;
};
