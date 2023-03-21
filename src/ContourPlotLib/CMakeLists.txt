make_library()

target_precompile_headers(${PROJECT} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)

target_include_directories(${PROJECT} PRIVATE
    ${GLAD_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    )

target_link_libraries(${PROJECT}
    ${GLAD_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${HMM_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${PLOG_LIBRARY}
    GraphicsLib
    MathLib
    )

if (USE_OPENMP)
    target_link_libraries(${PROJECT} ${OpenMP_CXX_FLAGS})
endif()
