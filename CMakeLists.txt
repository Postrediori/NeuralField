project(NeuralField)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 17)

include(cmake/3rdparty.cmake)
include(cmake/config.cmake)

add_subdirectory(src)
