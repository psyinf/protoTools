#pragma once

#include <string>
#include <vector>

struct ProtoHeader
{
    // always sent as first message, fixed format. It is at users discretion to keep the data reasonable short
    // The format is:
    // protocol_name | [adapter_descriptor]
    // the protocol is mandatory, the adapter is optional and may be left out. If it is left out, the | is also left out
    // e.g. the following header is valid: "CAN|USB-CAN
    std::string protocol_name;      ///< Name of the protocol, e.g. CAN
    std::string adapter_descriptor; ///< Descriptor of the adapter, e.g. USB-CANmodul
};

struct SubscriberCommand
{
    std::string command_verb;     ///< Verb of the command, e.g. "send_to"
    std::string command_receiver; ///< Receiver of the command, e.g. "CAN"
    std::string command_data;     ///< Data of the command
};

struct ProtoData
{
    std::vector<char> data;
};

struct ProtoPackage
{
    ProtoHeader header;
    ProtoData   data;
};
