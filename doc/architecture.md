# Architecture

```
┌──────────────────────────────────────────────────────────────────────┐
│                            apps/ (executables)                       │
│  ProtocolDemoServer  ProtocolClient  ProtocolPusher  ProtoProxy      │
└───────────────────────┬─────────────────────────┬────────────────────┘
                        │                         │
                        ▼                         ▼
┌──────────────────────────┐        ┌──────────────────────────────────┐
│  libs/services           │        │  libs/protos                     │
│  (ZMQ pub/sub, commands, │◀──uses─│  dissection  + interpretation    │
│   service directory)     │        │  + common bit utils + serializer │
└──────────────────────────┘        └──────────────────────────────────┘
                                                ▲
                                                │ depend only on std + {fmt/format}
                                                │
                                         tests/, examples/ 01-09
```

## Layers

### `libs/protos` — the core

Standalone, no transport, no ZMQ. Three concerns:

- **Dissection** (`libs/protos/dissection/`) — declarative byte-stream parser.
  Entry point: `GenericDissector`.
- **Interpretation** (`libs/protos/interpretation/`) — turn parsed bytes into
  typed `std::variant` values with optional custom mappers. Entry point:
  `GenericPacketInterpreter`.
- **Common** (`libs/protos/common/`) — bit/byte conversion helpers (`BitUtils`) and
  a `once` guard.
- **Serializer** (`libs/protos/serializer/`) — placeholder for persisting
  descriptors; currently one header.

Dissection and interpretation are linked but decoupled: dissection has no idea
what `FieldInterpretation` is, and interpretation consumes `PacketData` (the
dissector's output) without knowing how it was produced.

### `libs/services` — the ZMQ layer

Depends on `libs/protos` plus `cppzmq`, `libzmq`, `spdlog`, `nlohmann_json`.

- `ProtocolPublisher` — PUB socket + a two-frame wire format
  (`protocol_name[|source]`, payload).
- `ProtocolServer` — `ProtocolPublisher` composed with a `ProtocolCommandServer`
  (REQ/REP) running on a detached thread.
- `ProtocolClient` — SUB + REQ sockets, a blocking `receiveSubscribed()`, a
  synchronous `sendCommand()`.
- `directory/*` — a standalone REP/PUB service registry. The server answers
  `"list"` queries with a JSON array of `ProtocolDirectoryEntry` records and
  periodically broadcasts the current list.

### `apps/` — reference binaries

Each app is a small demonstration of how the libraries compose:

- `ProtocolDemoServer` — an embedded directory + a protocol server + a
  heartbeat-publishing loop.
- `ProtocolClient` — directory discovery → multi-subscription → interactive
  command sending.
- `ProtocolPusher` — the thinnest possible `ProtocolPublisher` user.
- `ProtoProxy` — a ZMQ XSUB↔XPUB bridge on ports 55555/55556.
- `_template` — empty scaffold for cloning when adding a new app.

Per-app docs under [`doc/apps/`](./apps/).

### `tests/`

Catch2 tests exercising the dissector and interpreter. Noteworthy: several
interpreter test cases (subfields, embedded dissectors, serialization round-trip)
are commented out and represent known incomplete features — don't rely on those
code paths without verifying.

### `examples/`

Twelve small, numbered walkthroughs. Numbers 01–09 stay in `libs/protos` territory
(dissection, interpretation, endianness, UDP transport). Numbers 10–12 cover the
ZMQ services layer.

## Build-time shape

The root `CMakeLists.txt` uses CPM (fetched by `cmake/setup_cpm.cmake`) to pull
third-party deps into the build tree at configure time. `cmake/detect_embedded.cmake`
sets `IS_STANDALONE_PROJECT`, which gates the apps, tests, and examples — so a
downstream project that does `add_subdirectory(protoTools)` gets the libraries
and nothing else by default.

## What is *not* here

- A transport abstraction. The services layer speaks ZMQ. UDP, raw TCP, serial,
  or in-process queues are the caller's responsibility (see [transport/udp](./transport/udp.md)).
- A code generator. `PacketDescriptor` / `FieldInterpretation` are built
  imperatively at runtime; there's no tool that turns an ICD spec into C++ today.
- A finished serialization story for descriptors and interpreters. The
  `GenericDissectorSerializer.hpp` header exists but is minimal; the interpreter
  serialization test in `tests/InterpreterTests` is commented out.
- Per-field endianness on the interpreter. The flag is declared but not consumed;
  see [endianness](./library/endianness.md).
