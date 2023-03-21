cmake_minimum_required(VERSION 3.16)
project(NeuralField C CXX)

set(CMAKE_CXX_STANDARD 17)

option(USE_OPENMP "Use OpenMP for parallel calculations on the CPU" ON)
option(USE_OPENCL "Use OpenCL for calculations" OFF)

include(cmake/3rdparty.cmake)
include(cmake/config.cmake)

add_subdirectory(src)
