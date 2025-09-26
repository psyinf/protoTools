#pragma once
#include <cstdint>
#include <optional>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace protos::dissector {

/**
 * Describes a field in a protocol message in terms of its name, size. It also holds the rules for subsequent fields
 * that may depend on the value of this field.
 */

struct FieldDescriptor
{
    struct SizeCallCallbackResult
    {
        SizeCallCallbackResult(bool more_bytes, uint16_t value, uint8_t consumed)
            : need_more_bytes(more_bytes), result_value(value), bytes_consumed(consumed)
        {
        }
        bool need_more_bytes{}; //more bytes are needed to determine the size
        uint16_t result_value{};
        uint8_t bytes_consumed{}; //the number of bytes consumed from the start of the buffer that don't contribute to the value
    };
    // TODO: this might need more context
    using SizeCalcCallback = std::function<SizeCallCallbackResult(const FieldDescriptor&, std::span<std::byte> available_bytes)>;
    using FieldId = std::string;

    FieldId     name;        ///< name of the field
    std::string description; ///< description of the field
    uint16_t    size{}; ///< field size in bytes. In case the externalSizeCalculation this is the minimum size to calculate the actual size

    std::vector<std::byte> value;            ///< value of the field, represented as a vector of bytes
    FieldId                determinesSizeOf; ///< name of the field's size that is determined by this field
    FieldId sizeDeterminesExistenceOf;       ///< if this field's value is zero, the referenced field has size 0
    bool    isHeaderValue{}; ///< true if the field is a header value and needs to be checked against its initial value
    int16_t sizeOffset{};    ///< offset to be added to the size of the field
    SizeCalcCallback externalSizeCalculation{}; ///< callback to calculate the size of the field

    // monadic methods
    FieldDescriptor& withDeterminesSizeOf(std::string_view field_name, int16_t sizeOffset = 0);

    FieldDescriptor& withSizeDeterminesExistenceOf(std::string_view field_name);

    FieldDescriptor& withValue(std::vector<std::byte> value);

    FieldDescriptor& withIsHeaderValue(bool is_header_value, std::vector<std::byte>&& value);

    FieldDescriptor& withExternalSizeCalculation(SizeCalcCallback callback)
    {
        this->externalSizeCalculation = std::move(callback);
        return *this;
    }

    template <typename T>
    FieldDescriptor& withValue(const T& value)
    {
        this->value = std::vector<std::byte>(reinterpret_cast<const std::byte*>(&value),
                                             reinterpret_cast<const std::byte*>(&value) + sizeof(T));
        return *this;
    }

    static auto make(const std::string& name, const std::string& description, uint16_t size) -> FieldDescriptor
    {
        return FieldDescriptor{name, description, size, std::vector<std::byte>(size)};
    }

    bool hasFixedSize() const
    {
        return externalSizeCalculation == nullptr && determinesSizeOf.empty() && sizeDeterminesExistenceOf.empty();
    }
};

} // namespace protos::dissector
