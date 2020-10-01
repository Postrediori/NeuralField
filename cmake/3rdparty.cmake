find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

set(USE_OPENMP ON CACHE BOOL "Use OpenMP")

# Skip OpenMP on macOS
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(USE_OPENMP OFF)
endif ()

if (USE_OPENMP)
  find_package(OpenMP REQUIRED)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(X11 REQUIRED)

  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

include(cmake/plog.cmake)
include(cmake/glad.cmake)
include(cmake/glfw.cmake)
include(cmake/glm.cmake)
include(cmake/imgui.cmake)
