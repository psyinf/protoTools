#pragma once

#include <protos/FieldDefinitions.hpp>

#include <vector>
#include <cstddef>

namespace protos {

/* A named field with a value inside a packet */
struct FieldData
{
    std::string            name;
    std::vector<std::byte> value;
};

} // namespace protos