#pragma once
#include <protos/interpretation/FieldInterpretation.hpp>
#include <protos/interpretation/InterpretationResult.hpp>
#include <protos/interpretation/GenericTypes.hpp>
#include <protos/interpretation/TypeFormatter.hpp>

#include <vector>
#include <string>
#include <optional>
#include <cstddef>
#include <ranges>
#include <algorithm>

namespace protos::interpreter {

struct FieldInterpretation;

class FieldInterpreter
{
public:
    static std::vector<std::string> toStringResult(const protos::interpreter::InterpretationResults& result)
    {
        std::vector<std::string> res;
        std::ranges::transform(result, std::back_inserter(res), [](const auto& r) {
            return protos::value::variant_to_formatted_string(r.value, "{}");
        });
        return res;
    }

    static protos::interpreter::InterpretationResults interpret(
        const std::string&                                name,
        const std::vector<std::byte>&                     data,
        const protos::interpreter::FieldInterpretation&   interpretation,
        const protos::interpreter::InterpretationResults& context);
    /**
     * @brief: Checks if the field can be interpreted safely.This encompasses checking if the field is of the correct
     * size for the type and if the format string is valid
     * @return std::optional<std::string> - empty if the field is valid, otherwise a string describing the error
     */
    // std::optional<std::string> validate() const;
};

} // namespace protos::interpreter