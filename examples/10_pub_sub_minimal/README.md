# 10 — Pub/Sub Minimal

Smallest working `ProtocolPublisher` + `ProtocolClient` pair.

## Run (two terminals)

```
# Terminal A
./ex10_subscriber

# Terminal B - start a second or two later
./ex10_publisher
```

Expected subscriber output:

```
[DEMO|loopback] tick 1
[DEMO|loopback] tick 2
[DEMO|loopback] tick 3
[DEMO|loopback] tick 4
[DEMO|loopback] tick 5
```

## Notes

- Start the subscriber **first**. ZMQ PUB/SUB drops messages that arrive before a
  subscriber has connected (the "slow joiner" problem). The publisher also sleeps
  500 ms before its first send to make this more forgiving.
- `subscribe("DEMO")` matches the ZMQ prefix `"DEMO"` — the header wire-format is
  `protocol_name | source`, so any message with `protocol_name == "DEMO"` passes.
- The subscriber fills in a `req_endpoint` too because `ProtocolClient::bind` always
  initializes both sockets. The REQ socket is never used here; `connect()` is lazy,
  so a non-listening endpoint is fine.

## See also

- Guide: `doc/library/services.md`
- App: `apps/ProtocolPusher/ProtoPusherMain.cpp` (essentially this example, inlined in the app tree).
