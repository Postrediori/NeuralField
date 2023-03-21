make_library()

target_precompile_headers(${PROJECT} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)

target_link_libraries(${PROJECT}
    ${PLOG_LIBRARY}
    )

if (USE_OPENMP)
    target_link_libraries(${PROJECT} ${OpenMP_CXX_LIB_NAMES})
endif()
