# ProtocolClient

Interactive console client. Queries a directory server, subscribes to every
protocol it finds, prints each received message, and reacts to single-key
keyboard commands.

## CLI

```
ProtocolClient [--service_discovery <host>]
```

| Flag                        | Default     | Effect                                    |
|-----------------------------|-------------|-------------------------------------------|
| `-s, --service_discovery`   | `localhost` | Hostname of the `ProtocolDirectoryServer` |

Talks to the directory at `tcp://<host>:9999` (REQ/REP) and `tcp://<host>:9998`
(PUB reserved for future use).

## Run

Terminal 1: `./ProtocolDemoServer` (or any server that registers in a directory).

Terminal 2: `./ProtocolClient`.

## Interactive keys

| Key | Effect                                                                       |
|-----|------------------------------------------------------------------------------|
| `s` | Sends `{"SEND", "ECHO", "Hullo 0"}` to the current publisher's command endpoint. |
| `c` | Sends `{"CONNECT", "ECHO", "Hullo 0"}`.                                      |
| `a` | Sends `{"CONNECT", <name>, ""}` to every discovered protocol.                |
| `q` | Quits.                                                                       |

Messages received via subscription are printed continuously on a detached
`std::jthread`.

## Output sample

```
[info] Starting Protocol client
[info] Querying protocols from directory at "localhost"
Found 1 protocols
[info] Protocol: [CAN-USB_CAN] publisher:tcp://127.0.0.1:41000 | cmd-req: tcp://*:41001
[info] Binding to publisher tcp://127.0.0.1:41000 and command endpoint tcp://*:41001
[info] Subscribed to CAN
Received: CAN[CAN_USB]
Message (CAN) 0
```

## Source

`apps/ProtocolClient/ProtocolClientMain.cpp`.

## Windows-only note

Uses `conio.h`'s `_getch`/`getch` for non-echoed keyboard input. Straight port to
POSIX requires swapping that for termios raw-mode reads.

## See also

- [apps/protocol-demo-server](./protocol-demo-server.md)
- [library/services](../library/services.md)
