#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace protos::value {

using Variant = std::variant<std::string, int64_t, uint64_t, double, bool, std::vector<std::byte>>;

enum class Type
{
    UNDEFINED = 0,
    STRING,           // string representation of the bytes (as characters)
    INTEGER,          // integer representation of the bytes(signed based on the number of bytes. if odd, fill with 0s)
    UNSIGNED_INTEGER, // unsigned integer representation of the bytes (unsigned based on the number of bytes, if odd
    // fill with 0s)
    FLOAT,
    BYTES, // individual bytes
    BOOL,  // boolean representation of the bytes
};

template <typename VariantType, typename T, std::size_t index = 0>
constexpr std::size_t variant_index()
{
    static_assert(std::variant_size_v<VariantType> > index, "Type not found in variant");
    if constexpr (index == std::variant_size_v<VariantType>) { return index; }
    else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) { return index; }
    else { return variant_index<VariantType, T, index + 1>(); }
}

constexpr std::size_t type_index(Type t)
{
    if (t == Type::UNDEFINED)
        return -1;
    else if (t == Type::STRING)
        return variant_index<Variant, std::string>();
    else if (t == Type::INTEGER)
        return variant_index<Variant, int64_t>();
    else if (t == Type::UNSIGNED_INTEGER)
        return variant_index<Variant, uint64_t>();
    else if (t == Type::FLOAT)
        return variant_index<Variant, double>();
    else if (t == Type::BYTES)
        return variant_index<Variant, std::vector<std::byte>>();
    else if (t == Type::BOOL)
        return variant_index<Variant, bool>();
    else
        return 0;
}

template <Type T>
constexpr std::size_t type_index()
{
    if constexpr (T == Type::STRING)
        return variant_index<Variant, std::string>();
    else if constexpr (T == Type::INTEGER)
        return variant_index<Variant, int64_t>();
    else if constexpr (T == Type::UNSIGNED_INTEGER)
        return variant_index<Variant, uint64_t>();
    else if constexpr (T == Type::FLOAT)
        return variant_index<Variant, double>();
    else if constexpr (T == Type::BYTES)
        return variant_index<Variant, std::vector<std::byte>>();
    else if constexpr (T == Type::BOOL)
        return variant_index<Variant, bool>();
}

constexpr Type type_from_index(std::size_t index)
{
    if (index == variant_index<Variant, std::string>())
        return Type::STRING;
    else if (index == variant_index<Variant, int64_t>())
        return Type::INTEGER;
    else if (index == variant_index<Variant, uint64_t>())
        return Type::UNSIGNED_INTEGER;
    else if (index == variant_index<Variant, double>())
        return Type::FLOAT;
    else if (index == variant_index<Variant, std::vector<std::byte>>())
        return Type::BYTES;
    else if (index == variant_index<Variant, bool>())
        return Type::BOOL;
    else
        return Type::UNDEFINED;
}

template <typename T>
T get_variant_as(const Variant& v)
{
    if constexpr (std::is_same_v<T, std::string>)
        return std::get<std::string>(v);
    else if constexpr (std::is_same_v<T, int64_t>)
        return std::get<int64_t>(v);
    else if constexpr (std::is_same_v<T, uint64_t>)
        return std::get<uint64_t>(v);
    else if constexpr (std::is_same_v<T, double>)
        return std::get<double>(v);
    else if constexpr (std::is_same_v<T, std::vector<std::byte>>)
        return std::get<std::vector<std::byte>>(v);
    else if constexpr (std::is_same_v<T, bool>)
        return std::get<bool>(v);
}

namespace operators {
template <typename T>
inline bool operator==(const T& t, const protos::value::Variant& v)
{
    const T* c = std::get_if<T>(&v);

    return nullptr != c && *c == t; // true if v contains a T that compares equal to t
}

template <typename T>
inline bool operator==(const protos::value::Variant& v, const T& t)
{
    return t == v;
}
} // namespace operators
} // namespace protos::value
