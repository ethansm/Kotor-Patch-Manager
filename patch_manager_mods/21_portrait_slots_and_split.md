# 21 - Portrait Slots provider and the Buff HUD / Shield Bar split

Mod 2 (Buff Duration HUD) and Mod 4 (Shield Bar) shipped as one DLL, `buff-duration-hud` v1.1.0. They shared the party-portrait hooks, the
slot -> creature table, the effect scan, the clip / 2D-batch / GL guards and the menu overlay. They are now three patches on shared code.
User decisions (10-03): a neutral provider patch `portrait-slots` on the KPM patch registry (RFC #180), and the registry merged into our
live patcher first.

## RESUME HERE (2026-10-03 evening)
| Step | State |
|---|---|
| W1 registry core | merged into KPM `ethansm/local-mods` fa67a3b; full gate (incl. i386) PASS; **live** core `36f10b7f` (backup `backups/registry_core_install_20261003_114439`); `bin/NoLaunch` = 36f10b7f, `bin/Debug` still old `2a706ac9` |
| W2 shared headers | 85eb91f; **live** `buff-duration-hud.dll` `feed7a4b` (single DLL, same behaviour; backup `backups/shield_split_w2_install_20261003_121921`); rollback `kotor-patch-manager/shield_live_patches/` (b7dbebe9 + art) |
| W2 live gate | **pending user session**: visuals of b7dbebe9 features, Proton log all `Applied`, `AR_ERROR.LOG` md5 `a2d88789` unchanged, diff `shieldprobe_log.txt`/`buffhud_log.txt` against `*_b7dbebe9_baseline.txt` (DRAW/POOL/MENUCOMP/FORCEMOVE/MENUMOVE equal modulo t=/f=) |
| W3 provider | 5954112 (`portrait_slots_src/`, `_shared_src/PortraitSlots.h`, ps_dispatch_test 53 checks) |
| W4 consumers | a193a08 + 354b507 (`buff-duration-hud` 2.0.0, `shield_bar_src/` 1.2.0, shield_logic_test 37 checks); KPM mirror 28ae185 + 6192f36 |
| W5 install + A/B | **variant both LIVE 15:33** (user chose to skip the separate W2 gate): portrait-slots b6914e69 + buff 2.0.0 f37135c8 + shield-bar 55dcd83e, patch_config 248c996a from `split_stage_both/`, kplog.ini enabled (portrait-slots info); backup `backups/shield_split_w5_install_20261003_153348` (W2 config + dll); W2 logs renamed `*_w2_20261003_153348.txt`; kpm_install_state.json NOT changed (a KPM reinstall reverts to the single W2 DLL = safe). Awaiting user session. Before: Packages in `kotor-patch-manager/split_build/` (portrait-slots `b6914e69`, buff 2.0.0 `f37135c8`, shield-bar `55dcd83e`); stage with `scripts/stage_portrait_split.py --variant both|buff|shield|none` |
| W6 adoption | later, owners: active_effects_panel -> Effects.h / EffectIcons.h |

## Architecture
```
_shared_src/ (synced to KPM Patches/Common by scripts/check_shared_headers.py --sync; *.inc included since W2)
  Effects.h       k2fx: effect list walk, currentTime, remainingRatio/effectRatio, isItemAbility, scanShieldPools     (effects_test 180)
  EffectIcons.h   + EffectIconTable.inc / DebuffIconTable.inc: iconForSpell, collectBuffs
  Gui2D.h         gui2d: Begin/EndGui2D, clip, flush, draw, hudScale, inMenuFrame, panelRect, Gui2DScope/ClipScope, CapGuard   (gui2d_test 159)
  PortraitSlots.h "k2.portraitslots" v1 (plain C) + k2ps::Client consumer helper
portrait-slots 1.0.0     hooks 1a 0x74F25A, 1b 0x74F8AB, 6 0x745774. Slot table, menu slot, phase dispatcher. Logs via kpatch.log (portrait-slots/core).
buff-duration-hud 2.0.0  hooks 2 0x745A84, 3 0x745A97, 5 0x745AEC. requires portrait-slots.
shield-bar 1.2.0         hook 7 0x744EEC (tooltip). requires portrait-slots. Owns ShieldTable.inc + art (uibit_bar_sh_p, uibit_bar_sc_p; gen_shield_bar_art.py).
```
No address is hooked by two patches (`check-hook-spans` PASS). Each DLL has its own CapGuard, imageFor pool and probe flags.

## Phases (hook bodies keep the v1.1.0 order)
| Phase | When | Buff (200) | Shield (100) |
|---|---|---|---|
| SNAPSHOT | hooks 1a/1b, before the table update | - | probeEffects (shieldprobe.txt) |
| PRE_ENGINE_READ | hook 6 entry, every slot, before the engine reads slot+0xddc/0xddd | ForceArrowCounters | - |
| FRAME_BEGIN | hook 6, leader call, after frame++ | buffhud_off.txt poll | shield_nodraw.txt poll, scene probe, F9 MARK |
| SLOT | hook 6, every slot | - | FORCEMOVE (leader), SLOT log |
| MENU_OVERLAY | leader call in a menu frame; provider opens Gui2D + panel clip + CapGuard once, only if a callback exists | companion icon columns | top-bar bar, MENUMOVE spacing, companion gauges + bars |
| HUD_DRAW | leader call in the HUD frame | - | all HUD bars (SlotAt walk) |

Lower priority runs first; ties keep registration order. `runSlotDraw` (PsDispatch.h) is the single implementation of the hook-6 sequence;
ps_dispatch_test T2 asserts the call order for a HUD frame (leader + 2 cards) and a menu frame equals v1.1.0's.

Behaviour differences vs v1.1.0 (intended):
- Menu companions: v1.1.0 interleaved bars0, cols0, bars1, cols1. Now it's bars0, bars1, cols0, cols1. They don't overlap: bars sit outside the portrait, columns are clipped inside it.
- `shield_nodraw.txt` no longer hides the buff columns in menus.
- `shield_noswap.txt` and swapHealthBar were removed; they were dead code (SquareHealthEdge=false).
- Logs: FLAGS is split into `nodraw` / `nobuff`. The DRAW line lost the buff raw/tracked fields. MENUDRAW is now `bar= companionBars=` plus a new `MENUCOLS`. A new `PORTRAITSLOTS ok|MISSING` line logs the provider connection.

## Registration and init order (important)
- KPatchInit runs per module, **in config order**, during patcher start, before any frame. Consumers `require` the provider in KPatchInit.
  If it's missing, they retry with `g_ps.poll()` in their own hooks (one `require` per 256 calls). Buff's hooks only run when arrows exist and shield's only on hover, so the retry is a fallback, not the plan.
- The KPM installer topo-sorts `requires` (DependencyValidator.CalculateInstallOrder), so a KPM install lists portrait-slots first. **A hand-edited patch_config.toml must list portrait-slots before buff/shield.** `stage_portrait_split.py` enforces this.
- The split needs a registry core (KPatchInit). On an old core no provider exists: both consumers are inert (no crash, engine arrows draw natively, no shield bar).
- KPatchInit is DllMain-class code (no file I/O). So the consumers stash the connect result and write the PORTRAITSLOTS line on the first frame.

## W5 install matrix (user, game closed, backup first)
Variants: both / buff only / shield only / none (provider only). For each, check:
- HUD: leader + 2 companions.
- Menu tabs: equip, inventory, character, abilities, map.
- Portrait tooltip.
- Droid / no-Force companion.
- Multiple shields.
- Peragus.
- ForceArrowCounters with a robot leader.
- Head gear after leaving the menu (P2).
- Kill switches.
- `AR_ERROR.LOG` unchanged.

Expected: removing one consumer leaves the other fully working. Provider only = engine untouched (no Begin/EndGui2D, no draws).
Install = copy `split_stage_<v>/patches/*.dll` and `patch_config.toml` into the game dir. Also update `kpm_install_state.json` InstalledPatches, or reinstall through KPM with the split_build packages.
Before W5, `Patches/BuffDurationHUD/BuffDurationHUD.kpatch` stays the live single-DLL build, so sibling reinstalls keep working. At W5, move the split_build packages into `Patches/` and tell sibling sessions.

## Lessons
- The KPM installer rejects two patches hooking one address (HookValidator.ValidateMultiPatchHooks). Shared hooks need one owner, and registry phases replace "hook the same place twice".
- A committed `exports.def` skips create-patch's export scan: add `KPatchInit` by hand.
- In a shared worktree, never `git checkout --` a shared file and never leave paths staged. Other sessions' commits sweep the index (4bcc143 took our renames). Use `git add P && git commit -- P` in one step.
- `check_raw_addresses.py --write-baseline` regenerates the whole allowlist. While other sessions have unreviewed addresses in flight, hand-edit only your own entries.
