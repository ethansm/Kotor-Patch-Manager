# INGEST plan: backfill names from mod code + research notes into the REA ledger / KDB

Resume: `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`.
Handoff: `.claude/handoffs/INGEST_PLAN.handoff.md`. Follow-up to `DECOMP_PLAN.md` (D0-D8 DONE 10-27).

## Why
Gap scan 2026-10-27 (scratch `gap.py`, $0): Steam-exe functions cited elsewhere but absent from `name_map`
(non-`wrong`) and every `rea_findings/*.tsv`:

| source | files | functions cited | missing |
|---|---|---|---|
| mod code (`patch_manager_mods/*_src/**/*.{toml,h,cpp,inc}`) | 578 | 380 | 79 |
| research notes (`.claude/research`, `patch_manager_mods/*research*`, `engine_research_findings`, numbered docs, `.claude/reviews`) | 836 | 1660 | 515 |

Examples: `0x0050dac0` = `CAurTriangleBin::RenderGrassPolys` (grass hooks.toml:44-48, live hook 4) and `0x007457e0`
= `CSWGuiMainInterface_PortraitIconRowDraw` (buff HUD, 33 cites in 4 docs): both unknown to `query_ghidra.py`.
D6 R2 NO-GO root cause was missing names / caller context, so coverage is the lever.

## Phases

| # | Phase | Status | Record |
|---|---|---|---|
| I0 | `tools/ingest_names.py gap| DONE | TODO | spec §I0 |
| I1 | `ingest_names.py ledger`: static gate + write `probable` ledger rows for `new` mod-code candidates (topic `ingest-modcode`); conflicts report only; lint stays at baseline | DONE | spec §I1 |
| I2 | Haiku verify with decoys on I1 rows (cards from `static_facts.py`); verdict TSV; promotion list | DONE | spec §I2 |
| I3 | Research-notes extraction (same tool, `--source notes`), top-cited first, Haiku verify, ledger rows topic `ingest-notes`; stop when yield < 30% | DONE | spec §I3 |
| I4 | Promotion (user OK) of verified names into `name_map`; regen class pages for touched classes; gap re-measure | DONE | spec §I4 |
| I5 | Docs sweep (tool listed free-first in CLAUDE.md, RE_AGENT_GUIDE §2.1, README, SUBAGENT_ROLES) + memory | DONE | Decisions |
| I6 | Next batch: the last 26 extractable candidates (24 notes, 2 mod-code globals) through cards `--decomp-lines 10 --decoy-every 2`, Haiku verify, ledger + name_map (user OK) | DONE | handoff |
| I7a | Tools for the non-text source: `ingest_names.py twins` (gap fns -> unique GOG fingerprint twin) + `batch.py candidates --gog-list FILE` | DONE | spec §I7 |
| I7b | Batch B17 (gap twins) through `steam_db_batches/tools/batch.py` candidates -> gates -> cards -> Haiku review -> finalize -> Opus gate | DONE | spec §I7 |
| I7c | Persist B17 to `name_map` only (user OK), class pages, gap re-measure, docs sweep, memory | DONE | spec §I7 |
| I8a | `ingest_names.py twins --buckets F`: write every gap fn with its bucket (named/unnamed/ambiguous/no_twin) + the colliding GOG/Steam addrs; `--order` pairs ambiguous groups by address order inside the run | DONE | spec §I8 |
| I8b | `ingest_names.py macnames`: Mac x86 symbol hypotheses (bdlib `mac_fit`, strings) for GOG-unnamed twins + the 7 B17 label_suspect rows + GOG 0x5C54B0 | DONE | spec §I8 |
| I8c | Batch B18: B17 leftovers (18 unresolved + 3 G6-wrong) + ambiguous rows I8a pairs uniquely; cards `--chunk 16 --decoys 10`; Haiku review -> finalize -> Opus gate | DONE | spec §I8 |
| I8d | Mac-name verify: I8b hypotheses through `ingest_names.py cards --decomp-lines 10` (decoy names), Haiku verify, ledger rows topic `ingest-macnames` | DONE | spec §I8 |
| I8e | no_twin 38: `batch.py candidates` (mapper + near only) + `gates` for B19, $0; report AUTO/REVIEW counts; review only if REVIEW <= 24 | DONE | spec §I8 |
| I8f | Persist B18 (+ B17 likely 3 via `finalize --with-likely`) + I8d names (user OK), gap re-measure, docs sweep, memory | DONE | spec §I8 |
| I9a | `ingest_names.py graphpair`: score GOG candidates for ambiguous + no_twin gap fns by callers/callees mapped through trusted pairs (+ fp equality); emit unique winners as `fpgraph` rows; + `bdlib` tolerate non-hex `corrected_addr` | DONE | spec §I9 |
| I9b | CRT lane: the 23 named twins absent from `gog_functions.tsv` (21 MSVC CRT by FID) -> `twins --extract-out` -> ledger topic `ingest-crt` + blind cards (decoy pool from name_map), Haiku verify | DONE | spec §I9 |
| I9c | `graphpair` re-check of the 13 B18 unresolved (incl. fp-group peers as alt candidates); corrections go into B20 | DONE | spec §I9 |
| I9d | Batch B20: graphpair rows (ambiguous + no_twin + I9c corrections) -> gates -> cards `--chunk 16 --decoys 10` -> Haiku -> finalize -> Opus gate | DONE | spec §I9 |
| I9e | Off-gap bonus: B21 = the 51 AUTO rows the wide B19 run found outside the gap; Opus gate AUTO (20% sample + all C1), REVIEW rows parked | DONE | spec §I9 |
| I9f | Persist B20/B21 + I9b promote list (user OK), re-card OnUpgradeSelected with the new decoy pool, gap re-measure, docs, memory | DONE | spec §I9 |
| I10a | Batch B22 re-gate: 4 B20 graph corrections (RenderDrawableSquares 0x004356e0, AurCubeMapAvailable 0x00484970, AurEnableShadows 0x00478ca0, GetTextureTID 0x004daf90) + Camera::MapToScreen (never batched) + 2 fresh graph rows + 6 B21 likely -> gates -> cards -> Haiku -> Opus gate | DONE | spec §I10 |
| I10b | CRT G1 lane: sprintf/memset/free rejected by G1 (Steam ANALYSIS already says `_sprintf`/`_memset`/`_free`) -> extract rows with the FLIRT name -> ledger `ingest-crt` -> cards `--decoy-pool kdb` -> Haiku -> score | DONE | spec §I10 |
| I10c | Label suspects (7, B17/B18): graph check each Steam fn vs its GOG label + neighbours; record verdicts in `steam_db_batches/GOG_LABEL_SUSPECTS.md` ($0, no KDB write) | DONE | spec §I10 |
| I10d | Persist B22 + I10b promote (user OK); user decision on the 13 B20 likely (`finalize B20 --with-likely` = persist at likely); gap re-measure, memory | DONE | spec §I10 |

## Cost log

| phase | start (ISO) | agents | est. cost | note |
|---|---|---|---|---|
| I0 | 2026-10-27T20:51 | 1 Sonnet worker (I0+I1 in one brief, 12 tool calls, peak 93k) | $0.52 | 21 tests, suite 314 OK; gap 79/515; extract 337 rows (new 150, agrees 71, conflicts 116) |
| I1 | 2026-10-27T20:51 | (same worker) | $0 extra | 69 rows -> `rea_findings/20261027_ingest-modcode.tsv` (24 function, 45 global), 3 name conflicts; lint 84 -> 84; modcode gap 79 -> 57 |
| I2 | 2026-10-27T22:40 | 8 Haiku recon (chunks) + 4 Haiku retries (chunks 3/5/6/7), 3 tool calls each, peak 51-69k | $0.15 | `ingest_names.py cards|score` (+8 tests, suite 322 OK); 86 cards (69 + 17 decoys, seed 1027); pass 1 FAIL 10/17 = 59% (all 7 misses `unsure`, 0 decoys accepted); retry with generic role calibration PASS 14/17 = 82% (function 2/4, global 12/13) but accepted 2 decoys (C31, C34); `promote.tsv` 62 rows (20 function, 42 global), 51 of them also consistent in pass 1 (strict set); 7 held back incl. `TexMgr::Tick`; lint 84 -> 84 |
| I3 | 2026-10-27T22:48 | 1 Sonnet worker (`extract --source notes`, `--top`, `ledger --out`, +11 tests) + 9 Haiku + 6 Haiku retries | $0.57 (all I3+I4 agents) | gate FAIL: pass 1 decoys 8/21 = 38%, retry 12/21 = 57%, 4 decoys accepted; real rows 61% consistent (stop rule not hit); nothing written to the ledger; lead added `TOOL_NAME_RX` (REA tool names); details `.claude/research/ingest/i3/README.md` |
| I3b | 2026-10-27T23:17 | 9 Haiku (richer cards) | $0.15 | `cards --decomp-lines 10` (+5 tests, suite 339 OK); decoys 17/21 = 81% PASS (function 17/20), 2 getter decoys accepted; 61 consistent -> ledger, then comma-pairing regex bug found and fixed, 5 unsupported rows removed -> `rea_findings/20261027_ingest-notes.tsv` 56 rows; lint 84; notes gap 455; I1 names in name_map unaffected |
| I4 | 2026-10-27T22:48 | none (lead) | $0 | user OK "I4 strict": new `tools/promote_names.py` (+4 tests; dry run default, one transaction + integrity_check, never overwrites); 51 strict rows -> `name_map` (source `ingest-modcode`, confidence `medium`, 0 exists/conflicts, integrity ok; pre-write copy in session scratch); class pages `steam_docs/CSWGuiMainCharGen.md` + `Scene.md` (CAurTriangleBin, CSWGuiMainInterface have no Steam-DB functions, so no page); gap after: modcode 57 (unchanged, rows were already ledgered at I1), notes 556 (was 515; notes corpus grew 10-27) |
| I4b | 2026-10-28T00:03 | none (lead) | $0 | user OK "I4b": 56 I3 rows -> `name_map` (source `ingest-notes`, medium, 0 exists/conflicts, integrity ok); `promote_names.py --stage` (evidence names the verify stage, +1 test); class pages `CSWGuiControl.md`, `CSWGuiManager.md` (30 other classes have no Steam-DB functions); name_map ingest total 107 |
| I5 | 2026-10-28T00:10 | none (lead) | $0 | docs sweep b2d4aa6c: `cards|score` + `promote_names.py` in CLAUDE.md (own hunk via index), AGENTS.md, README, RE_AGENT_GUIDE §2.1 (2 new rows), SUBAGENT_ROLES, kotor2-persist-knowledge skill; check_skills PASS |
| I6 | 2026-10-28T00:26 | 4 Haiku recon (chunks), 3 tool calls each, peak 50-77k | $0.07 | 26 candidates (24 notes, 2 modcode globals) -> 39 cards (13 decoys, seed 1030, `--decoy-every 2`); one pass, decoys 11/13 = 85% PASS (misses C07 consistent, C13 unsure); 12 consistent -> `rea_findings/20261028_ingest-notes.tsv` (11) + `20261028_ingest-modcode.tsv` (1, EffMasteryDark; EffMasteryLight not promoted); lint 84; user OK all 12 -> `name_map` (stage I6, 0 exists/conflicts, integrity ok; ingest total 119); weak: `CAurGUIStringInternal::GetFontPixelHeight` (getter), `Scene::AddObject` (generic vtable call; decoy on same facts got unsure); only `Scene` has a Steam-DB page (unchanged); gap notes 452 / modcode 52. Text-based extraction is exhausted: remaining gap has no name in notes or mod code, needs a non-text source |

## Spec

### I0: extractor + gap metric ($0 at run time)
- New `patch_manager_mods/ghidra_knowledge/tools/ingest_names.py` (stdlib), tests
  `tools/tests/test_ingest_names.py` + fixtures `tools/tests/fixtures/ingest/`.
- `gap [--source modcode|notes|all] [--json]`: reproduces the table above (exe addresses `0x00[4-9]xxxxx` below
  `0x980000`, mapped to the containing function via `steam_functions.tsv` starts; known = `name_map` confidence !=
  `wrong` + all ledger addrs).
- `extract --source modcode --out FILE`: name/address pairs from
  1. `*.hooks.toml`: `address = 0x...` plus the nearest preceding comment naming `Class::Method` (within 6 lines);
  2. `constexpr <type> <Name> = 0x00XXXXXX;` in `.h/.cpp/.inc` (e.g. `_shared_src/GameAddr.h:15`
     `constexpr uintptr_t PlayGuiSound = 0x004122A0; // thiscall(mgr, char idx)`), name = `<Name>`, trailing comment kept;
  3. comment lines matching `(\w+::\w+)\W{1,6}(?:at|@|=)?\s*0x00[4-9][0-9A-Fa-f]{5}` or the reverse order.
- Output columns: `addr kind name src fn_start offset status kdb_name` where kind = function (addr == fn_start),
  callsite (inside a function), global (>= 0x980000); status = new | agrees | conflicts (vs `name_map`).
- Names that are not Class::Method (`PlayGuiSound`) stay as-is, flagged `bare` in a `form` column; never invent a class.

### I1: ledger rows
- `ingest_names.py ledger --in FILE --topic ingest-modcode [--apply]` (dry run default): only `status=new`,
  `kind=function|global`. Row: `kind` function/global, `confidence probable`,
  `evidence` = `<addr>: <first asm insn from the dump .asm> | src <path:line>` (passes L2), date today.
  One file `rea_findings/<YYYYMMDD>_ingest-modcode.tsv` via `rea_ledger.append_rows` (README convention 2).
- Several names for one addr: keep the most frequent; the rest go to `conflicts.tsv`. `conflicts` (vs KDB) are never written.
- Gate: `rea_ledger.py lint` issue count unchanged; `query_ghidra.py address 0x0050dac0` shows `[rea]` RenderGrassPolys.

### I2: Haiku verify (model-verify rule: every promoted row needs Haiku + decoys)
- Cards: `static_facts.py addr` output + the candidate name; 1 decoy per 4 (a real name from another card).
  Verdict per card: consistent | inconsistent | unsure + one quoted line. Chunks of <= 12 cards per Haiku, cap 10 tool calls.
- Pass: decoys rejected >= 80%; rows consistent and not decoys -> `promote.tsv`.

### I3: research notes
- `extract --source notes`: same pattern 3 plus markdown tables with an address column and a name column.
  Rank by cite count; process top 100 first; Haiku verify as I2; ledger topic `ingest-notes`.
- Stop rule: if a 100-row chunk yields < 30% consistent, stop and report.

### I4: promotion
- `promote.tsv` -> `name_map` only after user OK (lead write, source `ingest-<topic>`, confidence `medium`, integrity check).
- `fn_doc.py page <Class>` for each touched class; `ingest_names.py gap` again (report the new missing counts).

### I7: GOG fingerprint twins (the non-text source)
- Why: after I6 no text names remain (gap notes 452 / modcode 52, 458 unique fns). A 10-28 dry sizing (fingerprint key
  `size retn n_insn disp imm #callees #str_refs` from `upstream_server/k2_win_{steam,gog}_aspyr_swkotor2.exe/fp.tsv`,
  unique on both sides) gave: 342 unique named GOG twins, 20 unique unnamed, 58 ambiguous, 38 no twin; 319 of the 342
  are in `DUMP/inputs/gog_functions.tsv`; 153 classes (Global 69, 133 classes with <= 3 rows). Sample: 0x00652810 ->
  `CSWRoomSurfaceMesh::TransformAABBToWorld` (I6 rejected `AurGrassMaterialInUse` there).
- GOG labels are hypotheses (`steam_db_batches/GOG_LABEL_SUSPECTS.md`): every row goes through the B-batch gates G1-G8
  (Mac contra/rotation), blind cards with decoys, and the Opus gate. A fingerprint twin alone never ships.
- I7a `twins`: `ingest_names.py twins [--source all|notes|modcode] --out F` writes `gog steam source name cites`
  (source `fptwin`), usable as `batch.py candidates --extra-candidates F`; prints the bucket counts above.
  `batch.py candidates --gog-list F`: keep only GOG rows whose address is in F's `gog` column (combines with other filters).
- I7b: `B17` = next free batch id. Classes = classes of the listed GOG rows. Haiku is the reviewer first pass (chunks of
  12, >= 10 decoys per chunk via `cards`); `finalize` ESCALATE -> one Sonnet reviewer pass on that chunk (DESIGN §4.6).
- I7c: `persist B17 --apply` writes `name_map` (source `steam-db-B17`) and the validated ledger. The branch address DB
  (`kpm-pr-steam-db`) is NOT imported (Decision: address-DB out of scope); `B17/functions.csv` stays a file.
- Result (10-28): B17 gated 285 verified / 3 likely / 6 rejected / 18 unresolved / 7 label_suspect (150/150 decoys
  rejected, Opus sample_errors 0); `persist --apply` 283 insert + 2 update (replaced `wrong` grass names at 0x6527c0/
  0x652810), idempotent. Gap after: notes 171 / modcode 17 (was 452 / 52); twins left: named 58 (unshipped B17 rows),
  unnamed 20, ambiguous 58, no_twin 38. Cost: 15 Haiku reviewers at 186-265k tokens each (22-card chunks are 2x the
  100k ceiling; use <= 12-card chunks next time) + 3 Opus parts, $7.35 total.

### I8: after B17 (gap 174 = named-unshipped 58 / unnamed 20 / ambiguous 58 / no_twin 38)
- Inputs: `steam_db_batches/B17/final_rows.tsv` (18 unresolved, 6 rejected: 3 CRT G1 = final, 3 G6 `name_map ...
  wrong` at 0x00430300/0x00444b60/0x006ddee0 = re-check the old demotion), `B17/gog_label_suspects.tsv` (7).
  23 named twins are absent from `DUMP/inputs/gog_functions.tsv`, so `candidates` dropped them: I8a lists them.
- I8a `--buckets F` columns: `steam bucket gog_addrs steam_peers name cites`. `--order`: inside one ambiguous key group,
  pair the k-th Steam address with the k-th GOG address only when both groups have equal size and both lie in one
  monotone run (no other trusted pair between them breaks the order); tag `fptwin_order`. Tests in
  `tools/tests/test_ingest_names.py`. Verify: `cd patch_manager_mods/ghidra_knowledge/tools && python3 -m unittest
  discover -s tests`.
- I8b `macnames`: for each target GOG fn, the top-3 Mac x86 fns by `bdlib.Corpus.mac_fit` (fit >= 0.5, margin >= 0.15
  over the 2nd) + shared strings; output `gog steam mac_name fit margin`. Mac names are hypotheses (different compiler):
  they reach `name_map` only through I8d cards + decoys.
- I8c/B18 cards: `--chunk N` counts real + decoy cards; 6 real + 10 decoys = `--chunk 16 --decoys 10`. B17 22-card
  chunks cost Haiku 186-265k tokens each (2x the 100k ceiling).
- Budget: I8a/I8b/I8e $0 (code: one Sonnet worker each, Haiku checks); I8c ~6 Haiku chunks + 1-2 Opus parts (~$3);
  I8d ~3 Haiku chunks (~$1).

### I9: after I8 (gap left after I8f ~ notes 150 / modcode 15; ambiguous 49, no_twin 25, B18 unresolved 13, CRT 23)
- Lesson from the I8c/I8e Opus gate: the decisive check was not the cards but `fp equal + callers/callees mapped through
  trusted name_map pairs (GOG ... in evidence)`. I8 example: StopAllOneShots 0x70bb00 callee+caller 1/1 hit, while the
  mapper pick 0x70b8f0 had 0; AurEnableForceFury 0x478f50 callers 2/2 vs sibling 0x478ee0 1/2.
- I9a `graphpair --buckets F --out G [--no-twin-window 60]`: for each ambiguous gap fn, candidates = its GOG key group;
  for each no_twin gap fn, candidates = GOG fns that are callees of the GOG twins of its trusted Steam callers (and
  callers of the twins of its trusted callees), not already in name_map. score = callee hits + caller hits (mapped via
  `load_kdb_pairs` + unique fp twins); emit `gog steam source=fpgraph name score margin fp_equal` when best >= 2 hits and
  beats the 2nd by >= 2 (or fp_equal and 2nd has 0). Reuse `twins_full`, `_load_fp`, `load_kdb_pairs`, `_gname`.
  Ambiguous 39 equal-size groups are the main target; 18 rows sit in groups >= 20 (thunks / tiny stubs, expect 0).
- bdlib fix: `finalize` crashed on reviewer `corrected_addr = none` (bdlib.parse_addr ValueError); treat non-hex as blank.
- I9b: the 23 are exact fp twins whose GOG fn was FID-named (Ghidra Function ID) but are absent from gog_functions.tsv,
  so `batch.py` cannot see them and G1 would reject library fns anyway. Ledger lane (probable) instead: extract rows ->
  `ledger --topic ingest-crt` -> `cards --decomp-lines 10` with decoys drawn from name_map (I8d showed 3 rows give only
  1-3 decoys: add `cards --decoy-pool kdb`).
- I9e: wide B19 run (169 GOG rows, anchor spans <= 25) gave AUTO 63 (12 on gap) / REVIEW 43 / REJECT 14 / NOCAND 49;
  the 51 off-gap AUTO rows are names for Steam fns nobody cites yet (value: fewer future gaps). Rebuild via
  `.claude/research/ingest/i8/` inputs (`buckets.tsv`; rerun the I8e span script, see handoff §5).
- Cards: keep `--chunk 16 --decoys 10` (`--chunk 12` = only 2 real). B18 16-card chunks (~16k tokens each) still peaked
  Haiku at 130-160k: the peak is re-reads, not card count; cap reviewer tool calls at 4.
- Budget: I9a/I9c $0 (code + inline), I9b ~2 Haiku (~$0.2), I9d ~4 Haiku chunks (~$0.4), I9e Opus inline. Total < $2.

### I10: after I9 (gap 87 fns: named 23, unnamed 19, ambiguous 36, no_twin 9; name_map 5177)
- Re-run 10-29 (`.claude/research/ingest/i10/`): `twins --buckets` + `graphpair` with the grown trusted map adds only
  3 fresh rows (GetAtPos has a broken fp namespace `long>`; skip it). It confirms 2 of the 4 B20 corrections; the
  RenderDrawableSquares pair is itself a unique fp twin (GOG 0x00827df0 <-> Steam 0x004356e0).
- Named-bucket leftovers: label_suspect 7, rejected 6 (3 G6 known-wrong, 3 G1 library), unresolved 4, likely 4,
  never batched 1 (Camera::MapToScreen GOG 0x008c1b40 <-> Steam 0x004809d0), FID_conflict 1 (skip).
- B22 input = `gog steam source` TSV fed to `candidates --gog-list F --extra-candidates F`; cards
  `--chunk 16 --decoys 10 --facts`; reviewers 4 tool calls; finalize ignores rows whose verdict cell is blank
  (column shift): read them in the Opus gate.
- Budget: I10a ~2-3 Haiku (~$0.2), I10b 1 Haiku (~$0.05), I10c $0. Total < $0.5.

## Decisions
- Ledger first, KDB later: ledger rows are agent observations (`[rea]`), promotion needs Haiku+decoys and user OK.
- Address-DB (Steam DB worktree) is out of scope.
- New tool advertised in all agent docs in the same phase it lands (I5 closes the sweep).
