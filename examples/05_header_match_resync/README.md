# 05 — Header Match

`GenericDissector::matchesHeader(header_bytes)` answers a cheap question: would this
descriptor's magic-value fields accept the given start-of-packet bytes? Use it to
pick between multiple candidate `PacketDescriptor`s before committing to a full
dissection.

## What it shows

- `FieldDescriptor::isHeaderValue = true` combined with a preset `.value` declares a
  field whose bytes must match exactly.
- Two descriptors with different magic bytes (`AAAA` vs `BBBB`) disambiguate by
  calling `matchesHeader` on a 4-byte window.

## Run

```
./ex05_header_match_resync
```

Expected output:

```
window matches A? no
window matches B? yes
packet.name=BBBB body[0]=0x1
```

## See also

- Guide: `doc/library/dissection.md` § "Header-match field"
- Related: `apps/ProtocolClient/ProtocolClientMain.cpp` for a dispatch-by-protocol example.
