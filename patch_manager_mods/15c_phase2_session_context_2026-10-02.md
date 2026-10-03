# Session context: doc-15 Phase 2 (L1 + L4 engine fixes) and the K1-bugs community reference (2026-10-02)

Companion to `15b_phase1_session_context_2026-10-02.md` (Phase 0/1), `16_candidate_register.md` (register + test cards) and
`17_dialogue_letterbox_and_wrap_fixes.md` (**build doc: mechanism, hooks, INI, install, test plan, rollback; read first**).
Branch `KOTOR-II`. Commits: `7d7e310` S1 (V7), `0fa8046` S2 (L4), `40e2161` S3 (L1), `f1f1aef` S4/S5 (doc 17, register, DB),
`54e3667` (installed), `1f9b1a1` (`K1_ENGINE_BUGS_IN_K2.md` + upstream note draft). Sibling sessions committed in between (normal).

## 1. What this session owns
- **`k2-dialogue-letterbox-fix` 1.0.0** (L1, KMRP K7). Source mirror `dialogue_letterbox_fix_src/` (cpp, toml, manifest, exports.def,
  `smoke_dialogueletterbox.cpp`, README). Build dir `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/K2DialogueLetterboxFix/`.
  kpatch md5 `6033b700...`, DLL md5 `9bbbfcdd...` (KERNEL32 + msvcrt only).
- **`k2-wrap-measure-fix` 1.0.0** (L4, KMRP K6). Mirror `wrap_measure_fix_src/`; build dir `Patches/K2WrapMeasureFix/`; kpatch md5 `11bfc231...`.
- **`K1_ENGINE_BUGS_IN_K2.md`**: community reference (every KMRP K1-K9 fix + popup/QPC/2DA leads, K2 verdict + deciding address).
- **Upstream KPM note draft**: "KOTOR II status" sections appended (LOCAL, uncommitted clone) to `Patches/K1StrayBugFixes/README.md` and
  `Patches/K1WidescreenPatch/KMRP-ENGINE-FIXES.md`; diff saved as `engine_bug_research/upstream_kpm_k2_status_note.patch`. **Not submitted**: a PR to
  `LaneDibello/Kotor-Patch-Manager` is outward-facing and needs the user's explicit go; it must be built on a clean branch from `origin/master`
  containing only those two files (the clone has many sibling edits).
- Tools: `engine_bug_research/tools/check_engfix_windows.py` (wraps `mods678_research/check_windows.py`, adds +-8 B proximity; output
  `check_engfix_windows_output_2026-10-02.txt`), `engine_bug_research/tools/persist_phase2_db.py` (5 lessons + name 0x411A80; idempotent).
- Analysis: `engine_research_findings/V7.md` (hook point, sole-reader proof, rounding note) + brief V7 in `V_BRIEFS.md`.

## 2. Live state (verified 2026-10-02 ~11:40)
- **Both fixes INSTALLED** 2026-10-02 01:33 at the user's request; still present after the sibling reinstalls at 10:32 (level-up-override 0.3.0-a) and
  11:36 (companion-ai 0.2.0-probe). Live = 28 ids / 186 hooks. Pre-install backup `~/Games/KOTOR2_Mod_Workspace/backups/pre_engfix_1002_0133/`.
  Stage dir `~/Games/KOTOR2_Mod_Workspace/stage_engfix_1002_0121` is now STALE for other ids (siblings changed level-up-override/companion-ai): never
  reinstall from it; re-stage with `mods678_research/stage_and_verify_install.py stage <NEW> <kpatches>`.
- No `dialogue_letterbox_fix.ini` in the game dir (defaults enabled=1 mode=h6 log=1).
- `dialogue_letterbox_fix_log.txt` (game dir, 12 lines, 02:38): 4 launches, each `APPLY src=SetScreenSize W=3840 H=1600 ... vanillaBar=-22 newBar=267
  X=1066 fs=2.0 panelH=200 replyY=1333` + `wrote ... verify OK, now de 1a 51 cc cc cc 0c 40 = 3.5999999964`. **0 SETTOP/SETBOTTOM lines = no conversation
  started yet** -> T1 not done. T2 not done. L4 has no log and no switch.

## 3. Facts a fresh context needs (verified this session)
- L1 hook: `0x411AC1` 7 B `8B 55 F4 83 7A 7C 00` in `CSWGuiManager::SetScreenSize` 0x411A80 (after W/H stores 0x411AAA/0x411AB5 and CALL 0x413DD0;
  manager `[ebp-0xC]`; reached only when W/H change; 4 callers: startup 0x78A20F, options Apply 0x78DDA7, credits force 0x89222B / restore 0x892756).
  Log hooks `0x8BAD7C` (SetTop) / `0x8BAF01` (SetBottom), 10 B `C7 81 88 00 00 00 01|02 00 00 00`, ECX = letterbox.
  Whole-exe scan: exactly 7 refs to 0x9A7D30, all `DC 35` FDIV, both exes.
- L4: `0x47B232` `DC 05 D8 C1 98 00` -> `DC 05 08 7F 98 00` (KPM `simple`; no min length / boundary rule for simple hooks, only Replace needs >= 5 B).
- **KPM detour wrapper runs the handler BEFORE the stolen bytes** (`src/KotorPatcher/src/core/wrapper_x86.cpp` "EXECUTE STOLEN ORIGINAL BYTES" after restore).
- Bars (engine formula): 3840x1600 **267** (not 266), 1920x1080 180, 3440x1440 240, 5120x1440 240, 800x600 100, 640x480 80. `c = 1.5*W/H*(1-1e-9)`.
- Smoke harness pattern: 32-bit test exe at base 0x400000 with a 7 MB `.bss` pad makes game VAs ordinary memory (ActivationFix + L1). Proton Experimental wine,
  throwaway prefix `/tmp/claude-1000/l1_smoke/prefix`.
- Build: `cd Patches/<Name> && python3 ../create-patch.py` worked WITHOUT the KPATCH_ADDRESS_DATABASES workaround (the empty kotor1_steam.db was moved).
- No KPM validate/dry-run CLI exists (validation is inside KPatchLauncher only).

## 4. Process notes / gotchas
- Sonnet agents: V7 analyst (~11 min), S2 coder (~30 s), S3 coder + smoke (~3.5 min), S5 drafter (~1.5 min). Opus review caught: "truncates the sum"
  (it is per glyph), wrong step labels, nothing structural. Brief with absolute paths and "KPM = <path>" (one agent took `KPM/` literally).
- Sibling sessions install at any time; always re-read `kpm_install_state.json` `UpdatedAt` and the live id list before any install; never reuse an old stage dir.
- Never stage `research/companion_levelup/logs/*` or `ranged_ai_src/script/kite_routine.nss` (sibling-owned).

## 5. Next phase
Plan file: `~/.claude/plans/` (written right after this file; repo copy `engine_bug_research/PHASE3_PLAN.md`). Summary: user runs T1/T2 (+T3/T4/T5) ->
log/screenshot analysis -> L1/L4 acceptance (log trim, version bump) -> static triage of the not-yet-examined KMRP topics (K3 + 7 docs) -> optional
upstream PR (user-gated).
