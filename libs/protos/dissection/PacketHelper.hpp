#pragma once

namespace protos::dissection::utils {
/* Experimental helpers. Not tested: I.E.: you're on your own here*/
// CAVEAT: not taking dependent fields into account
static size_t getPacketSize(const protos::dissector::PacketData& pd)
{
    return std::accumulate(pd.fields.begin(), pd.fields.end(), size_t{}, [](size_t acc, const auto& field) {
        return acc + field.value.size();
    });
}

static size_t getPacketSize(const protos::dissector::PacketDescriptor& pd)
{
    return std::accumulate(
        pd.fields.begin(), pd.fields.end(), size_t{}, [](size_t acc, const protos::dissector::FieldDescriptor& field) {
            return acc + field.size;
        });
}

static auto packetDataToBuffer(const protos::dissector::PacketData& pd) -> std::vector<std::byte>
{
    std::vector<std::byte> buffer;
    for (const auto& field : pd.fields)
    {
        if (field.value.empty()) { continue; }
        buffer.insert(buffer.end(), field.value.begin(), field.value.end());
    }
    return buffer;
}

static PacketData packetFromDescriptor(const PacketDescriptor& pd)
{
    PacketData packet;
    packet.name = pd.name;

    for (const auto& field : pd.fields)
    {
        packet.add(FieldData{field.name, field.value});
    }

    return packet;
}

static auto packetDescriptorToBuffer(const protos::dissector::PacketDescriptor& pd, bool swap_endian = false)
    -> std::vector<std::byte>
{
    std::vector<std::byte> buffer;
    for (const auto& field : pd.fields)
    {
        if (field.size == 0) { continue; }
        if (field.size != field.value.size())
        {
            throw std::runtime_error(fmt::format("Field {}::{} size mismatch. field size {} vs. field value size {}",
                                                 pd.name,
                                                 field.name,
                                                 field.size,
                                                 field.value.size()));
        }
        if (!swap_endian) { buffer.insert(buffer.end(), field.value.begin(), field.value.end()); }
        else { buffer.insert(buffer.end(), field.value.rbegin(), field.value.rend()); }
    }
    return buffer;
}

static std::vector<std::byte> packetDescriptorToBufferExcluding(const protos::dissector::PacketDescriptor& pd,
                                                                std::string excluded = {})
{
    std::vector<std::byte> buffer;
    for (const auto& field : pd.fields)
    {
        if (field.name == excluded) { continue; }
        if (field.size == 0) { continue; }
        buffer.insert(buffer.end(), field.value.begin(), field.value.end());
    }
    return buffer;
}

} // namespace protos::dissection