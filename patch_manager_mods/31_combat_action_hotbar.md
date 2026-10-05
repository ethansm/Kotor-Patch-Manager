# 31 — Combat Action Hotbar (master plan)

## Status
- **Mod id:** `combat-hotbar` · source `combat_hotbar_src/` · Steam Aspyr `6A522E71…` (live exe = vanilla+4GB, same as all mods)
- **Current (2026-10-04 23:17): CH1c + HL2 INSTALLED** (user GO). hud-host 0.1.0-dev dll `4dabbff1` + combat-hotbar 0.2.0-probe dll `4af3e9b6` (src commits `e091df9` / `3cdc84e`; KPM local-mods `d1a7dbb` `Patches/HudHost` + `Patches/CombatHotbar`). Stage `$K/combathud_stage_1004` (STAGE_INFO head=d1a7dbb; DIRTY = sibling .bak/CharEffectsPanel only; PROBLEMS 1 = known 4gb artifact), 36 ids applied, verify mismatches 0, config `b073054f` -> `67da27ea` (2 appended blocks, hud-host first). kplog.ini: `[hud-host]` info, `[combat-hotbar]` debug. Backup `$G/backups/combathud_install_20261004_231706` (rollback: restore it). Gate: pre_install_gate `--skip-default` PASS (default tier 118/119; the 1 FAIL = queue-hud session's `smoke_queuehud.cpp:60` raw address). **Next:** user targets enemy, door, placeable, mine, droid (~5 min), then `kotor2_logs.py check` on `$G/combat-hotbar_log.txt` + `hud-host_log.txt` -> answer open Q1/Q4, mark CH1c + HL2 DONE.
- **Current (2026-10-04 23:46): combined build INSTALLED** (user GO via plan approval). hud-host 0.2.0-dev dll `bf5421f0` (adds TARGET_MENU_POST hook 0x743850, src `8094fbd`) + combat-hotbar 0.3.0-dev dll `45b8d8ab` (CH2 invoke keys + CH3b arrow hide, src `6c8047e`); KPM local-mods `ac89da8`. Stage `$K/combathud2_stage_1005` (STAGE_INFO head=ac89da8d67b0; DIRTY = sibling .bak/CharEffectsPanel only; PROBLEMS 1 = known 4gb artifact), 36 ids applied, verify mismatches 0, config `67da27ea` -> `629754e3` (diff = the one new hud-host hook). Backup `$G/backups/combathud2_install_20261004_234629` (rollback: restore it). Gate: default tier 4 FAIL, none ours (save-events hook-span overlaps + queue-hud raw address = sibling sessions; exports/shared-header FAILs cleared by the rebuild + sync). **Next:** user runs the Play-test checklist below (~10 min).
- **Current (2026-10-05 00:54): HL3 + CH4b INSTALLED** (plan GO, game closed). hud-host 0.3.0-dev dll `c6121ae2` (BUILD from 0x40F5A0, src `9e18444`), companion-selfheal 0.4.2-dev dll `3e763e45` (build via hud-host, own 0x40F5A0 hook removed, src `a9ee44d`), combat-hotbar 0.4.0-dev dll `ee1112e0` (button grid, src `3635cde`+`d75d4d0`); KPM local-mods `0e8942d`. Stage `$K/combathud3_stage_1005`, 36 ids (hud-host before companion-selfheal), 220 hooks, config `629754e3` -> `1f7b0947` (0x40F5A0 moved to hud-host). perf-lab/mouse-probe/queue-hud live DLLs (`f085f754`/`71e8f8e5`/`2950ce26`, peer hot-swaps) copied back after the install, so verify shows those 3 as expected mismatches. Backup `$G/backups/combathud3_install_20261005_005409` (rollback: restore it). **Next:** Play-test checklist v2 (~10 min).
- **First CH4b play 10-05 01:11:** proof of concept works (grid under g0/g2, screenshot `~/Documents/Screenshot_2026-10-05_01-11-46.png`). User next: vanilla-styled buttons (border/fill/highlight/sound like the action hotbar), replace the vanilla group buttons, no lag (1-frame: POST runs at 0x743850 entry, before it writes the origin), wider rows. Plan = CH4c (handoff `.claude/handoffs/31_combat_action_hotbar.handoff.md`).
- **First play 10-05 00:18 (log CLEAN):** type 3 enemy: g0 = melee feats (4), g1 = empty, g2 = grenades; type 1 mine: g0 disable / g1 recover. User expected visible buttons under the target health bar: that is CH4b, blocked on HL3. Details: handoff `.claude/handoffs/31_combat_action_hotbar.handoff.md`.
- **Resume here:** first row of the §4 plan table whose Status is not DONE (`python3 scripts/plan_manager.py resume --plan patch_manager_mods/31_combat_action_hotbar.md`). Library: §5.

---

## 1. Goal

Vanilla targeting shows 3 "odometer" boxes under the target name (groups 0..2 of `CSWGuiTargetActionMenu`); each needs up/down arrows or the wheel to cycle. This mod adds dedicated 1-click buttons that queue a chosen target action (attack feat / offensive power / grenade) against the current target, using the engine's own invoke so range, pathing and cost checks stay native.

Not in scope: personal (self) actions — those are the Action Hotbar (doc 07, `action_hotbar_src`), which has **no** target-menu support (doc 07:575 deferred it: "selection storage unverified" — now verified, §2C).

---

## 2. Verified engine facts (Steam, review 2026-10-04)

Dumps: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/74/<addr>.c|.asm`. `menu = hud + 0xCC`.

### A. Hook site `0x00743540` = `CSWGuiTargetActionMenu::PopulateMenus`
- GOG `0x514B40` (`gog_map_mods678.tsv:1727`). Knowledge DB names it `Update` (wrong name, right function) — fix the DB row in CH1b.
- Prologue `55 8B EC 83 EC 38` = 3 whole insns, 16-aligned, no rel branches → valid 6-byte detour. `RET 0xC`, thiscall, `ECX = menu`.
- Sole caller `0x74CFE0` UpdateTargetMenu, called unconditionally from HUD Draw `0x74B3F0` → **runs every frame** (~0.98/frame, perf-lab W4).
- **Stack args at detour entry** (caller frame, `0074cfe0.asm:13-14,33,89-90`):
  | src | meaning |
  |---|---|
  | `[esp+4]` | party-leader client creature (`0x7E5D80`) |
  | `[esp+8]` | force-unusable flag (default 1, cleared when leader can act) |
  | `[esp+0xC]` | target client object (`GetGameObject(hud+0x68)`), 0 = none |
  `[ebp+0x10]` (Gemini's probe) reads 0x74CFE0's frame = garbage.
- Inside: frees the 3 lists, refills each via `0x77E370` GetTargetActions, `0x741660` per-group control refresh, then `0x742A00` UpdateNameLabel, `0x743850`. So at **detour entry the lists hold the previous frame's entries** (valid until the free inside the original).

### B. Group lists (same entry type as doc 07's HUD lists, reset fn `0x7ECFA0`)
- `menu + g*0xC` = `{entries, count, cap}`, g = 0..2. `menu+0x00 == hud+0x78+7*0xC` → target groups are HUD list indices 7..9.
- Entry stride `0x3C`: `+0x00/+0x04` CExoString name (ptr,len — layout per doc 07, still marked unconfirmed there), `+0x08` key, `+0x0C` invoke fn ptr, `+0x1C` owner obj, `+0x20` icon resref16, `+0x30` bit0 usable.
- Group *meanings* (feats / powers / items) — **UNVERIFIED**, CH1b probe must log them.

### C. Target type and selection
- `menu+0x1BAA` int8 = **target type** from `CSWCCreature::GetTargetInterfaceTargetType` `0x77DF90`: 0..3, `0xFF` = no target. (Gemini called it "active char index" — wrong.)
- Selected key: `menu + 0x24 + type*0xC + g*4`, 4 rows (types 0..3). With type `0xFF` the engine indexes `menu+0x18` (stale-read quirk) → never invoke when type == 0xFF.

### D. Invoke
- `0x007445D0` DoTargetAction: thiscall`(menu, g, unused)` `RET 8` (GOG `0x515BD0`). Takes **no key**: reads the selected key (§2C), falls back to entry[0]; needs entry `+0x1C` owner resolvable and `+0x30` bit0; then calls `entry+0x0C(key, leader)`; special cases type 0 / 3, reads `menu+0x1644`.
- Wrapper `0x00751710` thiscall`(hud, g, _)` `RET 8`: bounds `0<=g<3` → `0x7445D0(hud+0xCC, g, 1)`. Prefer the wrapper (bounds check for free).
- Recipe (mirrors Action Hotbar's personal recipe at `0x751750`): write selection slot → call → (restore or keep — CH2 decides from native behaviour).
- Neither address is in the Steam address DB yet → add rows (with Haiku verify + decoys, per DB rules) in CH2.

### E. Native carousel / arrows
- Group control `i` = `hud + 0x120 + i*0x750` = `menu + 0x54 + i*0x750` (`00743540.asm:103-106`).
- `ctl+0x48` is the **whole group container** (icon too). Flags: bit1 visible, bit5 `0x20` click-through.
- Arrow sub-controls: flags at `ctl+0x3E8` and `ctl+0x5B8`, **rewritten every frame by `0x741660`** inside PopulateMenus → a write at our detour entry is overwritten. Suppression needs a post-point (CH3a).

### F. Occupied hook sites (do not reuse)
| site | owner |
|---|---|
| `0x74B3F0` HUD Draw | action-hotbar (+EKC) |
| `0x748502` HUD ctor build window | action-hotbar |
| `0x40F5A0` StopLoadFromLayout | companion-selfheal (HUD-vtable filter) |
| `0x74CFE0` UpdateTargetMenu | perf-lab |
| `0x72CB63` key capture | action-hotbar |
`0x743540` is free (check_hook_spans PASS: 22 src / 209 spans, 33 installed / 247 spans).

### G. TARGET_MENU_POST site (CH3a, full evidence in `31a_target_menu_post_re.md`)
- **VERDICT: SUITABLE.** Detour `0x00743850`, `original_bytes = 55 8B EC 81 EC E4 00 00 00` (3 whole insns, 16-aligned, unowned). Its only caller is the single exit path of PopulateMenus (`0x743847`, after all `0x741660` calls and `0x742A00`).
- Entry: `ecx` = menu, `[esp+4]` = leader, `[esp+8]` = target (may be null), `RET 8`.
- `0x741660` writes only **bit 1** of `ctl+0x3E8` / `ctl+0x5B8` (set iff group count > 1). Both draw (`0x7414D0`) and hit test (`0x418D20`) check bit 1.
- **Hide write at POST**, for i in 0..2: `*(u32*)(menu+0x43C+i*0x750) &= ~2; *(u32*)(menu+0x60C+i*0x750) &= ~2;`
- UNVERIFIED: whether wheel events (500/0x1F5) are hit-test gated. Worst case, the wheel does not work while the pointer is over a hidden arrow. Check this in the play test.
- Steam DB rows for `0x7445D0` / `0x751710` / `0x743850`: deferred to the steam-db session.

---

## 3. Shared code: what to reuse, what to add

| Need | Use | Note |
|---|---|---|
| guarded reads | `SafeRead.h` (`readAt`, `probeRead`, `readExo`, `readResRef16`) | call `saferead::beginScope()` once per frame; `readExo` for UI names (`readExoLoose` only in logs) |
| logging | `KpLog.h` channels | already used |
| list reads | `ItemRead.h readGroup` | clamps g to 0..6 → add `readMenuGroup(menuBase, g)` or widen to 0..9 (Common change: sync via `check_shared_headers.py --sync`, contract tests in every adopter) |
| addresses | `GameAddr.h` / `K2Addr.h` | add `TargetMenuPopulate 0x743540`, `HudTargetInvoke 0x751710`, `TargetDoAction 0x7445D0`, `Menu_TargetType 0x1BAA`, `Menu_Sel 0x24` |
| leader / party | `Party.h` | cross-check `[esp+4]` leader |
| widgets, hit, clicks | `GuiKit.h` (`createLabelFromTemplate`, `appendToPanel`, `hit`, `clickEdge`, `winForeground`) | recipe: `RuntimeGuiControls.md:37-60` |
| tooltips | `GuiTip.h` | |
| INI hot-reload | `FileWatch.h` | CH5 |
| pure logic | new `combat_hotbar_src/CombatLogic.h` + host test | slot pick, auto-fill rank, INI codec (pattern: `HotbarLogic.h`, `SelfHealUi.h`) |
| HUD build + tick + target menu | **`hud-host` library, `k2.hud@1` (§5, batches HL0-HL4)** | all HUD sites are taken (§2F); 4 consumers now |

Template mod: `companion_selfheal_src` (GuiKit + GuiTip + ItemRead + registry client).
SafeRead site range: `0x6F00` — register it in doc 19b's site-id list (CH1b).

---

## 4. Plan

Rules: one batch per row; each batch ends with `build_check.py --mod combat_hotbar_src`, `run_tests.py -k combat`, `check_hook_spans.py`, commit (explicit paths), `plan_manager.py mark-done <id>`. Installs follow the KPM Install Rule (commit → `stage_and_verify_install.py stage` → KPatchLauncher NoLaunch, game closed, backup noted).

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| CH1 | Gemini probe scaffold (toml, manifest, contract test, CombatHotbar.cpp) | - | - | DONE | (uncommitted) | CH1b |
| CH1b | Fix probe (temporary own hook; moves into hud-host at HL2): hook params → `[esp+4]` leader, `[esp+8]` flag, `[esp+0xC]` target (KPM supports `source = "[esp+N]"`, e.g. inventory_sort_filter_src toml:23); rename `charIdx`→`targetType`, guard `0..3` (not `<3`), skip `0xFF`; `beginScope()` per call; log group meanings (entry names) + `menu+0x1644`; drop unused include; fix DB name `Update`→`PopulateMenus`; register 0x6F00 in 19b; extend contract test (readExo) | CH1 | - | DONE | c56f0ed | CH1c |
| CH1c | Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3 | CH1b | user play (5 min) | TODO | | HL0 |
| HL0 | `hud-host` design: `_shared_src/HudHost.h` (plain-C `k2.hud@1` iface + `k2hud::Client`, pattern = `PortraitSlots.h`), phase list §5, hook ownership, migration order; append §4.3 update to doc 22; user approves | CH1b | user approval | DONE | 71cb81e | HL1 |
| HL1 | `hud_host_src/` library (`kind = "library"`, `provides = ["k2.hud@1"]`): owns `0x743540` (TARGET_MENU_PRE + FRAME phases), `HhDispatch.h` pure dispatcher + `hh_dispatch_test.cpp` + `hh_registry_test.cpp` (pattern `portrait_slots_src/PsDispatch.h`); `check_library_manifests.py` PASS | HL0 | - | DONE | e091df9 | HL2 |
| HL2 | combat-hotbar becomes consumer: drop its own 0x743540 hook, `requires = ["hud-host"]`, registers TARGET_MENU_PRE; probe output identical to CH1b; install both | HL1, CH1c | user play | PARTIAL (offline ok, live pending) | 3cdc84e | CH2 |
| CH2 | Invoke test (inside the hud-host TARGET_MENU_PRE callback): per-frame edge poll (`GetAsyncKeyState` gated on `guikit::winForeground`, not key hook 0x72CB63) Ctrl+Shift+1/2/3 = invoke entry[last] of group g via write-sel + `0x751710` from inside the 0x743540 detour (entries still valid). Log queue result. Add DB rows 0x7445D0 / 0x751710 (Haiku verify + decoys) | HL2 | user play | PARTIAL (built 6c8047e, installed 10-04 23:46, live pending; keys Ctrl+Shift+F9/F10/F11 via INI, not digits; DB rows deferred to steam-db session) | 6c8047e | CH3a |
| CH3a | Static RE: find a post-`0x741660` point (becomes hud-host TARGET_MENU_POST phase) for arrow suppression. Candidates: entry of `0x743850` (called at end of PopulateMenus), or a mid-function site after the call at `0x74D19F` in 0x74CFE0 (must not overlap perf-lab span). Confirm arrow sub-control flags `+0x3E8/+0x5B8` and the hit-test path `0x7433B0` HitCheckMouse | CH2 | - | DONE | ec793cf | CH3b |
| CH3b | Suppression behind INI `hide_arrows=0/1` (default 0); wheel still works | CH3a | user play | PARTIAL (built 8094fbd + 6c8047e, installed 10-04 23:46, live pending; INI [Arrows] Hide=1) | 6c8047e | CH4 |
| HL3 | hud-host takes `0x40F5A0` (StopLoadFromLayout, filtered on HUD vtable `0x0099DCC4`) → BUILD phase with `ReserveIds(n)`; companion-selfheal migrates its build to BUILD (same install, KPM rejects double hooks) | HL2 | selfheal session owner + user play | PARTIAL (BUILT 9E18444+A9EE44D, LIVE PENDING) | a9ee44d | CH4b |
| CH4a | `CombatLogic.h` pure layout + hit + slot model + host tests (no engine) | CH2 | - | DONE | 30929bd | CH4b |
| CH4b | Runtime buttons via GuiKit on the HUD panel (hud-host BUILD phase), icon = entry `+0x20`, greyed when `+0x30` bit0 clear, GuiTip names, click → CH2 invoke path; dock under target banner; ultrawide-safe (`aspectFix`) | CH4a, HL3 | user play | PARTIAL (BUILT D75D4D0, LIVE PENDING) | d75d4d0 | CH5 |
| CH4c | Vanilla-styled buttons replace the 3 target-menu boxes: hud-host POST moves to the PopulateMenus tail 0x743847 (origin final, zero lag); 2 labels/slot (bg `lbl_miscroll_f`/`_hi` + icon), boxes hidden via `+0x48` bit1, banner-wide grid (`ch::layoutBannerGrid`), hover freeze by origin write-back (`ch::freezeOrigin`), hover/click sounds; INI `[Buttons] Replace/Cols/MaxRows/BtnPx/Pad/GroupGap/Inset/ClickSound` | CH4b, HL3 | user play | PARTIAL (A built: 78193fc hud-host tail, d0397cf layout, RE doc 31b) | d0397cf | CH5 |
| CH5 | Auto-fill + INI profiles: highest tier per feat/power family, carried grenades by count; `FileWatch.h` hot-reload; INI codec in `CombatLogic.h` with tests | CH4b | user play | TODO | | ship |
| CH6 | Ship: KpLog trim, mutation run on `CombatLogic.h`, doc 24 row, upstream PR candidate | CH5 | user approval | TODO | | - |
| HL4 | Later, optional: action-hotbar (`0x748502`, `0x74B3F0`) and queue-hud (`0x74C168`) migrate to BUILD/FRAME; GOG hooks file (static-verified, GOG callout rule doc 21) | HL3 | owners | TODO | | - |

### Play-test checklist v2 (HL3 + CH4b, ~10 min)
1. Target an enemy: icon buttons appear under the 3 vanilla boxes, 2 per row; unusable icons dimmed.
2. Hover a button: tooltip shows the name and the row stays put. Click Flurry: it queues.
3. Throw a grenade from the grenade column.
4. Selfheal regression: its HUD buttons + popover still work.
5. Say "check combat log" (look for `first BUILD dispatch`, `BUILD buttons created=`, `buttons: first draw`, `click g=`).

### Play-test checklist v1 (combined CH1c + CH2 + CH3b, ~10 min)
1. Target an enemy, door, placeable, mine and droid (CH1c).
2. With an enemy targeted, press Ctrl+Shift+F9, F10, F11. Does the action queue? (CH2)
3. Quit, create `combat_hotbar.ini` in the game dir with `[Arrows]` / `Hide=1`, relaunch: carousel arrows gone, mouse wheel still cycles (also with the pointer where an arrow was)? (CH3b)
4. Say "check combat log" -> `kotor2_logs.py check` on `combat-hotbar_log.txt` + `hud-host_log.txt` (look for `config:`, `invoke g`, `arrows: hide`, `first POST dispatch`).

### Open questions (answer in the batch shown)
1. Group meanings and which target types show which groups — CH1c.
2. After a 1-click invoke, keep the new selection (carousel shows it) or restore? — CH2.
3. Does `entry+0x0C` queue for the leader only, or the controlled character? (`[esp+4]` is the leader) — CH2.
4. Does the force-unusable flag `[esp+8]` grey whole group 1? — CH1c log.
5. Button placement vs Action Hotbar and ultrawide letterbox — CH4b.

---

## 5. Shared library `hud-host` (`k2.hud@1`)

Why now: four HUD mods each need a HUD hook and the free sites are gone (§2F): action-hotbar (build `0x748502` + tick `0x74B3F0`), companion-selfheal (build `0x40F5A0`), queue-hud (tick `0x74C168`), combat-hotbar (target menu `0x743540`). The KPM installer rejects two patches on one address (doc 21 Lessons), so shared sites need one owner. Doc 22 §4.3 said "not worth a library for the hotbar alone" — that condition is now met.

Pattern: copy `portrait-slots` (doc 21): `kind = "library"`, `provides`, `KPatchInit` provides the iface through `KPatchApi::provide`; consumers `require` in `KPatchInit` and retry with `poll()` (one require per 256 calls); missing provider = consumer inert, no crash. Consumers list `requires = ["hud-host"]` so KPM topo-sorts install order.

Interface sketch (`_shared_src/HudHost.h`, plain C, `struct_size`-versioned):
```c
#define K2HUD_IFACE "k2.hud"
#define K2HUD_VERSION 1
enum { K2HUD_BUILD, K2HUD_FRAME, K2HUD_TARGET_MENU_PRE, K2HUD_TARGET_MENU_POST, K2HUD_PHASE_COUNT };
typedef struct K2HudContext { uint32_t struct_size; void* hud; void* panel; void* menu;   /* hud+0xCC */
    void* leader; int32_t forceUnusable; void* target; int8_t targetType; uint32_t frame; } K2HudContext;
typedef void (KPATCH_CALL *K2HudCallback)(const K2HudContext* ctx, void* user);
typedef struct K2HudApi { uint32_t struct_size;
    int32_t (KPATCH_CALL *Register)(int32_t phase, int32_t priority, K2HudCallback fn, void* user, const char* owner);
    void    (KPATCH_CALL *Unregister)(int32_t token);
    int32_t (KPATCH_CALL *ReserveIds)(int32_t n);   /* BUILD only: first control id of a block; HL3 */
    void*   (KPATCH_CALL *Hud)(void);               /* last seen hud or 0 */
    uint32_t (KPATCH_CALL *Frame)(void); } K2HudApi;
```
Phase sources: FRAME + TARGET_MENU_PRE = `0x743540` entry (HL1, every HUD draw; entries of the previous frame still valid); TARGET_MENU_POST = CH3a site; BUILD = `0x40F5A0` + HUD vtable filter (HL3). FRAME runs before TARGET_MENU_PRE in the same call.

Rules: library draws nothing and never writes engine memory itself; per-phase priority, ties keep registration order; one SafeRead site range for the library (`0x7000`, register in 19b); GOG = static-verified only (callout rule, doc 21).
