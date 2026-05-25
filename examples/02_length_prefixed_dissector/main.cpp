// Example 02 - Length-prefixed variable field.
//
// Demonstrates FieldDescriptor::determinesSizeOf. The value read from "len"
// becomes the size (in bytes) of the field named "payload". Any following
// fields continue normally.
//
// Packet layout:
//   len      : 1 byte, unsigned, = size of payload
//   payload  : `len` bytes
//   trailer  : 4 bytes, uint32_t (fixed)

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/common/BitUtils.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    using namespace protos::dissector;

    PacketDescriptor tmpl;
    tmpl.add({.name = "len",     .size = 1, .determinesSizeOf = "payload"});
    tmpl.add({.name = "payload", .size = 0});  // size filled in from `len`
    tmpl.add({.name = "trailer", .size = sizeof(uint32_t)});

    // Build a stream: len=5, payload="hello", trailer=0xDEADBEEF
    std::vector<std::byte> stream;
    stream.push_back(std::byte{5});
    for (char c : std::string_view{"hello"}) stream.push_back(std::byte{static_cast<unsigned char>(c)});
    uint32_t trailer = 0xDEADBEEF;
    auto     trailer_bytes = protos::bytes::to_bytes(trailer);
    stream.insert(stream.end(), trailer_bytes.begin(), trailer_bytes.end());

    GenericDissector dissector(tmpl);
    auto packet = dissector.addBytes(stream);
    if (!packet) { std::cerr << "incomplete\n"; return 1; }

    std::cout << "len          = " << static_cast<unsigned>(packet->get("len").value[0]) << "\n";
    auto pay = protos::bytes::as_chars_span(packet->get("payload").value);
    std::cout << "payload      = " << std::string(pay.data(), pay.size()) << "\n";
    std::cout << "trailer(hex) = " << std::hex << protos::bytes::to_number<uint32_t>(packet->get("trailer").value) << "\n";
    return 0;
}
