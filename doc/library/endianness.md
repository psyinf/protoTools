# Endianness

Where in the pipeline does byte order matter, and what does `protoTools` do about it?

## One-line answer

**Dissection is byte-order-agnostic. Interpretation is host-byte-order today.** The
declared `FieldInterpretation::littleEndian` flag is not yet honored; for fields
whose wire order differs from the host, attach a `mapper` that swaps the bytes.

## Dissection is transparent

`GenericDissector` slices a byte stream at declared offsets and stores each
field's bytes in a `std::vector<std::byte>` in **stream order** — exactly as they
arrived. Endianness never enters the picture here. An LE host and a BE host will
produce identical `PacketData` for the same wire bytes.

## Interpretation is host-order

Two places turn bytes into numbers:

1. `protos::bytes::to_number<T>(span)` in `libs/protos/common/BitUtils.hpp` —
   uses `std::bit_cast`, which is host byte order by definition.
2. `protos::value::as_variant(Type, bytes)` in
   `libs/protos/interpretation/TypeFormatter.hpp` — same story; dispatches to
   `convert<Type>` overloads that `std::bit_cast` a pointer to the correct width.

`FieldInterpretation::littleEndian` is declared (default `true`) and *should*
control this, but `FieldInterpreter::interpret` in
`libs/protos/interpretation/FieldInterpreter.cpp` never reads it. This is a known
gap — see [the note in the interpretation guide](./interpretation.md#littleendian--documented-but-not-honored).

## What to do today (LE host, mixed wire)

On x86 and Windows ARM the host is little-endian. LE fields work natively — pick
the matching `Type` and you are done. For BE fields, you need to intervene.

### Option A — mapper that reverses and reinterprets

Most common pattern. Declare the field with `type = BYTES`; the interpreter's
initial conversion then produces a `std::vector<std::byte>` Variant. Your mapper
reverses and recasts:

```cpp
auto swap_u32 = [](const Variant& v) -> InterpretationResults {
    const auto& bs = std::get<std::vector<std::byte>>(v);
    std::vector<std::byte> rev(bs.rbegin(), bs.rend());
    uint32_t out{};
    std::memcpy(&out, rev.data(), sizeof(out));
    return {{{"seq", Variant{static_cast<uint64_t>(out)}}, "{}", false}};
};

interp.addField({.name = "seq_be", .type = Type::BYTES, .mapper = swap_u32});
```

Full runnable variant with three mixed-endian fields:
[`examples/07_icd_mixed_endian`](../../examples/07_icd_mixed_endian/).

### Option B — pre-reverse the FieldData bytes

If the same field is always BE and you control the code that calls the
interpreter, reverse the `FieldData::value` bytes before handing the packet off:

```cpp
auto& seq = packet->get("seq_be").value;   // unfortunately FieldData::value is const
// so construct a fresh PacketData with reversed bytes, then interpret that.
```

Since `FieldData::value` is const, this requires building a new `PacketData` with
reversed bytes rather than mutating in place. Verbose. Option A scales better.

### Option C — swap on serialization only

`PacketHelper::packetDescriptorToBuffer(pd, /*swap_endian=*/true)` reverses each
field's bytes when *producing* a buffer from a descriptor. This is the
complementary direction (host-native → BE wire), useful for transmitting. It is
marked experimental in the source; inspect the implementation before relying on
it for anything production-critical.

## Worked mixed-endian example

See [`examples/07_icd_mixed_endian`](../../examples/07_icd_mixed_endian/) for a
16-byte record with a BE `uint32_t` sequence number, LE `uint64_t` timestamp, and
BE `float` value — one descriptor, one interpreter, swap mappers only on the BE
fields.

## If/when `littleEndian` is wired up

The fix is a ~5-line change in `FieldInterpreter::interpret`: reverse the bytes
before calling `as_variant` when `interpretation.littleEndian == false` (or when
`littleEndian != std::endian::native == std::endian::little`). Once that lands,
the mappers in example 07 become unnecessary:

```cpp
// today
interp.addField({.name = "seq_be", .type = Type::BYTES, .mapper = swap_u32});

// post-fix
interp.addField({.name = "seq_be", .type = Type::UNSIGNED_INTEGER,
                 .littleEndian = false});
```

Keep an eye on that file if you are building long-lived ICD code here.
