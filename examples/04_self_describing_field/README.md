# 04 — Self-Describing Field

Most powerful dissection feature: attach an arbitrary callback that inspects the
bytes-so-far and decides when the field is complete.

## What it shows

Two common patterns implemented with `FieldDescriptor::externalSizeCalculation`:

- **Length-in-first-byte** — first byte is the total length including itself;
  `bytes_consumed = 1` marks the length byte as framing so it is not included in
  the payload value.
- **Null-terminated** — scan the buffer until a `0x00` is found; return `need_more_bytes=true`
  until the terminator arrives. `bytes_consumed = 0` because every byte (including the
  terminator) is part of the field value.

## Run

```
./ex04_self_describing_field
```

Expected output:

```
A length-in-first-byte: field1=0x1 payload(4)="helo"
B null-terminated      : field1=0x1 payload(5)="helo "
```

In case B the payload size is 5 because the null terminator is part of the field
value. The printed trailing character is the embedded `\0` (shows as blank on
most terminals).

## See also

- Guide: `doc/library/dissection.md` § "Self-describing length"
- Source tests: `tests/DissectorTests/ProtocolDissectorTests.cpp` — two `data_type_with_selfdescribing_length*` cases
