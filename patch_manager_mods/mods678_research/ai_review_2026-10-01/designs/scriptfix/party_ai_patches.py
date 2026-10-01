#!/usr/bin/env python3
"""Prototype of the "party AI scripts" build (scratch only; never writes the game dir or the workspace).

Applies the anchored replacements of design_ai_script_fixes.md to the vanilla sources, asserts that every
anchor occurs exactly once, writes the patched sources to OUT/src, compiles them with nwnnsscomp through a
scratch copy of the recompile kit, and runs scripts/check_ncs_arg_order.py --override-only on the outputs.

Each patch: (id, scope, file, old, new). `old` must occur exactly once in the (CRLF) source and is replaced
by `new`. Texts are written with LF and converted to CRLF.
"""
import hashlib, os, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = "/home/ethansm/orca/workspaces/local-setup/KOTOR-II"
WS = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/mod8_kite_script")
VAN = os.path.join(WS, "vanilla")          # k_ai_master + includes (read-only)
VAN_ALL = os.path.join(WS, "vanilla_all")  # all 638 vanilla .nss (read-only)
KIT = os.path.join(HERE, "kit", "compile.sh")  # scratch copy of the recompile kit (own wineprefix)
BIF = os.path.join(HERE, "bif")            # vanilla .ncs/.nss extracted from Scripts.bif (extract.py)
OUT = os.path.join(HERE, "build")
VENV_PY = os.path.join(REPO, ".venv", "bin", "python")


def crlf(s):
    return s.replace("\r\n", "\n").replace("\n", "\r\n")


def md5(p):
    return hashlib.md5(open(p, "rb").read()).hexdigest()


# --------------------------------------------------------------------------------------------------
# Mod 8 base (as build_kite_script.py today) + new prototypes
# --------------------------------------------------------------------------------------------------
PROTO_ANCHOR = "int GN_CheckRangedPreferred();\n"
PROTOS = (
    "//Mod 8 (ranged-ai) - party stance 16 \"Ranged (Kiting)\": keeps its distance, repositions when the\n"
    "//shot is blocked, switches to melee if cornered. Body at the end of this file.\n"
    "int NPC_AISTYLE_PARTY_KITE = 16;\n"
    "int GN_RunKiteAIRoutine(object oIntruder = OBJECT_INVALID);\n"
    "//Party AI script fixes (design_ai_script_fixes.md). Bodies at the end of this file.\n"
    "int GN_PartyStanceCombatExit(int nNPC_AI);\n"
    "void GN_PartyApproachUnseen(object oIntruder);\n")

KITE_BRANCH = (
    "                else if (nNPC_AI == NPC_AISTYLE_PARTY_KITE)\n"
    "                {\n"
    "                    //Mod 8: same leash as Ranged Support, then the kite routine.\n"
    "                    if (GN_CheckRangeFromLeader(10.0f) == TRUE)\n"
    "                    {\n"
    "                        return;\n"
    "                    }\n"
    "                    GN_RunKiteAIRoutine(oIntruder);\n"
    "                    return;\n"
    "                }\n")

G = "k_inc_generic.nss"
S = "k_inc_gensupport.nss"
M = "k_ai_master.nss"

PATCHES = [
    # ---------------- k_inc_generic ----------------
    ("MOD8-proto", "party", G, PROTO_ANCHOR, PROTO_ANCHOR + PROTOS),
    ("MOD8-dispatch", "party", G,
     "                    GN_RunRangedSupportAIRoutine(oIntruder);\n"
     "                    return;\n"
     "                }\n"
     "                else if (nNPC_AI == NPC_AISTYLE_PARTY_STATIONARY)\n",
     "                    GN_RunRangedSupportAIRoutine(oIntruder);\n"
     "                    return;\n"
     "                }\n" + KITE_BRANCH +
     "                else if (nNPC_AI == NPC_AISTYLE_PARTY_STATIONARY)\n"),
    ("E1", "party", G,
     "                if(nNPC_AI == NPC_AISTYLE_MELEE_ATTACK)\n",
     "                //Party AI fix E1 (party only): stances 10-13 and 16 never reach GN_DoPostDCRChecks below.\n"
     "                if(GN_PartyStanceCombatExit(nNPC_AI) == TRUE)\n"
     "                {\n"
     "                    return;\n"
     "                }\n"
     "                if(nNPC_AI == NPC_AISTYLE_MELEE_ATTACK)\n"),
    ("E8", "party", G,
     "    //RWT-OEI 10/31/04 - If our target is more than 30 meters away,\n",
     "    //Party AI fix E8 (party only): never keep a dead last target (GetLastHostileTarget has no not-dead\n"
     "    //guarantee); with no seen enemy the routine now returns FALSE and the combat exit runs.\n"
     "    if(IsObjectPartyMember(OBJECT_SELF) && GetIsObjectValid(oTarget) && GetIsDead(oTarget))\n"
     "    {\n"
     "        GN_MyPrintString(\"GENERIC DEBUG *************** Party E8: dropped dead last target\");\n"
     "        oTarget = OBJECT_INVALID;\n"
     "    }\n"
     "    //RWT-OEI 10/31/04 - If our target is more than 30 meters away,\n"),
    ("M10b", "party", G,
     "        //Okay, target is too far. Just drop out of combat all\n"
     "        //together.\n"
     "        oTarget = OBJECT_INVALID;\n"
     "        CancelCombat(OBJECT_SELF);\n",
     "        //Okay, target is too far. Just drop out of combat all\n"
     "        //together.\n"
     "        //Party AI fix M10 (party only): a party member switches to the nearest seen enemy instead.\n"
     "        if(IsObjectPartyMember(OBJECT_SELF) && GetIsObjectValid(oClose) && GetObjectSeen(oClose) &&\n"
     "           !GetIsDead(oClose) && GetDistanceToObject(oClose) <= 30.0f)\n"
     "        {\n"
     "            GN_MyPrintString(\"GENERIC DEBUG *************** Party M10: target beyond 30 m, closest becomes target\");\n"
     "            oTarget = oClose;\n"
     "        }\n"
     "        else\n"
     "        {\n"
     "            oTarget = OBJECT_INVALID;\n"
     "            CancelCombat(OBJECT_SELF);\n"
     "        }\n"),
    ("M9", "party", G,
     "                if ( GetIsObjectValid(oClose) && GetDistanceToObject(oClose) > 5.0f )\n",
     "                //Party AI fix M9: measure from the leader, as the comment above says (was from OBJECT_SELF).\n"
     "                if ( GetIsObjectValid(oClose) && GetDistanceBetween(oClose, oLeader) > 5.0f )\n"),
    ("H1a", "party", G,
     "    if (GN_CheckSeriesRevitalize() != -1)\n",
     "    //Party AI fix H1: GN_CheckSeriesRevitalize returns TRUE/FALSE, never -1, so this gate was always open.\n"
     "    //Party members only; NPCs keep the vanilla gate (user decision).\n"
     "    if ((IsObjectPartyMember(OBJECT_SELF) && GN_CheckSeriesRevitalize() == TRUE) ||\n"
     "        (!IsObjectPartyMember(OBJECT_SELF) && GN_CheckSeriesRevitalize() != -1))\n"),
    ("H1b", "party", G,
     "            tUse = TalentSpell(nID);\n"
     "            bValid = TRUE;\n"
     "            bHostile = FALSE;\n",
     "            tUse = TalentSpell(nID);\n"
     "            bValid = TRUE;\n"
     "            //Party AI fix H1: only with the power and the Force points for it; an invalid talent also\n"
     "            //keeps the generic fallback below from casting it.\n"
     "            if (IsObjectPartyMember(OBJECT_SELF) && (nID == -1 || !GetCreatureHasTalent(tUse)))\n"
     "            {\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party H1: no usable Revitalize\");\n"
     "                bValid = FALSE;\n"
     "                tUse = TalentSpell(-1);\n"
     "            }\n"
     "            bHostile = FALSE;\n"),
    ("H2a", "party", G,
     "        if(GetIsObjectValid(oP1)&& GetRacialType(oP0) != RACIAL_TYPE_DROID)\n",
     "        //Party AI fix H2: race test on the member itself (was oP0 for all three).\n"
     "        if(GetIsObjectValid(oP1)&& GetRacialType(oP1) != RACIAL_TYPE_DROID)\n"),
    ("H2b", "party", G,
     "        if(GetIsObjectValid(oP2) && GetRacialType(oP0) != RACIAL_TYPE_DROID)\n",
     "        if(GetIsObjectValid(oP2) && GetRacialType(oP2) != RACIAL_TYPE_DROID)\n"),
    ("H2c", "party", G,
     "        else if(GetIsObjectValid(oP1) && !GetIsDead(oP1) && (fDown00 < 0.5 && fDown00 > 0.0))\n",
     "        //Party AI fix H2: each branch tests its own member's ratio (all three tested fDown00).\n"
     "        else if(GetIsObjectValid(oP1) && !GetIsDead(oP1) && (fDown01 < 0.5 && fDown01 > 0.0))\n"),
    ("H2d", "party", G,
     "        else if(GetIsObjectValid(oP2) && !GetIsDead(oP2) && (fDown00 < 0.5 && fDown00 > 0.0))\n",
     "        else if(GetIsObjectValid(oP2) && !GetIsDead(oP2) && (fDown02 < 0.5 && fDown02 > 0.0))\n"),
    ("M5", "party", G,
     "        for(nCnt; nCnt > 2; nCnt++)\n",
     "        //Party AI fix M5: the loop never ran (nCnt = 0 is never > 2).\n"
     "        for(nCnt = 0; nCnt < 3; nCnt++)\n"),
    ("M3", "party", G,
     "            while(!GetIsDebilitated(oTarget) && GetIsObjectValid(oTarget))\n",
     "            //Party AI fix M3 (party only): skip debilitated enemies (the test was inverted, so the nearest\n"
     "            //stunned/held/asleep enemy was preferred). NPCs keep the vanilla order (user decision).\n"
     "            int nPartyM3 = IsObjectPartyMember(OBJECT_SELF);\n"
     "            while(GetIsObjectValid(oTarget) &&\n"
     "                  ((nPartyM3 && GetIsDebilitated(oTarget)) || (!nPartyM3 && !GetIsDebilitated(oTarget))))\n"),
    ("M1", "party", G,
     "            if ( nType2 != 2 )\n"
     "            {\n"
     "                //Our second config is NOT ranged, switch to it\n",
     "            //Party AI fix M1: switch only to a melee config 2 (an empty or non-weapon config 2 is type 0).\n"
     "            if ( nType2 == 1 )\n"
     "            {\n"
     "                //Our second config is NOT ranged, switch to it\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party M1: hit in melee, switch to melee config 2\");\n"),
    ("E3", "party", G,
     "                                    if(GetDistanceBetween(OBJECT_SELF, oIntruder) < 20.0 && !GetObjectSeen(oIntruder))\n"
     "                                    {\n"
     "                                        ActionMoveToObject(oIntruder, TRUE, 2.0);\n"
     "                                    }\n",
     "                                    //Party AI fix E3/M6 (party only): GN_PartyApproachUnseen (stand-off, Stationary holds,\n"
     "                                    //DCR on arrival). NPCs keep the vanilla 2 m run (user decision).\n"
     "                                    if(IsObjectPartyMember(OBJECT_SELF))\n"
     "                                    {\n"
     "                                        GN_PartyApproachUnseen(oIntruder);\n"
     "                                    }\n"
     "                                    else if(GetDistanceBetween(OBJECT_SELF, oIntruder) < 20.0 && !GetObjectSeen(oIntruder))\n"
     "                                    {\n"
     "                                        ActionMoveToObject(oIntruder, TRUE, 2.0);\n"
     "                                    }\n"),
    ("L4", "party (PC-only trees)", G,
     "            else if ( GetIsTalentValid(tBuff_05) &&\n"
     "                      GetCreatureHasTalent(tBuff_06) )\n",
     "            //Party AI fix L4: tier V tested the tier VI talent.\n"
     "            else if ( GetIsTalentValid(tBuff_05) &&\n"
     "                      GetCreatureHasTalent(tBuff_05) )\n"),
    # ---------------- k_inc_gensupport ----------------
    ("M2", "party", S,
     "        nEnemy = GN_CheckEnemyGroupingOnTarget(oCheck,4.0);\n"
     "        if(nEnemy > 2 && nEnemyCnt < nEnemy)\n",
     "        //Party AI fix M2 (party only): the AoE centre must be a live, seen enemy (no ally, corpse or\n"
     "        //unseen creature); skipping the inner scan for the rest also saves instructions (L8).\n"
     "        nEnemy = 0;\n"
     "        if(!IsObjectPartyMember(OBJECT_SELF) || (GetIsEnemy(oCheck) && !GetIsDead(oCheck) && GetObjectSeen(oCheck)))\n"
     "        {\n"
     "            nEnemy = GN_CheckEnemyGroupingOnTarget(oCheck,4.0);\n"
     "        }\n"
     "        if(nEnemy > 2 && nEnemyCnt < nEnemy)\n"),
    ("M4a", "party", S,
     "    int nChoke, nAfflict, nPlague, nPush, nWind, nLightning, nKill, nHorror, nWound, nStasis, nDroid, nKnock, nHowl, nCnt;\n",
     "    //Party AI fix M4 (party only): the local nDroid shadowed the parameter, so organic-only powers were\n"
     "    //picked against droids and Droid Disable never was. The flag is now nDisable; NPCs keep the vanilla\n"
     "    //result (nDroid forced FALSE, as the shadow did).\n"
     "    int nChoke, nAfflict, nPlague, nPush, nWind, nLightning, nKill, nHorror, nWound, nStasis, nDisable, nKnock, nHowl, nCnt;\n"
     "    if(!IsObjectPartyMember(OBJECT_SELF))\n"
     "    {\n"
     "        nDroid = FALSE;\n"
     "    }\n"),
    ("M4b", "party", S,
     "    if(GetHasSpell(FORCE_POWER_DROID_DISABLE) && nDroid == TRUE)\n"
     "    {\n"
     "        nDroid = 1;\n",
     "    if(GetHasSpell(FORCE_POWER_DROID_DISABLE) && nDroid == TRUE)\n"
     "    {\n"
     "        nDisable = 1;\n"),
    ("M4c", "party", S,
     "    if(nDroid== 1 && nRoll == 1)\n",
     "    if(nDisable == 1 && nRoll == 1)\n"),
    ("M4d", "party", S,
     "    if(nDroid == 1 && nRoll > 1)\n",
     "    if(nDisable == 1 && nRoll > 1)\n"),
    ("M7a", "party context", S,
     "    else\n"
     "    {\n"
     "        return 0;//Return 0 by default.\n"
     "    }\n",
     "    //Party AI fix M7 (party context only): TSL base items missing above. baseitems.2da 104 Mining_Laser,\n"
     "    //105 Adv_Mining_Laser (ranged); 106 Lightsaber_No_Sound, 107 DM_ElectroStaff (melee).\n"
     "    else if(IsObjectPartyMember(OBJECT_SELF) && (nBaseType == 104 || nBaseType == 105))\n"
     "    {\n"
     "        return 2;\n"
     "    }\n"
     "    else if(IsObjectPartyMember(OBJECT_SELF) && (nBaseType == 106 || nBaseType == 107))\n"
     "    {\n"
     "        return 1;\n"
     "    }\n"
     "    else\n"
     "    {\n"
     "        return 0;//Return 0 by default.\n"
     "    }\n"),
    ("M7b", "party context", S,
     "    GN_MyPrintString(\"GENERIC DEBUG *************** Return No Weapon Type\");\n",
     "    //Party AI fix M7 (party context only): see GN_GetWeaponTypeFromBaseItem.\n"
     "    if(GetIsObjectValid(oItem) && IsObjectPartyMember(OBJECT_SELF))\n"
     "    {\n"
     "        if(nBaseType == 104 || nBaseType == 105)\n"
     "        {\n"
     "            return 2;\n"
     "        }\n"
     "        if(nBaseType == 106 || nBaseType == 107)\n"
     "        {\n"
     "            return 1;\n"
     "        }\n"
     "    }\n"
     "    GN_MyPrintString(\"GENERIC DEBUG *************** Return No Weapon Type\");\n"),
    # ---------------- k_ai_master (hench cases: party only) ----------------
    ("E6", "hench", M,
     "            if ( nWeaponType == 1 || nWeaponType == 0)\n",
     "            //Party AI fix E6: an attacker with no known weapon in the right hand (creature weapons on\n"
     "            //droids, turrets and beasts; mod base items) counts as melee only when it is close.\n"
     "            if ( nWeaponType == 1 || (nWeaponType == 0 && GetIsObjectValid(oAttacker) &&\n"
     "                                      GetDistanceBetween(OBJECT_SELF, oAttacker) <= 5.0) )\n"),
    ("E13", "hench (leader)", M,
     "                else\n"
     "                {\n"
     "                    //Shout that I was attacked\n"
     "                    SpeakString(\"GEN_I_WAS_ATTACKED\", TALKVOLUME_SILENT_TALK);\n"
     "                }\n"
     "            }\n"
     "            if(GN_GetSpawnInCondition(SW_FLAG_EVENT_ON_ATTACKED))\n"
     "            {\n"
     "                SignalEvent(OBJECT_SELF, EventUserDefined(1005));\n",
     "                //Party AI fix E13: the leader already shouted twice above; only a member held back by\n"
     "                //player restrict mode shouts here.\n"
     "                else if(GetPartyMemberByIndex(0) != OBJECT_SELF)\n"
     "                {\n"
     "                    //Shout that I was attacked\n"
     "                    SpeakString(\"GEN_I_WAS_ATTACKED\", TALKVOLUME_SILENT_TALK);\n"
     "                }\n"
     "            }\n"
     "            if(GN_GetSpawnInCondition(SW_FLAG_EVENT_ON_ATTACKED))\n"
     "            {\n"
     "                SignalEvent(OBJECT_SELF, EventUserDefined(1005));\n"),
    ("E11", "hench", M,
     "                                            ActionMoveToLocation(GetLocation(oTarget), TRUE);\n"
     "                                            ActionDoCommand(GN_DetermineCombatRound());\n",
     "                                            //Party AI fix E11/M6: stand off by weapon as case 1006 does (was a run onto the\n"
     "                                            //damager's spot); Ranged, Stationary and Kite do not approach at all.\n"
     "                                            int nStyle2006 = GetNPCAIStyle(OBJECT_SELF);\n"
     "                                            if(nStyle2006 != NPC_AISTYLE_PARTY_RANGED && nStyle2006 != NPC_AISTYLE_PARTY_STATIONARY &&\n"
     "                                               nStyle2006 != NPC_AISTYLE_PARTY_KITE)\n"
     "                                            {\n"
     "                                                GN_MyPrintString(\"GENERIC DEBUG *************** Party E11: unseen damager, stand off\");\n"
     "                                                ActionMoveToObject(oTarget, TRUE, (GN_GetWeaponType(OBJECT_SELF) == 1) ? 4.0 : 15.0);\n"
     "                                                ActionDoCommand(GN_DetermineCombatRound());\n"
     "                                            }\n"),
    ("E10", "party (hench)", M,
     "        case 2009: //KOTOR_HENCH_EVENT_ON_BLOCKED\n"
     "        {\n"
     "            object oDoor = GetBlockingDoor();\n"
     "            if(!GN_GetSpawnInCondition(SW_FLAG_AI_OFF))\n"
     "            {\n"
     "                if(GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 5)\n"
     "                {\n"
     "                    if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_OPEN) && GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 7 )\n"
     "                    {\n"
     "                        DoDoorAction(oDoor, DOOR_ACTION_OPEN);\n"
     "                    }\n"
     "                    else if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_BASH))\n",
     "        case 2009: //KOTOR_HENCH_EVENT_ON_BLOCKED\n"
     "        {\n"
     "            object oDoor = GetBlockingDoor();\n"
     "            if(!GN_GetSpawnInCondition(SW_FLAG_AI_OFF))\n"
     "            {\n"
     "                if(GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 5)\n"
     "                {\n"
     "                    if(GetIsDoorActionPossible(oDoor, DOOR_ACTION_OPEN) && GetAbilityScore(OBJECT_SELF, ABILITY_INTELLIGENCE) >= 7 )\n"
     "                    {\n"
     "                        DoDoorAction(oDoor, DOOR_ACTION_OPEN);\n"
     "                    }\n"
     "                    //Party AI fix E10: party members open doors but never bash them (k_inc_generic post-DCR:\n"
     "                    //\"Party members won't help with bashing anymore\").\n"
     "                    else if(!IsObjectPartyMember(OBJECT_SELF) && GetIsDoorActionPossible(oDoor, DOOR_ACTION_BASH))\n"),
]

# E2: shout routing. Same two replacements in each of the 13 k_hen_<name>dlg; one in each DCR wrapper.
DLG = ["k_hen_%sdlg.nss" % n for n in
       "atton baodur discip g0t0 hanharr hk47 hndmaid kreia mand mira remote t3m4 visas".split()]
E2_DLG = [
    ("E2a", "party", "    int nMatch = GetListenPatternNumber();\n",
     "    int nMatch = GetListenPatternNumber();\n"
     "    //Party AI fix E2: a shout goes through k_ai_master case 2004, which is this script's shout path line for\n"
     "    //line (AI-off and commandable checks, GN_RespondToShout, user-defined 1004), so it runs the live party\n"
     "    //AI (stance 16, the fixes). Conversations (nMatch == -1) stay here for SelectAndRunDLG.\n"
     "    if(nMatch != -1)\n"
     "    {\n"
     "        ExecuteScript(\"k_ai_master\", OBJECT_SELF, 2004); //KOTOR_HENCH_EVENT_ON_DIALOGUE\n"
     "        return;\n"
     "    }\n"),
    ("E2b", "party", "                GN_RespondToShout(oShouter, nMatch, GetLastHostileActor(oShouter));\n",
     "                //Party AI fix E2: unreachable now (shouts return above); kept empty so this script no longer\n"
     "                //embeds its own copy of the vanilla combat AI.\n"),
]
E2_WRAP = {
    "a_force_combat.nss": [("E2c", "all", "    GN_DetermineCombatRound();\n",
                            "    //Party AI fix E2: run the live combat AI (k_ai_master case 3001 = GN_DetermineCombatRound()).\n"
                            "    ExecuteScript(\"k_ai_master\", OBJECT_SELF, 3001);\n")],
    "k_combat_rnd.nss": [("E2d", "all", "    GN_DetermineCombatRound();\n",
                          "    //Party AI fix E2: run the live combat AI (k_ai_master case 3001 = GN_DetermineCombatRound()).\n"
                          "    ExecuteScript(\"k_ai_master\", OBJECT_SELF, 3001);\n")],
    "a_atkonend.nss": [("E2e", "all", "    DelayCommand(0.3,GN_DetermineCombatRound());\n",
                        "    //Party AI fix E2: run the live combat AI (k_ai_master case 3001 = GN_DetermineCombatRound()).\n"
                        "    DelayCommand(0.3, ExecuteScript(\"k_ai_master\", OBJECT_SELF, 3001));\n")],
}


def apply(text, patches, fname, report):
    for pid, scope, *rest in patches:
        if len(rest) == 3:
            f, old, new = rest
            if f != fname:
                continue
        else:
            old, new = rest
        old_c, new_c = crlf(old), crlf(new)
        n = text.count(old_c)
        report.append((pid, scope, fname, n))
        if n != 1:
            sys.exit(f"{pid}: anchor found {n} times in {fname} (need 1): {old[:90]!r}")
        text = text.replace(old_c, new_c)
    return text


def compile_ncs(out, *nss):
    if os.path.exists(out):
        os.remove(out)
    r = subprocess.run([KIT, out, *nss], cwd=HERE, capture_output=True, text=True)
    if not os.path.exists(out):
        print(r.stdout[-3000:], r.stderr[-3000:])
        sys.exit(f"compile failed: {out}")
    return out


def main():
    src = os.path.join(OUT, "src"); ncs = os.path.join(OUT, "ncs")
    shutil.rmtree(OUT, ignore_errors=True)
    os.makedirs(src); os.makedirs(ncs)
    report = []

    # 1. includes + k_ai_master
    gen = apply(open(os.path.join(VAN, G), newline="").read(), PATCHES, G, report)
    gen = gen.rstrip("\r\n") + "\r\n\r\n" + crlf(open(os.path.join(HERE, "kite_routine_v2.nss")).read())
    gen = gen.rstrip("\r\n") + "\r\n\r\n" + crlf(open(os.path.join(HERE, "party_ai_fixes.nss")).read())
    sup = apply(open(os.path.join(VAN, S), newline="").read(), PATCHES, S, report)
    mas = apply(open(os.path.join(VAN, M), newline="").read(), PATCHES, M, report)
    for name, text in ((G, gen), (S, sup), (M, mas)):
        open(os.path.join(src, name), "w", newline="").write(text)
    inc = [os.path.join(src, G), os.path.join(src, S)]
    built = [compile_ncs(os.path.join(ncs, "k_ai_master.ncs"), os.path.join(src, M), *inc)]

    # 2. E2 routing scripts (compiled against the VANILLA includes: they no longer call the combat AI)
    for name in DLG:
        t = apply(open(os.path.join(VAN_ALL, name), newline="").read(), E2_DLG, name, report)
        open(os.path.join(src, name), "w", newline="").write(t)
        built.append(compile_ncs(os.path.join(ncs, name[:-4] + ".ncs"), os.path.join(src, name)))
    for name, pl in E2_WRAP.items():
        t = apply(open(os.path.join(VAN_ALL, name), newline="").read(), pl, name, report)
        open(os.path.join(src, name), "w", newline="").write(t)
        built.append(compile_ncs(os.path.join(ncs, name[:-4] + ".ncs"), os.path.join(src, name)))

    for pid, scope, fname, n in report:
        print(f"anchor ok  {pid:14s} {scope:22s} {fname:22s} count={n}")
    for p in built:
        print(f"built {os.path.basename(p):22s} {os.path.getsize(p):7d} bytes  md5 {md5(p)}")

    # 3. arg-order check on everything built
    chk = os.path.join(OUT, "checkroot")
    os.makedirs(os.path.join(chk, "override"))
    for p in built:
        shutil.copy(p, os.path.join(chk, "override"))
    r = subprocess.run([VENV_PY, os.path.join(REPO, "scripts", "check_ncs_arg_order.py"), chk, "--override-only"],
                       capture_output=True, text=True, cwd=HERE)
    print(r.stdout.strip()[-1500:])
    if r.returncode != 0:
        sys.exit("check_ncs_arg_order.py reported a problem")


if __name__ == "__main__":
    main()
