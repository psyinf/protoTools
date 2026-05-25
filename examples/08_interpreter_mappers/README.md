# 08 — Interpreter Mappers

Three canonical uses of `FieldInterpretation::mapper`:

1. **Enum code → label** — translate a numeric state to a human-readable string.
2. **Scaled integer → engineering unit** — raw counts × scale factor (`raw × 0.1`
   degrees C, etc.).
3. **Fan-out bit decomposition** — one input field produces *multiple* named results
   (one per flag). `Mapper` returns `InterpretationResults`, not a single result, so
   a single `addField` can yield any number of named outputs.

## Run

```
./ex08_interpreter_mappers
```

Expected output:

```
state = RUN
temp_c = 23.5
armed = true
ready = true
faulted = false
```

## See also

- Guide: `doc/library/interpretation.md` § "Mappers"
- Guide: `doc/library/icd-cookbook.md` § "Enumerated fields" and "Scaled fixed-point"
