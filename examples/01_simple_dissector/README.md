# 01 — Simple Dissector

Minimum working example of `GenericDissector` with three fixed-size fields.

## What it shows

- Building a `PacketDescriptor` with `.add({.name, .size})`.
- Feeding a complete buffer with `addBytes()` and reading the resulting `PacketData`.
- Converting field bytes to typed values with `protos::bytes::to_number<T>()` and
  `protos::bytes::as_chars_span()`.

## Run

```
./ex01_simple_dissector
```

Expected output:

```
header = PROT
count  = 42
x      = 3.14159
```

## See also

- Guide: `doc/library/dissection.md`
- Source: `libs/protos/dissection/GenericDissector.hpp`, `libs/protos/common/BitUtils.hpp`
