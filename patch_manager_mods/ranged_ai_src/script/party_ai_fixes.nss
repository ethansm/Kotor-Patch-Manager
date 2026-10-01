//::///////////////////////////////////////////////
//:: Party AI script fixes: helper bodies (appended after the kite routine)
//:: Design: scratchpad design_ai_script_fixes.md; review ai_review_2026-10-01
//:://////////////////////////////////////////////

// E1 (party only): stances 10-13 and 16 return straight after their routine, so they never reach
// GN_DoPostDCRChecks, the only party combat exit (CancelCombat + follow the leader); Aggressive does.
// This runs it for them, but only when the fight is OVER (S1). It is left alone, and the stance routine runs, while:
//  - the member is in a conversation, not commandable, dead or knocked out;
//  - a live enemy is the intruder of this call, the leader's attack target, or the member's last hostile actor
//    within 30 m (unseen shooters, enemies behind cover and stealthed ones are not "gone");
//  - an enemy is still seen (as before).
// With no enemy seen and none of the above: cure poison and heal first, as the Default AI does, else run the exit.
// Returns TRUE when the caller (GN_DetermineCombatRound) must return.
int GN_PartyStanceCombatExit(int nNPC_AI, object oIntruder)
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
    if (GetIsConversationActive() || GetIsInConversation(OBJECT_SELF) || !GetCommandable() ||
        GetIsDead(OBJECT_SELF) || GetCurrentHitPoints(OBJECT_SELF) < 1)
    {
        return FALSE;
    }
    if (GetIsObjectValid(GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, 1,
                                            CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN)))
    {
        return FALSE;
    }
    // S1: no enemy is SEEN, but the fight can still be on.
    object oLeaderTarget = GetAttackTarget(GetPartyMemberByIndex(0));
    object oAttacker = GetLastHostileActor(OBJECT_SELF);
    if ((GetIsObjectValid(oIntruder) && !GetIsDead(oIntruder) && GetIsEnemy(oIntruder)) ||
        (GetIsObjectValid(oLeaderTarget) && !GetIsDead(oLeaderTarget) && GetIsEnemy(oLeaderTarget)) ||
        (GetIsObjectValid(oAttacker) && !GetIsDead(oAttacker) && GetIsEnemy(oAttacker) &&
         GetDistanceToObject(oAttacker) <= 30.0))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E1: skipped, an unseen enemy is still fighting");
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
// nothing after the move. Ranged (11), Stationary (12) and Kite (16) never approach (user decision; the same
// rule as E11): they hold. The others use the weapon stand-off (4 m melee, 15 m otherwise) and plan a combat
// round on arrival.
void GN_PartyApproachUnseen(object oIntruder)
{
    int nStyle = GetNPCAIStyle(OBJECT_SELF);
    if (nStyle == NPC_AISTYLE_PARTY_RANGED || nStyle == NPC_AISTYLE_PARTY_STATIONARY || nStyle == NPC_AISTYLE_PARTY_KITE)
    {
        GN_MyPrintString("GENERIC DEBUG *************** Party E3: unseen intruder, stance " + GN_ITS(nStyle) + " holds");
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
