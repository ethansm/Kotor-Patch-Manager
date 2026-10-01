#!/usr/bin/env python3
"""Persist the 2026-10-01 companion AI review lessons into ghidra_knowledge/kotor2_ghidra_knowledge.db.
Idempotent by topic. Lessons only; the pipeline map in ai_review_2026-10-01/review_ai_engine.md is not yet persisted to name_map (needs asm checks)."""
import os, sqlite3
DB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")
DATE = "2026-10-01"
LESSONS = [
    ("combat-ai", "In-combat 3 s script fallback (creature+0x570) and why an attack loop never gets it",
     "AIUpdate 0x569320: at 0x5699CE, if [cre+0x520]==1 and [cre+0x570]<=0 and vt+0x94()==0 (no queued action) it runs script slot 6 "
     "(end of round, cre+0x270+6*8) via 0x6FD8D0, then sets +0x570=3000 (0x569A72); otherwise it counts +0x570 down. AIActionPhysicalAttacks "
     "prologue 0x6D740F writes +0x570=3000 on every entry, so an attack action that re-queues itself each frame (stance 11/12 skip-approach path, "
     "Mod 8 stance 16 via E-H3) never reaches the fallback: the companion idles until the player gives it an order (review A1, runtime check pending). An "
     "E-H1-held attack ends the action, so the fallback fires within 3 s (A2). Lead-checked in the asm."),
    ("party-ai-scripts", "Vanilla k_inc_generic party AI bugs (H1 Revitalize, H2 heal target)",
     "k_inc_generic.nss:1006 'GN_CheckSeriesRevitalize() != -1' is always true (TRUE/FALSE return); GEN_TALENT_REVITALIZE case :2512 sets bValid=TRUE "
     "with TalentSpell(-1), then :2725 ClearAllActions + ActionUseTalentOnObject(invalid) + return TRUE: Jedi Support without Revitalize wastes rounds while "
     "anyone is KO'd. GN_CheckIfInjured :2318-2340 tests fDown00 (leader) in all three branches and GetRacialType(oP0) for oP1/oP2: AI heals only the leader, "
     "only when the leader is under 50%. Full list: patch_manager_mods/mods678_research/ai_review_2026-10-01/."),
    ("party-ai-scripts", "Companion OnDialogue scripts carry their own k_inc_generic copy",
     "Each companion .utc uses k_hen_<name>dlg (13 scripts; spawn k_oei_spawn, end-conv k_oei_endconv -> k_ai_master 1011, userdef k_oei_userdef). "
     "The dlg scripts include vanilla k_inc_generic and call GN_RespondToShout -> GN_DetermineCombatRound directly (ExecuteScript k_ai_master is "
     "commented out), so any change to the party AI in k_ai_master must also be recompiled into the 13 dlg scripts, plus a_atkonend, a_force_combat and "
     "k_combat_rnd, or the shout-driven first combat action runs vanilla logic."),
    ("party-ai-scripts", "Vanilla AI debug trace exists, gated by ShipBuild()",
     "k_inc_debug Db_MyPrintString and k_inc_gensupport GN_MyPrintString are 'if(!ShipBuild()) PrintString(s)'; about 230 calls in k_ai_master + "
     "k_inc_generic + k_inc_gensupport narrate every decision branch. Retail ShipBuild() is TRUE. A DLL hook on the ShipBuild/PrintString VM commands "
     "(scoped to AI scripts) yields a full decision trace without recompiling."),
    ("combat-ai", "Perception throttle timestamp never stored (review P1)",
     "Engine review P1 (verified by the reviewer): the throttle on non-party creatures perceiving party members never stores its timestamp, so they "
     "re-perceive (LOS raycast) every AI tick instead of every 0.3-4 s; party members refresh perception every 4 s, the controlled one every 200 ms (P2). "
     "Addresses in ai_review_2026-10-01/review_ai_engine.md. The engine zeroes local number 8 for party members at every combat exit (0x57A5C1)."),
]
c = sqlite3.connect(DB); cur = c.cursor()
have = {r[0] for r in cur.execute("select topic from lessons")}
nxt = (cur.execute("select max(id) from lessons").fetchone()[0] or 0) + 1
for scope, topic, note in LESSONS:
    if topic in have: cur.execute("update lessons set note=? where topic=?", (note, topic)); continue
    cur.execute("insert into lessons (id,scope,topic,note,date) values (?,?,?,?,?)", (nxt, scope, topic, note, DATE)); nxt += 1
c.commit(); print(cur.execute("pragma integrity_check").fetchone(), "lessons:", cur.execute("select count(*) from lessons").fetchone()[0])
