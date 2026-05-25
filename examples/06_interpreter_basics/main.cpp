// Example 06 - Dissect + Interpret pipeline (little-endian host path).
//
// Shows how to turn raw PacketData (output of a dissector) into typed
// InterpretationResults with GenericPacketInterpreter. One interpreter registers
// one FieldInterpretation per dissector field name; unknown fields are handled
// per the configured EmptyFieldBehavior (default: SKIP_AND_WARN_ONCE).
//
// Packet layout (all little-endian on x86 hosts):
//   id      : 1 byte  unsigned
//   counter : 2 bytes unsigned
//   rate    : 4 bytes float

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
#include <protos/interpretation/TypeFormatter.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

int main()
{
    using namespace protos::dissector;
    using namespace protos::interpreter;

    PacketDescriptor tmpl;
    tmpl.add({.name = "id",      .size = 1});
    tmpl.add({.name = "counter", .size = 2});
    tmpl.add({.name = "rate",    .size = 4});

    GenericPacketInterpreter interp;
    interp.addField({.name = "id",      .type = protos::value::Type::UNSIGNED_INTEGER, .format = "{}"});
    interp.addField({.name = "counter", .type = protos::value::Type::UNSIGNED_INTEGER, .format = "{}"});
    interp.addField({.name = "rate",    .type = protos::value::Type::FLOAT,            .format = "{:.3f}"});

    // Build a packet buffer directly.
    std::vector<std::byte> buf(7);
    buf[0] = std::byte{0x2A};
    uint16_t counter = 1234;       std::memcpy(&buf[1], &counter, 2);
    float    rate    = 3.141593f;  std::memcpy(&buf[3], &rate,    4);

    GenericDissector diss(tmpl);
    auto packet = diss.addBytes(buf);
    if (!packet) { std::cerr << "incomplete\n"; return 1; }

    auto results = interp.interpretPacketData(*packet);
    for (const auto& r : results) {
        std::cout << r.name << " = "
                  << protos::value::variant_to_formatted_string(r.value, r.format) << "\n";
    }
    return 0;
}
