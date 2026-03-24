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
    /**
     * When an empty field description is encountered, this behavior determines how it is handled.
     */

    enum EmptyFieldBehavior
    {
        SKIP,               ///< Skip the field silently
        SKIP_AND_WARN_ONCE, ///< Skip the field and log a warning once
        EMPTY_STRING,       ///< Set the field to an empty string
        DASH,               ///< Set the field to a dash '-'
        TROW_ONCE,          ///< Throw an exception once
        CALL_BACK           ///< Call the callback function
    };

    struct Behaviors
    {
        // callback function for an empty field descriptor
        using EmptyFieldCallback = std::function<void(const std::string& fieldName)>;

        EmptyFieldBehavior emptyFieldBehavior{EmptyFieldBehavior::SKIP_AND_WARN_ONCE};
        EmptyFieldCallback emptyFieldCallback{nullptr};
    };

    void setBehaviors(const Behaviors& behaviors) { this->behaviors = behaviors; }

    protos::interpreter::InterpretationResults interpretPacketData(const protos::dissector::PacketData& data) const;

    void addField(FieldInterpretation&& fieldInterpreter);

    bool hasField(const std::string& name) const;

    void handleMissingField(const std::string&                          name,
                            protos::interpreter::InterpretationResults& intpretation_result) const;

    const FieldInterpretation& findInterpreterForField(const std::string& name) const;

    const std::string& getName() const { return name; }

    std::unordered_map<std::string, FieldInterpretation> fields;
    Behaviors                                            behaviors;
    std::string                                          name;
};

} // namespace protos::interpreter