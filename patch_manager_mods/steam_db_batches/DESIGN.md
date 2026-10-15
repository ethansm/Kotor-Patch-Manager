# Steam address-DB batch pipeline: design (lead-approved spec)

Status: 2026-10-03, Phase A. The plan is the session plan "map the Steam Aspyr exe into our DBs, in verified batches"
(transcript ffc8cb88...). This file is the contract for every tool in `steam_db_batches/tools/`.
`steam_db_pr3/` stays the Batch 1-8 archive.

Mandatory reading for anyone who writes a brief or judges a row: `ghidra_knowledge/RE_AGENT_GUIDE.md`.

## 0. Paths (all overridable by env var, defined once in `tools/bdlib.py`)
| name | default | env |
|---|---|---|
| DUMP | `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` | `BD_DUMP` |
| SERVER | `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/` (one subdir per program) | `BD_SERVER` |
| KDB | `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db` | `BD_KDB` |
| BRANCHDB | `~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db` | `BD_BRANCHDB` |
| GOGDB | `~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_gog_aspyr.db` | `BD_GOGDB` |
| BATCHES | `patch_manager_mods/steam_db_batches/` | `BD_ROOT` |
| B1_VERIFIED | `patch_manager_mods/steam_db_batches/gold/b1_verified_head.tsv` (= `steam_db_pr3/verified.tsv` at commit 1820a74, the Opus-reviewed Batch 1; the working copy carries Gemini's unreviewed "Option A" edits to 15 rows, listed in `gold/gemini_option_a_changed_rows.txt`, which Batch 0 reviews) | - |

All tools open every DB read-only (`file:...?mode=ro`, `uri=True`), except `persist --apply` and
`ingest_server.py --apply`, which only the lead runs. Stdlib only. No tool touches the game folder or any
`*credential*` / `*secret*` file.

## 1. Trust tiers (the most important rule)
Gemini's Batches 2-8 were delta projection only, yet were written to `name_map` as `high`. A row counts as
**trusted** (usable as a callee anchor, a gold positive or a "confirmed neighbour") only if:
- it is in `B1_VERIFIED` with verdict `verified`; or
- it is a `name_map` row with confidence `high` whose source is NOT unaudited; or
- it is in a later `steam_db_batches/B<n>/final_rows.tsv` with verdict `verified` (persisted as source `steam-db-B<n>`).

Unaudited sources (regex, case-sensitive), until Batch 0 re-audits them:
`^batch[2-8]_`, `^gog_.*_(cluster|block)$`, `^gog_cluster_delta_map\.py$`.
B1-only sources (`bdlib.B1_ONLY_SOURCES`): `steam-db-pr3-phase2` `name_map` rows are trusted only through the
B1_VERIFIED route. Gemini's unreviewed "Option A" edits (10-02) rewrote 13 rows under that label (rows Batch 1 had
rejected or left unresolved), so the label alone proves nothing.
Branch-DB rows are trusted only through one of the routes above (the branch DB itself is not a trust source).
`name_map` rows with confidence `wrong` are known-false: usable as negatives.

Seed hygiene (Opus calibration gate 2026-10-03, R1; `bdlib.Corpus.pairs`):
> A trusted row enters `pairs` (run_ok seed, A-CALLEE anchor, gold positive) only if its GOG name equals the trusted
> Steam name exactly (no `base_name` suffix stripping) AND the GOG and Steam fingerprints agree on body: equal `retn`, and
> either equal `size` or A-SIG >= 0.5 with intersection >= 3. A trusted row that fails is listed in `gate_report.md`
> under `## Trusted pair body conflicts` and is not a seed. It stays trusted as a G6 holder until Opus rules on it.

Gold corrections (wrong gold pairs, GOG label errors) live in `calibration/gold_annotations.tsv`, loaded by `calibrate.py`;
`gold/b1_verified_head.tsv` is never edited.

Note: some `level-up-P5-static-confirmed` rows have evidence text in the `confidence` column (old column swap):
treat any confidence not in {high, medium, med, low, wrong} as `medium`.

## 2. Server fingerprint exports (read-only, Ghidra scripts in the KPM `tools/ghidra_scripts/`)
`ExportFingerprints.java <outdir>` writes, for `currentProgram`:
- `<outdir>/fp.tsv`, one row per function, header:
  `addr size retn n_insn callconv thunk_target callees str_refs disp imm name namespace source`
  - `addr`: `0x%08x` entry. `size`: body address count. `retn`: max RET imm (0 plain ret, -1 none).
  - `thunk_target`: `0x%08x` if `fn.isThunk()` (resolve the chain to the final non-thunk), else empty.
  - `callees`: sorted unique entry addrs of functions called (CALL and tail JMP to a function entry), comma-sep.
  - `str_refs`: sorted unique addrs of defined string data (or data whose value is a string) referenced from
    the body, comma-sep.
  - `disp`: sorted multiset (repeats kept) of memory-operand displacements `[reg+disp]` with base register not
    ESP/EBP and `0 < disp <= 0x10000`, hex, comma-sep.
  - `imm`: sorted multiset of scalar immediates (any operand) that do NOT resolve to an address inside a memory
    block, excluding 0, 1, -1 (0xffffffff), hex, comma-sep.
  - empty list = empty cell.
- `<outdir>/strings.tsv`: `addr  text` for every defined string (tabs/newlines escaped as `\t`/`\n`).
`ExportDataTypes.java <outdir>` writes `<outdir>/types.tsv`:
  `category type_name kind size ordinal field_offset field_name field_type`
  (kind = struct/union/enum/typedef; enums: one row per value, field_offset = value; typedef: one row, field_type
  = base type; types with no fields: one row with empty field columns.)

Server programs (`SERVER/<program>/`): `k2_win_gog_aspyr_swkotor2.exe`, `k2_win_steam_aspyr_swkotor2.exe`,
`k2_linux_swkotor2.elf`, `k2_mac_KOTOR2_x86`, `k2_mac_KOTOR2_x64` (each with `*_functions.tsv`,
`*_symbols.tsv` from `ExportProgramSymbols.java`, plus `fp.tsv`, `strings.tsv`, `types.tsv`).
GOG fingerprints use GOG addresses; Steam fingerprints use Steam addresses.

## 3. Knowledge-DB server tables (`ingest_server.py`)
```sql
CREATE TABLE IF NOT EXISTS server_functions(
  program TEXT NOT NULL, addr TEXT NOT NULL,          -- '0x%08x'
  name TEXT, namespace TEXT, callconv TEXT, prototype TEXT,
  source TEXT,                                        -- USER_DEFINED / IMPORTED / ANALYSIS / DEFAULT
  trust TEXT,                                         -- 'trusted' (USER_DEFINED, IMPORTED) / 'auto' (ANALYSIS) / 'none' (DEFAULT)
  size INTEGER, retn INTEGER, thunk INTEGER,
  export_date TEXT, PRIMARY KEY(program, addr));
CREATE TABLE IF NOT EXISTS server_symbols(            -- non-function symbols, USER_DEFINED and IMPORTED only
  program TEXT NOT NULL, addr TEXT NOT NULL, name TEXT NOT NULL, namespace TEXT, symbol_type TEXT, source TEXT,
  export_date TEXT, PRIMARY KEY(program, addr, name, namespace));
CREATE TABLE IF NOT EXISTS server_types(
  program TEXT NOT NULL, category TEXT NOT NULL, type_name TEXT NOT NULL, kind TEXT, size INTEGER,
  ordinal INTEGER NOT NULL, field_offset INTEGER, field_name TEXT, field_type TEXT,
  export_date TEXT, PRIMARY KEY(program, category, type_name, ordinal));
```
Fingerprints stay as TSV files in SERVER (too large for the repo DB). Upserts are idempotent (a re-run changes 0
rows). Ingest writes a diff report: server USER_DEFINED Steam names vs `name_map` and the branch DB
(agree / conflict / server-only), and server GOG names vs the KPM GOG DB (label disagreements = GOG label suspects).

## 4. Batch pipeline (`tools/batch.py`)
Working dir per batch: `BATCHES/B<n>/` (`B0` = re-audit of the branch DB). Subcommands, all deterministic:

### 4.1 `candidates B<n> --classes C1,C2,...` (or `--from-branch` for B0)
- Re-exports `DUMP/inputs/name_map.tsv` first if older than the KDB file (`engine_ai_atlas/tools/export_dump_inputs.py`).
- Runs `ghidra_knowledge/gog_cluster_delta_map.py <class> --out DUMP/findings/B<n>_map_<class>.tsv` per class
  (mapper output columns: steam, gog, name, tag in {vote, drift, seeded, ...}, retn=N).
- One row per GOG function of the classes (`DUMP/inputs/gog_functions.tsv`). Candidate sources, each tagged:
  `mapper:<tag>`, `name_map:<conf>:<source>`, `branch`, `server` (trusted server Steam name = this name),
  `near` (top-3 Steam function starts within +-0x400 of `gog + run_delta`, ranked by fingerprint score, where
  run_delta = the median delta of the class's trusted rows, else of its mapper `vote` rows).
- For B0, the branch address is the candidate under test (`branch`), other sources are listed as alternatives.
- Output `B<n>/candidates.tsv`: `class name gog gog_callconv gog_param gog_size gog_retn candidates` where
  candidates = `addr|src1+src2;addr|src...`.
- `--name-regex RX` keeps only the GOG functions of the classes whose name matches (`re.search`). B15 uses
  `'^ExecuteCommand'` on `Global,CSWVirtualMachineCommands`.
- `--extra-candidates FILE` (TSV `gog steam source`) adds each Steam address to its GOG row as a candidate tagged
  `source`. B15 uses the command-table candidates, `B15/extra_candidates.tsv` (section 4.8). The tags `cmdtable` and
  `cmdtable_overwritten` have `source_strength` 6 (engine data, label-independent).

### 4.2 `gates B<n>` -> `B<n>/evidence.tsv` + `B<n>/gate_report.md`
For every (GOG row, candidate) compute; then pick the row's best candidate (most anchor families, then invariants,
then source strength); a second candidate that also reaches C2 makes the row ambiguous (REVIEW).

Invariants:
- G1 `start`: candidate is a function entry in `DUMP/inputs/steam_functions.tsv`; not a CRT/library function (server
  Steam `source=ANALYSIS` with a library-looking name, or in the CRT range listed in `bdlib.CRT_ADDRS`).
- G2 `align`: 16-byte aligned (signal only: unaligned -> REVIEW, never REJECT).
- G3 `prologue`: standard prologue per `re_card_prep.py prologue` logic (import it, do not copy).
- G4 `ret`: Steam retn == GOG retn from the GOG fingerprint (primary). Secondary (metadata): expected
  `ceil(gog_param/4)*4` (cdecl 0); mismatch with the GOG fingerprint -> flag `gog_db_param_bad`, not a failure.
- G5 `thunk`: thunks are resolved to the target before G3/G4 and all anchors; record the target.
- G6 `dup`: the address is not held by a different name in the branch DB or `name_map` (trusted holder -> REJECT;
  untrusted holder -> REVIEW + record). Opus gate R4 (2026-10-03):
  > G6 also fails `review:name_held_elsewhere` when a trusted row holds the GOG name at a different Steam address (AUTO
  > blocked).
- G7 `mac` (Opus gate R3, 2026-10-03; both routes; source `SERVER/k2_mac_KOTOR2_x86/fp.tsv`, names = `namespace::name`):
  > let V(f) = the set of `disp` and `imm` values v with 0x10 <= v < 0x100000 in fp.tsv. For every Mac x86
  > function with exactly the GOG `Class::Name`, overlap = |{x in V(S): exists y in V(M), |x-y| <= 4}| / |V(S)|. If |V(S)| >= 2,
  > |V(M)| >= 2 and overlap == 0 for every such Mac function, flag `mac_contra` -> REVIEW (never REJECT). No Mac function
  > of that name, or too few values -> pass (recorded `g7=n/a`).
- G8 `mac_rotation` (lead decision after the B0 Opus gate, 2026-10-03; both routes; REVIEW, never REJECT). Catches GOG label
  rotations / swaps inside a class, which G7 misses when the wrong name still has a Mac symbol with some overlap.
  Let V(X) be as in G7 (X = a GOG function from the GOG fp, M = a Mac function from the Mac fp). Define
  `fit(X, M) = 0.75 * overlap(X, M) + 0.25 * closeness(X, M)` where overlap is the average of the two directions of the
  +-4 value overlap (share of V(X) with a V(M) value within 4, and share of V(M) with a V(X) value within 4) and
  closeness = min(size)/max(size) of the two fingerprints; fit is undefined (skipped) if |V(X)| < 2 or |V(M)| < 2.
  For the row's GOG function G and every GOG function G' of the same class within +-4 positions in GOG address order
  (overloads of G, i.e. names equal up to a `_N` suffix, are skipped: one Mac symbol cannot tell them apart):
  - if the GOG name has Mac functions M: flag `rotation:<G' addr>:<fit G'>><fit G>` when some G' has
    fit(G', M) > fit(G, M) + 0.15 and fit(G', M) >= 0.5 (best over the Mac functions of that name);
  - if the GOG name has no Mac function: flag `rotation_in:<G' addr>:...` when G's body fits the Mac function named like a
    neighbour G' better than G''s own body does (same thresholds).
  Otherwise `pass`, or `n/a` when nothing can be computed. Recorded in the `g8` evidence column; why-codes `mac_rotation` /
  `mac_rotation_in`. Calibration (`tools/g8_study.py`): positives lose 2.5-2.6% of their AUTO rows (block-1 split).

Anchor families (computed for the candidate S AND for its decoys D = the previous and next Steam function starts
and every other candidate of the row; a family counts only if it is **discriminating**: S's score > every decoy's):
- A-STR: distinctive strings (len >= 4, referenced by <= 3 Steam functions) shared by text between the GOG function's
  `str_refs` and S's. Score = count.
- A-CALLEE: trusted GOG->Steam pairs P (GOG DB name == trusted Steam name). Score = |{(g,s) in P: g in callees(G),
  s in callees(S)}| + the same over callers, counting only callees/callers with Steam in-degree <= 40 (hubs such as
  CExoString/operator new excluded). Also report contradictions: g in callees(G) with trusted s not in callees(S).
- A-SIG: operand signature. Jaccard of `disp` sets and of `imm` sets (GOG vs S); score = mean, counts if
  >= 0.5 and |disp intersection| + |imm intersection| >= 3.
- A-SRV: a trusted server name at S equal to the GOG name (or a server-program name bridged by the same method).
- A-VT: `class_methods` row at S with the same symbol, or vtable slot equality where both sides are known.
Content level: C0 none, C1 one family, C2 >= 2 families. A-VT is reported but does not count toward C1/C2
(calibration 10-03: `class_methods` symbols come from the same labelling, so it is circular).
Run consistency `run_ok`: S-G equals the delta of the nearest mapped GOG neighbour on at least one side
(neighbour mapped by a trusted `pairs` row (seed hygiene, section 1) or this batch's `auto_route=c2` row), and no permuted pair (two same-size neighbours whose
fingerprint scores prefer the swapped assignment).

Gate class:
- AUTO: G1, G3, G4, G5, G6 pass; G7 not `mac_contra`; G8 not a rotation; C2; run_ok or mapper `vote`; not ambiguous; G2 pass.
  - `auto_route=c2` is the rule above.
  - `auto_route=sig` (R3; lead decision 2026-10-03, signed off with changes R1-R7 (calibration/OPUS_REVIEW.md)): every invariant
    passes (g1 pass, g2 pass, g3 pass or pass(exe), g4 pass, g6 pass, g7 not `mac_contra`, g8 not a rotation, no unresolved thunk), the counting
    families (VT excluded) are exactly {SIG}, a_sig >= 0.75, run_ok == yes, steam_size == gog_size, not permuted, not ambiguous,
    and no callee contradiction. `mapper:vote` does not help this route. Opus gate R2 (2026-10-03), exact rule text:
    > ... the best decoy's SIG <= a_sig - 0.25, where the decoys are the previous and next Steam starts, every other
    > candidate of the row, AND every Steam start within +-8 starts of the candidate with the same `size` and `retn`.
  - The sig route is NON-PROPAGATING: run_ok for any row may only be seeded by trusted pairs and by `auto_route=c2` rows; a
    `sig`-route row is never added to the mapped-neighbour set. Evidence rows carry `auto_route=c2|sig` in `flags`.
- REJECT: hard contradiction: G1 fails (not a start / CRT), G4 fails with no thunk explanation, or G6 with a trusted
  holder.
- REVIEW: everything else with a candidate. NOCAND: no candidate.
(The AUTO rule is calibrated in A3; tighten it there, never loosen it without the Opus calibration review.)

`evidence.tsv` columns = Batch 1 `verified.tsv` columns (`kind class name gog steam_addr verdict calling_convention
param_size_bytes evidence cand_orig cand_status content_level wrapper_issue steam_size gog_param_size`) +
`batch cand_sources mapper_tag gog_size gog_retn steam_retn g1 g2 g3 g4 g5_target g6 a_str a_callee a_sig a_srv a_vt
families run_ok decoy_best gate_class review_ref`. `evidence` = a short machine-written anchor summary, e.g.
`STR 'aborted walking...'@009965f8; CALLEE 3 (AddActionToFront 0x540ca0,...); SIG 0.82`.
`gate_report.md`: counts per class and gate class, every REJECT with reason, duplicates/conflicts, G4 metadata flags.

### 4.3 `cards B<n> [--chunk 60]` -> `B<n>/cards/chunk_<k>.md` + `B<n>/cards/key.tsv`
One card per REVIEW row: the GOG name/metadata and mapper tag; the computed invariants and anchor scores (as stated
facts); `re_card_prep.py annotate` (truncated to ~80 lines), `abi`, `anchors`, `prologue` of the candidate (and of
the best decoy, labelled `decoy`); both GOG-order neighbours with their gate class; any server name; the decoy note
("assume a same-ret neighbour"). Calibration chunks mix in decoy cards (key.tsv holds the answer; cards do not).
- No line may tell a real card from a decoy card. Candidate sources and the mapper tag are withheld on every card:
  the line reads `Candidate sources: (withheld on every card; judge the body)`. Before 18f0214, decoy cards read
  `card`, which leaked the answer in B0.
- `B<n>/card_facts.tsv` (`gog fact`, optional) holds GOG-side facts that are printed on every card of that GOG
  function, real and decoy alike. Example: the command-table slots of a GOG handler (section 4.8). A fact never
  names a Steam address.
- Decoys are never proven counterparts (fix F1, after B15). A same-retn neighbour that is a candidate of the host row
  with a proof source is skipped. The proof sources are `cmdtable`, `cmdtable_overwritten`, `server`, `recheck`,
  and `name_map:high:<src>` whose source is not unaudited. In B15 the gate's pick was wrong and its neighbour was the
  table counterpart (B15-4-38).

### 4.4 `finalize B<n>`
Inputs: `evidence.tsv`, reviewer TSVs `B<n>/review/*.tsv` (`gog_addr steam_addr verdict(match|mismatch|unsure)
corrected_addr content_anchor confidence reason`), Opus overrides `B<n>/overrides.tsv` (`gog_addr steam_addr
final_verdict note`). Final verdict per row: `verified` (AUTO, or REVIEW confirmed with a cited anchor and C1+),
`likely`, `rejected`, `unresolved`, `label_suspect` (reviewer verdict or Opus override; counterpart proven, GOG name contradicts the body: never shipped, listed in `B<n>/gog_label_suspects.tsv`). Overrides win. Outputs:
- `B<n>/final_rows.tsv` (evidence columns + `final_verdict final_note`);
- `B<n>/functions.csv`: SqliteTools `import-ghidra` format, header
  `class_name,function_name,address,calling_convention,param_size_bytes,notes`, verified rows only (likely only with
  `--with-likely`), notes = `Steam batch B<n>: <anchor summary> (evidence steam_db_batches/B<n>/evidence.tsv)`;
  param_size_bytes = the Steam retn (stack bytes) for thiscall/stdcall (record GOG's value in notes if different);
  the notes anchor is the gate evidence for AUTO rows, and the final note (`Opus: ...` / reviewer anchor, evidence
  file `final_rows.tsv`) for reviewed or overridden rows: the gate evidence describes the gate's pick, which an Opus
  correction may have replaced;
- `B<n>/kdb_upsert.json` (the name_map rows persist will write); for B0 also `B<n>/branch_deletes.tsv`
  (rows to remove) and `B<n>/branch_corrections.csv`.

Reviewer corrections (Opus calibration gate 2026-10-03, R5), exact rule text:
> A reviewer `mismatch` with `corrected_addr` gives `unresolved` plus a `recheck` entry. The corrected address is added
> as a candidate and re-gated in the next `gates` run, and is shipped only via that gate or an Opus override. It never
> becomes `likely` by itself.

Reviewer rows that answer a decoy card (pairs in `cards/key*.tsv`) only score the decoys (section 4.6). They never drive
a row's verdict, a recheck or the Opus 100% list (fix F4, after B15: 18 of 25 "reviewer corrected_addr" rows were decoy
rejections).
Implementation: `finalize` writes `B<n>/recheck.tsv` (`gog_addr corrected_addr source reason`); the next `gates B<n>` appends
each corrected address to its row as a candidate tagged `recheck`. A reviewer `label_suspect` with `corrected_addr` records
that address as the counterpart in `gog_label_suspects.tsv`. `finalize` also writes `B<n>/opus_gate.md` / `.json` (section 4.6).

### 4.5 `persist B<n> [--apply]`
Dry-run by default: prints the planned inserts/updates/demotions. `--apply` (lead only): one transaction;
`name_map` upsert with source `steam-db-B<n>`, confidence `high` (verified) / `medium` (likely); an existing row at
the same address with a different name is replaced and its old name/source/confidence is appended to `evidence`
(`replaces <old> (<source>/<conf>) <date>`); an existing row holding the same name at a different, now rejected
address gets confidence `wrong` + a dated note; lessons from `gate_report.md` section `## Lessons` inserted once
(dedupe on scope+topic). Ends with `PRAGMA integrity_check`. A second `--apply` changes 0 rows.
Opus gate R4 (2026-10-03), exact rule text:
> `persist` never inserts a second address for a name that a trusted row already holds. It never demotes a
> trusted row to `wrong` unless an `overrides.tsv` row from Opus names that address.
(Such upserts / demotions are printed as `skip` lines with the reason.)
`persist` also writes the validated ledger (section 4.7) from `B<n>/validated.tsv` (required; `mark=`/`unmark=` counts).

### 4.7 Validated ledger (user decision 2026-10-03: "every row we actually validate is marked, so a final validation
pass knows to skip it")
- `B<n>/validated.tsv` (`steam_addr name verdict tier gog_addr batch fp_sha evidence_ref`): one row per validated
  (address, shipped name). Written by `finalize` (the exported rows: verified, plus likely with `--with-likely`;
  name = GOG `Class::Name`) and, for B0, by `tools/branch_apply.py` (every shipped branch row under its KPM name; verdict
  verified / likely / `parity`). `tier` = `opus` (override) / `auto:c2` / `auto:sig` / `reviewer`. `fp_sha` = first 16 hex
  of sha1 over the Steam fp.tsv cells `size retn n_insn thunk_target callees str_refs disp imm`.
- `persist --apply` upserts it into the KDB table `steam_validated` (PK `steam_addr, name`, + `date`). A name is marked at
  one address only: a newer mark moves it (`unmark` of the old address). This batch's old marks that its current
  `validated.tsv` drops are removed. Idempotent like the rest of persist.
- `batch.py check-validated [--branch-db DB] [--out FILE]` (read-only) classifies every branch-DB `functions` row:
  `validated` (marked at this address under this name, body hash unchanged): a final validation pass skips it;
  `stale` (marked at another address, or the body hash changed): re-check; `unvalidated`: never gated: check.
  Phase E's final review reads only the non-`validated` rows.

### 4.6 Opus gate per batch (Opus calibration gate 2026-10-03, calibration/OPUS_REVIEW.md "Reviewer tier")
The Sonnet reviewer tier is a filter, not a final gate. Per batch:
- **100% Opus** reads: every reviewer `corrected_addr`; every `label_suspect`; every `match` that cites a layout/offset
  difference; every `match` on a row flagged `mac_contra`, twin (`sig_twin`, R2) or `name_held_elsewhere`.
- **20% random sample (minimum 5) of the remaining reviewer `match` rows, C0 rows first.**
- **Every chunk carries at least 10 decoy cards, 0 tolerance.** `cards` always mixes them in: prev / next same-retn
  neighbours (nearest within 8 starts) of REVIEW rows, hosted preferably by rows of other chunks; they are labelled only
  in `cards/key.tsv` (kind `decoy_prev` / `decoy_next`), never in the chunk file. Any decoy accepted (`match` or
  `label_suspect`), or any Opus-found error in the sample, sends 100% of that batch's reviewer matches to Opus.
- After 3 consecutive batches with 0 errors in the sample and 0 decoy accepts the sample rate may drop to 10%.
- AUTO rows get a per-batch Opus meaning check of 5 `sig` + 3 `c2` rows.
Tooling: `finalize` scores the decoy cards from `cards/key*.tsv` against the reviewer TSVs, writes `B<n>/opus_gate.md/json`
(decoys per chunk, accepted decoys, the 100% list, the sample, the AUTO meaning checks, the sample rate from the previous
batches' `opus_gate.json`) and prints the escalation decision (`OK` / `ESCALATE` / `INSUFFICIENT DECOYS`). After the Opus
pass, re-run `finalize --opus-sample-errors N`. `tools/score_review.py KEY REVIEW` prints the same decision for one chunk.
- **Decoy key errors** (fix F1). A decoy card is a key error if the reviewer accepted it and an Opus `overrides.tsv`
  row ships that same (gog, steam) pair as `verified` or `likely`: the decoy was the true counterpart. Key errors
  are listed in their own `opus_gate.md` section, `## Decoy key errors`, and counted as `key_errors` in the json.
  They are not counted in `decoys_accepted` and never escalate. `score_review.py` runs before Opus, cannot know
  this, and is unchanged.
- **Opus part files** (fix F2). `batch.py opus-parts B<n> [--rows 100]` reads the row lists in `opus_gate.json`
  (`rows_full`, `rows_sample`, `rows_auto`). It writes `B<n>/opus/part_a.md`, `part_b.md`, ... with at most N rows
  each, in the order 100% rows, then the sample, then the AUTO meaning checks. Every row lists its Class::Name, GOG
  address, Steam address and why it is listed. It also writes `B<n>/opus/parts.json`. Opus briefs name a part file,
  never a line range of `opus_gate.md`.

### 4.8 Command-table source (`tools/cmdtable.py`, B15)
NWScript command handlers (`ExecuteCommand*`) are proven by engine data, not by labels:
- **Input.** `server/ghidra_scripts/ExportCommandStores.java` is a read-only server export. Its output is archived
  in `server/cmdstores/` as `SERVER/<program>/cmdstores.tsv` (`fn insn base disp target`).
- **The table.** `InitCommands` fills a 0x36D-slot (877) handler table with `MOV [reg + stride*i], handler`. Slot i
  is the i-th function declared in `nwscript.nss`. A second init function stores the 103 SWMG minigame slots.
  - GOG: 0x77B530 + 0x7F9140.
  - Steam: 0x665F50 + 0x6F5B80.
  - Mac x86: 0x324A7C + 0x3579D4.
- **Stride.** 4 bytes on Windows (MSVC function pointers). 8 bytes on Mac x86 (gcc pointer-to-member `{ptr, adj}`).
- **Store order.** The init functions are read in order, and within each in instruction order. The last store to a
  slot wins.
- **Slot 156 double store.** Both exes store slot 156 twice. The first handler is dead (no callers): GOG 0x78D6B0,
  Steam 0x6780D0. The second is live: GOG 0x78C440, Steam 0x676E60. The dead pair is matched as
  `cmdtable_overwritten`.
- **Counterpart.** A GOG handler's counterpart is the Steam handler with the identical slot set (`match=exact`).
  All 655 GOG handler sets have exactly one Steam set. Source tag `cmdtable`.
- **Name checks.** `name_slot` (yes / no / n/a) says whether slot(`<X>`) of `ExecuteCommand<X>` is in the GOG
  handler's set; `no` = label suspect. `name_mac` says whether the GOG name is the Mac real symbol of that slot set
  (653/655 yes).
- **Outputs.**
  - `B<n>/cmdtable.tsv`, `cmdtable_rows.tsv`, `cmdtable_report.md`;
  - `extra_candidates.tsv`, for `candidates --extra-candidates`;
  - `card_facts.tsv`: the GOG slots, which never name a Steam address.
- **TBL anchor family** (deferred). An exact unique slot-set identity as an anchor family counting toward C2 would
  change the AUTO rule. It needs an Opus calibration review first (section 4.2), before B10b (dispatch switch
  tables). B15 has no TBL family: the gate ranked `near` over `cmdtable` in 7 rows, and Opus corrected all 7.

### 4.9 REA live evidence (report-only)
- **Flow.** `cards B<n>` -> `tools/rea_questions.py B<n> [--per-run 8] [--keys F]` -> `B<n>/rea/q_<k>.md` (<= 8 Steam addresses each) -> one `rea-recon` Haiku per file (<= 12 parallel) -> ledger `patch_manager_mods/ghidra_knowledge/rea_findings/<date>_B<n>-q<k>.tsv` -> `cards B<n> --rea [--out DIR]` prints `### Live evidence (REA, report-only)` per card, from ledger rows at the card's effective Steam address (topic prefix `B<n>-q` only) -> reviewers -> finalize -> Opus gate (rules unchanged).
- **Blindness.** Question files mix real and decoy card addresses, sorted and shuffled with seed `B<n>`. No GOG address, name, kind or card id. Agents write behavioural summaries, never names copied from `steam_functions.tsv` or other ledgers. Every card (real and decoy) gets the section by the same code path.
- **Exe.** REA opens only `~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe` (Steam md5 `6a522e71`), never the game folder (live exe = vanilla + 4 GB patch, md5 `62411a3a`).
- **Output.** The ledger is the only output. Agents never write either DB. `rea-row-verify` stays off DB rows.
- **Trust.** REA rows are report-only evidence. They change no gate, AUTO rule, ranking or trust route (section 1 trust tiers; 4.2 gate classes). A row counts as a content anchor only when it quotes an asm line. Promotion (e.g. REA callers as A-CALLEE) only via an Opus calibration (MASTER_PLAN R4).
- **Cost/time.** Cold first procedure call ~356 s. Target <= 8 addresses and < 100k tokens per run. Pilot result in `B13a/rea_pilot/PILOT.md` (R1).

## 5. Process rules
- **The PR changes only the Steam DB** (user, 2026-10-03). `kotor2_gog_aspyr.db` in the KPM repo is read-only input:
  never edited, never committed. GOG label suspects / wrong GOG param sizes are recorded (gate report, PR description),
  and a GOG DB change is proposed to the user only for a confirmed bug.
- Agents never commit, never write either DB, never touch the game folder or credential files; private `mktemp -d`.
- One `analyzeHeadless` at a time, always `-readOnly` (`pgrep -af '^java.*analyzeHeadless'` first).
- Branch-DB writes: the lead only, after a backup to `~/Games/KOTOR2_Mod_Workspace/backups/steam_db/`, via
  SqliteTools, then `SqliteTools validate` (0 errors).
