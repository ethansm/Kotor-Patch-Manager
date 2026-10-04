# HUD action bar does not refresh after a "Jedi mode" change - static findings (2026-10-04)

Scope: static only (local dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp`, `field_census.py`, knowledge DB).
No game access, no Steam dir reads. Names below are MY labels; the DB names for several of these functions are wrong or weak
(0x74CFE0 is DB-named "UpdateTargetMenu", 0x77E650 DB-named "AppendToScriptsRun" in the GOG DB) - judged by body only.

## 0. Answer in one paragraph
"Jedi mode" = AI stance 13 (0xD) = HUD group 4 entry key 0x414, icon `ib_jedisupport` for Jedi classes (`ib_grenadier` otherwise).
Stance lives in the server creature stats (`creature+0x1198` -> `+0x146`, u16). The HUD does NOT read that field to draw the bar:
it draws the entry whose key equals a HUD-side selection cell `hud+0x1C8C` (group 4). That cell is rewritten from the creature
only by `0x74DA40` (my label SyncStanceSelection), which runs on controlled-character change, on `HideSWInGameGui`, and after the
vanilla arrow-cycle. Nothing polls the stance field, and the per-frame bar rebuild does not touch the selection cells.
The Force Power Hotbar `invokeEntry` makes this worse: it sets the cell, calls 0x751750, then RESTORES the old cell, so a hotbar
stance change leaves the vanilla bar on the old icon. Vanilla mouse use (arrow cycle then click) keeps cell == state, so vanilla
by itself only shows the lag for non-HUD writers (script `SetNPCAIStyle`).

## 1. Who sets the mode (Q1)
VERIFIED (asm + field_census over all functions, offset 0x146 writers, `--writers`):
| Writer | Addr | What |
|---|---|---|
| HUD stance click (`StanceActionClicked`) | 0x77D890 (RET 8) | switch key 0x410/412/413/414 -> `*(u16*)(stats+0x146) = 9/0xB/0xC/0xD` (0x77D908/92A/94C/9C3); also tooltip strref `hud+0x15410`, timer `hud+0x1540C`, sound via 0x740A20 |
| Script `SetNPCAIStyle` | 0x696580 | writes `stats+0x146` at 0x696636 (no client/HUD call in the body) |
| Stance list builder | 0x77A9E0 (RET 4) | at 0x77AAAD forces invalid values to 9 (accepts 9,0xB,0xC,0xD) |
| Load / ctor | 0x6B14A6 (ReadStatsFromGff), 0x6A473C (stats ctor) | |
`stats` = `*(creature+0x1198)`; `creature` = `0x77D800(ECX = controlled member from 0x7E5D80)` (DB: "Party entry -> creature thiscall"). Single player: the HUD
writes the server object directly, no message round trip, so the change is synchronous.
Key map (0x74DB87..0x74DBD4): stance 9->0x410, 10->0x411, 0xB->0x412, 0xC->0x413, 0xD->0x414, other->-1 (RangedAI turns 16 into 0x415, see Q3).
Form (separate, group 5, keys 0x102..0x10C): `creature+0x1110`; writers 0x77D120 (HUD, via callback 0x77AEB0 list), script `SetCurrentForm` 0x691020 (write at 0x6910CE, guarded by HasSpell 0x6BD9C0), load 0x6B108D, ctor 0x562932. Same refresh gap applies.
Character sheet / abilities menu: no writer of 0x146 or 0x1110 found there (census above is exhaustive over the dump). HYPOTHESIS: no such path exists; absence is not proof (RE guide 3.10).

## 2. How the bar is built and drawn (Q2)
VERIFIED:
- Rebuild `0x74CFE0` (thiscall(hud, int unused) RET 4; the stack arg is never read). Per call: clears the 7 lists (`hud+0x78+g*0xC`, 0x7C = count) and refills via `0x77E650(g, list)` (case 0 -> 0x779510 powers, 1-3 -> 0x77C510 items, 4 -> 0x77A9E0 stances, 5 -> 0x77AEB0 forms), then `0x743540` for the target menu, then for g<6 finds the entry whose key (`entry+8`) == `hud+0x1C7C+g*4` and draws it with 0x7418A0 (icon = `entry+0x20`). No match -> first entry.
- Triggers of the rebuild: EVERY frame from the HUD draw `0x74B3F0` (`0x74B545`, guarded by `[hud+0x48]` bit 6 = visible); also `0x74DC43` (end of Sync), `0x7B3ECF` (0x7B3CA0), `0x7CEB6E` (0x7CEB50).
- Inside the rebuild the ONLY selection write-back is the character change test (`0x74D0D5`: `hud+0x6C != controlled id` -> store -> `CALL 0x74DA40` at `0x74D0E1`).
- `0x74DA40` (fastcall(hud), plain RET): reads `creature+0x1198+0x146` -> key (group 4 cell `hud+0x1C8C`), `creature+0x1110 & 0xDFFFFFFF` -> `hud+0x1C90` (group 5), and a Canderous implant byte -> `hud+0x1C7C` (group 0, only if item 0xCB owned); each cell written only if value > 0 (0x74DC08..0x74DC38); then `CALL 0x74CFE0` with push 0 (0x74DC43).
- Callers of 0x74DA40 (exhaustive grep of the dump): `0x74D0E1` (controlled character changed), `0x7CA0E8` (CGuiInGame::HideSWInGameGui 0x7CA060, i.e. closing in-game panels), `0x751BCB` (0x751B00, handler after the arrow buttons 0x751C30/0x751CE0 set `hud+0x15414 = 1`; resyncs only if the selected entry has `entry+0x38 != 0`, which 0x77A9E0 sets for stance rows, then clears 0x15414).
- Click path: button `0x7516B0` -> `0x751750(hud, g, 0)` (thiscall RET 8): sets `hud+0x1C98 = g`, finds the entry matching the cell, runs the entry callback `(*(entry+0xC))(entry.key, creature)`, then `0x741780(1)`. It never writes the selection cell. Cells are written only by the arrow cycle (0x752450 next / 0x7525D0 prev, both `W` of `hud+0x1C7C+g*4`) and by `0x74D930` (menu pick) and by Sync.
So the exact missed condition: **`creature+0x1198+0x146` (or `+0x1110`) changed while `hud+0x1C8C`/`0x1C90` stayed at the old key, with no character switch, no HideSWInGameGui, no arrow release.** There is no dirty flag and no poll comparing the two.
Vanilla consistency: arrow cycle shows the candidate, click activates; cell == state afterwards, and 0x751B00 reverts an un-clicked cycle. Non-HUD writers (script SetNPCAIStyle/SetCurrentForm, our hotbar) are the lag cases.

## 3. Mod involvement and hook overlap (Q3)
VERIFIED, read-only scan of every `*.toml` with `[[hooks]]` (patch_manager_mods/*/ and kotor-patch-manager/Patches/*/), address inside the functions in this note (script: scratchpad `ov.py`; Steam-targeted files only matter):
| Patch | Hook | Effect on this bug |
|---|---|---|
| force-power-hotbar | 0x74B3F0 detour (HUD draw entry, runs before the original) | **Cause (see below)**: `drainRing` -> `invokeEntry` restores `hud+0x1C7C+g*4` and `hud+0x1C98` after 0x751750 (ForcePowerHotbar.cpp:416-429) |
| perf-lab | 0x74CFE0 detour `PerfLabActionBar` | counter only (`InterlockedIncrement64`), `skip_original_bytes=false`; no behaviour change |
| ranged-ai | 0x77AA5E, 0x77ACBC (0x77A9E0), 0x77D8D4 (0x77D890), 0x74DBDD (0x74DA40) | adds stance 16 ("Ranged (Kiting)", key 0x415); touches the Sync default branch only; does not skip the Sync |
No hook sits on 0x751750, 0x751B00, 0x7CA060, 0x696580, 0x77D120. Not checked: what is actually installed live (Steam dir off limits).
Is it vanilla? HYPOTHESIS (medium-high): if the user changed the mode with the hotbar key/button (`other:414` in the "Modes and stances" picker), the stale icon is our restore line; with the vanilla HUD mouse flow it should not occur. Needs one fact from the user: how the mode was changed. The hotbar log (`invoke: returned group=4 key=00000414 ... selection restored to ...`) answers it.

## 4. Proposed fix (Q4)
Preferred (smallest): in `invokeEntry`, after the `0x751750` call and the restore block, for `e.group == 4 || e.group == 5` (also harmless for 0):
```
reinterpret_cast<void(__fastcall*)(void*)>(0x0074DA40)(reinterpret_cast<void*>(hud));   // fastcall(ECX=hud), no stack args, plain RET (RET 0)
```
It rewrites the cells from the real creature state (so the restore of the old key becomes irrelevant for groups 4/5) and rebuilds the bar (0x74DC43). It uses the game's own code, covers RangedAI stance 16, and runs in the HUD tick on the main thread (same thread as the game's click handler).
Equivalent alternative: skip the restore (`*sel` stays = key) for groups 4/5 - fewer calls, but does not fix the script writers.

Optional change-detection (covers `SetNPCAIStyle` / `SetCurrentForm` from dialogue or scripts), in `HotbarHudTick` once per frame, only when `hud+0x15414 == 0` (not mid arrow-cycle) and the HUD gate is open:
- server creature: `s = 0x77D800(ECX = controlledMember)`; if `s` null, stop.
- `stance = *(u16*)(*(s+0x1198) + 0x146)`; `form = *(s+0x1110) & 0xDFFFFFFF`.
- expected cell: stance 9/10/0xB/0xC/0xD -> 0x410..0x414, 16 -> 0x415, else skip; compare to `*(int*)(hud+0x1C8C)`; form > 0 vs `*(int*)(hud+0x1C90)`.
- if either differs, call `0x74DA40(hud)` once, then remember the pair so a persistently unmapped value (-1 key) cannot loop. Keep `lastStance/lastForm/lastCreature` statics (the existing `controlledCreature()` is already polled each frame).
Risks:
- Re-entrancy: the call runs the full rebuild, which frees and rebuilds all entry lists (`0x7ECFA0`). Never call it while `readGroup` pointers are held (resolve/invoke already finished) and not from the DirectInput capture hook (0x72CB63); the tick is right (the original 0x74B3F0 calls 0x74CFE0 itself a few instructions later, so a second rebuild per frame is the only cost).
- Menus open: the draw tick only runs when the HUD is visible (`[hud+0x48]` bit 6), and the mod's gate already blocks modal/dialogue; keep the poll behind that gate. HideSWInGameGui already syncs when panels close.
- Selection cells are shared with vanilla arrow cycling: do not call while `hud+0x15414 != 0`, or a cycling player gets snapped back (vanilla does that itself on release).
- Group 0 side effect: Sync also writes `hud+0x1C7C` for a Canderous implant (item 0xCB); vanilla does the same on character switch.
- Combat queue: the stance click callback writes the stat directly and plays a sound/message (0x740A20); our extra Sync only reads state and rebuilds, queues nothing. `0x751750` still runs the game's own `creature+0x458 & 1` branch (action queue vs immediate) unchanged.
- Cell/state mismatch for unmapped stances (16 without RangedAI loaded): the key is -1, cell untouched, bar keeps the old icon - same as vanilla.

## 5. Unverified
- How the user changed the mode (hotbar vs vanilla click vs script). Biggest open item.
- Callback calling convention of 0x77D890 (stack args key/creature) and exactly which event triggers 0x751B00 (no static callers; vtable dispatch) - arrow-release is a HYPOTHESIS from its reads of `hud+0x15414` and `param_2+0x50/0x5C`.
- Live check that `0x74DA40` after `invokeEntry` updates the icon (never run; user playing). Needs a probe log line before/after (cell value before, after).
- Whether other patches installed live hook these functions (only repo hook files read).
- "Jedi mode" = stance 13 is inferred from `ib_jedisupport` (0x77AA..: case 0xD with `0x6F2A30` Jedi-class check) and the review doc's "stances 13 (Jedi)"; strref text 0x1EC3E/0x1EC66 not read from a TLK.
