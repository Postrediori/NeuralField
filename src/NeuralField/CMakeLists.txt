make_executable()

target_precompile_headers(${PROJECT} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)

target_include_directories(${PROJECT} PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    )

target_link_libraries(${PROJECT}
    ${GLAD_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${HMM_LIBRARY}
    ${IMGUI_LIBRARIES}
    ${INIH_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${PLOG_LIBRARY}
    NeuralFieldLib
    ContourPlotLib
    GraphicsLib
    MathLib
    UtilsLib
    )

if (USE_OPENMP)
    target_link_libraries(${PROJECT} ${OpenMP_CXX_LIB_NAMES})
endif()

# Data files
configure_file(
    ${CMAKE_SOURCE_DIR}/data/amari.conf
    ${CMAKE_CURRENT_BINARY_DIR}/data/amari.conf COPYONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/data/plane.vert
    ${CMAKE_CURRENT_BINARY_DIR}/data/plane.vert COPYONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/data/plane.frag
    ${CMAKE_CURRENT_BINARY_DIR}/data/plane.frag COPYONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/data/texture.vert
    ${CMAKE_CURRENT_BINARY_DIR}/data/texture.vert COPYONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/data/texture.frag
    ${CMAKE_CURRENT_BINARY_DIR}/data/texture.frag COPYONLY)
