
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(IS_STANDALONE_PROJECT TRUE)
    message(STATUS "Your project is standalone")
else()
    set(IS_STANDALONE_PROJECT FALSE)
    message(STATUS "Your project is embedded")
endif()