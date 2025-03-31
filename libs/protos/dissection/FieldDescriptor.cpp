#include "FieldDescriptor.hpp"


protos::dissector::FieldDescriptor& protos::dissector::FieldDescriptor::withDeterminesSizeOf(std::string_view field_name, int16_t sizeOffset)
{
    determinesSizeOf = field_name;
    this->sizeOffset = sizeOffset;
    return *this;
}

protos::dissector::FieldDescriptor& protos::dissector::FieldDescriptor::withIsHeaderValue(bool is_header_value, std::vector<std::byte>&& value)
{
    isHeaderValue = is_header_value;
    this->value = std::move(value);
    return *this;
}

protos::dissector::FieldDescriptor& protos::dissector::FieldDescriptor::withValue(std::vector<std::byte> value)
{
    this->value = value;
    return *this;
}

protos::dissector::FieldDescriptor& protos::dissector::FieldDescriptor::withSizeDeterminesExistenceOf(std::string_view field_name)
{
    sizeDeterminesExistenceOf = field_name;
    return *this;
}
