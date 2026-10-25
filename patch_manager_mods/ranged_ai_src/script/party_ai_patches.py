#!/usr/bin/env python3
"""Party AI script fixes: the anchored patch list (data) plus the helper that applies it.

Design: patch_manager_mods/mods678_research/ai_review_2026-10-01/designs/design_ai_script_fixes.md (section 6 lists
every entry; the ids map to that document and to the review reports). Used by build_party_ai_scripts.py.

Each patch: (id, scope, file, old, new). `old` must occur EXACTLY once in the (CRLF) vanilla source and is
replaced whole by `new`. Texts are written with LF here and converted to CRLF by apply(). The patches are applied
top to bottom and no two of them overlap. Party-only: every vanilla fix is gated on IsObjectPartyMember(OBJECT_SELF)
(or sits in a path only party members reach); scope "all" (the E2 wrappers) leaves NPC behaviour identical.

Medpac rule (user decision 2026-10-01): a party member never selects the medpac/repair-kit talent (tSpe11_05) in
the healing selection (MEDPAC-a/b), and never the antidote talent (ANTIDOTE) when curing poison. A later iteration may allow it depending on the party's stock, and only when no
heal power is available.
"""


def crlf(s):
    return s.replace("\r\n", "\n").replace("\n", "\r\n")


# --------------------------------------------------------------------------------------------------
# Mod 8 base insertions (what build_kite_script.py inserted: the live k_ai_master.ncs 94db8304) + the new prototypes
# --------------------------------------------------------------------------------------------------
PROTO_ANCHOR = "int GN_CheckRangedPreferred();\n"
MOD8_PROTOS = (
    "//Mod 8 (ranged-ai) - party stance 16 \"Ranged (Kiting)\": keeps its distance, repositions when the\n"
    "//shot is blocked, switches to melee if cornered. Body at the end of this file.\n"
    "int NPC_AISTYLE_PARTY_KITE = 16;\n"
    "int GN_RunKiteAIRoutine(object oIntruder = OBJECT_INVALID);\n")
PARTY_PROTOS = (
    "//Party AI script fixes (design_ai_script_fixes.md). Bodies at the end of this file.\n"
    "int GN_PartyStanceCombatExit(int nNPC_AI, object oIntruder);\n"
    "void GN_PartyApproachUnseen(object oIntruder);\n"
    "int GN_PartyPowerFamily(int nSpell);\n"
    "int GN_PartyFamilyActive(int nSpell, object oTarget);\n")

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

DISPATCH_OLD = (
    "                    GN_RunRangedSupportAIRoutine(oIntruder);\n"
    "                    return;\n"
    "                }\n"
    "                else if (nNPC_AI == NPC_AISTYLE_PARTY_STATIONARY)\n")
DISPATCH_NEW = (
    "                    GN_RunRangedSupportAIRoutine(oIntruder);\n"
    "                    return;\n"
    "                }\n" + KITE_BRANCH +
    "                else if (nNPC_AI == NPC_AISTYLE_PARTY_STATIONARY)\n")

# The Mod 8 insertions alone (baseline proof: with the v1 kite routine they must rebuild the live k_ai_master.ncs).
MOD8_ONLY = [
    ("MOD8-proto", "party", "k_inc_generic.nss", PROTO_ANCHOR, PROTO_ANCHOR + MOD8_PROTOS),
    ("MOD8-dispatch", "party", "k_inc_generic.nss", DISPATCH_OLD, DISPATCH_NEW),
]

G = "k_inc_generic.nss"
S = "k_inc_gensupport.nss"
M = "k_ai_master.nss"

PATCHES = [
    # ---------------- k_inc_generic ----------------
    ("MOD8-proto", "party", G, PROTO_ANCHOR, PROTO_ANCHOR + MOD8_PROTOS + PARTY_PROTOS),
    ("MOD8-dispatch", "party", G, DISPATCH_OLD, DISPATCH_NEW),
    ("E1", "party", G,
     "                if(nNPC_AI == NPC_AISTYLE_MELEE_ATTACK)\n",
     "                //Party AI fix E1 (party only): stances 10-13 and 16 never reach GN_DoPostDCRChecks below; it runs\n"
     "                //only when the fight is over (S1: no live enemy as intruder, leader target or recent attacker).\n"
     "                if(GN_PartyStanceCombatExit(nNPC_AI, oIntruder) == TRUE)\n"
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
     "        //Party AI fix H2/S2: race test on the member itself (was oP0 for all three), and only members the heal\n"
     "        //powers reach (Heal and Master Heal act on non-droid party members within 15 m of the caster).\n"
     "        if(GetIsObjectValid(oP1)&& GetRacialType(oP1) != RACIAL_TYPE_DROID && GetDistanceBetween(OBJECT_SELF, oP1) < 15.0)\n"),
    ("H2b", "party", G,
     "        if(GetIsObjectValid(oP2) && GetRacialType(oP0) != RACIAL_TYPE_DROID)\n",
     "        if(GetIsObjectValid(oP2) && GetRacialType(oP2) != RACIAL_TYPE_DROID && GetDistanceBetween(OBJECT_SELF, oP2) < 15.0)\n"),
    ("H2e", "party", G,
     "        if(GetIsObjectValid(oP0) && GetRacialType(oP0) != RACIAL_TYPE_DROID)\n",
     "        //Party AI fix S2: the leader too counts only within the 15 m reach of the heal powers.\n"
     "        if(GetIsObjectValid(oP0) && GetRacialType(oP0) != RACIAL_TYPE_DROID && GetDistanceBetween(OBJECT_SELF, oP0) < 15.0)\n"),
    ("H2c", "party", G,
     "        else if(GetIsObjectValid(oP1) && !GetIsDead(oP1) && (fDown00 < 0.5 && fDown00 > 0.0))\n",
     "        //Party AI fix H2: each branch tests its own member's ratio (all three tested fDown00).\n"
     "        else if(GetIsObjectValid(oP1) && !GetIsDead(oP1) && (fDown01 < 0.5 && fDown01 > 0.0))\n"),
    ("H2d", "party", G,
     "        else if(GetIsObjectValid(oP2) && !GetIsDead(oP2) && (fDown00 < 0.5 && fDown00 > 0.0))\n",
     "        else if(GetIsObjectValid(oP2) && !GetIsDead(oP2) && (fDown02 < 0.5 && fDown02 > 0.0))\n"),
    ("M5", "party", G,
     "        int nCnt = 0;\n"
     "        for(nCnt; nCnt > 2; nCnt++)\n"
     "        {\n"
     "            ePoison = GetFirstEffect(GetPartyMemberByIndex(nCnt));\n"
     "            while(GetIsEffectValid(ePoison))\n"
     "            {\n"
     "                if(GetEffectType(ePoison) == EFFECT_TYPE_POISON)\n"
     "                {\n"
     "                    return GetPartyMemberByIndex(nCnt);\n"
     "                }\n"
     "                ePoison = GetNextEffect(GetPartyMemberByIndex(nCnt));\n"
     "            }\n"
     "        }\n",
     "        //Party AI fix M5/B1: the loop never ran (nCnt = 0 is never > 2). It now reports only members a poison\n"
     "        //cure reaches: Heal and Master Heal remove poison from NON-DROID party members within 15 m of the caster\n"
     "        //(k_inc_force), and Cure does not remove poison in TSL (see M5b). Anyone else is never reported, so the\n"
     "        //selection cannot recast a power that leaves the poison in place.\n"
     "        int nCnt = 0;\n"
     "        for(nCnt = 0; nCnt < 3; nCnt++)\n"
     "        {\n"
     "            object oPM5 = GetPartyMemberByIndex(nCnt);\n"
     "            if(GetIsObjectValid(oPM5) && !GetIsDead(oPM5) && GetRacialType(oPM5) != RACIAL_TYPE_DROID &&\n"
     "               GetDistanceBetween(OBJECT_SELF, oPM5) < 15.0)\n"
     "            {\n"
     "                ePoison = GetFirstEffect(oPM5);\n"
     "                while(GetIsEffectValid(ePoison))\n"
     "                {\n"
     "                    if(GetEffectType(ePoison) == EFFECT_TYPE_POISON)\n"
     "                    {\n"
     "                        return oPM5;\n"
     "                    }\n"
     "                    ePoison = GetNextEffect(oPM5);\n"
     "                }\n"
     "            }\n"
     "        }\n"),
    ("M5b", "party", G,
     "            tSpe11_03 = TalentSpell(67); //Remove Poison Item\n",
     "            tSpe11_03 = TalentSpell(67); //Remove Poison Item\n"
     "            //Party AI fix M5b/B1: Cure heals but never removes poison in TSL (k_inc_force FORCE_POWER_CURE: the poison\n"
     "            //removal is commented out), so a party member never offers it for poison; it would recast every round\n"
     "            //with the poison still on. Heal and Master Heal stay (they do remove it).\n"
     "            if(IsObjectPartyMember(OBJECT_SELF))\n"
     "            {\n"
     "                tSpe11_02 = TalentSpell(-1);\n"
     "            }\n"),
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
    # User decision 2026-10-01: no medpacs / repair kits for party members (H2 makes any member under 50% a heal
    # target, so a companion without a heal power must neither fall back to items nor hit an invalid talent).
    # Both patches skip tSpe11_05 for party members; bValid stays FALSE, tUse stays an invalid talent, so the
    # generic fallback (GetIsTalentValid(tUse)) is closed too and GN_TalentMasterRoutine returns FALSE: no idle,
    # the other actions run. Future rule: medpacs may later be allowed from the party's stock, and only when no heal
    # power is available.
    ("MEDPAC-a", "party", G,
     "            else if(GetCreatureHasTalent(tSpe11_05) ||\n"
     "                    GetCreatureHasTalent(tSpe11_06) ||\n"
     "                    GetCreatureHasTalent(tSpe11_07))\n",
     "            //Party AI rule MEDPAC (user decision 2026-10-01): a party member never takes the medpac / repair kit\n"
     "            //(tSpe11_05) branch. Drain Life and Death Field are NPC-only below, so the whole branch is skipped for the\n"
     "            //party: bValid stays FALSE and the routine returns FALSE (no talent-less idle). Future rule: medpacs may\n"
     "            //be allowed from the party's stock, and only when no heal power is available.\n"
     "            else if(!IsObjectPartyMember(OBJECT_SELF) &&\n"
     "                    (GetCreatureHasTalent(tSpe11_05) ||\n"
     "                     GetCreatureHasTalent(tSpe11_06) ||\n"
     "                     GetCreatureHasTalent(tSpe11_07)))\n"),
    ("MEDPAC-b", "party", G,
     "                        GN_MyPrintString(\"GENERIC DEBUG *************** I have a Med Pack\");\n"
     "                        bValid = TRUE;\n"
     "                        tUse = tSpe11_05;\n",
     "                        //Party AI rule MEDPAC (defence in depth; unreachable for the party after MEDPAC-a).\n"
     "                        if(IsObjectPartyMember(OBJECT_SELF))\n"
     "                        {\n"
     "                            GN_MyPrintString(\"GENERIC DEBUG *************** Party MEDPAC: never uses medpacs or repair kits\");\n"
     "                            bValid = FALSE;\n"
     "                        }\n"
     "                        else\n"
     "                        {\n"
     "                            GN_MyPrintString(\"GENERIC DEBUG *************** I have a Med Pack\");\n"
     "                            bValid = TRUE;\n"
     "                            tUse = tSpe11_05;\n"
     "                        }\n"),
    # Same decision for poison: the Force powers only. M5 makes the poison loop run, and the item branch of
    # GEN_TALENT_REMOVE_POISON would take "Remove Poison Item" (TalentSpell(67), an antidote) from the shared inventory.
    # For the party that branch is closed: bValid stays FALSE, tUse stays invalid, the routine returns FALSE.
    ("ANTIDOTE", "party", G,
     "            else if(GetCreatureHasTalent(tSpe11_03) && oTarget == OBJECT_SELF)\n"
     "            {\n"
     "                bValid = TRUE;\n"
     "                tUse = tSpe11_03;\n"
     "            }\n"
     "            //nTalent = 0xff4f;\n",
     "            //Party AI rule ANTIDOTE (user decision 2026-10-01, same as MEDPAC): a party member cures poison with Force\n"
     "            //powers only (Heal and Master Heal above; Cure is off for the party, M5b), never with the Remove Poison Item talent (an antidote from\n"
     "            //the shared inventory). Closed for the party: bValid stays FALSE and the routine returns FALSE (no\n"
     "            //talent-less idle). Future rule: items may be allowed from the party's stock, and only when no Force\n"
     "            //power is available.\n"
     "            else if(!IsObjectPartyMember(OBJECT_SELF) && GetCreatureHasTalent(tSpe11_03) && oTarget == OBJECT_SELF)\n"
     "            {\n"
     "                bValid = TRUE;\n"
     "                tUse = tSpe11_03;\n"
     "            }\n"
     "            //nTalent = 0xff4f;\n"),
    # F2 (party only): family-aware buff and debuff picks. F2a skips a buff whose family (any tier or sibling) is
    # already on the caster, and the invalid TalentSpell(-1) that the exact-id check leaves behind. F2b drops a force
    # power whose family is already on the target, so the caller falls through to its next action.
    ("F2a", "party", G,
     "            if ( GetIsTalentValid( tTemp ) )\n"
     "            {\n"
     "                int nCost = GetSpellBaseForcePointCost(GetIdFromTalent(tTemp));\n",
     "            //Party AI fix F2 (party only): the exact-id check above misses other tiers and leaves TalentSpell(-1),\n"
     "            //which still counts as valid, so vanilla casts an invalid talent. A buff whose family is already on the\n"
     "            //caster (any tier or sibling) or an invalid talent is skipped here and the routine falls through.\n"
     "            int bPartySkip = FALSE;\n"
     "            if(IsObjectPartyMember(OBJECT_SELF) && GetIsTalentValid( tTemp ) &&\n"
     "               (GetIdFromTalent(tTemp) < 0 || GN_PartyFamilyActive(GetIdFromTalent(tTemp), OBJECT_SELF)))\n"
     "            {\n"
     "                GN_MyPrintString(\"GENERIC DEBUG *************** Party F2: buff family active, no buff cast\");\n"
     "                bPartySkip = TRUE;\n"
     "            }\n"
     "            if ( GetIsTalentValid( tTemp ) && bPartySkip == FALSE )\n"
     "            {\n"
     "                int nCost = GetSpellBaseForcePointCost(GetIdFromTalent(tTemp));\n"),
    ("F2b", "party", G,
     "talent GN_CheckNonDroidForcePower(object oTarget, talent tUse)\n{\n    int bSwitch = FALSE;\n",
     "talent GN_CheckNonDroidForcePower(object oTarget, talent tUse)\n{\n    int bSwitch = FALSE;\n"
     "    //Party AI fix F2 (party only): a force power whose family is already active on the target (buffs: the caster;\n"
     "    //debuffs: the target) is dropped here, so the caller falls through to its next action instead of a wasted round.\n"
     "    if(IsObjectPartyMember(OBJECT_SELF) && GetTypeFromTalent(tUse) == TALENT_TYPE_FORCE && GN_PartyFamilyActive(GetIdFromTalent(tUse), oTarget))\n"
     "    {\n"
     "        GN_MyPrintString(\"GENERIC DEBUG *************** Party F2: family active, dropped spell \" + GN_ITS(GetIdFromTalent(tUse)));\n"
     "        talent tNone;\n"
     "        return tNone;\n"
     "    }\n"),
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
     "    }\n"
     "    else\n"
     "    {\n"
     "        GN_MyPrintString(\"GENERIC DEBUG *************** Party M4: targeted force power, droid flag \" + GN_ITS(nDroid));\n"
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
     "            //Party members only (S3): other creatures with k_hen_* scripts keep the vanilla test.\n"
     "            if ( nWeaponType == 1 || (nWeaponType == 0 && (!IsObjectPartyMember(OBJECT_SELF) ||\n"
     "                                      (GetIsObjectValid(oAttacker) && GetDistanceBetween(OBJECT_SELF, oAttacker) <= 5.0))) )\n"),
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
     "                                            //Party AI fix E11/M6/S3/S4: party members stand off by weapon as case 1006 does (was a\n"
     "                                            //run onto the damager's spot); Ranged, Stationary and Kite do not approach at all.\n"
     "                                            //Other creatures with k_hen_* scripts keep the vanilla run.\n"
     "                                            if(!IsObjectPartyMember(OBJECT_SELF))\n"
     "                                            {\n"
     "                                                ActionMoveToLocation(GetLocation(oTarget), TRUE);\n"
     "                                                ActionDoCommand(GN_DetermineCombatRound());\n"
     "                                            }\n"
     "                                            else\n"
     "                                            {\n"
     "                                                int nStyle2006 = GetNPCAIStyle(OBJECT_SELF);\n"
     "                                                if(nStyle2006 != NPC_AISTYLE_PARTY_RANGED && nStyle2006 != NPC_AISTYLE_PARTY_STATIONARY &&\n"
     "                                                   nStyle2006 != NPC_AISTYLE_PARTY_KITE)\n"
     "                                                {\n"
     "                                                    GN_MyPrintString(\"GENERIC DEBUG *************** Party E11: unseen damager, stand off\");\n"
     "                                                    ActionMoveToObject(oTarget, TRUE, (GN_GetWeaponType(OBJECT_SELF) == 1) ? 4.0 : 15.0);\n"
     "                                                    ActionDoCommand(GN_DetermineCombatRound());\n"
     "                                                }\n"
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
    """Apply every patch that targets `fname` (patch tuples with 5 fields) or, for the E2 lists (4 fields: no file),
    all of them. Asserts each anchor occurs exactly once. `report` collects (id, scope, file, count)."""
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
            raise SystemExit(f"{pid}: anchor found {n} times in {fname} (need 1): {old[:90]!r}")
        text = text.replace(old_c, new_c)
    return text
