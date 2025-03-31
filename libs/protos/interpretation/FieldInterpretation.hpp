#pragma once
#include <protos/interpretation/GenericTypes.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace protos::interpreter {

/**
 * Interprets a field of a packet from a collection of bytes into a human-readable format
 */
struct FieldInterpretation
{
public:
    std::string                      name;                                 // name of the field as per metadata
    datafw::value::Type              type{datafw::value::Type::UNDEFINED}; // type of the field
    std::string                      format = "{}";                        // std/fmt::format string
    std::string                      mapper{};                             // name of a mapper to use
    std::vector<FieldInterpretation> subFields;                            // subfields of this field
    std::string                      dissector{}; // name of a dissector to parse a sub-protocol in this field
    std::string                      function{};  // a function such as ("this & 0x0F << 4)
    // properties
    bool littleEndian{true}; // if true, the field is interpreted as little-endian, otherwise big-endian
    bool hidden{false};      // if true, the field is not displayed in the output
};

} // namespace datafw::dissector