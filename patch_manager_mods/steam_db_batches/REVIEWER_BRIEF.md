# Reviewer brief: Steam address-DB REVIEW cards (Sonnet)

Used for every batch and for calibration. The lead fills in CHUNK, OUT and SCRATCH.

## Task
Each card asks two separate questions:
1. **Counterpart:** is the candidate Steam function the same compiled function as the GOG function at the GOG address
   (same body: callees, strings, member offsets, immediates)?
2. **Name:** does the body do what the GOG name says?
Verdicts:
- `match`: 1 yes (with an anchor) and 2 yes or not contradicted.
- `label_suspect`: 1 yes (with an anchor) but 2 clearly no (the body contradicts the name, e.g. a "Get...Visible"
  predicate that dismisses popups). Say in `reason` which name the body fits, if you can tell. These are GOG DB
  label bugs; they are recorded, never shipped.
- `mismatch`: 1 no (the body differs from the GOG side).
- `unsure`: you cannot anchor 1.
Write one TSV row per card to OUT (header below). Work only from the card, the dump and the tools listed here.

## Read first (required)
`patch_manager_mods/ghidra_knowledge/RE_AGENT_GUIDE.md` sections 3 (traps) and 4 (prompt rules).

## Facts (pre-computed; do not recompute)
- `Stack Slot Match` and G1-G6 on the card are script output. Narrow GOG args (bool 1, short 2, odd totals) still
  pop 4-byte slots: GOG 1 byte -> `RET 4` is a match. cdecl pops 0.
- One-line thunks print without `__thiscall`; judge the thunk's target (the card resolves it).
- Bare decompile offsets are DECIMAL and scale by the pointer type: `int *p; p + 100` = byte 0x190. The asm operand
  (hex byte offset) settles it: quote the asm.
- Ghidra labels (`FID_conflict:*`, `Statistics`, `GetMargins`, ...) and our mapper/DB names are hypotheses, never
  evidence.
- The GOG side section comes from the GOG binary itself (callees named by the GOG DB, strings, operand displacements
  and immediates). Common values are marked `*`.

## Burden of proof
Assume the candidate is a neighbouring function or thunk with an identical `RET` signature. You are FORBIDDEN to
answer `match` on `ret N`, size, SIG score or name alone. To answer `match` you MUST cite at least one content anchor
you checked yourself, with its address:
- a string literal (address + text) that the GOG side also has, or that the GOG name clearly implies;
- a callee whose identity you confirmed (Steam address + name, and the GOG side lists the same callee);
- a member offset access (quote the asm line) that matches the GOG side or the function's documented purpose.
If you cannot, answer `unsure`. Answer `mismatch` when the body contradicts the GOG function (different callees,
strings or behaviour), and give `corrected_addr` only if you found the right function with an anchor.

## Negative few-shots (real cases from this repo)
- 0x5644B0 vs 0x564770, both once named `AddMoveToPointAction`: 0x5644B0 calls AddActionToFront
  (`005645e4 CALL 0x00540ca0`), 0x564770 calls AddAction (`005648a4 CALL 0x0053f7f0`). The callee separates them.
- 0x552A90 vs 0x552C90: GOG param sizes swapped Initialize/Put. 0x552A90 `ret 0`, `PUSH 0x2000` + `CALL 0x00919723`
  (allocates) = Initialize; 0x552C90 `RET 0x10`, calls the hash Get = Put. Structure beats position.
- bool parameter: GOG param 1 byte, Steam `RET 4` = match on the signature (not a mismatch).
- Offset shift (calibration 2026-10-03, a reviewer got this wrong): GOG `CSWSObject::SetKeepCorpse` writes
  `+0x158`/`+0x234`; the candidate 0x54A700 (its neighbour) writes `[EAX+0x154]`/`[ECX+0x230]`. Same size, same
  `RET 4`, but a "uniform -4 shift" means DIFFERENT FIELDS, i.e. a different function: `mismatch`. Struct layouts are
  identical between GOG and Steam (all 21 shared DB offsets agree), so every GOG displacement and immediate on the
  card must appear in the candidate unless the card says otherwise. Any unexplained difference = not `match`.

## Tools (read-only)
- `python3 patch_manager_mods/ghidra_knowledge/tools/re_card_prep.py annotate|abi|anchors|prologue 0xADDR`
- `python3 patch_manager_mods/engine_ai_atlas/tools/cg.py callers|callees 0xADDR`
- decompiles: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<first 2 hex digits>/<addr8>.{c,asm}`
- Put any helper script in your private SCRATCH (`mktemp -d`); other reviewers run in parallel.

## Output (evidence first, verdict decided last)
For each card, first write a notes block in SCRATCH/notes.md in this order: prologue/thunk, the anchor you checked
(quoted line with address), the decoy you considered and why it is not it, then the verdict. Then append the TSV row
to OUT:
```
gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason
```
`verdict` = match | label_suspect | mismatch | unsure; `content_anchor` = the quoted anchor with address (empty only for unsure);
`confidence` = 1-5; `reason` <= 20 words. One row per card, card order, no extra columns.

## Rules
Never commit, never write any database, never touch the game folder or any `*credential*`/`*secret*` file. Do not
open `key.tsv` or any answer file. Report at the end: count of match/mismatch/unsure and any card you found
malformed.
