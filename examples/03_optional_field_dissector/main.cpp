// Example 03 - Optional trailing field.
//
// Demonstrates FieldDescriptor::sizeDeterminesExistenceOf. If the value of "len"
// is zero, the referenced field ("crc") is treated as absent and never consumed
// from the stream. The resulting PacketData will not even contain a "crc" entry.
//
// Packet layout:
//   len  : 1 byte - length of payload AND gate for crc
//   data : `len` bytes
//   crc  : 1 byte - present only when len > 0

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>

#include <cstddef>
#include <iostream>
#include <vector>

static void dissect_and_print(const std::vector<std::byte>& stream, const char* label)
{
    using namespace protos::dissector;

    PacketDescriptor tmpl;
    tmpl.add({.name = "len",  .size = 1,
              .determinesSizeOf = "data",
              .sizeDeterminesExistenceOf = "crc"});
    tmpl.add({.name = "data", .size = 0});
    tmpl.add({.name = "crc",  .size = 1});

    GenericDissector dissector(tmpl);
    auto packet = dissector.addBytes(stream);
    if (!packet) { std::cout << label << ": incomplete\n"; return; }

    std::cout << label << ":\n"
              << "  len      = " << static_cast<unsigned>(packet->get("len").value[0]) << "\n"
              << "  data.sz  = " << packet->get("data").value.size() << "\n"
              << "  has crc? = " << (packet->has("crc") ? "yes" : "no") << "\n";
}

int main()
{
    // Case 1: empty payload - crc is skipped entirely.
    dissect_and_print({std::byte{0}}, "empty");

    // Case 2: 3-byte payload, followed by crc=0xAA.
    dissect_and_print({std::byte{3}, std::byte{'a'}, std::byte{'b'}, std::byte{'c'}, std::byte{0xAA}},
                      "with-data");
    return 0;
}
