# K2 Wrap Measure Fix

> **STATUS** Track: SHIPPABLE. Version 0.1.0 (matches manifest.toml).
> **What it does / explores:** Re-points one `FADD` operand in `CAurGUIStringInternal::WrapStrings` so word-wrap measures glyph widths by rounding instead of truncating; wrapped GUI lines no longer under-measure by about 0.25 px per character.
> **Requires:** KOTOR II Steam/Aspyr exe (SHA-256 `6A522E71...`), Kotor Patch Manager. No DLL, no INI.
> **Known limits:** Byte patch at one site (`0x0047B232`). **Play-test status:** see PLAY_TEST_QUEUE.md.
> **Install:** install the patch through KPM; no other files.


A tiny KPM "simple" byte patch for KOTOR II (Steam Aspyr exe, hash 6A522E71; the LAA alias was dropped 2026-10-02, the live exe is vanilla + 4gb-patch).

## What it does

`CAurGUIStringInternal::WrapStrings` (0x0047AF30) measures glyph advance for
word wrap with `FADD double ptr [0x0098C1D8]` (= 0.25) at VA 0x0047B232
(file offset 0x7A632) and truncates each glyph's width to int. Draw uses exact float
widths, so wrapped lines under-measure by about 0.25 px per character.
This patch re-points the operand to the existing 0.5 double at 0x00987F08,
turning the per-glyph truncation into rounding (the 0.25 constant itself is
not changed: 48 other code references). (KMRP "K6" equivalent; research in
engine_research_findings/V3.md and doc 16 lead L4.)

## Bytes

    0x0047B232  DC 05 D8 C1 98 00  ->  DC 05 08 7F 98 00

The manifest targets the vanilla Steam Aspyr exe only (6A522E71; the 4gb-patch
flag does not move these bytes). No function, no parameters, no INI.
(The hooks.toml comment still says "both the Steam Aspyr and Steam Aspyr LAA
exes"; it mirrors the KPM copy byte-for-byte, so it is left as is.)

## Build

    cd <KPM>/Patches/K2WrapMeasureFix && python3 ../create-patch.py

`<KPM>` = `the Patch Manager checkout`.
The `.toml` files here are byte-identical mirrors of the ones in that folder.

## Status

LIVE (in the installed KPM patch set; doc 20a audit §4, 2026-10-03). Rollback =
reinstall the KPM patch set without the id `k2-wrap-measure-fix`.

## Tests

T0 only (doc 20 §5 Low): `python3 scripts/check_hook_bytes.py` checks the
6 original bytes against the vanilla exe and the `.kpatch` packaging.
