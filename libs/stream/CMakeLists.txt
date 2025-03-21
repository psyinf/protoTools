project(protos_streams CXX)
set(CMAKE_CXX_STANDARD 23)
#linker language for header only


file(GLOB_RECURSE HEADER_FILES 	CONFIGURE_DEPENDS "*.h*")
file(GLOB_RECURSE CPP_FILES 	CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} STATIC)
add_library(protos::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_sources(${PROJECT_NAME}
    PRIVATE
    ${CPP_FILES}
    PUBLIC
    ${HEADER_FILES}

)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)