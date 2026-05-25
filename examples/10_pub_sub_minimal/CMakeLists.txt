project(ex10_pub_sub_minimal)
set(CMAKE_CXX_STANDARD 23)

add_executable(ex10_publisher   publisher.cpp)
add_executable(ex10_subscriber  subscriber.cpp)

target_link_libraries(ex10_publisher  PRIVATE protos::protos_services)
target_link_libraries(ex10_subscriber PRIVATE protos::protos_services)
