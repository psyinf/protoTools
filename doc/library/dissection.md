# Dissection

The **dissector** layer of `protos` slices a byte stream into named fields according
to a declarative template. It is byte-exact and endian-agnostic — it does not turn
bytes into numbers, strings, or anything else. That is the [interpreter](./interpretation.md)'s
job.

## Mental model

Three data types do all the work:

- `PacketDescriptor` — an ordered list of `FieldDescriptor`s plus an optional
  packet name. Declarative. Reusable.
- `FieldDescriptor` — metadata for one field: name, size, and optional rules that
  make the field's size or existence depend on what was parsed earlier.
- `GenericDissector` — a small state machine fed one or more bytes at a time.
  Returns `std::optional<PacketData>`; non-empty when a full packet has arrived.

```
 raw bytes ──▶ GenericDissector::addBytes() ──▶ PacketData { FieldData[] }
                     ▲
                     │
                     owns PacketDescriptor (the template)
```

Header file: `libs/protos/dissection/GenericDissector.hpp`.

## Minimal example

```cpp
#include <protos/dissection/GenericDissector.hpp>
#include <protos/dissection/PacketDescriptor.hpp>

using namespace protos::dissector;

PacketDescriptor tmpl;
tmpl.add({.name = "header", .size = 4});
tmpl.add({.name = "count",  .size = 4});

GenericDissector d(tmpl);
auto packet = d.addBytes(some_bytes_span);
if (packet) {
    auto& h = packet->get("header");   // FieldData
    auto& c = packet->get("count");
}
```

Full runnable version: [`examples/01_simple_dissector`](../../examples/01_simple_dissector/).

## FieldDescriptor features

Defined in `libs/protos/dissection/FieldDescriptor.hpp`. All fields are plain data
members; use designated initializers to set what you need.

### 1. Fixed-size field

```cpp
tmpl.add({.name = "x", .size = sizeof(double)});
```

The simplest case. The dissector consumes exactly `size` bytes and moves on.

### 2. Length-prefixed variable field

Use `determinesSizeOf` on the length field, and `size = 0` on the field it
governs. When the length field is parsed its value becomes the size of the named
field.

```cpp
tmpl.add({.name = "len",     .size = 1, .determinesSizeOf = "payload"});
tmpl.add({.name = "payload", .size = 0});  // filled in from len
```

The length field can be 1, 2, 4, or 8 bytes — those are the widths supported by
`GenericDissector::getSizeFromFieldValue`. Other widths throw at runtime.

Runnable: [`examples/02_length_prefixed_dissector`](../../examples/02_length_prefixed_dissector/).

### 3. Optional / conditional field

`sizeDeterminesExistenceOf` makes a *later* field vanish when the length field
evaluates to zero. Useful for "no payload ⇒ no CRC" patterns.

```cpp
tmpl.add({.name = "len",  .size = 1,
          .determinesSizeOf          = "data",
          .sizeDeterminesExistenceOf = "crc"});
tmpl.add({.name = "data", .size = 0});
tmpl.add({.name = "crc",  .size = 1});
```

When `len == 0`, `packet->has("crc")` is false and the field is not even present
in the result. Runnable: [`examples/03_optional_field_dissector`](../../examples/03_optional_field_dissector/).

### 4. Header-match field

Set `isHeaderValue = true` and provide an expected `value`. `GenericDissector::addByte`
resynchronises the stream if the incoming bytes do not match the expected value —
it drops one byte from the front of the buffer and keeps trying. Combine with
`GenericDissector::matchesHeader(header_bytes)` to pick between candidate
descriptors before committing to a full dissection.

```cpp
FieldDescriptor magic{.name = "magic", .size = 4, .isHeaderValue = true};
magic.withValue(std::vector<std::byte>{std::byte{'A'}, std::byte{'A'},
                                       std::byte{'A'}, std::byte{'A'}});
tmpl.add(std::move(magic));
```

**Caveat:** the fast path used by `addBytes` for entirely fixed-size packets does
not perform the `isHeaderValue` comparison — it just slices the bytes. If strict
magic-byte validation matters, either call `matchesHeader` first, or feed the
bytes one at a time via `addByte`.

Runnable: [`examples/05_header_match_resync`](../../examples/05_header_match_resync/).

### 5. Self-describing length (callback)

The most flexible option. Attach a `SizeCalcCallback` that inspects the buffer
accumulated so far and decides when the field is complete.

```cpp
using protos::dissector::FieldDescriptor;

auto length_byte = [](const FieldDescriptor&, std::span<std::byte> buf) {
    // buf[0] holds the total length including itself.
    return FieldDescriptor::SizeCallCallbackResult{
        /*need_more_bytes=*/false,
        /*result_value=*/   static_cast<uint16_t>(static_cast<unsigned>(buf[0]) - 1),
        /*bytes_consumed=*/ 1};
};

tmpl.add({.name = "payload", .size = 1, .externalSizeCalculation = length_byte});
```

Fields of `SizeCallCallbackResult`:

| Field             | Meaning                                                                 |
|-------------------|-------------------------------------------------------------------------|
| `need_more_bytes` | Set `true` to tell the dissector to call again after receiving another byte. |
| `result_value`    | Total size (bytes) of the field once you know it.                       |
| `bytes_consumed`  | Prefix bytes of `buf` that are framing, not part of the field's value. Those bytes are erased from the buffer; the dissector counts only the remaining `result_value` bytes toward the field's size. |

The minimum `size` declared on the field is the earliest point the callback is
consulted. Runnable: [`examples/04_self_describing_field`](../../examples/04_self_describing_field/)
(both length-in-first-byte and null-terminator variants).

### 6. `sizeOffset`

Passed as the second argument to `withDeterminesSizeOf(other, offset)`; the
resolved size is `raw_value + offset`. Use it when a length field counts something
other than raw bytes (e.g. words).

### 7. Preset values with `withValue`

`withValue<T>(v)` (arithmetic types) and `withValue(vector<std::byte>)` write into
`FieldDescriptor::value`. Used both for header matching (above) and for packet
*construction* via `PacketHelper::packetDescriptorToBuffer`.

## PacketData — the result

Returned as `std::optional<PacketData>` from `addByte`/`addBytes`.

```cpp
auto packet = dissector.addBytes(bytes);
if (!packet) { /* not yet complete */ }

packet->name;            // copied from PacketDescriptor::name
packet->fields;          // std::vector<FieldData>
packet->has("crc");      // bool
auto& fd = packet->get("x"); // FieldData (throws if absent)
fd.value;                // std::vector<std::byte>
```

Convert raw field bytes to typed values with the helpers in
`libs/protos/common/BitUtils.hpp`:

```cpp
auto n   = protos::bytes::to_number<uint32_t>(fd.value);   // host byte order
auto str = protos::bytes::as_chars_span(fd.value);         // std::span<const char>
```

`to_number<T>` uses `std::bit_cast` and therefore reads the bytes in **host** byte
order. For BE-on-LE or LE-on-BE, see [endianness](./endianness.md).

## PacketHelper utilities

`libs/protos/dissection/PacketHelper.hpp` — free functions marked experimental in
the source (`"you're on your own here"`). Useful for serialising a descriptor back
to bytes:

- `getPacketSize(PacketDescriptor&)` / `getPacketSize(PacketData&)`
- `packetDataToBuffer(PacketData&)` → `std::vector<std::byte>`
- `packetDescriptorToBuffer(PacketDescriptor&, swap_endian=false)` → bytes
- `packetDescriptorToBufferExcluding(PacketDescriptor&, excluded_name)`
- `packetFromDescriptor(PacketDescriptor&)` → seed a `PacketData` with the
  descriptor's preset values.

`swap_endian = true` in `packetDescriptorToBuffer` reverses each field's value
bytes as it writes them — useful when serialising a host-order descriptor to a BE
wire format.

## Persisting descriptors

`libs/protos/serializer/GenericDissectorSerializer.hpp` provides JSON round-trip
for `FieldDescriptor` and `PacketDescriptor` plus two helpers:

```cpp
#include <protos/serializer/GenericDissectorSerializer.hpp>
using namespace protos::dissector;

save("icd.json", my_packet_descriptor);
auto loaded = load<PacketDescriptor>("icd.json");
```

The shape-only subset is serialized: `name`, `description`, `size`, `value`,
`determinesSizeOf`, `sizeDeterminesExistenceOf`, `isHeaderValue`, `sizeOffset`.
`externalSizeCalculation` is a `std::function` and is **not** persisted —
re-attach any size callbacks in code after `load()`. Byte values are encoded
as `"0xNN"` strings (`["0x49", "0x43", "0x44", "0x31"]`) — human-readable for
eyeballing ICD descriptors. Parsing is case-insensitive and accepts the `0x`
prefix optionally.

## Related

- [Interpretation](./interpretation.md) — turn `PacketData` into typed, formatted results.
- [Endianness](./endianness.md) — the BE/LE story end-to-end.
- [ICD cookbook](./icd-cookbook.md) — recipes for common protocol patterns.
- Source tests: `tests/DissectorTests/ProtocolDissectorTests.cpp` — matching
  scenarios for every feature on this page.
