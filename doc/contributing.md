# Contributing

## Layout recap

```
apps/     _template + ProtocolDemoServer + ProtocolClient + ProtocolPusher + ProtoProxy
libs/     protos (core, no ZMQ) + services (ZMQ pub/sub + directory)
tests/    Catch2 tests for dissector and interpreter
examples/ 12 numbered walkthroughs; 01-09 use libs/protos only, 10-12 add services
doc/      this tree
cmake/    CPM bootstrap, coverage, stacktrace, embedded/standalone detection
```

## Adding a new app

Clone `apps/_template/` and rename. Its `CMakeLists.txt` is minimal enough to
serve as the canonical recipe:

```cmake
project(my_app)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "*.h*")
file(GLOB_RECURSE CPP_FILES    CONFIGURE_DEPENDS "*.cpp")
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC protos::protos)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
enable_coverage(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME})
```

Then add `add_subdirectory(my_app)` to `apps/CMakeLists.txt`. Need services?
`protos::protos_services` is the alias target to link instead of (or in addition
to) `protos::protos`.

## Adding an example

Create a new numbered subdirectory under `examples/`. Use any existing example's
`CMakeLists.txt` as a starting point — it is just:

```cmake
project(exNN_what_this_shows)
set(CMAKE_CXX_STANDARD 23)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE protos::protos)
```

Append `add_subdirectory(NN_what_this_shows)` to `examples/CMakeLists.txt`. Each
example ships a short `README.md` describing the wire format, the expected
output, and the guide it complements — please match that format.

## Tests

`tests/` uses Catch2 fetched via CPM. Two suites live there today:

- `tests/DissectorTests/ProtocolDissectorTests.cpp`
- `tests/InterpreterTests/GenericInterpreterTests.cpp`

Several interpreter test cases are commented out (subfields, embedded dissectors,
serialization round-trip) — they document features that exist in the headers but
are incomplete in the implementation. Don't ship code that depends on those paths
without first uncommenting the tests and making them green.

Run the test suite through CTest:

```bash
ctest --preset <preset-name>
```

## Build presets

The top-level `CMakePresets.json` defines configure and build presets covering
MSVC (x64 Debug/Release), Clang, and GCC across Windows and Linux. Use
`cmake --list-presets` to see the current set. Adding a new compiler/toolchain
is a matter of extending that file.

## Known gaps worth fixing

If you are looking for somewhere to contribute, here are TODOs surfaced by
reading the source — distinct from *intentional* removals (like the legacy
`subFields` expression engine and the registry-based sub-dissector, both torn
out deliberately in commit `bd2eb23`).

- `FieldInterpretation::littleEndian` is declared but never consulted. Wiring
  it up in `FieldInterpreter::interpret` (reverse bytes before `as_variant`
  when the flag disagrees with the host byte order) removes the need for the
  swap-mapper workaround documented in [endianness](./library/endianness.md).
- `libs/protos/serializer/GenericDissectorSerializer.hpp` still `#include`s
  the pre-refactor headers `<datafw/protocols/FieldDescriptor.hpp>` and
  `<datafw/protocols/PacketDescriptor.hpp>` — paths that don't exist after the
  `datafw::` → `protos::` namespace move. The header is effectively dead code
  today. Porting the `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT` glue to
  the current `protos::dissector::FieldDescriptor` / `PacketDescriptor` layout
  would restore the serializer feature that the v1.0.0 release notes already
  advertise.
- The directory client's PUB subscription is declared but not implemented.
  Today clients get updates only by explicitly calling `queryProtocols`.
- `PacketHelper` is marked experimental in the source ("you're on your own
  here"); solidifying its API and adding round-trip tests would let any
  future serializer work lean on it.
- `GenericPacketInterpreter::EmptyFieldBehavior::TROW_ONCE` is misspelled
  (should be `THROW_ONCE`). Fixing it is an ABI break, so probably deserves
  a deprecated alias.

## Commit / PR conventions

Follow whatever the existing `git log` shows. At the time of writing, commits are
terse and descriptive; PRs are welcome against `main`.
