# Interpretation

Where [dissection](./dissection.md) stops at *"these bytes belong to field X"*,
interpretation is where bytes become typed values (`uint64_t`, `double`, `string`,
`bool`, bitfields, enums, scaled engineering units). The pipeline is:

```
bytes ──▶ GenericDissector ──▶ PacketData ──▶ GenericPacketInterpreter ──▶ InterpretationResults
                                                     ▲
                                                     │
                                                     holds FieldInterpretation per field name
```

Header files:
- `libs/protos/interpretation/GenericPacketInterpreter.hpp`
- `libs/protos/interpretation/FieldInterpretation.hpp`
- `libs/protos/interpretation/InterpretationResult.hpp`
- `libs/protos/interpretation/TypeFormatter.hpp`

## FieldInterpretation — the per-field recipe

```cpp
struct FieldInterpretation {
    std::string         name;
    protos::value::Type type{Type::UNDEFINED};
    std::string         format = "{}";
    Mapper              mapper;            // std::function<Results(const Variant&)>

    std::vector<FieldInterpretation> subFields;   // experimental, see below
    bool littleEndian{true};                      // NOT currently honored - see below
    bool hidden{false};
};
```

### `type`

Drives the initial conversion of bytes to a typed `Variant`.

| `Type`             | Variant alternative       | How bytes are read                                    |
|--------------------|---------------------------|-------------------------------------------------------|
| `STRING`           | `std::string`             | Raw bytes copied as chars.                            |
| `INTEGER`          | `int64_t`                 | `std::bit_cast` of 1/2/4/8 bytes to `intN_t`, then widening. |
| `UNSIGNED_INTEGER` | `uint64_t`                | `std::bit_cast` of 1/2/4/8 bytes to `uintN_t`, then widening. |
| `FLOAT`            | `double`                  | `std::bit_cast` of 4 or 8 bytes to `float` / `double`. |
| `BYTES`            | `std::vector<std::byte>`  | Pass-through.                                         |
| `BOOL`             | `bool`                    | `bit_cast` of the first byte.                         |

The `Variant` is `std::variant<std::string, int64_t, uint64_t, double, bool,
std::vector<std::byte>>` (`libs/protos/interpretation/GenericTypes.hpp`).

### `format`

Stored as-is on the `InterpretationResult`. Pass it to
`protos::value::variant_to_formatted_string(value, format)` (in `TypeFormatter.hpp`)
to render. It uses `std::format`, so any format spec accepted there works:
`"{:#04x}"` for hex, `"{:.3f}"` for floats, and so on.

### `mapper`

```cpp
using Mapper = std::function<InterpretationResults(const Variant&)>;
```

Runs *after* `as_variant(type, bytes)`. It receives the initially-typed Variant
and returns zero or more `InterpretationResult`s — so one field can fan out into
several results (e.g. a byte decomposed into bit flags). When a mapper is set,
the original `Variant`/`type`/`format` of the field is discarded; whatever the
mapper returns is what the caller sees.

Three canonical uses — enum, scaled numeric, and fan-out — are demonstrated in
[`examples/08_interpreter_mappers`](../../examples/08_interpreter_mappers/).

### `littleEndian` — documented but not honored

`FieldInterpretation::littleEndian` is declared in the struct (default `true`)
but **the current `FieldInterpreter::interpret` implementation never reads it**.
`as_variant` goes straight to `std::bit_cast`, so interpretation is always host
byte order.

If your ICD specifies BE fields on an LE host, you have three options:

1. Attach a `mapper` that reverses the bytes and re-interprets. This is the
   workaround used in [`examples/07_icd_mixed_endian`](../../examples/07_icd_mixed_endian/).
2. Pre-reverse the bytes in the `FieldData` before handing it to the interpreter.
3. Wait for the flag to be wired up, then flip it to `false`. If/when that lands,
   simplify your descriptors and delete the mappers.

See [endianness](./endianness.md) for the full discussion.

### `hidden`

Propagates to `InterpretationResult::hidden`. The interpreter still emits the
result; rendering code decides whether to display it. Use it for reserved /
padding bytes you want in the data model but not in the UI.

### `subFields` — legacy, superseded by mappers

The container survives and `FieldInterpreter::handleSubFields` still iterates
it, but the feature it originally powered was removed deliberately. In the
pre-April-2025 design, each subfield carried a `function` string — a
`cparse::calculator`-evaluated expression like `"value & 0x0F << 4"` — that
let you declare bitfield slices inline. Commit `bd2eb23` ("FieldInterpretation
adapted", 15 Apr 2025) dropped the `function`, `dissector`, and registry-based
`mapper` strings in favor of a single `std::function<InterpretationResults(Variant)>`.

Without that expression engine, `subFields` is effectively vestigial — a subfield
receives the same bytes as its parent, and `mapper` already returns
`InterpretationResults` (plural), so one parent field with a mapper does
everything the old subfield tree did. The commented-out tests in
`tests/InterpreterTests/GenericInterpreterTests.cpp` (lines 60-169) were
exercising the old expression machinery, not the current code path; they are
kept around as design history, not an intent-to-fix.

**Recommendation:** use a `mapper` that emits multiple `InterpretationResult`s
for bitfield work. [`examples/08_interpreter_mappers`](../../examples/08_interpreter_mappers/)
shows the pattern.

## GenericPacketInterpreter

```cpp
GenericPacketInterpreter interp;
interp.addField({.name = "id",  .type = Type::UNSIGNED_INTEGER, .format = "{}"});
interp.addField({.name = "pos", .type = Type::FLOAT,            .format = "{:.2f}"});

auto results = interp.interpretPacketData(packet_data);
for (const auto& r : results) {
    std::println("{} = {}", r.name,
        protos::value::variant_to_formatted_string(r.value, r.format));
}
```

The internal storage is `std::unordered_map<std::string, FieldInterpretation>` —
adding a second `FieldInterpretation` with the same name overwrites the first.

### Missing-field behavior

Fields present in `PacketData` but not registered on the interpreter are handled
per `Behaviors::emptyFieldBehavior`:

| Value                  | Effect                                                                 |
|------------------------|------------------------------------------------------------------------|
| `SKIP`                 | No output. Silent.                                                     |
| `SKIP_AND_WARN_ONCE`   | **Default.** Skip; `std::println` a warning the first time per field/interpreter. |
| `EMPTY_STRING`         | Emit `{field_name, Variant{""}}`.                                      |
| `DASH`                 | Emit `{field_name, Variant{"-"}}`.                                     |
| `TROW_ONCE`            | Throw `std::runtime_error` the first time. Yes, the enum is misspelled. |
| `CALL_BACK`            | Call `Behaviors::emptyFieldCallback(field_name)`.                      |

Set with `interp.setBehaviors({EmptyFieldBehavior::DASH, nullptr})`.

## Consuming InterpretationResult

```cpp
struct NamedValue { std::string name; protos::value::Variant value; };
struct InterpretationResult : NamedValue {
    std::string format{"{}"};
    bool        hidden{false};
};
using InterpretationResults = std::vector<InterpretationResult>;
```

Extract typed values with `std::get`, `std::holds_alternative`, or the
equality-with-Variant helper exposed in `GenericTypes.hpp`:

```cpp
using namespace protos::value::operators;
assert(result.value == uint64_t{42});  // picks the uint64_t alternative
```

Render with:

```cpp
protos::value::variant_to_formatted_string(result.value, result.format);   // uses std::format
protos::value::variant_to_string(result.value);                            // type-driven default
```

## Related

- [Dissection](./dissection.md) — the layer that produces `PacketData`.
- [Endianness](./endianness.md) — what `littleEndian` *should* do, what it actually does, and how to work around it today.
- [ICD cookbook](./icd-cookbook.md) — recipes using mappers for enums, scaled units, and bitfields.
- Examples: [`06`](../../examples/06_interpreter_basics/), [`07`](../../examples/07_icd_mixed_endian/), [`08`](../../examples/08_interpreter_mappers/).
