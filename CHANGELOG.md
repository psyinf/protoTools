# Changelog

All notable changes to this fork of `protoTools` are documented here. This file
lives on the `dev` integration branch. Upstream (`psyinf/protoTools`) does not
currently maintain a changelog.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
The upstream project uses [SemVer](https://semver.org/spec/v2.0.0.html); this
fork follows the same convention.

## [Unreleased]

Integrated on `dev` from three independent feature branches and proposed
upstream as a single PR against `psyinf/protoTools:main`.

### Added

- **`feat/docs-and-examples`** — full documentation set and twelve runnable
  walkthroughs.
  - Top-level `README.md` with requirements, build, quickstart, repo tour.
  - `doc/` tree: `getting-started`, `architecture`, `protocols-and-packets`,
    `contributing`, per-component library guides (`dissection`,
    `interpretation`, `endianness`, `icd-cookbook`, `services`),
    `transport/udp`, and per-app references.
  - `examples/` 01–12: 01–09 exercise `libs/protos` (dissection, interpretation,
    UDP); 10–12 exercise `libs/services` (pub/sub, commands, discovery).
  - `PROTOTOOLS_BUILD_EXAMPLES` CMake option (default `ON` when standalone).
  - Two-message ICD "Worked example" capstone at the end of the cookbook.
- **`feat/serializer-port`** — working JSON round-trip for protocol
  descriptors.
  - Templated `protos::dissector::save<T>` / `load<T>` helpers in
    `libs/protos/serializer/GenericDissectorSerializer.hpp`.
  - `nlohmann::adl_serializer<std::byte>` specialisation; bytes encode as
    uppercase `"0xNN"` hex strings, case-insensitive on parse.
  - `tests/DissectorTests/SerializerTests.cpp` — round-trip, encoding
    contract, callback-drop, and human-readable-output cases (28 assertions
    across 5 cases).
- **`feat/little-endian`** — wired-up per-field endianness.
  - `tests/InterpreterTests/LittleEndianTests.cpp` — LE native, BE uint32 /
    uint64 / float, signed BE int16, mixed-endian packet, STRING / BYTES
    passthrough (17 assertions across 7 cases).

### Changed

- `FieldInterpreter::interpret` now honors `FieldInterpretation::littleEndian`
  for `INTEGER`, `UNSIGNED_INTEGER`, and `FLOAT` fields. When the declared
  wire endianness disagrees with `std::endian::native`, the field's bytes
  are reversed before `as_variant`. `STRING` and `BYTES` are unaffected;
  `BOOL` is single-byte so the flag is a no-op there. Previously the flag
  was declared but never read.
- Serializer `save` / `load` accept `std::filesystem::path` instead of
  `std::string` — Unicode-safe on Windows, matches the rest of the
  standard library's file API.
- `examples/07_icd_mixed_endian` rewritten to use `.littleEndian = false`
  directly. The earlier `BYTES` + swap-mapper workaround is gone; the
  example dropped from ~90 to ~60 lines without losing coverage.
- `libs/protos/CMakeLists.txt` now links `nlohmann_json::nlohmann_json`
  publicly (previously commented out).

### Fixed

- `libs/protos/serializer/GenericDissectorSerializer.hpp` compiles again.
  The header was orphaned by the April 2025 `datafw::` → `protos::`
  namespace rename; includes pointed at non-existent paths, and the
  NLOHMANN macros sat in the wrong namespace so ADL could not find
  `to_json` / `from_json` for `FieldDescriptor` and `PacketDescriptor`.
- Serializer `save` / `load` now `throw std::runtime_error` on file-open
  failure with the offending path in the message, rather than silently
  succeeding.

### Removed

- Pre-refactor PlantUML sources and rendered PNG/SVG assets under `doc/`
  (replaced by the new docs set).
- `doc/ProtoPackage.md` three-line stub (replaced by
  `doc/protocols-and-packets.md`).

### Known gaps (still pending)

- `ProtocolDirectoryClient::addChangeCallback` wires up a callback but the
  server's PUB socket has no matching subscriber on the client side today.
  Directory updates require explicit `queryProtocols()` calls.
- `PacketHelper` remains marked experimental in the source
  (`"you're on your own here"`).
- `GenericPacketInterpreter::EmptyFieldBehavior::TROW_ONCE` is misspelled;
  renaming is an ABI break, so deserves a deprecated alias rather than an
  in-place rename.

## [1.0.0] — 2025-09-29

Upstream baseline. See the
[release notes](https://github.com/psyinf/protoTools/releases/tag/v1.0.0)
for the full feature list introduced in 1.0.0.
