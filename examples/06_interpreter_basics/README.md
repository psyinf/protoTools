# 06 — Interpreter Basics

Dissect → interpret pipeline. `GenericDissector` splits bytes into a `PacketData`;
`GenericPacketInterpreter` turns each field's bytes into a typed `Variant` and a
`std::format` string.

## What it shows

- Registering one `FieldInterpretation` per field name with a `type` and a `format`.
- Using `variant_to_formatted_string(value, format)` to render an `InterpretationResult`
  for display.

## Run

```
./ex06_interpreter_basics
```

Expected output (on a little-endian host):

```
id = 42
counter = 1234
rate = 3.142
```

## Caveat

`FieldInterpretation::littleEndian` is **declared but not honored** by the current
interpreter implementation (`FieldInterpreter::interpret` always reinterprets via
`std::bit_cast`, i.e. host byte order). See example 07 for the correct workaround
when a field is BE on an LE host.

## See also

- Guide: `doc/library/interpretation.md`
- Source test: `tests/InterpreterTests/GenericInterpreterTests.cpp` — "simpleField"
