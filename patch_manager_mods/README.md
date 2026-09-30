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
  family). **Status 2026-09-29: idea + verified `forceshields.2da` data, no Ghidra yet.**

- [`09_load_hang_investigation.md`](09_load_hang_investigation.md) — the intermittent
  load-screen hang (`NtGetContextThread` spin, ~7 incidents, unattributed): incident
  table, log evidence, ranked hypotheses (incl. `taskset -c 0,6` = one physical core),
  capture procedure and an A/B experiment plan.

- [`10_equipment_3d_character.md`](10_equipment_3d_character.md) — live 3D character
  on the equipment screen (armour/helmet/gloves follow the chosen gear, party switch, idle + flourishes,
  drag/wheel, toolbar with tooltips, weapons prototype). **Status 2026-09-30 (session 2): 0.1.0-probe live at
  stage 5 (layout A), user-tested; weapons render in hands behind the Weapons toggle; open: dual-wield stance
  rule (decoded, R7f), animations with weapons, conditional Reset, holstered toggle, character-page mirror, final review.**

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
