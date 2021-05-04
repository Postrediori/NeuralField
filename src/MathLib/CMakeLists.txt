make_library()

target_link_libraries(${PROJECT} PRIVATE
    ${PLOG_LIBRARY}
    ${OpenMP_CXX_FLAGS}
    )
