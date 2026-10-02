# Kotor Patch Manager — Mod Ideas & Research

Tracking doc for a new KOTOR II modding avenue: **runtime DLL-injection /
engine hooking** via [LaneDibello/Kotor-Patch-Manager](https://github.com/LaneDibello/Kotor-Patch-Manager),
as opposed to our existing TSLPatcher/GFF/2DA/texture pipeline
(`~/Games/KOTOR2_Mod_Workspace/`, `neocities_mods.json`).

This is a fundamentally different mod class from everything in the current
154-mod build: instead of shipping data files that TSLPatcher merges, or
loose-file overrides, Patch Manager lets us **detour game engine functions at
runtime** (C++ code executing alongside `swkotor2.exe`'s own render/update
loop). That's the only way to do either of the two ideas below — neither is
expressible in NWScript, 2DA edits, or GFF/GUI structs alone.

## Contents

- [`01_inventory_3d_viewport.md`](01_inventory_3d_viewport.md) — render a
  live 3D model of the selected inventory item in unused ultrawide space.
  **Status 2026-09-30: v2 (0.2.0-dev) COMPLETE and user-accepted: per-class orientation + fit, drag/roll/zoom/wheel, custom `inv3d_light` rig, reworked inventory layout with character-page arc covers. Live in the current install (any reinstall must include `inventory-3d-viewport`). Verbose logging still on. Next: equipment-page live 3D character (research plan).**
- [`02_buff_duration_hud.md`](02_buff_duration_hud.md) — visual
  countdown/cooldown-wipe overlay for active Force power / buff durations.
- [`03_framework_research.md`](03_framework_research.md) — findings on the
  Patch Manager framework itself: maturity, toolchain, Linux/Proton
  compatibility, and existing mods built on it (including whether **PartySwap**,
  already installed as mod #97, actually uses this framework).
- [`04_recommended_installs.md`](04_recommended_installs.md) — mods built on
  Patch Manager that are safe to install alongside the current build.
- [`05_build_plan.md`](05_build_plan.md) — concrete phased plan for building
  our own two mods, gated on the open questions in `03_framework_research.md`.
  **Phase 0 (toolchain proof) done 2026-09-28** — see its Outcome section.
- [`06_gog_to_steam_conversions.md`](06_gog_to_steam_conversions.md) — a
  lower-risk starting point than our two custom mods: porting existing,
  already-working GOG-only patches to add Steam Aspyr support, since the
  behavior is already proven and only the hook addresses need finding.
- [`07_force_power_hotbar.md`](07_force_power_hotbar.md) — architectural
  spec + full build log: **working proof of concept** (0.2.0-dev) of ten
  on-screen HUD buttons keybound 1-0 with hover tooltip, click/key invoke,
  auto-fill; next phase = in-game configuration GUI. STATUS block at the top
  of that file is the entry point.
- [`08_shield_bar.md`](08_shield_bar.md) — shield-absorption bar beside each
  party portrait's vitality/Force bars (stacked segments, colored by shield
  family). **Status 2026-09-30: WORKING LIVE (bars on all 3 cards, hover tooltip line, no black screen; DLL `81fb7d08`, version string still `1.1.0-probe4`). Read the "Session 4 summary + root cause" section at the bottom of the doc first; next phase = colour/UX polish, then 1.1.0 finalisation.**

- [`08_level_up_override.md`](08_level_up_override.md) - per-character fixed level-up path: Phase 1 Ghidra investigation of the level-up code (addresses, offsets, Q1-Q8). Plan/status: `research/companion_levelup/MASTER_PLAN.md`.
- [`09_load_hang_investigation.md`](09_load_hang_investigation.md) — the intermittent
  load-screen hang (`NtGetContextThread` spin, ~7 incidents, unattributed): incident
  table, log evidence, ranked hypotheses (incl. `taskset -c 0,6` = one physical core),
  capture procedure and an A/B experiment plan.

- [`10_equipment_3d_character.md`](10_equipment_3d_character.md) — live 3D character
  on the equipment screen (armour/helmet/gloves follow the chosen gear, party switch, idle + flourishes,
  drag/wheel, toolbar with tooltips, weapons prototype). **Status 2026-09-30 (session 2): 0.1.0-probe live at
  stage 5 (layout A), user-tested; weapons render in hands behind the Weapons toggle; open: dual-wield stance
  rule (decoded, R7f), animations with weapons, conditional Reset, holstered toggle, character-page mirror, final review.**

**Idea backlog (2026-09-30, research COMPLETE, definitive build plans written, no code; addresses come from the full-program
dump in `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71` and are validated by `mods678_research/check_windows.py`):**

- [`11_inventory_sort_filter.md`](11_inventory_sort_filter.md) - Mod 6: inventory sort + hidden filters. Ships as its own patch id `inventory-sort-filter`. **Stage 0 (logging) built + installed 2026-09-30, awaiting in-game test**; source `inventory_sort_filter_src/`.
  Re-clicking the active **All** tab cycles All -> New -> Usable-by-me -> Open upgrade slot. Sort = buffer backpack rows at
  `0x8A7E22`, flush sorted at `0x8A7E76`; unusable rows tinted with `SetCanUse(row,2)`. 3 hooks, no data files.
- [`12_empty_container_indicators.md`](12_empty_container_indicators.md) - Mod 7: **CANCELLED 2026-09-30 (vanilla already shows Empty/Nothing).** KOTOR II is a two-step interface, so the vanilla
  "Empty" label is selection-only (answered statically) and the highlight-everything call has **no live trigger** in this exe.
  Plan: `SetMouseCursor` hook (plain pointer over empty containers), grey tint in `HiliteAreaObjects` case 9, and our own
  hotkey that calls the engine's `on=1` path. 3 hooks, INI only.
- [`13_ranged_ai_spacing.md`](13_ranged_ai_spacing.md) (+ [`13a`](13a_ai_community_research.md) community survey) - Mod 8: **Stage 0 logging DLL built 2026-09-30 (`ranged_ai_src/`), not installed.** Resume from `mods678_research/HANDOFF_2026-09-30.md`.
  `creature+0x11AC` = party flag, `+0x10E8` = IsPC. Engine DLL holds blocked ranged party members instead of walking in
  (`0x6D83EA`) and extends the vanilla LOS retarget (`0x6D7C73`); new stance 16 needs 3 engine hooks + `aiscripts.2da` row +
  2 TLK strings; script `k_inc_generic` adds `GN_RunKiteAIRoutine` with the fixed `HasLineOfSight`. Party first, enemies later.
- [`15_engine_bug_candidates_research_plan.md`](15_engine_bug_candidates_research_plan.md) (+ [`15a`](15a_research_session_context_2026-10-01.md) session context) - **Research plan (2026-10-01, no code):** 14 leads (KMRP K1 engine bugs incl. dialogue letterbox past 21:9 and listbox row inflation, QPC pacing, Aspyr Linux symbol check, 2DA token overflow, Gemini round-2 items) -> evidence standard -> ranked candidate list. Gemini round 2 was triaged: CPU pinning, save-leak and 2DA-buffer ideas rejected or premise-corrected; none of the K1 bugs had been investigated here. **Phase 0+1 DONE (2026-10-01, static):** results and hand-off in [`16_candidate_register.md`](16_candidate_register.md) (analyses `engine_research_findings/V1-V6.md`, bridge + 2DA scanner in `engine_bug_research/`): dialogue letterbox bar goes negative past 21:9 (confirmed, 8-byte data fix) and glyph-advance 0.25->0.5 (confirmed, cosmetic); listbox/wrap/minimap/popup/QPC/2DA-token leads killed; awaiting user test cards. Fresh-context entry: [`15b_phase1_session_context_2026-10-02.md`](15b_phase1_session_context_2026-10-02.md), K1 reference notes in `engine_bug_research/kmrp_reference_notes.md`. **Phase 2 DONE (2026-10-02):** L1 `k2-dialogue-letterbox-fix` 1.0.0 and L4 `k2-wrap-measure-fix` 1.0.0 built and staged (not installed, awaiting user install + T1/T2); see [`17_dialogue_letterbox_and_wrap_fixes.md`](17_dialogue_letterbox_and_wrap_fixes.md).
- [`17_dialogue_letterbox_and_wrap_fixes.md`](17_dialogue_letterbox_and_wrap_fixes.md) - Engine fixes L1 + L4 (doc 15 Phase 2): `k2-dialogue-letterbox-fix` 1.0.0 (DLL rewrites the 0x9A7D30 double to 1.5*W/H so the dialogue bar = H/6; 3840x1600 -> 267 px instead of -22) and `k2-wrap-measure-fix` 1.0.0 (one byte patch at 0x47B232, glyph advance +0.25 -> +0.5). **Built + staged 2026-10-02, not installed**; stage dir `stage_engfix_1002_0121`, install steps + T1/T2 test plan + rollback in the doc; sources `dialogue_letterbox_fix_src/`, `wrap_measure_fix_src/`.
- [`18_mouse_camera_investigation.md`](18_mouse_camera_investigation.md) - **Investigation only (2026-10-02, no code):** mouse camera / right-drag / double-click / jumpy-camera regression; timeline (predates most patches; first reported 09-26), 6,941 Wine `MOUSE_VIRTUAL_DESKTOP` dinput fixmes, ranked hypotheses, A/B matrix, instrumentation plan.
- [`K1_ENGINE_BUGS_IN_K2.md`](K1_ENGINE_BUGS_IN_K2.md) - **Community reference:** every KMRP/KPM K1 engine fix (K1-K9) plus the popup-cap, QPC and 2DA leads, with the K2 verdict (present / absent / latent / not checked) and the deciding K2 Steam address. Read this before investigating a K1 bug in K2. The same K2 status table is drafted for upstream KPM's `K1StrayBugFixes/README.md` and `K1WidescreenPatch/KMRP-ENGINE-FIXES.md` (`engine_bug_research/upstream_kpm_k2_status_note.patch`, not submitted).
- [`mods678_research/`](mods678_research/README.md) - raw evidence (decompile+asm excerpts of every cited function, container
  survey, live hook table, window checker, DB persist script) and the methods learned. Read its README before the next round.

## Why this matters for our specific install

Two things about this build make Patch Manager both *more* attractive and
*more* risky than for a vanilla install:

1. **Ultrawide (3840x1600) GUI work already done.** See
   [[kotor2-ultrawide-gui]]. The inventory 3D viewport idea directly targets
   the same "unused ultrawide real estate" we've spent days reclaiming for
   16:9 content layers — this would be a natural continuation, but any new
   `CONTROL_3DVIEW` injected into `inventory.gui` must respect the X/Y
   bounding boxes and `HUD_DESC_COEFF` conventions already established in
   `popos/docs/kotor2_ultrawide.md`, or it'll fight our own layout math.
2. **Linux + Proton, not native Windows.** Patch Manager is a Windows DLL
   injection framework built against a Windows PE target. Everything we've
   done so far (pykotor, TSLPatcher via Wine, nwnnsscomp via Proton Wine) has
   had to be re-verified for Wine behavior — see [[kotor2-script-compiler]]
   for a case where a "should just work" toolchain silently produced broken
   output under this exact environment. Assume the same risk here until
   proven otherwise; this is the single biggest open question in
   `03_framework_research.md`.
3. **Existing exe-level patch already applied.** Mod #4 (4GB/LAA Patcher) and
   mod #1 (3C-FD Patcher, which also applies an LAA patch and explicitly
   **refuses to run on an already-patched executable** — see its install
   note) already modify `swkotor2.exe`'s PE headers. `KPatchLauncher.exe`
   injects at runtime rather than patching the binary on disk, so it should
   be independent of LAA — but this needs confirming, not assuming, per
   [[feedback-calibrate-detectors]] ("verify claims on disk").

**Status: research phase only.** No `.kpatch` has been written or installed.
Nothing in this directory has touched the live game.

## Tooling notes

- **`pykotor` is available in this worktree's `.venv`** (`source
  .venv/bin/activate`) and is the right tool for parsing this build's 2DA
  files, which are the binary `V2.b` format, not plain text — a raw
  `head`/`grep` on e.g. `spells.2da` shows garbled binary, not rows. Use
  `from pykotor.resource.formats.twoda import read_2da` +
  `t.get_row(i).get_string(col)`. Always read the **live install's own**
  `override/*.2da` (`~/.local/share/Steam/steamapps/common/Knights of the
  Old Republic II/override/`), not a vanilla/TSLRCM reference copy —
  confirm via MD5 that it actually differs from both, since this build has
  ~150 mods that add/change rows.
- **No `sqlite3` CLI on this machine** — see
  `ghidra_knowledge/README.md`'s Querying section for the `python3`-based
  workaround, which applies to any SQLite file in this project, not just
  the Ghidra knowledge DB.
- Ghidra-specific methodology (RTTI lookup, byte-search API gotchas, vtable
  diffing technique, etc.) lives in the `ghidra_knowledge` DB's `lessons`
  table and its `README.md` — read those before any headless Ghidra work,
  not just this file.

## Engine optimization research

- [`10_engine_optimization_research.md`](10_engine_optimization_research.md) — performance / memory / error-handling research program (living record, status and next steps at the top).
- 2026-09-30: Mod idea 14 (character-screen active effects panel): research started, see 14_character_active_effects_panel.md
- 2026-09-30: Mod 5 session 3 update in 10_equipment_3d_character.md (SESSION 3 UPDATE block)
- 2026-09-30 (evening): Mod 5 session 4 update in 10_equipment_3d_character.md (SESSION 4 UPDATE block): Reset/Spin row, Emotes + Attacks pop-up menu, hide/buffer timing; DLL md5 dcbdfde3 installed, awaiting user test.
- 2026-10-01: Mod 14 Phase 3+4: research R2b-R2e done (effect-icon cache = effecticon.2da, runtime text colour 0x417140); DLL `character-effects-panel` 0.1.0-probe built (scripts/check_chareffects_kpatch.py ALL PASS) and INSTALLED 18:44 at stage 0 (23 live ids), see 14_character_active_effects_panel.md
