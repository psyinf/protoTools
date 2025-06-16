#pragma once

#include <bit>
#include <exception>
#include <span>
#include <stdexcept>
#include <vector>
#include <span>
#include <ranges>

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

std::span<const std::byte> as_bytes_span(const std::ranges::common_range auto& container)
    requires std::ranges::viewable_range<decltype(container)>
{
    return std::span<const std::byte>(reinterpret_cast<const std::byte*>(container.data()), container.size());
}

std::span<const char> as_chars_span(const std::ranges::common_range auto& container)
    requires std::ranges::viewable_range<decltype(container)>
{
    return std::span<const char>(reinterpret_cast<const char*>(container.data()), container.size());
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