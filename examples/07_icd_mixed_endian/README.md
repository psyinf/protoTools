# 07 — ICD Mixed Endian

A realistic ICD record with BE and LE fields in the same packet. Primary use case:
parsing binary protocols defined by an Interface Control Document that specifies
per-field byte order.

## What it shows

- How to handle per-field endianness on an LE host.
- The **workaround** for `FieldInterpretation::littleEndian` being a no-op in the
  current interpreter (declared but never read — confirmed via source inspection of
  `libs/protos/interpretation/FieldInterpreter.cpp`).
- Pattern: declare BE fields with `type = BYTES` and attach a `mapper` that reverses
  the raw bytes and reinterprets them as the target type. LE fields on an LE host
  pass through with a direct numeric `type`.

## Wire layout

```
offset  size  field     endian  wire bytes        logical value
 0       4    seq_be    BE      00 01 02 03       0x00010203 (66051)
 4       8    ts_le     LE      EF BE AD DE 00*4  0xDEADBEEF
12       4    val_be    BE      3F 80 00 00       1.000 (IEEE-754 float)
```

## Run

```
./ex07_icd_mixed_endian
```

Expected output (on a little-endian host):

```
seq = 66051
ts_le = 0xdeadbeef
val = 1.000
```

## Forward-compatibility note

If `FieldInterpretation::littleEndian` is later wired up in `FieldInterpreter`, the
mappers in this example become unnecessary and you can simplify to:

```cpp
interp.addField({.name = "seq_be", .type = Type::UNSIGNED_INTEGER, .littleEndian = false});
```

## See also

- Guide: `doc/library/endianness.md` — full discussion of the declared-vs-implemented gap.
- Guide: `doc/library/icd-cookbook.md` — more ICD recipes.
