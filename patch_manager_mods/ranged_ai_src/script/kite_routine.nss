//::///////////////////////////////////////////////
//:: Mod 8 "Ranged (Kiting)" party AI (aistate 16)
//:: Doc: patch_manager_mods/13_ranged_ai_spacing.md sec. 3 (local-setup/KOTOR-II)
//:://////////////////////////////////////////////
/*
    Party members (not the leader) on stance 16 with a ranged weapon:
      1. Kite: an enemy that fights in melee (or any enemy while we are below
         60% HP) is within 3.5 m => step to a random walkable spot that is
         at least 2 m further from it, within 10 m of the leader and with a
         clear shot at our target (and forget the "hit in melee" mark).
      2. Cornered (no such spot) => the vanilla Ranged Support weapon rule:
         swap to the melee config if we were hit in melee recently.
      3. Blocked shot (HasLineOfSight with both eye heights and both
         objects, the same test the engine uses) => move to a spot with a
         clear shot within 15 m of the target and 10 m of the leader, and
         outside the kite radius of the nearest enemy.
      4. Otherwise the default attack routine.
    A move is always followed by a queued ActionAttack, so the creature
    never idles after it arrives. The engine half (ranged-ai DLL) keeps a
    blocked creature from walking into melee instead of shooting.
    SW_NUMBER_KITE_COOLDOWN (local number 20, unused by vanilla) keeps a
    move from being re-planned at once; it counts GN_DetermineCombatRound
    calls, not rounds. The engine rule "Ranged companions do not walk to
    their target" also applies to stance 16 (DLL hook E-H3).
*/

int SW_NUMBER_KITE_COOLDOWN = 20;
float KITE_THREAT_RANGE = 3.5;
float KITE_LEASH = 10.0;
float KITE_SHOT_RANGE = 15.0;
int KITE_TRIES = 12;

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
        // stay in shot range: stance 16 does not walk back to its target (vanilla Ranged engine rule, DLL E-H3)
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

    // 1. Kite away from a close melee threat.
    if (nRanged && nCool == 0)
    {
        object oThreat = GetNearestCreature(CREATURE_TYPE_REPUTATION, REPUTATION_TYPE_ENEMY, OBJECT_SELF, 1,
                                            CREATURE_TYPE_PERCEPTION, PERCEPTION_SEEN);
        if (GetIsObjectValid(oThreat) && !GetIsDead(oThreat) && GetDistanceBetween(OBJECT_SELF, oThreat) <= KITE_THREAT_RANGE)
        {
            object oThreatWeapon = GetItemInSlot(INVENTORY_SLOT_RIGHTWEAPON, oThreat);
            int nThreatMelee = !GetIsObjectValid(oThreatWeapon) || GN_GetWeaponTypeFromBaseItem(GetBaseItemType(oThreatWeapon)) != 2;
            int nHurt = GetCurrentHitPoints(OBJECT_SELF) * 10 < GetMaxHitPoints(OBJECT_SELF) * 6;
            if (nThreatMelee || nHurt)
            {
                object oShootAt = GetIsObjectValid(oTarget) ? oTarget : oThreat;
                if (GN_KiteFindSpot(1, oThreat, oShootAt, oLeader))
                {
                    GN_KiteMoveThenAttack(oShootAt, 2);
                    return 1;
                }
            }
        }
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
            GN_KiteMoveThenAttack(oTarget, 1);
            return 1;
        }
    }

    // 4. Normal attack behaviour (re-issues ActionAttack every round).
    GN_RunDefaultAIRoutine(oIntruder);
    return 0;
}
