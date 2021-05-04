project(NeuralField)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

include(cmake/3rdparty.cmake)
include(cmake/config.cmake)

add_subdirectory(src)
