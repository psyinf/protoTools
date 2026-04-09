cmake_minimum_required(VERSION 3.22)
project(app_template)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(cmake/detect_embedded.cmake)

option(ENABLE_TESTING "Enable testing" ON)
option(BUILD_APPS "Build apps" ON)


include(cmake/PreventInSourceBuilds.cmake)
if (IS_STANDALONE_PROJECT)
    include(cmake/coverage.cmake)
endif()
include(cmake/stacktrace.cmake)
include(cmake/setup_cpm.cmake)
include(cmake/deps_cpm.cmake)


add_subdirectory(libs)
if (BUILD_APPS AND IS_STANDALONE_PROJECT)
    add_subdirectory(apps)
endif()

if (ENABLE_TESTING AND IS_STANDALONE_PROJECT)
    enable_testing()
	add_subdirectory(tests)
endif()

#install(DIRECTORY
#        data/
#    DESTINATION
#        data
#    COMPONENT
#        ${RUNTIME_COMPONENT}
#)