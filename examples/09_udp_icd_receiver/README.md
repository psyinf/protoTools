# 09 — UDP ICD Receiver

Protos' dissector and interpreter consume `std::span<const std::byte>` — they don't
care about transport. This example shows the smallest possible glue between a UDP
socket and the library. Uses the OS socket APIs directly (Winsock on Windows,
POSIX sockets elsewhere); no ZMQ.

## What it shows

- Binding a UDP socket and looping on `recvfrom()`.
- Handing each datagram straight to `GenericDissector::addBytes(std::span<std::byte>)`.
- Interpreting with a scaled-temperature mapper (see example 08).

## Run (two terminals)

Terminal A — receiver (start first, it binds the port):

```
./ex09_udp_receiver
```

Terminal B — sender:

```
./ex09_udp_sender
```

Expected receiver output:

```
listening on 127.0.0.1:5555...
datagram 1:  seq=1  temp_c=20.5
datagram 2:  seq=2  temp_c=21.0
datagram 3:  seq=3  temp_c=21.5
datagram 4:  seq=4  temp_c=22.0
```

## Datagram vs stream notes

- UDP is datagram-oriented. Each `recvfrom()` is a complete packet — so a fresh
  `GenericDissector` per datagram is the simplest model.
- For protocols that pack multiple records per datagram, you can keep one dissector
  alive across records and call `addBytes` repeatedly; `addBytes` returns a
  `std::optional<PacketData>` that is non-empty once a full packet has arrived.
- For TCP (stream-oriented), the byte-by-byte `addByte` API is the natural fit.

## See also

- Guide: `doc/transport/udp.md` — longer-form walkthrough of this pattern.
