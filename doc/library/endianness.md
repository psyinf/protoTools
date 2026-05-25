# Endianness

Where in the pipeline does byte order matter, and how does `protoTools` handle it?

## One-line answer

**Dissection is byte-order-agnostic. Interpretation honors
`FieldInterpretation::littleEndian` for numeric types.** Declare each BE field
with `.littleEndian = false`; on an LE host the interpreter reverses those
fields' bytes before decoding. LE fields (default) pass through natively.

## Dissection is transparent

`GenericDissector` slices a byte stream at declared offsets and stores each
field's bytes in a `std::vector<std::byte>` in **stream order** — exactly as
they arrived. Endianness never enters the picture here. An LE host and a BE
host produce identical `PacketData` for the same wire bytes.

## Interpretation respects the flag

When `FieldInterpreter::interpret` sees a numeric type (INTEGER,
UNSIGNED_INTEGER, FLOAT) and the declared `littleEndian` disagrees with
`std::endian::native`, it reverses the field's bytes before calling
`as_variant`. The result round-trips correctly on any host.

Non-numeric types are not affected:

- `STRING` keeps stream order — reversing characters would corrupt text.
- `BYTES` is the raw escape hatch; a caller reaching for `BYTES` has already
  opted into manual handling via a `mapper`.
- `BOOL` is a single byte, so the flag is a no-op.

## How to use it

```cpp
GenericPacketInterpreter interp;
interp.addField({.name = "seq_be", .type = Type::UNSIGNED_INTEGER,
                 .littleEndian = false});
interp.addField({.name = "ts_le",  .type = Type::UNSIGNED_INTEGER});
interp.addField({.name = "val_be", .type = Type::FLOAT,
                 .littleEndian = false});
```

Full runnable demo — a 16-byte record with one LE and two BE fields:
[`examples/07_icd_mixed_endian`](../../examples/07_icd_mixed_endian/).

## When you still need a mapper

The flag-based swap only applies to numeric types. For everything else a
`mapper` remains the right tool:

- Decoding a custom BCD or fixed-point encoding that is not a straight
  numeric reinterpretation.
- Re-ordering arbitrary byte blobs (for example extracting an embedded
  sub-struct where the outer field is `BYTES`).
- Packet-level transforms that depend on multiple fields.

See the mapper recipes in [icd-cookbook](./icd-cookbook.md#enumerated-fields)
and [`examples/08_interpreter_mappers`](../../examples/08_interpreter_mappers/).

## Serialisation (host → wire)

`PacketHelper::packetDescriptorToBuffer(pd, /*swap_endian=*/true)` reverses
each field's bytes when *producing* a buffer from a descriptor. The helper is
marked experimental in the source; inspect the implementation before relying
on it in production, and prefer writing the fields yourself if you need
precise control.

## Historical note

`FieldInterpretation::littleEndian` was declared from the start but, until
recently, never consulted — ICDs with BE fields on LE hosts needed a
`BYTES` + swap-mapper workaround. That code still works and remains correct,
it just became unnecessary for numeric fields once the flag was wired up in
`FieldInterpreter::interpret`.
