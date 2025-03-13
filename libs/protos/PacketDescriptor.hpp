#pragma once
#include <protos/FieldDescriptor.hpp>

#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace protos{

// describes a packet as fields of bytes with an associated name, type and further contextual information
struct PacketDescriptor
{
    std::vector<FieldDescriptor> fields;
    std::string                  name;

    void add(FieldDescriptor&& field) { fields.emplace_back(field); }

    FieldDescriptor& get(std::string_view name)
    {
        auto iter = std::ranges::find_if(fields, [&name](const FieldDescriptor& f) { return f.name == name; });
        if (iter == fields.end()) { throw std::runtime_error("Field not found"); }
        return *iter;
    }

    auto getName() const { return name; }
};

} // namespace protos::