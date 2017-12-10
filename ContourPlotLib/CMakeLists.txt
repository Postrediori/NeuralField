include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${FREEGLUT_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    )
make_library()
target_link_libraries(${PROJECT}
    ${OPENGL_LIBRARIES}
    ${FREEGLUT_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    GlUtilsLib
    LinAlgLib
    plog
    )

