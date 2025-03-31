#pragma once
#include <protos/interpretation/GenericTypes.hpp>

#include <string>
#include <vector>

namespace protos::interpreter {
struct NamedValue
{
    std::string            name;
    datafw::value::Variant value;
};

struct InterpretationResult : NamedValue
{
    using Format = std::string;

    Format format{"{}"};
    bool   hidden{false}; // TODO: flags
};

using InterpretationResults = std::vector<InterpretationResult>;

} // namespace protos::interpreter