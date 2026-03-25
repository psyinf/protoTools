option(ENABLE_COVERAGE "Enable code coverage instrumentation (GCC/Clang on Unix-like only)" OFF)

function(enable_coverage project_name)
  if(NOT ENABLE_COVERAGE)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(${project_name} INTERFACE --coverage -O0 -g)
    target_link_libraries(${project_name} INTERFACE --coverage)
  endif()
endfunction()
