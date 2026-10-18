# `.k2pack` format, version 1

One file = one imported character. Written on the user's machine by
`k1_port_pack.py` (called by `k1_port_import.py build`), read at game start by the
`character-packs` loader (`character_packs_src/PackFormat.h`). Contains no code.
All integers are little-endian. All offsets are absolute from the start of the file.

## Layout

| Offset | Size | Part |
|---|---|---|
| 0 | 32 | header |
| 32 | 208 | character block |
| 240 | 32 x asset_count | asset table |
| 240 + 32 x asset_count | 120 x cell_count | cell table |
| next multiple of 8 | ... | blobs, each starting on a multiple of 8, zero padding between; the file ends at the last blob byte (no trailing pad) |

The CRC covers everything after the header, not the header itself: a changed count that still
fits the layout is caught by the table/blob bounds checks, not by the CRC.

### Header (32 bytes)

| Off | Type | Field | Rule |
|---|---|---|---|
| 0 | char[4] | magic | `K2PK` |
| 4 | u32 | version | `1` |
| 8 | u32 | header_size | `32` |
| 12 | u32 | asset_count | 1..192 |
| 16 | u32 | cell_count | 0..256 |
| 20 | u32 | total_size | equals the file length |
| 24 | u32 | crc32 | zlib/IEEE CRC-32 (`zlib.crc32`, poly 0xEDB88320, init and xorout 0xFFFFFFFF) of bytes `[32, total_size)` |
| 28 | u32 | reserved | `0` |

### Character block (208 bytes, offset 32)

Strings are NUL-padded fixed fields; a `char[N]` field holds at most N-1 characters
and must contain a NUL (the reader rejects a field without one). Every byte after the
first NUL is also NUL, and every `reserved` field is 0 (the reader rejects anything else).

| Off | Type | Field | Rule |
|---|---|---|---|
| 0 | char[32] | character_id | `[a-z0-9_]{1,31}` |
| 32 | char[64] | display_name | 1..63 printable ASCII (0x20..0x7E) |
| 96 | char[32] | default_tag | 0..31 printable ASCII |
| 128 | char[32] | default_utc | empty or resref `[a-z0-9_]{1,16}` |
| 160 | char[32] | default_dlg | empty or resref `[a-z0-9_]{1,16}` |
| 192 | u32 | appearance_id | appearance.2da row |
| 196 | u32 | head_id | heads.2da row |
| 200 | u32 | soundset_id | soundset.2da row |
| 204 | u32 | reserved | `0` |

### Asset entry (32 bytes)

| Off | Type | Field | Rule |
|---|---|---|---|
| 0 | char[16] | resref | `[a-z0-9_]{1,16}`, NUL-padded; may fill all 16 bytes (no NUL then) |
| 16 | u16 | res_type | one of 2002 mdl, 3008 mdx, 3007 tpc, 2022 txi, 2027 utc, 2029 dlg, 2010 ncs, 2060 ssf, 3004 lip, 4 wav; 3 (tga) and 2017 (2da) are refused |
| 18 | u16 | flags | 0 = RESMAN, 1 = STREAM (stream only for res_type 4) |
| 20 | u32 | data_offset | multiple of 8, >= blob start, `data_offset + size <= total_size` |
| 24 | u32 | size | > 0 |
| 28 | u32 | reserved | `0` |

No two entries share `(flags, res_type, resref)`. The writer orders entries by
`(flags, res_type, resref)`; the reader does not require an order. Blobs must not overlap.

### Cell entry (120 bytes)

| Off | Type | Field | Rule |
|---|---|---|---|
| 0 | char[16] | table | `[a-z0-9_]{1,16}`, NUL-padded; may fill all 16 bytes |
| 16 | u32 | row | 2DA row index |
| 20 | i32 | template_row | -1 = none, else the row the new row is copied from |
| 24 | char[32] | column | 1..31 chars, printable ASCII, no whitespace |
| 56 | char[64] | value | 0..63 printable ASCII |

No two cells share `(table, row, column)`.

## Reader rules (loader)

Every rule above is checked; any failure skips the whole pack with one log line naming
the pack and the first failed rule. The reader never trusts a count or offset before
checking it against `total_size`, and a file larger than 256 MiB is refused before it is read.
The loader also checks the shared pools before registering (asset-vfs: 192 static assets,
1024 2DA cells since asset-vfs 0.1.3-dev, 64 characters, shared by every mod) and WARNs when a pack does not fit.

## Versioning

A reader refuses any version other than the ones it knows. Adding a field means a new
version number; version 1 is never changed.
