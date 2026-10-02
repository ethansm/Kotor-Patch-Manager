# R2 script follow-ups (2026-10-02)

Static research only: nothing compiled, no game or Wine run, nothing written outside this file and `/tmp/claude-1000/r2_scratch/`. The proposed patches are not in `party_ai_patches.py`. "verified" means the quoted source lines prove it, "plausible" means it needs a runtime check.

**Paths.** `W` = `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script`. `src_party_ai/` = `W/src_party_ai/` (the live, patched output; every `file:line` below is a line of that copy unless a path says `vanilla/` or `vanilla_all/`). `nwscript.nss` = `W/vanilla/nwscript.nss` (CRLF, read with `grep -a`).

**How a patch is expressed (read from `party_ai_patches.py` + `build_party_ai_scripts.py`).**
- A patch is a tuple `(id, scope, file, old, new)` in the list `PATCHES`; `file` is one of `G = "k_inc_generic.nss"`, `S = "k_inc_gensupport.nss"`, `M = "k_ai_master.nss"`. The texts are written with LF and converted to CRLF by `crlf()`.
- `apply()` (patches.py:477) takes the vanilla file and applies the patches of that file top to bottom. `old` must occur exactly once in the text *at that moment* (vanilla plus the earlier patches), and is replaced whole by `new`.
- Build step 4, `lint_patches()` (build_party_ai_scripts.py:231), additionally checks every anchor on the *pristine vanilla* text: it occurs once, no two anchor spans overlap, and no anchor lies inside another patch's `new`.
- `kite_routine.nss` and `party_ai_fixes.nss` are appended to `k_inc_generic` only *after* `apply()` (build step_patch, `gen.rstrip(...) + routine + fixes`). So an anchor can never land in them, and code in the middle of the file cannot see the constants they define (`SW_NUMBER_KITE_COOLDOWN` is at k_inc_generic.nss:4170 but k_ai_master sees it, because it includes the whole file). The proposals therefore carry their own helper bodies and file-scope constants inside the `new` text, defined before first use, and need no change to `PARTY_PROTOS`.
- Party gate idiom (copy): `IsObjectPartyMember(OBJECT_SELF)` inline (patches.py E8 line 82 ff., M10b), or `if(!IsObjectPartyMember(OBJECT_SELF)) { vanilla }`; leader exclusion `GetPartyMemberByIndex(0) != OBJECT_SELF` (party_ai_fixes.nss:17). Trace idiom: `GN_MyPrintString("GENERIC DEBUG *************** Party X: ...");` (party_ai_fixes.nss:78, patches.py:88).

**What I ran to check the proposals** (scripts in `/tmp/claude-1000/r2_scratch/`: `r2_patches.py`, `harness.py`, `ident_check.py`, `fmt.py`, `inv.py`):
1. `harness.py` rebuilds the three sources from `vanilla/` with the real `party_ai_patches.apply` and `PATCHES`: `k_inc_gensupport.nss` and `k_ai_master.nss` come out byte-identical to `src_party_ai/`, and `k_inc_generic.nss` is a prefix of it (the rest is the appended routine and fixes). So `src_party_ai/` is current.
2. It then appends the 18 proposed patches after E10 and runs the builder's own two checks (pristine lint, sequential apply). All 18: count 1 on pristine, count 1 at apply time, count 1 on `src_party_ai/`; no overlap; no anchor inside any other patch's text. Table in each section.
3. `ident_check.py`: every identifier in the new text is declared before use (nwscript, an include, or the same patch text). Braces and parentheses balance in the three patched files. The block below each section is the exact text that round-trips through `exec` into the tuples `harness.py` used.
4. Not done: compiling with nwnnsscomp (forbidden). Constructs I rely on that already compile in the live build: ternary in an argument (E11, `(GN_GetWeaponType(OBJECT_SELF) == 1) ? 4.0 : 15.0`, k_ai_master.nss:785), `vector .x` (kite), `ActionDoCommand(GN_DetermineCombatRound())` (E11, k_ai_master.nss:786; E3, party_ai_fixes.nss:88). `%` is used by vanilla (`vanilla_all/a_master_teach.nss:161`).

Integration: append the entries of the chosen patch blocks to `PATCHES` after E10 (their order does not matter; `lint_patches` works on pristine text). `expect` in `step_patch` counts them automatically. Optionally add `b"Party E4:"`, `b"Party M8:"`, `b"Party E9:"` to `need` in `step_static` (build_party_ai_scripts.py:430).

## Summary table

| item | recommendation | local(s) used | marker | risk |
|---|---|---|---|---|
| E4 stuck-move timeout | DO (heartbeat-based: E4a + E4b) | **23** (new, packed byte) | `Party E4: stuck move ...` | low-medium: a false positive only clears one move; legit move that stands still 9+ s (rare) |
| E7 follow distance in 5 m hooks | **DON'T** restore follow there; optional trace probe (E7a, E7b) | none | `Party E7 probe: enter5m/exit5m ...` | none for the probe; the real restore would cancel kite/E3/E11 moves (verified) |
| M8 swap counter | DO, low priority (M8a + M8b + M8c; M8d optional) | 8 (existing, semantics only) | `Party M8: round end, melee mark now N`, `Party M8: melee mark clear, switch back ...` | low; a never-approach companion stays in melee config about 2 rounds |
| L1 combo table | DO, party only, **ship with L2** | none | `Party L1: ...`, `Starting Jedi Combo SW_COMBO_JEDI_FOCUS` (existing) | low |
| L2 combo buffs | DO (L2a + L2b + L2c; the flag alone is not enough) | boolean 84 (existing `SW_FLAG_TARGET_FRIEND`) | `Party L2: self buff, spell N`, `Party L2: buff N already active, step skipped` | low-medium: `GetHasSpellEffect` on Battle Meditation / Fury unverified |
| L3 party droids | DO, only stances 9, 10, 13 | none | `Party L3: party droid may roll a utility combo, stance N` | low; Ranged/Stationary/Kite deliberately excluded (never approach) |
| E9/L7 allied shout 15 | DO with tight guards, flag as behaviour change | none | `Party E9: ally X shouted 15, engaging Y` | medium: companions join fights the leader did not start |
| M5 matrix row | replace the row (below) | - | - | - |

Suggested batch: E4, M8a-c, L1 + L2 + L3, E9 (user decision), E7 probe optional. Dependencies: L1a needs L2a-c; L2c needs L2a-b; E4b needs E4a; E9b needs E9a.

Binding decisions, checked against each proposal: party-only (every patch is gated on `IsObjectPartyMember(OBJECT_SELF)`, or sits in a branch only party members reach; the NPC result is kept in L1, L2, L3 and M8a); vanilla Ranged/Stationary never approach (E9 answers only a seen enemy within 15 m for 11/12/16; L3 excludes 11/12/16; E4 clears a stuck move but never issues an approach; E7 not restored); Force powers only for healing (no healing or item code touched); E5 skipped (E4 and E7 do not touch player orders; E4 acts only after 3 heartbeats in one spot, and never on the leader); E10 on (untouched).

## Local-number inventory (index -> users, file:line)

**Engine range and width (verified, `swkotor2.exe.unpatched_vanilla`, md5 2f2d31e21acc..., `objdump -d -M intel`; read from the unpatched copy; the live exe was not disassembled):**
- `0x5E9310` GetLocalNumber: `5e931d: cmp DWORD PTR [ebp+0x8],0x20` then `5e9321: jl`; out of range returns 0; in range `5e932d: add eax,[ebp+0x8]`, `5e9330: mov al,BYTE PTR [eax+0x14]` (one byte).
- `0x5E9340` SetLocalNumber: `5e934d: cmp DWORD PTR [ebp+0x8],0x20`, `5e935b: mov cl,BYTE PTR [ebp+0xc]`, `5e935e: mov BYTE PTR [eax+0x14],cl`. So index 0..31, value stored mod 256 (I keep E4's byte below 128 to be safe about sign extension).
- `0x5E9210` GetLocalBoolean and `0x5E9260` SetLocalBoolean: `cmp DWORD PTR [ebp+0x8],0xa0` (bit index 0..159, five dwords at the start of the table; the number bytes start at `+0x14`). So boolean 84 is valid; the nwscript.nss comment "20 and 63" (lines 5147, 5152) is stale.
- Prototypes: `int GetLocalNumber( object oObject, int nIndex );` (nwscript.nss:5158), `void SetLocalNumber( object oObject, int nIndex, int nValue );` (:5164), `int GetLocalBoolean( object oObject, int nIndex );` (:5148), `void SetLocalBoolean( object oObject, int nIndex, int nValue );` (:5153).

**Users (numbers).** Sources searched: `src_party_ai/` (all 18 files; `inv.py src_party_ai`), all 638 `vanilla_all/*.nss` plus `vanilla/` (`inv.py vanilla_all`), the 279 `.nss` of the installed mods under `01_extracted/` (grep), and a partial literal-index scan of the 288 `.ncs` in the live `override/` (constants held in variables are invisible to that scan, so it is a lower bound).

| index | name / meaning | users (file:line) |
|---|---|---|
| 0 | WALKWAYS_CURRENT_POSITION; Skirmish "times not moved" | vanilla k_inc_walkways.nss:13,109; k_inc_generic.nss:1708,1715,1722 |
| 1, 2 | WALKWAYS_END_POINT, WALKWAYS_SERIES_NUMBER | k_inc_walkways.nss:14-15; k_ai_master.nss:60 (index 2) |
| 3 | SW_NUMBER_COMBO_ROUTINE | k_inc_gensupport.nss:21; k_inc_generic.nss:2907,2976,2984,2993,3013,3024,3034 |
| 4 | SW_NUMBER_COMBO_INDEX | k_inc_gensupport.nss:22,728,1179; k_inc_generic.nss:2906,2975,2983,2992,3012,3023,3033 |
| 5 | SW_NUMBER_LAST_COMBO | k_inc_gensupport.nss:23; compared as a zone value at k_inc_generic.nss:2043; k_ai_master.nss:163-165 (commented) |
| 6 | SW_NUMBER_ROUND_COUNTER | k_inc_gensupport.nss:24 (constant only, no user): treat as reserved |
| 7 | SW_NUMBER_COMBAT_ZONE | k_inc_gensupport.nss:25; k_inc_generic.nss:2043-2045,2280; vanilla k_inc_zone |
| 8 | SW_NUMBER_HEALERAI_THRESHOLD = "hit by melee" mark (stances 11/16), heal threshold, mine damage | k_inc_gensupport.nss:26; k_ai_master.nss:702; k_inc_generic.nss:1458,1631,3688,3875,3880,4036,4294; vanilla k_inc_utility.nss:3192,3202,3205 (on the leader) |
| 9 | SW_NUMBER_HEALERAI_PERCENTAGE; also the literal "firing position searches" counter | k_inc_gensupport.nss:27; k_inc_generic.nss:804-846,890 (literal), 1448,1488,3694,4079; k_inc_utility.nss:3193,3204 |
| 10, 11 | SW_NUMBER_COOLDOWN, COOLDOWN_FIRE | k_inc_gensupport.nss:28-29; k_inc_generic.nss:2953-2977; vanilla k_oei_hench_inc.nss, turret spawn scripts |
| 12, 13, 14, 16 | arena master "inviso Kreia" | vanilla a_master_*.nss, a_fire_learn.nss (another object) |
| 15 | special-attack damage | k_inc_generic.nss:1752,1804 |
| 16, 17 | For Mandalore (mod120) KEY1, KEY2 | `01_extracted/mod120_for_mandalore/.../source/Full/m_act_mandsummon.nss`, `m_htb_henchmand.nss`; set on the object tagged `Mand`, the Mandalore **companion** (`m_cnd_mandquest.nss:17` `GetObjectByTag("Mand")`) |
| 18, 19 | For Mandalore NARFOUND/DANFOUND (Lite: QUEST/POINTS) on `Mand`; True Sith Assassins and Mines Overhaul use 19 on their own creatures | `m_act_mandclans.nss`, `m_cnd_mandquest.nss`; `m_sith_user`, `k_trp_generic` |
| 20 | **Mod 8** SW_NUMBER_KITE_COOLDOWN (k_inc_generic.nss:4170; uses 4291,4312-4323,4441); For Mandalore "leaving" flag on the trooper tags `henchmand1-4` (non-party) | `m_cnd_mandcheck.nss`, `m_act_manddismis.nss` |
| 21 | **Mod 8** SW_NUMBER_KITE_STREAK (k_inc_generic.nss:4171; uses 4312-4317,4347-4365,4442); For Mandalore trooper position | `851nih_enter.nss`, `m_htb_henchmand.nss` |
| 22 | For Mandalore trooper command | `m_htb_henchmand.nss` |
| 29 | animation id, **set on party members** | vanilla k_inc_walkways.nss:283; k_oei_hench_inc.nss:270,296,347,436,465 (`AssignCommand(oPartyMember, SetLocalNumber(OBJECT_SELF, 29, ...))`) |
| 23-28, 30, 31 | **no user found** in any of the sources above | - |

Mod 8's 20 and 21 only run on party members (the kite routine returns for non-party). The Mandalore users of 20-22 are the trooper creatures tagged `henchmand1-4` (plausible: not party members; the mod source has no AddPartyMember or AddAvailableNPC call), so Mod 8 has no conflict today.

**Choice: index 23** for E4. It is inside the documented 12..28 window (nwscript.nss:5157), inside the engine's 0..31, free in every source above, and it was already the candidate in design_ai_script_fixes.md section 9.7. Indices 16-19 and 29 are taken on companions by installed mods and must not be used. No other proposal needs a new local: M8 reuses 8, L2 reuses boolean 84 (`SW_FLAG_TARGET_FRIEND`, k_inc_gensupport.nss:68; valid per the 0xA0 bound above). Spare for later: 24-28, 30, 31.

**Booleans.** Direct uses in the party files: only index 84 (k_inc_gensupport.nss:727 `SetLocalBoolean(OBJECT_SELF, SW_FLAG_TARGET_FRIEND, FALSE);`, k_inc_generic.nss:650 `GetLocalBoolean(...)`). Over all 638 vanilla scripts nothing ever sets 84 TRUE (inventory: two uses, one get, one set-FALSE). The `SW_FLAG_*` set (active block k_inc_generic.nss:98-130 ff.) goes through GN_Get/SetSpawnInCondition. No new boolean index is proposed.

## E4: stuck-move timeout

**Problem (verified).**
```
k_inc_generic.nss:3736-3743   (GN_CheckRangeFromLeader; callers 383, 411, 427, 438 = stances 5, 10, 11, 16)
    int nActionID = GetCurrentAction(OBJECT_SELF);
    if ( nActionID == ACTION_MOVETOPOINT )
    {
        //AurPostString( "Actively trying to move to leader.", 10, 34, 2.0f );
        //Leave alone
        return 1;
    }
k_ai_master.nss:532-536       (heartbeat 2001 follow gate)
    if(IsObjectPartyMember(OBJECT_SELF) &&  //Note that this check replaces GetIsObjectValid(oMaster)
       //GetCurrentAction(OBJECT_SELF) != ACTION_FOLLOW &&
       GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT &&
k_inc_gensupport.nss:371-372  (GN_GetIsFighting counts a move toward an enemy as fighting)
     (GetIsObjectValid(oMove) && GetIsEnemy(oMove)))
k_inc_generic.nss:4325        (kite: its own "let the walk finish", bounded: nCool counts down every call, lines 4320-4324)
```
A `MOVETOPOINT` that never finishes (the engine's post-combat movement lock, a blocked path) therefore blocks DCR for stances 10/11/16 beyond the leash and blocks the heartbeat follow for every stance, with no way out. Whether such stuck moves happen in play is plausible (events review E4, engine review FM1); the code path is verified.

**Design.** Observe at the heartbeat only, because a heartbeat is a fixed-rate clock (engine review: slot 0 every 3000-4199 ms, review_ai_engine.md:395; the events review says 6 s: unverified, so the limit is expressed in heartbeats). DCR calls come in bursts (2003, 2005, 2006, shouts), so they are no clock.
- `GN_PartyStuckMoveCheck(oEnemy)` runs first in the party branch of case 2001 (already inside `!AI_OFF && !GetSoloMode()` and `GetPartyMemberByIndex(0) != OBJECT_SELF`, k_ai_master.nss:528-531).
- It resets local 23 to 0 whenever the member is not on a `MOVETOPOINT`, or is in a conversation, not commandable, in spectator state or meditating.
- Otherwise it packs `repeats * 32 + cell` (cell = 1..31, a hash of the 2 m grid cell of the position; byte stays below 128). Same cell as the previous heartbeat: repeats + 1; different cell: repeats = 0 (a moving member never accumulates; a hash collision needs `STUCK_MOVE_LIMIT` consecutive hits, about 1 in 30000).
- At `STUCK_MOVE_LIMIT` (3) repeats, i.e. the 4th heartbeat in the same cell (9-12.6 s at 3.0-4.2 s): clear local 23 and the kite cooldown (local 20), `ClearAllActions()`, then `ActionFollowLeader()` when no fight (the heartbeat's own test `!GN_GetIsFighting(OBJECT_SELF) && !GetIsObjectValid(oEnemy)`, k_ai_master.nss:548), else `GN_DetermineCombatRound()` (a heartbeat-side DCR call has precedent: case 2002, k_ai_master.nss:589,608).
- `GN_CheckRangeFromLeader` is left alone: the heartbeat recovers the stuck move, and the DCR then sees a normal state.

Prototypes relied on (nwscript.nss): `int GetCurrentAction(object oObject=OBJECT_SELF);` (:4535), `vector GetPosition(object oTarget);` (:2195), `int FloatToInt(float fFloat);` (:3240), `int IsMeditating( object oCreature );` (:5817), `int GetIsConversationActive();` (:5248), `int GetCommandable(object oTarget=OBJECT_SELF);` (:2873), `void ActionFollowLeader();` (:5399), `void ClearAllActions();` (:2087), `int GetSoloMode();` (:4261), plus the Get/SetLocalNumber lines above.

**Patch block** (`E4a` adds the helper and its two file-scope constants before `main`; `E4b` calls it):
```python
    ("E4a", "party", M,
     "void main()\n"
     "{\n"
     "    int nEvent = GetRunScriptVar();\n",
     "//Party AI fix E4 (party only): stuck-move recovery. ACTION_MOVETOPOINT is \"leave alone\" for the heartbeat below and\n"
     "//for GN_CheckRangeFromLeader, so a move that never completes (the engine's post-combat movement lock, a blocked path)\n"
     "//freezes the member for good. Local 23 (free, see R2 report) packs one byte (kept below 128): repeats * 32 + position cell (1..31).\n"
     "//Called from the heartbeat; counts only heartbeats on which the member is on a MOVETOPOINT in the same 2 m cell as at\n"
     "//the previous heartbeat. At STUCK_MOVE_LIMIT repeats the queue is cleared: follow the leader when no fight is on, else\n"
     "//a fresh combat round. Returns TRUE when it recovered the member.\n"
     "int SW_NUMBER_STUCK_MOVE = 23;\n"
     "int STUCK_MOVE_LIMIT = 3;\n"
     "int GN_PartyStuckMoveCheck(object oEnemy)\n"
     "{\n"
     "    int nPacked = GetLocalNumber(OBJECT_SELF, SW_NUMBER_STUCK_MOVE);\n"
     "    if (!IsObjectPartyMember(OBJECT_SELF) || GetPartyMemberByIndex(0) == OBJECT_SELF ||\n"
     "        GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT || GetIsConversationActive() || !GetCommandable() ||\n"
     "        GN_GetSpawnInCondition(SW_FLAG_SPECTATOR_STATE) || IsMeditating(OBJECT_SELF))\n"
     "    {\n"
     "        if (nPacked != 0)\n"
     "        {\n"
     "            SetLocalNumber(OBJECT_SELF, SW_NUMBER_STUCK_MOVE, 0);\n"
     "        }\n"
     "        return FALSE;\n"
     "    }\n"
     "    vector vPos = GetPosition(OBJECT_SELF);\n"
     "    int nCell = FloatToInt(vPos.x / 2.0) * 7 + FloatToInt(vPos.y / 2.0);\n"
     "    if (nCell < 0)\n"
     "    {\n"
     "        nCell = -nCell;\n"
     "    }\n"
     "    nCell = (nCell % 31) + 1;\n"
     "    int nRepeat = 0;\n"
     "    if (nPacked % 32 == nCell)\n"
     "    {\n"
     "        nRepeat = nPacked / 32 + 1;\n"
     "    }\n"
     "    if (nRepeat < STUCK_MOVE_LIMIT)\n"
     "    {\n"
     "        SetLocalNumber(OBJECT_SELF, SW_NUMBER_STUCK_MOVE, nRepeat * 32 + nCell);\n"
     "        return FALSE;\n"
     "    }\n"
     "    SetLocalNumber(OBJECT_SELF, SW_NUMBER_STUCK_MOVE, 0);\n"
     "    SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, 0);\n"
     "    if (!GN_GetIsFighting(OBJECT_SELF) && !GetIsObjectValid(oEnemy))\n"
     "    {\n"
     "        GN_MyPrintString(\"GENERIC DEBUG *************** Party E4: stuck move (same cell for \" + GN_ITS(nRepeat) + \" heartbeats), follow leader\");\n"
     "        ClearAllActions();\n"
     "        ActionFollowLeader();\n"
     "    }\n"
     "    else\n"
     "    {\n"
     "        GN_MyPrintString(\"GENERIC DEBUG *************** Party E4: stuck move in a fight (same cell for \" + GN_ITS(nRepeat) + \" heartbeats), new combat round\");\n"
     "        ClearAllActions();\n"
     "        GN_DetermineCombatRound();\n"
     "    }\n"
     "    return TRUE;\n"
     "}\n"
     "\n"
     "void main()\n"
     "{\n"
     "    int nEvent = GetRunScriptVar();\n"),
    ("E4b", "party", M,
     "                    if(IsObjectPartyMember(OBJECT_SELF) &&  //Note that this check replaces GetIsObjectValid(oMaster)\n"
     "                       //GetCurrentAction(OBJECT_SELF) != ACTION_FOLLOW &&\n"
     "                       GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT &&\n",
     "                    //Party AI fix E4 (party only): count heartbeats spent on a MOVETOPOINT in one spot and recover a stuck move\n"
     "                    //(the gate below leaves every MOVETOPOINT alone).\n"
     "                    GN_PartyStuckMoveCheck(oEnemy);\n"
     "                    if(IsObjectPartyMember(OBJECT_SELF) &&  //Note that this check replaces GetIsObjectValid(oMaster)\n"
     "                       //GetCurrentAction(OBJECT_SELF) != ACTION_FOLLOW &&\n"
     "                       GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT &&\n"),
```

**Uniqueness.** Applied text = `vanilla/k_ai_master.nss` + earlier patches (E6, E13, E11, E10). `grep -cF 'void main()' vanilla/k_ai_master.nss` = 1. E4b anchors three lines; the commented copy in case 2014 has `//GetCurrentAction(...) != ACTION_MOVETOPOINT` commented, so the three-line block is unique.

| patch | file | pristine vanilla | at apply time | src_party_ai |
|---|---|---|---|---|
| E4a | k_ai_master.nss | 1 | 1 | 1 |
| E4b | k_ai_master.nss | 1 | 1 | 1 |

**Gate.** `IsObjectPartyMember(OBJECT_SELF)` and `GetPartyMemberByIndex(0) == OBJECT_SELF` inside the helper, plus the enclosing heartbeat gate. **Marker.** `Party E4: stuck move (same cell for 3 heartbeats), follow leader` and `Party E4: stuck move in a fight (same cell for 3 heartbeats), new combat round`.

**Test-matrix row.**

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| E4 | Any companion on a move that cannot finish: the "stands still after the fight while the leader walks away" lock, or an E3 stand-off move at an unseen sniper behind a wall. Control: the leader sprints 30+ m and the companions run to catch up. | After about 9-13 s the stuck companion follows the leader again (no enemy near) or plans a new combat round (fight on). The control run never triggers it. | `Party E4: stuck move (same cell for 3 heartbeats), follow leader` / `... stuck move in a fight ..., new combat round`; none during the control run |

**Risks.**
- A legitimate `MOVETOPOINT` that stands in one 2 m cell for 9+ s is cleared (a wait at a door the 2009 handler is still opening is shorter). The result is only ClearAllActions plus follow, the heartbeat's own action.
- kite: kite moves last about 2-3 s and never reach the limit; E4 zeroes local 20 on recovery so a stuck kite does not keep the cooldown. Local 21 (streak) untouched.
- E1: E1's exit leaves the member on `ActionFollowLeader` (action 38, not `MOVETOPOINT`), which resets local 23. E3/E11 stand-off moves that cannot complete are exactly what E4 frees (combat branch: `GN_GetIsFighting` counts a move toward an enemy as fighting). H1, H2, M1, M4, M10, E8 are inside DCR and untouched; the combat branch only calls DCR once.
- If the engine lock survives `ClearAllActions` the member re-sticks and E4 fires again every 9-12 s: no worse than today, and the trace shows it.

## E7: follow distance in the 5 m hooks

**Recommendation: do not put follow distance there.** Optional: ship the trace probe (E7a, E7b) to count the events.

**What the hooks are (verified, my reading of `objdump` output of the unpatched exe).** Engine function `0x7E6CE0` fires the scripts. Per party member from list index 1 on (the leader is index 0) that has a FollowInfo (`[creature+0x500]`) it compares the squared distance to the leader with the constant at `0x989ED0` and the member's stored flag `FollowInfo+0x38` (InSafetyRange):
```
0x989ED0 = 25.0 (double)                                   -> 5 m squared
7e77ab: fcomp QWORD PTR ds:0x989ed0 ... [ebp-0xbc] = (dist^2 <= 25.0)
7e77da: cmp [ebp-0xbc],0 ; jne 7e7850
7e77e3: cmp [ebp-0xb8],0 ; je  7e7850      ([ebp-0xb8] = FollowInfo+0x38)
7e77ec: push 0x9a2198        ("k_hen_exit5m") ... call 0x6fd8d0 ... 7e7838: mov DWORD PTR [edx+0x38],0
7e7850: cmp [ebp-0xbc],0 ; je  7e78c4
7e7859: cmp [ebp-0xb8],0 ; jne 7e78c4
7e7862: push 0x9a2188        ("k_hen_enter5m") ... call 0x6fd8d0 ... 7e78ae: mov DWORD PTR [edx+0x38],1
```
So both events are edge-triggered: `exit5m` fires once when a member goes from within 5 m to beyond 5 m (flag 1 to 0), `enter5m` once when it comes back (0 to 1). (FM4 re-fires them after a save: review_ai_engine.md:320-322.) Strings: `0x9a2188` = `k_hen_enter5m`, `0x9a2198` = `k_hen_exit5m` (found by scanning the exe, only these two xrefs).

**Why not (all verified from the quoted lines):**
1. **Edge-triggered, so it cannot do the job claimed.** The E7 argument is "any interruption that leaves a companion idle is fixed only by the next heartbeat". But a companion that is already beyond 5 m does not get another `exit5m` until it has re-entered 5 m. After a combat end, a stuck move or a door action while the member is far away, nothing fires. The heartbeat (k_ai_master.nss:523-557), E1 (post-fight follow: `GN_DoPostDCRChecks`, k_inc_generic.nss:556-593) and now E4 cover those cases.
2. **The vanilla `exit5m` body would break the move-based fixes.** Its guard `//GetCurrentAction(OBJECT_SELF) != ACTION_MOVETOPOINT &&` is commented out (k_ai_master.nss:880) and the only remaining test is "no attack/spell target" (lines 888-891). A kite step, an E3/E11 stand-off move or an E4 re-plan is a move with no attack target, so `ClearAllActions(); ActionFollowLeader();` (lines 896-897) would cancel it the instant the member crosses 5 m from the leader.
3. **The vanilla `enter5m` body unconditionally runs `SetCommandable(TRUE);`** (k_ai_master.nss:856), which would undo a script's `SetCommandable(FALSE)` lock (cutscene, dialogue wait). Its "join when the leader attacks" branch (lines 861-865) duplicates the shouts (patterns 1, sent at 2003/2005 round ends, k_ai_master.nss:633,679) and perception.
4. It touches the player's non-combat orders, the E5 area the user skipped.

A safe variant exists only in theory (exit5m: `ActionFollowLeader()` without `ClearAllActions`, when `GetCurrentAction() == ACTION_QUEUEEMPTY` and no fight), but a following member reports `ACTION_FOLLOWLEADER` (38, nwscript.nss:1507), so it would almost never fire. Not recommended.

**Optional probe (trace only, no behaviour).** Counts both edges with the member's action and distance, to confirm the above in play and to see the post-save re-fire:
```python
    ("E7a", "party (probe)", M,
     "        case 2013: //KOTOR_HENCH_EVENT_ON_ENTER_5m\n"
     "        {\n",
     "        case 2013: //KOTOR_HENCH_EVENT_ON_ENTER_5m\n"
     "        {\n"
     "            //Party AI probe E7 (trace only, no behaviour): how often the engine fires enter-5m, and in what state.\n"
     "            if(IsObjectPartyMember(OBJECT_SELF))\n"
     "            {\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party E7 probe: enter5m \" + GN_ReturnDebugName(OBJECT_SELF) + \" action \" + GN_ITS(GetCurrentAction(OBJECT_SELF)) + \" dist \" + FloatToString(GetDistanceBetween(OBJECT_SELF, GetPartyMemberByIndex(0)), 4, 1));\n"
     "            }\n"),
    ("E7b", "party (probe)", M,
     "        case 2014: //KOTOR_HENCH_EVENT_ON_EXIT_5m\n"
     "        {\n",
     "        case 2014: //KOTOR_HENCH_EVENT_ON_EXIT_5m\n"
     "        {\n"
     "            //Party AI probe E7 (trace only, no behaviour): how often the engine fires exit-5m, and in what state.\n"
     "            if(IsObjectPartyMember(OBJECT_SELF))\n"
     "            {\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party E7 probe: exit5m \" + GN_ReturnDebugName(OBJECT_SELF) + \" action \" + GN_ITS(GetCurrentAction(OBJECT_SELF)) + \" dist \" + FloatToString(GetDistanceBetween(OBJECT_SELF, GetPartyMemberByIndex(0)), 4, 1));\n"
     "            }\n"),
```

| patch | file | pristine vanilla | at apply time | src_party_ai |
|---|---|---|---|---|
| E7a | k_ai_master.nss | 1 | 1 | 1 |
| E7b | k_ai_master.nss | 1 | 1 | 1 |

Anchors: `case 2013: //KOTOR_HENCH_EVENT_ON_ENTER_5m` + `{` and the 2014 equivalent (the header comment at k_ai_master.nss:520-521 uses a different text). **Gate:** `IsObjectPartyMember(OBJECT_SELF)`. **Marker:** `Party E7 probe: enter5m <name> action <n> dist <d>` / `... exit5m ...`.

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| E7 | (probe only) Walk the leader away from and back to an idle companion a few times; then save and reload. | No behaviour change. One `exit5m` per departure past 5 m and one `enter5m` per return; possibly one of each pair right after the load (FM4). Settles the rate. | `Party E7 probe: exit5m <name> action <n> dist <d>`, `Party E7 probe: enter5m <name> ...` |

**Risks.** Probe: a string concatenation per edge event, only evaluated for party members; prints only when the trace layer is on (`GN_MyPrintString` is ShipBuild-gated).

## M8: swap counter

**Problem (verified).**
```
k_inc_generic.nss:3875-3881   (GN_CheckRangedPreferred)
    int nHitByMelee = GetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD);
    if ( nHitByMelee > 0 )
    {
        nHitByMelee--;
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, nHitByMelee);
k_ai_master.nss:696-703       (2005: every melee-ish hit)      SetLocalNumber( OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, 2);
k_inc_generic.nss:3839-3842 and 4369-4372   a swap returns 1, so no attack is issued on that call
```
The caller is stance 11 (k_inc_generic.nss:3837-3839, `nCurAI == NPC_AISTYLE_PARTY_RANGED`) and stance 16 (kite, line 4369); Stationary 12 never calls it. DCR runs from 2003 (one per round, k_ai_master.nss:621-635), from 2006 (lines 794-808 and the unseen-damager branch), from shouts when the member is idle (k_inc_generic.nss:2077) and from E3/E11 queued calls, so "2 rounds" is "2 calls". In a plain fight the 2003 call dominates, so the practical effect of per-call counting is smaller than the review's "less than one round" (2005's own DCR is gated by `!GetIsInCombat()`, k_ai_master.nss:719-722); extra calls shorten the melee period only when other callers fire inside a round.

**Design.** Count rounds: remove the per-call decrement for party members (M8a) and decrement once per `2003` for stances 11/16 (M8b), after that round's decision, so a hit leaves the member in melee config for the rest of that round plus about two more round ends. M8c adds the missing trace line for the swap back. M8d (optional) queues a combat round behind the swap to melee, because a swap round issues no attack.

**Patch block:**
```python
    ("M8a", "party context", G,
     "        nHitByMelee--;\n"
     "        SetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, nHitByMelee);\n",
     "        //Party AI fix M8 (party only): the melee mark is counted down once per combat round (k_ai_master case 2003), not\n"
     "        //once per call: calls also come from 2006, shouts and queued DCR calls, so the \"2 rounds\" could be less than one\n"
     "        //and the member flipped between its configs (each swap round issues no attack).\n"
     "        if(!IsObjectPartyMember(OBJECT_SELF))\n"
     "        {\n"
     "            nHitByMelee--;\n"
     "            SetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, nHitByMelee);\n"
     "        }\n"),
    ("M8b", "party", M,
     "                    Db_MyPrintString(\"GENERIC DEBUG *************** End of Combat Round: \" + GN_ReturnDebugName(OBJECT_SELF));\n"
     "                    SpeakString(\"GEN_I_WAS_ATTACKED\", TALKVOLUME_SILENT_TALK);\n"
     "                    GN_DetermineCombatRound();\n",
     "                    Db_MyPrintString(\"GENERIC DEBUG *************** End of Combat Round: \" + GN_ReturnDebugName(OBJECT_SELF));\n"
     "                    SpeakString(\"GEN_I_WAS_ATTACKED\", TALKVOLUME_SILENT_TALK);\n"
     "                    GN_DetermineCombatRound();\n"
     "                    //Party AI fix M8 (party only; stances using GN_CheckRangedPreferred: 11 and 16): one round is over, so the\n"
     "                    //\"hit in melee\" mark (local 8, set to 2 by 2005) counts down once, after this round's decision.\n"
     "                    if(IsObjectPartyMember(OBJECT_SELF) &&\n"
     "                       (GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_PARTY_RANGED || GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_PARTY_KITE) &&\n"
     "                       GetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD) > 0)\n"
     "                    {\n"
     "                        SetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, GetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD) - 1);\n"
     "                        GN_MyPrintString(\"GENERIC DEBUG *************** Party M8: round end, melee mark now \" + GN_ITS(GetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD)));\n"
     "                    }\n"),
    ("M8c", "party context", G,
     "                //AurPostString( \"Melee ----> Ranged\",20,28,2.0f);\n"
     "                ClearAllActions();\n"
     "                ActionSwitchWeapons();\n",
     "                //AurPostString( \"Melee ----> Ranged\",20,28,2.0f);\n"
     "                //Party AI fix M8 (trace marker): the swap back to the ranged config.\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party M8: melee mark clear, switch back to the ranged config\");\n"
     "                ClearAllActions();\n"
     "                ActionSwitchWeapons();\n"),
    ("M8d", "party context", G,
     "                //AurPostString( \"Ranged ----> Melee\",20,24,2.0f);\n"
     "                ClearAllActions();\n"
     "                ActionSwitchWeapons();\n",
     "                //AurPostString( \"Ranged ----> Melee\",20,24,2.0f);\n"
     "                ClearAllActions();\n"
     "                ActionSwitchWeapons();\n"
     "                //Party AI fix M8b (optional): carry on fighting once the swap is done (the swap round issued no attack).\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party M8b: swap to melee, combat round queued behind it\");\n"
     "                ActionDoCommand(GN_DetermineCombatRound());\n"),
```

**Uniqueness.**

| patch | file | pristine vanilla | at apply time | src_party_ai |
|---|---|---|---|---|
| M8a | k_inc_generic.nss | 1 | 1 | 1 |
| M8b | k_ai_master.nss | 1 | 1 | 1 |
| M8c | k_inc_generic.nss | 1 | 1 | 1 |
| M8d | k_inc_generic.nss | 1 | 1 | 1 |

(`grep -cF 'nHitByMelee--;' vanilla/k_inc_generic.nss` = 1; `grep -cF 'End of Combat Round' vanilla/k_ai_master.nss` = 1. M8c/M8d anchor the `//AurPostString(... "Melee ----> Ranged"/"Ranged ----> Melee" ...)` comment plus the two following lines; M1's anchor is next to M8d's but not overlapping.)

**Gate.** M8a: `!IsObjectPartyMember(OBJECT_SELF)` keeps the old decrement for any non-party caller (there is none today). M8b: `IsObjectPartyMember(OBJECT_SELF)` and the style test. **Marker.** `Party M8: round end, melee mark now N`, `Party M8: melee mark clear, switch back to the ranged config`, and (M8d) `Party M8b: swap to melee, combat round queued behind it`.

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| M8 | Ranged (11) or Kite (16) companion, blaster in config 1, melee weapon in config 2; a melee enemy that hits it once or twice, then switches to another target. | One swap to melee; it stays in melee config until about two round ends after the last hit, then one swap back. No blaster-melee-blaster flip inside a round. | `Party M1: hit in melee, switch to melee config 2`, then `Party M8: round end, melee mark now 1`, `... now 0`, then `Party M8: melee mark clear, switch back to the ranged config` |

**Risks.**
- The engine's never-approach rule (stances 11 and 16 via E-H3) means a companion in melee config can only hit adjacent enemies. Vanilla's early flip back (when extra calls shorten the count) hid that; with round counting the member stays in melee config about 2 round ends. If the melee enemy retreats the member idles with a melee weapon for that time. M8d does not change this.
- If `2003` does not fire (engine attack-loop A1 states), the mark stays at 2 until the engine zeroes local 8 at combat exit (`SetCombatState`, review_ai_engine.md:393) or a kite zeroes it (k_inc_generic.nss:4294). Bounded by the fight.
- M8d (optional): if `ActionSwitchWeapons` silently fails (equip lock), the queued DCR re-runs the same decision and loops at action speed. Not recommended until M8 shows lost rounds.
- Interaction: M1 (swap only to a melee config 2), M7/E6 (what counts as a melee hit) unchanged; the Remote's use of local 8 as a heal threshold is untouched (M8b is limited to stances 11 and 16); H1/H2/M4/M10/E1/E3/E8 not involved.

## L1: combo table

**Problem (verified, k_inc_gensupport.nss).**
```
575   nRand = Random(25)+1; //RWT-OEI 09/03/04 - Changed from 23->25            (1..25; 1-9 return melee combos, 10..25 -> +3 = 13..28)
581   else if(nRand >= 10 || nRand <= 26)   { nRand = nRand+3; }                  (always true)
741   GN_GetSeriesScream();                                                       (Sith Attack step 1: result discarded, nID stays -1)
931-932 nID = FORCE_POWER_FORCE_CRUSH;  nFeat = TRUE;                             (Jedi Blitz step 1 -> TalentFeat(177))
2022-2028 else if(nComboType == SW_COMBO_JEDI_CRUSH) { Hold + Push + Jump }
2029-2035 else if(nComboType == SW_COMBO_JEDI_CRUSH) { Choke + DrainLife + Push }  (duplicate; Sith Brutalize (22) is never valid)
```
- Battle Meditation: `SW_COMBO_JEDI_FOCUS = 29` (gensupport:60, steps 995-1011, validity 2085-2092) is outside 13..28, so it can never be rolled. The boss roll (`Random(15)+14`, line 588-589, 14..28) misses it too, but that is NPC-only and untouched.
- Feat 177: `int FORCE_POWER_FORCE_CRUSH = 177;` (nwscript.nss:877) versus `int FEAT_MASTERCRAFT_WEAPONS_2 = 177;` (nwscript.nss:1200); live `override/feat.2da` row 177 label `MASTERCRAFT_WEAPONS_II`; live `override/spells.2da` row 177 `FORCE_POWER_FORCE_CRUSH` (range M, hostile 1). The Blitz validity test itself uses the spell: `GetHasSpell(FORCE_POWER_FORCE_CRUSH)` (gensupport:2055). So `nFeat = TRUE` is wrong.

**Design (party only; NPCs keep every vanilla result).**
- L1a: `Random(party ? 26 : 25)+1` so 29 is reachable.
- L1b: the second `JEDI_CRUSH` validity block becomes the `SITH_BRUTALIZE` block for party members (its steps exist at gensupport:874-888).
- L1c: Sith Attack step 1 uses `GN_GetSeriesScream()` and falls back to `GN_GetSeriesChoke()` (the combo's validity test requires the Choke series, gensupport:1971).
- L1d: Blitz step 1 casts Force Crush as a spell (`nFeat = FALSE`).
- **L1a must ship with L2.** Focus step 1 is Battle Meditation (spells.2da 138-140: range `T`, hostile 0, `FORCE_POWER_BATTLE_MEDITATION_PC`, nwscript.nss:838-840 "Only PCs, CNPCs, and Friendly allies should cast"). Without L2 the step is cast with the enemy as target, as the buffs are today.

**Patch block:**
```python
    ("L1a", "party", S,
     "            nRand = Random(25)+1; //RWT-OEI 09/03/04 - Changed from 23->25\n",
     "            //Party AI fix L1 (party only): Random(25)+1 is 1..25, nRand+3 therefore stopped at 28 and SW_COMBO_JEDI_FOCUS\n"
     "            //(29, Battle Meditation) was never rolled.\n"
     "            nRand = Random(IsObjectPartyMember(OBJECT_SELF) ? 26 : 25)+1; //RWT-OEI 09/03/04 - Changed from 23->25\n"),
    ("L1b", "party", S,
     "    else if(nComboType == SW_COMBO_JEDI_CRUSH)\n"
     "    {\n"
     "        if(GN_CheckSeriesChoke() && GN_CheckSeriesDrainLife() && GN_CheckSeriesForcePush())\n",
     "    //Party AI fix L1 (party only): this second SW_COMBO_JEDI_CRUSH block (Choke + Drain + Push) is Sith Brutalize's\n"
     "    //(its steps exist above); as written it could never run, so Brutalize was never valid.\n"
     "    else if(nComboType == (IsObjectPartyMember(OBJECT_SELF) ? SW_COMBO_SITH_BRUTALIZE : SW_COMBO_JEDI_CRUSH))\n"
     "    {\n"
     "        if(GN_CheckSeriesChoke() && GN_CheckSeriesDrainLife() && GN_CheckSeriesForcePush())\n"),
    ("L1c", "party", S,
     "            //nID = GN_GetSeriesChoke();\n"
     "            GN_GetSeriesScream();\n",
     "            //nID = GN_GetSeriesChoke();\n"
     "            //Party AI fix L1 (party only): the series result was thrown away, so this step was a plain attack. Scream, else\n"
     "            //Choke (the combo's validity test requires the Choke series).\n"
     "            if(IsObjectPartyMember(OBJECT_SELF))\n"
     "            {\n"
     "                nID = GN_GetSeriesScream();\n"
     "                if(nID == -1)\n"
     "                {\n"
     "                    nID = GN_GetSeriesChoke();\n"
     "                }\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party L1: Sith Attack step 1 uses spell \" + GN_ITS(nID));\n"
     "            }\n"
     "            else\n"
     "            {\n"
     "                GN_GetSeriesScream();\n"
     "            }\n"),
    ("L1d", "party", S,
     "            nID = FORCE_POWER_FORCE_CRUSH;\n"
     "            nFeat = TRUE;\n",
     "            nID = FORCE_POWER_FORCE_CRUSH;\n"
     "            //Party AI fix L1 (party only): Force Crush (177) is a spell (the combo's validity test uses GetHasSpell);\n"
     "            //as feat 177 it is MASTERCRAFT_WEAPONS_II, so the step was a plain attack.\n"
     "            nFeat = !IsObjectPartyMember(OBJECT_SELF);\n"
     "            if(IsObjectPartyMember(OBJECT_SELF))\n"
     "            {\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party L1: Blitz step 1 uses Force Crush as a spell (177)\");\n"
     "            }\n"),
```

| patch | file | pristine vanilla | at apply time | src_party_ai |
|---|---|---|---|---|
| L1a | k_inc_gensupport.nss | 1 | 1 | 1 |
| L1b | k_inc_gensupport.nss | 1 | 1 | 1 |
| L1c | k_inc_gensupport.nss | 1 | 1 | 1 |
| L1d | k_inc_gensupport.nss | 1 | 1 | 1 |

(`grep -cF 'Random(25)+1;' vanilla/k_inc_gensupport.nss` = 1, `grep -cF 'nID = FORCE_POWER_FORCE_CRUSH;'` = 1. L1b's anchor is the second duplicate only: its third line `GN_CheckSeriesChoke() && GN_CheckSeriesDrainLife() && GN_CheckSeriesForcePush()` differs from the first block's.)

**Gate.** `IsObjectPartyMember(OBJECT_SELF)` in each. **Marker.** Existing `Starting Jedi Combo SW_COMBO_JEDI_FOCUS` / `SW_COMBO_SITH_BRUTALIZE` (k_inc_generic.nss:3010), plus `Party L1: Sith Attack step 1 uses spell N` and `Party L1: Blitz step 1 uses Force Crush as a spell (177)`.

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| L1 | A party Jedi on Aggressive (9) with Battle Meditation and a melee feat, another with the Choke, Drain Life and Push series, one with Force Crush; several fights. | Over many combo rolls Focus and Brutalize appear; Blitz step 1 casts Force Crush; Sith Attack step 1 casts Scream (or Choke). | `Starting Jedi Combo SW_COMBO_JEDI_FOCUS`, `... SW_COMBO_SITH_BRUTALIZE`, `Party L1: Blitz step 1 uses Force Crush as a spell (177)`, `Party L1: Sith Attack step 1 uses spell N` |

**Risks.** Low. Combo steps still go through the existing talent validity test (`GetCreatureHasTalent`, gensupport:1201) so a power the member cannot afford stays a plain attack. Force Crush costs 60 FP (spells.2da): H2/H1 heal and revive decisions run first in `GN_RunDefaultAIRoutine` (k_inc_generic.nss:637-646). No interaction with M4/M10/E3/E8/kite (kite uses the default routine only in its step 4 and shares the combo code, so it benefits too).

## L2: combo buffs

**Problem (verified).**
```
k_inc_gensupport.nss:727        SetLocalBoolean(OBJECT_SELF, SW_FLAG_TARGET_FRIEND, FALSE);     (the only write of boolean 84 in 638 scripts + src)
k_inc_generic.nss:648-661       tUse = GN_GetComboMove(); int nFriend = GetLocalBoolean(OBJECT_SELF, SW_FLAG_TARGET_FRIEND);
                                if(nFriend == TRUE) { ... oTarget = OBJECT_SELF; } else { oTarget = tPR.oLastTarget; ... }
k_inc_generic.nss:915-929       if(GetIsTalentValid(tUse) && GetIsEnemy(oTarget)) { ActionUseTalentOnObject(tUse, oTarget); return TRUE; }
                                else if(GetIsEnemy(oTarget)) { ActionAttack(oTarget); return TRUE; }
                                ... "Default AI has failed to do an action" ... return FALSE;
k_inc_gensupport.nss:754-771    Buff Attack: Force Armor/Speed with nBuff = TRUE (never read); 979-994 Sith Fury: GN_GetSeriesFury() (no flag)
```
Two defects, not one: (1) the flag is never TRUE, so buffs are cast with the enemy as target, with no already-active test (vanilla's Jedi Support buff path does test it: k_inc_generic.nss:2756-2768 loops `GetEffectSpellId`); (2) **even with the flag set, the self branch could not act**: with `nFriend == TRUE`, `oTarget` is the member itself and `GetIsEnemy(oTarget)` is FALSE in both branches at 915 and 921, so the routine clears the queue (line 909) and returns FALSE; a round is lost. The earlier suggestion ("set the flag and the default AI then targets self") is therefore incomplete.

spells.2da (live override): Armor 17, Aura 18, Shield 24, Speed Burst 8, Knight Speed 34, Speed Mastery 37, Fury 164-166: range `P`, hostile 0, category 0x1808; Battle Meditation 138-140: range `T`, hostile 0, category 0x1808. Vanilla casts all of these on the caster with `ActionUseTalentOnObject(tUse, OBJECT_SELF)` in the Jedi Support buff path (k_inc_generic.nss:2829), including the Battle Meditation tree (2680-2685). `GN_CheckThrowLightSaberUsage` and `GN_CheckNonDroidForcePower` (k_inc_generic.nss:3133-3219) pass these talents through unchanged. `GetHasSpellEffect` has precedent on the speed series (k_inc_generic.nss:1387-1389).

**Design.**
- L2a: two helpers before `GN_GetNextTalentInCombo`: `GN_PartyIsComboBuff(nSpell)` (the Armor/Aura/Shield, Speed, Fury and Battle Meditation families) and `GN_PartyBuffActive(nSpell)` (any member of the family has an effect on the caster, using `GetHasSpellEffect`; the same family does not stack).
- L2b: at the end of `GN_GetNextTalentInCombo`, for a party member whose talent is one of those buffs: if the family is already active return an invalid talent (the step becomes a plain attack, as for any talent the member cannot use; the combo index has already advanced); else, if the member can cast it, set `SW_FLAG_TARGET_FRIEND` TRUE.
- L2c: in `GN_RunDefaultAIRoutine`, before the enemy test: a party member with `nFriend == TRUE` and `oTarget == OBJECT_SELF` casts the talent on itself and returns TRUE.

Prototypes relied on: `int GetHasSpellEffect(int nSpell, object oObject=OBJECT_SELF);` (nwscript.nss:3578; comment line 3575 "Determine if oObject has effects originating from nSpell."), `int GetIdFromTalent(talent tTalent);` (:3844), `int GetTypeFromTalent(talent tTalent);` (:3841), `int GetIsTalentValid(talent tTalent);` (:3829), `int GetCreatureHasTalent(talent tTalent, object oCreature=OBJECT_SELF);` (:3585), `void ActionUseTalentOnObject(talent tChosenTalent, object oTarget);` (:3606), `int TALENT_TYPE_FORCE = 0;` (:1402).

**Patch block:**
```python
    ("L2a", "party", S,
     "talent GN_GetNextTalentInCombo(int nCombo)\n"
     "{\n",
     "//Party AI fix L2 (party only): buff spells of the combos (all range P, or T for Battle Meditation, hostile 0 in spells.2da).\n"
     "//TRUE when the buff family of nSpell already runs on the caster (the same family does not stack).\n"
     "int GN_PartyBuffActive(int nSpell)\n"
     "{\n"
     "    if (nSpell == FORCE_POWER_FORCE_ARMOR || nSpell == FORCE_POWER_FORCE_AURA || nSpell == FORCE_POWER_FORCE_SHIELD)\n"
     "    {\n"
     "        return GetHasSpellEffect(FORCE_POWER_FORCE_ARMOR) || GetHasSpellEffect(FORCE_POWER_FORCE_AURA) || GetHasSpellEffect(FORCE_POWER_FORCE_SHIELD);\n"
     "    }\n"
     "    if (nSpell == FORCE_POWER_SPEED_BURST || nSpell == FORCE_POWER_KNIGHT_SPEED || nSpell == FORCE_POWER_SPEED_MASTERY)\n"
     "    {\n"
     "        return GetHasSpellEffect(FORCE_POWER_SPEED_BURST) || GetHasSpellEffect(FORCE_POWER_KNIGHT_SPEED) || GetHasSpellEffect(FORCE_POWER_SPEED_MASTERY);\n"
     "    }\n"
     "    if (nSpell == FORCE_POWER_FURY || nSpell == FORCE_POWER_IMPROVED_FURY || nSpell == FORCE_POWER_MASTER_FURY)\n"
     "    {\n"
     "        return GetHasSpellEffect(FORCE_POWER_FURY) || GetHasSpellEffect(FORCE_POWER_IMPROVED_FURY) || GetHasSpellEffect(FORCE_POWER_MASTER_FURY);\n"
     "    }\n"
     "    if (nSpell == FORCE_POWER_BATTLE_MEDITATION_PC || nSpell == FORCE_POWER_IMPROVED_BATTLE_MEDITATION_PC || nSpell == FORCE_POWER_MASTER_BATTLE_MEDITATION_PC)\n"
     "    {\n"
     "        return GetHasSpellEffect(FORCE_POWER_BATTLE_MEDITATION_PC) || GetHasSpellEffect(FORCE_POWER_IMPROVED_BATTLE_MEDITATION_PC) || GetHasSpellEffect(FORCE_POWER_MASTER_BATTLE_MEDITATION_PC);\n"
     "    }\n"
     "    return FALSE;\n"
     "}\n"
     "//TRUE for the buff spells GN_PartyBuffActive knows (the combos' Armor, Speed, Fury and Battle Meditation steps).\n"
     "int GN_PartyIsComboBuff(int nSpell)\n"
     "{\n"
     "    return nSpell == FORCE_POWER_FORCE_ARMOR || nSpell == FORCE_POWER_FORCE_AURA || nSpell == FORCE_POWER_FORCE_SHIELD ||\n"
     "           nSpell == FORCE_POWER_SPEED_BURST || nSpell == FORCE_POWER_KNIGHT_SPEED || nSpell == FORCE_POWER_SPEED_MASTERY ||\n"
     "           nSpell == FORCE_POWER_FURY || nSpell == FORCE_POWER_IMPROVED_FURY || nSpell == FORCE_POWER_MASTER_FURY ||\n"
     "           nSpell == FORCE_POWER_BATTLE_MEDITATION_PC || nSpell == FORCE_POWER_IMPROVED_BATTLE_MEDITATION_PC ||\n"
     "           nSpell == FORCE_POWER_MASTER_BATTLE_MEDITATION_PC;\n"
     "}\n"
     "\n"
     "talent GN_GetNextTalentInCombo(int nCombo)\n"
     "{\n"),
    ("L2b", "party", S,
     "    //Use GetCreatureHasTalent here to make the talent is currently usable - ie enough force points.\n"
     "    //Currently it terminates the script, so do not use it.\n",
     "    //Party AI fix L2 (party only): a combo buff step is cast on the member itself (GN_RunDefaultAIRoutine reads\n"
     "    //SW_FLAG_TARGET_FRIEND, which nothing ever set), and only while the buff is not already running; an active buff makes\n"
     "    //the step a plain attack (invalid talent), as the combo already does for a talent it cannot use.\n"
     "    if(IsObjectPartyMember(OBJECT_SELF) && GetIsTalentValid(tUse) && GetTypeFromTalent(tUse) == TALENT_TYPE_FORCE &&\n"
     "       GN_PartyIsComboBuff(GetIdFromTalent(tUse)))\n"
     "    {\n"
     "        if(GN_PartyBuffActive(GetIdFromTalent(tUse)))\n"
     "        {\n"
     "            GN_MyPrintString(\"GENERIC DEBUG *************** Party L2: buff \" + GN_ITS(GetIdFromTalent(tUse)) + \" already active, step skipped\");\n"
     "            return tInvalid;\n"
     "        }\n"
     "        if(GetCreatureHasTalent(tUse))\n"
     "        {\n"
     "            SetLocalBoolean(OBJECT_SELF, SW_FLAG_TARGET_FRIEND, TRUE);\n"
     "        }\n"
     "    }\n"
     "    //Use GetCreatureHasTalent here to make the talent is currently usable - ie enough force points.\n"
     "    //Currently it terminates the script, so do not use it.\n"),
    ("L2c", "party", G,
     "        if(GetIsTalentValid(tUse) && GetIsEnemy(oTarget))\n"
     "        {\n"
     "            GN_MyPrintString(\"GENERIC DEBUG *************** Using Talent on Target\");\n",
     "        //Party AI fix L2 (party only): a combo buff step (SW_FLAG_TARGET_FRIEND, oTarget = self) is cast on the member\n"
     "        //itself. Before, the enemy test below failed for oTarget = self and the round was lost.\n"
     "        if(IsObjectPartyMember(OBJECT_SELF) && nFriend == TRUE && oTarget == OBJECT_SELF && GetIsTalentValid(tUse))\n"
     "        {\n"
     "            GN_MyPrintString(\"GENERIC DEBUG *************** Party L2: self buff, spell \" + GN_ITS(GetIdFromTalent(tUse)));\n"
     "            ActionUseTalentOnObject(tUse, OBJECT_SELF);\n"
     "            return TRUE;\n"
     "        }\n"
     "        if(GetIsTalentValid(tUse) && GetIsEnemy(oTarget))\n"
     "        {\n"
     "            GN_MyPrintString(\"GENERIC DEBUG *************** Using Talent on Target\");\n"),
```

| patch | file | pristine vanilla | at apply time | src_party_ai |
|---|---|---|---|---|
| L2a | k_inc_gensupport.nss | 1 | 1 | 1 |
| L2b | k_inc_gensupport.nss | 1 | 1 | 1 |
| L2c | k_inc_generic.nss | 1 | 1 | 1 |

(L2a anchors the function header with its `{` on the next line; the prototype at gensupport:92 ends with `;`. `grep -cF 'Currently it terminates the script' vanilla/k_inc_gensupport.nss` = 1.)

**Gate.** `IsObjectPartyMember(OBJECT_SELF)` in L2b and L2c. **Marker.** `Party L2: self buff, spell N`, `Party L2: buff N already active, step skipped`.

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| L2 | A Jedi companion on Aggressive (9) with Force Armor, Force Speed and a melee feat; fight long enough to roll the Buff Attack combo twice, the second time while the buffs still run. | First roll: Armor and Speed are cast on the companion itself (it does not walk to the enemy), then a melee feat. Second roll while active: those steps become plain attacks, no Force points spent. | `Starting Jedi Combo SW_COMBO_BUFF_ATTACK`, `Party L2: self buff, spell 17` (or 24/18), then spell 8/34/37; second roll: `Party L2: buff N already active, step skipped` |

**Risks.**
- `GetHasSpellEffect` on Battle Meditation (138-140) and Fury is unverified (the speed series has precedent). If it misses, the worst case is a recast of that buff when the combo comes round again (a random roll, not every round).
- A buff step costs one round (as designed by the combo); heals and poison cures run before the combo, so H2/M5 keep priority. Force points: `GetCreatureHasTalent` is the FP test (gensupport:1201), Fury 30 FP, Armor 20 FP.
- Interaction: E8/M10 act on the enemy branch, which the self branch bypasses; M4/M3/M2 (target and area picks) are not involved; kite step 4 reaches the same code and benefits; E1 exits are decided before. H1 (Revitalize) is a separate branch.
- Without L2c the flag alone would lose a round: ship the three together.

## L3: party droids

**Problem (verified).**
```
k_inc_gensupport.nss:621-622    if((IsObjectPartyMember(OBJECT_SELF) && GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_JEDI_SUPPORT) ||
                                    !IsObjectPartyMember(OBJECT_SELF))        (in GN_GetStandardDroidCombo: only then rolls 5/6 become the utility combos 30/31)
nwscript.nss:1593               int NPC_AISTYLE_JEDI_SUPPORT    = 5;//Depricated, we don't use this anymore.
k_inc_generic.nss:457-473       stance 13 (PARTY_SUPPORT): Jedi -> GN_RunJediSupportAIRoutine, else GN_RunGrenadeAIRoutine
k_inc_generic.nss:976-1003      GN_RunGrenadeAIRoutine falls back to GN_RunDefaultAIRoutine when no grenade applies
```
So a party droid on any live stance rolls `SW_COMBO_INVALID` for utilities and re-rolls (k_inc_gensupport.nss:633-644). Utility items are spells.2da category 0x8000 with usertype 4, 0 Force points (rows 116-125, 188-189, 244-245, 274-281: stun ray, shield disruptor, flame thrower, carbonite, gravity generator, ion blast, ...); several are range `S` (flame thrower, gravity generator, plasma, toxin, bio spray).

**Design.** Allow the utility combos for party droids on the stances that may approach: 9 (Aggressive), 10 (Defensive), 13 (Support, via the grenade fallback). Ranged (11), Stationary (12) and Kite (16) keep the vanilla result because the user decision is that they never approach and short-range utilities would need it. `NPC_AISTYLE_PARTY_KITE` is declared in k_inc_generic, after this include, so the literal 16 is used with a comment. `GN_CheckDroidUtilityUsage` (k_inc_generic.nss:3065-3118) still filters stun ray and shield disruptor targets.

**Patch block:**
```python
    ("L3", "party", S,
     "            if((IsObjectPartyMember(OBJECT_SELF) && GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_JEDI_SUPPORT) ||\n"
     "                !IsObjectPartyMember(OBJECT_SELF))\n",
     "            //Party AI fix L3 (party only): party droids use their utility items (stun ray, flame thrower, ...) on the stances\n"
     "            //that may approach (9, 10, 13). Ranged (11), Stationary (12) and Kite (16) never approach (user decision) and\n"
     "            //several utilities are short range, so they keep the vanilla result (no utility). NPC_AISTYLE_PARTY_KITE is\n"
     "            //declared in k_inc_generic, after this include: literal 16.\n"
     "            int nPartyUtil = IsObjectPartyMember(OBJECT_SELF) && (GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_JEDI_SUPPORT ||\n"
     "                  (GetNPCAIStyle(OBJECT_SELF) != NPC_AISTYLE_PARTY_RANGED && GetNPCAIStyle(OBJECT_SELF) != NPC_AISTYLE_PARTY_STATIONARY &&\n"
     "                   GetNPCAIStyle(OBJECT_SELF) != 16));\n"
     "            if(nPartyUtil)\n"
     "            {\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party L3: party droid may roll a utility combo, stance \" + GN_ITS(GetNPCAIStyle(OBJECT_SELF)));\n"
     "            }\n"
     "            if(nPartyUtil || !IsObjectPartyMember(OBJECT_SELF))\n"),
```

| patch | file | pristine vanilla | at apply time | src_party_ai |
|---|---|---|---|---|
| L3 | k_inc_gensupport.nss | 1 | 1 | 1 |

**Gate.** `IsObjectPartyMember(OBJECT_SELF)` plus the stance test; non-party callers keep exactly the old condition (`nPartyUtil || !IsObjectPartyMember(OBJECT_SELF)`). **Marker.** `Party L3: party droid may roll a utility combo, stance N`; the pick shows as the existing `Starting Droid Combo SW_COMBO_DROID_UTILITIES` (k_inc_generic.nss:3022).

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| L3 | T3-M4 (or another droid with a utility item in an arm slot) on Aggressive (9), against a human and a droid enemy; then the same on Ranged (11). | On 9/10/13 the droid sometimes fires the utility (about one combo in three starts as a utility combo); stun ray only at humans. On 11/12/16 it never starts one. | `Party L3: party droid may roll a utility combo, stance 9`, `Starting Droid Combo SW_COMBO_DROID_UTILITIES`, `Droid Utility Check is TRUE`; none of these on stance 11 |

**Risks.** Whether party droids' utility items work as unlimited-use is plausible, not checked; they cost no Force points (usertype 4). A short-range utility (flame thrower) makes the droid walk toward the target on stances 9/10/13, which is the normal behaviour for those. Interaction: M4 (the droid flag) is a different function; H1/H2/M1/M10/E1/E3/E8/kite untouched (kite excluded by the literal 16).

## E9 / L7: allied shout 15

**Problem (verified).**
```
k_inc_generic.nss:2054   if(nShoutIndex == 1 && GetIsFriend(oShouter) && oShouter != OBJECT_SELF)
k_inc_generic.nss:2066     if((IsObjectPartyMember(OBJECT_SELF) && !GetPlayerRestrictMode()) || !IsObjectPartyMember(OBJECT_SELF))   { ... }
k_inc_generic.nss:2136-2139    //I AM IN COMBAT
                               else if(nShoutIndex == 15 && GetIsFriend(oShouter) && oShouter != OBJECT_SELF)
                               {  if(GetCurrentAction(OBJECT_SELF) == ACTION_INVALID) ...
k_inc_generic.nss:2208-2220    GN_SetListeningPatterns: ... SetListenPattern(OBJECT_SELF, "GEN_COMBAT_ACTIVE", 15);
k_ai_master.nss:171 (case 1003, default AI)    SpeakString("GEN_COMBAT_ACTIVE", TALKVOLUME_SILENT_TALK);
k_ai_master.nss:633 (case 2003, party)         SpeakString("GEN_I_WAS_ATTACKED", TALKVOLUME_SILENT_TALK);
```
The `else if(15)` at 2137 belongs to the `if` at 2066, which sits inside `if(nShoutIndex == 1 ...)` at 2054, and the 2066 condition is always true (2063 already required `!GetPlayerRestrictMode()`). So shout 15 never runs for anyone. Allied non-party creatures shout 15 every round (1003); party members register to hear it (2220) and shout 1 themselves; the shout-1 branch accepts party shouters only for party listeners (2056). A separate defect: the dead branch tests `GetCurrentAction(OBJECT_SELF) == ACTION_INVALID`, but an idle companion reports `ACTION_FOLLOWLEADER` (38, nwscript.nss:1507) or `ACTION_QUEUEEMPTY` (65534, :1512), never `ACTION_INVALID` (65535, :1511), so simply un-nesting would still do nothing. The shout-1 branch uses the "no attempted attack/spell target and no attack target" test instead (2060), which is what the proposal copies. The shout reaches the party AI through the E2 routing (k_hen_<name>dlg -> k_ai_master 2004 -> GN_RespondToShout, k_ai_master.nss:660).

**Design.** A party-only early branch at the top of `GN_RespondToShout`'s "Check 1 Pass" block (so the existing `!AI_OFF`, not-spectator, `!GetUserActionsPending()` gates apply): for `nShoutIndex == 15`, call `GN_PartyAnswerAllyShout(oShouter, oIntruder)`. It answers only when all hold: not the leader, not solo, not restrict mode, not Passive, no conversation, commandable, alive; the shouter is a non-party friend; the shouter's last hostile actor is a live enemy **seen** by this member (an unseen one is never approached: the Ranged/Stationary/Kite rule, and no walk-offs); the member is idle (the triple test); the enemy is within 25 m (15 m for stances 11/12/16, which never approach); the member is within 15 m of the leader. Then `ClearAllActions(); GN_DetermineCombatRound(oIntruder);` as the shout-1 branch does (k_inc_generic.nss:2076-2077). All numbers are in the helper (`fMax`, 15.0) for tuning; deleting E9b switches the feature off.

Prototypes relied on: `object GetLastHostileActor(object oVictim=OBJECT_SELF);` (nwscript.nss:4714, "This value will never be a dead/destroyed creature" at :4713), `int GetObjectSeen(object oTarget, object oSource=OBJECT_SELF);` (:3513), `int GetIsFriend(object oTarget, object oSource=OBJECT_SELF);` (:3264), `int GetIsEnemy(...)` (:3261), `int GetPartyAIStyle();` (:5263), `int PARTY_AISTYLE_PASSIVE = 2;` (:1586), `int GetPlayerRestrictMode(object oObject = OBJECT_SELF);` (:2469), `object GetAttemptedAttackTarget();` (:3838), `object GetAttemptedSpellTarget();` (:3898), `object GetAttackTarget(object oCreature=OBJECT_SELF);` (:3635), `float GetDistanceToObject(object oObject);` (:2288), `int GetIsInConversation(object oObject);` (:4181).

**Patch block:**
```python
    ("E9a", "party", G,
     "void GN_RespondToShout(object oShouter, int nShoutIndex, object oIntruder = OBJECT_INVALID)\n"
     "{\n",
     "//Party AI fix E9/L7 (party only): an idle party member answers an allied NPC's GEN_COMBAT_ACTIVE shout (15) by taking on\n"
     "//the shouter's seen enemy. Held back when: leader, solo, restrict or Passive, in conversation, already fighting; the\n"
     "//shouter is a party member (shout 1 covers those) or not a friend; the enemy is unseen (never approach), dead, or farther\n"
     "//than 25 m (15 m for Ranged, Stationary and Kite, which never approach); the member is over 15 m from the leader.\n"
     "//Returns TRUE when it started a combat round.\n"
     "int GN_PartyAnswerAllyShout(object oShouter, object oIntruder)\n"
     "{\n"
     "    object oLeader = GetPartyMemberByIndex(0);\n"
     "    if (oLeader == OBJECT_SELF || GetSoloMode() || GetPlayerRestrictMode() || GetPartyAIStyle() == PARTY_AISTYLE_PASSIVE ||\n"
     "        GetIsConversationActive() || GetIsInConversation(OBJECT_SELF) || !GetCommandable() || GetIsDead(OBJECT_SELF))\n"
     "    {\n"
     "        return FALSE;\n"
     "    }\n"
     "    if (!GetIsObjectValid(oShouter) || oShouter == OBJECT_SELF || IsObjectPartyMember(oShouter) || !GetIsFriend(oShouter))\n"
     "    {\n"
     "        return FALSE;\n"
     "    }\n"
     "    if (!GetIsObjectValid(oIntruder) || GetIsDead(oIntruder) || !GetIsEnemy(oIntruder) || !GetObjectSeen(oIntruder))\n"
     "    {\n"
     "        return FALSE;\n"
     "    }\n"
     "    if (GetIsObjectValid(GetAttemptedAttackTarget()) || GetIsObjectValid(GetAttemptedSpellTarget()) || GetIsObjectValid(GetAttackTarget()))\n"
     "    {\n"
     "        return FALSE;\n"
     "    }\n"
     "    int nStyle = GetNPCAIStyle(OBJECT_SELF);\n"
     "    float fMax = 25.0;\n"
     "    if (nStyle == NPC_AISTYLE_PARTY_RANGED || nStyle == NPC_AISTYLE_PARTY_STATIONARY || nStyle == NPC_AISTYLE_PARTY_KITE)\n"
     "    {\n"
     "        fMax = 15.0;\n"
     "    }\n"
     "    if (GetDistanceToObject(oIntruder) > fMax || GetDistanceBetween(OBJECT_SELF, oLeader) > 15.0)\n"
     "    {\n"
     "        return FALSE;\n"
     "    }\n"
     "    GN_MyPrintString(\"GENERIC DEBUG *************** Party E9: ally \" + GN_ReturnDebugName(oShouter) + \" shouted 15, engaging \" + GN_ReturnDebugName(oIntruder));\n"
     "    ClearAllActions();\n"
     "    GN_DetermineCombatRound(oIntruder);\n"
     "    return TRUE;\n"
     "}\n"
     "\n"
     "void GN_RespondToShout(object oShouter, int nShoutIndex, object oIntruder = OBJECT_INVALID)\n"
     "{\n"),
    ("E9b", "party", G,
     "            GN_MyPrintShoutString(\"GENERIC SHOUT DEBUG *************** Check 1 Pass\");\n"
     "            if(nShoutIndex == 1 && GetIsFriend(oShouter) && oShouter != OBJECT_SELF)\n",
     "            GN_MyPrintShoutString(\"GENERIC SHOUT DEBUG *************** Check 1 Pass\");\n"
     "            //Party AI fix E9/L7 (party only): the shout-15 branch further down is nested inside the shout-1 test below and\n"
     "            //never ran; party members answer it here (GN_PartyAnswerAllyShout). NPCs keep the vanilla routing.\n"
     "            if(nShoutIndex == 15 && IsObjectPartyMember(OBJECT_SELF) && GN_PartyAnswerAllyShout(oShouter, oIntruder))\n"
     "            {\n"
     "                return;\n"
     "            }\n"
     "            if(nShoutIndex == 1 && GetIsFriend(oShouter) && oShouter != OBJECT_SELF)\n"),
```

| patch | file | pristine vanilla | at apply time | src_party_ai |
|---|---|---|---|---|
| E9a | k_inc_generic.nss | 1 | 1 | 1 |
| E9b | k_inc_generic.nss | 1 | 1 | 1 |

(`grep -cF 'Check 1 Pass' vanilla/k_inc_generic.nss` = 1; the function header with its `{` is unique, the prototype at line 148 ends with `;`.)

**Gate.** `IsObjectPartyMember(OBJECT_SELF)` at the call and inside the helper; NPCs keep the vanilla routing. **Marker.** `Party E9: ally <name> shouted 15, engaging <enemy>`.

| id | in-game setup | expected after the fix | trace line(s) |
|---|---|---|---|
| E9 | An allied NPC (a friendly soldier or summoned ally) fights a hostile creature within sight of the idle party; the leader does not attack. Repeat with a companion on Ranged (11) and the enemy at 20 m, and with the enemy behind cover (unseen). | Aggressive/Defensive/Support companions engage the enemy within a round of the ally's shout. Ranged/Stationary/Kite only if it is within 15 m and seen. Nobody walks to an unseen enemy; the leader is not dragged. | `Party E9: ally <name> shouted 15, engaging <enemy>`, then the DCR lines (`Clear 1300`, `Using Talent on Target` or `Action Attack by Default`) |

**Risks.**
- Behaviour change: companions start fights the leader did not choose (stealth or avoid-the-patrol play, scripted "friendly vs hostile" scenes that are meant to be watched). The guards (seen, idle, 15 m from the leader, not in a conversation or restrict mode, not Passive) limit it, but not remove it; an INI-style kill switch is not possible in script, so the build gate is the switch.
- E3 (shout 1, unseen intruder) and E11 are different branches and unchanged; E9 handles only seen enemies, so no overlap. kite/Ranged/Stationary: held to 15 m and seen, consistent with E3/E11. E1: after the ally's fight the normal combat exit runs. M10/E8/H1/H2/M1/M4: inside the DCR that E9 calls; unchanged.
- The ally shouts every round; after the first answer the member has an attack target, so the idle test stops repeats (no queue wipe each round).

## M5 matrix row (old -> new)

Old, `design_ai_script_fixes.md:869`:
```
| M5 | Poison a party member; companion with Cure/Heal. | Cure/Heal cast. | `Starting Poison Check` **without** `Returning Invalid Poison Object`, then `Clear 2300` / `Spells.2DA ID = 28 / 10 / 134` |
```
New (exact replacement text; basis: k_inc_generic.nss:2464-2495 M5b leaves Heal 28, Master Heal 134 for the party, the order is Heal first and Master Heal only when the caster has no Heal; k_inc_generic.nss:2309-2326 M5 reports only non-droid members within 15 m):
```
| M5 | Poison a non-droid party member within 15 m of a companion that has Heal or Master Heal. Repeat with a companion that has only Cure. | The Heal-capable companion casts Heal (id 28; Master Heal 134 only when it has no Heal) once and the poison clears. The Cure-only companion casts nothing for the poison and does not repeat a cast every round. | `Starting Poison Check` **without** `Returning Invalid Poison Object`, then `Clear 2300` / `Spells.2DA ID = 28` (or `134`), never `10` during the poison test (Cure can still be chosen by the injury heal, not the poison cure). Cure-only companion: no `Clear 2300` for the poison. |
```
(The matrix row for CAI-3 in `PLAY_CHECKLIST_2026-10-02.md` already reads "A companion with only Cure doesn't try", so the two now agree.)

## Open questions

1. E4: the heartbeat period. The engine review says 3000-4199 ms (review_ai_engine.md:395), the events review 6 s. `STUCK_MOVE_LIMIT = 3` gives 9-12.6 s or 18 s; tune after the first trace.
2. E4: does `ClearAllActions` free the engine's movement lock (the cause the events review names), or does the member re-stick? Needs a trace of a real stuck companion; the proposal is harmless either way.
3. E7: confirm with the probe that `exit5m`/`enter5m` really are once-per-crossing in play (asm says so) and how often FM4's save re-fire occurs.
4. M8: is the benefit worth the "stays in melee config about 2 round ends" cost for never-approach companions? It is a semantics fix; if the user prefers fast flip-back, ship only M8c (the marker) and skip M8a/M8b.
5. L2: do `GetHasSpellEffect(138..140)` (Battle Meditation) and Fury 164-166 report on the caster? Settled by the "already active" marker the first time a Jedi recasts within the buff's duration.
6. L2/L1: which companions own the Battle Meditation series at all? If none do, L1a changes nothing and could be dropped.
7. L3: do party droids' utility items have any use limit or reuse delay (spells.2da lists 0 Force points, usertype 4)? Not checked.
8. E9: user decision, since it makes companions pick fights; alternatives are to require the leader to be in combat, or to answer only when the shouter is within 10 m of the member.
9. NCS size and instruction budget are not measurable without compiling. The live `k_ai_master.ncs` is 131,658 bytes (md5 b003ea4135c7, identical to `out_party_ai/`); it already exceeds 128 KiB, so no such file limit is known to apply (plausible); the new code is a few hundred VM instructions per call against the 131,072 cap that only the shape scans (L8) approach.
10. The new text is not compiled. First build step to run after applying: nwnnsscomp compile of `k_ai_master` only, then the arg-order check (`check_ncs_arg_order.py`), as the build already does.
