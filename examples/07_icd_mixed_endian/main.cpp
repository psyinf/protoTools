// Example 07 - Mixed big/little endian ICD record.
//
// Declare each field's wire endianness via FieldInterpretation::littleEndian.
// The interpreter reverses a field's bytes before decoding when the declared
// endianness disagrees with std::endian::native, for numeric types
// (INTEGER / UNSIGNED_INTEGER / FLOAT). STRING and BYTES keep stream order.
//
// Record layout (total 16 bytes):
//   seq_be  : 4 bytes, BE uint32
//   ts_le   : 8 bytes, LE uint64
//   val_be  : 4 bytes, BE float

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
#include <protos/interpretation/TypeFormatter.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

using namespace protos::interpreter;
using namespace protos::value;

int main()
{
    using namespace protos::dissector;

    // --- Build a 16-byte wire buffer with hand-set BE/LE fields. ---
    std::vector<std::byte> wire(16);
    // seq_be = 0x00010203 written big-endian on the wire
    wire[0] = std::byte{0x00}; wire[1] = std::byte{0x01};
    wire[2] = std::byte{0x02}; wire[3] = std::byte{0x03};
    // ts_le = 0xDEADBEEF written little-endian
    uint64_t ts = 0xDEADBEEFull;
    std::memcpy(&wire[4], &ts, 8);
    // val_be = 1.0f written big-endian (float 1.0 = 0x3F800000)
    wire[12] = std::byte{0x3F}; wire[13] = std::byte{0x80};
    wire[14] = std::byte{0x00}; wire[15] = std::byte{0x00};

    // --- Descriptor: three fixed fields. ---
    PacketDescriptor tmpl;
    tmpl.add({.name = "seq_be", .size = 4});
    tmpl.add({.name = "ts_le",  .size = 8});
    tmpl.add({.name = "val_be", .size = 4});

    // --- Interpreter: one declarative flag per field. ---
    GenericPacketInterpreter interp;
    interp.addField({.name = "seq_be", .type = Type::UNSIGNED_INTEGER, .format = "{}",
                     .littleEndian = false});
    interp.addField({.name = "ts_le",  .type = Type::UNSIGNED_INTEGER, .format = "{:#x}"});
    interp.addField({.name = "val_be", .type = Type::FLOAT, .format = "{:.3f}",
                     .littleEndian = false});

    GenericDissector diss(tmpl);
    auto packet = diss.addBytes(wire);
    if (!packet) { std::cerr << "incomplete\n"; return 1; }

    auto results = interp.interpretPacketData(*packet);
    for (const auto& r : results) {
        std::cout << r.name << " = " << variant_to_formatted_string(r.value, r.format) << "\n";
    }
    return 0;
}
