file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.7/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=c0fc82149e00c43a21febe7b2ca57b2ffea2b8e88ab867022c21d6b81937eb50
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)