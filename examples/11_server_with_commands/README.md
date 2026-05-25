# 11 — Server With Commands

`ProtocolServer` composes a publisher (PUB socket) with a command handler (REP
socket run on a detached thread). The callback registered via `setCommandCallback`
runs for every inbound `Command` and returns a `CommandReply`.

## Run (two terminals)

```
# Terminal A
./ex11_server

# Terminal B
./ex11_client
```

Expected client output:

```
reply: ACK counter=1
reply: ACK counter=2
reply: ACK counter=3
```

Expected server output (interleaved with heartbeat sends):

```
cmd: BUMP -> counter ()
cmd: BUMP -> counter ()
cmd: BUMP -> counter ()
```

## See also

- Guide: `doc/library/services.md`
- App: `apps/ProtocolDemoServer/ProtocolDemoServer.cpp` — same shape, with directory registration added.
