make_executable()

target_include_directories(${PROJECT} PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    )

target_link_libraries(${PROJECT}
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES}
    ${IMGUI_LIBRARIES}
    ${PLOG_LIBRARY}
    ${OpenMP_CXX_FLAGS}
    NeuralFieldLib
    ContourPlotLib
    GraphicsLib
    MathLib
    UtilsLib
    )

# Data files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/amari.conf
    ${CMAKE_CURRENT_BINARY_DIR}/data/amari.conf COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/plane.vert
    ${CMAKE_CURRENT_BINARY_DIR}/data/plane.vert COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/plane.frag
    ${CMAKE_CURRENT_BINARY_DIR}/data/plane.frag COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/texture.vert
    ${CMAKE_CURRENT_BINARY_DIR}/data/texture.vert COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/texture.frag
    ${CMAKE_CURRENT_BINARY_DIR}/data/texture.frag COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/texture-blur.vert
    ${CMAKE_CURRENT_BINARY_DIR}/data/texture-blur.vert COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/texture-blur.frag
    ${CMAKE_CURRENT_BINARY_DIR}/data/texture-blur.frag COPYONLY)

# Installation
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
    DESTINATION ${CMAKE_INSTALL_PREFIX})
