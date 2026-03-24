#pragma once
#include <protos/interpretation/GenericTypes.hpp>
#include <protos/interpretation/InterpretationResult.hpp>
#include <string>
#include <functional>


namespace protos::interpreter {

/**
 * Interprets a field of a packet from a collection of bytes into a human-readable format
 */
struct FieldInterpretation
{
public:
    using Mapper = std::function<protos::interpreter::InterpretationResults(const protos::value::Variant&)>;
    std::string         name;                                 // name of the field as per metadata
    protos::value::Type type{protos::value::Type::UNDEFINED}; // type of the field
    std::string         format = "{}";                        // std/fmt::format string
    Mapper              mapper;                               // function to map the field value

    std::vector<FieldInterpretation> subFields; // subfields of this field
    // std::string                      dissector{}; // name of a dissector to parse a sub-protocol in this field
    //  properties
    bool littleEndian{true}; // if true, the field is interpreted as little-endian, otherwise big-endian
    bool hidden{false};      // if true, the field is not displayed in the output
};

} // namespace protos::interpreter