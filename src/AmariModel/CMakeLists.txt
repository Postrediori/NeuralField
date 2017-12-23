make_executable()

include_directories(
    ${PLOG_INCLUDE_DIR}
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
    GraphicsLib
    MathLib
    )

if(MSVC)
    add_definitions(
        -DFREEGLUT_LIB_PRAGMAS=0
        -DFREEGLUT_STATIC
        -DGLEW_STATIC
        )
endif()
    
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
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
    DESTINATION ${CMAKE_INSTALL_PREFIX})

