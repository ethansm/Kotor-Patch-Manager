#!/usr/bin/env python3
"""Persist the engine-AI atlas wave 1 (AI1-AI4 + Opus gate, 2026-10-02) into
ghidra_knowledge/kotor2_ghidra_knowledge.db.  Python 3, stdlib only.

Sources of truth: engine_ai_atlas/GATE_wave1.md (sections 3, 4, 6; it OVERRIDES the batches) and the (e) struct field
tables of engine_ai_atlas/findings/AI1.md .. AI4.md (parsed at run time).

Idempotent:
  * name_map: gate corrections update an existing row once (skipped when the row already has the new name and SRC);
    new rows / action-table rows are inserted only where steam_addr is absent (a differing existing name = CONFLICT line).
  * lessons: keyed by (scope, topic), note updated in place.
  * struct_fields: CREATE TABLE IF NOT EXISTS; insert if missing, update only when the new confidence rank >= the old one
    and something changed.
  * candidates: keyed by id, updated in place.
Everything runs in one transaction; --dry-run rolls it back.
"""
import argparse, os, re, sqlite3, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ATLAS = os.path.normpath(os.path.join(HERE, ".."))
DEFAULT_DB = os.path.normpath(os.path.join(ATLAS, "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db"))
FINDINGS = os.path.join(ATLAS, "findings")

DATE = "2026-10-02"
SRC = "engine-ai-atlas wave1"
EV = "engine_ai_atlas/GATE_wave1.md"
EV_SUFFIX = " Evidence: engine_ai_atlas/GATE_wave1.md"
NOTE_MAX = 600

# --------------------------------------------------------------------------------------------------------------------
# name_map: gate section 4, first table (corrections).  (addr, new name or None=keep, conf, gate evidence)
CORRECTIONS = [
    (0x0051d790, "CServerAIMaster::UpdateState", "high",
     "`0051d79f MOV [EBP-0x18],0x2710`, levels 4..0, sole caller `0053396e` with `[EAX+0x10044]`"),
    (0x005644b0, "CSWSCreature::AddMoveToPointActionToFront", "high",
     "`005645e4 CALL 0x00540ca0` (AddActionToFront); 17 args, packs node flags"),
    (0x00564770, "CSWSCreature::AddMoveToPointAction", "high",
     "`005648a4 CALL 0x0053f7f0` (AddAction)"),
    (0x0051e170, "CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)", "high",
     "`005c3ea5 CALL 0x0051c280 / MOV ECX,EAX / 005c3eac CALL 0x0051e170`; node stores `[2..5] = p3..p6`"),
    (0x0051d1c0, "CServerAIList::GetNextObject", "medium",
     "`this` = list (`param_1[3]` cursor, `param_1[1]` count)"),
    (0x0051d270, "CServerAIList::GetObjectAtPosition", "medium",
     "`FUN_0051c0a0(*(int*)(*param_1 + param_2*4))`"),
    (0x00585420, "CSWSCreature::GetIsBlindOrInDarkness", "medium",
     "body: `+0xFF4 & 0x10 || (+0xFF4 & 8 && !(+0xFF4 & 2))`"),
    (0x005802c0, "CSWSCreature::GetDetectModeState", "medium",
     "`(+0xd8 == 10000 && +0x510 != 1) ? 2 : +0x510`"),
    (0x0066b640, "Global::ExecuteCommandSetCreatureAILevel", "high",
     "`0066b6e9 MOV [EAX+0x7c],CL` (saved level), switch -> `CALL 0x0051e120`"),
    (0x0055ab30, "CSWSModule::PlotPath (pathfind request)", "medium",
     "`005c1f11 CALL 0x0051bd90` (GetModule) / `005c1f16 MOV ECX,EAX` / `005c1f18 CALL 0x0055ab30`; only caller"),
    (0x0057e280, None, "low",
     "called from perception for `+0xFF5` in {0x12,0x13} near a non-hostile target (AI3); name does not fit; not read"),
    (0x00550e30, "CSWSArea::PlotGridPath (returns closest path as SUCCESS)", "high",
     "strings \"Exceeded limits:*\"; l.238-250 closest-result rule"),
]

# gate section 4, second table (new rows).  (addr, name, conf, evidence)
NEW_ROWS = [
    (0x0051e0c0, "CServerAIMaster::RemoveObject", "high", "`0051e10d MOV [ECX+0x78],0xffffffff` after the list erase"),
    (0x0051ed10, "CServerAIMaster::EventPending(day,ms)", "high", "head time `0x51AFE0`-compared to args, `<1` -> 1"),
    (0x0051ed80, "CServerAIMaster::GetPendingEvent", "medium", "6 outs, called in the UpdateState drain `0051da2b`"),
    (0x0063c1e0, "UpdateObjectInPlayerUpdateSet (writes CSWSObject+0x220)", "medium",
     "`0063c257 MOV [EDX+0x220],EAX` = `0x63DD30(...) || +0x260`"),
    (0x0063dd30, "IsObjectInPlayerUpdateRange(obj, player)", "medium",
     "party/self -> 1; range^2 vs max(spot,listen) + `0x585480`"),
    (0x005ffd60, "CSWSItem::Constructor", "medium", "vptr `005ffdc0 MOV [EAX+0x10],0x9972cc`; `005ffe52 CALL 0x0051e040` level 0"),
    (0x0059e860, "CSWSEffectListHandler::OnRemoveSetStateInternal", "high",
     "remove table idx 9 `0059414c`; slot-6 RunScript `0059e904`"),
    (0x0059de30, "CSWSEffectListHandler::OnApplySetStateInternal", "medium", "apply table idx 9 `0059413f MOV [EDX+0x24],0x59de30`"),
    (0x0059cba0, "CSWSEffectListHandler::OnApplySetState", "medium",
     "apply table idx 8 `00593d0b`; creates type-9 effect, writes creature `+0xFF5`"),
    (0x0059db50, "CSWSEffectListHandler::OnRemoveSetState", "medium",
     "remove table (`[+8]`) idx 8 `00593d15 MOV EAX,[EDX+0x8]` / `00593d18 MOV [EAX+0x20],0x59db50`"),
    (0x005959e0, "CSWSEffectListHandler::OnApplyDamage", "medium", "mapper + `005966d0 CALL 0x6fd8d0` slot 4 (OnDamaged)"),
    (0x006e0b10, "CSWSCreature::ResolveAttackRoll", "medium", "`006e0bd9 CALL 0x006a2000` (1,0x14) d20; result `attack+0x5C`"),
    (0x006a2000, "CSWRules::RollDice(n,sides)", "medium", "ECX=`[0xA1B4D0]`, `PUSH 0x14/PUSH 1` at `006e0bd1`"),
    (0x00550980, "CSWSArea::WalkLineTest (1 clear, 0 wall/no transition, -1 face-walk or collision-mesh fail, "
                 "-2 door/placeable box, -3 creature)", "medium", "`0x550980.c` `local_3c = -1/-2/-3/0`"),
    (0x006f9200, "CPathfindInformation::Constructor", "medium", "`005621bf PUSH 0x278` ... `005621dc CALL 0x006f9200`"),
    (0x006f99f0, "CPathfindInformation::Shutdown (leaks +0x18C/+0x1B4/+0x1D8/+0x200)", "medium",
     "frees `0x90..0x238` at `006f9bb6..006f9c5c` (U3 PFLK)"),
    (0x006f9c90, "CPathfindInformation::ResetRequest", "high", "zeroes goal `+0x5C..+0x80`, `+0x58`; called at walker end `005c3efe`"),
    (0x006f9d40, "CPathfindInformation::ResetAtEndOfPath", "medium", "`+0x84=0, +0x88=1`, frees `+0xB8`"),
    (0x006fc5d0, "CSWSCreature::AvoidCreature (runs k_def_pathfail01)", "medium", "string `0x99B5E0` \"k_def_pathfail01\""),
    (0x005d1400, "CSWSCreature::AddToVisibleList(id,seen,heard,invis,hidden)", "medium", "`new(8)` entry, bit packing (AI3 Q1)"),
    (0x0051bd90, "GetModule (fastcall wrapper of 0x52F4C0)", "medium", "body `FUN_0052f4c0(param_1)`"),
    (0x00590a50, "CSWSCombatRound::AddAttackAction", "medium", "`new(0x88)` + ctor `0x58C7D0`; callers `005685e5`, `006da9c6`"),
    (0x005905f0, "CSWSCombatRound::CalculateOnHandAttacks", "medium", "`+0xAE0 = 0x6A7170() + min(+0xAF4,2) + bonuses`"),
    (0x005df920, "CSWSCreature::AIActionType47_ReequipResumeAttack (type 0x47)", "low",
     "table entry `0x5D682A` -> `CALL 0x005df920`; body per AI4"),
]

# AI4 action table (RunActions JMP [EDX*4+0x5D7038], index = type-1).  (addr, "types label", AI4 name, gate-verified)
# AI4 names are mapper/DB names (unverified) except for the 7 gate-verified entries.
ACTION_TABLE = [
    (0x5BFC00, "0x01", "AIActionMoveToPoint (stop-loop type)", True),
    (0x5BBF40, "0x02", "AIActionCheckMoveToObject", False),
    (0x5BB5A0, "0x03", "AIActionCheckMoveToPointRadius", False),
    (0x5BBB70, "0x04", "AIActionChangeFacingPoint", False),
    (0x5BE950, "0x05", "AIActionJumpToPoint (stop-loop type)", False),
    (0x5D9DA0, "0x06", "AIActionPlayAnimation", False),
    (0x5CF8A0, "0x07", "AIActionPickUpItem", False),
    (0x5BE0A0, "0x08", "AIActionEquipItem", False),
    (0x5CFF50, "0x09", "AIActionDropItem", False),
    (0x5BC700, "0x0A", "AIActionCheckMoveToPoint", False),
    (0x5D0A80, "0x0B", "AIActionUnequipItem", False),
    (0x6D7390, "0x0C", "AIActionPhysicalAttacks", True),
    (0x5D9890, "0x0E", "AIActionSpeak", False),
    (0x5B96F0, "0x0F", "AIActionCastSpell (creature version; then 0x590180 if result 2 or 3)", False),
    (0x5F6180, "0x0F", "AIActionCastSpell (placeable version, object type byte 9)", False),
    (0x5D0C00, "0x10", "unnamed (174 B, calls 0x590180)", False),
    (0x5BC1D0, "0x11", "AIActionCheckMoveToObjectRadius", True),
    (0x5BCA60, "0x12", "AIActionCheckMoveToPoint_2", False),
    (0x5BB2C0, "0x13", "AIActionChangeFacingObject", False),
    (0x5DB0A0, "0x14", "AIActionOpenDoor", False),
    (0x5DAD10, "0x15", "AIActionCloseDoor", False),
    (0x5CF7B0, "0x16", "AIActionOrientCamera", False),
    (0x5D9BF0, "0x17", "AIActionPlaySound", False),
    (0x5D75B0, "0x18", "AIActionDialogObject", False),
    (0x5B43C0, "0x19", "DisarmMine", False),
    (0x5B3380, "0x1A", "RecoverMine", False),
    (0x5B2C10, "0x1B", "FlagMine", False),
    (0x5B5360, "0x1C", "ExamineMine", False),
    (0x5B5AD0, "0x1D", "SetMine", False),
    (0x5D9AA0, "0x1E", "AIActionWait", True),
    (0x5D9570, "0x1F", "unnamed (274 B, no callees: dead/zero-HP check, vtbl+0xC0, returns 3 or 2)", False),
    (0x5D9690, "0x20", "unnamed (320 B, calls 0x563D20/0x563DA0)", False),
    (0x5D97E0, "0x21", "SpeakStrRef", False),
    (0x5DA4D0, "0x22", "GiveItem", False),
    (0x5DAB80, "0x23", "TakeItem", False),
    (0x5BD0B0, "0x24", "AIActionEncounterCreatureDestroySelf", False),
    (0x5D99D0, "0x25", "DoCommand", False),
    (0x5DB8F0, "0x26", "UnlockObject", False),
    (0x5DDA60, "0x27", "LockObject", False),
    (0x5DFBE0, "0x28", "UseObject", False),
    (0x5D1990, "0x29/0x2B/0x36", "constant stub `return DAT_009944bc` (3)", False),
    (0x5D17D0, "0x2A", "constant stub `return DAT_009944b8` (2)", False),
    (0x5BB920, "0x2C", "AIActionCheckInterAreaPathfinding", False),
    (0x5BF000, "0x2D", "AIActionRandomWalk", False),
    (0x5B8000, "0x2E", "AIActionItemCastSpell", False),
    (0x5E0E70, "0x2F", "AIActionSetCommandable", False),
    (0x5BE1F0, "0x30", "AIActionJumpToObject (stop-loop type)", False),
    (0x5BB470, "0x31", "AIActionCheckMoveAwayFromLocation", False),
    (0x5D25E0, "0x32", "AIActionCounterSpell", False),
    (0x5CCC50, "0x33", "AIActionDrive (stop-loop type)", False),
    (0x5D17F0, "0x34", "2000 ms timed animation action (anim 0x274F), returns 1 until 2000 ms then 2 or 3 (unnamed)", False),
    (0x5D18C0, "0x35", "2000 ms timed animation action (anim 0x2711), returns 1 until 2000 ms then 2 or 3 (unnamed)", False),
    (0x5CEED0, "0x37", "AIActionForceFollowObject", False),
    (0x5CF660, "0x3A", "AIActionCheckForceFollowObject", False),
    (0x5D19B0, "0x38", "3102 B, strings Subs_Save/Save_DC, creates effects; mapper says AIActionHeal (low)", False),
    (0x5BEFC0, "0x3C", "unnamed (64 B, calls GetArea 0x5453C0)", False),
    (0x5BF7F0, "0x3D", "AIActionFollowLeader", True),
    (0x5D7480, "0x3E", "AIActionBarkString", False),
    (0x6D9500, "0x3F", "AIActionCombat", True),
    (0x5BCDC0, "0x40", "AIActionCheckMoveToFollowRadius", False),
    (0x5D29B0, "0x41", "unnamed (55 B, calls 0x5D51A0)", False),
    (0x5DEC20, "0x43", "AIActionDoorSaber", False),
    (0x5D29F0, "0x44", "AIActionStanceTotalDefense", False),
    (0x5D2EA0, "0x45", "unnamed (216 B, calls 0x57A990, count 0x5210F0)", False),
    (0x5BF940, "0x46", "AIActionFollowOwner (puppet flag +0x11FC)", False),
    (0x5DF920, "0x47", "unnamed: ClearAllActions(1), re-equip, re-queue attack at round target", True),
]
ACTION_SRC = "engine-ai-atlas AI4"

# gate section 6 lessons: (scope, topic, note)
LESSONS = [
    ("combat-ai", "FM2b: unreachable target = partial leg then FAIL loop",
     "`0x55AC80` returns 2 for an unreachable goal via (a) the straight bisection fallback (`0x55B3E9..0x55B692`, needs PI+0 == 0 "
     "and cached start->goal code != -1, 6 probes, 2-point path) or (b) the grid closest-result rule (`0x550E30` l.238-250). The walker "
     "end calls ResetRequest (`0x5C3EFE`), so every 0x11 re-approach re-plans. From the obstacle the probes fail and give 3: the 2-4 s "
     "wait at `0x5C2484`. FM2 and FM2b are phases of one loop. Key fail-fast on 0x11 re-approach (`0x5BC5F4`). Anchor: GATE_wave1 1.1."),
    ("combat-ai", "WalkLineTest codes",
     "`0x550980`: 1 clear; 0 = wall (perimeter edge without room transition), 100 steps, start not in a room; -1 = face-walk failure "
     "(`0x651090`) or swept collision-mesh failure (`0x5EA8F0`, swcollisionmesh.cpp); -2 = door/placeable box (`0x52D130`); -3 = creature "
     "on the line. A cached -1 disables the partial fallback (`0055b4fb CMP [EBP-0x20],-1`)."),
    ("combat-ai", "Action 0x11 has no LOS",
     "`0x5BC1D0`: `005bc4b8 XOR EDX,EDX` / `005bc4ba JZ 0x5bc5de` is always taken, so the LOS + range-halving block `0x5BC4C0..0x5BC5D7` "
     "is dead. CheckMoveToObjectRadius = same area && GetIsInUseRange(range - target radius). LOS only matters in "
     "AIActionPhysicalAttacks (`+0x558`). AI1 misread it; R1 was right."),
    ("method", "`XOR r,r` + `Jcc` is an unconditional jump",
     "before calling a block live, check the flag source of the branch that guards it. `XOR reg,reg; JZ` (ZF=1) is the compiler's form "
     "of `if (0)`. Inbound branches into the block from elsewhere decide whether it is dead (grep every `.asm` for targets). Example "
     "`0x5BC4B8` (lesson 3)."),
    ("combat-ai", "OnBlocked has 6 post sites; door vs trigger in MoveToPoint",
     "payload 0x1F at `0x5C1240` (5BFC00 door case 0xA), `0x5C3D7D`, `0x5C5A68`, `0x5CCBB4`, `0x5C8BB4`, `0x5C9F8F`, all delta-0, none "
     "throttled. 5BFC00 switch `JMP [ECX*4+0x5C25BC]` (index type-5): type 7 trigger -> `0x5C0410` (posts 0x1E at `0x5C0732`), type 0xA "
     "door -> `0x5C1143`. R1 had 3 sites and called `0x5C0732` the door."),
    ("perception", "spot distance sign bug",
     "DoSpotDetection `0x578850` adds `+ftol(d/3-2)` past 6 m (`00579007 NEG EAX`, `0057900f SUB ECX,[EBP-0x10]`; consts `0x98B968`=3.0, "
     "`0x987490`=2.0). The margin `[EBP-0x48]` is spotted iff > 0, and all other adverse terms SUB. Listen subtracts (`005787b9`). "
     "Stealthed targets are easier to spot far away. 1-byte fix `0x57900F` 2B->03 (KPM simple). Balance change = user decision."),
    ("perception", "UpdatePerception mode 1 never calls `0x522AF0`",
     "`0x5D0F30` mode 1 walks the party table (non-party perceivers only, `param_1[0x46b]==0`). Modes 0/2 refresh the existing list, then "
     "scan `area+0x194` from `0x522AF0`, whose 0 return skips the discovery and the `+0x11C8` reset. The R1 abort cannot block enemy "
     "aggro on the party."),
    ("scheduler", "`+0x220` = player update-set flag",
     "CSWSObject `+0x220` is written only by `0x63C1E0` (`0063c257`) = `0x63DD30(obj, player) || ForceAlwaysUpdate(+0x260)`. 1 for "
     "self/party, for creatures in perception range and not invisible, or seen/heard by the player. It gates the UpdateState pre-pass "
     "QuickWalk and the client sync. It is NOT a party flag (U3 was wrong); visible enemies are included."),
    ("scheduler", "action table `0x5D7038`",
     "RunActions `JMP [EDX*4+0x5D7038]` at `0x5D5C28`, index = type-1, 0x47 dword entries. Verified 1->0x5BFC00, 0xC->0x6D7390, "
     "0x3F->0x6D9500, 0x3D->0x5BF7F0 (gated by leader valid + `+0x11AC`), 0x1E->0x5D9AA0, 0x11->0x5BC1D0, 0x47->0x5DF920. Types "
     "0xD/0x39/0x3B/0x42 have no handler and are deleted as 3. Read the table from the vanilla exe (`.rdata` file offset = "
     "VA-0x986000+0x584C00)."),
    ("scheduler", "items are AI objects at level 0, but their AIUpdate is cheap",
     "CSWSItem ctor `0x5FFD60` -> `AddObject(item,0)` (`0x5FFE52`). Item vtable `0x9972CC` (at item+0x10) slot 0x70 = `0x600F20`, which "
     "only scans the item's property list for type 10 (timed subtypes 0xE..0x12). The per-item cost is the scheduler pre-pass lookup per "
     "loop iteration (AIQ), not the item tick."),
    ("combat-ai", "slot-6 also fires when a SetState effect ends",
     "`OnApplySetState 0x59CBA0` (effect type 8: DroidStun 3, Choke 7, Horrified 8, Whirlwind 10 ...) creates an internal type-9 effect "
     "and sets creature `+0xFF5` = state. Its removal `0x59E860` (remove table idx 9, `0059414c`) runs slot 6 (OnEndRound) for states 1, "
     "3-10, 0xE-0x13: no leader/dead/combat gate. It also fires when a stronger state replaces a weaker one."),
    ("combat-ai", "range metrics",
     "the attack decision in `0x6D7390` uses 2D centre dist2 (`006d7e4a FLDZ` zeroes z) vs (MaxAttackRange+0.1)^2. Melee reach = 1.6 + "
     "HITRADIUS(att) + HITRADIUS(tgt) + 0.5 (appearance HITRADIUS, not PERSPACE). GetNearestVisibleEnemyInRange `0x57C990` accepts by 3D "
     "length minus both `PI+8` radii (`0057cd80`, `0057cdc0`). A strict retarget must recompute the 2D test."),
    ("combat-ai", "grid search returns a closest path as SUCCESS",
     "`0x550E30` sets `PI+0x24=1` every call. When an iteration crosses 4x the slice, or 1x with no best-distance gain, and the best path "
     "has >= 2 points (`PI+0x188`) and the 64-expansion abort (`area+0x1D0`) is clear, it returns 2 with the best path. Do not read the "
     "\"Exceeded limits\" strings as \"no path => FAIL\"."),
    ("scheduler", "dismissed party member -> AI level 2",
     "`SetPartyMemberFlag(0, arg2!=0)` sets level 2 (`0058a448 PUSH 0x2` / `0058a458 CALL 0x51e120`), not 0/1. Its own next AIUpdate "
     "demotes it to 0/1 (`0x56A0FE/0x56A111`), a self-transition (FM3 skip-one-object effect). `+0x11AC` has one runtime writer "
     "(`0x58A35E`)."),
]

# candidates: (id, title, subsystem, hypothesis, evidence, expected_win, risk, status, decision_date)
CANDIDATES = [
    ("AI-SPOTSIGN", "Spot check adds the distance term with the wrong sign", "ai",
     "DoSpotDetection 0x578850 adds +ftol(d/3-2) to the spotter's margin past 6 m (00579007 NEG EAX, 0057900f SUB ECX,[EBP-0x10]; "
     "consts 0x98B968=3.0, 0x987490=2.0). The margin [EBP-0x48] means spotted iff > 0 and every other adverse term (target Stealth rank "
     "and dice, moving, blind spot, in combat) SUBs; the listen check subtracts its distance term (005787b9). So a stealthed target is "
     "EASIER to spot far away (swing 2n, 8 points at 18 m). Only reached for a target in stealth mode with Stealth rank > 0.",
     "Static, gate-verified (Opus): " + EV + " section 1.2 and lesson 'spot distance sign bug'; AI3 X1. Fix = KPM simple 1-byte hook "
     "0x57900F (2B 4D F0 SUB ECX,[EBP-0x10] -> 03 4D F0 ADD ECX,[EBP-0x10]); flags rewritten at 0x579015 before any read. Not yet "
     "tested in game.",
     "stealth works as designed at range",
     "balance change; 1-byte KPM simple hook 0x57900F 2B->03",
     "confirmed-static", None),
    ("AI-SETSTATE-SLOT6", "SetState effect removal runs the EndRound (slot 6) script", "ai",
     "OnApplySetState 0x59CBA0 (effect type 8: DroidStun 3, Choke 7, Horrified 8, Whirlwind 10 ...) creates an internal type-9 effect and "
     "sets creature +0xFF5 = state. Its removal 0x59E860 (remove table idx 9, 0059414c) runs slot 6 (OnEndRound script) for states 1, 3-10, "
     "0xE-0x13 with no leader/dead/combat gate, and also when a stronger state replaces a weaker one (0x59CBA0 removes the existing "
     "type-9 effects first). So a companion's EndRound AI script (k_ai_master) runs out of cycle when a stun-like state ends.",
     "Static, gate-verified: " + EV + " section 1 (AI2 slot-6 site 3 row) and lesson 'slot-6 also fires when a SetState effect ends'.",
     "documents an extra slot-6 trigger (explains out-of-cycle EndRound runs; relevant to the A1/E2 slot-6 cadence)",
     "low; behaviour = extra EndRound script run when a stun-like state ends",
     "confirmed-static", None),
]

# --------------------------------------------------------------------------------------------------------------------
STRUCT_DDL = """CREATE TABLE IF NOT EXISTS struct_fields(
  class TEXT NOT NULL,          -- 'CSWSCreature', 'CSWSCombatRound', 'CPathfindInformation', 'CAction' (0x74 node), ...
  offset INTEGER NOT NULL,      -- byte offset from the class base (store int; print as hex)
  size INTEGER,                 -- bytes (1/2/4/8/N); NULL if unknown
  type TEXT,                    -- 'int', 'float', 'objid', 'ptr CSWSCombatRound', 'CExoArrayList<objid>', ...
  name TEXT,                    -- proposed name
  confidence TEXT,              -- high (quoted asm reader/writer) / medium / low
  evidence TEXT,                -- one quoted line with address, or the findings file reference
  readers TEXT,                 -- space-separated hex instruction addresses (or fn addresses)
  writers TEXT,
  source TEXT,                  -- 'engine-ai-atlas AI<n>' / 'review_ai_engine' / ...
  date TEXT,
  PRIMARY KEY(class, offset)
)"""

RANK = {"low": 1, "medium": 2, "med": 2, "high": 3}
CLASS_MAP = {"PI": "CPathfindInformation", "ActionNode": "CAction", "CSWSObject/Creature": "CSWSObject",
             "perception entry": "PerceptionEntry", "time object": "TimeObject"}


def norm_conf(c):
    c = (c or "").strip().lower()
    return "medium" if c == "med" else c


def hexaddr(a):
    return "0x%08x" % a


def clean(s):
    return (s or "").replace("`", "").strip()


# ---- struct table parsing ------------------------------------------------------------------------------------------
HEXTOK = re.compile(r"^\+?(?:0x)?([0-9A-Fa-f]+)$")
ADDR_RE = re.compile(r"0x[0-9A-Fa-f]{4,}(?![0-9A-Fa-fx])|\b[0-9a-f]{8}\b")


def split_row(line):
    line = line.replace("\\|", "\x00")
    cells = [c.strip().replace("\x00", "|") for c in line.strip().strip("|").split("|")]
    return cells


def parse_offsets(cell):
    """-> list of (offset:int, note:str) ; note is extra text such as '(+0x8C also)' for list groups."""
    cell = clean(cell)
    cell = re.sub(r"\s*\([^)]*\)", "", cell)  # drop '(15 x short)', '(i<13)'
    out = []
    for grp in re.split(r"\s+/\s+", cell):
        grp = grp.strip()
        if not grp:
            continue
        if ".." in grp or "*" in grp:
            m = re.match(r"^\+?(?:0x)?([0-9A-Fa-f]+)", grp)
            if not m:
                raise ValueError("offset cell %r" % cell)
            note = ""
            if "*" in grp:
                note = "stride " + grp
            elif ".." in grp:
                note = "range " + grp
            out.append((int(m.group(1), 16), note))
            continue
        if "," in grp:
            parts = [p.strip() for p in grp.split(",")]
            m = HEXTOK.match(parts[0])
            out.append((int(m.group(1), 16), "also " + ",".join(parts[1:])))
            continue
        subs = [p.strip() for p in grp.split("/")]
        if len(subs) > 1 and all(HEXTOK.match(p) for p in subs):
            out.extend((int(HEXTOK.match(p).group(1), 16), "") for p in subs)
            continue
        m = re.match(r"^\+?(?:0x)?([0-9A-Fa-f]+)", grp)
        if not m:
            raise ValueError("offset cell %r" % cell)
        out.append((int(m.group(1), 16), ""))
    return out


def parse_size(s):
    s = clean(s)
    if not s or s == "-":
        return None
    m = re.match(r"^(\d+)\s*x\s*(\d+)$", s)
    if m:
        return int(m.group(1)) * int(m.group(2))
    m = re.match(r"^(\d+)\*(0x[0-9A-Fa-f]+|\d+)$", s)
    if m:
        return int(m.group(1)) * int(m.group(2), 0)
    m = re.match(r"^(0x[0-9A-Fa-f]+|\d+)(\s+each)?$", s)
    if m:
        return int(m.group(1), 0)
    return None


def split_parts(s, n, seps):
    for sep in seps:
        p = [x.strip() for x in re.split(sep, s)]
        if len(p) == n:
            return p
    return None


def addr_cell(cell):
    """readers/writers cell -> clean normalised addresses, or the raw text if it carries other information."""
    raw = clean(cell)
    if raw in ("", "-"):
        return None
    addrs = []
    for t in ADDR_RE.findall(raw):
        t = t.lower()
        a = int(t[2:], 16) if t.startswith("0x") else int(t, 16)
        h = hexaddr(a)
        if h not in addrs:
            addrs.append(h)
    return " ".join(addrs) if addrs else raw  # no address at all (e.g. 'Save only', 'everywhere'): keep the text


def parse_findings_e(batch):
    path = os.path.join(FINDINGS, "AI%d.md" % batch)
    txt = open(path, encoding="utf-8").read().split("\n")
    start = next(i for i, l in enumerate(txt) if l.startswith("## (e)"))
    end = next((i for i in range(start + 1, len(txt)) if txt[i].startswith("## ")), len(txt))
    rows, warnings = [], []
    ncell = 0
    for l in txt[start:end]:
        if not l.startswith("|"):
            continue
        cells = split_row(l)
        if cells[0].lower() == "class" or set(cells[0]) <= set("-: "):
            continue
        if len(cells) != 9:
            warnings.append("AI%d: row with %d cells skipped: %s" % (batch, len(cells), l[:100]))
            continue
        ncell += 1
        rows.append(cells)
    return rows, warnings


def expand(batch, cells):
    cls, off, size, typ, name, rd, wr, conf, ev = cells
    cls = re.sub(r"\s*\(.*\)$", "", cls.strip())
    cls = CLASS_MAP.get(cls, cls)
    offs = parse_offsets(off)
    name_c = clean(name)
    n = len(offs)
    size_p = [parse_size(x) for x in re.split(r"\s*/\s*", clean(size))] if clean(size) else []
    split = n > 1 and (split_parts(name_c, n, [r"\s+/\s+", r",\s*"]) is not None or len(size_p) == n)
    out = []
    if not split:
        o, note = offs[0]
        nm = name_c + (" [%s]" % note if note else "")
        if n > 1:
            nm += " [offsets " + "/".join("0x%X" % x for x, _ in offs) + "]"
        out.append(dict(cls=cls, off=o, size=(size_p[0] if len(size_p) == 1 else parse_size(size)), type=clean(typ) or None,
                        name=nm, conf=norm_conf(conf), ev=clean(ev), rd=addr_cell(rd), wr=addr_cell(wr), batch=batch))
        return out
    names = split_parts(name_c, n, [r"\s+/\s+", r",\s*"])
    if names is None and n == 2:
        m = re.match(r"^(.+\s)(\w+)/(\w+)$", name_c)
        if m:
            names = [m.group(1) + m.group(2), m.group(1) + m.group(3)]
    types = split_parts(clean(typ), n, [r"\s*/\s*", r",\s*"]) if clean(typ) else None
    rds = split_parts(clean(rd), n, [r"\s+/\s+"])
    wrs = split_parts(clean(wr), n, [r"\s+/\s+"])
    for i, (o, note) in enumerate(offs):
        nm = names[i] if names else "%s [+0x%X]" % (name_c, o)
        if note:
            nm += " [%s]" % note
        sz = size_p[i] if len(size_p) == n else (size_p[0] if len(size_p) == 1 else None)
        e = clean(ev)
        if rds:
            r = addr_cell(rds[i])
        else:
            r = addr_cell(rd) if i == 0 else None
        if wrs:
            w = addr_cell(wrs[i])
        else:
            w = addr_cell(wr) if i == 0 else None
        if not (rds and wrs) and i == 0 and n > 1:
            e += " [grouped row: shared readers/writers cell kept on the first offset]"
        out.append(dict(cls=cls, off=o, size=sz, type=(types[i] if types else (clean(typ) or None)), name=nm,
                        conf=norm_conf(conf), ev=e, rd=r, wr=w, batch=batch))
    return out


def merge_lists(a, b):
    """union of two readers/writers values; address lists win over free text."""
    if not a or not b:
        return a or b
    ia, ib = all(ADDR_RE.fullmatch(t) for t in a.split()), all(ADDR_RE.fullmatch(t) for t in b.split())
    if ia and ib:
        return " ".join(a.split() + [t for t in b.split() if t not in a.split()])
    if ia != ib:
        return a if ia else b
    return a if b in a else a + " | " + b


def build_struct_rows(log):
    rows = {}
    notes = []
    for batch in (1, 2, 3, 4):
        cells_list, warns = parse_findings_e(batch)
        for w in warns:
            notes.append("WARNING " + w)
        for cells in cells_list:
            try:
                parts = expand(batch, cells)
            except Exception as e:  # report, never silently drop
                notes.append("WARNING AI%d: could not transcribe row %s (%s)" % (batch, cells[:2], e))
                continue
            for r in parts:
                # gate: AI1 'CSWSCreature +0x220' is CSWSObject +0x220
                if batch == 1 and r["cls"] == "CSWSCreature" and r["off"] == 0x220:
                    r["cls"] = "CSWSObject"
                key = (r["cls"], r["off"])
                if key in rows:
                    old = rows[key]
                    new_better = RANK[r["conf"]] >= RANK[old["conf"]]  # tie -> the later batch
                    win, lose = (r, old) if new_better else (old, r)
                    merged = dict(win)
                    merged["rd"] = merge_lists(old["rd"], r["rd"])
                    merged["wr"] = merge_lists(old["wr"], r["wr"])
                    if clean(lose["name"]) != clean(win["name"]):
                        merged["ev"] = (win["ev"] + " | AI%d name: %s" % (lose["batch"], lose["name"])).strip(" |")
                    if merged["size"] is None:
                        merged["size"] = lose["size"]
                    if not merged["type"]:
                        merged["type"] = lose["type"]
                    merged["merged_from"] = sorted(set(old.get("merged_from", [old["batch"]]) + [r["batch"]]))
                    notes.append("MERGE %s +0x%X: AI%d(%s) + AI%d(%s) -> keep AI%d '%s' (%s)" % (
                        key[0], key[1], old["batch"], old["conf"], r["batch"], r["conf"], win["batch"], win["name"], win["conf"]))
                    rows[key] = merged
                else:
                    rows[key] = r
    apply_struct_overrides(rows, notes)
    return rows, notes


def set_row(rows, notes, cls, off, batch, **kw):
    key = (cls, off)
    r = rows.get(key)
    if r is None:
        r = dict(cls=cls, off=off, size=None, type=None, name="", conf="low", ev="", rd=None, wr=None, batch=batch)
        rows[key] = r
        notes.append("ADD (gate/override) %s +0x%X" % (cls, off))
    for k, v in kw.items():
        if k == "ev_add":
            r["ev"] = (r["ev"] + " | " + v).strip(" |") if r["ev"] else v
        else:
            r[k] = v
    return r


def apply_struct_overrides(rows, notes):
    G = "GATE_wave1 sec 3: "
    # AI3 gives both CSWSArea +0x194/+0x198 as one grouped row; AI4 has the per-field names
    set_row(rows, notes, "CSWSArea", 0x194, 3, name="objectIds (all-objects id array sorted by X)", type="ptr")
    set_row(rows, notes, "CSWSArea", 0x198, 3, name="objectCount (all-objects id array count)", type="int")
    # PI +0x024 / +0x188 / +0x18C  (AI1)
    set_row(rows, notes, "CPathfindInformation", 0x24, 1, name="closest-result enabled (set every grid call, l.171)",
            conf="medium", ev_add=G + "renamed from 'grid search started'; PI+0x24 = 1 written on every 0x550E30 call (l.171)")
    set_row(rows, notes, "CPathfindInformation", 0x188, 1, name="best (closest) path point count, returned as SUCCESS",
            conf="high", ev_add=G + "0x550E30 l.238-250 closest-result rule")
    set_row(rows, notes, "CPathfindInformation", 0x18C, 1, name="best (closest) path buffer, returned as SUCCESS; NOT freed by Shutdown",
            conf="high", ev_add=G + "0x550E30 l.238-250 closest-result rule")
    # bRangeStopInWalker is +0x240 (the AI1 row groups 0x23C/240)
    r = rows.get(("CPathfindInformation", 0x23C))
    if r is not None and "bRangeStopInWalker" in r["name"]:
        rd, wr, ev = r["rd"], r["wr"], r["ev"]
        r["size"], r["name"], r["conf"] = 4, "unknown (pairs with +0x240)", "low"
        r["rd"], r["wr"] = None, None
        r["ev"] = "AI1 grouped '0x23C/240' row; only +0x240 has a quoted writer"
        set_row(rows, notes, "CPathfindInformation", 0x240, 1, size=4, type="u32", name="bRangeStopInWalker (node bit10)",
                conf="high", ev=ev, rd=rd, wr=wr)
    # +0x220 : CSWSObject, player update-set flag  (AI1 low, AI4 med -> high)
    set_row(rows, notes, "CSWSObject", 0x220, 4, name="playerUpdateSetFlag (1 = in player update set; = 0x63DD30(obj,player) || ForceAlwaysUpdate +0x260)",
            conf="high", type="int",
            ev_add=G + "0063c257 MOV [EDX+0x220],EAX (0x63C1E0); NOT a party flag (U3/AI1 wrong), CSWSObject not CSWSCreature")
    # +0xFF5 (AI3 low -> medium, renamed)
    set_row(rows, notes, "CSWSCreature", 0xFF5, 3, name="current SetState id (highest active SetState; written by 0x59CBA0)",
            conf="medium", ev_add=G + "renamed from 'creature class byte'; 0x59CBA0 `if (+0xff5 < state) +0xff5 = state`")
    # combat round pointer + pathinfo pointer on the owner
    set_row(rows, notes, "CSWSCreature", 0x10DC, 2, size=4, type="ptr CSWSCombatRound",
            name="combatRound (separate heap object, new(0xB2C))", conf="high", wr="0x00562290",
            ev="`0056223f PUSH 0xb2c` / `00562290 MOV [EDX+0x10dc],EAX` (AI2, gate-confirmed)")
    set_row(rows, notes, "CSWSCreature", 0x380, 1, type="ptr CPathfindInformation", name="pathfindInfo (new(0x278))",
            ev_add="new(0x278) at 0x5621BF, init 0x6F9440; owner of the CPathfindInformation rows")
    # absence claims are never 'high' (lesson 168)
    r = rows.get(("CSWSCreature", 0x3A0))
    if r is not None and r["conf"] == "high":
        r["conf"] = "medium"
        r["ev"] += " | " + G + "'never stored after ctor' is an absence claim (field_census): medium"
    # lead check: the AIUpdate dt field is +0xD0 (0056956c MOV [ECX+0xd0],EDX; 0056994b SUB ECX,[EAX+0xd0]); AI1's +0xD4 is unsupported
    r = rows.get(("CSWSCreature", 0xD4))
    if r is not None and "since last" in r["name"]:
        r["conf"] = "low"
        r["ev"] += " | lead 2026-10-02: AIUpdate 0x569320 uses +0xD0 (0056956c, 0056994b); +0xD4 likely an offset slip"
    # AI2 'AD0..AD8' range -> three 4-byte fields
    r = rows.get(("CSWSCombatRound", 0xAD0))
    if r is not None and "range" in r["name"]:
        base = dict(r)
        for i, nm in enumerate(("ParryIndex", "NumAOOs", "NumCleaves")):
            off = 0xAD0 + 4 * i
            rr = dict(base)
            rr.update(off=off, size=4, name=nm, ev=base["ev"] + " [split from range 0xAD0..0xAD8]")
            rows[("CSWSCombatRound", off)] = rr
        notes.append("SPLIT CSWSCombatRound 0xAD0..0xAD8 -> ParryIndex/NumAOOs/NumCleaves")


# ---- DB operations -------------------------------------------------------------------------------------------------
class Stats(dict):
    def __missing__(self, k):
        return 0

    def inc(self, k, n=1):
        self[k] = self[k] + n


def do_name_map(cur, st):
    def get(a):
        return cur.execute("select steam_addr,name,source,confidence,evidence,date from name_map where steam_addr=?",
                           (hexaddr(a),)).fetchone()

    for a, new_name, conf, ev in CORRECTIONS:
        row = get(a)
        if row is None:
            print("MISSING correction target %s: inserting as new row" % hexaddr(a))
            cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                        (hexaddr(a), new_name or "(unnamed)", SRC, conf, "%s section 4; %s" % (EV, ev), DATE))
            st.inc("nm_added")
            continue
        _, old_name, old_src, old_conf, old_ev, _ = row
        if new_name is None:  # demotion: keep name
            if (old_ev or "").startswith("DEMOTED 2026-10-02 (GATE_wave1)"):
                st.inc("nm_skipped"); continue
            cur.execute("update name_map set confidence=?, evidence=? where steam_addr=?",
                        (conf, "DEMOTED 2026-10-02 (GATE_wave1): %s. %s" % (ev, old_ev or ""), hexaddr(a)))
            st.inc("nm_renamed"); print("DEMOTE %s %s (%s -> %s)" % (hexaddr(a), old_name, old_conf, conf))
            continue
        if old_name == new_name and old_src == SRC:
            st.inc("nm_skipped"); continue
        if old_name == new_name:
            new_ev = "UPGRADED 2026-10-02 (GATE_wave1): name confirmed, was (%s, %s). %s [%s s4]" % (old_src, old_conf, ev, EV)
        else:
            new_ev = "RENAMED 2026-10-02 (GATE_wave1): was '%s' (%s, %s). %s [%s s4]" % (old_name, old_src, old_conf, ev, EV)
        cur.execute("update name_map set name=?, confidence=?, source=?, evidence=?, date=? where steam_addr=?",
                    (new_name, conf, SRC, new_ev, DATE, hexaddr(a)))
        st.inc("nm_renamed")
        print("%s %s '%s' -> '%s' (%s)" % ("UPGRADE" if old_name == new_name else "RENAME", hexaddr(a), old_name, new_name, conf))

    for a, name, conf, ev in NEW_ROWS:
        row = get(a)
        if row is None:
            cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                        (hexaddr(a), name, SRC, conf, "%s section 4; %s" % (EV, ev), DATE))
            st.inc("nm_added")
        elif row[1] == name:
            st.inc("nm_skipped")
        else:
            print("CONFLICT %s: existing '%s' (%s, %s) vs gate '%s' (%s) -> not overwritten" % (
                hexaddr(a), row[1], row[2], row[3], name, conf))
            st.inc("nm_skipped"); st.inc("nm_conflict")

    for a, types, ai4name, verified in ACTION_TABLE:
        row = get(a)
        if row is not None:
            st.inc("nm_skipped"); st.inc("nm_action_existing")
            continue
        generic = "AIAction type %s handler (RunActions table 0x5D7038)" % types
        if verified:
            name = re.sub(r" \(.*\)$", "", ai4name) if ai4name.startswith("AIAction") else generic
            conf = "high"
            ev = ("%s sec 1 (AI4 action table row, gate-verified: table bytes read from the vanilla exe, call site quoted); "
                  "RunActions table 0x5D7038 type %s; AI4 name: %s" % (EV, types, ai4name))
        else:
            name = generic
            conf = "medium"
            ev = ("AI4.md action handler table (RunActions JMP [EDX*4+0x5D7038], index type-1): table bytes high, handler identity "
                  "medium (not spot-checked by the gate); AI4/mapper name: %s" % ai4name)
        cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                    (hexaddr(a), name, ACTION_SRC, conf, ev, DATE))
        st.inc("nm_added"); st.inc("nm_action_added")


def do_lessons(cur, st):
    for scope, topic, note in LESSONS:
        if len(note) > NOTE_MAX:
            print("WARNING lesson '%s': note %d chars > %d, truncated" % (topic, len(note), NOTE_MAX))
            note = note[:NOTE_MAX]
        elif len(note) + len(EV_SUFFIX) <= NOTE_MAX:
            note += EV_SUFFIX
        else:
            print("note: lesson '%s' %d chars, no room for the evidence suffix" % (topic, len(note)))
        row = cur.execute("select id,note from lessons where scope=? and topic=?", (scope, topic)).fetchone()
        if row is None:
            cur.execute("insert into lessons (scope,topic,note,date) values (?,?,?,?)", (scope, topic, note, DATE))
            st.inc("les_added")
        elif row[1] != note:
            cur.execute("update lessons set note=?, date=? where id=?", (note, DATE, row[0]))
            st.inc("les_updated")
        else:
            st.inc("les_skipped")


def do_struct(cur, rows, st, per_class):
    cur.execute(STRUCT_DDL)
    for (cls, off), r in sorted(rows.items(), key=lambda kv: (kv[0][0], kv[0][1])):
        per_class[cls] = per_class.get(cls, 0) + 1
        src = "engine-ai-atlas AI%d" % r["batch"]
        new = (r["size"], r["type"], r["name"], r["conf"], r["ev"], r["rd"], r["wr"], src, DATE)
        old = cur.execute("select size,type,name,confidence,evidence,readers,writers,source,date from struct_fields "
                          "where class=? and offset=?", (cls, off)).fetchone()
        if old is None:
            cur.execute("insert into struct_fields (class,offset,size,type,name,confidence,evidence,readers,writers,source,date) "
                        "values (?,?,?,?,?,?,?,?,?,?,?)", (cls, off) + new)
            st.inc("sf_added")
        elif RANK.get(norm_conf(old[3]), 0) > RANK[r["conf"]]:
            st.inc("sf_skipped"); st.inc("sf_skipped_lower")
        elif tuple(old[:8]) == tuple(new[:8]):  # same content (date ignored)
            st.inc("sf_skipped"); st.inc("sf_skipped_same")
        else:
            cur.execute("update struct_fields set size=?,type=?,name=?,confidence=?,evidence=?,readers=?,writers=?,source=?,date=? "
                        "where class=? and offset=?", new + (cls, off))
            st.inc("sf_updated")


def do_candidates(cur, st):
    cols = [r[1] for r in cur.execute("pragma table_info(candidates)")]
    want = ["id", "title", "subsystem", "hypothesis", "evidence", "expected_win", "risk", "status", "decision_date"]
    assert cols == want, "candidates columns changed: %s" % cols
    for c in CANDIDATES:
        old = cur.execute("select * from candidates where id=?", (c[0],)).fetchone()
        if old is None:
            cur.execute("insert into candidates values (?,?,?,?,?,?,?,?,?)", c)
            st.inc("cand_added")
        elif tuple(old) != tuple(c):
            cur.execute("update candidates set title=?,subsystem=?,hypothesis=?,evidence=?,expected_win=?,risk=?,status=?,"
                        "decision_date=? where id=?", c[1:] + (c[0],))
            st.inc("cand_updated")
        else:
            st.inc("cand_skipped")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--db", default=DEFAULT_DB, help="knowledge DB (default: the real DB next to the repo)")
    ap.add_argument("--dry-run", action="store_true", help="report what would change; roll everything back")
    ap.add_argument("-v", "--verbose", action="store_true", help="print every struct merge note")
    args = ap.parse_args()
    db = os.path.abspath(args.db)
    if not os.path.exists(db):
        sys.exit("DB not found: " + db)
    print("DB: %s%s" % (db, "  [DRY RUN, nothing written]" if args.dry_run else ""))

    rows, notes = build_struct_rows(None)
    for n in notes:
        if args.verbose or n.startswith(("WARNING", "MERGE", "SPLIT", "ADD")):
            print(n)

    conn = sqlite3.connect(db)
    conn.isolation_level = None  # explicit transaction so --dry-run can roll back DDL too
    cur = conn.cursor()
    cur.execute("BEGIN")
    st, per_class = Stats(), {}
    try:
        do_name_map(cur, st)
        do_lessons(cur, st)
        do_struct(cur, rows, st, per_class)
        do_candidates(cur, st)
    except Exception:
        cur.execute("ROLLBACK")
        raise
    cur.execute("ROLLBACK" if args.dry_run else "COMMIT")

    print("\n=== SUMMARY%s ===" % (" (dry run)" if args.dry_run else ""))
    print("name_map   : renamed %d | added %d (of which AI4 action-table %d) | skipped-existing %d (action-table rows already named: %d; conflicts: %d)" % (
        st["nm_renamed"], st["nm_added"], st["nm_action_added"], st["nm_skipped"], st["nm_action_existing"], st["nm_conflict"]))
    print("lessons    : added %d | updated %d | unchanged %d" % (st["les_added"], st["les_updated"], st["les_skipped"]))
    print("struct_flds: added %d | updated %d | skipped %d (same %d, lower-confidence %d)   [transcribed rows after merge: %d]" % (
        st["sf_added"], st["sf_updated"], st["sf_skipped"], st["sf_skipped_same"], st["sf_skipped_lower"], len(rows)))
    print("  per class: " + ", ".join("%s %d" % kv for kv in sorted(per_class.items())))
    print("candidates : added %d | updated %d | unchanged %d" % (st["cand_added"], st["cand_updated"], st["cand_skipped"]))
    chk = sqlite3.connect(db)
    print("integrity_check:", chk.execute("pragma integrity_check").fetchone()[0])
    chk.close()


if __name__ == "__main__":
    main()
