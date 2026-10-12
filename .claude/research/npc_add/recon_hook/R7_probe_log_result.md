# R7: NPC-add probe 0.1.0 log and diversity log results

Recorded 10-12 from waves A1 and A2 (`.claude/waves/w2-34/`). This record does not re-run the checkers.

Source tags:
- `[check]` = `python3 scripts/kotor2_logs.py check <log>` output, as reported by the wave.
- `[A1]` = `.claude/waves/w2-34/A1.md` (area_hook_probe_log.txt, `--all-launches`).
- `[A2]` = `.claude/waves/w2-34/A2.md` (npc-diversity_log.txt, per-launch split).
- `[R6]` = `.claude/research/npc_add/recon_hook/R6_area_loaded_hook.md`.

## Facts

### Area hook probe 0.1.0 (area_hook_probe_log.txt, 15 lines, 1 launch)

1. [A1] H3 (LoadModuleStart) is followed one tick later by H2 (LoadModuleInProgress) on the same module ptr. Visible rounds: t21089 and t26653/t26654. Every H4 line shows `last load hook: H2 fresh`.
2. H2 (0x55a460) fires on saved loads too. R6's claim that 0x5582f0 never reaches 0x55a460 is REFUTED. R6 lines 16, 46 and 77 rely on the callee list of 0x5582f0. Hooking H2, or running after it, covers saved and fresh loads.
3. [A1] The first load (L4) is labelled H3 "saved module load" by the probe. Not independently confirmed: lines 1-10 are hidden and the first resref is masked.
4. [A1] Area resref at area+0x110 reads 204tel and 208tel (sane). Key on resref.
5. [A1] area+0x124 id reads 00000001 for BOTH areas. Not unique. Dead key.
6. [A1] Area pointer 0c5d4420 is reused by both areas (area freed and recreated per load). Dead key.
7. [A1] H1 latch (this+0x14) never changed after its first value, and no DUMP was logged. Useless as an area-ready marker.
8. [A1] H4 "area changed" fires at the first add of the NEXT area and prints the previous area's counts: prev_area_calls 301 (204tel), 286 (208tel); total_adds 477, 764. It does not mark the end of population for the current area. The total_adds delta (287) vs prev_area_calls (286) is a one-call discrepancy, unexplained.
9. [A1] The H4 address is past the banner truncation, so mapping H4 to AddObjectToArea is unverified.
10. [A1] log_triage exposes only the last 5 raw lines plus masked signatures. Lines 1-10 are hidden, so save-load vs door transition for the 204tel entry at +435 s is unknown.
11. Result: no safe spawn point is derivable from 0.1.0. Probe 0.2.0 (`patch_manager_mods/area_hook_probe_src/AreaHookProbe.cpp` lines 7-8, 41) adds a POP watch: 30 ticks with no add call after at least one add gives `H1 POP-STABLE`; 1800 ticks without one gives `H1 POP-TIMEOUT`. In tree; no 0.2.0 log is cited in this record.

### Diversity log (npc-diversity_log.txt, 882 lines, 3 launches)

12. [A2] `check` judges the last launch only. `--launch N` is unsupported. The per-launch split was done by calling the checker's own functions on each slice.
13. [check] Launch 3 (0.1.4-probe): 120 RMP lines, 12 changed (10.0%). Why: skip:vary 74, nochg 18, ok 12, keep 7, skip:gender 7, tvarmor 2 (sums to 120). Percentage math: SUPPORTED.
14. [check] 5 of 17 commoner-row oids changed (29.4%). Keep rows: 0 changed.
15. [check] Checks 5, 9, 12, 13, 14, 15 PASS.
16. [check] Check 6 (exclusions) PASS is vacuous: tag is empty on 120/120 RMP lines, tagun=24/24. Exclusions: NOT SUPPORTED. Exclusion works only through the position-keep default.
17. [check] Bad textures: NOT JUDGEABLE from logs. The checker prints no texture names; 0 swaps.
18. [check] STAT counters (remap chg=0, tex calls 40) do not reconcile with 12 changed RMP lines / 88 TEX lines. Cause unverified.

### Per-launch table ([A2])

| Launch | Lines | Mod version | RMP lines | Changed share (rule 13) | Rule 11 (tv determinism) | Rule 16 (this launch only) | Oids with tv flip (tv b>a, b!=a) |
|---|---|---|---|---|---|---|---|
| 1 | L1-L99 | 0.1.1-probe | 43 | 0.0% (0/6) FAIL | INFO, not judged (no tv= fields) | INFO, not judged | 0 |
| 2 | L100-L572 | 0.1.2-probe | 172 | 90.0% (27/30) FAIL | PASS (39 oids stable per oid+final app) | PASS (39 keys stable) | 27 |
| 3 | L573-L882 | 0.1.4-probe | 120 | 29.4% (5/17) PASS | FAIL (oid 7f000000, final app 452, tv 1 and 102) | FAIL (same 1 key) | 5 |

Rule 16 keys, all launches ([A2]): 77 repeated (oid, before) keys carry 2+ tv= lines. 23 of them map to more than one (after, tv) value. Those 23 = 22 cross-launch (all L2 to L3) + 1 within-launch flip. 43 of 76 distinct oids appear in more than one launch.

## Bugs

- **B1 (real, fixed in source).** Rule 11 FAIL, launch 3, oid=7f000000, final app 452, tv=1 vs 102.
  Cause: `patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:470` passed `d.textureVar` as `currentTv` to `PickTextureVar`. `DiversityLogic.h:150-163` moves to the next allowlist slot when the draw equals `currentTv`, so re-application flips the variant.
  Fix: 2-arg call `PickTextureVar(oid, newApp)` at :470, in `0.1.5-probe` (manifest version confirmed in tree). Pin: `diversity_logic_test.cpp` lines 489-502 (comment at 494-495; 2-arg determinism at 500-501). Not built or installed in this task.
- **B2 (not a bug by itself).** Rule 16 FAIL: 22 cross-launch flips (all L2 to L3; mod versions differ, possible oid reuse) + 1 within-launch flip (oid 7f000000 in L3). Only the within-launch flip is B1.

## Open

- **O1 Spawn mechanism.** CreateObject at 0x66f990 reads the VM stack; no standalone spawn found. R3 route (a): run a one-line NCS via RunScript from H1 ticks. Needs VM pointer addresses. See `.claude/research/npc_add/R3_engine_route.md` section 5.
- **O2 Existence guard.** No GetObjectByTag string. Walk the area object list by tag.
- **O3 Persistence.** Flags +0x11AC / +0x11FC in SaveCreatures 0x52a600.
- **O4 Save-load vs door transition** for 204tel at +435 s. Needs a probe 0.2.0 run (lines 1-10 hidden in 0.1.0).
- **O5 22 cross-launch tv flips.** Oid reuse vs real tv change cannot be decided from the logs ([A2] Open).
- **O6 STAT counter mismatch** (remap chg=0 vs 12 changed; tex calls 40 vs 88 TEX lines). Cause unverified.
- **O7 H4 address** (banner truncated). AddObjectToArea mapping unverified.
