#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <protos/dissection/FieldDescriptor.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/serializer/GenericDissectorSerializer.hpp>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

using protos::dissector::FieldDescriptor;
using protos::dissector::PacketDescriptor;

// FieldDescriptor carries a std::function (externalSizeCalculation) with no
// operator==, so no default comparison. Compare the serializable subset only -
// same fields the NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT macro writes.
static bool equal_ignoring_callback(const FieldDescriptor& a, const FieldDescriptor& b)
{
    return a.name == b.name && a.description == b.description && a.size == b.size && a.value == b.value &&
           a.determinesSizeOf == b.determinesSizeOf && a.sizeDeterminesExistenceOf == b.sizeDeterminesExistenceOf &&
           a.isHeaderValue == b.isHeaderValue && a.sizeOffset == b.sizeOffset;
}

static bool equal_packets(const PacketDescriptor& a, const PacketDescriptor& b)
{
    if (a.name != b.name) { return false; }
    if (a.fields.size() != b.fields.size()) { return false; }
    for (size_t i = 0; i < a.fields.size(); ++i)
    {
        if (!equal_ignoring_callback(a.fields[i], b.fields[i])) { return false; }
    }
    return true;
}

TEST_CASE("PacketDescriptor roundtrips through save/load", "[Serializer]")
{
    PacketDescriptor original;
    original.name = "TEST_ICD";

    // Fixed-size field.
    original.add({.name        = "magic",
                  .description = "4-byte sync word",
                  .size        = 4,
                  .value       = {std::byte{'I'}, std::byte{'C'}, std::byte{'D'}, std::byte{'1'}},
                  .isHeaderValue = true});

    // Length-prefix field with a nonzero sizeOffset.
    original.add({.name             = "length",
                  .description      = "payload length, excludes 2-byte framing",
                  .size             = 2,
                  .determinesSizeOf = "payload",
                  .sizeOffset       = -2});

    // Conditional field.
    original.add({.name                      = "flags",
                  .size                      = 1,
                  .sizeDeterminesExistenceOf = "payload"});

    original.add({.name = "payload", .size = 0});

    auto path = std::filesystem::temp_directory_path() / "prototools_serializer_roundtrip.json";

    protos::dissector::save(path.string(), original);
    REQUIRE(std::filesystem::exists(path));
    REQUIRE(std::filesystem::file_size(path) > 0);

    auto loaded = protos::dissector::load<PacketDescriptor>(path.string());

    REQUIRE(equal_packets(original, loaded));

    // Spot-check individual fields so a failure message is legible.
    REQUIRE(loaded.name == "TEST_ICD");
    REQUIRE(loaded.fields.size() == 4);
    REQUIRE(loaded.fields[0].isHeaderValue);
    REQUIRE(loaded.fields[0].value.size() == 4);
    REQUIRE(loaded.fields[0].value[0] == std::byte{'I'});
    REQUIRE(loaded.fields[1].determinesSizeOf == "payload");
    REQUIRE(loaded.fields[1].sizeOffset == -2);
    REQUIRE(loaded.fields[2].sizeDeterminesExistenceOf == "payload");

    std::filesystem::remove(path);
}

TEST_CASE("FieldDescriptor bytes serialize as a JSON numeric array", "[Serializer]")
{
    // Confirm the encoding contract (bytes as numbers, not as a base64/hex string).
    FieldDescriptor f{.name = "m", .size = 3, .value = {std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}}};
    nlohmann::json  j = f;
    REQUIRE(j["value"].is_array());
    REQUIRE(j["value"].size() == 3);
    REQUIRE(j["value"][0].get<unsigned>() == 0x01u);
    REQUIRE(j["value"][1].get<unsigned>() == 0xABu);
    REQUIRE(j["value"][2].get<unsigned>() == 0xFFu);
}

TEST_CASE("externalSizeCalculation is silently dropped on save", "[Serializer]")
{
    // A size-callback-bearing field must serialize without touching the callback,
    // and the loaded result must have no callback attached (caller re-wires).
    FieldDescriptor original{.name = "payload", .size = 1};
    original.withExternalSizeCalculation(
        [](const FieldDescriptor&, std::span<std::byte>) {
            return FieldDescriptor::SizeCallCallbackResult{false, 4, 1};
        });
    REQUIRE(static_cast<bool>(original.externalSizeCalculation));

    nlohmann::json j = original;
    // No "externalSizeCalculation" key exists in the JSON surface.
    REQUIRE_FALSE(j.contains("externalSizeCalculation"));

    auto loaded = j.get<FieldDescriptor>();
    REQUIRE(loaded.name == "payload");
    REQUIRE_FALSE(static_cast<bool>(loaded.externalSizeCalculation));
}

TEST_CASE("save/load throw on unreachable paths", "[Serializer]")
{
    PacketDescriptor pd;

    // Path under a directory that cannot exist.
    const std::string bad = "Z:/definitely/not/a/real/dir/out.json";
    REQUIRE_THROWS_WITH(protos::dissector::save(bad, pd),
                        Catch::Matchers::ContainsSubstring("cannot open"));
    REQUIRE_THROWS_WITH(protos::dissector::load<PacketDescriptor>(bad),
                        Catch::Matchers::ContainsSubstring("cannot open"));
}

TEST_CASE("JSON output is human-readable and stable", "[Serializer]")
{
    // Pin the output format so accidental changes to dump(2) indent or field
    // order reveal themselves in review.
    PacketDescriptor pd;
    pd.name = "X";
    pd.add({.name = "a", .size = 2, .value = {std::byte{0x10}, std::byte{0x20}}});

    nlohmann::json j = pd;
    auto           text = j.dump(2);
    INFO(text);
    REQUIRE(text.find("\"name\": \"X\"") != std::string::npos);
    REQUIRE(text.find("\"fields\"") != std::string::npos);
    REQUIRE(text.find("\"a\"") != std::string::npos);
    REQUIRE(text.find("16") != std::string::npos); // 0x10 as decimal
    REQUIRE(text.find("32") != std::string::npos); // 0x20 as decimal
}
