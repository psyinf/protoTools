# 07 — ICD Mixed Endian

A realistic ICD record with BE and LE fields in the same packet. Each field
declares its wire endianness via `FieldInterpretation::littleEndian`; the
interpreter swaps bytes on the LE host as needed.

## What it shows

- Per-field endianness declared with `.littleEndian = false` for BE,
  default `true` for LE.
- Mix of `UNSIGNED_INTEGER` and `FLOAT` numeric types in one packet.
- Zero boilerplate — no custom mappers, no byte-swap helpers.

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
seq_be = 66051
ts_le = 0xdeadbeef
val_be = 1.000
```

## When the flag isn't enough

`FieldInterpretation::littleEndian` applies only to numeric types (`INTEGER`,
`UNSIGNED_INTEGER`, `FLOAT`). For `STRING`, `BYTES`, or any custom decoding
you still need a `mapper` — see
[`examples/08_interpreter_mappers`](../08_interpreter_mappers/) for the
idioms.

## See also

- Guide: [`doc/library/endianness.md`](../../doc/library/endianness.md) —
  end-to-end discussion of byte-order handling.
- Guide: [`doc/library/icd-cookbook.md`](../../doc/library/icd-cookbook.md) —
  more ICD recipes.
