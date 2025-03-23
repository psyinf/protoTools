#pragma once
#include <string>

struct ProtocolDirectoryEntry
{
    std::string protocol_name;
    std::string adapter_descriptor;
    std::string publisher_endpoint;
    std::string command_endpoint;
};
