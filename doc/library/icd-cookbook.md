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

---

## Worked example: a two-message ICD

Closing capstone that composes the recipes above into one end-to-end sketch.
Two messages share a 4-byte envelope (sync word, message ID, length), both
big-endian on the wire:

- **Sensor Status** (`msg_id = 0x01`) — `UInt32` timestamp, scaled `Int16`
  temperature (÷ 100 → °C), `UInt8` bit-mask (error / warning / calibrated).
- **Vehicle Telemetry** (`msg_id = 0x05`) — embedded `Vector3` position
  (three scaled `Int16`s), `UInt8` battery, `UInt32` checksum trailer.

### Shared `Vector3` sub-dissector

Define once, reuse across every message that embeds the struct:

```cpp
PacketDescriptor vec3;
vec3.name = "Vector3";
vec3.add({.name = "x", .size = 2});
vec3.add({.name = "y", .size = 2});
vec3.add({.name = "z", .size = 2});

GenericPacketInterpreter vec3_interp;
auto scaled_coord = [](const std::string& name) {
    return FieldInterpretation::Mapper{[=](const Variant& v) -> InterpretationResults {
        double val = static_cast<double>(std::get<int64_t>(v)) / 100.0;
        return {{{name, Variant{val}}, "{:.2f}", false}};
    }};
};
// Designators must be in struct declaration order: mapper before littleEndian.
vec3_interp.addField({.name = "x", .type = Type::INTEGER, .mapper = scaled_coord("x"), .littleEndian = false});
vec3_interp.addField({.name = "y", .type = Type::INTEGER, .mapper = scaled_coord("y"), .littleEndian = false});
vec3_interp.addField({.name = "z", .type = Type::INTEGER, .mapper = scaled_coord("z"), .littleEndian = false});
```

### Helper: shared envelope

The magic-match header is identical across messages apart from the one `msg_id`
byte. Factor it into a small builder:

```cpp
FieldDescriptor header_field(const std::string& name, uint8_t size, std::vector<std::byte> expected)
{
    FieldDescriptor f{.name = name, .size = size, .isHeaderValue = true};
    f.withValue(std::move(expected));
    return f;
}

PacketDescriptor make_envelope(const std::string& name, std::byte msg_id)
{
    PacketDescriptor pd;
    pd.name = name;
    pd.add(header_field("sync",   2, {std::byte{0xAB}, std::byte{0xCD}}));
    pd.add(header_field("msg_id", 1, {msg_id}));
    pd.add({.name = "length", .size = 1, .determinesSizeOf = "payload"});
    pd.add({.name = "payload", .size = 0});
    return pd;
}
```

### Message 1 — Sensor Status (0x01)

```cpp
auto sensor_status = make_envelope("SensorStatus", std::byte{0x01});

// Inner payload descriptor (4 + 2 + 1 = 7 bytes).
PacketDescriptor sensor_body;
sensor_body.add({.name = "timestamp",   .size = 4});
sensor_body.add({.name = "temperature", .size = 2});
sensor_body.add({.name = "flags",       .size = 1});

GenericPacketInterpreter sensor_interp;
sensor_interp.addField({.name = "timestamp", .type = Type::UNSIGNED_INTEGER,
                        .format = "{} ms", .littleEndian = false});

// Signed scaled-integer mapper: Int16 on the wire, raw ÷ 100 = °C.
sensor_interp.addField({.name = "temperature", .type = Type::INTEGER,
    .mapper = [](const Variant& v) -> InterpretationResults {
        double c = static_cast<double>(std::get<int64_t>(v)) / 100.0;
        return {{{"temp_c", Variant{c}}, "{:.2f}", false}};
    },
    .littleEndian = false});

// Fan-out mapper: one byte, three named booleans.
sensor_interp.addField({.name = "flags", .type = Type::UNSIGNED_INTEGER,
    .mapper = [](const Variant& v) -> InterpretationResults {
        auto b = std::get<uint64_t>(v);
        return {
            {{"error",      Variant{(b & 0x01) != 0}}, "{}", false},
            {{"warning",    Variant{(b & 0x02) != 0}}, "{}", false},
            {{"calibrated", Variant{(b & 0x04) != 0}}, "{}", false},
        };
    }});
```

### Message 2 — Vehicle Telemetry (0x05)

```cpp
auto vehicle_telemetry = make_envelope("VehicleTelemetry", std::byte{0x05});

// Inner payload: 6-byte opaque position + 1-byte battery + 4-byte checksum.
PacketDescriptor vehicle_body;
vehicle_body.add({.name = "position", .size = 6});   // dissected separately via vec3
vehicle_body.add({.name = "battery",  .size = 1});
vehicle_body.add({.name = "checksum", .size = 4});

GenericPacketInterpreter vehicle_interp;
// 'position' is consumed by the vec3 sub-dissector after the outer pass;
// don't register a direct interpreter for it (or register a BYTES hidden one).
vehicle_interp.addField({.name = "position", .type = Type::BYTES, .hidden = true});
vehicle_interp.addField({.name = "battery",  .type = Type::UNSIGNED_INTEGER, .format = "{}%"});
vehicle_interp.addField({.name = "checksum", .type = Type::UNSIGNED_INTEGER,
                         .format = "{:#010x}", .littleEndian = false});
```

### Dispatch across descriptors

When a stream can carry either message, probe the first few bytes with
`matchesHeader` and feed the winner:

```cpp
std::optional<PacketData> dispatch(std::span<const std::byte> wire)
{
    // Need at least sync(2) + msg_id(1) to disambiguate.
    if (wire.size() < 3) { return std::nullopt; }
    std::vector<std::byte> probe(wire.begin(), wire.begin() + 3);

    GenericDissector d_sensor(sensor_status);
    if (d_sensor.matchesHeader(probe)) {
        return d_sensor.addBytes(wire);
    }

    GenericDissector d_vehicle(vehicle_telemetry);
    if (d_vehicle.matchesHeader(probe)) {
        return d_vehicle.addBytes(wire);
    }

    return std::nullopt;  // unknown message, drop or log
}
```

### Interpreting the result

```cpp
auto packet = dispatch(bytes_from_socket);
if (!packet) { /* unknown or short */ return; }

if (packet->name == "SensorStatus") {
    PacketDescriptor body_tmpl = sensor_body;  // or keep one long-lived instance
    GenericDissector inner(body_tmpl);
    auto body = inner.addBytes(packet->get("payload").value);
    auto results = sensor_interp.interpretPacketData(*body);
    // ... render results ...
}
else if (packet->name == "VehicleTelemetry") {
    GenericDissector inner(vehicle_body);
    auto body = inner.addBytes(packet->get("payload").value);
    auto results = vehicle_interp.interpretPacketData(*body);

    // Position nested: dissect+interpret the 6-byte blob on its own.
    GenericDissector pos_diss(vec3);
    auto pos = pos_diss.addBytes(body->get("position").value);
    auto pos_results = vec3_interp.interpretPacketData(*pos);
    results.insert(results.end(), pos_results.begin(), pos_results.end());
    // ... render merged results ...
}
```

### Checksum — protoTools' scope ends here

The `checksum` field parses fine, but protoTools does **not** compute CRCs.
Verification is the caller's job: slice the bytes to be covered (typically
everything up to the checksum field) and run your algorithm of choice. The
`PacketHelper` utilities can help assemble the covered region, but both the
algorithm choice and the compare step live in application code.

### Cross-references

Recipes composed in this capstone:

- [Fixed header + variable payload](#fixed-header--variable-payload) — envelope.
- [Length-prefixed record](#length-prefixed-record-list) — `determinesSizeOf`.
- [Mixed-endian fields](#mixed-endian-fields) — `.littleEndian = false`.
- [Enumerated / scaled fixed-point](#scaled-fixed-point) — the temperature
  and Vector3 coordinate mappers, adapted to signed `int64_t`.
- [Bit flags from one byte](#bit-flags-from-one-byte) — the status byte.
- [Nested / embedded sub-protocol](#nested--embedded-sub-protocol) — Vector3.

Related examples and docs:

- [`examples/05_header_match_resync/`](../../examples/05_header_match_resync/)
  — the `matchesHeader` primitive in isolation.
- [`examples/07_icd_mixed_endian/`](../../examples/07_icd_mixed_endian/) —
  the BYTES + swap-mapper workaround that `.littleEndian = false` replaces
  once `feat/little-endian` merges.
- [`doc/library/endianness.md`](./endianness.md) — what the flag means and
  the history of when it did / didn't work.
