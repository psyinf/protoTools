#pragma once
#include <protos/GenericDissector.hpp>
#include <protos/BitUtils.hpp>

using namespace protos::dissector;
using namespace protos;
using Catch::Matchers::Equals;

template <std::size_t N>
constexpr std::array<char, N - 1> to_array_wo_null(const char (&str)[N])
{
    std::array<char, N - 1> result{};
    for (std::size_t i = 0; i < N - 1; ++i)
    {
        result[i] = str[i];
    }
    return result;
}

std::optional<protos::PacketData> test_dissect(protos::dissector::GenericDissector& dissector,
                                               const std::span<const std::byte>&    bytes)
{
    std::optional<protos::PacketData> res;
    // dissector.setPacketDataCallback([&res](const PacketData& packet) { res.emplace(packet); });
    for (const auto& [index, byte] : std::views::enumerate(bytes))
    {
        res = dissector.addByte(byte);

        if (index < bytes.size() - 1) { REQUIRE(!res.has_value()); }

        else { REQUIRE(res.has_value()); }
    }
    std::optional<protos::PacketData> res2 = dissector.addBytes(bytes);
    REQUIRE(res == res2);
    REQUIRE(res2.has_value());
    return res;
}



struct SimpleTestPacket
{
    std::array<char, 16> header;
    double               x;
    double               y;
};

struct PaddedTestPacket
{
    std::array<char, 14> header;
    double               x;
    double               y;
};

template <uint8_t Size>
struct DependentSizePacket
{
    const uint8_t          size{Size};
    std::array<char, Size> data;
    float                  x{Size};
};

template <uint8_t Size>
struct DependentSizeWithCRCPacket
{
    const uint8_t          size{Size};
    std::array<char, Size> data;
    uint8_t                crc{0};
};
