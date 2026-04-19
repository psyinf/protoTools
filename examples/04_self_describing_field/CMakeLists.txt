project(ex04_self_describing_field)
set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE protos::protos)
