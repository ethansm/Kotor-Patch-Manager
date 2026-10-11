# 30 — Queue HUD (`queue-hud` 0.2.0-dev): green out-of-combat action queue, leader + companion strips

## RESUME HERE
- **queue-fix QL1 0.5.0-dev (2026-10-11)**: implemented in source, NOT installed, NOT play-tested. Details: QH4e and the "2026-10-11 Queue-fix QL1" section at the end of this doc.
- **0.4.0-dev INSTALLED 2026-10-05 21:34 (doc 24 CS10-CS12, hot-swap 7d7650a8 -> ab52dfd7, backup `$G/backups/cs10_ch6_dclk_20261005_2134`)**: left strip reserve comes from hud-host StripSlot (order 1, selfheal order 0; fallback `shui::stripReserve`); kit art from `_shared_src/HudKit.h` (kit_frame/_hi, kit_cancel/_hi, kit_disc, byte-identical to the old qh_*); slim auto icon = selfheal button side (24 px on 47x96; `strip_icon_px` overrides). Play check: no overlap with the selfheal button, art identical, both mods register the same kit_* names (watch for missing/garbled textures).
- **0.3.0-dev INSTALLED 2026-10-05 01:41 (user GO, with selfheal CS9b)**: stage `$K/cs11_stage_1005` (STAGE_INFO head=70e5144), 36 ids / 221 hooks,
  live dll `8bfa99d9` (kpatch `8ddf09fe`), config `d9ed58a2`. Backup `$G/backups/cs11_install_20261005_0141` (rollback = restore it, game closed).
  Hand-swapped shield-bar / perf-lab / mouse-probe dlls restored after the install; verify = exactly those 3 md5 mismatches. `kplog.ini` has `[queue-hud] enabled=1`.
  0.3.0 checklist (play test, then "check queue-hud log"):
  1. Out of combat, pause, queue a medpac: green strip uses the kit frame (`qh_frame`), current slot highlighted (`qh_frame_hi`).
  2. A Cancel button shows under the leader queue; clicking it empties the queue (ClearAllActions 0x51CB10, R7).
  3. Companion strips show a Cancel too. OPEN RISK: after a companion Cancel, check the companion still follows the leader (0x3D may be dropped).
  4. Combat: green panel + Cancel hidden, vanilla red panel unchanged.
  5. Log file `$G/queue-hud_log.txt` exists (0.2.0 never wrote one).
- **0.2.0-dev LIVE 2026-10-05 00:09 (user "install when ready")**: DLL hot-swap (hooks/exports unchanged since 0.1.0), live dll `2950ce26`,
  config `629754e3` untouched. Backup `$G/backups/queuehud020_swap_20261005_0009/queue-hud.dll` (= 0.1.0 `75defebf`; rollback = copy it back,
  game closed). Repo `1181d18`, KPM `b53229b`. Gate `--mod queue_hud_src --mod save_events_src` GATE PASS, smokes queuehud 7/7 (new `strips`).
  0.1.0 was never play-tested, so the checklist below covers both. No `queue_hud.ini` in `$G` = defaults (companions=1, strip_side=left).
- Strips: `stripRects` default `left` = row ends `selfHealReserve(vit)` (38 px at the live 47x96 small vit) left of the vit bar, auto icon
  40% of vit.h (38 px), centred on the bar. `above`/`below` (opt-in) only fit with a small `strip_icon_px`: small cards are 85 px tall,
  104 px apart. Slot creature matched against the member's client OR server pointer (slotAt kind unconfirmed; first live `strip k=` line).
- Finding for the selfheal session (not fixed here): `shui::buttonRect` returns empty for `vit.w < 70` and CompanionSelfHeal.cpp:597 treats
  `vit.w < 70` as the leader, but live small-card vits are 47x96 (doc 08 probe 1), so the selfheal button probably never draws on companions.
- State: **INSTALLED 2026-10-04 22:44** (user GO). Stage `$K/queuehud_stage_1004` (STAGE_INFO head=cd0eba3; DIRTY = sibling .bak/CharEffectsPanel only;
  PROBLEMS 1 = known 4gb artifact), 34 ids applied, verify mismatches 0, live dll `75defebf`, config `56e83f2e` -> `bdadbf1e` (appended block only).
  Backup `$G/backups/queuehud010_install_20261004_2244` (rollback: restore it). No INI = defaults. Awaiting the user play test (checklist below).
- Repo commits: `e7fa879` (RE + logic/tests), `66b3702` (DLL), `e4fc966` (Wine smoke), this doc. KPM `ethansm/local-mods` `cd0eba3` (`Patches/QueueHud`).
- Gate: `pre_install_gate.sh --mod queue_hud_src` GATE PASS (113/113 default tier), smokes `queuehud` 6/6.
- Offline next (user 10-04 22:50 "build strips and do L3"): companion strips 0.2.0-dev (build + gate, NOT install) + doc 24 L3 save-events library (not installable until hotbar migrates).
- Next: user runs the checklist below, then "check queue-hud log" (KpLog file `$G/queue-hud_log.txt` or per KpLog naming).

## What it does
When NOT in combat mode, draws a green copy of the vanilla queue panel (4 slots: slot 0 big, 1-3 small) at the vanilla `LBL_QUEUE0..3`
extents, for party member 0. The vanilla red panel still handles combat. Companion strips near the portraits are a later iteration.

## Design
- Hook H8 `0x0074C168` (12 B `8B 8D 94 FE FF FF 8B 91 04 A9 00 00`), detour `QueueHudCapture([ebp-0x16C] = hud)`: stores the hud,
  combat = `([hud+0xA904]>>1)&1` (an unreadable value counts as combat), tick. Read-only.
- Draw: PortraitSlots `K2PS_HUD_DRAW` prio 400. Show rule `qh::shouldShow`: enabled, !combat, !inMenu, capture < 250 ms, >= 1 slot.
- Data: `party::serverCreature(member 0)` (same chain as vanilla `0x74DC80`, RE.md §3). Queue = inline CExoLinkedList at `cr+0x100`
  (cap 16). Round `R=[cr+0x10DC]`: list `[R+0xB00]` (pointer, cap 4), current `[R+0xB18]`.
- Slots: `qh::buildSlots` (QueueHudLogic.h). Mirrors vanilla, with these deviations:
  - 0x3F shows without vanilla's `[R+0xA84] && [R+0xB24]` gate; that gate is why nothing shows out of combat.
  - Only the first 0x3F expands the round list, and a node equal to `cur` is deduped.
  - A 0x3F with no nodes and no `cur` shows nothing; vanilla would show `i_attack`.
  - Nodes with a blank icon take no slot.
- Icons: the engine's own `0x74E0B0` (ECX ignored, 6 args, `RET 0x18`), cached per key. Type 0x0F spell id is pre-validated against
  `[[0xA1B4D0]+0x104]` {count, data}, because the engine's 0x0F path does not NULL-check the row.
- Skipped icons compact: the next icon takes the next slot position (no empty big slot).
- Waiting: `paused || [R+0xAB8]>=1 || [R+0xAA8]!=0` dims slot 0 (alpha 0.6). Pause bit = `[[[[0xA1B4A4]+8]+4]+0x10078] & 2`
  (the plan's chain was missing the `+4`; confirmed from `0x51C720` / `0x538CD0`).
- INI `queue_hud.ini` (game dir, optional): `enabled, bg_rgb, bg_alpha, border_rgb, border_alpha, border_px (1-6), waiting_dim, waiting_alpha, log`.
- Log channels (KpLog): `main`, `cap` (hud ptr changes), `icon` (resolved / blank / bad spell id, once per key), `draw` (shown/hidden edges + slot0 rect + hudScale).

## Open / to check live
- Slot rects come from `guikit::getExtent` with no hudScale applied. Check the first `draw shown` line (slot0 rect, hudScale).
- Do hidden vanilla labels still report a valid extent out of combat? If not, the log says `extent invalid`.
- The atlas tool `check_atlas_windows.py` has no QH row. H8 was checked OK (no overlap) with a scratch driver only.
- `split_build/QueueHud.kpatch` (local, git-ignored) is needed by the Wine smoke.

## In-game checklist
1. Pause out of combat, queue a medpac: a green slot appears.
2. Unpause: it clears when the action fires.
3. Enter combat: green hides, red vanilla shows.
4. Two queued commands: two slots, slot 0 dimmed while waiting.
5. Log: `python3 scripts/kotor2_logs.py check <queue-hud log> --new`.
6. (0.2.0) Pause, give a companion a command (selfheal click or a Force power on them): a green strip appears LEFT of that companion's
   health bar, not on the leader's panel. Log: `draw strip k=1|2 shown n=.. types=[..] rect=(..)`.
7. (0.2.0) Strip clears when the companion's action fires; both companions queued = two strips, neither overlapping a card, the
   selfheal button or the leader's bars.
8. (0.2.0) Combat or a menu: strips hide with the leader panel (`strip k=.. hidden`).

---

## 2026-10-08 Polish Additions (doc 33 workstreams QH4, QH5)

1. **QH4: Force Power & Unified Non-Combat Action Queue Buffering:**
   - **Hook & Calling Convention:** Detour `CSWCCreature::PerformCastSpell` at `0x00775703` (`83 ec 0c 89 4d f4`, consumed exit `0x0077574C`, `RET 0x1C`, `__thiscall` on client creature with 7 DWORD stack arguments = spellId, arg2, targetId, multispell, metamagic, domain, pos).
   - **QH4a: Round-Node Busy Gate (`haveCur` / `nrn` Idle Check):**
     - Crucial discovery: `s.engineNe` can drop to 0 while `s.haveCur` is still running the 3000 ms round node (`subType == 9` spell or `10` item). Dispatching while `haveCur` is true causes server `ActionManager(1)` -> `ClearAllActions(1)` to wipe the in-flight action!
     - `qh::shouldDispatchNext` must strictly require `engineQueueLen == 0 && !haveCur && nrn == 0` so creatures are 100% idle before the next action fires.
     - `actionInQueue` must detect both `subType == 9` (spell) and `subType == 10` (item), as well as action types `0x0F` (`T_CAST_SPELL`) and `0x2E` (`T_USE_ITEM`).
   - **QH4b: Unified `QueuedAction` Buffer:**
     - Replace `ItemBuffer` with `ActionBuffer` storing `QueuedAction` union for both `ActionKind::Item` and `ActionKind::Spell`.
     - Sequential FIFO dispatcher calls `AddrPerformCastSpell` (`0x00775700`) or `AddrPerformUseItem` (`0x00775600`).
   - **QH4c: Spell Icon Resolution in `QueueHud.cpp:174`:**
     - `AddrIconResolver` (`0x0074E0B0`) reads spell ID from `fakeAction[14]` (+0x38), matching items. Expand `fakeAction` to set `fakeAction[0] = 0x0F` and `fakeAction[14] = spellId` so queued spells render their `spells.2da` icons.
   - **QH4d: Follower AI Re-Inject Cap:**
     - Follower AI (`AIActionFollowLeader` 0x3D) cancels follower casting when leader moves. Cap re-injections to max 2 attempts to prevent infinite stutter loops when leader sprints away.
   - **QH4e: Combat Transition Hand-off (QL1, 0.5.0-dev; supersedes the old "cleared on combat entry" rule):**
     - At the combat rising edge the queued commands are handed to the engine, one per frame (`HANDOFF` / `HANDOFF_ONE`). Order: the active command first if it is no longer in the engine, then the buffer in FIFO order.
     - Guard per hand-off: `nrn + 1 <= 4` (engine round list cap is 4; one round node runs per ~3000 ms round) and the engine is not mid-flight. After hand-off, clicks go straight to vanilla.
     - The vanilla red combat bar shows the engine's round nodes. Our leader panel and Cancel stay hidden in combat. `show_in_combat` applies to the companion strips only.
     - Companion (m=1,2) hand-off is best effort.
     - `handoff_on_combat=0`: the old behaviour applies (`DROP why=combat_clear`, once per member).
2. **QH5: Cancel Queue Button Color Parity:**
   - Harmonize Cancel button border and fill tints in `cwLookLeader` with `lbl_miqueue_teal` palette: border `{ 0.067f, 0.514f, 0.404f }` (hover `{ 0.10f, 0.72f, 0.57f }`), dark translucent fill `{ 0.00f, 0.00f, 0.00f }` (alpha `0.70f`).

---

## 2026-10-11 Queue-fix QL1 (0.5.0-dev): pause restore, combat hand-off, identity guard

Status: **implemented in source, NOT installed, NOT play-tested.** Replaces the QH4e "cleared on combat entry" rule above when `handoff_on_combat=1`.

### Pause restore (leader)
- A vanilla quickbar click wipes the leader's engine queue (main queue and round list) even when our hook swallows the call, so a paused queued item vanishes.
- Each wipe of an active command gets an **active-command verdict** (debounce 150 ms, min active 500 ms): `executed`, `unseen`, or `eaten` (wiped while paused or while the engine was busy).
- An `eaten` command is re-injected once the engine is idle. It is not re-injected while a foreign order (e.g. a right-click move) owns the engine; the queue survives a new move order. Max 6 restores, 250 ms apart. Give up after 20 s of unpaused waiting (`DROP why=reinject_exhausted` or `eaten_timeout`).
- An unpaused vanish that was never wiped is simply resolved (no loop).
- A leftover attack node (subType 1, aa8=ab8=0) is a ghost `cur`, not busy (`STALE_CUR` once per episode). Item/spell nodes (subType 9/10) with zero flags are stale after 1500 ms unpaused.
- `aa8`/`ab8` are sticky while paused and ignored then.

### Identity guard
- Slot k is not a stable creature: Tab / party rotation moves creatures between slots m=0/1/2.
- The tick matches each buffer's creature to its slot and swaps buffers (`IDENT_SWAP from=k to=j`).
- If the creature is in no slot, verdicts are skipped. After 20 s unpaused it drops (`DROP why=identity_lost`).

### Ini switches (`queue_hud.ini`)
All default to 1 in code; an absent key means 1.

| Key | 1 | 0 |
|---|---|---|
| `pause_reinject` | re-inject eaten paused commands | no re-inject |
| `handoff_on_combat` | hand the queue to the engine at combat entry | old behaviour: `DROP why=combat_clear`, once per member |
| `show_in_combat` | companion strips show in combat | companion strips hide in combat |

### New log lines (`queue-hud` log)
| Line | Fields |
|---|---|
| `VANISH` | `cmd= m= kind=executed\|unseen\|eaten_paused\|eaten_wait paused= exec= ne= round= cur= aa8= ab8= age=` |
| `REINJECT` | `... why=eaten_paused attempt=` |
| `HANDOFF` | `m= active= buffered= cmds=` |
| `HANDOFF_ONE` | `cmd= m= seq=i/n` |
| `STALE_CUR` | `m= sub= since= aa8= ab8= paused=` |
| `IDENT_SWAP` | `from= to=` |

