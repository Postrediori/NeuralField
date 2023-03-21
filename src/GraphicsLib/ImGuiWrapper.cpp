#include "stdafx.h"
#include "ImGuiWrapper.h"

ImGuiWrapper::~ImGuiWrapper() {
    Release();
}

void ImGuiWrapper::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // Disable .ini

    const std::string ImguiGlslVersion = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(ImguiGlslVersion.c_str());
}

void ImGuiWrapper::Release() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void ImGuiWrapper::StartFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWrapper::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
