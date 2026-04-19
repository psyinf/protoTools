// Example 07 - Mixed big/little endian ICD record.
//
// Realistic case for the user's ICD-over-UDP goal: the wire format has some
// fields in big-endian and some in little-endian, and the host is little-endian.
//
// KNOWN LIBRARY LIMITATION
// ------------------------
// FieldInterpretation::littleEndian is declared on the struct but NEVER consulted
// by the interpreter (FieldInterpreter::interpret goes straight to
// protos::value::as_variant, which uses std::bit_cast = host byte order).
//
// Until that flag is honored, the idiomatic workaround is: declare BE fields with
// `type = BYTES` so the interpreter hands the raw bytes to a `mapper`, then have
// the mapper reverse the bytes and reinterpret them as the target type. LE fields
// on an LE host flow through normally.
//
// Record layout (total 16 bytes):
//   seq_be  : 4 bytes, BE uint32
//   ts_le   : 8 bytes, LE uint64
//   val_be  : 4 bytes, BE float

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
#include <protos/interpretation/TypeFormatter.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

using namespace protos::interpreter;
using namespace protos::value;

// Mapper that reverses the field bytes and reinterprets them as T, boxed into a Variant.
template <typename T>
static FieldInterpretation::Mapper swap_and_reinterpret(const std::string& out_name, const std::string& fmt)
{
    return [out_name, fmt](const Variant& v) -> InterpretationResults {
        const auto& bytes = std::get<std::vector<std::byte>>(v);
        std::vector<std::byte> rev(bytes.rbegin(), bytes.rend());
        T   native{};
        std::memcpy(&native, rev.data(), sizeof(T));
        if constexpr (std::is_integral_v<T>) {
            return {{{out_name, Variant{static_cast<uint64_t>(native)}}, fmt, false}};
        } else {
            return {{{out_name, Variant{static_cast<double>(native)}}, fmt, false}};
        }
    };
}

int main()
{
    using namespace protos::dissector;

    // --- Build a 16-byte wire buffer with hand-set BE/LE fields. ---
    std::vector<std::byte> wire(16);
    // seq_be = 0x00010203 written big-endian on the wire
    wire[0] = std::byte{0x00}; wire[1] = std::byte{0x01};
    wire[2] = std::byte{0x02}; wire[3] = std::byte{0x03};
    // ts_le = 0x00000000DEADBEEF written little-endian
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

    // --- Interpreter: LE goes through natively; BE uses swap mapper. ---
    GenericPacketInterpreter interp;
    interp.addField({.name = "seq_be", .type = Type::BYTES,
                     .mapper = swap_and_reinterpret<uint32_t>("seq", "{}")});
    interp.addField({.name = "ts_le",  .type = Type::UNSIGNED_INTEGER, .format = "{:#x}"});
    interp.addField({.name = "val_be", .type = Type::BYTES,
                     .mapper = swap_and_reinterpret<float>("val", "{:.3f}")});

    GenericDissector diss(tmpl);
    auto packet = diss.addBytes(wire);
    if (!packet) { std::cerr << "incomplete\n"; return 1; }

    auto results = interp.interpretPacketData(*packet);
    for (const auto& r : results) {
        std::cout << r.name << " = " << variant_to_formatted_string(r.value, r.format) << "\n";
    }
    return 0;
}
