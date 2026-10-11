# action-hotbar 1.3.0-dev: bar never visible (recon, read-only)

## Root cause (most likely)
- Gate loop skips any panel with PanelIgnoredByInput bits (mask 0x600) BEFORE the `p == hud` check.
  patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:284 isIgnored reads Panel_Flags (0x48, :73), mask 0x600 (:74).
  ActionHotbar.cpp:300-306 `if (isIgnored(p)) continue;` then `if (p == hud) ... "hud on top"`.
- New log: action-hotbar_log.txt:34 `panel[0]=08a01878 vt=0099dcc4 flags=ff8fb445`. 0xff8fb445 & 0x600 = 0x400, so the HUD itself is treated as ignored.
  Loop never matches hud, falls to ActionHotbar.cpp:307 "hud not in list". Gate false -> updateButtons hides all slots (:1070 "hud not active").
- Old log (.1): hud flags were 0x5 / 0x45 every time (no 0x400 bit), gate ACTIVE "hud on top" (.1 lines 19, 38, 47, 62).
- Gate code is byte-identical 78807d4..HEAD (diff grep for isIgnored/gameplayHudActive/Panel_Flags: 0 hits). The gate did not lose a hook.

## What changed in the hook plumbing
- Diff 78807d4..HEAD header (ActionHotbar.cpp:5-8): Draw and HUD ctor moved to hud-host. onHudFrame (:1865) calls g_se.poll() then HotbarHudTick(ctx->hud). onHudBuild (:1872) calls HotbarBuildButtons(ctx->panel).
- Per-frame tick still runs (heartbeat frame=1800/3600 in new log), so the tick is alive; the gate input (hud flags) is what is wrong.
- Hud pointer changed session to session (08886b08 old, 08a01878 new); vtable 0099dcc4 matches, so it is the real CSWGuiMainInterface.
- New log also shows the HUD missing from the manager list at one point: action-hotbar_log.txt:48-51 `panels=3`, panel[0..2] = 08b224c0/0876a610/084b0828, hud absent -> "other panel on top". Suggests hud-host timing or a HUD rebuild/reorder, unconfirmed.

## Character key 'objfffffffd' (question 4)
- Computed in computeCharacterKey, ActionHotbar.cpp:350-375; OID read by creatureOid (:349, S_Misc +4); key via hb::characterKeyFor at :375.
- Not garbage: comment ActionHotbar.cpp:1235 says objfffffffd is the Exile's stable OID. objffffffff is the invalid one (handled at :380+).
- Real defect: key flips kreia <-> objfffffffd (action-hotbar_log.txt:35, :40) when the tag lookup (party::serverCreature / readCreatureTag, :370-372) fails. Profile changes (0/10 vs 4/10 slots bound). Secondary, not the cause of the hidden bar.

## Suggested fix (smallest first)
1. ActionHotbar.cpp:300 move `if (p == hud) { g_gateWhy = "hud on top"; return true; }` ABOVE the isIgnored continue. Hud's own flags must not hide it.
2. Add hud flags to the gate line (:2087) so the flag is visible without logPanels.
3. Find which code writes bit 0x400 into hud+0x48 (diff onHudBuild/onHudFrame timing vs old Draw hook; check if save-events poll runs during a load that sets flags).
4. Stabilize key: keep last resolved tag-based key when tag read fails in the same creature OID.

## Not verified
- Meaning of bit 0x400 in Panel_Flags is unconfirmed (no source for it in repo checked).
- Build/run not done (read-only per brief).
