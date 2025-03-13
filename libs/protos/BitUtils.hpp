#pragma once

#include <bit>
#include <span>
#include <stdexcept>
#include <vector>

namespace protos::bytes {

/**
 * Convert a span of bytes to a number of type T
 */
template <class T>
constexpr T as_number(const std::span<const std::byte>& container)
{
    if (container.size() != sizeof(T)) { throw std::runtime_error("Invalid size"); }
    return *std::bit_cast<T*>(container.data());
}

/**
 * Convert a number of type T to a span of bytes
 */

template <typename T>
    requires std::is_arithmetic_v<T>
std::vector<std::byte> toBytes(T value, size_t width)
{
    if (width > sizeof(T)) { throw std::runtime_error("Width exceeds size of type"); }
    std::vector<std::byte> bytes;
    bytes.resize(width);
    std::memcpy(bytes.data(), &value, width);
    return bytes;
}
} // namespace protos::bytes