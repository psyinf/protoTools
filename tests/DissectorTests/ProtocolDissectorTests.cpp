#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "ProtocolDissectorTestsFunctions.hpp"

TEST_CASE("simple struct", "[ProtocolDissector]")
{
    using namespace protos::bytes;
    using namespace std::string_literals;
    // packet descriptor
    PacketDescriptor packet_template;
    packet_template.fields.push_back(FieldDescriptor{"header", "Header field", 16});
    packet_template.fields.push_back(FieldDescriptor{"x", "pos.x", sizeof(double)});
    packet_template.fields.push_back(FieldDescriptor{"y", "pos.y", sizeof(double)});
    // the described packet as struct
    SimpleTestPacket packet{"HELLO, WORLD!", 3.14, -2.71};
    auto             bytes = std::as_bytes(std::span{&packet, 1});

    GenericDissector dissector{packet_template};

    auto res = test_dissect(dissector, bytes);

    auto header_val = as_chars_span(res->get("header").value);
    auto x_val = protos::bytes::to_number<double>(res->get("x").value);
    auto y_val = protos::bytes::to_number<double>(res->get("y").value);

    REQUIRE_THAT(std::string(header_val.data(), header_val.size()), Equals("HELLO, WORLD!\0\0\0"s));
    REQUIRE(x_val == 3.14);
    REQUIRE(y_val == -2.71);
}

TEST_CASE("dependent size struct", "[ProtocolDissector]")
{
    // packet descriptor
    PacketDescriptor packet_template;
    packet_template.add({.name{"SIZE"}, .size{1}, .determinesSizeOf{"DATA"}});
    packet_template.add({.name{"DATA"}, .size{0}});
    packet_template.add({.name{"FLOAT"}, .size{sizeof(float)}});
    // the described packet as struct
    DependentSizePacket<3> packet{.data{to_array_wo_null("123")}};
    GenericDissector       dissector{packet_template};

    auto bytes = std::as_bytes(std::span{&packet, 1});

    float f = 3.0f;
    auto  res = test_dissect(dissector, bytes);
    REQUIRE(res->get("SIZE").value[0] == std::byte(3));
    REQUIRE(res->get("DATA").value.size() == 3);
    REQUIRE(res->get("DATA").value[0] == std::byte('1'));
    REQUIRE(res->get("DATA").value[1] == std::byte('2'));
    REQUIRE(res->get("DATA").value[2] == std::byte('3'));
    REQUIRE(res->get("FLOAT").value.size() == sizeof(float));
    REQUIRE(protos::bytes::to_number<float>(res->get("FLOAT").value) == 3.0f);
}

TEST_CASE("dependent removal struct", "[ProtocolDissector]")
{
    // packet descriptor
    PacketDescriptor packet_template;
    packet_template.add({.name{"SIZE"}, .size{1}, .determinesSizeOf{"DATA"}, .sizeDeterminesExistenceOf{"CRC"}});
    packet_template.add({.name{"DATA"}, .size{0}});
    packet_template.add({.name{"CRC"}, .size{1}});
    // the described packet as struct, but we skip the CRC field
    DependentSizeWithCRCPacket<0> packet{.data{}};
    GenericDissector              dissector{packet_template};

    auto bytes_complete = std::as_bytes(std::span{&packet, 1});
    auto bytes = bytes_complete.subspan(0, 1);

    float f = 3.0f;
    auto  res = test_dissect(dissector, bytes);

    REQUIRE(res->get("SIZE").value[0] == std::byte(0));
    REQUIRE(res->get("DATA").value.size() == 0);
    REQUIRE(!res->has("CRC"));
    // REQUIRE(res->get("CRC").value.size() == 0);
}

TEST_CASE("data_type_with_selfdescribing_length", "[ProtocolDissector]")
{
    auto self_describing_size_calculation = [](const protos::dissector::FieldDescriptor& field, std::span<std::byte> current_buffer) {
        // the first byte of the field value is the size of the data
        return static_cast<uint16_t>(current_buffer[0]) - 1;
    };
    // a type that contains the length of its own data in the first byte

    PacketDescriptor packet_template;
    packet_template.add({.name{"Field1"}, .size{1}});
    packet_template.add(
        {.name{"SelfDescribingField2"}, .size{1}, .externalSizeCalculation{self_describing_size_calculation}});
    //first byte = count (5) , following bytes = "helo"
    PacketWithSelfdescribingSizeField<5> packet{.field1{0x01}, .data{0x05, 0x68, 0x65, 0x6c, 0x6f}};
    GenericDissector                     dissector{packet_template};

    auto bytes = std::as_bytes(std::span{&packet, 1});
    auto res = test_dissect(dissector, bytes);
    REQUIRE(res->get("Field1").value[0] == std::byte(0x01));
    REQUIRE(res->get("SelfDescribingField2").value.size() == 4); // has the calculated size of 4
    

}