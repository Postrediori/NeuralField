make_executable()
include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${FREEGLUT_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    )

target_link_libraries(${PROJECT}
    ${OPENGL_LIBRARIES}
    ${FREEGLUT_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    AmariLib
    ContourPlotLib
    GlUtilsLib
    LinAlgLib
    plog
    )

# Data files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/amari.conf
    ${CMAKE_CURRENT_BINARY_DIR}/data/amari.conf COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/font.ttf
    ${CMAKE_CURRENT_BINARY_DIR}/data/font.ttf COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/plane.vert
    ${CMAKE_CURRENT_BINARY_DIR}/data/plane.vert COPYONLY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/plane.frag
    ${CMAKE_CURRENT_BINARY_DIR}/data/plane.frag COPYONLY)

# Installation
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../bundle/${PROJECT}")
install(
    TARGETS ${PROJECT}
    DESTINATION ${CMAKE_INSTALL_PREFIX})
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
    DESTINATION ${CMAKE_INSTALL_PREFIX})

