//::///////////////////////////////////////////////
//:: Party AI script fixes: helper bodies (appended after the kite routine)
//:: Design: scratchpad design_ai_script_fixes.md; review ai_review_2026-10-01
//:://////////////////////////////////////////////

// E1 (party only): stances 10-13 and 16 return straight after their routine, so they never reach
// GN_DoPostDCRChecks, the only party combat exit (CancelCombat + follow the leader); Aggressive does.
// When this member sees no enemy: cure poison and heal first, as the Default AI does, else run the exit.
// Returns TRUE when the caller (GN_DetermineCombatRound) must return.
int GN_PartyStanceCombatExit(int nNPC_AI)
{
    if (!IsObjectPartyMember(OBJECT_SELF) || GetPartyMemberByIndex(0) == OBJECT_SELF)
    {
        return FALSE;
    }
    if (nNPC_AI != NPC_AISTYLE_PARTY_DEFENSE && nNPC_AI != NPC_AISTYLE_PARTY_RANGED &&
        nNPC_AI != NPC_AISTYLE_PARTY_STATIONARY && nNPC_AI != NPC_AISTYLE_PARTY_SUPPORT &&
        nNPC_AI != NPC_AISTYLE_PARTY_KITE)
    {
        return FALSE;
    }
    if (GetIsObjectValid(GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, 1,
                                            CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN)))
    {
        return FALSE;
    }
    // E12: the kite cooldown and streak end with the fight.
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, 0);
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, 0);

    object oPoisoned = GN_CheckIfPoisoned();
    if (GetIsObjectValid(oPoisoned) && GN_TalentMasterRoutine(GEN_TALENT_REMOVE_POISON, oPoisoned))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E1: no enemy seen, cure poison first");
        return TRUE;
    }
    object oInjured = GN_CheckIfInjured();
    if (GetIsObjectValid(oInjured) && GN_TalentMasterRoutine(GEN_TALENT_HEALING, oInjured))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E1: no enemy seen, heal first");
        return TRUE;
    }
    GN_MyPrintString("GENERIC DEBUG *************** Party E1: no enemy seen, stance " + GN_ITS(nNPC_AI) + " combat exit");
    return GN_DoPostDCRChecks();
}

// E3/M6 (party only): GN_RespondToShout, shout 1, intruder valid but not seen. The vanilla code ran every
// such member to 2 m of the intruder (the "< 20 m and not seen" test is always true there) and queued
// nothing after the move. Use the weapon stand-off (4 m melee, 15 m otherwise), never move on Stationary,
// and plan a combat round on arrival.
void GN_PartyApproachUnseen(object oIntruder)
{
    if (GetNPCAIStyle(OBJECT_SELF) == NPC_AISTYLE_PARTY_STATIONARY)
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E3: unseen intruder, Stationary holds");
        return;
    }
    float fRange = 15.0;
    if (GN_GetWeaponType(OBJECT_SELF) == 1)
    {
        fRange = 4.0;
    }
    GN_MyPrintString("GENERIC DEBUG *************** Party E3: unseen intruder, stand off " + FloatToString(fRange, 4, 1));
    ActionMoveToObject(oIntruder, TRUE, fRange);
    ActionDoCommand(GN_DetermineCombatRound(oIntruder));
}
