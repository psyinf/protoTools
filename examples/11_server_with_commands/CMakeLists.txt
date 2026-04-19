project(ex11_server_with_commands)
set(CMAKE_CXX_STANDARD 23)

add_executable(ex11_server  server.cpp)
add_executable(ex11_client  client.cpp)

target_link_libraries(ex11_server PRIVATE protos::protos_services)
target_link_libraries(ex11_client PRIVATE protos::protos_services)
