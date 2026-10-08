# Level-up override Phase 7: Common + library-DLL integration (MASTER PLAN, chunked)

One chunk per plan-mode session: enter plan mode with "Phase 7 chunk <id>", read only that chunk's section + the files it names,
plan it in detail, execute with Sonnet agents, commit, then tick its row below. Never load the whole Phase 6b history.

## Status

| Chunk | Title | Status | Commit |
|---|---|---|---|
| P0 | Close Phase 6b (review fixes, 1.0.0 rebuild, doc renumber, this file) | DONE; 1.0.0 (31c3a83f) INSTALLED LIVE 22:54 | 6408800, 456a3b0, KPM eebfd18 |
| C1 | Integration audit baseline (read-only) | DONE | 83cb016 |
| C2 | Common contract test | DONE | d89b511 |
| C3 | Include style + parity stub alignment | DONE | d8438c6 |
| C4 | Engine addresses -> GameAddr.h (+ proposal) | TODO | |
| C5 | Party.h adoption | TODO | |
| C6 | FileWatch.h adoption | TODO | |
| C7 | KpLog migration (C7a design / C7b code) | TODO | |
| C8 | Library DLL interfaces: consumer requirements (design only) | TODO | |
| C9 | Offers to Common owners (notes only) | TODO | |

## 0. Context

- User direction (verbatim): "next plan should focus on reviewing and ensuring were properly integrated with patches common and the
  new library dlls being developed.  don't do full implementation of everything, keep a master plan which we can handle chunks of at a time in plan
  mode to keep main opus thread context low"
- Branch `KOTOR-II` at `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`. KPM tree `$T` = `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`
  (fork, branch `ethansm/local-mods`). Game `$G` = `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II`.
- Our docs: 08 (`08_level_up_override.md`), 26 (`26_FORCED_STATE_TESTING.md`, was 21; 24 = other session's 24_master_plan_status.md), 25 (`25_pc_levelup_filter.md`, was 22).
- Files other sessions own (never commit): tests/suites.toml, tests/static/*, scripts/run_tests.py, `_shared_src/*`, doc 19b, doc 20 (except our link line),
  steam_db_pr3/*, the knowledge DB, other mods' src, debug_pykotor logs.

### Anti-patterns
- Editing `$T/Patches/Common/*.h` directly: Common is synced ONE WAY from `patch_manager_mods/_shared_src/` by `scripts/check_shared_headers.py --sync`;
  `_shared_src` is owned by other sessions - level-up only PROPOSES additive changes (doc 19b section D rules: additive, defaulted params, one mod at a time).
- Changing any log format string (`scripts/luo_log_check.py`, the M4 fixtures and parsers depend on them; KpLog migration keeps text byte-identical).
- `if (!logOpen()) return;` guarding behaviour (A2 F1) - logging never gates the rewrite (KpLog caps at 60000 lines).
- Two agents building `tools/parity/*_bin` in-tree at once - one C++/parity agent at a time.
- pykotor `GlobalVars` (LSB-first) for saves; fixed scratch paths in tests; `HOLD tag=x` regex without `^` (matches UNHOLD).
- Committing the knowledge DB while other sessions' rows are in it (33 R10 rows of ours are uncommitted there).

## 1. Facts (exploration 2026-10-03)
- Common headers (source of record `patch_manager_mods/_shared_src/`): SafeRead, GuiKit, GameAddr, K2Addr (Steam/GOG table), Party, FileWatch,
  SceneView, Gui2D, Effects, EffectIcons, PortraitSlots; KPM-only: KpLog.h, KpDebugHook.h, kpatch_api.h. Not built yet: IniFile.h, CodePatch.h, TestHarness.h.
- Library DLLs: registry + kpatch.log core LIVE (36f10b7f); `kind="library"` patches with `provides`/`requires_interfaces` (KPM 94b6b3a/a28217d,
  `$T/docs/PatchRegistry.md:273-312`); `portrait-slots` (`k2.portraitslots@1`) LIVE. Designed only: `k2.saveevents@1`, `k2.hotkeys@1`, `k2.hud@1`
  (doc `22_hotbar_shared_services.md:78-127`), `k2.guiinput`, `k2.charscreen` (doc 19b:16-17). KpLog adoption table: level-up "pending" (doc 19e:299).
- Level-up today: includes only `../_shared/SafeRead.h` + `GuiKit.h` (LevelUpOverride.cpp:71-72; forwarders in `$T/Patches/_shared/`). Local copies:
  logging (cpp:129-169, rotate :1126), engine addresses `struct Engine` (:1019-1031; in GameAddr.h already: AppGlobal, PartyGetAt 0x7E5DA0,
  MemberServerCreature 0x77D800, ClientPartyTable 0x73FB90, ExoCtor/Dtor, PtrListAdd 0x83EA60; missing: rules 0xA1B4D0, 0x73F450,
  0x740FC0, 0x6B9790, 0x72FB00, GUID fns 0x654xxx, 0x51C450, 0x7BE090, 0x6F5B40/0x6F5B00), leaderCreature chain (:1926-1934; = Party.h chain;
  GameAddr GetControlledCreature = 0x7E5D80 vs our 0x73F450 GetPlayerCreature - verify semantics), codeMatches + 19 kXxxCode (:1032-1070, no Common
  equivalent), fileStamp/reloadDue (:183-190 vs FileWatch Stamp/ReloadPoll), ptWriteText .tmp+MoveFileExA WRITE_THROUGH (:1396 vs filewatch::atomicWrite),
  save GUID binding (:1273-1485, future `k2.saveevents@1` consumer), fileCrc32 (:1162). No `common_contract_test.cpp`/`test.toml` (doc 20:127,134).
  Line numbers drift - re-grep before use.
- Integration rules: doc 19b:56-63, doc 20:127,134,204, doc 23:109-116 (behaviour-neutral recipe: checkers before/after, byte-identical log text,
  copy KPM source back to `_src`, install only with user OK), doc `22_compat_check_plan.md:250,286` (contract test; compat_check.py after `_shared_src` edits).

## 2. Chunks
Each chunk: Opus plans + reviews + commits; Sonnet agents do the edits; Haiku only for wide read-only scans. Every chunk ends: gates green ->
commit (explicit paths) -> status row + doc 08 note -> memory line.

### P0 - Close Phase 6b
- Verify + commit review fixes (section 3 gates, mutate 100 %, mingw syntax, check_paths rc). Rebuild 1.0.0, md5s in doc 08, ask install consent
  (DLL swap + v3 table together, game closed, backups, re-md5 live dll).
- Renumber docs 21->24, 22->25; fix links. Create this file.

### C1 - Integration audit baseline (read-only, Sonnet; small)
- Run and record: `python3 scripts/check_shared_headers.py`, `check_hook_bytes.py --quiet`, `check_hook_spans.py --quiet`, `check_exports.py`,
  `check_gui_ids.py` (.venv), `check_raw_addresses` (if present), `python3 scripts/run_tests.py`, `compat_check.py` (if built).
- Output `research/companion_levelup/phase7_evidence/C1_integration_audit.md`: per integration rule PASS/FAIL/N-A with evidence, plus a gap table
  (local copy -> Common target -> effort -> risk -> owner to ask). Decides the order of C2-C8.
- Verification: report exists; nothing outside phase7_evidence changed.

### C2 - Common contract test (small)
- `patch_manager_mods/level_up_override_src/common_contract_test.cpp` + `test.toml` (pattern: an existing `<mod>_src/common_contract_test.cpp`, doc 23:114):
  asserts what level-up relies on from SafeRead/GuiKit (signatures, out-params, constants); messages start `level-up-override relies on:`.
- If a registry is needed: `tests/include/FakeKPatchApi.h` (T1 by the patches-common session; registry.cpp semantics, `fakekp::withLog()`).
- Doc 24 (`24_master_plan_status.md`, 9f1e9ac) marks T5d, S1c and level-up C2 as owned by this session - tick them there when done.
- Verification: `run_tests.py` picks it up and passes; hand-break one assumption -> red.

### C3 - Include style + parity stub alignment (behaviour-neutral; small)
- `"../_shared/X.h"` -> `"X.h"`, build flags `-I Common`; parity stub include path (`tools/parity/stub/_shared` symlinks, `parity.py`, `scenarios.py`,
  `mutate.py`) resolves the same headers.
- Verification: mingw syntax in a `$TMPDIR` copy of `$T/Patches`; all parity gates.

### C4 - Engine addresses -> GameAddr.h (behaviour-neutral; medium)
- Replace `struct Engine` constants that exist in GameAddr.h with its names; write a PROPOSAL (additive entries for missing addresses with DB evidence)
  in doc 19b notes - no `_shared_src` edit without owner OK. Resolve 0x73F450 vs 0x7E5D80 semantics with the knowledge DB.
- Verification: gates + `check_raw_addresses` baseline unchanged or reviewed.

### C5 - Party.h adoption (medium)
- `leaderCreature` -> `party::` helpers (same chain); tests switch from `fakePartyGet` to Party.h test hooks (`g_member`).
- Verification: gate_test/scenarios green incl. -m32; mutate 100 %.

### C6 - FileWatch.h adoption (medium)
- `fileStamp/reloadDue` -> `filewatch::Stamp/ReloadPoll`; `ptWriteText` -> `filewatch::atomicWrite` only if WRITE_THROUGH is kept (else propose an
  additive flag to the owner).
- Verification: playthrough_test + scenarios (X4/X5 lossy, ptsnap/ptdiff) green; mutate 100 %.

### C7 - KpLog migration (largest; C7a design / C7b code)
- Follow `patch_manager_mods/19f_kplog_migration_guide.md`. Format strings byte-identical; keep the file name or teach `luo_log_check.py` the KpLog
  path; logging never gates behaviour (F1); decide rotation vs 60000-line cap; banner crc32 stays. Doc 19e adoption row via owning session.
- Verification: `scripts/test_luo_log_check.py` + every M4 fixture unchanged; gates; one live smoke after install consent.

### C8 - Library DLL interfaces: consumer requirements (design only)
- Level-up's requirements for `k2.saveevents@1` (pre-save / post-load / new-server events + per-mod globals namespace; hooks 0x656640, 0x65675B,
  0x401C52, LUO_G globals; removal-safety S5/S7) and `k2.charscreen` (ctor 0x84D82C, tick 0x84F221, GUI ids 151..169, shared with Mod 14) into doc
  `22_hotbar_shared_services.md` / 19b notes sections. No code.

### C9 - Offers to Common owners (notes only)
- CodePatch.h candidate (codeMatches + kCode arrays), IniFile.h candidate (Playthrough.h doc model), CRC helper. One note each in doc 19b section D.

## 3. Verification gates (every code chunk)
```
cd research/companion_levelup && python3 tools/parity/parity.py && python3 tools/parity/scenarios.py && python3 tools/parity/scenarios.py --m32 \
 && python3 tools/parity/options_parity.py && python3 tools/parity/wizard_parity.py && python3 tools/parity/guikit_test.py && python3 simulate_rewrite.py \
 && ../../.venv/bin/python3 check_paths.py paths/*.toml paths/presets/*.toml && python3 tools/parity/mutate.py --jobs 6
cd ../.. && python3 scripts/test_luo_log_check.py && python3 scripts/test_luo_scenario.py && python3 scripts/test_kotor_save_forge.py \
 && python3 scripts/check_shared_headers.py && python3 scripts/run_tests.py
# mingw: copy $T/Patches to $TMPDIR, swap in our sources, i686-w64-mingw32-g++-posix -std=c++17 -Wall -Wextra -fsyntax-only -I ../Common (0 warnings)
```
Live changes only with user consent, game closed, backups, never launch the game.

Note 2026-10-04: R10 PC level-up filter batch 1 built inside this mod (doc 25 section 12; commits eb079ba f68f2b1 f782862, KPM 06c67a4), not installed.
