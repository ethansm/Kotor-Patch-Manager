# Party AI recasts active powers (Kreia Barrier) — findings 2026-10-25

Part of the companion combat AI overhaul (RFC `patch_manager_mods/new_issue_drafts/posted/issue_rfc_companion_combat_ai.md`).
Logs archived: `~/Games/KOTOR2_Mod_Workspace/logs_archive/20261025_104348_kreia_recast_20261025/`
(`cmdq-probe_log.txt`, `companion-ai_log.txt`, `buff-duration-hud_log.txt`). Kreia = actor `a9b88f0` (her own
`ClearAllActions` CLR, caller `0x66f392`, lands 7 ms after her `Clear 2300` trace line).

Tools (read the archived or live log; run with the repo venv for pykotor):
- `cmdq_family_recast.py`: same buff family cast again inside its base duration, per actor, queue (real=0) vs build (real=1).
- `cmdq_queue_vs_build.py`: for each build (real=1) CMD, compares the spell with the one the script queued that round.

## Facts

1. **Kreia re-casts Barrier while it is active.** 7 Barrier builds, 4 of them 3.0-7.6 s after the previous one
   (base duration 30 s). Valor too (6.0 s). Buff HUD shows `ip_frcbarrier01` on her slot at ratio 1.000 right after the
   T=315152 FIRE, so the effect was on her (the HUD maps the icon from the effect's spell id, `BuffDurationHUD.cpp:368`).
2. **The engine builds a different spell than the script queued.** cmdq-probe `CMD real=0` = queue
   (`AddCastSpellActions` 0x565310 from `ActionUseTalentOnObject`), `real=1` = build at round expansion (doc 29). Kreia:
   - T=311982 queued Barrier (0x87), built Battle Meditation (0x8a, queued at T=284619).
   - T=317991 queued Valor (0x16), built **Barrier**. T=323530 queued Speed Burst (8), built **Barrier**.
   - T=665585 and T=667113 the script's own "already active" test fired (trace `Spells.2DA ID = -1`), it queued the
     invalid talent, and the engine still built **Barrier** (T=665617, T=668632).
   So the vanilla script check (`k_inc_generic.nss:2655-2669`, exact spell id loop) can be correct and still lose:
   the cast that runs is a stale one from an earlier round. This matches "powers cast while already active" for every
   power, not only Barrier.
3. **Vanilla script gaps (independent of 2):**
   - Buff picker (Jedi Support, `GEN_TALENT_BUFF`, `k_inc_generic.nss:2520-2680`): tests only the exact spell id of
     the best tier known, not the family (a lower/higher tier from Force Chain or the PC is missed).
   - When the buff is already active it sets `tTemp = TalentSpell(-1)`; `GetIsTalentValid` is TRUE for it, so it
     `ClearAllActions` + casts the invalid talent (`id=ffffffff` in cmdq): a wasted round, and fact 2 then fires a
     stale cast.
   - Combo buff steps (`GN_GetNextTalentInCombo`, `k_inc_gensupport.nss:1083-1120` `GetCreatureTalentRandom(0xf8ff)`;
     `SW_COMBO_BUFF_ATTACK` 754-771; Fury 979-994) have no active test at all (doc R2 L2, never shipped).
   - Single-target debuffs (`k_inc_gensupport` targeted power picker, M4 area): no `GetHasSpellEffect` on the target
     anywhere in `k_inc_gensupport.nss`; only `GetIsDebilitated` (stun/paralyze/sleep/horror class) is tested in the
     target pick. Slow, Affliction, Wound, Choke, Fear, Plague, Drain etc. can be re-cast on an affected target.
4. Barrier's impact script is the third-party `m_imp_fpaligned` (mod143 alignment force powers, 51 powers routed to it
   in the live `spells.2da`); its Barrier branch mirrors vanilla (`Sp_RemoveRelatedPowers` / `Sp_BetterRelatedPowerExists`,
   resist amount scaled by alignment), applied directly (no `DelayCommand`), so it is not the cause.
5. Live `override/k_ai_master.ncs` md5 `b003ea41` = our party AI build (2026-10-01); none of its patches touch buff
   or debuff selection.

## CQ-QS1 play data (2026-10-25 17:18 session, F2 NOT installed)

Logs: `logs_archive/20261025_172733_cqqs1_kreia/`. Kreia = `a9b3480` (98/98 of her `Clear NNNN` trace lines match a
CLR of that actor), Bao-Dur = `a9b1138`. Tools: `cmdq_idle_casts.py`, `cmdq_rn_backlog.py` (this folder).

6. **The combat round's node list (QS `rn=`) is a FIFO cast backlog.** Each queue call (real=0) appends a spell node
   (sub 9) at the tail; the build (real=1) takes the head. Example L168-184: at `rend` Kreia has `q=` (empty action
   queue) but `rn=9:ffffffff,9:87`; she queues Speed Burst (`rn=9:87,9:8`); the next builds run `-1`, then Barrier.
   Kreia had 1-3 pending spell nodes at 29 of 50 queue calls (cmd0 depth {0:21, 1:16, 2:8, 3:5}); the other caster
   (`a9aa4d0`) 0 at all 9. A script `ClearAllActions` (CLR 0x66f392) on an actor with pending spell nodes left them in
   place 28 of 36 times: **ClearAllActions does not clear the round's cast nodes.** This answers O1's "why stale".
7. **Why only Kreia builds a backlog: her combat round script runs 2-3 times per round end.** 102 of her 108
   DetermineCombatRound runs are event 2003 (OnEndRound, slot 6); 28 of her 73 engine round ends were followed by 2
   runs, 3 by 3 runs, 20-40 ms apart; every run does `ClearAllActions` + a new cast (one more node). Engine site of the
   runs (companion-ai `PR RS site=`): S6A EndCombatRound 20, S6B AIUpdate fallback 18, **64 from no probed site**
   (most of the <100 ms pairs). Open: which caller (ExecuteScript router / wrapper) produces those 64.
8. **The "pause" the user saw = invalid casts.** 17 of Kreia's 46 builds were id `-1` (vanilla `TalentSpell(-1)`, fact
   3); one burst of 12 `-1` builds over 6.4 s at T=150.7 s. Her round ends after a `-1` come 1-3 ms apart (an invalid
   cast ends its round at once), which re-runs the script: the -1 loop. F2a (no -1 cast) removes this.
9. Recasts within duration this session: Speed Burst 2 (4.6 s, 6.0 s), Valor 1, Barrier 0 (was 4): fewer, but by
   chance of the random buff tree (nothing behavioural was installed). S1 stale builds 11.
10. O2 attack flood not reproduced: action queue length max 5 (enemies), 2 for party; Q1 0. Round attack nodes (sub 1)
   reach 3-4 for Bao-Dur and enemies; the queue HUD may be showing those round nodes. Needs a screenshot or the HUD
   row the user saw.

## Open questions

- O1 (RE, the main one): why does the round expansion build an older cast? Candidates: the script's `ClearAllActions`
  does not remove the pending combat-round cast node (0x3F) / the round's stored spell field, so the new node queues
  behind it; or something (our A1 requeue, queue-hud, action-hotbar) re-inserts it. Probe: log the action queue
  (ActQueue.h `RNode` spellId) of the actor at queue time and at build time.
  **ANSWERED 2026-10-25 (WI-135)** by facts 6-7 plus doc 29 PQ2 (`29_paused_command_evidence/PQ2.md` 4.3/4.4, X3):
  queue call `AddCastSpellActions` real=0 -> `AddSWSpellAction 0x591670` (site `0056538d`, its only caller) ->
  `CSWSCombatRound::AddAction 0x590270` (insert sorted by time, equal time after existing, cap 4). The round takes the
  head with `GetAction 0x590220` (only caller `AIActionCombat 0x6D9500` at `006d970d`) and builds it with
  `AddCastSpellActions` real=1 at `006d9b60`. Script `ClearAllActions` = `CSWSObject::ClearAllActions 0x541080`, which
  only resets the 5 attack-data slots (`0x5908F0`) and never calls `RemoveAllActions 0x590430` (callers: app-level
  `CServerExoApp::ClearAllActions 0x51CB10`, `ClearAllQueuedCombatActions 0x51CBA0`, `AddAttackActions`, `SetCombatState(0)`,
  `OnApplySetStateInternal`, type 0x47, `0x65E670`). So every extra script run in one round leaves one more cast node.
  Guard sites (none is cmdq-probe's 0x565310 entry): F4 = `AddSWSpellAction 0x591670` entry (party owner: drop its
  pending sub-9 nodes before the insert; removal API only `RemoveLastAction 0x5904C0` = tail, so an unlink helper is
  needed); F1 = `AIActionCombat` after `GetAction` (return `006d9712`) or the build call `006d9b60` (skip a stale or
  family-active head node). Static facts: `engine_ai_atlas/tools/cg.py callers` (2026-10-25). Open: who runs Kreia's
  64 unattributed end-round script runs; companion-ai `PR RS` lines are per-subject throttled (`g_psubj`,
  `CompanionAI.cpp:1402`), so site attribution needs per-site counters, not lines.
- O2: attack flood ("queue filled with single attack actions"): same probe; count 0x3D/0x3F nodes per actor per round,
  and who appended them (script `ActionAttack` without a clear, companion-ai A1 self-requeue, engine auto-attack).
- O3 (research): group debuffs (Storm, Plague, Insanity, Mass Stasis, Scream, Crush Opposition): pick the AoE centre by
  "unaffected enemies in radius" instead of "enemies in radius"? Needs a cost/benefit study before a rule.
  STUDIED (`O3_study.md`, WI-140): only Jedi Support (style 13) reaches GN_FindAOETarget (plain count in 4 m, no
  affected check); vanilla roll-chain typos at gensupport:3235 (Insanity tests nHorror) and :3310 (Sonic Howl tests
  nKillDroid). Recommendation B (keep the centre, require >= 2 unaffected for Horror/Insanity/Stasis Field, fix both
  typos). Owner decision pending.

## Proposed fixes (ordered)

- F0 probe (DLL, log only): extend cmdq-probe with queue snapshots at `real=0` and `real=1` (spell ids in order) and
  the actor's effect spell ids for the cast family. Settles O1/O2. Cheap, reuses ActQueue.h + Effects.h readers.
  BUILT 2026-10-25 (WI-134): cmdq-probe 0.2.0 `ch_qs` (QS lines at cmd0/cmd1/fire/rend); checker flags S1 stale-build
  (archived log: 7 hits, all Kreia) and Q1 deep-queue. Not installed; play row CQ-QS1. Extra fact from S1: 6 of 7 builds
  run 9-67 us after the queue call (same engine call chain), so the queue call itself triggers the stale build (the
  7th, after a -1 skip, came 32 ms later).
- F1 engine guard (DLL, companion-ai, INI-gated): at the build call (`AddCastSpellActions` real=1) for a party member,
  if the spell is a friendly self buff whose family is already on the caster, or a single-target debuff whose family
  is already on the target, drop the cast (return without building) and log it. This is the baseline that covers
  every power and every script path (vanilla AI, combos, stale nodes), and does not touch player-issued casts
  (skip when the leader / player queued it; cmdq tags `real`, need a player-origin flag).
  F1a BUILT (WI-136): companion-ai G1 detour 0x6D9ADB -> 0x6D9B65 `CompanionAiCastGuard`, INI `cast_family_guard=1`:
  a non-leader party member's spell round node whose buff family (1-10) is active on it is skipped; log `G1 drop`.
  Host tests + build_check PASS. New hook = KPM reinstall (needs GO). F1b debuffs = WI-137.
- F2 script (party AI patch set, `ranged_ai_src/script/party_ai_patches.py`): one family table + `GN_PartyFamilyActive
  (nSpell, oTarget)` used by the buff picker, the combo buff steps (R2 L2a-c, extended to Barrier, Valor, Energy
  Resist, Inspire, Crush Opposition) and the single-target debuff picker; when the family is active return an invalid
  talent *without* `ClearAllActions`/cast, so the member falls through to another action instead of idling.
  BUILT 2026-10-25 (WI-138, 27373545): `GN_PartyPowerFamily` (10 buff + 5 single-target debuff families, spells.2da
  ids) + `GN_PartyFamilyActive` in `party_ai_fixes.nss`; F2a = buff picker (family check, no `TalentSpell(-1)` cast),
  F2b = `GN_CheckNonDroidForcePower` (every combo/targeted cast passes it: vanilla k_inc_generic 860/1070/1224), so
  k_inc_gensupport needs no edit. Full build PASS (64 anchors, 17 outputs, 0 swapped args), test
  `test_party_ai_patches.py`. Not installed (override, needs GO); play row CA-FG3. Note: F2 only reduces the
  script-side picks; the stale engine build (fact 2) still needs F1/F4.
- F3 group debuffs: after O3.
- F4 after O1: fix the stale-node root cause itself (clear the old cast node on script clear) if F0 confirms it.
  CONFIRMED by facts 6-7: at a party member's queue call (real=0), drop that actor's pending spell nodes (sub 9) from
  the round list before the new one is appended, so the newest script decision wins. Needs the round node list layout
  (append/consume functions) from static RE (WI-135 step 2).
  RE DONE (`F4_unlink.md`): the round list is a CExoLinkedList (round+0xB00 -> wrapper -> internal; count internal+8),
  FIFO; +0xB18 = in-progress action (never free it); owner creature round+0xB04. Unlink one node via 0x73A130, free via
  0x623320 + 0x9196FD. Hook 0x5917C1 (AddSWSpellAction, before AddAction). Steps pinned in WI-143 (ready).
- F5 (script, party): run DetermineCombatRound at most once per engine round end for a party member (local int stamp
  of the round, skip a second run within the same round), which removes the backlog source of fact 7.
  BUILT (WI-142): entry F5 in `party_ai_patches.py` at the top of GN_DetermineCombatRound: a non-leader party member
  skips a run within 0.15 s of the previous one (local number 22, cleared by DelayCommand; 3 skips in a row = stuck flag,
  run anyway), trace `Party F5: duplicate combat round run skipped`. Full build PASS. Not installed; play row CA-FG4.
- Scope: F2 (and F1/F4/F5) are party-wide (`IsObjectPartyMember`), not Kreia-only. F2 covers Default, Aggressive,
  Defense, Ranged, Stationary (via `GN_RunDefaultAIRoutine`), Support/Jedi Support and Boss targeted; not covered:
  grenades, area powers (`GN_RunBossAOEPowerRoutine`, O3).
