set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bundle")

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/data"
    DESTINATION ${CMAKE_INSTALL_PREFIX})

macro(make_project_)
    if (NOT DEFINED PROJECT)
        get_filename_component(PROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    endif ()

    project(${PROJECT} CXX)

    if (NOT DEFINED HEADERS)
        file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
    endif ()

    if (NOT DEFINED SOURCES)
        file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    endif ()

    source_group("Header Files" FILES ${HEADERS})
    source_group("Source Files" FILES ${SOURCES})
endmacro ()

macro(make_project_options_)
    if (USE_OPENMP)
        target_compile_options(${PROJECT} PUBLIC ${OpenMP_CXX_FLAGS})
        target_compile_definitions(${PROJECT} PUBLIC USE_OPENMP)
    endif ()

    if (USE_OPENCL)
        target_compile_definitions(${PROJECT} PUBLIC USE_OPENCL
            CL_TARGET_OPENCL_VERSION=120)
    endif ()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Flags for Visual Studio compiler
        target_compile_options(${PROJECT} PUBLIC /Wall)
        target_compile_definitions(${PROJECT} PUBLIC _USE_MATH_DEFINES)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # Flags for gcc compiler
        target_compile_options(${PROJECT} PUBLIC -Wall -Wextra -Wpedantic -Werror)
        target_compile_options(${PROJECT} PUBLIC -Wno-stringop-truncation)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Flags for clang compiler
        target_compile_options(${PROJECT} PUBLIC -Wall -Wextra -Wpedantic -Werror)
        # Flags to compile HandmadeMath with Clang
        target_compile_options(${PROJECT} PUBLIC
            -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-missing-field-initializers -Wno-missing-braces)
    endif ()
endmacro()

macro(make_executable)
    make_project_()
    add_executable(${PROJECT} ${HEADERS} ${SOURCES})
    make_project_options_()

    install(
        TARGETS ${PROJECT}
        DESTINATION ${CMAKE_INSTALL_PREFIX})
endmacro()

macro(make_library)
    make_project_()
    add_library(${PROJECT} STATIC ${HEADERS} ${SOURCES})
    make_project_options_()
    target_include_directories(${PROJECT} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

    set_target_properties(${PROJECT} PROPERTIES FOLDER Libraries)

    if (NOT SOURCES)
        set_target_properties(${PROJECT} PROPERTIES LINKER_LANGUAGE CXX)
    endif ()
endmacro()

function(add_all_subdirectories)
    file(GLOB CHILDREN RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

    foreach(CHILD ${CHILDREN})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CHILD})
            add_subdirectory(${CHILD})
        endif ()
    endforeach ()
endfunction()
