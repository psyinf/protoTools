# Services

The `protos_services` library is a thin ZeroMQ wrapper for distributing protocol
data in a pub/sub pattern, with an optional REQ/REP sidechannel for commands and
a separate directory service for discovery.

The dissector/interpreter libraries are **transport-agnostic** — you can use them
without ever linking `protos_services`. This guide covers the ZMQ integration
specifically. For a UDP-based deployment, see [transport/udp](../transport/udp.md).

## Components

```
┌──────────────┐     ProtoPackage     ┌──────────────┐
│ProtocolServer│──PUB─────────▶SUB────│ProtocolClient│
│              │                      │              │
│              │◀─REP────────◀REQ─────│              │
└──────────────┘                      └──────────────┘
      │                                      ▲
      │ addProtocol(entry)                   │ queryProtocols()
      ▼                                      │
┌───────────────────────┐  LIST (REQ/REP)    │
│ProtocolDirectoryServer│◀───────────────────┘
│                       │
│                       │──PUB──▶ (change notifications, not yet wired)
└───────────────────────┘
```

Headers (all under `libs/services/`):

- `ProtocolPublisher.hpp` — PUB socket wrapper.
- `ProtocolServer.hpp` — PUB + REQ/REP command handler.
- `ProtocolClient.hpp` — SUB + REQ socket wrapper.
- `ProtocolMessages.hpp` — `ProtoHeader`, `ProtoData`, `ProtoPackage`, `Command`, `CommandReply`.
- `ProtoUtils.hpp` — `makeContext(num_worker_threads)` factory for `zmq::context_t`.
- `directory/ProtocolDirectoryServer.hpp` / `ProtocolDirectoryClient.hpp`.

## Wire format

See [protocols-and-packets](../protocols-and-packets.md) for the detailed framing.
Short version: every published message is two ZMQ frames — a `protocol_name[|source]`
header string followed by the raw payload bytes. Commands are three frames:
verb, receiver, data. Replies are two frames: verb, data.

## ProtocolPublisher

```cpp
auto ctx = ProtoUtils::makeContext(1);
ProtocolPublisher pub(ctx);
pub.bind({.pub_endpoint = "tcp://*:5557"});
pub.send({"DEMO", "source_A"}, {std::vector<char>{'h','i'}});
```

`ProtoPublisherConfig::is_proxy = true` switches from `bind()` to `connect()` — use
it when the publisher is feeding into a XSUB/XPUB proxy (see `apps/ProtoProxy`).

## ProtocolClient

```cpp
ProtocolClient sub(ctx);
sub.bind({.sub_endpoint = "tcp://localhost:5557",
          .req_endpoint = "tcp://localhost:5558"});
sub.subscribe("DEMO");                           // ZMQ prefix match on the header

auto pkg = sub.receiveSubscribed();              // blocking
auto reply = sub.sendCommand({"BUMP", "target", "payload"});
```

Both sockets are always created — even a pure-subscribe client sets a
`req_endpoint`. ZMQ `connect()` is lazy, so pointing the REQ at an endpoint that
never answers is fine as long as you never call `sendCommand`.

### Slow-joiner caveat

ZMQ PUB drops messages published before any subscriber has connected. Practical
consequences:

- Start the subscriber first.
- Give it a moment to finish `bind()` before the publisher begins sending.
- Do not rely on the first few messages arriving — send a leading handshake or a
  sequence number so missing packets are detectable.

Example: [`examples/10_pub_sub_minimal`](../../examples/10_pub_sub_minimal/).

## ProtocolServer

```cpp
ProtocolServer server(ctx);
server.bind("tcp://*:5561", /*is_proxy=*/false);
server.setCommandCallback([](const Command& c) -> CommandReply {
    if (c.command_verb == "PING") return {"PONG", ""};
    return {"NACK", "unknown verb"};
});
server.startCommandHandler("tcp://*:5562");

server.publish({"DEMO", "server"}, {std::vector<char>{'x'}});
```

`startCommandHandler` spawns a detached `std::jthread` running
`ProtocolCommandServer::receive` in a loop. The callback must be thread-safe
relative to any shared state touched from the main thread.

Example: [`examples/11_server_with_commands`](../../examples/11_server_with_commands/).

## ProtocolDirectoryServer / Client

Service discovery. The server holds a JSON-serialised list of
`ProtocolDirectoryEntry{protocol_name, adapter_descriptor, publisher_endpoint,
command_endpoint}` records and answers `"list"` over REQ/REP.

```cpp
// Server
ProtocolDirectoryServer dir(ctx);
dir.bind("tcp://*:9999", "tcp://*:9998");
dir.addProtocol({"CAN", "USB_CAN", "tcp://127.0.0.1:41000", "tcp://127.0.0.1:41001"});
dir.startRunning();

// Client (elsewhere)
ProtocolDirectoryClient client(ctx);
client.bind("tcp://localhost:9999", "tcp://localhost:9998");
auto protocols = client.queryProtocols(/*timeout_msec=*/2000);
```

**Current implementation notes (from source review):**

- `ProtocolDirectoryClient::bind` accepts a `pub_endpoint` argument for receiving
  change notifications, but the sub socket isn't actually created in the current
  code. `addChangeCallback` wires up a callback that fires on
  `updateDirectory(...)` — today only invoked from `queryProtocols`, so it
  effectively behaves like "notify on every successful query".
- The server's PUB socket periodically broadcasts the current list, but there is
  no client-side subscriber to consume it today.

Example: [`examples/12_service_discovery`](../../examples/12_service_discovery/).

## Proxy

`apps/ProtoProxy` runs a ZMQ XSUB↔XPUB bridge (`tcp://*:55555` in, `tcp://*:55556`
out). Publishers set `is_proxy = true` in their `ProtoPublisherConfig` and
*connect* to the proxy's frontend instead of binding themselves. Subscribers
connect to the backend. Useful when many publishers need a stable address for
subscribers to discover.

## Related

- App reference: [`apps/protocol-demo-server`](../apps/protocol-demo-server.md),
  [`apps/protocol-client`](../apps/protocol-client.md),
  [`apps/protocol-pusher`](../apps/protocol-pusher.md),
  [`apps/proto-proxy`](../apps/proto-proxy.md).
