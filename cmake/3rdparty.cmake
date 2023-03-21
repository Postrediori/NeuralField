find_package(OpenGL REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)

  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR " X11 Xi library is required")
  endif ()
endif ()

# Setup OpenMP
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  message(STATUS "Disabling OpenMP on macOS")
  set(USE_OPENMP OFF)
endif ()

if (USE_OPENMP)
  message(STATUS "Using OpenMP for parallel calculations")
  find_package(OpenMP REQUIRED)
endif ()

# Setup OpenCL
if (USE_OPENCL)
  message(STATUS "Using OpenCL for calculations")
  find_package(OpenCL REQUIRED)
endif ()

include(cmake/glad.cmake)
include(cmake/glfw.cmake)
include(cmake/hmm.cmake)
include(cmake/imgui.cmake)
include(cmake/inih.cmake)
include(cmake/plog.cmake)
