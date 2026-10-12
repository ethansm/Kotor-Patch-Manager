# WI-10b Force Power Hotbar EKC hook move and arrow art (split from WI-10)
area: force-power-hotbar
size: S
ready: ready
after: WI-10
items: C051,C053,C056
files: patch_manager_mods/action_hotbar_src/ActionHotbar.cpp; patch_manager_mods/action_hotbar_src/kotor2-steam-aspyr.hooks.toml
out-of-repo: none
blockers: after: WI-10 (both edit ActionHotbar.cpp); C051 has no arrow texture (`arrow` absent from HotbarTextures.inc and scripts/gen_hotbar_textures.py), art source unknown; if art is supplied later, add it in scripts/gen_hotbar_textures.py and regenerate HotbarTextures.inc, never hand-edit it; C056 premise stale: 0x0074B3F0 HotbarHudTick migrated to hud-host (kotor2-steam-aspyr.hooks.toml:22), so the 0x74B3F5 move has no detour to move; 0x72CB84 site not re-verified here (no REA run); WI-11 and WI-12 also edit ActionHotbar.cpp.
docs: patch_manager_mods/07_force_power_hotbar.md (C051, C053, C056)
deferred: C051,C056

## Goal
Apply option (c) to the EKC coexistence detour at 0x0072CB63 (C053). C051 needs arrow art first. C056 needs a re-scope because its 0x74B3F5 target has no detour left.
## Steps
1. C053: option (c) (`07_force_power_hotbar.md:87`) moves the CaptureHotbarKey detour from 0x0072CB63 to 0x0072CB84. Apply only that half; the 0x74B3F5 half has no detour (see blockers). Before editing, check the 0x72CB84 record site against the parameters at `kotor2-steam-aspyr.hooks.toml:14-18`, using `patch_manager_mods/upstream_prs/ekc_hotbar_coexist/DESIGN.md` section 6. If it cannot be confirmed, report OPEN and skip.
2. Change `address` at `kotor2-steam-aspyr.hooks.toml:7` to 0x0072CB84 and update the matching site comment at `ActionHotbar.cpp:6`.
3. Deferred: C051 and C056 (see deferred line). Do not touch them.
## Verify
1. `python3 scripts/build_check.py`
2. C053 needs a live check with EKC and hotbar both installed. This is the user's check; do not claim it.
3. `compat_check.py` not required: `_shared_src` is not touched.
## Docs
1. After build passes, mark C053 done with a note that the EKC live check is user-pending: `python3 scripts/plan_manager.py mark-done <row id> --state <STATE>`. Row-id form and state name unknown.
2. C051 and C056 stay open.
3. CHANGELOG bump: unknown (no path in TSV).

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C051 patch_manager_mods/07_force_power_hotbar.md:82 id=- open
- C053 patch_manager_mods/07_force_power_hotbar.md:87 id=- open
- C056 patch_manager_mods/07_force_power_hotbar.md:508 id=U4 open
</content>
