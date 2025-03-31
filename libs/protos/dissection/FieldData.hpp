#pragma once

#include <protos/FieldDefinitions.hpp>

#include <vector>
#include <cstddef>

namespace protos {

/* An immutable data structure representing a field in a packet */

struct FieldData
{
    const std::string            name;
    const std::vector<std::byte> value;

    bool operator==(const FieldData& other) const { return name == other.name && value == other.value; }
};

} // namespace protos