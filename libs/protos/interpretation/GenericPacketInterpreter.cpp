#include "GenericPacketInterpreter.hpp"
#include "TypeFormatter.hpp"
#include "FieldInterpretation.hpp"
#include "FieldInterpreter.hpp"
#include <common/Once.hpp>

// #include <datafw/utils/once.hpp>
// #include <fmt/format.h>
// #include <magic_enum.hpp>
// #include <spdlog/spdlog.h>

bool protos::interpreter::GenericPacketInterpreter::hasField(const std::string& name) const
{
    return fields.find(name) != fields.end();
}

const protos::interpreter::FieldInterpretation& protos::interpreter::GenericPacketInterpreter::findInterpreterForField(
    const std::string& name) const
{
    auto it = fields.find(name);
    if (it != fields.end()) { return (*it).second; }
    else
    {
        throw std::runtime_error(
            std::format("Interpreter for field {} not found in GenericPacketInterpreter {}", name, getName()));
    };
}

void protos::interpreter::GenericPacketInterpreter::addField(FieldInterpretation&& fieldInterpreter)
{
    fields.emplace(fieldInterpreter.name, std::move(fieldInterpreter));
}

protos::interpreter::InterpretationResults protos::interpreter::GenericPacketInterpreter::interpretPacketData(
    const protos::dissector::PacketData& data) const
{
    protos::interpreter::InterpretationResults result;
    for (const auto& field : data.fields)
    {
        const auto has_field = hasField(field.name);
        if (!has_field) { handleMissingField(field.name, result); }

        else
        {
            auto& interpreter = findInterpreterForField(field.name);
            auto  res = protos::interpreter::FieldInterpreter::interpret(field.name, field.value, interpreter, result);
            result.insert(result.end(), res.begin(), res.end());
        }
    }

    return result;
}

// void protos::interpreter::GenericPacketInterpreter::handlePacketData(const protos::dissector::PacketData& data) const
// {
//     if (interpretationResultsCallback) { interpretationResultsCallback(interpretPacketData(data)); }
//     else
//     {
//         throw std::runtime_error(
//             std::format("No callback set for interpretation results in GenericPacketInterpreter {}", getName()));
//     }
// }

void protos::interpreter::GenericPacketInterpreter::handleMissingField(
    const std::string&                       name,
    protos::interpreter::InterpretationResults& intpretation_result) const
{
    switch (behaviors.emptyFieldBehavior)
    {
    default:
        [[fallthrough]];
    case EmptyFieldBehavior::SKIP:
        break;
    case EmptyFieldBehavior::SKIP_AND_WARN_ONCE:
        protos::utils::once(name + getName(), [&]() {
            throw std::runtime_error("decide if we use logger here");
            // spdlog::warn("Field '{}' not found in GenericPacketInterpreter '{}'", name, getName());
        });
        break;
    case EmptyFieldBehavior::EMPTY_STRING:
        intpretation_result.push_back({name, protos::value::Variant{""}});
        break;
    case EmptyFieldBehavior::DASH:
        intpretation_result.push_back({name, protos::value::Variant{"-"}});
        break;
    }
}
