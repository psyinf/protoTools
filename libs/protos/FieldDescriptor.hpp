#pragma once
#include <string>
#include <vector>
#include <cstddef>

namespace protos {

/**
 * Describes a field in a protocol message in terms of its name, size. It also holds the rules for subsequent fields
 * that may depend on the value of this field.
 */

struct FieldDescriptor
{
    using FieldId = std::string;

    FieldId     name;        ///< name of the field
    std::string description; ///< description of the field
    uint16_t    size;        ///< size of the field in bytes

    std::vector<std::byte> value;            ///< value of the field, represented as a vector of bytes
    FieldId                determinesSizeOf; ///< name of the field's size that is determined by this field
    FieldId sizeDeterminesExistenceOf;       ///< if this field's value is zero, the referenced field has size 0
    bool    isHeaderValue; ///< true if the field is a header value and needs to be checked against its initial value
    int16_t sizeOffset{};  ///< offset to be added to the size of the field

    // monadic methods
    FieldDescriptor& withDeterminesSizeOf(std::string_view field_name, int16_t sizeOffset = 0);

    FieldDescriptor& withSizeDeterminesExistenceOf(std::string_view field_name);

    FieldDescriptor& withValue(std::vector<std::byte> value);

    FieldDescriptor& withIsHeaderValue(bool is_header_value, std::vector<std::byte>&& value);

    static auto make(const std::string& name, const std::string& description, uint16_t size) -> FieldDescriptor
    {
        return FieldDescriptor{ name, description, size, std::vector<std::byte>(size)};
    }
};

} // namespace protos
