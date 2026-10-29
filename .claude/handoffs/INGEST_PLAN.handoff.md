# Session Handoff & Plan State: patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md

## 0. Next-Phase Direction
Execute I8f: Persist B18 (+ B17 likely 3 via `finalize --with-likely`) + I8d names (user OK), gap re-measure, docs sweep, memory

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`
- **Branch:** `KOTOR-II` (6177ec90 INGEST I9d: batch B20 (graphpair + I9c rows): 36 verified, 13 likely, 8 unresolved; decoys 0/60 accepted, Opus sample_errors 0; persist dry run insert=35)
- **Generated:** 2026-10-29 00:08:45 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
6177ec90 INGEST I9d: batch B20 (graphpair + I9c rows): 36 verified, 13 likely, 8 unresolved; decoys 0/60 accepted, Opus sample_errors 0; persist dry run insert=35
 .../steam_db_batches/B20/candidates.tsv            |   58 +
 .../steam_db_batches/B20/cards/chunk_1.md          | 2140 ++++++++++++++++++++
 .../steam_db_batches/B20/cards/chunk_2.md          | 1843 +++++++++++++++++
 .../steam_db_batches/B20/cards/chunk_3.md          | 2102 +++++++++++++++++++
 .../steam_db_batches/B20/cards/chunk_4.md          | 1980 ++++++++++++++++++
 .../steam_db_batches/B20/cards/chunk_5.md          | 1823 +++++++++++++++++
 .../steam_db_batches/B20/cards/chunk_6.md          | 1884 +++++++++++++++++
 .../steam_db_batches/B20/cards/key.tsv             |  107 +
 .../steam_db_batches/B20/evidence.tsv              |   58 +
 .../steam_db_batches/B20/final_rows.tsv            |   58 +
 .../steam_db_batches/B20/functions.csv             |   37 +
 .../steam_db_batches/B20/gate_report.md            |  133 ++
 .../steam_db_batches/B20/gog_label_suspects.tsv    |    1 +
 .../steam_db_batches/B20/kdb_upsert.json           |  290 +++
 .../steam_db_batches/B20/opus/overrides_a.md       |    9 +
 .../steam_db_batches/B20/opus/overrides_a.tsv      |   23 +
 .../steam_db_batches/B20/opus/part_a.md            |   15 +
 .../steam_db_batches/B20/opus/parts.json           |   22 +
 .../steam_db_batches/B20/opus_gate.json            |   87 +
 .../steam_db_batches/B20/opus_gate.md              |   34 +
 .../steam_db_batches/B20/overrides.tsv             |   23 +
 .../steam_db_batches/B20/recheck.tsv               |    1 +
 .../steam_db_batches/B20/review/haiku_c1.tsv       |   19 +
 .../steam_db_batches/B20/review/haiku_c2.tsv       |   19 +
 .../steam_db_batches/B20/review/haiku_c3.tsv       |   19 +
 .../steam_db_batches/B20/review/haiku_c4.tsv       |   19 +
 .../steam_db_batches/B20/review/haiku_c5.tsv       |   19 +
 .../steam_db_batches/B20/review/haiku_c6.tsv       |   17 +
 .../steam_db_batches/B20/validated.tsv             |   36 +
 29 files changed, 12876 insertions(+)
```
- **Dirty Files (195):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/52_quick_loot.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ... +185 more (run `git status --short`)

## 2. Immediate Objective (I8f)
**Persist B18 (+ B17 likely 3 via `finalize --with-likely`) + I8d names (user OK), gap re-measure, docs sweep, memory**

Status: TODO
Task: Persist B18 (+ B17 likely 3 via `finalize --with-likely`) + I8d names (user OK), gap re-measure, docs sweep, memory

## 2b. Upcoming Batches in Pipeline
- **I9e** : Off-gap bonus: B21 = the 51 AUTO rows the wide B19 run found outside the gap; Opus gate AUTO (20% sample + all C1), REVIEW rows parked
- **I9f** : Persist B20/B21 + I9b promote list (user OK), re-card OnUpgradeSelected with the new decoy pool, gap re-measure, docs, memory

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate I8f` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- Gap prototype (I0 starting point): `.claude/research/ingest/gap_prototype.py` (4368b1e4); `python3 .claude/research/ingest/gap_prototype.py` prints 79 / 515.
- `name_map` marks bad names with confidence `wrong` (38 rows incl. 0x006a8fe0, demoted 10-27); there is no `superseded` value. `static_facts.py` uses only `high` names.
- `rea_ledger.py lint` baseline is 84 issues (10-27), not 53; it grows when other sessions add ledger files, so compare before/after your own run.
- rea-recon v2: peak ctx 66k on 2 addrs after the Grep cap (115k on 4); it still Globs for the exe even when given the path. REA write tools are denied in `.claude/settings.json` (87948820).
- Haiku briefs: token caps are ignored (112-200k peaks); cap tool calls and name line ranges.
- Verify: `cd patch_manager_mods/ghidra_knowledge/tools && python3 -m unittest discover -s tests` (314 OK after I0/I1, 10-27); `python3 scripts/query_ghidra.py address 0x0050dac0` (shows `[rea]` RenderGrassPolys).
- I0/I1 landed 335c5487 (+3bf1bf2e plan rows): 69 rows in `rea_findings/20261027_ingest-modcode.tsv` (24 function, 45 global). `ledger --apply` refuses if the target file exists: never re-run it for the same date/topic.
- `agent_contract.py check` is unreliable on this tree: baseline snapshot caps at 2000 dirty paths, so new untracked files read NOT_DIRTY and other sessions' commits read OUT_OF_SCOPE. Gate with `git status --porcelain -- <scope>` instead.
- Extract quality: 45 globals are mostly bare mod constants (`AddrPathN`, `kConstVA`, IAT slots, vtables) and `0x0098bfe0 TexMgr::Tick` is a global (likely a vtable slot); weakest candidates for I2. The 116 KDB `conflicts` include comment prose captured as names (e.g. "Party entry -> creature ..."); never ledger them.
- Name conflicts kept out: `.claude/research/ingest/conflicts.tsv` (0x0046a980 DoFog/SetFogRange, 0x0085d430, 0x0098c45c).
- CLAUDE.md is dirty with another session's hunks: commit your hunk via `git hash-object -w` of HEAD+hunk, `git update-index --cacheinfo`, then mirror the edit to the working file.
- 10-28 (I2-I5 lessons): Haiku defaults to `unsure` on thin cards; static-facts-only function cards gave 38-57% decoy rejection, `cards --decomp-lines 10` gave 81%. Getter decoys (one-field return) still pass: treat getter names as weak. Promote strict = consistent in every pass.
- 10-28: name/addr regex used to span ", " in lists (mis-paired the next address); fixed, plus `.claude/research/ingest/` excluded from notes. After any extractor change, re-extract and drop ledger rows that lose source support.
- 10-28 next batch sizing (dry runs, nothing written): `extract --source notes --top 100` now yields only 25 new rows (24 ledger-able functions); `extract --source modcode` yields 2 (globals 0x009960c8 EffMasteryLight, 0x009960cc EffMasteryDark, 4 bytes apart so they cannot decoy each other). Gap 460 notes / 53 modcode is mostly addresses without any name in text: new names need another source.
- `promote_names.py --stage I<n>` labels evidence; DB copies before writes went to session scratch (`kdb_before_I4*.db`).
- Do not chain `test | tail && apply`: tail masks the test exit code (I4b applied with 1 red test; it was a test-edit bug).
- 10-28 I6 (74b64d63): 12 names, gate 11/13; text extraction exhausted (gap 452/52). Only Steam-DB classes get `fn_doc.py page` output ("no functions for class X" is normal). `steam_docs/_work/` is another session's, never commit it.
- 10-28 I7 sizing (scratch script, not committed as a tool): `.claude/research/ingest/i7/sizing_gap_gog_twins_20261028.tsv` (342 rows `steam gog name`). Full gap list = `ingest_names.gap(top_n=100000)[g]["top"]`. Key that worked: `(size, retn, n_insn, disp, imm, #callees, #str_refs)` from fp.tsv, unique in BOTH exes. Server Steam names are mostly FID noise (ANALYSIS 15295); never use them as names.
- I7 verify: `cd patch_manager_mods/steam_db_batches/tools && python3 -m unittest discover -s tests` (scope it alone, check exit); batch dir `patch_manager_mods/steam_db_batches/B17/`.
- 10-28 I7a DONE (4b23629e): `ingest_names.py twins --out .claude/research/ingest/i7/twins.tsv` -> gap=458 named=342 unnamed=20 ambiguous=58 no_twin=38 no_fp=0 (twins.tsv committed, force-added). `batch.py candidates --gog-list FILE` landed. Suites: ghidra tools 346 OK, steam_db_batches tools 331 OK. 2 Sonnet workers ~122k tokens total; `agent_usage.py --since` read 0 (workers logged under another session id). Next = I7b (B17 candidates with `--gog-list` + `--extra-candidates` on twins.tsv).
- 10-28 I7b/I7c: `batch.py cards --chunk N` counts real + decoy cards (`--chunk 12 --decoys 10` gives 2 real per chunk, 89 chunks). Haiku reviewers on 22-card chunks peaked 186-265k tokens: use `--chunk 16 --decoys 10`.
- `merge_overrides.py B<n>` reads `B<n>/opus/overrides_*.tsv` (copy the Opus gate TSVs to that name). `plan_manager.py mark-done` needs `--plan <path>` (the shared pointer pointed at doc 51). Use the python sqlite3 backup API (no `sqlite3` CLI); backup `kdb_before_I7c.db` is in the 10-28 session scratch.
- `fn_doc.py page` covers only branch Steam DB classes; B17 refreshed CSWGuiControl/CSWGuiManager/Scene only.
- Commit e80e8c5f message wrongly says the 3 likely rows went in as medium: only 285 verified rows were persisted (no `--with-likely`).
- 10-28 I8a-e (1ea4cc11, 16c942af, f7b574f1): twins --buckets/--order; macnames; B18 11 verified/1 likely; B19 12 verified/1 likely; I8d 0 promoted (decoy 2/3). I8f (persist 26 rows: B17 likely 3 + B18 11 + B19 12) awaits user OK; B18/B19 likely rows not in kdb_upsert (no --with-likely).
- I8 inputs kept in `.claude/research/ingest/i8/` (buckets.tsv, twins.tsv, macnames.tsv). I8e span script (not a tool): for each no_twin steam fn take nearest trusted pairs (load_kdb_pairs) on both sides; if gog increasing, GOG fns of gog_functions.tsv strictly between them not in name_map (<= 25) -> `candidates B19 --gog-list`. 22 of 38 had windows > 25 or crossing anchors.
- Global Mac search by mac_fit is noise (margin ~0 over 15.6k fns); restrict candidates by class. Mac name hypotheses yield little (3/30 kept).
- Haiku reviewers wrote `corrected_addr=none` -> finalize ValueError in bdlib.parse_addr; workaround = blank non-hex cells in review TSVs (I9a fixes the tool).
- Opus gate check that decided every row: `fp equal + callers/callees mapped through load_kdb_pairs`; mapper pick for StopAllOneShots (0x70b8f0) was wrong, fptwin_order 0x70bb00 right. Steam RenderStandard 0x4f5f50 inlines GOG RenderMotionBlurred (+1008 B).
- `batch.py finalize` prints INSUFFICIENT DECOYS whenever a chunk has < 10 decoys (short last chunk); it is not a reviewer failure.
- Verify: ghidra tools `python3 -m unittest discover -s tests` (from tools/) OK; steam_db_batches `python3 -m unittest discover -s tools/tests` OK (10-28).
- 10-29 I9a-d (75eb6bfd, 79874e21, c6ac2086, 6177ec90): `graphpair` on i8 buckets = 73 rows (66 named); 14/14 rows overlapping B17-B19 verified agree (precision signal). Inputs/outputs in `.claude/research/ingest/i9/` (graphpair.tsv, b20_extra.tsv, crt/). Explain helper (not a tool): session scratch `explain.py` = per-pair mapped caller/callee hits via `trusted_map` + `_caller_index`.
- I9b CRT: 25 ledger rows `rea_findings/20261029_ingest-crt.tsv` (22 twins, FID_conflict skipped, + _fread/_memmove/__vsnprintf from graphpair); score 96% decoys (C18 miss on _fread, FLIRT header in decomp says _fread). Promote list `.claude/research/ingest/i9/crt/promote.tsv` (25) awaits user OK.
- B20: 36 verified / 13 likely / 8 unresolved; persist dry run insert=35. Re-gate later (gate top != graph pick): RenderDrawableSquares 0x004356e0, AurCubeMapAvailable 0x00484970, AurEnableShadows 0x00478ca0, GetTextureTID 0x004daf90. GetAtPos (0x007f3870 -> 0x00561430) skipped: fp namespace splits the template name ("long>").
- Haiku B20 reviewers peaked 137-149k on 18-card chunks (4 tool calls held); 3 rows had column shift (verdict blank, `match` in corrected_addr) -> finalize ignores them; read them in the Opus gate. CRT cards (13/chunk, 10 KB) peaked ~60k.
- I9e (B21 off-gap) skipped this session (optional).
