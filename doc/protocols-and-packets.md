# Protocols, Packets, and Messages

Two framing layers show up in `protoTools`:

1. **Wire-level packet framing** — how raw protocol bytes are sliced into fields.
   Owned by the dissection library. Documented in [library/dissection](./library/dissection.md).
2. **Service-level message framing** — how protocol data travels on ZMQ sockets
   between `ProtocolPublisher`, `ProtocolServer`, and `ProtocolClient`. Covered
   here.

## ProtoHeader

```cpp
struct ProtoHeader {
    std::string protocol_name;  // e.g. "CAN"
    std::string source;         // e.g. "USB_CAN_Channel_1" (optional)
};
```

Serialised on the wire as `"<protocol_name>"` or `"<protocol_name>|<source>"` if
`source` is non-empty. This single string becomes the first frame of every
published ZMQ message.

Subscriber filtering uses ZMQ's built-in prefix match. Subscribing to `"CAN"`
matches every published message whose header starts with `"CAN"` — including
`"CAN|USB_CAN_Channel_1"`. Keep the source short; it is sent on every message.

## ProtoData

```cpp
struct ProtoData {
    std::vector<char> data;
};
```

Opaque byte payload. No framing beyond what the sender put there. This is where
the output of `PacketHelper::packetDataToBuffer` typically goes, or a raw
`std::string` / `std::vector<char>` built by the user.

## ProtoPackage

```cpp
struct ProtoPackage {
    ProtoHeader header;
    ProtoData   data;
};
```

`ProtocolClient::receiveSubscribed()` returns one of these — the two incoming ZMQ
frames stitched back together.

## Command / CommandReply

The REQ/REP side of the services API. Three frames outbound, two frames back.

```cpp
struct Command {
    std::string command_verb;      // e.g. "CONNECT", "SEND"
    std::string command_receiver;  // e.g. "CAN", "ECHO"
    std::string command_data;      // arbitrary payload
};

struct CommandReply {
    std::string reply_verb;        // e.g. "ACK", "NACK", "PONG"
    std::string reply_data;        // arbitrary payload
};
```

No enumerated set of verbs is defined by the framework — they are conventions
between your server's command callback and the clients talking to it. The demo
server in `apps/ProtocolDemoServer` accepts any verb and echoes `{verb, "ACK"}`;
the demo client sends `SEND` and `CONNECT` verbs.

## Wire summary

| Direction              | Frame 1             | Frame 2             | Frame 3           |
|------------------------|---------------------|---------------------|-------------------|
| Publisher → Subscriber | `protocol_name[|source]` | payload bytes  | —                 |
| Client → Server (cmd)  | `command_verb`      | `command_receiver`  | `command_data`    |
| Server → Client (reply)| `reply_verb`        | `reply_data`        | —                 |
| Client → Directory     | `"list"`            | —                   | —                 |
| Directory → Client     | JSON list           | —                   | —                 |

ZMQ handles multipart delivery atomically, so a subscriber always sees the full
`ProtoPackage` or nothing; a command handler always sees all three frames of a
command.

## Relationship to the dissection/interpretation libraries

These message types are *only* relevant when you use `libs/services`. The
dissector and interpreter consume raw bytes (`std::span<std::byte>` /
`std::vector<std::byte>`) and have no opinion on how those bytes arrived. A
UDP-based stack bypasses all of the `Proto*` types — see [transport/udp](./transport/udp.md).

Related: [library/services](./library/services.md).
