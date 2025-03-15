#pragma once
#include <protos/PacketDescriptor.hpp>
#include <protos/PacketData.hpp>

#include <functional>
#include <ranges>
#include <stack>
#include <string>
#include <vector>
#include <numeric>
#include <optional>
#include <iostream>
#include <stdexcept>

namespace protos::dissector {
// dissect a byte stream following a packet description
class GenericDissector 
{
public:
    GenericDissector(const protos::PacketDescriptor& packet_template);
    GenericDissector(protos::PacketDescriptor&& packet_template);

    // process the next byte to build the packet. If the packet is complete the function returns the packet
    
    
    std::optional<protos::PacketData>  addByte(const std::byte b);
    // check if a potential header in the PacketDescriptor matches the given header.
    bool matchesHeader(const std::vector<std::byte>& header) const;
protected:
    

    void makePacketStack();
    void newPacket();
    bool stackIsEmpty() const;

    protos::FieldDescriptor& getFieldInStack(std::string_view name);

    protos::FieldDescriptor& stackTop() { return current_packet_stack.front(); }

    const protos::FieldDescriptor& stackTop() const { return current_packet_stack.front(); }

    uint32_t getSizeFromFieldValue(const protos::FieldDescriptor& field);

private:
    std::deque<protos::FieldDescriptor> current_packet_stack;
    std::vector<std::byte>                        current_message_buffer;
    protos::PacketData                  current_packet;  // the packet currently being built
    const protos::PacketDescriptor      packet_template; // the packet template
};

} // namespace protos::dissector