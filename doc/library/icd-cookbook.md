# ICD Cookbook

Task-oriented recipes for parsing binary protocols defined by an Interface
Control Document. Each recipe pairs a short descriptor snippet with the
`FieldDescriptor` / `FieldInterpretation` features it leans on. If you need the
underlying concepts, start with [dissection](./dissection.md) and
[interpretation](./interpretation.md).

---

## Fixed header + variable payload

ICD: a 4-byte magic, 1-byte version, 2-byte length, then `length` bytes of
payload.

```cpp
PacketDescriptor tmpl;

FieldDescriptor magic{.name = "magic", .size = 4, .isHeaderValue = true};
magic.withValue(std::vector<std::byte>{std::byte{'I'}, std::byte{'C'},
                                       std::byte{'D'}, std::byte{'1'}});
tmpl.add(std::move(magic));

tmpl.add({.name = "version", .size = 1});
tmpl.add({.name = "length",  .size = 2, .determinesSizeOf = "payload"});
tmpl.add({.name = "payload", .size = 0});
```

`isHeaderValue` drops bytes from the front of the stream until the magic aligns.

---

## Length-prefixed record list

A common "header says how many records follow" pattern. One descriptor parses
the header; a second parses each record separately.

```cpp
PacketDescriptor envelope;
envelope.add({.name = "count",   .size = 1});
envelope.add({.name = "records", .size = 0,
              .externalSizeCalculation =
                  [](const FieldDescriptor&, std::span<std::byte> buf) {
                      // Each record is 8 bytes. We don't know `count` here,
                      // so close the outer packet at (count * 8) bytes - but
                      // we need to look at the previously-parsed count byte.
                      // See note below.
                      return FieldDescriptor::SizeCallCallbackResult{false, 0, 0};
                  }});
```

Because the callback does not receive the full `PacketData` built so far, the
cleanest idiom is to parse the count with one descriptor, then loop outside
`protoTools`, constructing a fresh `GenericDissector` per record.

---

## TLV (Type-Length-Value)

```cpp
PacketDescriptor tlv;
tlv.add({.name = "type",  .size = 1});
tlv.add({.name = "len",   .size = 2, .determinesSizeOf = "value"});
tlv.add({.name = "value", .size = 0});
```

Layer a mapper on `type` to dispatch different interpretations per value. Since
`mapper` has no visibility into the `value` bytes, do the interpreter swap in
your own code: keep one interpreter per `type` and pick by looking at the
returned `InterpretationResult` for `type`.

---

## CRC trailer (optional on empty payload)

```cpp
tmpl.add({.name = "len",  .size = 1,
          .determinesSizeOf          = "data",
          .sizeDeterminesExistenceOf = "crc"});
tmpl.add({.name = "data", .size = 0});
tmpl.add({.name = "crc",  .size = 1});
```

`packet->has("crc")` is false when `len == 0`. Runnable:
[`examples/03_optional_field_dissector`](../../examples/03_optional_field_dissector/).

---

## Reserved / padding bytes

Include them in the dissector (the bytes are on the wire and the parser has to
consume them) but hide them from the interpreter's output:

```cpp
tmpl.add({.name = "reserved", .size = 3});

interp.addField({.name = "reserved", .type = Type::BYTES, .hidden = true});
```

The `InterpretationResult` is produced with `hidden = true`; rendering code
(anything that iterates `results` for display) is responsible for checking the
flag and skipping.

---

## Mixed-endian fields

LE-on-LE-host fields need nothing. BE-on-LE-host fields need a swap mapper until
`FieldInterpretation::littleEndian` is honored — see [endianness](./endianness.md)
and [`examples/07_icd_mixed_endian`](../../examples/07_icd_mixed_endian/).

---

## Enumerated fields

Code-to-label translation via a mapper:

```cpp
auto state_labels = [](const Variant& v) -> InterpretationResults {
    std::string s;
    switch (std::get<uint64_t>(v)) {
        case 0: s = "IDLE"; break;
        case 1: s = "RUN";  break;
        case 2: s = "FAIL"; break;
        default: s = "UNKNOWN";
    }
    return {{{"state", Variant{s}}, "{}", false}};
};

interp.addField({.name = "state", .type = Type::UNSIGNED_INTEGER, .mapper = state_labels});
```

Runnable: [`examples/08_interpreter_mappers`](../../examples/08_interpreter_mappers/).

---

## Scaled fixed-point

Raw counts × scale factor → engineering units:

```cpp
auto temp_c = [](const Variant& v) -> InterpretationResults {
    double c = static_cast<double>(std::get<uint64_t>(v)) * 0.1;
    return {{{"temp_c", Variant{c}}, "{:.1f}", false}};
};

interp.addField({.name = "raw_temp", .type = Type::UNSIGNED_INTEGER, .mapper = temp_c});
```

---

## Bit flags from one byte

A mapper may return *many* results. One byte, eight named booleans:

```cpp
auto flags = [](const Variant& v) -> InterpretationResults {
    auto b = std::get<uint64_t>(v);
    return {
        {{"armed",   Variant{(b & 0x01) != 0}}, "{}", false},
        {{"ready",   Variant{(b & 0x02) != 0}}, "{}", false},
        {{"faulted", Variant{(b & 0x04) != 0}}, "{}", false},
    };
};
```

Runnable: [`examples/08_interpreter_mappers`](../../examples/08_interpreter_mappers/).

---

## Nested / embedded sub-protocol

Do this explicitly: parse the outer packet, take one field's bytes, feed them
to a second dissector.

```cpp
auto packet = outer_dissector.addBytes(wire);
auto results = outer_interp.interpretPacketData(*packet);

// Treat a specific outer field's bytes as a new packet to dissect.
auto& payload = packet->get("payload");
GenericDissector inner(inner_template);
auto inner_packet = inner.addBytes(payload.value);
auto inner_results = inner_interp.interpretPacketData(*inner_packet);
```

Historical note: `FieldInterpretation` used to carry a `std::string dissector{}`
member (resolved against a global `datafw::getRegistry<...>` at interpret time)
that did this automatically. It was removed in commit `bd2eb23` (15 Apr 2025)
along with the other string-based indirection mechanisms; the matching
`delegateToDissector` cpp logic is still present but `#ifdef EXTENDED_INTERPRETER`
gated and references `datafw::` symbols that no longer exist. The commented-out
line in `FieldInterpretation.hpp:23` is a design signpost, not a TODO.

---

## When none of these fit

If the protocol has cross-field constraints that the declarative model cannot
express (CRC validation, length-counting-in-words with a multiplier, variable
records whose count is embedded in a header field), the escape hatch is
`externalSizeCalculation`. It runs arbitrary C++ with access to the buffer so
far — you can implement whatever framing logic the ICD requires.
