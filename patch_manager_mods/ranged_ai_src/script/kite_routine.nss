//::///////////////////////////////////////////////
//:: Mod 8 "Ranged (Kiting)" party AI (aistate 16)
//:: Doc: patch_manager_mods/13_ranged_ai_spacing.md sec. 3 (local-setup/KOTOR-II)
//:: v2 (party AI script fixes): K1 target check, K2 kite streak cap,
//:: K3 threat scan of the 3 nearest enemies, E12 fresh-fight reset.
//:://////////////////////////////////////////////
/*
    Party members (not the leader) on stance 16 with a ranged weapon:
      1. Kite: one of the 3 nearest seen live enemies within 3.5 m fights in
         melee (or any of them while we are below 60% HP) => step to a random
         walkable spot that is at least 2 m further from it, within 10 m of
         the leader and with a clear shot at our target (and forget the "hit
         in melee" mark). At most KITE_MAX_STREAK kites in a row: then one
         DCR call shoots instead (K2).
      2. Cornered (no such spot) => the vanilla Ranged Support weapon rule:
         swap to the melee config if we were hit in melee recently (the
         vanilla rule now requires a melee weapon in config 2, fix M1).
      3. Blocked shot (HasLineOfSight with both eye heights and both
         objects, the same test the engine uses) => move to a spot with a
         clear shot within 15 m of the target and 10 m of the leader, and
         outside the kite radius of the nearest enemy.
      4. Otherwise the default attack routine.
    The target must be a live enemy (K1); anything else is dropped and
    steps 2 and 4 handle the round the vanilla way.
    A move is always followed by a queued ActionAttack, so the creature
    never idles after it arrives. The engine half (ranged-ai DLL) keeps a
    blocked creature from walking into melee instead of shooting.
    Locals (vanilla uses none of 20/21): 20 = SW_NUMBER_KITE_COOLDOWN keeps
    a move from being re-planned at once (counts GN_DetermineCombatRound
    calls, not rounds); 21 = SW_NUMBER_KITE_STREAK counts kites in a row.
    Both are cleared when a fight starts (not in combat at entry, E12) and
    by the stance combat exit (GN_PartyStanceCombatExit, E1). Local 8
    ("hit in melee", set by k_ai_master 2005) is zeroed by the engine at
    combat exit (SetCombatState 0x57A5C1) and by every kite here.
    With ranged-ai 0.3.0 (kite_approach=1) the engine walks stance 16 to a target
    that is out of range or holds a melee weapon; the 15 m spot filter below keeps a
    kite move inside every ranged base item's attack range, so a kite never triggers it.
*/

int SW_NUMBER_KITE_COOLDOWN = 20;
int SW_NUMBER_KITE_STREAK = 21;
float KITE_THREAT_RANGE = 3.5;
float KITE_LEASH = 10.0;
float KITE_SHOT_RANGE = 15.0;
int KITE_TRIES = 12;
int KITE_MAX_STREAK = 2;
int KITE_THREAT_SCAN = 3;

vector vKiteSpot;   // result of GN_KiteFindSpot

vector GN_KiteEye(vector v)
{
    return Vector(v.x, v.y, v.z + 1.5);
}

// The engine's attack LOS test: foot position + 1.5 m on both ends, both objects excluded.
int GN_KiteHasShot(vector vFrom, object oTarget)
{
    return HasLineOfSight(GN_KiteEye(vFrom), GN_KiteEye(GetPosition(oTarget)), OBJECT_SELF, oTarget);
}

float GN_KiteDist(vector a, vector b)
{
    return VectorMagnitude(a - b);
}

// K3: the nearest of the KITE_THREAT_SCAN nearest seen enemies that is alive, within KITE_THREAT_RANGE
// and fights in melee (any weapon when bHurt). GetNearestCreature is sorted by distance, so the scan
// stops at the first one out of range.
object GN_KiteFindThreat(int bHurt)
{
    int i;
    for (i = 1; i <= KITE_THREAT_SCAN; i++)
    {
        object oCheck = GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, i,
                                           CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN);
        if (!GetIsObjectValid(oCheck) || GetDistanceBetween(OBJECT_SELF, oCheck) > KITE_THREAT_RANGE)
        {
            return OBJECT_INVALID;
        }
        if (!GetIsDead(oCheck))
        {
            object oCheckWeapon = GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON, oCheck);
            if (bHurt || !GetIsObjectValid(oCheckWeapon) ||
                GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oCheckWeapon)) != 2)
            {
                return oCheck;
            }
        }
    }
    return OBJECT_INVALID;
}

// nMode 1 = away from oThreat (keep the spot furthest from it), 2 = regain a shot at oTarget (keep the nearest spot).
// Returns TRUE and sets vKiteSpot when a spot is found.
int GN_KiteFindSpot(int nMode, object oThreat, object oTarget, object oLeader)
{
    vector vMe = GetPosition(OBJECT_SELF);
    vector vLeader = GetPosition(oLeader);
    vector vThreat = vMe;
    float fThreatNow = 0.0;
    if (GetIsObjectValid(oThreat))
    {
        vThreat = GetPosition(oThreat);
        fThreatNow = GN_KiteDist(vMe, vThreat);
    }
    int nFound = FALSE;
    float fBest = 0.0;
    int i;
    for (i = 0; i < KITE_TRIES; i++)
    {
        vector v = GetRandomDestination(OBJECT_SELF, (nMode == 1) ? 8 : 10);
        int nOk = TRUE;
        if (GetIsObjectValid(oLeader) && oLeader != OBJECT_SELF && GN_KiteDist(v, vLeader) > KITE_LEASH)
        {
            nOk = FALSE;
        }
        float fThreat = 99.0;
        if (nOk && GetIsObjectValid(oThreat))
        {
            fThreat = GN_KiteDist(v, vThreat);
            // at least 2 m further and outside the kite trigger radius
            if (nMode == 1 && (fThreat < fThreatNow + 2.0 || fThreat < KITE_THREAT_RANGE + 0.5))
            {
                nOk = FALSE;
            }
            if (nMode == 2 && fThreat < KITE_THREAT_RANGE + 0.5)   // never land inside the kite trigger radius
            {
                nOk = FALSE;
            }
        }
        // stay in shot range (15 m; every ranged base item reaches 17 m or more): a kite move must not make the
        // engine approach the target (ranged-ai 0.3.0 kite_approach)
        if (nOk && GN_KiteDist(v, GetPosition(oTarget)) > KITE_SHOT_RANGE)
        {
            nOk = FALSE;
        }
        if (nOk && !GN_KiteHasShot(v, oTarget))
        {
            nOk = FALSE;
        }
        if (nOk)
        {
            float fScore = (nMode == 1) ? fThreat : -GN_KiteDist(v, vMe);
            if (!nFound || fScore > fBest)
            {
                nFound = TRUE;
                fBest = fScore;
                vKiteSpot = v;
            }
        }
    }
    return nFound;
}

void GN_KiteMoveThenAttack(object oTarget, int nCooldown)
{
    ClearAllActions();
    ActionMoveToLocation(Location(vKiteSpot, GetFacing(OBJECT_SELF)), TRUE);
    ActionAttack(oTarget);
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, nCooldown);
    // We got away: forget the "hit in melee" mark (set by k_ai_master on attack, only counted down by
    // GN_CheckRangedPreferred), or the next round would swap to the melee config and walk back in.
    SetLocalNumber(OBJECT_SELF, SW_NUMBER_HEALERAI_THRESHOLD, 0);
}

int GN_RunKiteAIRoutine(object oIntruder)
{
    if (!IsObjectPartyMember(OBJECT_SELF))
    {
        return GN_RunDefaultAIRoutine(oIntruder);
    }
    object oLeader = GetPartyLeader();
    if (OBJECT_SELF == oLeader || GetCurrentHitPoints(OBJECT_SELF) < 1)
    {
        return 0;
    }

    // E12: a new fight starts without the last fight's cooldown or streak.
    if (!GetIsInCombat(OBJECT_SELF))
    {
        if (GetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN) != 0 || GetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK) != 0)
        {
            GN_MyPrintString("GENERIC DEBUG *************** Kite E12 reset: not in combat, cooldown/streak cleared");
        }
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, 0);
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, 0);
    }

    int nCool = GetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN);
    if (nCool > 0)
    {
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_COOLDOWN, nCool - 1);
        // Still walking to the last spot: let it finish (the attack is queued behind it).
        if (GetCurrentAction(OBJECT_SELF) == ACTION_MOVETOPOINT)
        {
            return 1;
        }
    }

    object oWeapon = GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON);
    int nRanged = GetIsObjectValid(oWeapon) && GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oWeapon)) == 2;
    object oTarget = GN_DetermineAttackTarget(oIntruder);
    // K1: only a live enemy is a target here. A neutral, a bashed door or placeable, or a corpse
    // (GetLastHostileTarget has no such checks) is dropped; steps 2 and 4 then run the vanilla way.
    if (GetIsObjectValid(oTarget) && (!GetIsEnemy(oTarget) || GetIsDead(oTarget)))
    {
        GN_MyPrintString("GENERIC DEBUG *************** Kite K1 drop target " + GN_ReturnDebugName(oTarget));
        oTarget = OBJECT_INVALID;
    }

    // 1. Kite away from a close melee threat.
    if (nRanged && nCool == 0)
    {
        int nHurt = GetCurrentHitPoints(OBJECT_SELF) * 10 < GetMaxHitPoints(OBJECT_SELF) * 6;
        object oThreat = GN_KiteFindThreat(nHurt);
        int nStreak = GetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK);
        if (GetIsObjectValid(oThreat) && nStreak < KITE_MAX_STREAK)
        {
            object oShootAt = GetIsObjectValid(oTarget) ? oTarget : oThreat;
            if (GN_KiteFindSpot(1, oThreat, oShootAt, oLeader))
            {
                GN_MyPrintString("GENERIC DEBUG *************** Kite step 1 streak " + GN_ITS(nStreak + 1) + " from " + GN_ReturnDebugName(oThreat));
                SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, nStreak + 1);
                GN_KiteMoveThenAttack(oShootAt, 2);
                return 1;
            }
        }
        // K2: no kite on this call (no threat, streak capped or no spot): the streak ends here, and
        // the attack queued below keeps a whole round before the next kite can start.
        if (GetIsObjectValid(oThreat) && nStreak >= KITE_MAX_STREAK)
        {
            GN_MyPrintString("GENERIC DEBUG *************** Kite K2 cap: shoot this round");
        }
        SetLocalNumber(OBJECT_SELF, SW_NUMBER_KITE_STREAK, 0);
    }

    // 2. Weapon config (cornered and hit in melee => melee config; safe again => ranged config), as Ranged Support.
    if (GN_CheckRangedPreferred())
    {
        return 0;
    }

    // 3. Blocked shot => find a spot with a clear shot.
    if (nRanged && nCool == 0 && GetIsObjectValid(oTarget) && !GetIsDead(oTarget) &&
        !GN_KiteHasShot(GetPosition(OBJECT_SELF), oTarget))
    {
        object oNear = GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, 1,
                                          CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN);
        if (GN_KiteFindSpot(2, oNear, oTarget, oLeader))
        {
            GN_MyPrintString("GENERIC DEBUG *************** Kite step 3 regain shot at " + GN_ReturnDebugName(oTarget));
            GN_KiteMoveThenAttack(oTarget, 1);
            return 1;
        }
    }

    // 4. Normal attack behaviour (re-issues ActionAttack every round).
    GN_RunDefaultAIRoutine(oIntruder);
    return 0;
}
