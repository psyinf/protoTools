# 02 — Length-Prefixed Dissector

Uses `FieldDescriptor::determinesSizeOf` to let one field's value define the length of
the next field. This is the classic TLV / Pascal-string pattern.

## What it shows

- `len` is 1 byte and declares `determinesSizeOf = "payload"`.
- `payload` is declared with `size = 0`; the dissector fills in its real size when `len`
  is parsed.
- A subsequent fixed-size `trailer` continues normally.

## Run

```
./ex02_length_prefixed_dissector
```

Expected output:

```
len          = 5
payload      = hello
trailer(hex) = deadbeef
```

## See also

- Guide: `doc/library/dissection.md` § "Length-prefixed variable field"
- Source test: `tests/DissectorTests/ProtocolDissectorTests.cpp` — "dependent size struct"
