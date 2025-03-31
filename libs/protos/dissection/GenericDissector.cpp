#include "GenericDissector.hpp"
#include "PacketData.hpp"
#include <protos/common/BitUtils.hpp>

#include <algorithm>
#include <map>
#include <set>

void protos::dissector::GenericDissector::newPacket()
{
    current_packet = protos::PacketData{};
    makePacketStack();
}

protos::FieldDescriptor& protos::dissector::GenericDissector::getFieldInStack(std::string_view name)
{
    auto iter = std::ranges::find_if(current_packet_stack,
                                     [&name](const protos::FieldDescriptor& f) { return f.name == name; });
    if (iter == current_packet_stack.end()) { throw std::runtime_error("Field not found"); }
    return *iter;
}

void protos::dissector::GenericDissector::makePacketStack()
{
    // build stack from protocol description by adding all field of the protocol using ranges
    for (auto& field : packet_template.fields | std::ranges::views::reverse)
    {
        current_packet_stack.push_front(field);
    }
}

std::optional<protos::PacketData> protos::dissector::GenericDissector::addByte(const std::byte b)
{
    // check if we need to start a new packet
    if (stackIsEmpty() && current_message_buffer.empty()) { newPacket(); }

    current_message_buffer.push_back(b);
    // don't process empty fields
    if (!stackIsEmpty() && 0 == stackTop().size)
    {
        // add the empty field to the packet
        current_packet.fields.push_back(protos::FieldData{stackTop().name, {}});
        // skip empty
        current_packet_stack.pop_front();
    }
    // check if we have a complete field
    if (!stackIsEmpty() && current_message_buffer.size() == stackTop().size)
    {
        // check if we have a complete header
        if (stackTop().isHeaderValue)
        {
            // still waiting for a complete header?
            if (stackTop().value != current_message_buffer)
            {
                // remove front element from current_message_buffer
                // still waiting for the correct sequence
                current_message_buffer.erase(current_message_buffer.begin());
                return std::nullopt;
            }
        }

        // we have a complete field

        const auto field = stackTop();
        auto       field_data = protos::FieldData{field.name, current_message_buffer};
        current_packet_stack.pop_front();
        current_packet.fields.push_back(field_data);
        current_message_buffer.clear();

        if (!field.determinesSizeOf.empty())
        {
            auto& ref_field = getFieldInStack(field.determinesSizeOf);
            ref_field.size = getSizeFromFieldValue(field) + field.sizeOffset;
        }

        if (!field.sizeDeterminesExistenceOf.empty())
        {
            if (0 == getSizeFromFieldValue(field))
            {
                removeFieldFromStack(field.sizeDeterminesExistenceOf);
                // getFieldInStack(field.sizeDeterminesExistenceOf).size = 0;
            }
        }
        // while top is a zero size field, consider it handled
        while (!stackIsEmpty() && 0 == stackTop().size)
        {
            auto field_data = protos::FieldData{stackTop().name, {}}; // empty field
            current_packet.fields.push_back(protos::FieldData{stackTop().name, {}});
            current_packet_stack.pop_front();
        }
    }

    // check if we have a complete packet
    if (stackIsEmpty())
    {
        current_message_buffer.clear();

        return current_packet;
    }
    else { return std::nullopt; }
}

protos::dissector::GenericDissector::GenericDissector(const protos::PacketDescriptor& packet_template)
  : packet_template(packet_template)
{
    newPacket();
}

protos::dissector::GenericDissector::GenericDissector(protos::PacketDescriptor&& packet_template)
  : packet_template(std::move(packet_template))
{
    newPacket();
}

bool protos::dissector::GenericDissector::stackIsEmpty() const
{
    return current_packet_stack.empty();
}

uint32_t protos::dissector::GenericDissector::getSizeFromFieldValue(const protos::FieldDescriptor& field)
{
    //  TODO byte is only a placeholder, we need to convert it to the correct type based on the number of bytes
    switch (field.size)
    {
    case 1:
        return protos::bytes::as_number<uint8_t>(current_packet.get(field.name).value);
    case 2:
        return protos::bytes::as_number<uint16_t>(current_packet.get(field.name).value);
    case 4:
        return protos::bytes::as_number<uint32_t>(current_packet.get(field.name).value);
    case 8:
        return protos::bytes::as_number<uint64_t>(current_packet.get(field.name).value);
    default:
        throw std::runtime_error(
            std::format("Unsupported size for field describing a size in field '{}' of GenericDissector '{}' ",
                        field.name,
                        this->packet_template.getName()));
    }
}

bool protos::dissector::GenericDissector::matchesHeader(const std::vector<std::byte>& header) const
{
    auto top = packet_template.fields.front();
    if (header.size() < top.size) { return false; }

    if (top.isHeaderValue)
    {
        // get a span of the header with the same size as the header
        auto header_span = std::vector(header.begin(), header.begin() + top.size);
        return header_span == top.value;
    }
    return false;
}

std::optional<protos::PacketData> protos::dissector::GenericDissector::addBytes(std::span<const std::byte> bytes)
{
    // check if all fields are fixed size (.e.g. determinesSizeOf is empty)
    auto fixed_size = (std::ranges::all_of(
        packet_template.fields, [](const protos::FieldDescriptor& f) { return f.determinesSizeOf.empty(); }));

    if (true)
    {
        auto current_data_index = 0;
        // if all fields are fixed size, we can process the bytes in one go
        auto                            packet = protos::PacketData{packet_template.getName()};
        std::map<std::string, uint32_t> field_sizes;
        std::set<std::string>           removed_fields;

        for (const auto& field : packet_template.fields)
        {
            auto field_size = field.size;
            // check if we have a size for the field determined by another field
            if (field_sizes.contains(field.name)) { field_size = field_sizes[field.name]; }
            // if the field determines the existence of another field, we need to set the size to 0

            if (field_size == 0)
            {
                packet.add(protos::FieldData{field.name, {}});
                continue;
            }
            if (removed_fields.contains(field.name))
            {
                //
                continue;
            }

            auto current_start = bytes.begin() + current_data_index;
            auto current_end = bytes.begin() + current_data_index + field_size;
            current_data_index += field_size;

            auto field_data = protos::FieldData{field.name, std::vector<std::byte>(current_start, current_end)};
            packet.add(field_data);
            // remember the size of the field if it determines the size of another field
            if (!field.determinesSizeOf.empty()) { field_sizes[field.determinesSizeOf] = getSizeFromFieldValue(field); }

            if (!field.sizeDeterminesExistenceOf.empty() && getSizeFromFieldValue(field) == 0)
            {
                removed_fields.insert(field.sizeDeterminesExistenceOf);
            }
        }
        return packet;
    }
    for (const auto& b : bytes)
    {
        auto packet = addByte(b);
        if (packet.has_value()) { return packet; }
    }
    return std::nullopt;
}

void protos::dissector::GenericDissector::removeFieldFromStack(std::string_view name)
{
    auto iter = std::ranges::find_if(current_packet_stack,
                                     [&name](const protos::FieldDescriptor& f) { return f.name == name; });
    if (iter == current_packet_stack.end()) { throw std::runtime_error("Field not found"); }
    current_packet_stack.erase(iter);
}
