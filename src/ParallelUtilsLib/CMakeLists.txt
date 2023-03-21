make_library()

target_precompile_headers(${PROJECT} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h
    )

target_include_directories(${PROJECT} PRIVATE
    ${OpenCL_INCLUDE_DIR}
    )

target_link_libraries(${PROJECT}
    ${PLOG_LIBRARY}
    ${OpenCL_LIBRARY}
    )
