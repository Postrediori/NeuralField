project(ProgramsPlanes)
cmake_minimum_required(VERSION 2.8)

include(cmake/3rdparty.txt)
include(cmake/config.txt)

add_subdirectory(AmariLib)
add_subdirectory(ContourPlotLib)
add_subdirectory(GlUtilsLib)
add_subdirectory(LinAlgLib)
add_subdirectory(AmariModel)
