#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
#include <protos/dissection/PacketDescriptor.hpp>

#include <protos/dissection/GenericDissector.hpp>

struct EqualsResultMatcher : Catch::Matchers::MatcherGenericBase
{
    using Result = protos::interpreter::InterpretationResult;

    EqualsResultMatcher(Result const& result)
      : result{result}
    {
    }

    bool match(Result const& other) const { return other.name == result.name && other.value == result.value; }

    std::string describe() const override { return "Equals: TODO"; }

private:
    Result const& result;
};

TEST_CASE("simpleField", "[GenericInterpreter]")
{
    using namespace protos::dissector;
    using namespace protos::interpreter;
    using namespace protos::value::operators;
    using namespace std::string_literals;
    // packet descriptor
    PacketDescriptor packet_template;
    FieldDescriptor  field{"header", "Header field", 4};

    packet_template.add(std::move(field));

    GenericPacketInterpreter interpreter;

    interpreter.addField(FieldInterpretation{
        .name = "header",
        .type = protos::value::Type::UNSIGNED_INTEGER,
        .format{"{}"},
        .mapper{},
    });

    auto packet_data = PacketData{};
    auto v = protos::bytes::as_bytes(0xff11ee22);
    packet_data.add({"header", v});

    auto result = interpreter.interpretPacketData(packet_data);
    REQUIRE(result.size() == 1);
    auto x = result.at(0).value;
    REQUIRE((result.at(0).value == uint64_t{4279365154}));
}

/* Needs rework
TEST_CASE("subfields", "[GenericInterpreter]")
{
    using namespace datafw::dissector;
    using namespace datafw::protocol;
    using namespace std::string_literals;
    SECTION("single subfield")
    {
        auto v = std::vector<unsigned char>{{0xFF}, {0x0}, {0x0}, {0x1}};

        GenericPacketInterpreter interpreter;
        // define a subfield spanning 4 bits
        FieldInterpretation subField1{
            {.name = "upper", .type = protos::value::Type::INTEGER, .format{"{:#04x}"}, .mapper{}},
            .function{"value & 0x0000000F"} };

        interpreter.addField(
            FieldInterpretation{.name = "header", .format{"{}"}, .mapper{}, .subFields{{subField1}}});

        auto packet_data = PacketData{};
        packet_data.add({"header", datafw::utils::to_byte_vec(v)});
        auto result = interpreter.interpretPacket(packet_data);
        REQUIRE(result.size() == 1);
        REQUIRE(result.at(0) == "0x0f");
        // test remainder
    }
    SECTION("multiple subfield")
    {
        auto v = std::vector<unsigned char>{{0xFE}, {0x0}, {0x0}, {0x1}};

        GenericPacketInterpreter interpreter;
        // define a subfield spanning 4 bits
        FieldInterpretation subField1{
            {.name = "upper", .type = protos::value::Type::INTEGER, .format{"{:#04x}"}, .mapper{}},
            "value & 0x0000000F"};
        FieldInterpretation subField2{
            {.name = "lower", .type = protos::value::Type::UNSIGNED_INTEGER, .format{"{:#04x}"}, .mapper{}},
            "(value & 0x000000F0) >> 4"};

        interpreter.addField(
            FieldInterpretation{.name = "header", .format{"{}"}, .mapper{}, .subFields{{subField1}, {subField2}}});

        auto packet_data = PacketData{};
        packet_data.add({"header", datafw::utils::to_byte_vec(v)});
        auto result = interpreter.interpretPacket(packet_data);

        REQUIRE(result.size() == 2);
        REQUIRE(result.at(0) == "0x0e");
        REQUIRE(result.at(1) == "0x0f");
    }
    SECTION("subfield types")
    {
        // TODO: add tests for all types
        auto v = std::vector<unsigned char>{{0x1E}, {0x10}, {0x20}, {0x80}};

        GenericPacketInterpreter interpreter;

        FieldInterpretation subField1{{.name = "int", .type = protos::value::Type::INTEGER}, "value"};
        FieldInterpretation subField2{{.name = "uint", .type = protos::value::Type::UNSIGNED_INTEGER},
                                         "value"};
        FieldInterpretation subField3{{.name = "float", .type = protos::value::Type::FLOAT}, "value"};
        // FieldInterpretation subField4{{.name = "string", .type = protos::value::Type::STRING}, "value"};
        // //separate test
        FieldInterpretation subField5{{.name = "bool", .type = protos::value::Type::BOOL}, "value"};

        interpreter.addField(FieldInterpretation{.name = "header",
                                                 .format{"{}"},
                                                 .mapper{},
                                                 .subFields{{subField1}, {subField2}, {subField3}, {subField5}}});

        auto packet_data = PacketData{};
        packet_data.add({"header", datafw::utils::to_byte_vec(v)});
        auto result = interpreter.interpretPacket(packet_data);

        REQUIRE(result.at(0) == "-2145382370");
        REQUIRE(result.at(1) == "2149584926");
        REQUIRE(result.at(2) == "-2.944517634519523e-39");
        REQUIRE(result.at(3) == "true");
    }
}
TEST_CASE("Serialization roundtrip", "[GenericInterpreter]")
{
    using namespace datafw::dissector;
    using namespace datafw::protocol;
    using namespace std::string_literals;
    // packet descriptor
    PacketDescriptor packet_template;
    FieldDescriptor  field{"header", "Header field", 4};
    auto             v = std::vector<unsigned char>{{0xFE}, {0x0}, {0x0}, {0x1}};

    GenericPacketInterpreter interpreter;
    // define a subfield spanning 4 bits
    FieldInterpretation subField1{
        {.name = "upper", .type = protos::value::Type::INTEGER, .format{"{:#04x}"}, .mapper{}},
        "value & 0x0000000F"};
    FieldInterpretation subField2{
        {.name = "lower", .type = protos::value::Type::INTEGER, .format{"{:#04x}"}, .mapper{}},
        "value & 0x000000F0 >> 4"};

    interpreter.addField(
        FieldInterpretation{.name = "header", .format{"{}"}, .mapper{}, .subFields{{subField1}, {subField2}}});

    auto packet_data = PacketData{};
    packet_data.add({"header", datafw::utils::to_byte_vec(v)});
    auto result = interpreter.interpretPacket(packet_data);
    datafw::dissector::save("c:/temp/interpreter.json", interpreter);
    auto deserialized = datafw::dissector::load<GenericPacketInterpreter>("c:/temp/interpreter.json");

    auto result2 = deserialized.interpretPacket(packet_data);
    REQUIRE(result == result2);
}
*/
/*
TEST_CASE("mapped field", "[GenericInterpreter]")
{
    using namespace datafw::dissector;
    using namespace protos::value::operators;
    using namespace datafw::protocol;
    using namespace std::string_literals;
    getRegistry<FieldMapperInterface>().add(
        "testMapper",
        std::make_shared<LambdaFieldMapper>(
            [](const FieldMapperInterface::TypedData& data) -> FieldMapperInterface::Result {
                if (std::byte{0x00} == data[0]) { return FieldMapperInterface::Result({"0x0e"}); }
                if (std::byte{0x01} == data[0]) { return FieldMapperInterface::Result({"0x0f"}); }
                return FieldMapperInterface::Result({"0x00"});
            }));
    auto                     v = std::vector<unsigned char>{{0x00}};
    auto                     v1 = std::vector<unsigned char>{{0x01}};
    auto                     v2 = std::vector<unsigned char>{{0x0F}};
    GenericPacketInterpreter interpreter;

    interpreter.addField(FieldInterpretation{.name = "header",
                                             .type = protos::value::Type::UNSIGNED_INTEGER,
                                             .format{"{}"},
                                             .mapper{"testMapper"}});

    {
        auto packet_data = PacketData{};
        packet_data.add({"header", datafw::utils::to_byte_vec(v)});
        auto result = interpreter.interpretPacket(packet_data);

        REQUIRE(result.size() == 1);
        REQUIRE_THAT(result.at(0), EqualsResultMatcher({ "header", 0x0e }));
    }
    {
        auto packet_data = PacketData{};
        packet_data.add({"header", datafw::utils::to_byte_vec(v1)});
        auto result = interpreter.interpretPacket(packet_data);

        REQUIRE(result.size() == 1);
        REQUIRE_THAT(result.at(0), EqualsResultMatcher({ "header", 0x0f }));
    }
    {
        auto packet_data = PacketData{};
        packet_data.add({"header", datafw::utils::to_byte_vec(v2)});
        auto result = interpreter.interpretPacket(packet_data);

        REQUIRE(result.size() == 1);
        REQUIRE_THAT(result.at(0), EqualsResultMatcher({ "header", 0x00 }));
    }
}*/
/*
TEST_CASE("Embedded protocol", "[GenericInterpreter]")
{
    using namespace datafw;
    using namespace datafw::dissector;
    using namespace datafw::interfaces;
    using namespace protos::value::operators;
    using namespace datafw::protocol;
    using namespace std::string_literals;
    // packet descriptor
    PacketDescriptor outer_packet_template;
    outer_packet_template.add(FieldDescriptor{"header", "Header field", 1});
    outer_packet_template.add(FieldDescriptor{"payload", "Payload field", 4});
    //     PacketDescriptor inner_packet_template;
    //     inner_packet_template.add(FieldDescriptor{"speed", "speed in [m/s]", 4});
    //     inner_packet_template.add(FieldDescriptor{"position", "position in [m]", 4});
    // interpreter for outer packet
    GenericPacketInterpreter outer_interpreter;
    outer_interpreter.addField(FieldInterpretation{
        .name = "header",
        .type = protos::value::Type::UNSIGNED_INTEGER,
        .format{"{:#04x}"},
        .mapper{},
    });
    outer_interpreter.addField(FieldInterpretation{.name = "payload", .dissector{"inner"}});

    // dissector for outer packet's payload
    PacketDescriptor inner_packet_template{.name = "inner"};
    inner_packet_template.add(FieldDescriptor{"speed", "speed in [m/s]", 2});
    inner_packet_template.add(FieldDescriptor{"position", "position in [m]", 2});

    // interpreter for outer packet's payload
    GenericPacketInterpreter inner_interpreter;
    inner_interpreter.addField(FieldInterpretation{
        .name = "speed",
        .type = protos::value::Type::UNSIGNED_INTEGER,
        .format{"{}"},
        .mapper{},
    });
    inner_interpreter.addField(FieldInterpretation{
        .name = "position",
        .type = protos::value::Type::INTEGER,
        .format{"{}"},
        .mapper{},
    });
    // register inner interpreter
    datafw::getRegistry<PacketDescriptor>().add("inner", std::make_shared<PacketDescriptor>(inner_packet_template));
    datafw::getRegistry<PacketInterpreterInterface>().add(
        "inner", std::make_shared<GenericPacketInterpreter>(inner_interpreter));

    auto packet_data = PacketData{};
    // header with EA, speed is 12369, position is -8558
    auto v = std::vector<unsigned char>{
        {0xEA},
    };

    packet_data.add({"header", datafw::utils::to_byte_vec(std::vector<unsigned char>{{0xEA}})});
    packet_data.add(
        {"payload", datafw::utils::to_byte_vec(std::vector<unsigned char>{{0x51}, {0x30}, {0x92}, {0xDE}})});
    auto result = outer_interpreter.interpretPacketData(packet_data);
    REQUIRE(result.size() == 3);
    REQUIRE_THAT(result.at(0), EqualsResultMatcher({"header", uint64_t{0x0ea}}));
    REQUIRE_THAT(result.at(1), EqualsResultMatcher({"speed", uint64_t{12369}}));
    REQUIRE_THAT(result.at(2), EqualsResultMatcher({"position", int64_t{-8558}}));
}*/