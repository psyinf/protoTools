#pragma once
#include <protos/FieldData.hpp>
#include <format>

namespace protos {

// describes structure of a byte-oriented packet of fields.
struct PacketData
{
    std::string            name; // usually the name of the protocol
    std::vector<FieldData> fields;

    void add(FieldData&& field) { fields.emplace_back(field); }

    void add(const FieldData& field) { fields.push_back(field); }

    FieldData& get(const std::string& field_name)
    {
        auto iter = std::ranges::find_if(fields, [&field_name](const auto& field) { return field.name == field_name; });
        if (iter != fields.end()) { return *iter; }
        else { throw std::runtime_error(std::format("Field {} not found in PacketData {}", field_name, name)); }
    }

    const FieldData& get(const std::string& field_name) const { return const_cast<PacketData*>(this)->get(field_name); }

    bool has(const std::string& field_name) const
    {
        return std::ranges::find_if(fields, [&field_name](const auto& field) { return field.name == field_name; }) !=
               fields.end();
    }
};
} // namespace protos::