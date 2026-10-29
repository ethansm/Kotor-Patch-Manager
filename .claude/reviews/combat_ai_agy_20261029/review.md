# Combat AI & Companion Controls: Code Review & New Rule Findings

> [!IMPORTANT]
> All 4 existing unit/contract suites currently pass (`companion_ai`, `ranged_ai`, `test_party_ai_patches.py`, `companion_ai_logic_test`). However, auditing the newly added C++ hooks (`CompanionAI.cpp`, `RangedAI.cpp`) alongside the patched NWScript decision layer (`party_ai_fixes.nss`, `party_ai_patches.py`, `k_inc_generic.nss`, `k_inc_gensupport.nss`, `k_inc_force.nss`) exposed **7 issues in our existing mod changes** and **11 additional combat-AI rule bugs** that should be fixed.

---

## Part 1: Issues Found in Our Existing Combat-AI Mod Changes

### 1. `[HIGH]` `RangedAI.cpp` E-H1 (`0x6D83EA`) & E-H2 (`0x6D7C79`) Check `!isPC` (`+0x10E8`) Instead of Player-Controlled Creature
- **Location**: [RangedAI.cpp](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/RangedAI.cpp#L232-L303) (`RangedAiHold`, `RangedAiRetarget`), [RangedAiLogic.h](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/RangedAiLogic.h#L47-L92)
- **Cause**: Both hooks read `CrIsPC = 0x10E8`, which is permanently `1` on the Exile and `0` on all companions regardless of who the player currently controls (`GetPartyLeader` / `GetPlayerByControlledObjectId`).
- **Impact**:
  1. When the player directly controls a companion in stance 16 (Kiting) and clicks an enemy behind cover, `RangedAiHold` sees `!isPC == true` and cancels the player's manual attack order.
  2. When the player controls a companion and leaves the Exile to AI in stance 16, `isPC == 1` disables both `RangedAiHold` and `RangedAiRetarget` for the AI-driven Exile.
- **Fix**: Replace the `!isPC` gate in `RangedAI.cpp` with the same `controlledCreatureId()` check (`0x51C600` / party table leader slot) used in [CompanionAI.cpp](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/companion_ai_src/CompanionAI.cpp#L350-L359).

### 2. `[HIGH]` Force Enlightenment (`176`) Bypasses F1a (`G1`) and F2 (`GN_PartyFamilyActive`) via Force Chain
- **Location**: [CompanionAiLogic.h](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/companion_ai_src/CompanionAiLogic.h#L837-L875) (`castFamily`), [party_ai_fixes.nss](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_fixes.nss#L94-L202) (`GN_PartyPowerFamily`, `GN_PartyFamilyActive`)
- **Cause**: In [k_inc_force.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla_all/k_inc_force.nss#L5221-L5440), `FORCE_POWER_FORCE_ENLIGHTENMENT` (`176`) applies the caster's highest learned tiers of Speed, Armor, and Valor (and chains them to Kreia via `FEAT_FORCE_CHAIN`) inside `case 176`. The engine stamps all resulting effects with **`GetEffectSpellId == 176`**, not the Speed (`8/34/37`), Armor (`18/24/17`), or Valor (`22/33/36`) spell IDs.
- **Impact**: Whenever the Exile casts Force Enlightenment (and chains it to Kreia), `castFamily` and `GN_PartyPowerFamily` see no active effect in families 1, 2, or 3. Kreia (and an AI-driven Exile) immediately waste rounds recasting Speed, Armor, and Valor on top of active Force Enlightenment.
- **Fix**: Treat active effect `spellId == 176` (`FORCE_POWER_FORCE_ENLIGHTENMENT`) on the caster as satisfying buff families `1` (Speed), `2` (Armor), and `3` (Valor) in both `castFamilyActive` ([CompanionAI.cpp](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/companion_ai_src/CompanionAI.cpp#L964-L980)) and `GN_PartyFamilyActive` ([party_ai_fixes.nss](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_fixes.nss#L176-L202)), and map `176` itself so it is not recast while active.

### 3. `[HIGH]` Fear (`22`) and Hold (`23`) Families Omit Tier II/III Spells (`30, 31, 44`), Breaking Cross-Tier & Combo Debuff Guards
- **Location**: [CompanionAiLogic.h](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/companion_ai_src/CompanionAiLogic.h#L868-L872), [party_ai_fixes.nss](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_fixes.nss#L156-L163), [party_ai_fixes.nss](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_fixes.nss#L224-L254) (`GN_PartyAOEUnaffectedCheck`)
- **Cause**:
  1. Family `22` only includes `Fear (16)` and omits `Horror (30)` and `Insanity (31)` (`GN_CheckSeriesFear`).
  2. Family `23` only includes `Stun (46)` and `Stasis (29)` and omits `Stasis Field / Sleep (44)` (`GN_CheckSeriesHold`).
  3. `GN_GetSeriesHold()` returns `44` (`FORCE_POWER_SLEEP`) for any Jedi with Stasis Field in combos (`SW_COMBO_JEDI_SMITE`, `SW_COMBO_JEDI_CRUSH`), and `GN_GetTargetedForcePower` rolls `Horror (30)` on single targets.
  4. `GN_PartyAOEUnaffectedCheck` ([party_ai_fixes.nss](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_fixes.nss#L244)) checks `GN_PartySpellEffectOn(nSpell, oNear)` (exact `nSpell` only) rather than whether the enemy is already debilitated (`GetIsDebilitated(oNear)`) or affected by the Fear/Hold family.
- **Impact**:
  - A companion will cast `Fear (16)` on an enemy already under `Horror (30)` or `Insanity (31)`, or cast `Stun (46)` / `Stasis (29)` on an enemy already paralyzed by `Stasis Field (44)`.
  - In Jedi Support, when `< 3` enemies are clustered (`GN_FindAOETarget` returns invalid), the single-target branch (`SW_BOSS_ATTACK_TYPE_NPC`) or combo fallback (`GN_GetSeriesHold`) casts `Horror (30)`, `Insanity (31)`, or `Stasis Field (44)` on an already-affected single target because neither `O3`, `F2b`, nor `G1` guards them there.
  - In `O3`, if Companion A casts `Stasis Field (44)` or `Insanity (31)` on a group, Companion B will still cast `Horror (30)` or `Stasis Field (44)` onto the already-paralyzed cluster.
- **Fix**:
  - Include `30, 31` in family `22` and `44` in family `23` for active-effect checks (`GN_PartyFamilyActive` / `castFamilyActive`), while keeping single-target cast guards (`G1`/`F2b`) vs AoE centre checks (`O3`) properly scoped (or check `!GetIsDebilitated(oNear)` in `GN_PartyAOEUnaffectedCheck`).
  - Also apply `O3` (or `GetIsDebilitated(oTarget)`) when `GN_GetAOEForcePower` is rolled on the single-target fallback path (`SW_BOSS_ATTACK_TYPE_NPC`).

### 4. `[MEDIUM]` F4 (`CompanionAiCastQueueNewest`) + G1 (`CompanionAiCastGuard`) Can Leave the Round Empty + Freed Pointer Reuse in `g_f4Seen`
- **Location**: [CompanionAI.cpp](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/companion_ai_src/CompanionAI.cpp#L982-L1080)
- **Cause**:
  1. `CompanionAiCastQueueNewest` (`0x5917C1`) unlinks and frees all older pending spell nodes (`>= 16 ms` old) when a new spell node is added, *before* G1 (`0x6D9ADB`) runs at round expansion. If a buggy script path queues a spell whose family is already active, F4 deletes the valid older spell node and G1 later drops the new one.
  2. When F4 frees an older action `a` via `FnOpDelete(a)` (line 1076), its address is not evicted from `g_f4Seen[32]`. If the allocator reuses the same small heap block within 16 ms for another creature's or subsequent `CSWSCombatRoundAction`, `f4AddedAt(act)` matches the freed pointer's timestamp and treats it as `sameRun` (suppressing F4 drop).
- **Fix**:
  1. Evict `a` from `g_f4Seen` (`g_f4Seen[k].act = 0`) right before `FnOpDelete(a)` in `CompanionAiCastQueueNewest`, and overwrite `g_f4Seen` timestamp when `f4Remember(cur, now)` sees an existing `act == cur`.
  2. In `CompanionAiCastQueueNewest`, if `g_cfg.cast_family_guard` is enabled and the newly queued `cur` node would be dropped by `G1` (buff/debuff family already active), either skip dropping the older valid node or reject the duplicate at queue time.

### 5. `[MEDIUM]` F5 (`SW_NUMBER_PARTY_DCR_STAMP = 22`) Drops Valid `oIntruder` Calls and Can Stick Across Area Transitions
- **Location**: [party_ai_patches.py](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_patches.py#L94-L110), [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L286-L299)
- **Cause**:
  1. F5 sets local number `22` to `1` and schedules `DelayCommand(0.15, SetLocalNumber(OBJECT_SELF, 22, 0))`. If `GN_DetermineCombatRound()` is called first with `oIntruder == OBJECT_INVALID` (e.g., 2003 round end) and `< 150 ms` later called with a valid `oIntruder` (e.g., 2006 `OnDamaged` retarget to a dangerous attacker or 2004 shout), F5 drops the call that carried `oIntruder`.
  2. If the `DelayCommand(0.15, ...)` is lost across an area transition or module load while `nPartyStamp == 1`, the companion skips its next **3** legitimate combat rounds (`nPartyStamp = 1 -> 2 -> 3 -> 4`), and at `4` sets `nPartyStamp = 1` again (if `DelayCommand` ever fails, 3 of every 4 DCR calls are skipped).
- **Fix**:
  1. Bypass the F5 skip when `GetIsObjectValid(oIntruder)` is `TRUE` (or only apply F5 when `oIntruder == OBJECT_INVALID`).
  2. Reset `SW_NUMBER_PARTY_DCR_STAMP` to `0` in `GN_PartyStanceCombatExit` / `GN_DoPostDCRChecks` and on `k_hen_spawn01` / `2001` heartbeat out-of-combat.

### 6. `[MEDIUM]` `O3` Drop in `GN_RunJediSupportAIRoutine` Skips Single-Target Support Powers and Drops `oIntruder`
- **Location**: [party_ai_patches.py](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_patches.py#L418-L429), [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L1109-L1154)
- **Cause**: In `GN_RunJediSupportAIRoutine`, `oTarget = GN_FindAOETarget()` runs at line 1109. When `oTarget` is valid (a 3+ enemy cluster exists) and `O3` drops the rolled AoE control spell because the cluster is already affected (`tUse = tO3None`), execution exits the `if (GetIsObjectValid(oTarget))` block without entering the `else` block (`GN_DetermineAttackTarget()` + `SW_BOSS_ATTACK_TYPE_NPC`), and falls through to `return GN_RunDefaultAIRoutine();` (which also omits `oIntruder`).
- **Impact**: Once an enemy cluster is stunned/horrified, a Jedi Support companion stops using single-target `SW_BOSS_ATTACK_TYPE_NPC` support powers and drops straight into melee/default combo routines.
- **Fix**: When `O3` drops `tUse` in `GN_RunJediSupportAIRoutine`, fall back to `oTarget = GN_DetermineAttackTarget(oIntruder)` + `GN_GetBossCombatMove(SW_BOSS_ATTACK_TYPE_NPC, ...)` (and pass `oIntruder` to lines 1057, 1126, 1153).

### 7. `[LOW-MED]` Wookiee Rage (`173..175`) and Force Aura/Shield/Armor Chaining Missing from Buff Family Checks
- **Location**: [party_ai_fixes.nss](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ranged_ai_src/script/party_ai_fixes.nss#L148-L152), [CompanionAiLogic.h](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/companion_ai_src/CompanionAiLogic.h#L860-L863)
- **Cause**: In [k_inc_force.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla_all/k_inc_force.nss#L4827-L4833), `FORCE_POWER_WOOKIEE_RAGE_I..III` (`173, 174, 175`) share the exact handler with `FORCE_POWER_FURY` (`164, 165, 166`), but family `9` only checks `164..166`.
- **Fix**: Add `173, 174, 175` to family `9` in `GN_PartyPowerFamily` and `castFamily`.

---

## Part 2: Additional Combat-AI Rule Issues Discovered in Vanilla / Patched Scripts

### 8. `[CRITICAL]` R2's Proposed `L2a` (`SW_FLAG_TARGET_FRIEND = TRUE`) Would Break `GN_RunDefaultAIRoutine` Because Line 941 Gates on `GetIsEnemy(oTarget)`
- **Location**: [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L668-L679) and [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L932-L955)
- **Cause**:
  - In `GN_GetNextTalentInCombo` ([k_inc_gensupport.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_gensupport.nss#L727)), `SW_FLAG_TARGET_FRIEND` is hardcoded to `FALSE`, so combo buffs (`SW_COMBO_BUFF_ATTACK`, `SW_COMBO_SITH_FURY`, `SW_COMBO_JEDI_FOCUS`, `SW_COMBO_BUFF_*`) are cast via `ActionUseTalentOnObject(tUse, oTarget)` with the **enemy** as `oTarget` (causing companions to walk toward an enemy to cast a self-buff).
  - **Trap in `R2_script_followups.md` (`L2a`)**: `R2` proposed setting `SW_FLAG_TARGET_FRIEND = TRUE` in `GN_GetNextTalentInCombo`. However, in `GN_RunDefaultAIRoutine`:
    - When `SW_FLAG_TARGET_FRIEND == TRUE`, `oTarget` is set to `OBJECT_SELF` (line 677) and enemy target selection (`else` at line 680) is skipped.
    - At line 935, `ClearAllActions()` executes.
    - At line 941, `if (GetIsTalentValid(tUse) && GetIsEnemy(oTarget))` evaluates **`GetIsEnemy(OBJECT_SELF)` -> `FALSE`**!
    - At line 947, `else if (GetIsEnemy(oTarget))` evaluates **`FALSE`**!
    - Result: `GN_RunDefaultAIRoutine` wipes the companion's actions with `ClearAllActions()` and returns `FALSE` without queueing anything!
- **Fix**: Keep `oTarget` as the enemy during `GN_RunDefaultAIRoutine` so `F2b` fallthrough to `ActionAttack(oTarget)` works when a buff is already active; at line 944, if `IsObjectPartyMember(OBJECT_SELF)` and `tUse` is a beneficial/self-buff talent (`GN_PartyPowerFamily` `1..10`), pass `OBJECT_SELF` to `ActionUseTalentOnObject(tUse, OBJECT_SELF)`.

### 9. `[HIGH]` Sticky `oLastTarget` in `GN_RunDefaultAIRoutine` Blocks `oIntruder`, Ignores Debilitated Targets, and Bypasses Defensive 5m Leash
- **Location**: [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L682-L775)
- **Cause**:
  1. Line 682 initializes `oTarget = tPR.oLastTarget`.
  2. In `NPC_AISTYLE_PARTY_DEFENSE` (lines 683-729), when no enemy is within 5m of the leader (melee) or the leader is not fighting (ranged), lines 725-726 clear `oClose` and `oIntruder` **but never clear `oTarget` (`tPR.oLastTarget`)**.
  3. Line 754 checks `if (IsObjectPartyMember(OBJECT_SELF) && GetIsObjectValid(oTarget) && !GetIsDead(oTarget) && GetObjectSeen(oTarget))` **before** `else if (GetIsObjectValid(oIntruder) && GetObjectSeen(oIntruder))` (line 760).
- **Impact**:
  1. **Broken 2006 / Shout Retargeting**: When `k_ai_master` case 2006 (`OnDamaged`) calls `GN_DetermineCombatRound(oAttacker)` because a dangerous new enemy just hit the companion for `> 25%` max HP, line 754 ignores `oIntruder` whenever `tPR.oLastTarget` is still alive and visible!
  2. **Broken Defensive Stance (`10`)**: A Defensive companion with a valid `oLastTarget` ignores the 5m-from-leader filter (melee) and leader-target filter (ranged) and chases `oLastTarget` up to 30m.
  3. **Glued to Stunned/Stasis'd Enemies**: Unlike `GN_DetermineAttackTarget` (which skips `GetIsDebilitated` targets after `M3`), line 754 has no `!GetIsDebilitated(oTarget)` check, so companions keep hitting a stunned/paralyzed target or casting follow-up combo steps on it while active enemies attack them.
- **Fix**:
  1. In `NPC_AISTYLE_PARTY_DEFENSE`, set `oTarget = oClose` (so `oTarget` respects the leader's target / 5m radius).
  2. Check `oIntruder` (when valid, seen, not dead, and not debilitated) **before** sticky `oTarget`, matching `GN_DetermineAttackTarget` (`k_inc_generic.nss:3588-3597`).
  3. If `GetIsDebilitated(oTarget)` is `TRUE` and `oClose` is valid, seen, and `!GetIsDebilitated(oClose)`, switch `oTarget = oClose`.

### 10. `[HIGH]` `GN_GetTargetedForcePower` and `GN_GetAOEForcePower` Add Lower Tiers Alongside Higher Tiers
- **Location**: [k_inc_gensupport.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_gensupport.nss#L3170-L3239) (`GN_GetAOEForcePower`), [k_inc_gensupport.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_gensupport.nss#L3427-L3506) (`GN_GetTargetedForcePower`)
- **Cause**: In KotOR II, learning Tier II/III of a Force power leaves `GetHasSpell(Tier I)` = `TRUE`. Both pickers test every tier with independent `if (GetHasSpell(...))` blocks instead of `else if` highest-tier selection:
  - `GN_GetTargetedForcePower` adds `Choke` + `Wound` + `Kill` (3 slots), `Affliction` + `Plague` (2 slots), `Push` + `Whirlwind` (2 slots), and `Scream I` + `Scream II` + `Scream III` (3 slots).
  - `GN_GetAOEForcePower` adds `Horror` + `Insanity` (2 slots), `Lightning` + `Storm` (2 slots), `Stun Droid` + `Destroy Droid` (2 slots), and `Scream I` + `Scream II` + `Scream III` (3 slots).
- **Impact**: High-level Jedi companions in Jedi Support cast Tier I `Wound`, `Force Push`, `Force Scream I`, `Horror`, or `Affliction` 50–67% of the time instead of their learned Master tiers (`Kill`, `Whirlwind`, `Master Force Scream`, `Insanity`, `Plague`), with lower DC and weaker effects.
- **Fix**: For party members (`IsObjectPartyMember(OBJECT_SELF)`), select only the highest known tier per power tree in `GN_GetTargetedForcePower` and `GN_GetAOEForcePower`.

### 11. `[HIGH]` Jedi Support Buff Picker (`GEN_TALENT_BUFF`) Rolls 1 Random Tree (`0..5` / `0..8`) Before Checking Known/Inactive Trees & Treats Crush Opposition (`144..149`) as a Self-Buff
- **Location**: [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L2678-L2853)
- **Cause**:
  1. `GEN_TALENT_BUFF` rolls `nSpellTree = Random(10000) % 6` (or `% 9` for `GetFirstPC()`) once. If the companion only knows 1–2 buff trees, or already has 1 buff active, `GEN_TALENT_BUFF` misses the known uncast buff 67–83% of the time and falls through without buffing.
  2. Tree 8 (`Fury`, `164..166`) is gated behind `GetFirstPC() == OBJECT_SELF` (so a Marauder companion or Hanharr with Rage spells `173..175` never rolls it).
  3. Tree 6 (`Crush Opposition I–VI`, `144..149`) is an **enemy AoE debuff** ([k_inc_force.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla_all/k_inc_force.nss#L4452-L4508)) that never places an effect on `OBJECT_SELF`. Checking `OBJECT_SELF` for effect `144..149` is always `FALSE`, so an AI-controlled Exile recasts Crush Opposition repeatedly even when all nearby enemies are already affected or are droids.
- **Fix**:
  - In `GEN_TALENT_BUFF` for party members, iterate/collect only candidate buff trees that the caster actually knows (`GetCreatureHasTalent`) and whose family is not yet active on `OBJECT_SELF` (`!GN_PartyFamilyActive`).
  - For Tree 6 (`Crush Opposition`, `144..149`), check that at least 1 organic enemy within 15m lacks the Crush Opposition family effect before selecting it.

### 12. `[MEDIUM]` Unshipped Combo Table & Droid Utility Bugs (`L1a-d`, plus Newly Found `L1e-g`)
- **Location**: [k_inc_gensupport.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_gensupport.nss#L567-L1192) and [k_inc_gensupport.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_gensupport.nss#L1960-L2092)
- **Cause**:
  - **`L1a`** (line 741): `SW_COMBO_SITH_ATTACK` step 1 calls `GN_GetSeriesScream();` without `nID =`, discarding the return value (`nID` stays `-1`).
  - **`L1b`** (lines 931-932): `SW_COMBO_JEDI_BLITZ` step 1 sets `nID = FORCE_POWER_FORCE_CRUSH; nFeat = TRUE;`, building `TalentFeat(163)` instead of `TalentSpell(163)`.
  - **`L1c`** (lines 1034, 1072): `SW_COMBO_MELEE_DISCIPLINED` and `SW_COMBO_RANGED_DISCIPLINED` ("USE 1 FEAT") test `if (nLocal == 1 || nLocal == 2)` (2 feats, identical to `AGGRESSIVE`).
  - **`L1d`** (line 2029): `GN_GetIsComboValid` has a duplicate `else if (nComboType == SW_COMBO_JEDI_CRUSH)` instead of `SW_COMBO_SITH_BRUTALIZE` (22), making combo 22 unreachable.
  - **`L1e` (NEW)** (lines 575, 581, 588): `SW_COMBO_JEDI_FOCUS` (`29`, Battle Meditation combo) is **unreachable** in `GN_GetStandardJediCombo` because `Random(25) + 1 + 3` maxes out at `28` (despite line 581 testing `nRand <= 26`) and `Random(15) + 14` also maxes out at `28`!
  - **`L1f` (NEW)** (line 777): `SW_COMBO_SITH_CONFOUND` (`//Sith Confound (FEAR, FEAT, FEAT)`) calls `GN_GetSeriesLightning()` instead of `GN_GetSeriesFear()`, leaving `GN_GetSeriesFear()` completely unused in the entire codebase.
  - **`L1g` (NEW)** (lines 1978-2087): `GN_GetIsComboValid` tests `GetCreatureTalentRandom(0x1104)` (melee feats only) for all 10 Jedi Force+Feat combos even though `GN_GetSeriesMeleeFeat()` supports ranged weapons (`0x1111`). Ranged Jedi companions fail validation for all 10 combos unless they also know a melee feat.
  - **`L3`** (lines 621, 3016): Party droids (`T3-M4`, `G0-T0`, `HK-47`) never use equipped droid arm utilities (`SW_COMBO_DROID_UTILITIES`) because line 621 checks `NPC_AISTYLE_JEDI_SUPPORT` (4) instead of `NPC_AISTYLE_PARTY_SUPPORT` (13) and line 3016 excludes `!GN_CheckNPCIsInParty`.

### 13. `[MEDIUM]` `GN_CheckNonDroidForcePower` Missing Drain Force (`153..155`), Force Breach/Suppress (`19, 47`), and Force Crush (`163`) Against Droids
- **Location**: [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L3257-L3283)
- **Cause**: `GN_CheckNonDroidForcePower` switches organic-only powers to a combat feat when `GetRacialType(oTarget) == RACIAL_TYPE_DROID`, but the list omits TSL's `FORCE_POWER_DRAIN_FORCE` (`153..155`), `FORCE_POWER_FORCE_CRUSH` (`163`), and `FORCE_POWER_FORCE_BREACH` / `SUPRESS_FORCE` (`19, 47`) used by `SW_COMBO_SITH_SCYTHE` and `SW_COMBO_JEDI_BLITZ`. Also, line 3286 always switches to a melee feat (`0x1104`) even if the caster is holding a ranged weapon (`GN_GetWeaponType() == 2`).
- **Fix**: Add `153, 154, 155, 163, 19, 47` to the droid exclusion list for party members, and pick `0x1111` when `GN_GetWeaponType() == 2` (and verify `GetIsTalentValid(tFeat)` instead of `GetIsTalentValid(tUse)` at line 3287).

### 14. `[MEDIUM]` Unshipped R1/R2 Movement, Weapon-Swap, & Perception Follow-ups (`M8`, `E4`, `FM2`, `P2`, `a1_retarget_strict`)
- **Locations**:
  - **`M8` (Ranged Support weapon-swap ping-pong)**: [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L3958-L4037) — `SW_NUMBER_HEALERAI_THRESHOLD` (local 8) decrements on every DCR call (shouts, damage, round-end) instead of only on `2003` (`OnCombatRoundEnd`) and when no melee enemy is within 5m.
  - **`E4` (Stuck `ACTION_MOVETOPOINT` recovery)**: [k_ai_master.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_ai_master.nss#L533) and [k_inc_generic.nss](file:///home/ethansm/Games/KOTOR2_Mod_Workspace/mod8_kite_script/src_party_ai/k_inc_generic.nss#L3821) — both heartbeat follow and `GN_CheckRangeFromLeader` return immediately whenever `GetCurrentAction() == ACTION_MOVETOPOINT`, so a companion whose move wedged on geometry never re-paths until manual control.
  - **`FM2` & `a1_retarget_strict` & `P2`** ([R1_engine_followups.md](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research/ai_research_2026-10-02/R1_engine_followups.md)):
    - `a1_retarget_strict`: `0x57C990` uses 3D surface-to-surface distance (`d3D - r_self - r_tgt < range`), while `AIActionPhysicalAttacks` tests 2D centre distance (`d2D <= range + 0.1`), causing A1 retargets near max range to silently end on the next tick.
    - `FM2` (`0x5C2438`): pathfinding failure returns code `3` (treated as success) and queues a `2.0–3.98s` wait without firing `OnBlocked`, looping on unreachable targets.
    - `P2` (`0x56BE79`): non-PC party members only run `UpdatePerception(0)` every `4000 ms` vs `200 ms` for `IsPC`.

---

## Summary Priority Matrix

| Priority | ID | Component | Issue Summary | Est. Effort |
|---|---|---|---|---|
| **P0** | **#1** | `ranged-ai` (`RangedAI.cpp`) | E-H1 / E-H2 check `!isPC` (`+0x10E8`) instead of controlled creature (`0x51C600`) | 15 min |
| **P0** | **#2, #3, #7** | `companion-ai` + `party_ai_fixes.nss` | Add Enlightenment (`176`), Horror/Insanity (`30, 31`), Mass Stasis (`44`), Wookiee Rage (`173..175`) to family tables & `O3` debuff check | 25 min |
| **P0** | **#4** | `companion-ai` (`CompanionAI.cpp`) | Evict freed action pointer from `g_f4Seen` in F4 + guard against F4 deleting valid cast when new cast is family-blocked | 20 min |
| **P1** | **#5, #6** | `party_ai_patches.py` (`F5`, `O3`) | Bypass F5 skip when `oIntruder` is valid + clear stamp on combat exit; fall back to single-target support when `O3` drops AoE | 20 min |
| **P1** | **#8, #9** | `party_ai_patches.py` (`DefaultAI`) | Self-target combo buffs at `ActionUseTalentOnObject` without breaking `GetIsEnemy(oTarget)`; fix `oIntruder` priority, Defensive 5m filter, & debilitated sticky target | 30 min |
| **P2** | **#10, #11, #12, #13** | `party_ai_patches.py` (`gensupport` / `generic`) | Highest-tier power selection in Targeted/AoE pickers, smart `GEN_TALENT_BUFF` tree filter + Crush Opposition check, `L1a-g` combo fixes, `L3` droid utilities, `M8`/`E4` | 45 min |
| **P3** | **#14** | `companion-ai` (`CompanionAI.cpp`) | `a1_retarget_strict` 2D range check, `FM2` pathfind-fail wait skip, `P2` perception cadence | 45 min |
