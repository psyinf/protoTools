# protoTools

A C++23 toolkit for **dissecting binary protocol messages**, **interpreting**
their fields into typed values, and (optionally) **transporting** them over a
ZMQ pub/sub fabric.

Good fit for:

- Parsing ICD-defined binary streams (CAN, MODBUS, custom aerospace/industrial
  protocols) from UDP, TCP, serial, or any other source.
- Building a small service mesh where each process publishes one protocol and
  consumers subscribe by name.
- Exploratory protocol work — declare a packet layout at runtime, feed it
  bytes, inspect typed results.

Not a fit for: high-throughput wire parsers where every allocation matters, or
projects looking for a code-generator from an ICD spec (neither is a goal here).

## Features

- **Declarative packet parsing** — `PacketDescriptor` + `FieldDescriptor` express
  fixed-size, length-prefixed, optional, and self-describing fields.
- **Typed interpretation** — `GenericPacketInterpreter` turns parsed bytes into
  `std::variant` values with `std::format`-style rendering and custom mappers
  for enums, scaled values, and bitfields.
- **ZMQ services** — `ProtocolPublisher`, `ProtocolClient`, `ProtocolServer`,
  plus a small JSON-backed directory for discovery.
- **XSUB/XPUB proxy** — a bundled `ProtoProxy` binary for many-to-many fan-out.

## Requirements

- C++23 compiler (MSVC 19.34+, Clang 16+, GCC 13+).
- CMake ≥ 3.22.
- Git and network access at configure time (dependencies fetched via CPM:
  spdlog, CLI11, cppzmq, libzmq, nlohmann_json, Catch2).

No system packages to install.

## Build

```bash
cmake --preset <preset-name>
cmake --build --preset <preset-name>
```

`cmake --list-presets` shows the available configure presets. Build options:

| Option                      | Default | Effect                          |
|-----------------------------|---------|---------------------------------|
| `ENABLE_TESTING`            | `ON`    | Build `tests/`.                 |
| `BUILD_APPS`                | `ON`    | Build `apps/`.                  |
| `PROTOTOOLS_BUILD_EXAMPLES` | `ON`    | Build the `examples/` tree.     |

## Quickstart

Two terminals:

```bash
# Terminal A
./ProtocolDemoServer     # starts directory + server, publishes "Message (CAN) N"

# Terminal B
./ProtocolClient         # queries directory, subscribes, prints messages
                         # 's' send | 'c' connect | 'a' connect-all | 'q' quit
```

Or skip ZMQ entirely and run one of the self-contained dissection/interpretation
examples:

```bash
./ex01_simple_dissector
./ex07_icd_mixed_endian
./ex09_udp_receiver & ./ex09_udp_sender
```

## Repo tour

| Path         | What's there                                                                    |
|--------------|----------------------------------------------------------------------------------|
| `apps/`      | Reference binaries: `ProtocolDemoServer`, `ProtocolClient`, `ProtocolPusher`, `ProtoProxy`, `_template`. |
| `libs/protos/`    | Core dissection + interpretation libraries. Zero ZMQ dependency. |
| `libs/services/`  | ZMQ pub/sub, command server, service directory.              |
| `tests/`     | Catch2 unit tests for the dissector and interpreter.                             |
| `examples/`  | Twelve numbered, runnable walkthroughs — 01–09 cover parsing/UDP, 10–12 cover ZMQ services. |
| `doc/`       | The documentation you are reading now.                                           |
| `cmake/`     | CPM bootstrap, coverage, stacktrace, embedded/standalone detection.              |

## Documentation map

- **Start here**: [`doc/getting-started.md`](./doc/getting-started.md) and
  [`doc/architecture.md`](./doc/architecture.md).
- **Library integrators**: [`doc/library/dissection.md`](./doc/library/dissection.md),
  [`doc/library/interpretation.md`](./doc/library/interpretation.md),
  [`doc/library/endianness.md`](./doc/library/endianness.md),
  [`doc/library/icd-cookbook.md`](./doc/library/icd-cookbook.md),
  [`doc/library/services.md`](./doc/library/services.md).
- **Transport**: [`doc/transport/udp.md`](./doc/transport/udp.md).
- **App users**: [`doc/apps/`](./doc/apps/).
- **Contributors**: [`doc/contributing.md`](./doc/contributing.md).
- **Wire formats**: [`doc/protocols-and-packets.md`](./doc/protocols-and-packets.md).

## Known gaps (documented where relevant)

- `FieldInterpretation::littleEndian` is declared but not yet honored; see
  [endianness](./doc/library/endianness.md) for the mapper-based workaround.
- `ProtocolDirectoryClient`'s change-notification PUB subscription is declared
  but not wired up; today clients update via explicit `queryProtocols()`.

Intentional removals (not gaps): the legacy string-expression `subFields`
engine and the registry-resolved `dissector{}` member on `FieldInterpretation`
were torn out in commit `bd2eb23` in favor of `std::function` mappers. The
commented-out test cases that exercised those features are design history,
not a TODO. See [interpretation](./doc/library/interpretation.md#subfields--legacy-superseded-by-mappers).

## License

See the repository root for license information.
