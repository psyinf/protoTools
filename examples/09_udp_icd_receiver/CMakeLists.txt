project(ex09_udp_icd_receiver)
set(CMAKE_CXX_STANDARD 23)

add_executable(ex09_udp_sender   sender.cpp)
add_executable(ex09_udp_receiver receiver.cpp)

target_link_libraries(ex09_udp_sender   PRIVATE protos::protos)
target_link_libraries(ex09_udp_receiver PRIVATE protos::protos)

if (WIN32)
    target_link_libraries(ex09_udp_sender   PRIVATE ws2_32)
    target_link_libraries(ex09_udp_receiver PRIVATE ws2_32)
endif()
