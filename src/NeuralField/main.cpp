#include "stdafx.h"
#include "Matrix.h"
#include "Gauss.h"
#include "GraphicsUtils.h"
#include "GraphicsLogger.h"
#include "GraphicsResource.h"
#include "NeuralFieldModel.h"
#include "PlainTextureRenderer.h"
#include "TextureRenderer.h"
#include "ContourPlot.h"
#include "ContourLine.h"
#include "ContourFill.h"
#include "QuadRenderer.h"
#include "NeuralFieldContext.h"
#include "LogFormatter.h"
#include "GlfwWrapper.h"
#include "ImGuiWrapper.h"


constexpr int Width = 800;
constexpr int Height = 600;

const std::string Title = "Model of Planar Neural Field";


int main(int argc, const char* argv[]) {
    try {
        plog::ConsoleAppender<plog::LogFormatter> logger;
#ifdef NDEBUG
        plog::init(plog::info, &logger);
#else
        plog::init(plog::debug, &logger);
#endif

        GlfwWrapper glfwWrapper;
        if (glfwWrapper.Init(Title, Width, Height) != 0) {
            LOGE << "Failed to load GLFW";
            return EXIT_FAILURE;
        }
        
        glfwSwapInterval(0); // Disable vsync to get maximum number of iterations

        NeuralFieldContext context;
        if (!context.Init(glfwWrapper.GetWindow(), argc, argv)) {
            LOGE << "Initialization failed";
            return EXIT_FAILURE;
        }
        
        // Setup ImGui
        ImGuiWrapper imguiWrapper;
        imguiWrapper.Init(glfwWrapper.GetWindow());

        // Setup of ImGui visual style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.WindowBorderSize = 0.0f;

        // Main loop
        while (!glfwWindowShouldClose(glfwWrapper.GetWindow())) {
            glfwPollEvents();

            // Start ImGui frame
            imguiWrapper.StartFrame();

            context.Display();

            // Render ImGui
            imguiWrapper.Render();

            context.Update();

            glfwSwapBuffers(glfwWrapper.GetWindow());
        }
    }
    catch (const std::exception& ex) {
        LOGE << ex.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
