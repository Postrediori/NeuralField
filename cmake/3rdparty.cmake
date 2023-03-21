find_package(OpenGL REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)

  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

# Setup OpenMP
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  message(INFO " Disabling OpenMP on macOS")
  set(USE_OPENMP OFF)
endif ()

if (USE_OPENMP)
  message(INFO "Using OpenMP for parallel calculations")
  find_package(OpenMP REQUIRED)
endif ()

include(cmake/glad.cmake)
include(cmake/glfw.cmake)
include(cmake/hmm.cmake)
include(cmake/imgui.cmake)
include(cmake/inih.cmake)
include(cmake/plog.cmake)
