#!/usr/bin/env python3
"""Persist the 2026-10-02 background research (Phase 0 tracks R1-R5, ai_research_2026-10-02/) into
ghidra_knowledge/kotor2_ghidra_knowledge.db. Idempotent: lessons by topic (updated in place); name_map rows (steam_addr UNIQUE)
are added only where no row exists. Every row here was spot-checked by the lead on disk (asm/source) unless marked medium."""
import os, sqlite3
DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-02"
SRC = "ai-research-2026-10-02"
EV = "ai_research_2026-10-02/"
# (addr, name, confidence, evidence file, role)
MAP = [
    (0x58A2C0, "CSWSCreature::GetItemRepository", "high", "R4", "(creature; 1): party members (+0x11AC) resolve to the one shared repository"),
    (0x6C0330, "GetCreatureHasTalent item-spell scan", "medium", "R4", "spell talent: scans 20 equip slots + shared repository for ACTIVATE_ITEM of that spell (0x58A2C0, 0x606100)"),
    (0x7FDFA0, "CSWCItem::GetName", "high", "R4", "thiscall(clientItem; CExoString* out, int identified) RET 8, EAX=out"),
    (0x73F530, "CClientExoApp item lookup by id", "medium", "R4", "ECX=*(*(0xA1B4A4)+4); id = *(serverItem+0x14) -> CSWCItem*"),
    (0x7FEE70, "CSWCItem -> server item", "medium", "R4", "ECX=clientItem"),
    (0x7356B0, "CExoLocString::GetString", "medium", "R4", "ECX=item+0x2C0; used by SetItem 0x8B1250 for the row text"),
    (0x8B1250, "CSWGuiInventoryItem::SetItem", "medium", "R4", "inventory row text = server item loc string +0x2C0"),
    (0x56E1E0, "CSWSCreature::RangeWeaponEquipped", "medium", "R3", "reads the baseitems ranged flag"),
    (0x57AC30, "combat-state refresh (ActionAttack path)", "medium", "R1", "0x568915 -> 0x57AC30 -> SetCombatState 0x57A560 at 0x57AD02/0x57AD15"),
    (0x5E9310, "CSWVarTable::GetLocalNumber", "medium", "R2", "index 0..31, one byte"),
]
LESSONS = [
    ("combat-ai", "FM2 verified: pathfind failure is treated as success + 2-4 s wait",
     "AIActionMoveToPoint 0x5BFC00: a failed path returns 3, and 0x5C21C4 CMP EAX,[0x9944BC] (=3) treats it like success; then rand()%100 -> "
     "AddPathfindingWaitActionToFront 0x5646C0 at 0x5C2484 (2.0-3.98 s) and return 3 at 0x5C25A5, no event. CheckMoveToObjectRadius re-queues a "
     "move + 0.3 s wait each pass (0x5BC652/0x5BC6D1/0x5BC6EA), so the companion loops and never attacks. The door branch sends event 0x1E to the "
     "door (0x5C0732), not OnBlocked; party OnBlocked (k_ai_master 2009) only opens doors, so firing OnBlocked would do nothing. Design: CA-FM2 "
     "0x5C2438 consumed exit 0x5C24C8 (log-only, then fail-fast) + probe 0x5BC5F4. Unproven: the pathfinder returns fail (not partial) for unreachable. "
     "Evidence: ai_research_2026-10-02/R1_engine_followups.md."),
    ("combat-ai", "A1 does not let combat lapse; the A1 retarget metric mismatch",
     "+0x524 (combat-exit ms) is set only by SetCombatState 0x57A560 (0x57A64F =8000, 0x57A67C =0) and counted down in AIUpdate (0x569957); every "
     "script ActionAttack refreshes it (0x568915 -> 0x57AC30 -> 0x57AD02/0x57AD15). EndAttackAction never calls SetCombatState, so the E2 3 s fallback "
     "fires with ~5 s left. New issue: GetNearestVisibleEnemyInRange accepts by 3D distance minus both radii (0x57CD51-0x57CDD6) but the attack tests "
     "2D centre distance (0x6D7E4A-0x6D7E5F) and InitializeNewAttackTarget sets +0x52C passive (0x6DACDB), so an a1_retarget target up to ~2 radii "
     "out of range ends silently (0x6D83F6/0x6D83FF). Fix in the A1 handler: a1_retarget_strict. R1 report."),
    ("combat-ai", "FM3/FM5/FM6/FM7/T1/R1/P2 re-check (2026-10-02)",
     "Verified in code: FM3 (scheduler wrap sentinel; the old probe window 0x51DBE7 is on the per-object hot path; use cold 0x51DBE1/0x51DD33/0x51E0C9; "
     "the fix changes ENEMY scheduling = user decision), FM5 (3 unthrottled ON_PATH_BLOCKED 0x1F post sites; throttle by retyping the event to 0, "
     "not by skipping, which leaks the 0x34-byte event), FM7, T1, R1, P2 (companion perception 4 s; hook 0x56BE79). FM6 plausible (consumer of the "
     "per-slot flag written at 0x7E6546 unknown). 10 candidate windows: check_windows ALL OK, both exes ALL MATCH (scratch run). R1 report."),
    ("party-ai-scripts", "Enemies already run our k_ai_master (routing census)",
     "2,354 creature templates: ScriptEndRound = k_def_combend01 on 1,930 (82%), k_hen_combend01 on 118; both just ExecuteScript(\"k_ai_master\") "
     "(k_def_combend01.nss:13). Only 18 of 53 include-embedding .ncs contain the decision tree (the compiler drops unused functions): k_ai_master, "
     "13 k_hen_<name>dlg, a_atkonend, a_force_combat, k_combat_rnd, k_def_rependd (ally repair droid, the only unrouted one). 51 module-local "
     "turn-hostile scripts (no sources) embed it but only run the first decision. So enemy fixes = gate flips in k_ai_master's include. "
     "Enemy H1/M2/M3/M4 are dead code: no script assigns NPC style 5 or sets SW_FLAG_BOSS_AI (only k_inc_generic.nss mentions it). "
     "Stage 0 enemy trace: log_all=1 alone (party_only=0 with log_all=0 passes nothing, CompanionAI.cpp:264); ~0.4 MB/min at 3 enemies. R3 report."),
    ("party-ai-scripts", "Free local numbers and the self-buff/5 m event facts",
     "Local numbers are 0..31 (one byte, 0x5E9310/0x5E9340). In use on companions: 8, 9, 20 (kite cooldown), 21 (kite streak), 16-19 (For Mandalore "
     "on Mand, m_cnd_mandquest.nss:17), 29 (k_oei_hench_inc.nss:270); free: 23-28, 30, 31. Vanilla self/ally buffs never fire: SW_FLAG_TARGET_FRIEND "
     "sets oTarget = self/leader (k_inc_generic ~650) but the cast and attack need GetIsEnemy(oTarget) (~915-929). k_hen_enter5m/exit5m are "
     "edge-triggered (FollowInfo+0x38 toggled at 0x7E77DA-0x7E78AE, 25.0 = 5 m squared), so they cannot restore follow for an idle far companion. R2 report."),
    ("party-ai-scripts", "Counting party medpacs from a script",
     "No special inventory owner: GetItemPossessedBy(GetFirstPC(), tag) resolves to the shared repository. Cheapest count = vanilla "
     "CheckInventoryNumbers (k_inc_item_gen.nss:250) over g_i_medeqpmnt01/02/03 (~9 engine commands). GetCreatureHasTalent(tSpe11_05) already implies "
     "stock >= 1 (0x6C0330 scans the repository). build_party_ai_scripts.py medpac_guard_check allows exactly one tSpe11_05 and must learn a second, "
     "party-gated one. Threshold: script constant for v1; INI-backed virtual global via GetGlobalNumber hook 0x67DC4A only as v2. R4 report."),
    ("inventory-sort-filter", "Mod 6 v2: name sort source and the level-requirement tint gate",
     "Sort works on server items (0x606100). Name key = the server item's CExoLocString at +0x2C0 via 0x7356B0, the same recipe SetItem 0x8B1250 "
     "uses for the row text; CSWCItem::GetName 0x7FDFA0 (client item via 0x73F530) is a fallback. Level tint in 0x5BD6A0 is unreachable with Mod 6's "
     "call (p4=p5=p6=0): 0x5BD6AA CMP [EBP+0x10],1 / JNZ skips it; it also needs [EBP+0x14]==0 (0x5BD6F7), the PC, GetLevel < min level and a "
     "module flag [..+0xD0] (0x5BD739) of unknown value; log that flag before shipping. R4 report."),
    ("tooling", "analyze_play_logs.py: play-session log analysis",
     "mods678_research/analyze_play_logs.py <playlogs dir> writes REPORT.md: DETOUR/consumed-exit table from the hook tomls, ai_trace checks (A1->E2 "
     "within 3.5 s, FM4 tid = T1, FM1 keep/clear, E1 SPIN with a1=1, caps), script markers + recast/E1-skip anomalies, ranged H1 los=0 per stance "
     "(--window for E3), inventory flush stats, pass/fail per checklist id (ai_research_2026-10-02/PLAY_CHECKLIST_2026-10-02.md). Tests: "
     "test_analyze_play_logs.py (35, incl. regexes vs the C++ format strings). Gaps: R5_log_tool.md."),
]
c = sqlite3.connect(DB); cur = c.cursor()
added = kept = 0
for a, name, conf, ev, role in MAP:
    addr = f"0x{a:08x}"
    if cur.execute("select 1 from name_map where steam_addr=?", (addr,)).fetchone():
        kept += 1; continue
    cur.execute("insert into name_map (steam_addr,name,source,confidence,evidence,date) values (?,?,?,?,?,?)",
                (addr, name, SRC, conf, f"{EV}{ev}; {role}", DATE)); added += 1
have = {r[0] for r in cur.execute("select topic from lessons")}
nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
new = []
for scope, topic, note in LESSONS:
    if topic in have: cur.execute("update lessons set note=? where topic=?", (note, topic)); continue
    cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); new.append(nxt); nxt += 1
c.commit()
print("name_map added", added, "kept existing", kept, "| new lesson ids", new)
print(cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0])
