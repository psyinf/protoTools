#include "FieldInterpreter.hpp"

#include "TypeFormatter.hpp"
#include "FieldInterpretation.hpp"
#include <protos/dissection/GenericDissector.hpp>

#include <bit>

// #include <datafw/utils/BitConversions.hpp>
// #include <datafw/utils/Once.hpp>
// #include <datafw/registries/Registry.hpp>
// #include <datafw/functional/cparse_funcs.hpp>
// #include <datafw/protocols/detail/Placeholder.hpp>
// #include <fmt/format.h>
// #include <magic_enum.hpp>

/*
 * Extract a value from the current InterpretationResults/Context
 */
std::string getFieldValueAsString(std::string_view name, const protos::interpreter::InterpretationResults& context)
{
    auto it = std::find_if(context.begin(), context.end(), [&](const auto& field) { return field.name == name; });
    if (it == context.end()) { throw std::runtime_error(std::format("Field {} referenced was not found", name)); }
    return protos::value::variant_to_string(it->value);
}
#ifdef EXTENDED_INTERPRETER
/*
The data of a field is essentially handled as a separate protocol.
This means that the field has a dissector and interpreter
*/
std::optional<protos::interpreter::InterpretationResults> delegateToDissector(
    const std::vector<std::byte>&                     data,
    const std::string&                                name,
    const protos::interpreter::InterpretationResults& context)
{
    std::string dissector_name = datafw::detail::placeholders::replaceAllTokens(
        name, [&](std::string_view token) { return getFieldValueAsString(token, context); });

    using namespace magic_enum::bitwise_operators;
    using namespace datafw;
    using namespace datafw::protocol;
    using namespace datafw::dissector;
    using namespace datafw::interfaces;
    auto result = protos::interpreter::InterpretationResults{};
    if (!(getRegistry<datafw::interfaces::PacketInterpreterInterface>().has(dissector_name) &&
          getRegistry<PacketDescriptor>().has(dissector_name)))
    {
        datafw::utils::once(dissector_name,
                            [&]() { spdlog::debug("Dissector {} not found in registry", dissector_name); });
        // TODO: global callback to collect the missing dissectors
        return std::nullopt;
    }
    // there must be a packet descriptor with the interpretation's id
    auto& packet_descriptor = getRegistry<PacketDescriptor>().getRef(dissector_name);
    auto& sub_interpreter = getRegistry<PacketInterpreterInterface>().getRef(dissector_name);
    sub_interpreter.setInterpretationResultsCallback(
        [&](auto& res) { result.insert(result.end(), res.begin(), res.end()); });
    GenericDissector dissector(packet_descriptor);

    dissector.setPacketDataCallback([&](auto& packet) { sub_interpreter.handlePacketData(packet); });
    dissector.addBytes(data);

    return std::optional<protos::interpreter::InterpretationResults>{result};
}
#endif
/*
 * The data of a can be used to generate subfields.
 */
protos::interpreter::InterpretationResults handleSubFields(
    const std::string&                                fieldName,
    const std::vector<std::byte>&                     data,
    const protos::interpreter::FieldInterpretation&   interpretation,
    const protos::interpreter::InterpretationResults& context)
{
    auto internal_context = protos::interpreter::InterpretationResults{context};
    auto result = std::vector<protos::interpreter::InterpretationResult>{};

    for (const auto& subfield : interpretation.subFields)
    {
        auto res = (protos::interpreter::FieldInterpreter::interpret(subfield.name, data, subfield, internal_context));
        internal_context.insert(internal_context.end(), res.begin(), res.end());
        result.insert(result.end(), res.begin(), res.end());
    }
    return result;
}
#ifdef EXTENDED_INTERPRETER
TODO : use a callback an external function to handle the expression
           /*
            * The data of a field can be used to generate a value using a function
            * Caveat: Currently the input and output type must be the same.
            */

           protos::value::Variant
           handleFunction(const std::vector<std::byte>&                     data,
                          const protos::interpreter::FieldInterpretation&   interpretation,
                          const protos::interpreter::InterpretationResults& context)
{
    std::string func_str = datafw::detail::placeholders::replaceAllTokens(
        interpretation.function, [&](std::string_view token) { return getFieldValueAsString(token, context); });
    using namespace datafw::protocol;
    using namespace protos::value;
    auto value = protos::value::as_variant(interpretation.type, data);

    using namespace cparse;
    TokenMap vars;

    switch (interpretation.type)
    {
    case Type::INTEGER:
        vars["this"] = TypeTag<Type::INTEGER>::get(value);
        return (calculator::calculate(func_str.data(), &vars).asInt());
        break;
    case Type::UNSIGNED_INTEGER:
        vars["this"] = TypeTag<Type::UNSIGNED_INTEGER>::get(value);
        return (calculator::calculate(func_str.data(), &vars).asInt());
        break;
    case Type::FLOAT:
        vars["this"] = TypeTag<Type::FLOAT>::get(value);
        return (calculator::calculate(func_str.data(), &vars).asDouble());
        break;
    case Type::BOOL:
        vars["this"] = TypeTag<Type::BOOL>::get(value);
        return (calculator::calculate(func_str.data(), &vars).asBool());
        break;
    case Type::STRING:
        vars["this"] = TypeTag<Type::STRING>::get(value);
        return (calculator::calculate(func_str.data(), &vars).asString());
        break;
    case Type::BYTES:
    default:
        throw std::runtime_error("Subfield type (Bytes, ...) not supported");
        break;
    }
}
#endif
protos::interpreter::InterpretationResults protos::interpreter::FieldInterpreter::interpret(
    const std::string&                                name,
    const std::vector<std::byte>&                     data,
    const protos::interpreter::FieldInterpretation&   interpretation,
    const protos::interpreter::InterpretationResults& context)
{
    // using namespace datafw::protocol;

    // subfields are handled separately
    if (!interpretation.subFields.empty()) { return handleSubFields(name, data, interpretation, context); }

    // else we do the following data flow order: function -> mapper -> type
    // where function and mapper are optional. If no function or mapper is present, the type is used
    // if the function is empty, the input is the type of the field as per description
    auto result = protos::value::Variant{};

    // Honor FieldInterpretation::littleEndian for numeric types. STRING and BYTES
    // keep stream order; BOOL is single-byte so the flag is a no-op there.
    // as_variant uses std::bit_cast under the hood, which is host-order. If the
    // declared wire endianness disagrees with the host, reverse the bytes first.
    using protos::value::Type;
    const auto t            = interpretation.type;
    const bool numeric      = (t == Type::INTEGER || t == Type::UNSIGNED_INTEGER || t == Type::FLOAT);
    const bool host_le      = (std::endian::native == std::endian::little);
    const bool needs_swap   = numeric && (interpretation.littleEndian != host_le);
    if (needs_swap)
    {
        std::vector<std::byte> reversed(data.rbegin(), data.rend());
        result = protos::value::as_variant(t, reversed);
    }
    else { result = protos::value::as_variant(t, data); }

    // map the result. Mappers allow for type conversion
    if (interpretation.mapper)
    { // TODO: if no function is used, the type can be derived from the mapper source type
        return interpretation.mapper(result);
    }
    return {{name, result, interpretation.format, interpretation.hidden}};
}

/*
template <typename T>
std::optional<std::string> verifyTypeFormatter(std::string_view format)
{
    try
    {
        [[maybe_unused]] auto result = fmt::format(fmt::runtime(format), T{});
    }
    catch (const fmt::format_error& e)
    {
        return e.what();
    }
    return std::nullopt;
}
*/
/*
std::optional<std::string> protos::interpreter::FieldInterpretation::validate() const
{
    std::string result;
    // for each field try the format and see if it throws an exception with a default value for the type
    switch (type)
    {
    case Type::STRING:
        result += verifyTypeFormatter<std::string>(format).value_or(std::string{});
        break;
    case Type::INTEGER:
        result += verifyTypeFormatter<int64_t>(format).value_or(std::string{});
        break;
    case Type::UNSIGNED_INTEGER:
        result += verifyTypeFormatter<uint64_t>(format).value_or(std::string{});
        break;
    case Type::FLOAT:
        result += verifyTypeFormatter<double>(format).value_or(std::string{});
        break;
    case Type::BOOL:
        result += verifyTypeFormatter<bool>(format).value_or(std::string{});
        break;
    default:
        [[fallthrough]];
    case Type::BYTES:
        result += verifyTypeFormatter<std::byte>(format).value_or(std::string{});
        break;
    }
    return result.empty() ? std::nullopt : std::optional<std::string>{result};
}
*/