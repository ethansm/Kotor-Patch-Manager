# P6 - test and verify plan for the NPC-add PoC (read-only recon, 10-11)

Method: repo docs and scripts read; saves dir listed with `ls -l` only (no save opened). No game launch, no writes outside this file.

## 1. Forged pre-visit save slot and restore
- Visit state is per module. A save holds one nested SAV per VISITED module (M0 s1). A module GIT edit reaches a save only if that module is not yet nested in it (R1 s4). "Before the medical center" therefore means "before the first visit to the module that holds the PoC creature".
- Forge entry points (skill kotor2-forced-state-testing, 26_FORCED_STATE_TESTING.md): `python3 scripts/kotor_save_forge.py list`; `... forge <src> <scenario>` (dry run); `... forge <src> <scenario> --write` (game closed, user yes); `... cleanup` (removes only `LUO TEST*` slots with forge_manifest.json). Saves dir: `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/cloudsaves/62703561`.
- Forge limits: new slot = max+1 only; edits existing Creature List entries by Tag; never adds an NPC and never removes a nested module SAV (R1 s3). Route B (NPC in the .mod GIT) needs only a pre-visit source slot. Routes A or C need the forge extended.
- Must know before forging: target module resref (2xxTEL, .mod vs .rim precedence OPEN); its area Name (not in repo); spawn XYZ on walkmesh; waypoint tags WP_<tag>_NN (routes A/B); NPC absent from PT_MEMBERS (PT_NUM_MEMBERS unchanged); no availnpc file for the NPC; target module not nested in the source; savenfo LASTMODULE not the target; GLOBALVARS untouched.
- `luo_scenario.py` (plan/apply/restore) and `luo_log_check.py` are LUO-only. The PoC needs neither.
- Slot list (names, mtimes, sizes from ls):
  - 000000 QUICKSAVE 10-09 00:50, 11.3 MB. Overwritten since M0 (10-03); M0's "no 2xxtel" fact is stale.
  - 000001 AUTOSAVE 10-09 00:50, 11.3 MB.
  - 000002 Game1 2019-08-02 (preserved mtime), 0.99 MB.
  - 000003 Game2 10-08 19:36, 9.1 MB. M0 (10-03 copy) nested 003ebo, 201tel, 202tel, 203tel: post-Telos.
  - 000004 Game3 10-06 23:37, 8.7 MB.
  - 000005 Game4 10-06 23:49, 7.9 MB.
  - 000006 Game5 09-27 18:38, 1.8 MB.
  - 000007 Game6 10-09 00:11, 9.9 MB.
  - 000008 Game7 10-09 00:53, 11.8 MB.
  - 000009 Game8 09-27 19:37, 3.6 MB.
  - 000010 Game9 10-01 22:17, 5.3 MB.
- Pre-Telos judgment (names, mtimes, sizes only): no slot is proven pre-Telos. Weakest-evidence candidates (oldest mtime, smallest size): Game1, Game5, Game8. No `LUO TEST` slot exists, so nothing leftover to clean.
- Zero-risk check: the in-game load menu shows each slot's AREANAME and time played (savenfo, M0 s4). Read it there; no file opening needed.

## 2. Offline checks before any launch (copy check_diversity_assets.py patterns)
- Reuse its parsers: parse_tables / parse_app_constants (DiversityTables.h, DiversityRows.h), load_appearance (read_2da), chitin_names (Installation), file_status / need_status (patch, then chitin, then live Override).
- Run `.venv/bin/python scripts/check_diversity_assets.py --no-resources` once the PoC row is added to appearance.2da (the generic-table gate).
- Asserts (6):
  - A1 Template: each new GIT Creature List TemplateResRef resolves to a UTC in the built .mod or override; the UTC parses (read_gff); Tag unique in the module; Appearance_Type = PoC row.
  - A2 Row: PoC row exists in appearance.2da with modela and texa set; it is absent from the IsGeneric* commoner/Twilek case lists and from APP_* 802-811.
  - A3 Assets: each model (.mdl+.mdx), texture (.tpc/.tga), head row, and soundset row (UTC SoundSetFile -> soundset.2da) resolves in patch, chitin, or live Override.
  - A4 Scripts: every script named by UTC/GIT/UTW exists as .ncs; `scripts/check_ncs_arg_order.py` passes on built override/ and Modules/*.mod (nwnnsscomp output, not pykotor).
  - A5 Placement: GIT XYZ lies on the area .wok walkmesh and is at least 1.0 m from every kKeepPositions entry (158 points, area-agnostic, tol 0.3 m: DiversityRoles.h:131, DiversityKeep.h:73). Waypoints WP_<tag>_NN: count >= 3, each on walkmesh (routes A/B).
  - A6 Package: against the source .mod, only the target GIT Creature List and the new UTC/UTW differ; every other resource is byte-equal; a second round trip is stable (M0 s7 rule). Only one file per module name on disk (201TEL.mod vs 201tel.mod case duplicates, R1 Q7).

## 3. In-game verification via the npc-diversity RMP log
- Can confirm a build at xy: each SetAppearance call that reaches the hook end logs RMP (oid, app before>after, xy, why) at NpcDiversityDispatcher.cpp:487. It proves the build call ran, not that the creature is visible or usable.
- Rate limit (main risk): `g_bRmp.take(300,500)` (NpcDiversityDispatcher.cpp:71, 487; NpcDivLog.h:210-219) is a process-wide counter, never reset. Per launch, calls 1-300 are logged, then only 800, 1300, ... A PoC creature built after call 300 is most likely dropped. A missing RMP line is NOT evidence of no spawn (npc_div_whatis exits 1 for both).
- Hiding and remap check (PoC row outside the commoner tables):
  - Remap: only commoner rows change (HookDecision.h:21-28, NotCommonerRow). Custom row: no row write, no tv write (PickTextureVar returns 0 for FamilyOf None, DiversityLogic.h:153-160). Not remapped.
  - Keep by position: area-agnostic, 0.3 m (DiversityKeep.h:73). A creature within 0.3 m of a vanilla keep point gets why=keep. Not hidden, just unchanged.
  - Keep by tag and companion/story exclusion are DEAD in this hook: tag is unreadable (`tag[1]={}`, tagN=0 at NpcDiversityDispatcher.cpp:401) and `ExcludeKind kind` is never assigned (line 426). RMP tag= is always empty; join by oid + xy.
  - ShouldVary (35%, oid + original row, DiversityLogic.h:143) runs before the row check: custom creatures log skip:vary about 65% of the time. A label only; nothing hidden.
  - The hook writes only appearance and textureVar (lines 455, 476). It never removes a creature.
- Lookup: `python3 scripts/npc_div_whatis.py <x> <y> [--tol 1.0]` (last launch only, exact float bits). Log check via `python3 scripts/kotor2_logs.py check <npc-diversity log> --new` (dispatches to npc_div_log_check.py). Do not tail or grep the raw log.
- Mitigation: (a) fresh launch and load the PoC slot first; (b) a probe DLL that also logs rows equal to the PoC row (a predicate change at line 487). Option (b) is a DLL change: needs user GO.

## 4. User play test (3 steps, about 2 min each) and rollback
1. Fresh launch: confirm the game is closed (`pgrep -fa '^S:.*swkotor2\.exe'` empty), start KOTOR II, load the PoC slot first. Pass: it loads into the expected pre-visit area.
2. Walk to the spawn point (A5 XYZ, with a landmark from the area). Pass: the creature stands there, is not doubled or floating, and walks its waypoints if route A/B. Take a screenshot.
3. Tell Claude "done". Claude runs `npc_div_whatis.py <x> <y>` and `kotor2_logs.py check --new`. Pass: RMP line at xy (exit 0). Exit 1 is inconclusive; the visual pass stands.

Rollback (confirm each step with the user first):
- Quit the game. `python3 scripts/kotor_save_forge.py cleanup` removes only LUO TEST slots with a manifest. A PoC slot with another name is deleted by hand after the user says yes.
- Restore the pre-PoC module: `cp <backup>/<module>.mod "<game>/Modules/<module>.mod"`, then `sha256sum` against the backup hash. No NPC-add rollback script exists yet (OPEN).
- Override files: remove `<game>/override/<poc file>`, then the next launch is vanilla.

## OPEN
1. Which 2xxTEL is "Residential West" and which is the medical center (not in repo; read the .are Name).
2. .mod vs .rim load precedence (R1 Q1).
3. Route B: whether a new GIT entry needs an ObjectId; waypoint wiring.
4. Route B side effect: every existing slot that has not visited the target module also gets the NPC.
5. Memory "pause live fix runs" (D0, 10-10): confirm it is lifted before step 1.
