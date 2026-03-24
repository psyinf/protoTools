#pragma once
#include <string>

struct ProtocolDirectoryEntry
{
    std::string protocol_name;
    std::string adapter_descriptor;
    std::string publisher_endpoint;
    std::string command_endpoint;

    friend bool operator==(const ProtocolDirectoryEntry& lhs, const ProtocolDirectoryEntry& rhs)
    {
        return lhs.protocol_name == rhs.protocol_name && lhs.adapter_descriptor == rhs.adapter_descriptor &&
               lhs.publisher_endpoint == rhs.publisher_endpoint && lhs.command_endpoint == rhs.command_endpoint;
    }
};
