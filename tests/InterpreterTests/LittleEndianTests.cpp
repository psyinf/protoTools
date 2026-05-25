// Verifies FieldInterpretation::littleEndian is honored for numeric types.
// These tests construct PacketData directly (no dissector) and feed it to a
// GenericPacketInterpreter configured with per-field endianness.
//
// Assumes an LE host (all supported build targets today: x86_64, aarch64 in
// Windows mode). On a BE host the flag interpretation flips but the behaviour
// matches, so the tests remain correct.

#include <catch2/catch_test_macros.hpp>

#include <protos/dissection/PacketData.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

using protos::dissector::FieldData;
using protos::dissector::PacketData;
using protos::interpreter::FieldInterpretation;
using protos::interpreter::GenericPacketInterpreter;
using protos::value::Type;

namespace {

std::vector<std::byte> bytes_from(std::initializer_list<unsigned> xs)
{
    std::vector<std::byte> out;
    out.reserve(xs.size());
    for (auto v : xs) { out.push_back(std::byte{static_cast<unsigned char>(v)}); }
    return out;
}

} // namespace

TEST_CASE("LE uint32 with default flag reads natively on LE host", "[LittleEndian]")
{
    // 0xDEADBEEF stored as LE: EF BE AD DE
    PacketData pd;
    pd.add(FieldData{"val", bytes_from({0xEF, 0xBE, 0xAD, 0xDE})});

    GenericPacketInterpreter interp;
    interp.addField({.name = "val", .type = Type::UNSIGNED_INTEGER /* littleEndian defaults to true */});

    auto results = interp.interpretPacketData(pd);
    REQUIRE(results.size() == 1);
    REQUIRE(std::get<uint64_t>(results[0].value) == 0xDEADBEEFull);
}

TEST_CASE("BE uint32 with littleEndian=false reads correctly on LE host", "[LittleEndian]")
{
    // 0xDEADBEEF stored as BE: DE AD BE EF
    PacketData pd;
    pd.add(FieldData{"val", bytes_from({0xDE, 0xAD, 0xBE, 0xEF})});

    GenericPacketInterpreter interp;
    interp.addField({.name = "val", .type = Type::UNSIGNED_INTEGER, .littleEndian = false});

    auto results = interp.interpretPacketData(pd);
    REQUIRE(results.size() == 1);
    REQUIRE(std::get<uint64_t>(results[0].value) == 0xDEADBEEFull);
}

TEST_CASE("Mixed endian packet: one LE field and one BE field", "[LittleEndian]")
{
    PacketData pd;
    // seq: BE uint32 = 0x00010203
    pd.add(FieldData{"seq_be", bytes_from({0x00, 0x01, 0x02, 0x03})});
    // ts: LE uint64 = 0x00000000DEADBEEF
    pd.add(FieldData{"ts_le", bytes_from({0xEF, 0xBE, 0xAD, 0xDE, 0x00, 0x00, 0x00, 0x00})});

    GenericPacketInterpreter interp;
    interp.addField({.name = "seq_be", .type = Type::UNSIGNED_INTEGER, .littleEndian = false});
    interp.addField({.name = "ts_le", .type = Type::UNSIGNED_INTEGER /* LE by default */});

    auto results = interp.interpretPacketData(pd);
    REQUIRE(results.size() == 2);
    REQUIRE(std::get<uint64_t>(results[0].value) == 0x00010203ull);
    REQUIRE(std::get<uint64_t>(results[1].value) == 0xDEADBEEFull);
}

TEST_CASE("BE float with littleEndian=false reads correctly", "[LittleEndian]")
{
    // 1.0f as BE: 3F 80 00 00
    PacketData pd;
    pd.add(FieldData{"val", bytes_from({0x3F, 0x80, 0x00, 0x00})});

    GenericPacketInterpreter interp;
    interp.addField({.name = "val", .type = Type::FLOAT, .littleEndian = false});

    auto results = interp.interpretPacketData(pd);
    REQUIRE(results.size() == 1);
    REQUIRE(std::get<double>(results[0].value) == 1.0);
}

TEST_CASE("Signed BE int16 round-trips through littleEndian=false", "[LittleEndian]")
{
    // -2 as BE int16: FF FE
    PacketData pd;
    pd.add(FieldData{"val", bytes_from({0xFF, 0xFE})});

    GenericPacketInterpreter interp;
    interp.addField({.name = "val", .type = Type::INTEGER, .littleEndian = false});

    auto results = interp.interpretPacketData(pd);
    REQUIRE(results.size() == 1);
    REQUIRE(std::get<int64_t>(results[0].value) == -2);
}

TEST_CASE("STRING ignores littleEndian flag (stream order preserved)", "[LittleEndian]")
{
    // STRING must NOT be reversed even with littleEndian=false, or "ABCD" becomes "DCBA".
    PacketData pd;
    pd.add(FieldData{"s", bytes_from({'A', 'B', 'C', 'D'})});

    GenericPacketInterpreter interp;
    interp.addField({.name = "s", .type = Type::STRING, .littleEndian = false});

    auto results = interp.interpretPacketData(pd);
    REQUIRE(results.size() == 1);
    REQUIRE(std::get<std::string>(results[0].value) == "ABCD");
}

TEST_CASE("BYTES ignores littleEndian flag (raw passthrough)", "[LittleEndian]")
{
    // BYTES is the escape hatch for manual endian handling via mappers;
    // auto-swapping would break existing code.
    PacketData pd;
    pd.add(FieldData{"b", bytes_from({0x01, 0x02, 0x03, 0x04})});

    GenericPacketInterpreter interp;
    interp.addField({.name = "b", .type = Type::BYTES, .littleEndian = false});

    auto results = interp.interpretPacketData(pd);
    REQUIRE(results.size() == 1);
    const auto& out = std::get<std::vector<std::byte>>(results[0].value);
    REQUIRE(out.size() == 4);
    REQUIRE(out[0] == std::byte{0x01});
    REQUIRE(out[3] == std::byte{0x04});
}
