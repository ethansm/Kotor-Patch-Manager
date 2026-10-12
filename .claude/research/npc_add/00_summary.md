# Adding NPCs to vanilla areas: feasibility (Haiku recon R1-R5, 10-11)
Verdict: YES. Three routes, cheapest first. Details in R1..R5 in this dir.
A. Override-only script spawn (R2): an Override UTC for a Telos creature runs a k_* script that CreateObject()s the Wookiee (UTC ScriptSpawn=k_def_spawn01) plus WP_<tag>_NN waypoints (UTW). Vanilla GN_WalkWayPoints walks them ping-pong (no loop). No module edit, but KPM cannot ship override/ (R5).
B. Module GIT Creature List append + UTC in a .mod (R1, precedent mod115, scripts/04_merge_modules.py): works for new games / saves from before the area was visited; visited areas are frozen in the save.
C. DLL: RunScript 0x6FD8D0 on a shipped .ncs (R3). Direct creature build rejected (no object-id allocator, LoadCreature needs GFF). Needs probe of CreateObject from a DLL-run script.
Wookiee: row 271 (N_WookieM, 7 vanilla placements) usable as is; hook 2 does not touch Wookiee rows (R5). Voice: 0/60 alienvo cells resolve, only 4 happy growl WAVs exist (R4). Dialog: new DLG in Override attachable to a UTC; Shyriiwook subtitle convention UNSURE.
Open: .mod vs .rim vs _s.rim precedence (R1); Wookiee SoundSetFile + DLG text needs one read-only pykotor pass (R4); faction/hostility, object limits (R5).
Suggested PoC: one Wookiee, one Citadel area, 3 waypoints, route A or B, new save before the visit, no dialog.
