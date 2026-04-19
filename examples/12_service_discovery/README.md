# 12 — Service Discovery

`ProtocolDirectoryServer` is a REP/PUB server that holds a map of
`ProtocolDirectoryEntry` records. `ProtocolDirectoryClient` queries it via the REQ
socket.

## Run (two terminals)

```
# Terminal A
./ex12_dir_server

# Terminal B
./ex12_dir_client
```

Expected client output:

```
found 2 protocols:
  CAN|USB_CAN  pub=tcp://127.0.0.1:41000  cmd=tcp://127.0.0.1:41001
  MODBUS|TCP  pub=tcp://127.0.0.1:42000  cmd=tcp://127.0.0.1:42001
```

## See also

- Guide: `doc/library/services.md` § "Directory"
- App: `apps/ProtocolDemoServer/ProtocolDemoServer.cpp` — registers its own protocol in an embedded directory.
- App: `apps/ProtocolClient/ProtocolClientMain.cpp` — queries the directory then subscribes to what it finds.
