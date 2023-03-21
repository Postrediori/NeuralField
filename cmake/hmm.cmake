set(HMM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/hmm)

add_library(hmm INTERFACE)
target_include_directories(hmm INTERFACE ${HMM_INCLUDE_DIR})

set(HMM_LIBRARY hmm)
