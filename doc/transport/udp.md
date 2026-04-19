# UDP Transport

`protoTools` ships a ZMQ-based service layer, but the dissection and
interpretation libraries do not care how bytes arrive. This guide shows the
smallest possible wiring between a plain OS UDP socket and
`GenericDissector` + `GenericPacketInterpreter`.

Runnable pair (sender + receiver, Winsock and POSIX):
[`examples/09_udp_icd_receiver`](../../examples/09_udp_icd_receiver/).

## Why UDP directly

- **ICD-defined wire formats often specify UDP** rather than a messaging layer.
- `protos_services` adds a ZMQ dependency, framing overhead, and a header
  contract (`protocol_name|source` prefix) you may not want in a raw UDP
  deployment.
- `protos` (the core lib) has no ZMQ dependency. Link only `protos::protos` and
  you have the parser without any transport baggage.

## Minimum receiver

```cpp
#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>
#include <protos/interpretation/GenericPacketInterpreter.hpp>
#include <protos/interpretation/TypeFormatter.hpp>

// OS sockets (POSIX). For Windows, call WSAStartup and link ws2_32.
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace protos::dissector;
using namespace protos::interpreter;
using namespace protos::value;

int main() {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{AF_INET, htons(5555), {htonl(INADDR_ANY)}};
    bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    PacketDescriptor tmpl;
    tmpl.add({.name = "seq",  .size = 2});
    tmpl.add({.name = "data", .size = 6});

    GenericPacketInterpreter interp;
    interp.addField({.name = "seq",  .type = Type::UNSIGNED_INTEGER, .format = "{}"});
    interp.addField({.name = "data", .type = Type::BYTES,            .format = "{}"});

    char buf[512];
    while (true) {
        ssize_t n = recv(s, buf, sizeof(buf), 0);
        if (n <= 0) break;

        GenericDissector d(tmpl);  // fresh per datagram
        auto bytes = std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(buf), static_cast<size_t>(n));
        if (auto pkt = d.addBytes(bytes)) {
            for (auto& r : interp.interpretPacketData(*pkt)) {
                // render r...
            }
        }
    }
}
```

The only protoTools concept that appears is
`GenericDissector::addBytes(std::span<std::byte>)`. Everything else is vanilla
Berkeley sockets.

## Datagram vs. stream

- **UDP is datagram-oriented.** Each `recvfrom` returns a complete packet. A
  fresh `GenericDissector` per datagram is the simplest model and keeps state
  from one packet from leaking into the next if framing is corrupt.
- **If datagrams pack multiple records** (repeated fixed-size records with no
  outer length), you can keep one dissector alive and call `addBytes` in a loop,
  harvesting each `std::optional<PacketData>` that returns non-empty.
- **TCP / serial streams**, when you get to those, benefit from the byte-at-a-time
  `addByte` interface — the dissector is designed to return `std::nullopt` until
  a full packet arrives.

## Windows specifics

On Windows, prepend:

```cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

WSADATA wsa;
WSAStartup(MAKEWORD(2, 2), &wsa);
```

and replace `close()` with `closesocket()`. The example
[`examples/09_udp_icd_receiver`](../../examples/09_udp_icd_receiver/) has the
cross-platform `#ifdef _WIN32` shim ready to copy.

## Endianness

UDP wire formats frequently specify network byte order (big-endian). On an LE
host, that means every multi-byte number needs a swap — see
[endianness](../library/endianness.md) for the mapper-based workaround, and
[`examples/07_icd_mixed_endian`](../../examples/07_icd_mixed_endian/) for a
full mixed-order record.
