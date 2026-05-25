// Example 04 - Self-describing length via externalSizeCalculation.
//
// Two scenarios for fields whose length is carried INSIDE the field itself:
//
//  A) "length byte + payload" - first byte is the total length, remainder is payload.
//  B) "null-terminated string" - read bytes until we find 0x00.
//
// Both implement protos::dissector::FieldDescriptor::SizeCalcCallback. Return a
// SizeCallCallbackResult{need_more_bytes, result_value, bytes_consumed} where:
//   need_more_bytes - we haven't accumulated enough bytes yet to decide
//   result_value    - total size of the field when decided
//   bytes_consumed  - bytes at the start that shouldn't count toward the value
//                     (e.g. if the length byte is NOT part of the payload, set 1)

#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/common/BitUtils.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>

using namespace protos::dissector;

static FieldDescriptor::SizeCallCallbackResult length_in_first_byte(
    const FieldDescriptor&, std::span<std::byte> buf)
{
    // First byte is the total length including itself.
    // The value we want (payload size) is length-byte - 1, and we consume 1 byte of
    // framing (the length byte) that is not part of the payload value.
    return {false, static_cast<uint16_t>(static_cast<unsigned>(buf[0]) - 1), 1};
}

static FieldDescriptor::SizeCallCallbackResult until_null_terminator(
    const FieldDescriptor&, std::span<std::byte> buf)
{
    auto it = std::ranges::find(buf, std::byte{0});
    if (it == buf.end()) { return {true, 0, 0}; }  // need more bytes
    auto size = static_cast<uint16_t>(std::distance(buf.begin(), it) + 1);
    return {false, size, 0};
}

template <class Callback>
static void run(const char* label, std::vector<std::byte> stream, Callback cb)
{
    PacketDescriptor tmpl;
    tmpl.add({.name = "field1", .size = 1});
    tmpl.add({.name = "payload", .size = 1, .externalSizeCalculation = cb});
    GenericDissector dissector(tmpl);
    auto packet = dissector.addBytes(stream);
    if (!packet) { std::cout << label << ": incomplete\n"; return; }
    auto chars = protos::bytes::as_chars_span(packet->get("payload").value);
    std::cout << label << ": field1=0x" << std::hex
              << static_cast<unsigned>(packet->get("field1").value[0])
              << std::dec << " payload(" << packet->get("payload").value.size() << ")=\""
              << std::string(chars.data(), chars.size()) << "\"\n";
}

int main()
{
    // A: field1=0x01, length=5, then "helo" (4 payload bytes).
    run("A length-in-first-byte",
        {std::byte{0x01}, std::byte{0x05},
         std::byte{'h'}, std::byte{'e'}, std::byte{'l'}, std::byte{'o'}},
        length_in_first_byte);

    // B: field1=0x01, "helo\0" (5 bytes total).
    run("B null-terminated      ",
        {std::byte{0x01},
         std::byte{'h'}, std::byte{'e'}, std::byte{'l'}, std::byte{'o'}, std::byte{0}},
        until_null_terminator);

    return 0;
}
