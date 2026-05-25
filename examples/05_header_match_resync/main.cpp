// Example 05 - Header match with multiple descriptors.
//
// When framing can drop bytes or splice multiple candidate protocols onto the same
// stream, GenericDissector::matchesHeader(header_bytes) lets you pre-check whether
// a PacketDescriptor's header would accept a given start-of-packet window. This is
// the building block for "try all known descriptors, pick the one whose magic
// matches" style dispatch.

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/common/BitUtils.hpp>

#include <cstddef>
#include <iostream>
#include <vector>

using namespace protos::dissector;

static PacketDescriptor make_tmpl(const char* magic, uint16_t body_size)
{
    PacketDescriptor tmpl;
    tmpl.name = magic;
    FieldDescriptor hdr{.name = "magic", .size = 4, .isHeaderValue = true};
    hdr.withValue(std::vector<std::byte>{
        std::byte{static_cast<unsigned char>(magic[0])},
        std::byte{static_cast<unsigned char>(magic[1])},
        std::byte{static_cast<unsigned char>(magic[2])},
        std::byte{static_cast<unsigned char>(magic[3])}});
    tmpl.add(std::move(hdr));
    tmpl.add({.name = "body", .size = body_size});
    return tmpl;
}

int main()
{
    auto a = make_tmpl("AAAA", 2);
    auto b = make_tmpl("BBBB", 2);
    GenericDissector dissA(a);
    GenericDissector dissB(b);

    // A 4-byte window. matchesHeader inspects only the header fields.
    std::vector<std::byte> window{std::byte{'B'}, std::byte{'B'}, std::byte{'B'}, std::byte{'B'}};

    std::cout << "window matches A? " << (dissA.matchesHeader(window) ? "yes" : "no") << "\n"
              << "window matches B? " << (dissB.matchesHeader(window) ? "yes" : "no") << "\n";

    // Dispatch the full packet to whichever descriptor matches.
    std::vector<std::byte> full{std::byte{'B'}, std::byte{'B'}, std::byte{'B'}, std::byte{'B'},
                                 std::byte{0x01}, std::byte{0x02}};
    auto packet = dissB.addBytes(full);
    if (packet) {
        std::cout << "packet.name=" << packet->name
                  << " body[0]=0x" << std::hex << static_cast<unsigned>(packet->get("body").value[0]) << "\n";
    }
    return 0;
}
