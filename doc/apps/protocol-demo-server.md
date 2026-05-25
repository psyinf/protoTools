# ProtocolDemoServer

Reference implementation that ties everything together: an embedded
`ProtocolDirectoryServer`, a `ProtocolServer` with a command callback, and a loop
that publishes a synthesised message every 100 ms.

## Endpoints

| Service              | Endpoint                  | Role                            |
|----------------------|---------------------------|---------------------------------|
| Directory REQ/REP    | `tcp://*:9999`            | `"list"` queries                |
| Directory PUB        | `tcp://localhost:9998`    | (list broadcasts)               |
| Protocol publisher   | `tcp://127.0.0.1:41000`   | `"CAN"` heartbeat messages      |
| Command handler      | `tcp://*:41001`           | Accepts any verb, replies `ACK` |

The registered directory entry reports `("CAN", "USB_CAN", "tcp://127.0.0.1:41000",
"tcp://*:41001")`.

## CLI

No arguments today. Future-proof by running `--help` first.

## Run

```bash
./ProtocolDemoServer
```

Log sample:

```
[info] Starting Protocol demo server
[info] Server started
[info] Message: Message (CAN) 0
[info] Message: Message (CAN) 1
...
[info] Received command: SEND:ECHO:Hullo 0       # when a client sends one
```

## How it works

The binary is essentially:

```cpp
auto context = ProtoUtils::makeContext(1);

ProtocolDirectoryServer directory(context);
directory.bind("tcp://*:9999");
directory.addProtocol({"CAN", "USB_CAN", "tcp://127.0.0.1:41000", "tcp://*:41001"});
directory.startRunning();

ProtocolServer server(context);
server.bind("tcp://127.0.0.1:41000", false);
server.setCommandCallback([](const Command& c) {
    return CommandReply{c.command_verb, "ACK"};
});
server.startCommandHandler("tcp://*:41001");

while (true) {
    server.publish({"CAN", "CAN_USB"}, {/* bytes */});
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
```

Source: `apps/ProtocolDemoServer/ProtocolDemoServer.cpp`.

## See also

- [library/services](../library/services.md) — API reference for each component.
- [apps/protocol-client](./protocol-client.md) — the intended client for this server.
- Minimal variants in examples: [`10`](../../examples/10_pub_sub_minimal/),
  [`11`](../../examples/11_server_with_commands/), [`12`](../../examples/12_service_discovery/).
