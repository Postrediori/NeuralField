#pragma once

#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>

#include <array>
#include <chrono>
#include <cmath>
#include <ctime>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <HandmadeMath.h>
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#include <INIReader.h>
#pragma GCC diagnostic pop

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
