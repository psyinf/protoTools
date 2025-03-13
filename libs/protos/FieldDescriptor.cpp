#include "FieldDescriptor.hpp"


protos::FieldDescriptor& protos::FieldDescriptor::withDeterminesSizeOf(std::string_view field_name, int16_t sizeOffset)
{
    determinesSizeOf = field_name;
    this->sizeOffset = sizeOffset;
    return *this;
}

protos::FieldDescriptor& protos::FieldDescriptor::withIsHeaderValue(bool is_header_value, std::vector<std::byte>&& value)
{
    isHeaderValue = is_header_value;
    this->value = std::move(value);
    return *this;
}

protos::FieldDescriptor& protos::FieldDescriptor::withValue(std::vector<std::byte> value)
{
    this->value = value;
    return *this;
}

protos::FieldDescriptor& protos::FieldDescriptor::withSizeDeterminesExistenceOf(std::string_view field_name)
{
    sizeDeterminesExistenceOf = field_name;
    return *this;
}
