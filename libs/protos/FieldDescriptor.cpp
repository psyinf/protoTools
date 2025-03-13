#include "FieldDescriptor.hpp"


protos::FieldDescriptor& protos::FieldDescriptor::withDeterminedSizeOf(std::string_view field_name)
{
    determinesSizeOf = field_name;
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
