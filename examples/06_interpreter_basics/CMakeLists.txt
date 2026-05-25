project(ex06_interpreter_basics)
set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE protos::protos)
