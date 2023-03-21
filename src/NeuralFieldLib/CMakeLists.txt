make_library()

target_precompile_headers(${PROJECT} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)

target_link_libraries(${PROJECT}
    ${PLOG_LIBRARY}
    MathLib
    )

if (USE_OPENMP)
    target_link_libraries(${PROJECT} ${OpenMP_CXX_LIB_NAMES})
endif ()

if (USE_OPENCL)
    target_include_directories(${PROJECT} PRIVATE ${OpenCL_INCLUDE_DIR})
    target_link_libraries(${PROJECT} ${OpenCL_LIBRARY}
        ParallelUtilsLib)
endif ()
