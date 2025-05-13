#pragma once

#include <bit>
#include <exception>
#include <span>
#include <stdexcept>
#include <vector>
#include <span>
#include <ranges>
#include <cstring>

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

template <class T>
constexpr auto as_bytes(const T& value)
{
    // vector of bytes
    return std::vector<std::byte>(reinterpret_cast<const std::byte*>(&value),
                                  reinterpret_cast<const std::byte*>(&value) + sizeof(T));
}

template <class T>
constexpr std::span<const char> as_chars(const T& container)
{
    return std::span<const char>{std::bit_cast<char*>(container.data()), container.size()};
}

/**
 * Convert a number of type T to a span of bytes
 */

template <typename T>
    requires std::is_arithmetic_v<T>
std::vector<std::byte> to_bytes(T value, size_t width)
{
    if (width > sizeof(T)) { throw std::runtime_error("Width exceeds size of type"); }
    std::vector<std::byte> bytes;
    bytes.resize(width);
    std::memcpy(bytes.data(), &value, width);
    return bytes;
}

/**
 * Convert a span of bytes to a number of type T
 */
template <typename T>
    requires std::is_arithmetic_v<T>
T to_number(const std::span<const std::byte>& container)
{
    if (container.size() != sizeof(T)) { throw std::runtime_error("Invalid size"); }
    return *std::bit_cast<T*>(container.data());
} 

} // namespace protos::bytes