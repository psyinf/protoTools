// Example 01 - Simple fixed-size dissection.
//
// Shows the minimum wiring: define a PacketDescriptor, feed it bytes, read the
// resulting PacketData fields as typed values via BitUtils helpers.
//
// Packet layout (host byte order in this demo):
//   header : 4 bytes, ASCII text
//   count  : 4 bytes, unsigned 32-bit int
//   x      : 8 bytes, double

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/common/BitUtils.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>

int main()
{
    using namespace protos::dissector;

    PacketDescriptor tmpl;
    tmpl.add({.name = "header", .size = 4});
    tmpl.add({.name = "count",  .size = sizeof(uint32_t)});
    tmpl.add({.name = "x",      .size = sizeof(double)});

    std::array<std::byte, 4 + 4 + 8> bytes{};
    std::memcpy(bytes.data() + 0, "PROT",      4);
    uint32_t count = 42;
    std::memcpy(bytes.data() + 4, &count,      4);
    double   x     = 3.14159;
    std::memcpy(bytes.data() + 8, &x,          8);

    GenericDissector dissector(tmpl);
    auto packet = dissector.addBytes(std::span<const std::byte>{bytes});
    if (!packet) {
        std::cerr << "Dissection did not complete\n";
        return 1;
    }

    auto header_span = protos::bytes::as_chars_span(packet->get("header").value);
    auto count_val   = protos::bytes::to_number<uint32_t>(packet->get("count").value);
    auto x_val       = protos::bytes::to_number<double>(packet->get("x").value);

    std::cout << "header = " << std::string(header_span.data(), header_span.size()) << "\n"
              << "count  = " << count_val << "\n"
              << "x      = " << x_val     << "\n";
    return 0;
}
