# NPC-Add PoC: one custom NPC per species in Telos Citadel Residential West (medical bed ward)
Status: PLAN (not started). Source recon: `.claude/research/npc_add/00_summary.md`, `R1..R5`, `poc/P1..P6`. Orchestrator Sonnet 5.5; bulk = Haiku waves (1 deliverable, <=3 files, <=60k tokens, 5-line contract). No OpenCode. No live writes without user GO.

## Decisions needed from the user (Gate 0)
1. AREA: A = 204TEL (Residential West, north bed ward, 20 plc_bed1 at y 22-31; 16 creatures in module) or B = 205TEL (own module, 7 creatures, Republic office props, no beds). Default: A.
2. SAVE: the PoC needs a save made BEFORE 204TEL was first visited (visited modules are frozen in the save). Candidates by mtime only: Game1, Game5, Game8. User checks AREANAME in the load menu and names one; else we start a fresh game.
3. Scope: stage the roster (canary of 4, then the rest) = default; or all at once.

## Mechanism (ranked, P3)
C hybrid: append creature entries to the 204TEL GIT inside `204tel.mod` + UTC templates in `override/`. Outside KPM (override/ and Modules/ are never packed in a kpatch). Fallback A: Override UTC that CreateObject()s the NPCs (needs the spawn-script probe).
Unknowns for the first probe: U1 does the engine read `204tel.mod` over `.rim/_s.rim` (ASSUMED, only npc_div_registry.py:80 says so); U2 do the chosen appearance rows resolve (appearance.2da is owned by npc_diversity); U3 do added GIT creatures need an ObjectId, object count / frame cost.

## Roster (P2; rows not in the diversity remap tables)
Tier 1 (race/body present, HIGH-MED): Twi'lek M 48, Twi'lek F 47, Wookiee M 271, Wookiee F 50, Rodian 39, Bith 12, Duros 24, Sullustan 601, Gand 569, Selkath 260, Devaronian 570 (human body + head), Gran 565, Quarren 537, Weequay 599.
Tier 2 (blank modela, race-MDL fallback UNPROVEN; probe first): Ithorian 72, Jawa 73 (walk 0.7), Trandoshan 464, Protocol droid 63 (race 5).
Not usable: Zabrak, Cathar, Mon Cal, Aqualish (no row, no MDL). Avoid collisions: 416 449 450 606 618 805 806 280 446 620.
Canary (4): Wookiee M 271, Rodian 39, Bith 12, Twi'lek F 47: two standing, one walking a 3-waypoint route, one idle.

## Template recipe (P4): clone vanilla `n_telf01` (202TEL) -> new resref/tag `npcadd_<species>`; FactionID 5 (neutral); Plot/Min1HP/NoPermDeath 0; IsPC 0; Interruptable 1; Race 6 (Selkath 2, droid 5); Appearance_Type = row; inline FirstName; SoundSet per species (vanilla 35 is a Rodian voice); Conversation blank for phase 1; scripts: stand = blank ScriptSpawn; walk = k_def_spawn01 + WP_<tag>_01..NN (>=2, ping-pong); idle = k_def_ambient + k_def_heartbt01 flag 29. Never: Plot/Min1HP/NoPermDeath=1, FactionID 0/1/3/6/19, Interruptable 0, reusing a vanilla TemplateResRef.

## Waves
### Wave 1: probe/recon (1 batched rea-recon + 3 Haiku; ~25 min; static, can run while the user plays)
- W1a rea-recon (ONE brief, swkotor2.exe): module resource lookup order (.mod vs .rim/_s.rim, override for GIT); whether CreateObject-spawned creatures run ScriptSpawn/heartbeat; blank-modela -> race MDL fallback in SetAppearance MDL-name path. Output `.claude/research/npc_add/poc/W1a_engine.md`.
- W1b Haiku: load all 18 rows (Tier 1+2) with pykotor: model, head, texture, soundset and creaturespeed resolve in chitin+override+modules; table. Scope `poc/W1b_rows.md`.
- W1c Haiku: walkmesh: read 204TEL `.wok` via the module/BIF path P1 failed on (try Installation.module resources), confirm 12 free walkable points + facing in the bed ward, 3 waypoint routes. Scope `poc/W1c_walkmesh.md`.
- W1d Haiku: read the vanilla 204TEL GIT creature struct fields (all keys incl. ObjectId, XPosition.., XOrientation) to emit exact append format. Scope `poc/W1d_git_fields.md`.

### Wave 2: build (6 Haiku, disjoint scopes; snapshot before, `agent_contract.py check --scope` after)
- B1 `patch_manager_mods/npc_add_src/species.toml` + `scripts/npc_add_build_utc.py` + test: 18 UTCs from the n_telf01 clone, byte-stable output into `patch_manager_mods/npc_add_src/out/override/`.
- B2 `scripts/npc_add_build_git.py` + test: reads vanilla `204tel.mod`, appends creature entries (positions from W1c), writes `out/modules/204tel.mod`; pykotor re-read asserts original entries unchanged, N added.
- B3 `scripts/npc_add_waypoints.py` + test: UTWs `WP_npcadd_<tag>_01..03` appended to the GIT Waypoint List (shares B2's output; sequence after B2).
- B4 `scripts/check_npc_add.py` + test: the 6 asserts (P6): GIT re-read, every model/texture/soundset/script resolves, tags/resrefs unique, no vanilla resref reused, forbidden UTC fields absent, row not in diversity collision list.
- B5 `scripts/npc_add_install.py` + test: backup `Modules/204tel.mod` + override files to `<game>/backups/npc_add_<stamp>/`, install, `--rollback <stamp>`; pattern from `kpm_hotswap.py`; refuses while the game runs; dry-run default.
- B6 docs: `patch_manager_mods/35_npc_add_poc.md` (PoC doc) + README of tool usage.
Ordering: B1/B2/B4/B5/B6 parallel; B3 after B2.

### Wave 3: verify + gate (3 Haiku counts + Sonnet): run all new tests + `check_npc_add.py` against `out/`, `generate/--check` for existing diversity files untouched, `git diff --stat`; commit explicit paths.

### Gate 1 (user GO, ~5 min): authorize the live write (`npc_add_install.py --apply`: replaces `Modules/204tel.mod` with backup). Confirm the game is closed and the D0 live-run pause is lifted.

### Gate 2 (user play, ~10 min, 3 steps): (1) load the pre-visit save, walk into Residential West -> medical bed ward (2) find the canary NPCs: standing, walking route, idle (3) talk/attack check, reload once, quit, say "done" and note positions of any problem. RMP log lines are supporting evidence only (first 300 hook calls then 1/500 sampling); the visual check is primary.

### Wave 4 (after Gate 2): fix positions/rows from notes; if U1 failed -> switch to route A (Override UTC + spawn script); expand to the full roster (Tier 1, then Tier 2 rows that the W1a fallback answer allows); then phase 2 (one-line DLG or bark via appended dialog.tlk strref, Wookiee Shyriiwook subtitle convention).

## Risks
Visited-area freeze; creature doubling in one small interior (cap ~18 additions, canary 4 first); neutral NPCs becoming hostile if attacked (untested); `.mod` over `.rim` precedence; Tier 2 blank-modela rows; hook 2 does not remap or protect custom rows (tag unreadable) but our rows are outside the commoner tables so they are not remapped; D0 pause on live runs.

## Estimates
Gate 0 answers 2 min; Wave 1 25 min; Wave 2 45 min; Wave 3 10 min; Gate 1 5 min; Gate 2 10 min; expand 60 min.
