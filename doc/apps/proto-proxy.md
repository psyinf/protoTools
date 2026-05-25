# ProtoProxy

Plain ZMQ XSUB ↔ XPUB relay. Publishers that would otherwise need a
stable bind address `connect()` to the proxy's frontend; subscribers
`connect()` to the backend. Subscriptions propagate across the proxy
automatically.

## Endpoints

| Socket type  | Endpoint           | Role                                 |
|--------------|--------------------|--------------------------------------|
| XSUB         | `tcp://*:55555`    | Frontend — publishers connect here   |
| XPUB         | `tcp://*:55556`    | Backend — subscribers connect here   |

## CLI

No flags.

## Run

```bash
./ProtoProxy
```

A publisher then uses `ProtoPublisherConfig{.pub_endpoint = "tcp://localhost:55555",
.is_proxy = true}` so `ProtocolPublisher::bind` calls `connect()` instead of
`bind()`. Subscribers point at `tcp://localhost:55556`.

## Why

Scales many-publishers → many-subscribers without every subscriber needing to
know every publisher's address. Also survives publishers coming and going
without subscribers having to reconnect.

## Source

`apps/ProtoProxy/ProtoProxyMain.cpp`. Essentially one `zmq::proxy(frontend,
backend, nullptr)` call with logging.

## See also

- [library/services](../library/services.md) § "Proxy"
- [ProtocolPublisher config](../library/services.md#protocolpublisher) — the
  `is_proxy` flag.
