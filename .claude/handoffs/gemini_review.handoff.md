# Session Handoff: Gemini 2026-10-14 review (phase G1 done, then G2 plan)

## 0. Next-Phase Direction
G1 (persist, triage, anchor check, Haiku recon) is DONE and committed. Next is **G2: verify, validate and persist the
recon results for development**. Plan path: **`.claude/plans/gemini_review_g2.md`** (approved and saved 2026-10-14; G2 started 2026-10-14T23:28-04:00). Resume with `python3 scripts/plan_manager.py resume .claude/plans/gemini_review_g2.md`
(never bare). No fixes have been decided. No game files were touched.

## 1. Provenance and commits (branch `KOTOR-II`)
- `d871ff0` A: review_bundle packer + manifest + tests, doc 43, MASTER_KEY row 43 (index-blob partial stage; the other session's BG row left unstaged).
- `2c6635b` B: `.claude/reviews/gemini_20261014/` responses (11, cmp-identical to `.claude/bundles/`), 20 manifests, `_index.tsv`, `_review_prompts.md`, README.
- `e4ad3ae` C: triage checkpoint (findings.tsv 135 rows, topics.tsv 100 topics, topics.md, anchors.tsv, triage/ per-batch).
- D: recon/ (100 facts files + units.tsv), verdicts.tsv, tools/, README recon summary, this handoff. SHA in the `git log` after this commit.
- Bundles were built from the WORKTREE at git_head `02b5be44bb54`. Dirty bundles: 2a, 2b, 5a, 5b, 5d, 6. Since then another session has edited `queue_hud_src/QueueHud.cpp` and `QueueHudLogic.h` (uncommitted), so line numbers there may drift.

## 2. Data layout (all tracked, `.claude/reviews/gemini_20261014/`)
- `findings.tsv`: batch, fid, set, bundle, file, function, anchor, gemini_confidence, category, claimed_severity, title. fid = `<HB|QH|CO|SH|PL|K1|HN>-<bundle>-NN`.
- `topics.tsv`: batch, tid, fids, priority, question, entries. Cross-batch merges X-T01..X-T05 (guikit wr, readAt wrap, embtex heal, createLabel leak, WindowHook). QH-T09 was split.
- `anchors.tsv`: fid, found(exact|partial|token|NOT-FOUND), where. 10 NOT-FOUND: HB-1b-01, HB-1b-03, HB-1d-03, HB-1d-04, SH-3a-08, SH-3b-05, SH-3c-07, PL-4b-01, HN-6h-02, HN-6h-03 (mostly paraphrased; recon found the real code for most).
- `verdicts.tsv`: fid, tid, uid, priority, set, verdict, evidence, fix_sound, note, title. **135/135**.
- `recon/<tid>.md`: per-topic facts table + supporting bullets. `recon/units.tsv` maps uNNN to tid.
- `tools/`: merge_triage.py, dedupe.py, add_units.py (wave brief generator incl. verdict rules), aggregate.py, triage_rules.md (Sonnet brief), example_recon_brief.md.
- The wave dir `.claude/waves/gem1/` is gitignored and holds briefs, replies and the manifest (keep it for `wave_tools.py report`).

## 3. Results
Totals: CONFIRMED 35, PARTIAL 86, UNSURE 10, REFUTED 4.
| pri | C | P | U | R |
|---|---|---|---|---|
| P1 | 7 | 11 | 1 | 0 |
| P2 | 16 | 43 | 3 | 3 |
| P3 | 12 | 32 | 6 | 1 |
- P1 topics: X-T01 (wr has no write probe, CONFIRMED), SH-T03 (writeChecked reads before checking, CONFIRMED), HB-T01 (TwoDA nrows*cols wrap, 1 CONFIRMED), X-T02 (readAt wrap: the Opus gate **downgraded it to latent P3**, because the wrapped address is still probed, so the worst case is a wrong read), HN-T06 (engineSpawn unprobed vtbl dtor, PARTIAL), PL-T01 (CRes raw derefs, PARTIAL), CO-T02 (FM3 removed flag gated on level: stale, not crash), HN-T02 (holster/cmdq-probe ECX adjacency, PARTIAL; needs the cmdq wrapper read).
- The P2 CONFIRMED list is in README "P2 CONFIRMED".
- REFUTED: CO-2d-01, HN-6h-01, HN-6n-04, PL-4c-02 (the Opus gate re-checked 2 and both held).
- **Why so many PARTIAL:** units read at most 3 windows of about 80 lines. Most PARTIALs say "code fact true; impact or whole-file absence needs out-of-window read" (the OPEN line in each recon file names exactly what to read). Test-gap claims ("never tested") are nearly all PARTIAL/UNSURE for this reason. **G2 must resolve PARTIAL/UNSURE with wider, targeted reads (rg over whole file / caller), not re-run the same windows.**

## 4. Lessons / gotchas for G2
- RECON contract "write only your OUTPUT file" conflicts with wave_tools' reply.txt. Say in the launch prompt that both files are in scope.
- Haiku recon about $0.013/unit, 50-65k peak context. 7 Sonnet triage agents (~70-100k each). Total G1 cost $3.71.
- The u008 roll-up showed that a unit VERDICT is the worst case across fids. Judge per-fid rows, not the reply line.
- Related existing tracking: doc 37 bug board (`patch_manager_mods/37_bug_board.md`, BG01-BG25, handoff `37_bug_board.handoff.md`, MASTER_KEY prefix **BG**, uncommitted row from another session), dev work items `.claude/work_items/dev_work_items.md` via `scripts/wi_driver.py` and the `/next-dev-work-item` skill.
- Fixes to `_shared_src` need `python3 scripts/compat_check.py <files>`. Mod tests are scoped with `run_tests.py <name>`. Live installs need user GO (kotor2-kpm-install skill).
- Never commit other sessions' dirty files. Use `git commit -- <paths>`.
