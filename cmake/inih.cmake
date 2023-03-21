set(INIH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/inih)

add_library(inih INTERFACE)
target_include_directories(inih INTERFACE ${INIH_INCLUDE_DIR})

set(INIH_LIBRARY inih)
