cmake_minimum_required(VERSION 3.22)

# All examples expect to be built from the top-level protoTools build.
# Each example is a small, self-contained executable that links against the
# in-tree protos / protos_services libraries. Numbering groups them:
#   01-05  dissection features (primary focus)
#   06-08  interpretation features (primary focus)
#   09     dissector + interpreter fed from a UDP socket
#   10-12  ZMQ services (secondary)

add_subdirectory(01_simple_dissector)
add_subdirectory(02_length_prefixed_dissector)
add_subdirectory(03_optional_field_dissector)
add_subdirectory(04_self_describing_field)
add_subdirectory(05_header_match_resync)
add_subdirectory(06_interpreter_basics)
add_subdirectory(07_icd_mixed_endian)
add_subdirectory(08_interpreter_mappers)
add_subdirectory(09_udp_icd_receiver)
add_subdirectory(10_pub_sub_minimal)
add_subdirectory(11_server_with_commands)
add_subdirectory(12_service_discovery)
