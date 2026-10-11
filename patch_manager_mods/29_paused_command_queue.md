# 29 Paused-command delay and missing "queued" indicator

Status 2026-10-04: static RE done (batches PQ1 + PQ2, Sonnet; lead spot-checks below). Probe `cmdq-probe` DESIGNED, not built.
Evidence: `29_paused_command_evidence/PQ1.md` (pause gate, command path, HUD panel), `PQ2.md` (post-unpause gates, cast/item timing).
Exe: Steam swkotor2.exe 6a522e71.

## Symptom (user report)
The game is paused out of combat, and the player orders a party member to use a consumable or cast a Force power. The action often starts
noticeably late after unpause, and nothing on screen shows that the command was queued.

## How a command flows (CONFIRMED)
1. Client sends the input (use item `0x879AF0` major 6 / minor 9; cast `0x87A5C0` minor 0x12). Pause does NOT buffer it. The server tick
   runs `ProcessReceivedFrames 0x703F30` every tick with no pause test, and the handler `0x65BE70` runs at once.
2. The handler queues the command at once, even while paused:
   - it adds one combat-round node (`AddSWItemSpellAction 0x591890` type 0xA / `AddSWSpellAction 0x591670` type 9) to round list `+0xB00`;
   - it appends one `AIActionCombat (0x3F)` node at the TAIL (`AddAction 0x53F7F0`);
   - it may call `ActionManager(4) 0x563C90` -> `ClearAllActions(1)` first. This happens only for the player's own object and only when `+0x520`
     (inCombat) allows it. The clear has opposite polarity for spells and items (PQ1 item 5).
3. Pause = bit 2 of `byte [CServerExoAppInternal+0x10078]` (`00538cda` read, `00538f27` write in SetPauseState `0x538ED0`).
   - Pause does NOT stop UpdateState, AIUpdate or RunActions.
   - Pause freezes the world timer, so dt = 0.
   - The `0x3F` handler `0x6D9500` returns 1 ("in progress", node goes back to the head) while paused (`006d957c PUSH 2` / `CALL 0x51C720` / `JNZ`).
4. After unpause, `0x3F` expands the round node into real actions (move-to-range, face, `0x0F` CastSpell / `0x2E` ItemCastSpell) with
   AddActionToFront.

## Why it starts late (ranked)
| # | cause | verdict | delay |
|---|---|---|---|
| D1 | One action per 3000 ms "round". `0x3F` holds while `[round+0xAB8] >= 1` (`006d96c0..006d96d3`, lead-checked). AB8 is set by the first cast/use fire (`005bb076`, `005b9447`) and cleared only by EndCombatRound (`0058ea53`). A cast/use OUT of combat still starts a round (`StartCombatRound 0x58E730`, length `0058e8da MOV [EAX+0xa9c],0xbb8`). | CONFIRMED code | up to ~3000 ms of UNPAUSED time after the previous cast/use |
| D2 | Busy flag. `0x3F` holds while `[round+0xAA8] != 0` (`006d9554..006d9567`, lead-checked), i.e. the previous action's SetRoundPaused window (cast 1500 ms, item 1331 ms default). | CONFIRMED code | up to ~1.5 s |
| D3 | Round time freezes with pause. `UpdateCombatRoundTimer 0x571030` returns while bit 2 is set. Queuing a 2nd/3rd command while paused does not use up the round, so the wait runs entirely after unpause. Matches "paused, then gave orders". | CONFIRMED code | D1 + D2 shifted fully past unpause |
| D4 | 8 s combat tail. While `+0x520 == 1` (8000 ms after the last fight, `+0x524`), the handler skips ActionManager and the Add*Actions skip their clears. The new `0x3F` lands behind whatever is queued. The engine still counts this as "in combat" although it looks out of combat. | CONFIRMED code, reach unknown | the length of the old queue |
| D5 | Follower starvation. A non-controlled follower's FollowLeader `0x3D` (returns 1 or 3 only) stays at the head, so a tail `0x3F` never runs until something clears the queue. Only applies if the actor is not the player's object, which the vanilla UI path never does (the actor is the controlled character). | CONFIRMED code, reach LOW | unbounded |
| D6 | Orphan round nodes. ClearAllActions never empties the round list `+0xB00`. A wiped command leaves its round node, and the next `0x3F` runs that node first, then waits D1. | CONFIRMED mechanism, reach unknown | +1 round |
| - | Cast/item handler timing (spell fires at conj 170 ms, item at 1 ms; facing is instant) | REFUTED as cause | < 0.2 s |

Medpacs and Force powers both use 170/1330/0 conj/cast/catch in spells.2da (rows 64-66, Force rows 8, 10, 17, ...).

## Why nothing shows "queued" (CONFIRMED)
- The client queue panel (`0x74DC80`, called from HUD Update `0x74B3F0` at `0074c181`) is gated by bit 1 of `[CSWGuiMainInterface+0xA904]`
  (`0074c16e`, lead-checked). That bit is set every frame from `GetCombatMode()` = `[CClientExoAppInternal+0x320]` (`0x740860`).
- Combat mode is on only while a party member has a live target in `+0x550/+0x564/+0x568` (`0x79D840`). Out of combat the panel is hidden.
- The panel reads party slot 0's server queue (`+0x100`) directly: 4 slots, types 0x3F/0x0C/0x0F/0x2E.
- The server mirror `UpdateActionQueue 0x588ED0` (>= 300 ms, Exile only, `00569efb`) never lists the `0x3F` placeholder.

## Probe design: `cmdq-probe` 0.1.0 (log only, KpLog.h)
### Status (HK1, 2026-10-11)
- BUILT, NOT INSTALLED. Batch 1 = 11 windows (SetPauseState 0x538ED0, AddCastSpellActions 0x565310, AddItemCastSpellActions 0x5648D0, cast fire 0x5BB07C, item fire 0x5B9414, StartCombatRound 0x58E730, EndCombatRound 0x58EA53, ClearAllActions 0x541080, ReequipResumeAttack 0x5DF920, PartyMenu item handler 0x77C780, CanUseItem exit 0x5D32AE). Source: patch_manager_mods/cmdq_probe_src; log checker scripts/cmdq_probe_log_check.py.
- New channel `med`: `MED site=entry` (0x77C780: ecx this, `[esp+4]` handle, `[esp+8]`) and `MED site=canuse` (0x5D32AE, 7 bytes `5E 8B E5 5D C2 0C 00`; read `eax` = real result, item `[ebp+8]`). NOT 0x5D32AB: `JMP 0x5D32AE` at 0x5D2F99 (null-item return) lands inside a 0x5D32AB span; and `[ebp-4]` is 1 on that null path while eax is 0.
- New channel line `REEQ actor=` at 0x5DF920 (AIActionType47_ReequipResumeAttack, cold per 0x47 node). Finding: AddAttackActions (0x568540) never re-adds 0x3F (all 12 callers pass a8!=0 and it re-adds 0x0C); 0x3F is re-added in AddCutsceneAttackActions 0x5683D0 and sibling Add*Actions.
- Held for batch 2 (warm): gate 0x6D9500 / 0x6DA49F, mode 0x563C90, AIActionCombat clear+attack sites 0x6D9931 / 0x6D996B.
- Install = full KPatchLauncher reinstall of cmdq-probe (needs the user's GO + KPM Patches/CmdqProbe mirror + pre_install_gate.sh --mod cmdq_probe_src).

Standalone patch (template: `Patches/MouseProbe`), `KP_LOG_PATCH("cmdq-probe")`, log `cmdq-probe_log.txt` via the kpatch.log service.
All windows below were checked by script in PQ1/PQ2 (boundary, len >= 5, no relative branch, no inbound target, no overlap with live hooks).
They are plain detours (stolen bytes run after the handler); do not write flags. `0x5D6A60` is NOT used: companion-ai's RAR probe owns it.

| channel | hook | addr | len | stolen bytes | reads | cost |
|---|---|---|---|---|---|---|
| `pause` | SetPauseState entry | 0x538ED0 | 9 | `55 8B EC 81 EC C4 00 00 00` | `[esp+4]` mask, `[esp+8]` value; log only mask & 2 | cold |
| `cmd` | AddCastSpellActions entry | 0x565310 | 9 | `55 8B EC 81 EC 20 01 00 00` | ECX actor, spell `[esp+4]`, target p10, real flag p19 `[esp+0x48]` (0 = queue, 1 = build) | cold |
| `cmd` | AddItemCastSpellActions entry | 0x5648D0 | 9 | `55 8B EC 81 EC 10 01 00 00` | ECX actor, item `[esp+4]`, target `[esp+0x1C]`, real flag `[esp+0x2C]` | cold |
| `cmd` | ActionManager entry | 0x563C90 | 6 | `55 8B EC 83 EC 10` | ECX actor, mode `[esp+4]`, `+0x1116`, `+0x520` | cold |
| `gate` | AIActionCombat entry | 0x6D9500 | 5 | `55 8B EC 6A FF` | ECX actor; snapshot round AA8/AB8/A84/A94/A9C + pause bit | warm (per tick while a 0x3F is queued) |
| `gate` | AIActionCombat exit | 0x6DA49F | 10 | `8B 4D F4 64 89 0D 00 00 00 00` | EAX result 1/2/4 | warm |
| `fire` | cast fire | 0x5BB07C | 6 | `8B 85 7C FF FF FF` | actor `[EBP+0xFFFFFD48]` | cold |
| `fire` | item fire | 0x5B9414 | 6 | `8B 85 A0 FD FF FF` | actor `[EBP+0xFFFFFDA0]` | cold |
| `round` | StartCombatRound | 0x58E730 | 6 | `55 8B EC 83 EC 40` | ECX round, owner `[round+0xB04]` | cold |
| `round` | EndCombatRound AB8 reset | 0x58EA53 | 10 | `C7 81 B8 0A 00 00 00 00 00 00` | ECX round, A94/A9C | cold |
| `clr` | ClearAllActions entry | 0x541080 | 6 | `55 8B EC 83 EC 7C` | ECX victim, caller `[esp]`; log only party (`+0x11AC`) victims whose queue has 0x3F/0x0F/0x2E | warm (filter first) |

Real-flag offsets are settled by the lead. Cast `0x565310`: `RET 0x48` (18 args), flag = last arg, `0056531f CMP [EBP+0x4c],0` = `[esp+0x48]`
at entry. Item `0x5648D0`: `RET 0x2c` (11 args), `005648f4 CMP [EBP+0x30],0` = `[esp+0x2C]` at entry. Re-read every other arg offset from
the `.asm` before coding.

### Log contract
- `gate`: edge-triggered per actor. Log only when the hold reason changes (PAUSED / BUSY(AA8) / ROUND(AB8) / NOPEND / EXPANDED / DROPPED),
  with the ms spent in the previous reason. Never one line per tick.
- Command ledger: each queue-path call (`cmd`, real flag 0) opens `CMD#n` (actor tag, leader?, spell/item id, inCombat, combatExitMs, queue
  types, round state, paused). It closes on the actor's next `fire` or a `clr` that removes its `0x3F`. On close, print one line:
  `LAT CMD#n queued=<t> unpause=<t> expand=<t> fire=<t> held paused=<ms> busy=<ms> round=<ms> pend=<ms> outcome=fire|cleared|dropped`.
- `cmdq_probe.ini`: `party_only=1`, per-channel enable, `max_lines`.
- Add a `cmdq` parser to `scripts/kotor2_logs.py` that prints LAT lines and a hold-reason histogram.

### What each outcome means
- `held round` dominant -> D1/D3. Fix candidates: zero AB8/AA8 out of combat, or let the round timer run while paused.
- `held paused` only, short -> no engine bug, only the missing indicator.
- `cleared` -> D4/D6, the `clr` caller names the culprit.
- A `cmd` whose actor is not the leader and that never expands -> D5.

## Follow-up (not started)
- Indicator fix: in HUD Update, when pause bit 2 is set and slot 0's queue holds a `0x3F`, show the panel. The hook would be H8 `0x74C168`
  (12 B, `8B 8D 94 FE FF FF 8B 91 04 A9 00 00`), setting bit 1 of `[this+0xA904]` before the stolen reload. The action_hotbar mod already
  hooks `0x74B3F0` (same function, different window).
- Open questions: PQ1 (d) and PQ2 (d), especially which creature the hotbar's member path acts on.
