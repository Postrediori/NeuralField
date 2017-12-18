
macro(set_flags_)
    set(STRICT_COMPILATION "-Wall -Werror -std=c++11 -fstrict-aliasing -pedantic-errors -pedantic -Wno-deprecated-declarations -Wno-unused-variable -Wno-unknown-pragmas")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
endmacro()

macro(make_project_)
    if (NOT DEFINED PROJECT)
        get_filename_component(PROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    endif ()

    project(${PROJECT} CXX)

    set_flags_()

    if(MSVC)
        add_definitions(
            -D_USE_MATH_DEFINES
            -DFREEGLUT_LIB_PRAGMAS=0
            -DFREEGLUT_STATIC
            -DGLEW_STATIC
            )
    endif()
    
    if (NOT DEFINED HEADERS)
        file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
    endif ()

    if (NOT DEFINED SOURCES)
        file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    endif ()

    source_group("Header Files" FILES ${HEADERS})
    source_group("Source Files" FILES ${SOURCES})
endmacro ()

macro(make_executable)
    make_project_()
    add_executable(${PROJECT} ${HEADERS} ${SOURCES})

    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bundle/${PROJECT}")
    install(
        TARGETS ${PROJECT}
        DESTINATION ${CMAKE_INSTALL_PREFIX})
endmacro()

macro(make_library)
    make_project_()
    add_library(${PROJECT} STATIC ${HEADERS} ${SOURCES})
    target_include_directories(${PROJECT} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

    if (NOT SOURCES)
        set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)
    endif ()
endmacro()
