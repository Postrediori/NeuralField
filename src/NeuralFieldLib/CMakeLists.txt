make_library()

include_directories(
    ${PLOG_INCLUDE_DIR}
    )

target_link_libraries(${PROJECT}
    MathLib
    )
