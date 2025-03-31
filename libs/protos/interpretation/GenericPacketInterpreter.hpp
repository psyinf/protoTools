#pragma once

#include <protos/interpretation/FieldInterpretation.hpp>
#include <protos/interpretation/InterpretationResult.hpp>
#include <protos/dissection/PacketData.hpp>
#include <unordered_map>

namespace protos::interpreter {

/**
 * Interprets a packet from a collection of bytes (PacketData, usually from a dissector) into InterpretationResults
 */
class GenericPacketInterpreter 
{
public:
    enum EmptyFieldBehavior
    {
        SKIP,
        SKIP_AND_WARN_ONCE,
        EMPTY_STRING,
        DASH
    };

    struct Behaviors
    {
        EmptyFieldBehavior emptyFieldBehavior{EmptyFieldBehavior::SKIP_AND_WARN_ONCE};
    };

    void setBehaviors(const Behaviors& behaviors) { this->behaviors = behaviors; }

    protos::interpreter::InterpretationResults interpretPacketData(const protos::dissector::PacketData& data) const;

    void addField(FieldInterpretation&& fieldInterpreter);

    bool hasField(const std::string& name) const;

    void handleMissingField(const std::string&                       name,
                            protos::interpreter::InterpretationResults& intpretation_result) const;

    const FieldInterpretation& findInterpreterForField(const std::string& name) const;

    const std::string& getName() const { return name; }

    std::unordered_map<std::string, FieldInterpretation> fields;
    Behaviors                                            behaviors;
    std::string                                          name;
};

} // namespace datafw::dissector