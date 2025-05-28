#pragma once
#include <protos/dissection/FieldData.hpp>
#include <format>

namespace protos::dissector {

// describes structure of a byte-oriented packet of fields.
struct PacketData
{

    PacketData clone() const
    {
        PacketData clonePacket {};
        clonePacket.name = name;
        for (auto& field : fields)
        {
            clonePacket.fields.push_back({field.name, field.value});
        }
        return clonePacket;
    };
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

    bool operator==(const PacketData& other) const { return name == other.name && fields == other.fields; }

};
} // namespace protos
