# ProtocolPusher

Smallest possible binary that exercises `ProtocolPublisher`. Binds a PUB socket
on `tcp://*:5555` and emits a short string every 500 ms under the `"CAN"`
protocol, adapter `"USB-CAN"`.

## CLI

No flags. Endpoint and cadence are hardcoded.

## Run

```bash
./ProtocolPusher
```

Subscribe from anything that speaks the two-frame ZMQ PUB format — e.g. a bare
`zmq::socket_t(sub)` in another process or the `ProtocolClient` app pointed at
this endpoint.

## Source

`apps/ProtocolPusher/ProtoPusherMain.cpp`. Forty lines, no dependencies beyond
`libs/services`.

## See also

- [library/services](../library/services.md#protocolpublisher)
- Example: [`10_pub_sub_minimal`](../../examples/10_pub_sub_minimal/) — the same
  idea as two separate executables, with a subscriber you can run alongside.
