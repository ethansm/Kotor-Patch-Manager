# Session Handoff: Gemini 2026-10-14 review (G1 + G2 DONE)

## 0. Next-Phase Direction
**FX3 (2026-10-15, IN PROGRESS):** plan `.claude/plans/gemini_fix_wave3.md` (22 fix units + WI-89c inline; wave dir `.claude/waves/fix3/`,
brief generator `tools/fix3_briefs.py`). Resume: `python3 scripts/plan_manager.py resume --plan .claude/plans/gemini_fix_wave3.md`.
**FX2 (2026-10-15, DONE, $9.08):** 11 mod-side bugs landed: WI-79 9dbfed7, WI-77 b4c80a5, WI-78 49a57a7, WI-74 a2ecc13, WI-82 ab456aa,
WI-75 066320e, WI-83 b18c0f3, WI-84 685fede, WI-80 810590a, WI-81 6d24306, BG29c ce3cec1; doc 37 BG27-BG37 DONE + rea notes 2ae4cb2.
Not installed: rebuild + install (user GO) queue_hud, companion_selfheal, companion_ai, holster, combat_hotbar; play checks WI-74 (companion
dispatch behind resting 0x3D) and WI-80 (empty tip hides). rea-recon PARTIAL/UNSURE: WI-87 stays parked (dtor 0x7510b0 not in DB; allocator unknown).
**re2 + FX3 triage (2026-10-15, after FX2):** 9 Haiku rea-recon runs answered WI-87 (unpark: label vtable slot0(1) is safe, same heap) and SH-3a-08
(likely real: top-level "Render Window" recreated via 0x409750 on the activate path); `.claude/reviews/gemini_20261014/fx3/re2/SUMMARY.md`.
7 Haiku triage units re-checked the FX3 queue at HEAD 2dc7684: `.claude/reviews/gemini_20261014/fx3/triage.tsv` (23 rows: 18 FIX-NOW, 3 OWNER-DECISION
WI-100a/b/d, 2 NEEDS-RUNTIME WI-94b/c; splits 89a-c, 91a-c, 94a-c, 95a-b, 100a-d). The REA exe uses GameAddr.h addresses, not K2Addr.h `t.*`.
**Next = FX3:** WI-73, 89, 90, 91, 92, 93, 94 (split), 95, 96, 100 + follow-ups: selfheal_log_check TEXSET_RE lacks `tog=`; holster_log_check COMB
regex should accept `old=? edge=?`; selfheal g_texInit never re-queries null disc/cross handles. Plan `.claude/plans/gemini_fix_wave2.md` (wave dir `.claude/waves/fix2/`,
triage `.claude/waves/fx2_pick/`). Resume: `python3 scripts/plan_manager.py resume --plan .claude/plans/gemini_fix_wave2.md`.

**FX1 (2026-10-15, DONE):** 7 landed (WI-99 f6ecaf6, WI-88 09bfbf1, WI-86 424a79a, WI-85 4a9f047, WI-76 c0910fe PARTIAL/BG29 owner callers
pending, WI-98 e4f19b4, WI-97 0af3d09); WI-87 parked 215410c (needs RE of engine control release); doc 37 d94955f. Haiku variant wave + Sonnet consolidators + Sonnet reviewers, $11.70; plan
`.claude/plans/gemini_fix_wave1.md` (wave dir `.claude/waves/fix1/`, brief generator `tools/fix1_briefs.py`). Resume: `python3 scripts/plan_manager.py resume --plan .claude/plans/gemini_fix_wave1.md`.

G1 and **G2 are DONE** (plan `.claude/plans/gemini_review_g2.md`, all 6 phases marked; commits f427b92, dedb65e, 93d371a, 30ba0b6, b213db6,
6e8af73 doc 37, WI-73..WI-101 commits by wi_driver, af03494, f7f8974). Single source of truth: `.claude/reviews/gemini_20261014/validated.tsv`
(real 61, latent 49, refuted 21, unsure 4; 0 high). Persisted: doc 37 BG26-BG41, work items WI-73..WI-101 (origin gemini-20261014),
`refuted.md`, `scorecard.md`, README "G2 result". No fixes were made in G2 and no game files were touched.
**Next:** fixing runs through `/next-dev-work-item`, one item per session. Ready now: WI-76 (BG29), WI-85..WI-88 (BG38-BG41), WI-97..WI-99
(shared test gaps). Hold items wait for their owner sessions (queue-hud, selfheal, hotbar, k1, holster, npc, perf-lab, companion-ai).
WI-100 (latent hardening) needs an owner decision. WI-101 is runtime-only verification.
Open G2 notes: rea-recon answered 1 of 7 engine questions (g2/re.md); the 4 `unsure` fids sit in WI-101. G2 cost $9.16 (budget was $4-5).

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
