#pragma once
#include <protos/dissection/FieldData.hpp>
#include <algorithm>
#include <fmt/format.h>

namespace protos::dissector {

// describes structure of a byte-oriented packet of fields.
struct PacketData
{
    std::string            name; // usually the name of the protocol
    std::vector<FieldData> fields;

    void add(FieldData&& field) { fields.emplace_back(field); }

    void add(const FieldData& field) { fields.push_back(field); }

    FieldData& get(const std::string& field_name)
    {
        auto iter = std::find_if(fields.begin(), fields.end(), [&field_name](const auto& field) { return field.name == field_name; });
        if (iter != fields.end()) { return *iter; }
        else { throw std::runtime_error(fmt::format("Field {} not found in PacketData {}", field_name, name)); }
    }

    // this is a const overload of get, the const_cast is safe here because we are not modifying the object
    // we really don't want to duplicate the code of get.
    const FieldData& get(const std::string& field_name) const { return const_cast<PacketData*>(this)->get(field_name); }

    bool has(const std::string& field_name) const
    {
        return std::find_if(fields.begin(), fields.end(), [&field_name](const auto& field) { return field.name == field_name; }) !=
               fields.end();
    }

    bool operator==(const PacketData& other) const { return name == other.name && fields == other.fields; }
};
} // namespace protos::dissector