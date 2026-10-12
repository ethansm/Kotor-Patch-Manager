# 35 NPC-add PoC: one custom NPC per species in Telos Residential West

Status (10-11): Wave 1 recon done. Wave 2 build in progress (scripts below are not on disk yet). Nothing installed. Gate 1 (user GO for the live write) not given.
Sources: `.claude/plans/npc_add_poc.md`, `.claude/research/npc_add/00_summary.md`, `.claude/research/npc_add/poc/P6_test_verify.md`, Wave 1 results in the plan (lines 45-52).
Unknowns are marked OPEN. Nothing here is verified in game.

## Goal

Add one custom NPC per species to the medical bed ward of Telos Citadel Residential West (module 204TEL), so each species appears as a placed, standing or walking NPC.

- Phase 1 (this doc): placed creatures only. No dialogue (Conversation blank).
- Canary first: 4 NPCs (see Roster). Then the rest of Tier 1, then Tier 2.
- Cap: about 18 added creatures in one small interior (plan Risks). Creature doubling is the main risk.
- Out of scope: phase 2 (one-line dialogue or barks, Wookiee subtitle convention).

## Area

- Module: 204TEL, Residential West, north bed ward. 20 `plc_bed1` placeables at y 22-31. 16 creatures in the module today (plan line 5).
- Ward room: 204TELb. WOK vertices are world coordinates. 20 of 20 beds are on walkable faces (W1c).
- Doctor Ganius: template and tag `corteldc`, conversation `corteldc`, at x -68.7 y 31.3 (plan line 5). He exists only in the `.mod` GIT, not in `rim+_s.rim` (W1 results).
- Area choice: option A (204TEL), decided 10-11 by the user. Option B (205TEL: 7 creatures, no beds) was not chosen.
- Spawn points: 12 free walkable points and 3 routes are in the W1c walkmesh report. Use picks with x <= -66.45 (other points sit between bed columns). Facing points at the bed centroid (-63.35, 27.00).
- Orientation format: unit vector toward the target point (`XOrientation = dx/len`, `YOrientation = dy/len`), not radians (Opus gate decision 2).
- Area display name: OPEN. Not in the repo. Read the `AREANAME` in the in-game load menu.

## Roster

Rows come from the diversity appearance table. Avoid collision rows 416 449 450 606 618 805 806 280 446 620.

| Tier | Species | Row | Note |
|---|---|---|---|
| 1 | Twi'lek M | 48 | |
| 1 | Twi'lek F | 47 | canary; loose Override file, see Open questions |
| 1 | Wookiee M | 271 | canary |
| 1 | Wookiee F | 50 | |
| 1 | Rodian | 39 | canary; walks |
| 1 | Bith | 12 | canary; idle |
| 1 | Duros | 24 | |
| 1 | Sullustan | 601 | |
| 1 | Gand | 569 | |
| 1 | Selkath | 260 | Race 2 |
| 1 | Devaronian | 570 | human body and head |
| 1 | Gran | 565 | |
| 1 | Quarren | 537 | |
| 1 | Weequay | 599 | |
| 2 | Ithorian | 72 | blank modela; probe first |
| 2 | Jawa | 73 | walk 0.7 |
| 2 | Trandoshan | 464 | blank modela; loose Override file |
| 2 | Protocol droid | 63 | Race 5 |

- Tier 1 = 14 rows. Tier 2 = 4 rows.
- Not usable (no row, no MDL): Zabrak, Cathar, Mon Cal, Aqualish.
- W1b: all 18 rows resolve (16 OK). Rows 47 and 464 rely on loose Override files that this mod does not ship. They work on this install only.
- Tier 2 model path: W1a found no race fallback in the model routine, but vanilla rows 72, 73, 464, 255, 63 render with 13-54 placements. Probably fine, still UNPROVEN.
- Canary (4): Twi'lek F 47 and Wookiee M 271 stand. Rodian 39 walks a 3-waypoint route. Bith 12 idles.

## Mechanism

Primary (C hybrid): append creature and waypoint entries to the live `Modules/204tel.mod` GIT, with the template UTCs and UTWs embedded in the same `.mod`.

- Method (W1d): `read_erf` -> `read_gff` -> copy the Creature List and Waypoint List -> append struct id 4 (creature) or 5 (waypoint) -> `set_list` -> `bytes_gff` -> `set_data("204tel")` -> `bytes_erf`. Do NOT use the GIT model; its round trip is lossy.
- Dry run (W1d): creatures 17 -> 18, waypoints 48 -> 49, the other 8 lists bit-exact, 186 of 186 resources kept.
- Target: live `204tel.mod`. It differs from `rim+_s.rim`: 5 extra resources, 6 extra DLGs.
- Creature entries omit `ObjectId`. The GIT loader reads it only on the instance path, and the default is 0x7F000000 (W1a).
- Templates: UTCs go inside the `.mod`. Override-vs-`.mod` precedence for UTC is unproven, so UTCs are not left to override/ alone. `out/override/` is a mirror only (Opus gate decision 6).
- Names (Opus gate decision 3): creature TemplateResRef = Tag = `npcadd_<slug>` (<=16). Waypoint Tag `WP_npcadd_<slug>_NN` (any length). Waypoint TemplateResRef `npcwp_<slug7>_NN` (<=16). Waypoints are numbered 01..NN with no gaps.
- Precedence: `.mod` is assumed to win over `.rim` (U1). Evidence is circumstantial (Doctor Ganius sits only in the `.mod` GIT). Confirm at Gate 2.

Fallback route A (only if U1 fails or the GIT route cannot work): Override UTC with ScriptSpawn `k_def_spawn01` that CreateObject()s the creature plus `WP_<tag>_NN` waypoints. Vanilla `GN_WalkWayPoints` walks them ping-pong, not looped (R2).
- Spawn-script firing for CreateObject-made creatures is UNRESOLVED (W1a). Route A is lower priority.
- override/ cannot be packed in a KPM kpatch (R5). Route A is not shippable through KPM as is.

Route B side effect: every existing save that has not yet visited 204TEL also gets the NPC (P6 OPEN 4).

Patch-only route (no `.mod` edit): hook H2 (`0x0055A460`, CSWSModule::LoadModuleInProgress), key on area resref (not area id or pointer), and wait about 30 ticks of unchanged add count (probe 0.2.0 POP-STABLE) before spawning. H2 fires on saved and fresh loads. R6's claim that 0x5582f0 never reaches 0x55a460 is REFUTED (see `.claude/research/npc_add/recon_hook/R7_probe_log_result.md`). Spawn mechanism and existence guard stay open (R7 O1-O2).

## Template recipe

Clone vanilla `n_telf01` (202TEL). Set these fields:

- TemplateResRef = Tag = `npcadd_<slug>`
- FactionID 5 (neutral)
- Plot 0, Min1HP 0, NoPermDeath 0
- IsPC 0
- Interruptable 1
- Race 6 (Selkath 2, droid 5)
- Appearance_Type = roster row
- FirstName inline: `Telos Resident`
- Gender 0
- SoundSetFile: 35 for the canary (Opus gate decision 4). Vanilla 35 is a Rodian voice, so non-Rodian canary voices are OPEN.
- Conversation blank (phase 1)
- ScriptSpawn: stand = blank; walk = `k_def_spawn01` plus `WP_<tag>_01..NN`; idle = `k_def_ambient` plus `k_def_heartbt01` with flag 29 (value as stated in the plan; no further detail in the sources)

Walk routes: waypoint count >= 3 (P6 gate A5). The plan line 19 says >= 2. The gate wins.

Never:
- Plot, Min1HP or NoPermDeath = 1
- FactionID 0, 1, 3, 6 or 19
- Interruptable 0
- a vanilla TemplateResRef

## Build, install, rollback commands

Run in this order. None of the four `npc_add_*` / `check_npc_add.py` scripts exists on disk yet (checked 10-11). They are Wave 2 outputs (B1, B2, B4, B5). `npc_div_whatis.py`, `kotor2_logs.py` and `kotor_save_forge.py` already exist.

1. Build UTCs (B1):
   `python3 scripts/npc_add_build_utc.py --vanilla-mod <202tel.mod> [--check]`
   Check what `--check` does with the script's `--help` once it is built.
2. Build GIT and waypoints (B2 with B3 helper):
   `python3 scripts/npc_add_build_git.py`
   Writes `out/modules/204tel.mod`. A pykotor re-read asserts the original entries are unchanged and N were added.
3. Check (B4): `python3 scripts/check_npc_add.py`
   Runs asserts A1-A6 (P6 section 2) against `out/`. A5 covers waypoints >= 3 and >= 1.0 m from placeables.
4. Install (B5), dry run is the default:
   - `python3 scripts/npc_add_install.py` (dry run)
   - `python3 scripts/npc_add_install.py --apply` (needs Gate 1 user GO, game closed)
   - `python3 scripts/npc_add_install.py --rollback <stamp>`
   Backups go to `<game>/backups/npc_add_<stamp>/`. The tool refuses while the game runs.
5. Manual rollback fallback (P6): `cp <backup>/204tel.mod "<game>/Modules/204tel.mod"`, then `sha256sum` against the backup hash.
6. Remove a PoC save slot by hand only after the user says yes. `python3 scripts/kotor_save_forge.py cleanup` removes only `LUO TEST` slots with a manifest.

## Play test and D0 check

Pre-check (before step 1):
1. D0 pause (memory, 10-10): no new live fix or map runs until the sandbox hides `$HOME`. Do not stop a live run without asking. Confirm it is lifted. OPEN: the repo has no record that it is lifted.
2. Game closed: `pgrep -fa '^S:.*swkotor2\.exe'` must print nothing.
3. Gate 1 user GO recorded for `npc_add_install.py --apply`.

Play test (about 2 min per step):
1. Fresh launch. Load the PoC slot first. Pass: it loads into the pre-visit area named in the load menu.
   - Slot candidates by mtime only: Game1, Game5, Game8. None is proven pre-Telos. The user checks `AREANAME` and time played in the load menu and names one. If none fits, start a fresh game.
2. Walk to the spawn point (A5 XYZ, with a landmark from the area). Pass: canary creatures stand where expected, are not doubled or floating, and the Rodian walks its route. Take a screenshot.
3. Tell Claude "done". Claude runs:
   - `python3 scripts/npc_div_whatis.py <x> <y>`
   - `python3 scripts/kotor2_logs.py check <npc-diversity log> --new`
   Pass: an RMP line at xy (exit 0). Exit 1 is inconclusive; the visual pass stands.

Log limits: the RMP log is rate-limited (calls 1-300 logged, then 800, 1300, ...). A missing line is not evidence of no spawn. Do not `cat`, `tail` or `grep` the raw log.

Plan Gate 2 also asks for a talk check, an attack check and one reload. These are not in the 3 steps above. Confirm with the user.

Rollback (confirm each step with the user first):
- Quit the game.
- Restore the module with the install tool's `--rollback <stamp>`, or with the manual `cp` fallback above.
- Remove any PoC override files from `<game>/override/`. The next launch then runs vanilla.

## Open questions

| ID | Question | Evidence so far | Resolve at |
|---|---|---|---|
| U1 | Does the engine load `204tel.mod` over `.rim`/`_s.rim`? | Doctor Ganius in `.mod` GIT only; no static path found (W1a) | Gate 2 |
| U2 | Does k_def_spawn01 walk a GIT-placed creature? | Spawn-script firing for CreateObject creatures UNRESOLVED (W1a) | Gate 2 |
| U3 | Does a blank ScriptSpawn stand without waypoints? | Not in the sources | Gate 2 |
| U4 | Is a neutral NPC (FactionID 5) hostile after an attack? | Untested | Gate 2 |
| U5 | Which save slot is pre-Telos? | None proven; candidates Game1, Game5, Game8 by mtime only | Load menu |
| U6 | Do Twi'lek F 47 and Trandoshan 464 need the loose Override files on other installs? | W1b: yes, this mod does not ship them | Packaging |
| U7 | Is SoundSetFile 35 (Rodian voice) acceptable for non-Rodian canary NPCs? | Vanilla 35 is Rodian | Gate 2 |
| U8 | Does Override UTC take precedence over `.mod` UTC? | Unproven; UTCs embedded in `.mod` | Route A decision |
| U9 | Is the Tier 2 model path fine (blank modela)? | W1a: probably fine; unproven | Tier 2 batch |
| U10 | Object count and frame cost at about 18 added creatures | Not measured (plan U3) | Gate 2 |
| U11 | Patch-only route: spawn point after H2, existence guard, save persistence | H2 fires on saved and fresh loads (R7); area id, area pointer and H1 latch are dead keys; H2 to H1 POP-STABLE observed on 7/7 probe 0.2.0 loads, 33-34 ticks after load (R7 "Probe 0.2.0 play result (10-12)"); spawn, existence guard and persistence still open | Spawn, guard and persistence probe, then Gate 2 |
