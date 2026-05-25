project(ex12_service_discovery)
set(CMAKE_CXX_STANDARD 23)

add_executable(ex12_dir_server  server.cpp)
add_executable(ex12_dir_client  client.cpp)

target_link_libraries(ex12_dir_server PRIVATE protos::protos_services)
target_link_libraries(ex12_dir_client PRIVATE protos::protos_services)
