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

## Open questions

- O1 (RE, the main one): why does the round expansion build an older cast? Candidates: the script's `ClearAllActions`
  does not remove the pending combat-round cast node (0x3F) / the round's stored spell field, so the new node queues
  behind it; or something (our A1 requeue, queue-hud, action-hotbar) re-inserts it. Probe: log the action queue
  (ActQueue.h `RNode` spellId) of the actor at queue time and at build time.
- O2: attack flood ("queue filled with single attack actions"): same probe; count 0x3D/0x3F nodes per actor per round,
  and who appended them (script `ActionAttack` without a clear, companion-ai A1 self-requeue, engine auto-attack).
- O3 (research): group debuffs (Storm, Plague, Insanity, Mass Stasis, Scream, Crush Opposition): pick the AoE centre by
  "unaffected enemies in radius" instead of "enemies in radius"? Needs a cost/benefit study before a rule.

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
- F2 script (party AI patch set, `ranged_ai_src/script/party_ai_patches.py`): one family table + `GN_PartyFamilyActive
  (nSpell, oTarget)` used by the buff picker, the combo buff steps (R2 L2a-c, extended to Barrier, Valor, Energy
  Resist, Inspire, Crush Opposition) and the single-target debuff picker; when the family is active return an invalid
  talent *without* `ClearAllActions`/cast, so the member falls through to another action instead of idling.
- F3 group debuffs: after O3.
- F4 after O1: fix the stale-node root cause itself (clear the old cast node on script clear) if F0 confirms it.
