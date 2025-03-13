#pragma once
#include <span>

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
} // namespace protos::bytes