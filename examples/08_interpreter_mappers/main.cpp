// Example 08 - Mappers for enums, scaled values, and bit decomposition.
//
// A `FieldInterpretation::mapper` is a free-form std::function that takes the raw
// Variant produced by as_variant() and returns zero or more InterpretationResults.
// Three common ICD patterns:
//
//  1. Enum code -> human label
//  2. Scaled integer -> engineering unit (e.g. raw*0.1 = tenths of degC)
//  3. One byte -> multiple named bit-flags (a mapper may emit MANY results)

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
#include <protos/interpretation/TypeFormatter.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace protos::interpreter;
using namespace protos::value;

int main()
{
    using namespace protos::dissector;

    // --- 1. State code -> label.
    auto state_mapper = [](const Variant& v) -> InterpretationResults {
        auto code = std::get<uint64_t>(v);
        std::string label;
        switch (code) {
            case 0: label = "IDLE"; break;
            case 1: label = "RUN";  break;
            case 2: label = "FAIL"; break;
            default: label = "UNKNOWN";
        }
        return {{{"state", Variant{label}}, "{}", false}};
    };

    // --- 2. Raw * 0.1 -> degrees C.
    auto temp_scale = [](const Variant& v) -> InterpretationResults {
        auto raw = std::get<uint64_t>(v);
        double degC = static_cast<double>(raw) * 0.1;
        return {{{"temp_c", Variant{degC}}, "{:.1f}", false}};
    };

    // --- 3. Flags byte -> individual booleans.
    auto flag_bits = [](const Variant& v) -> InterpretationResults {
        auto byte = std::get<uint64_t>(v);
        return {
            {{"armed",   Variant{(byte & 0x01) != 0}}, "{}", false},
            {{"ready",   Variant{(byte & 0x02) != 0}}, "{}", false},
            {{"faulted", Variant{(byte & 0x04) != 0}}, "{}", false},
        };
    };

    PacketDescriptor tmpl;
    tmpl.add({.name = "state",    .size = 1});
    tmpl.add({.name = "raw_temp", .size = 2});
    tmpl.add({.name = "flags",    .size = 1});

    GenericPacketInterpreter interp;
    interp.addField({.name = "state",    .type = Type::UNSIGNED_INTEGER, .mapper = state_mapper});
    interp.addField({.name = "raw_temp", .type = Type::UNSIGNED_INTEGER, .mapper = temp_scale});
    interp.addField({.name = "flags",    .type = Type::UNSIGNED_INTEGER, .mapper = flag_bits});

    // Build a test packet: state=1 (RUN), raw_temp=235 (23.5 C), flags=0b00000011 (armed+ready)
    std::vector<std::byte> buf(4);
    buf[0] = std::byte{0x01};
    uint16_t raw = 235; std::memcpy(&buf[1], &raw, 2);
    buf[3] = std::byte{0x03};

    GenericDissector diss(tmpl);
    auto packet = diss.addBytes(buf);
    if (!packet) { std::cerr << "incomplete\n"; return 1; }

    auto results = interp.interpretPacketData(*packet);
    for (const auto& r : results) {
        std::cout << r.name << " = " << variant_to_formatted_string(r.value, r.format) << "\n";
    }
    return 0;
}
