#pragma once

#include <protos/dissection/FieldDefinitions.hpp>

#include <vector>
#include <cstddef>

namespace protos::dissector {

/* An immutable data structure representing a field in a packet */

struct FieldData
{
    const std::string            name;
    const std::vector<std::byte> value;

    bool operator==(const FieldData& other) const { return name == other.name && value == other.value; }
};

} // namespace protos