# 03 — Optional Trailing Field

Uses `FieldDescriptor::sizeDeterminesExistenceOf` to make a trailing field conditional
on the length prefix. Common for protocols that omit a CRC on empty records.

## What it shows

- When `len == 0`, the dissector skips `crc` entirely — `packet->has("crc")` is false.
- When `len > 0`, `crc` consumes its byte normally.

## Run

```
./ex03_optional_field_dissector
```

Expected output:

```
empty:
  len      = 0
  data.sz  = 0
  has crc? = no
with-data:
  len      = 3
  data.sz  = 3
  has crc? = yes
```

## See also

- Guide: `doc/library/dissection.md` § "Optional / conditional field"
- Source test: `tests/DissectorTests/ProtocolDissectorTests.cpp` — "dependent removal struct"
