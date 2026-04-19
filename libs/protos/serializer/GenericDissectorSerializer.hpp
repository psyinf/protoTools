#pragma once
#include <protos/dissection/FieldDescriptor.hpp>
#include <protos/dissection/PacketDescriptor.hpp>

#include <nlohmann/json.hpp>

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>

// nlohmann does not know std::byte natively. Serialize each byte as an
// uppercase "0xNN" hex string - readable when eyeballing an ICD descriptor
// and explicit about the encoding. FieldDescriptor::value is therefore a
// JSON array of strings.
namespace nlohmann {
template <>
struct adl_serializer<std::byte>
{
    static void to_json(json& j, const std::byte& b)
    {
        j = std::format("0x{:02X}", std::to_integer<unsigned>(b));
    }
    static void from_json(const json& j, std::byte& b)
    {
        // std::stoul with base 16 accepts an optional "0x" / "0X" prefix.
        auto s = j.get<std::string>();
        b      = std::byte{static_cast<unsigned char>(std::stoul(s, nullptr, 16))};
    }
};
} // namespace nlohmann

namespace protos::dissector {

// Static shape only. FieldDescriptor::externalSizeCalculation is a std::function
// and therefore never round-trips; if the descriptor you are loading uses a
// size callback, re-attach it after load() via withExternalSizeCalculation().
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FieldDescriptor,
                                                name,
                                                description,
                                                size,
                                                value,
                                                determinesSizeOf,
                                                sizeDeterminesExistenceOf,
                                                isHeaderValue,
                                                sizeOffset)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PacketDescriptor, fields, name)

template <class T>
void save(const std::filesystem::path& path, const T& obj)
{
    std::ofstream os(path);
    if (!os.is_open())
    {
        throw std::runtime_error(std::format("save: cannot open '{}' for writing", path.string()));
    }
    nlohmann::json j = obj;
    os << j.dump(2);
}

template <class T>
T load(const std::filesystem::path& path)
{
    std::ifstream is(path);
    if (!is.is_open())
    {
        throw std::runtime_error(std::format("load: cannot open '{}' for reading", path.string()));
    }
    nlohmann::json j;
    is >> j;
    return j.get<T>();
}

} // namespace protos::dissector
