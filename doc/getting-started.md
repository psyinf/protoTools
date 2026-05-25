# Getting Started

## Requirements

- A C++23-capable compiler (MSVC 19.34+, Clang 16+, or GCC 13+).
- CMake ≥ 3.22.
- Git (for CPM-driven dependency fetching).
- Network access the first time you configure — CPM downloads spdlog, CLI11,
  cppzmq, libzmq, nlohmann_json, and Catch2 into the build tree.

No system packages are required. All deps are vendored via CPM at configure
time.

## Build

From the repo root:

```bash
cmake --preset <preset-name>
cmake --build --preset <preset-name>
```

Available presets are listed by `cmake --list-presets`. Names follow the pattern
`<os>-<compiler>-<config>`, e.g. `windows-msvc-debug`, `linux-gcc-release`.

The top-level `CMakeLists.txt` exposes three knobs:

| Option                       | Default | Effect                                                   |
|------------------------------|---------|----------------------------------------------------------|
| `ENABLE_TESTING`             | `ON`    | Build and register `tests/`.                             |
| `BUILD_APPS`                 | `ON`    | Build the apps in `apps/`.                               |
| `PROTOTOOLS_BUILD_EXAMPLES`  | `ON`    | Build the `examples/` tree (runnable walkthroughs).      |

All three gates are also guarded by `IS_STANDALONE_PROJECT`, so when `protoTools`
is pulled in via `add_subdirectory` from a larger project nothing unexpected gets
built.

## Quickstart: the demo stack in two terminals

After a successful build, the demo binaries live under your build tree's `apps/`
directory.

```bash
# Terminal 1 - starts a directory server, a protocol server, and publishes
# "Message (CAN) N" every 100 ms on tcp://127.0.0.1:41000.
./ProtocolDemoServer
```

```bash
# Terminal 2 - queries the directory, subscribes, prints messages.
# Press 's' to send a SEND command, 'c' for CONNECT, 'a' to CONNECT-all, 'q' to quit.
./ProtocolClient
```

The server registers itself in its own embedded `ProtocolDirectoryServer`, and
the client discovers the endpoints from the directory rather than hardcoding
them. Full writeups:

- [`apps/protocol-demo-server`](./apps/protocol-demo-server.md)
- [`apps/protocol-client`](./apps/protocol-client.md)

## Quickstart: smallest possible dissector

If you only want the parsing library, skip the apps entirely:

```bash
./ex01_simple_dissector
```

Source at [`examples/01_simple_dissector/main.cpp`](../examples/01_simple_dissector/main.cpp).
From there, walk up through the examples — 02 adds length-prefixed fields, 06
introduces the interpreter, 07 handles mixed-endian ICDs, and 09 wires the
library to a real UDP socket.

## Where to go next

- Library integrator? Start with [architecture](./architecture.md), then
  [library/dissection](./library/dissection.md) and
  [library/interpretation](./library/interpretation.md).
- Running the stock apps? [apps/](./apps/).
- Building an ICD parser over UDP? [library/icd-cookbook](./library/icd-cookbook.md)
  and [transport/udp](./transport/udp.md).
- Contributing? [contributing](./contributing.md).
