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

// F2 (party only): power family of a spell, 0 = untracked. Buff families 1-11 are self buffs (checked on the
// caster); families 20-24 are single-target debuffs (checked on the target). Ids are spells.2da rows (labels
// checked 2026-10-25). Kill (32) is instant damage, Horror (30) and Insanity (31) are area powers: untracked.
int GN_PartyPowerFamily(int nSpell)
{
    // 1 Speed
    if (nSpell == 8 || nSpell == 34 || nSpell == 37)
    {
        return 1;
    }
    // 2 Armor
    if (nSpell == 17 || nSpell == 18 || nSpell == 24)
    {
        return 2;
    }
    // 3 Valor
    if (nSpell == 22 || nSpell == 33 || nSpell == 36)
    {
        return 3;
    }
    // 4 Energy Resist
    if (nSpell == 40 || nSpell == 42 || nSpell == 133)
    {
        return 4;
    }
    // 5 Barrier
    if (nSpell == 135 || nSpell == 136 || nSpell == 137)
    {
        return 5;
    }
    // 6 Battle Meditation (PC)
    if (nSpell == 138 || nSpell == 139 || nSpell == 140)
    {
        return 6;
    }
    // 7 Fury
    if (nSpell == 164 || nSpell == 165 || nSpell == 166)
    {
        return 7;
    }
    // 8 Inspire Followers
    if (nSpell == 167 || nSpell == 168 || nSpell == 169 || nSpell == 170 || nSpell == 171 || nSpell == 172)
    {
        return 8;
    }
    // 9 Force Body
    if (nSpell == 150 || nSpell == 151 || nSpell == 152)
    {
        return 9;
    }
    // 10 Force Immunity / Resist Force
    if (nSpell == 20 || nSpell == 41)
    {
        return 10;
    }
    // 11 Force Enlightenment (applies Speed, Armor and Valor)
    if (nSpell == 180)
    {
        return 11;
    }
    // 20 Slow / Affliction / Plague
    if (nSpell == 45 || nSpell == 7 || nSpell == 38)
    {
        return 20;
    }
    // 21 Wound / Choke (Kill 32 is instant damage, untracked)
    if (nSpell == 50 || nSpell == 9)
    {
        return 21;
    }
    // 22 Fear (Horror 30 and Insanity 31 are area powers, untracked)
    if (nSpell == 16)
    {
        return 22;
    }
    // 23 Stun / Stasis
    if (nSpell == 46 || nSpell == 29)
    {
        return 23;
    }
    // 24 Drain Force
    if (nSpell == 153 || nSpell == 154 || nSpell == 155)
    {
        return 24;
    }
    return 0;
}

// TRUE when an active effect of family nActive satisfies family nFamily (Enlightenment 11 covers Speed 1, Armor 2, Valor 3).
int GN_PartyFamilyCovers(int nActive, int nFamily)
{
    if (nActive == nFamily)
    {
        return TRUE;
    }
    if (nActive == 11 && nFamily >= 1 && nFamily <= 3)
    {
        return TRUE;
    }
    return FALSE;
}

// F2 (party only): TRUE when a power of the same family as nSpell is already on oTarget. Buff families check
// OBJECT_SELF (the caster), debuff families check the target. Untracked spells and invalid targets give FALSE.
int GN_PartyFamilyActive(int nSpell, object oTarget)
{
    int nFamily = GN_PartyPowerFamily(nSpell);
    if (nFamily == 0)
    {
        return FALSE;
    }
    object oCheck = OBJECT_SELF;
    if (nFamily >= 20)
    {
        oCheck = oTarget;
    }
    if (!GetIsObjectValid(oCheck))
    {
        return FALSE;
    }
    effect eCur = GetFirstEffect(oCheck);
    while (GetIsEffectValid(eCur))
    {
        if (GN_PartyFamilyCovers(GN_PartyPowerFamily(GetEffectSpellId(eCur)), nFamily))
        {
            return TRUE;
        }
        eCur = GetNextEffect(oCheck);
    }
    return FALSE;
}

// O3 (party only): TRUE when oTarget carries an effect whose spell id is nSpell (exact GetEffectSpellId match).
int GN_PartySpellEffectOn(int nSpell, object oTarget)
{
    effect eCur = GetFirstEffect(oTarget);
    while (GetIsEffectValid(eCur))
    {
        if (GetEffectSpellId(eCur) == nSpell)
        {
            return TRUE;
        }
        eCur = GetNextEffect(oTarget);
    }
    return FALSE;
}

// WI-174 (party only): TRUE when nOther is in the vanilla series of the lasting area power nSpell
// (GN_CheckSeriesFear / GN_CheckSeriesHold, k_inc_gensupport.nss): Fear 16, Horror 30, Insanity 31;
// Hold 29, Sleep 44 (in-game "Mass Stasis"), Stun 46. Not a power family: families 22/23 stay as they are.
int GN_PartySameSeries(int nSpell, int nOther)
{
    if (nSpell == nOther)
    {
        return TRUE;
    }
    if ((nSpell == 30 || nSpell == 31) && (nOther == 16 || nOther == 30 || nOther == 31))
    {
        return TRUE;
    }
    if (nSpell == 44 && (nOther == 29 || nOther == 44 || nOther == 46))
    {
        return TRUE;
    }
    return FALSE;
}

// WI-174 (party only): TRUE when oTarget carries an effect of nSpell's series.
int GN_PartySeriesEffectOn(int nSpell, object oTarget)
{
    effect eCur = GetFirstEffect(oTarget);
    while (GetIsEffectValid(eCur))
    {
        if (GN_PartySameSeries(nSpell, GetEffectSpellId(eCur)))
        {
            return TRUE;
        }
        eCur = GetNextEffect(oTarget);
    }
    return FALSE;
}

// O3 (party only): a lasting area power (Horror 30, Insanity 31, Mass Stasis 44) rolled for the AoE centre is dropped
// when fewer than 2 enemies within 4 m of that centre are free of its effect. The centre counts when it is free.
// A dropped pick is a null roll: the caller falls through. Force Storm 25 and Scream 159-161 are never filtered.
// Spec: WI-160 / O3_study.md section 6. The test is a series match (WI-174: Fear/Horror/Insanity, Hold/Sleep/Stun), still not a power family: a power family here would also make
// GN_CheckNonDroidForcePower (F2b) drop the pick whenever the centre alone is affected, which the >= 2 rule does not ask for.
talent GN_PartyAOEUnaffectedCheck(talent tUse, object oCentre)
{
    talent tNone;
    if (!IsObjectPartyMember(OBJECT_SELF) || !GetIsTalentValid(tUse) || GetTypeFromTalent(tUse) != TALENT_TYPE_FORCE ||
        !GetIsObjectValid(oCentre))
    {
        return tUse;
    }
    int nSpell = GetIdFromTalent(tUse);
    if (nSpell != 30 && nSpell != 31 && nSpell != 44)
    {
        return tUse;
    }
    int nFree = 0;
    location lCentre = GetLocation(oCentre);
    object oNear = GetFirstObjectInShape(SHAPE_SPHERE, 4.0, lCentre);
    while (GetIsObjectValid(oNear))
    {
        if (GetIsEnemy(oNear) && !GetIsDead(oNear) && !GN_PartySeriesEffectOn(nSpell, oNear))
        {
            nFree++;
        }
        oNear = GetNextObjectInShape(SHAPE_SPHERE, 4.0, lCentre);
    }
    if (nFree >= 2)
    {
        return tUse;
    }
    GN_MyPrintString("GENERIC DEBUG *************** Party D O3: spell " + GN_ITS(nSpell) + " dropped, " + GN_ITS(nFree) + " unaffected enemy within 4 m of the centre");
    return tNone;
}
