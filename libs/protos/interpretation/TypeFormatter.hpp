#pragma once
#include <protos/interpretation/GenericTypes.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
//#include <datafw/utils/Hashers.hpp>

#include <format>

#include <vector>
#include <cstddef>
#include <cstdint>
#include <variant>

namespace datafw::value {

template <typename T>
struct always_false : std::false_type
{
};

template <Type Type>
inline auto convert(const std::vector<std::byte>& data)
{
    // TODO: doesn't compile
    // static_assert(false, "Type not supported");
}

template <>
inline auto convert<Type::INTEGER>(const std::vector<std::byte>& data)
{
    if (data.size() == 1) { return static_cast<int64_t>(*std::bit_cast<const int8_t*>(data.data())); }
    if (data.size() == 2) { return static_cast<int64_t>(*std::bit_cast<const int16_t*>(data.data())); }
    if (data.size() == 4) { return static_cast<int64_t>(*std::bit_cast<const int32_t*>(data.data())); }
    if (data.size() == 8) { return static_cast<int64_t>(*std::bit_cast<const int64_t*>(data.data())); }
    return std::numeric_limits<int64_t>::quiet_NaN();
}

template <>
inline auto convert<Type::UNSIGNED_INTEGER>(const std::vector<std::byte>& data)
{
    if (data.size() == 1) { return static_cast<uint64_t>(*std::bit_cast<const uint8_t*>(data.data())); }
    if (data.size() == 2) { return static_cast<uint64_t>(*std::bit_cast<const uint16_t*>(data.data())); }
    if (data.size() == 4) { return static_cast<uint64_t>(*std::bit_cast<const uint32_t*>(data.data())); }
    if (data.size() == 8) { return static_cast<uint64_t>(*std::bit_cast<const uint64_t*>(data.data())); }
    return std::numeric_limits<uint64_t>::quiet_NaN();
}

template <>
inline auto convert<Type::FLOAT>(const std::vector<std::byte>& data)
{
    if (data.size() == 4) { return static_cast<double>(*std::bit_cast<const float*>(data.data())); }
    if (data.size() == 8) { return static_cast<double>(*std::bit_cast<const double*>(data.data())); }
    return std::numeric_limits<double>::quiet_NaN();
}

template <>
inline auto convert<Type::BOOL>(const std::vector<std::byte>& data)
{
    return std::bit_cast<const bool>(*data.data());
}

template <>
inline auto convert<Type::STRING>(const std::vector<std::byte>& data)
{
    return std::string{std::bit_cast<const char*>(data.data()), data.size()};
}

template <>
inline auto convert<Type::BYTES>(const std::vector<std::byte>& data)
{
    // TODO: avoid copy
    return data;
}

// TODO: Make Variant a full blown type and add implicit conversion
/*
template <typename T>
  operator T () const
   {
     return std::visit(
        [](auto const & val)
        { if constexpr ( std::is_convertible_v<decltype(val), T> )
             return T(val);
           else
            { throw std::bad_variant_access{}; return T{}; } }, var);
   }
   see:
https://stackoverflow.com/questions/67348379/how-to-support-implicit-conversion-from-a-variant-type-e-g-from-int-to-unsigne
*/
inline Variant as_variant(Type type, const std::vector<std::byte>& data)
{
    switch (type)
    {
    case Type::STRING:
        return convert<Type::STRING>(data);
        break;
    case Type::INTEGER:
        return convert<Type::INTEGER>(data);
        break;
    case Type::UNSIGNED_INTEGER:
        return convert<Type::UNSIGNED_INTEGER>(data);
        break;
    case Type::FLOAT:
        return convert<Type::FLOAT>(data);
        break;
    case Type::BOOL:
        return convert<Type::BOOL>(data);
        break;
    case Type::BYTES:
        return convert<Type::BYTES>(data);
        break;
    default:
        return std::string{"Unsupported type"};
        break;
    }
}

// TODO: tests
inline Variant variant_from_string(Type type, const std::string& data)
{
    // TODO use from_string
    switch (type)
    {
    case Type::STRING:
        return data;
        break;
    case Type::INTEGER:
        return std::stoll(data, nullptr, 0);
        break;
    case Type::UNSIGNED_INTEGER:
        return std::stoull(data, nullptr, 0);
        break;
    case Type::FLOAT:
        return std::stod(data);
        break;
    case Type::BOOL:
        return data == "true";
        break;
    case Type::BYTES:
        return data;
        break;
    default:
        return std::string{"Unsupported type"};
        break;
    }
}

inline constexpr std::string variant_to_formatted_string(const Variant& v, const std::string& format_str)
{
    return std::visit(
        [&](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            // as string
            if constexpr (std::is_same_v<T, std::string>) { return std::vformat(format_str, std::make_format_args(arg)); }
            // as integer
            else if constexpr (std::is_same_v<T, int64_t>) { return std::vformat(format_str, std::make_format_args(arg)); }
            // as unsigned integer
            else if constexpr (std::is_same_v<T, uint64_t>) { return std::vformat(format_str, std::make_format_args(arg)); }
            // as float
            else if constexpr (std::is_same_v<T, double>) { return std::vformat(format_str, std::make_format_args(arg)); }
            // as bool
            else if constexpr (std::is_same_v<T, bool>) { return std::vformat(format_str, std::make_format_args(arg)); }
            // as bytes
            else if constexpr (std::is_same_v<T, std::vector<std::byte>>)
            {
                throw std::runtime_error("Not implemented");
                //return std::format(format_str, fmt::join(arg, " "));
            }

            else { static_assert(always_false<T>::value, "non-exhaustive visitor!"); }
        },
        v);
}

inline constexpr std::string variant_to_string(const Variant& v)
{
    return std::visit(
        [](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            // as string
            if constexpr (std::is_same_v<T, std::string>) { return arg; }
            // as integer
            else if constexpr (std::is_same_v<T, int64_t>) { return std::format("{}", arg); }
            // as unsigned integer
            else if constexpr (std::is_same_v<T, uint64_t>) { return std::format("{}", arg); }
            // as float
            else if constexpr (std::is_same_v<T, double>) { return std::format("{}", arg); }
            // as bool
            else if constexpr (std::is_same_v<T, bool>) { return arg ? "true" : "false"; }
            // as bytes
            else if constexpr (std::is_same_v<T, std::vector<std::byte>>)
            {
                throw std::runtime_error("Not implemented");
                //return std::format("{}", fmt::join(arg, " "));
            }

            else { static_assert(always_false<T>::value, "non-exhaustive visitor!"); }
        },
        v);
}

template <typename T>
inline const T& variant_as(const Variant& v)
{
    return std::get<T>(v);
}

template <Type type>
struct TypeTag
{
    // clang-format off
    using type_t = //
        std::conditional_t<type == Type::STRING, std::string, //
        std::conditional_t<type == Type::INTEGER, int64_t, //
        std::conditional_t<type == Type::UNSIGNED_INTEGER, uint64_t,//
        std::conditional_t<type == Type::FLOAT, double,
        std::conditional_t<type == Type::BOOL, bool,
        std::conditional_t<type == Type::BYTES, std::vector<std::byte>, void>>>>>>;

    // clang-format on

    static type_t get(Variant v) { return std::get<type_t>(v); }
};

} // namespace datafw::dissector