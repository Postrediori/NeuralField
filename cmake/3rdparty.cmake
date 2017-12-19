set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/3rdparty")
find_package(OpenGL REQUIRED)
find_package(FreeGLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenMP REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)

file(GLOB_RECURSE PLOG_HEADERS ${CMAKE_SOURCE_DIR}/3rdparty/*.h)
add_library(plog STATIC ${PLOG_HEADERS})
set_target_properties(plog PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(plog PROPERTIES FOLDER Include)
