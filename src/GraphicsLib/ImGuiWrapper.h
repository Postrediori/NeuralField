#pragma once

struct ImGuiWrapper {
    ImGuiWrapper() = default;
    ~ImGuiWrapper();
    
    void Init(GLFWwindow* window);
    void Release();
    
    void StartFrame();
    void Render();
};
