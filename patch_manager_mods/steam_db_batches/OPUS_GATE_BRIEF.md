# Opus per-batch gate brief (DESIGN 4.6)

The lead fills in BATCH, PART (a line range or section of `B<n>/opus_gate.md`) and OUT.

## Job
For every row in your PART of `B<n>/opus_gate.md`, decide the final Steam DB row for that GOG function. You are the
final gate before rows reach the knowledge DB and the KPM branch DB: a wrong address row calls unrelated code.

Per row, read: the row in `B<n>/evidence.tsv` (gate facts and anchors), every reviewer row for that GOG address in
`B<n>/review/*.tsv` (decoy cards in `B<n>/cards/key.tsv` with kind != real are calibration only), the card in
`B<n>/cards/chunk_*.md` (grep the GOG address), and then the bodies on disk:
- GOG side: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/k2_win_gog_aspyr_swkotor2.exe/fp.tsv`
  (addr size retn n_insn callconv thunk_target callees str_refs disp imm name namespace source; list cells unpadded hex);
- Steam side: `.../k2_win_steam_aspyr_swkotor2.exe/fp.tsv` and the dump
  `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<hex digits 3-4>/<addr8>.{c,asm}`;
- Mac x86 real symbol names: `.../k2_mac_KOTOR2_x86/fp.tsv` (name+namespace columns; different compiler, compare
  shape/values only; Mac vtable offsets are +4 vs Windows).
Tools: `patch_manager_mods/ghidra_knowledge/tools/re_card_prep.py`, `patch_manager_mods/engine_ai_atlas/tools/cg.py`.
Traps: `patch_manager_mods/ghidra_knowledge/RE_AGENT_GUIDE.md` section 3. Layout growth exists (CSWGuiSaveLoad +0x2d0,
REVIEWER_BRIEF "Layouts"); a uniform small offset shift is a different function.

Decide one of:
- `verified`: the Steam address is the body counterpart of the GOG address AND the body fits the name, with at least
  one content anchor you quote (asm line with address, string, or callee identity).
- `likely`: counterpart is right but the only anchors are positional/thin (stub, getter) and nothing contradicts.
- `label_suspect`: counterpart is right, the GOG name contradicts the body. Add the name it fits if known (Mac).
- `rejected`: the Steam address is wrong and you cannot give the right one.
- a correction: give the right Steam address with `verified`/`likely` only if you checked its body against the GOG fp
  yourself (quote it). Otherwise `unresolved` and note the address to re-gate.
- `unresolved`: cannot decide.
If you find the GOG name belongs at a DIFFERENT GOG address (label rotation), say so in the note.

## Output
OUT (TSV, header): `gog_addr steam_addr final_verdict note` (note <= 200 chars: anchor + reason). One row per gated
row you decide, even when you agree with the pipeline (write its verdict). Plus `OUT.md`: per-row evidence in short
form, and at the end `sample_errors: N` = number of SAMPLED reviewer `match` rows (section "Sample of the other
reviewer matches") you found wrong (0 if your part has no sample rows), and for AUTO meaning checks a pass/fail table.

## Rules
Read-only except OUT and OUT.md. Never commit, never write any DB, never touch the game folder or any
`*credential*`/`*secret*` file. Private `mktemp -d` for scratch. Final report <= 15 lines: counts per verdict,
sample_errors, anything systematic (e.g. a GOG label rotation, a class shifted by one).
