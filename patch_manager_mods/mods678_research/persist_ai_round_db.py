#!/usr/bin/env python3
"""Persist the 2026-10-01 companion AI round (engine review map, trace design VM facts, build/review lessons)
into ghidra_knowledge/kotor2_ghidra_knowledge.db. Idempotent: lessons by topic; name_map rows (steam_addr UNIQUE) are added
only where no row exists; rows the engine review proved wrong are demoted to confidence 'low' with a note."""
import os, sqlite3
DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-01"
SRC = "ai-review-2026-10-01"
EV = "ai_review_2026-10-01/review_ai_engine.md map"
# (addr, name, confidence, role)  H -> high, M -> medium
MAP = [
    (0x51D790, "CServerAIMaster::UpdateState", "high", "10 ms budget, 5 levels; vtbl+0x70 AIUpdate"),
    (0x569320, "CSWSCreature::AIUpdate", "high", "party/PC forced to AI level 4 (0x5694B2); 3 s slot-6 fallback 0x5699CE..0x569A72 (+0x570)"),
    (0x5D56B0, "CSWSObject::RunActions", "high", "1000 us budget [0x9944C4]; 1=requeue+stop, 2/3=delete+continue"),
    (0x5D7160, "ActionTypeStopsLoop", "high", "types 1, 5, 0x30, 0x33"),
    (0x6D9500, "CSWSCreature::AIActionCombat", "high", "type 0x3F; NULL spell row deref 0x6D9E2D (A4)"),
    (0x6DAB20, "CSWSCreature::EndAttackAction", "high", "ignores its argument"),
    (0x6DAC70, "CSWSCreature::InitializeNewAttackTarget", "high", "(id, node): node+0x38, +0x88, +0x52C=1"),
    (0x6DAB80, "CSWSCreature::HasTargetBeenDetected", "high", "party attackers always pass"),
    (0x6DA4F0, "CSWSCreature::CreateNewAttackActions", "high", "auto-continue, controlled creature only"),
    (0x6DACF0, "CSWSCreature::ResolveAttack", "medium", "NULL target safe"),
    (0x568540, "CSWSCreature::AddAttackActions", "high", "p8==0 -> combat-round queue (cap 4)+0x3F, else 0xC"),
    (0x540CA0, "CSWSObject::AddActionToFront", "high", "new(0x74), no de-dup"),
    (0x53F7F0, "CSWSObject::AddAction", "high", "new(0x74), no de-dup"),
    (0x5644B0, "CSWSCreature::AddMoveToPointAction", "high", "type 1; clears orientation lock"),
    (0x5BB2C0, "CSWSCreature::AIActionChangeFacingObject", "high", "type 0x13, instant"),
    (0x5BC1D0, "CSWSCreature::AIActionCheckMoveToObjectRadius", "medium", "type 0x11"),
    (0x5BFC00, "CSWSCreature::AIActionMoveToPoint", "medium", "type 1; pathfind 0x55AB30"),
    (0x590FF0, "CSWSCombatRound::ResolveEngagedState", "high", "party attacker always master"),
    (0x5912B0, "CSWSCombatRound::ResolveMasterState", "high", ""),
    (0x58E730, "CSWSCombatRound::StartCombatRound", "high", "3000 ms round"),
    (0x58E990, "CSWSCombatRound::EndCombatRound", "high", "slot 6 at 0x58F50F"),
    (0x571030, "CSWSCreature::UpdateCombatRoundTimer", "high", ""),
    (0x58F7A0, "CSWSCombatRound::IncrementTimer", "high", ""),
    (0x58FB40, "CSWSCombatRound::DecrementPauseTimer", "high", ""),
    (0x58FC60, "CSWSCombatRound::SetRoundPaused", "high", "+0xAA8/+0xAAC"),
    (0x58FCD0, "CSWSCombatRound::SetPauseTimer", "high", "+0xAB0/+0xAB4"),
    (0x58E6D0, "CSWSCombatRound::SetSpellCastRound", "high", "+0xA88/+0xA8C"),
    (0x590270, "CSWSCombatRound::AddAction", "medium", "cap 4, frees overflow"),
    (0x57A560, "CSWSCreature::SetCombatState", "high", "party: SetLocalNumber(8,0) at 0x57A5C1"),
    (0x5E9340, "CSWVarTable::SetLocalNumber", "high", ""),
    (0x56B8A0, "CSWSCreature::SpawnInHeartbeatPerception", "high", "every 3000-4199 ms; P1 timestamp never stored"),
    (0x5D0F30, "CSWSObject::UpdatePerception", "high", "mode 0 all / 1 party / 2 non-party"),
    (0x576DC0, "CSWSCreature::DoPerceptionUpdateOnCreature", "medium", ""),
    (0x57C990, "CSWSCreature::GetNearestVisibleEnemyInRange", "medium", "T1"),
    (0x56EFD0, "CSWSCreature::EventHandler", "medium", "OnBlocked only when not player-controlled"),
    (0x6FD8D0, "CVirtualMachine::RunScript", "high", "(slot/name, oid, 1)"),
    (0x73B720, "Timer::Init", "high", "QPC"),
    (0x73B810, "Timer::NowUs", "high", "QPC us"),
    (0x5915F0, "CSWSCombatRound::ClearAllSpecialAttacks", "medium", ""),
    (0x51E120, "CServerAIMaster::SetAILevel", "high", "4 = highest"),
    (0x51E040, "CServerAIMaster::AddObject", "high", ""),
    (0x586940, "CSWSCreature::GetIsCreatureBumpable", "high", ""),
    (0x586B70, "CSWSCreature::BumpFriends", "high", "FM1: ClearAllActions(1) on a bumped companion at 0x586F43"),
    (0x541080, "CSWSObject::ClearAllActions", "high", "(clearAttacks)"),
    (0x58A330, "CSWSCreature::SetPartyMemberFlag", "high", "FollowInfo alloc/free, AI level 4/2"),
    (0x574500, "CSWSCreature::SaveCreature", "high", "FM4: frees/rebuilds FollowInfo every save"),
    (0x5BF7F0, "CSWSCreature::AIActionFollowLeader", "medium", "type 0x3D"),
    (0x5BF940, "CSWSCreature::AIActionFollowOwner", "medium", "type 0x46"),
    (0x5BCDC0, "CSWSCreature::AIActionCheckMoveToFollowRadius", "medium", "type 0x40"),
    (0x5646C0, "CSWSCreature::AddPathfindingWaitActionToFront", "medium", "type 0x1E"),
    (0x7E6CE0, "CClientParty::RangeMonitor", "medium", "k_hen_enter5m/exit5m per frame"),
    (0x679050, "ExecuteCommandExecuteScript", "high", "sets [0xA7E10C] event, RunScript synchronous depth+1"),
    (0x687FB0, "ExecuteCommandGetRunScriptVar", "high", "reads [0xA7E10C]"),
    (0x665F50, "CNWSVirtualMachineCommands::InitCommands", "high", "877-entry table; AurPostString filled by 0x6F5B80"),
    (0x51C0A0, "CServerExoApp::GetGameObject", "high", "__thiscall(ECX=[[0xA1B4A4]+8], id) RET 4; NULL for stale ids (NOT cdecl)"),
]
DEMOTE = {
    0x412C00: "GUI code (callers 0x747010/0x7AC470), not WalkUpdateLocation/QuickWalk",
    0x418990: "GUI code, not AIActionWaitForEndOfRound",
    0x971DC0: "not a function in the full dump",
    0x983C20: "15-30 byte static-init stub, not AIActionTaunt",
    0x983D10: "15-30 byte static-init stub",
    0x984DE0: "15-30 byte static-init stub, not AIActionSurrenderToEnemies",
    0x9852D0: "15-30 byte static-init stub, not AIActionStanceMeditative",
    0x681880: "returns constant 0; GetLastAttacker name doubtful (sub-review)",
    0x696BF0: "GetAttacker name doubtful (sub-review)",
    0x5752C0: "CFollowInfo ctor, not RemoveMindTrickEffects",
}
LESSONS = [
    ("nwscript-vm", "VM state: running script name, OBJECT_SELF, event id, command table (Steam)",
     "VM=[0xA1B4A8], VMI=[VM+0x1C], depth=[VMI+0x28] (0..7, -1 idle). Script name = CExoString at VMI+0x44+depth*0x28 (char* = [that]). "
     "OBJECT_SELF = [VMI+0x18C+depth*4], mirrored at [impl+8], impl=[VMI+0x3D4] (= ECX in command handlers). ExecuteScript 0x679050 sets the "
     "event (GetRunScriptVar) global [0xA7E10C], then RunScript 0x6FD8D0 synchronously at depth+1. Command table 0x36D entries filled by 0x665F50 "
     "(+0x6F5B80): PrintString idx 1 0x68F260, SendMessageToPC 374 0x690A90, AurPostString 582 0x6F6200 (empty stub in retail), ShipBuild 761 "
     "0x692C40 (MOV [EBP-4],1 then push). A handler returning negative aborts the script. Evidence: designs/design_ai_trace.md s1-2."),
    ("nwscript-vm", "ShipBuild() only gates debug prints in every loadable script",
     "Scan of all 34,927 script copies (4,883 names) in the install: 518 ShipBuild calls guard only PrintString (495), AurPostString (5) or "
     "nothing (18). companion-ai T1 still scopes the flip to AI scripts (trace_scripts). Re-scan after installing script mods."),
    ("kpatch-framework", "consumed_exit: EAX = handler result on BOTH paths",
     "In a consumed_exit_address hook EAX equals the handler's return value on the normal resume path too, not only in the stolen bytes. "
     "EAX must be dead on both paths (check_windows.py eax_state checks it, incl. implicit readers since 2026-10-01b). Self-patching DLLs "
     "(k2-activation-fix 0x40D69E, k2-equip-icon-fix CALL rewrites 0x8ABA8C..0x8AE793) have no hooks in patch_config.toml: check_windows "
     "cannot see them; check their sites by hand at every install."),
    ("combat-ai", "Creature AI fields and script slots (Steam)",
     "Script slots +0x270+8*i: 0 HB,1 Notice,2 SpellAt,3 Attacked,4 Damaged,5 Disturbed,6 EndRound(+0x2A0),7 Dialogue,8 Spawn,9 Rested,10 Death,"
     "11 UserDef,12 Blocked,13 EndDialogue. +0x380 pathinfo; +0x520 in combat; +0x524 combat-exit ms; +0x52C bPassive/end-instead-of-approach; "
     "+0x550 attempted target; +0x558 last blocked pos; +0x570 slot-6 fallback ms; +0x104C orientation lock; +0x10DC combat round; +0x10E8 IsPC; "
     "+0x1198 stats (+0x146 stance); +0x11AC party. Combat round: +0xA84 active, +0xA94 timer, +0xA9C length, +0xAA8 paused, +0xB10 master, +0xB20 target. "
     "Only engine reader of the stance that changes behaviour: 0x6D8508 (11/12 no approach); 10 and 13 are script-only."),
    ("party-ai-scripts", "TSL Cure does not remove poison; heal-power reach",
     "k_inc_force.nss: FORCE_POWER_CURE's Sp_RemoveSpecificEffect(EFFECT_TYPE_POISON) is commented out (~937). Heal / Master Heal remove poison "
     "and heal only non-droid party members within 15 m of the caster. Any AI that targets poison or injury must use the same reach or it recasts forever."),
    ("party-ai-scripts", "Hostile creatures use the companion k_hen_* scripts",
     "HK-50s (Telos, Malachor), the 745kor training remote, the evil-Kreia boss and the Telos sisters run k_hen_attacked01/k_hen_damage01 etc. "
     "Party-only fixes in those paths must test IsObjectPartyMember(OBJECT_SELF). Companion .utc: OnDialogue k_hen_<name>dlg, spawn k_oei_spawn, "
     "end-conv k_oei_endconv, userdef k_oei_userdef (live copy from Obscured Tomb)."),
    ("party-ai-scripts", "Party AI script build + routing (installed 2026-10-01)",
     "ranged_ai_src/script/build_party_ai_scripts.py: 20 byte-identical vanilla proofs, Mod 8 repro 94db8304, 62 unique anchors (party_ai_patches.py), "
     "17 outputs; the 13 k_hen_<name>dlg route shouts to k_ai_master event 2004, a_atkonend/a_force_combat/k_combat_rnd to 3001 (router scripts, "
     "~3 KB each) so all party combat logic lives in one k_ai_master. Installer apply_party_ai_scripts.py (BEFORE.txt rollback; refuses to install "
     "over its own build: roll back first). Locals 20 (kite cooldown) and 21 (kite streak) collide only on non-party objects."),
]
c = sqlite3.connect(DB); cur = c.cursor()
cols = [r[1] for r in cur.execute("pragma table_info(name_map)")]
added = kept = demoted = 0
for a, name, conf, role in MAP:
    addr = f"0x{a:08x}"
    if cur.execute("select 1 from name_map where steam_addr=?", (addr,)).fetchone():
        kept += 1; continue
    cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                (addr, name, SRC, conf, (EV + ("; " + role if role else "")), DATE)); added += 1
for a, why in DEMOTE.items():
    addr = f"0x{a:08x}"
    row = cur.execute("select name, evidence from name_map where steam_addr=?", (addr,)).fetchone()
    if not row: continue
    note = f"DEMOTED {DATE} by {SRC}: {why}"
    if note in (row[1] or ""): continue
    cur.execute("update name_map set confidence='low', evidence=? where steam_addr=?", (((row[1] or "") + " | " + note).strip(" |"), addr)); demoted += 1
have = {r[0] for r in cur.execute("select topic from lessons")}
nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
for scope, topic, note in LESSONS:
    if topic in have: cur.execute("update lessons set note=? where topic=?", (note, topic)); continue
    cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1
c.commit()
print("name_map added", added, "kept existing", kept, "demoted", demoted)
print(cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0])
