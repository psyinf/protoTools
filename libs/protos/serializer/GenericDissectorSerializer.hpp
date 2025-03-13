#pragma once
#include <datafw/protocols/FieldDescriptor.hpp>
#include <datafw/protocols/PacketDescriptor.hpp>

#include <nlohmann/json.hpp>

namespace protos {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FieldDescriptor,
                                                name,
                                                description,
                                                size,
                                                value,
                                                determinesSizeOf,
                                                sizeDeterminesExistenceOf,
                                                isHeaderValue)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PacketDescriptor, fields, name)
} // namespace datafw::protocol
