# Implementation Plan: Echoes of the Republic Compatch
## For KOTOR II Full Community Mod Build (154 Mods, Non-M4-78 Stack)

> [!NOTE]
> **Status: FULLY IMPLEMENTED & VERIFIED (100% Complete)**  
> **Update 2026-09-24:** Phase 4 ran against the old `Modules/`, whose mod edits
> pykotor bug #6 had reverted. `Modules/` was then rebuilt by replaying every mod
> run (`kotor2_mod_build.md`, "Repair: replay capsule edits"), and Phase 4 and the
> 128-module copy were re-applied to the rebuilt capsules before the swap. All
> `verify_all_compatch.py` checks pass. Its `303nar` WaypointList count is now
> 100 (was 99), because the rebuilt file restores a mod waypoint edit.  
> All 7 implementation phases have been completed. Automated verification (`scripts/verify_all_compatch.py`) confirms that all 2DA tables, module capsules, companion dialogues, quest journals, galaxy map GUI controls, and texture assets are fully integrated without regressing any community build fixes.

This plan defines the engineering process, automated tooling, and verification protocols that were designed and executed to build the compatibility patch (compatch) between **Echoes of the Republic** (v0.93a by vault13deathclaw) and the **r/kotor Full Community Mod Build** (154 mods, curated by Snigaroo / Neocities).

---

## 1. Architectural Scope & Core Decisions

### 1.1 The M4-78 Decision
* **Policy:** **M4-78 Enhancement Project is explicitly excluded.**
* **Rationale:** The Community Build strictly prohibits M4-78 due to high bug density, pervasive voice/quest friction, and broad module-level incompatibility. Critical mods in the current stack (*PartySwap*, *Extended Enclave*, *For Mandalore*) were specifically compiled and installed using `"Without M4-78"` compatibility options.
* **Compatch Impact:** Echoes normally expects M4-78 in its galaxy map logic. The compatch will **decouple M4-78** by patching `a_galaxymap.nss` and `k_sup_galaxymap.nss` to stub out `PLANET_M4_78` routines and bypass `000_M478_Found` checks.

### 1.2 The Monolithic Overwrite Problem
* Echoes of the Republic is distributed as a **1.01 GB monolithic loose-file overwrite** (14,153 files) intended to be dropped directly into the game folder.
* Dropping it into a 154-mod install destroys the work of dozens of TSLPatcher installers by overwriting:
  * 12 critical `.2da` tables
  * 6 companion `.dlg` dialogue trees
  * 7 base-game `.mod` area capsules
  * Core Ebon Hawk logic (`k_003ebo_enter.ncs`, `k_inc_hawk.nss`)
  * Quest definitions (`global.jrl`)
  * 35 texture files that cause immediate engine crashes due to duplicate `.tga`/`.tpc` base names.
* **Solution:** Rather than copying loose files, our compatch acts as an **intelligent surgical injector**: extracting only delta rows, grafting dialogue branches, merging GFF module trees, and deduplicating textures before deployment.

---

## 2. Conflict Matrix & Subsystems

```
                       ┌────────────────────────────────────────────────────────┐
                       │           Custom Compatch Architecture                 │
                       └────────────────────────────────────────────────────────┘
                                                    │
         ┌──────────────────┬───────────────────────┼───────────────────────┬──────────────────┐
         ▼                  ▼                       ▼                       ▼                  ▼
┌─────────────────┐ ┌───────────────┐     ┌───────────────────┐   ┌───────────────────┐ ┌───────────────┐
│ Phase 1: Assets │ │ Phase 2: 2DA  │     │ Phase 3: Galaxy & │   │ Phase 4: Modules  │ │ Phase 5: DLG  │
│  & Deduplication│ │  Table Merge  │     │  PartySwap Script │   │   Surgical Merge  │ │  & JRL Splice │
└─────────────────┘ └───────────────┘     └───────────────────┘   └───────────────────┘ └───────────────┘
```

| Subsystem | Source in Echoes | Conflict in Community Build | Compatch Resolution Strategy |
| :--- | :--- | :--- | :--- |
| **Galaxy Map & Ebon Hawk** | `a_galaxymap.ncs`, `k_sup_galaxymap.ncs`, `k_003ebo_enter.ncs` | Hardcoded M4-78 checks; overwrites PartySwap (#98) dual-companion spawning & Galaxy Map Fix Pack (#135). | Decompile NSS, strip M4-78 checks, merge PartySwap spawning loop into Echoes enter script, recompile with pykotor. |
| **2DA Tables (12)** | `appearance`, `heads`, `spells`, `upgrade`, `placeables`, `globalcat`, `modulesave`, `genericdoors`, `planetary`, `ambientmusic`, `soundset`, `effecticon` | Monolith copies overwrite rows added by 50+ mods (Hologram Models, lightsaber varieties, PartySwap). | Programmatic 2DA diff against vanilla baseline; dynamically append delta rows onto live tables using pykotor. |
| **Base Modules (7)** | `204tel`, `221tel`, `302nar`, `303nar`, `601dan`, `701kor`, `710kor` | Overwrites module capsules patched by K2CP (#12), PartySwap (#98), For Mandalore (#120), Mira Escape (#113), Jedi Masters (#107). | Extract `.mod` capsules, diff `.git` (Game Instance Tree) to extract Echoes' new doors/triggers/creatures, merge into live `.git`, repack `.mod`. |
| **Companion Dialogs (6)** | `atton.dlg`, `handmaiden.dlg`, `kreia.dlg`, `mandalore.dlg`, `mira.dlg`, `visasmarr.dlg` | Overwrites trees patched by PartySwap, For Mandalore, Kreia Fall, Visas VO. | Parse DLG GFF structures; graft Echoes’ new conversational subtrees onto the live patched DLG files. |
| **Quest Journal** | `Override/global.jrl` | Overwrites the entire game's quest database with the author's copy. | Parse JRL structures; merge Echoes’ new quest categories/entries into live `global.jrl`. |
| **Engine Crash Textures** | 7,954 loose textures in `Override/` | 35 base-name collisions between existing `.tpc` and Echoes `.tga` (or vice-versa). | Run `dedup_tga_tpc.py` across incoming assets before installation. |
| **Cockpit Window Models** | `003EBOq` room animations, `ebo_sky01.tga`, `ebo_sky02.tga` | Conflicts with HD Cockpit Skyboxes (#85) & Transparent Windows (#84). | Exclude community cockpit skyboxes to let Echoes' planet-projection animations render cleanly. |

---

## 3. Phased Implementation Protocol

### Phase 0: Staging & Safety Baseline
1. **Full Working Backup:**
   Create an immutable snapshot of the working game directory before any Echoes files touch disk:
   ```bash
   tar -cpf ~/Games/KOTOR2_Mod_Workspace/backups/live_pre_echoes_$(date +%Y%m%d_%H%M%S).tar \
     -C "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II" .
   ```
2. **Download & Staging:**
   Download `KOTOR 2 Echoes of the Republic.zip` (~1.01 GB) using `ds_download.py` or curl, and extract exclusively to an isolated staging folder:
   `~/Games/KOTOR2_Mod_Workspace/staging_echoes/`.
3. **Environment Verification:**
   Ensure the local Python virtual environment (`.venv`) has `pykotor` active with our patches (#4 capsule case resolution and #5 binary 2DA Kaitai fallback).

---

### Phase 1: Asset Sanitization & Collision Deduplication
1. **TGA / TPC Deduplication:**
   Compare all textures in `staging_echoes/Override/` against the live `Knights of the Old Republic II/override/`.
   Identify all 35 colliding pairs (e.g. `dxn_mt05`, `kor_sky03`, `m36_rock`, `kor_flr01`, `nar_mat`).
   For any colliding texture:
   * If a high-res TPC already exists in `override/` from the Ultimate High Resolution Texture Pack, suppress Echoes' loose `.tga`.
   * If Echoes provides a new unique texture that clashes only in format, delete the inferior loose copy using `dedup_tga_tpc.py`.
2. **Cockpit Skybox Clean-Up:**
   Remove community mods #81, #82, #84, and #85 (`ebo_sky*.tga`, `003EBOq.mdl`) from `override/` to prevent clipping with Echoes' dynamic planet projections.

---

### Phase 2: Dynamic 2DA Table Reconciliation
Write and run `scripts/merge_echoes_2da.py`:
1. For each of the 12 tables:
   * Load clean baseline copy from `~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean`.
   * Load Echoes' table from `staging_echoes/Override/<name>.2da`.
   * Load live patched table from game `override/<name>.2da`.
2. Compute delta rows: isolate rows where `row_id >= baseline_height` or where cell content differs from vanilla.
3. Append those delta rows onto the live table:
   * Keep a lookup table of old Echoes row ID $\rightarrow$ new live row ID.
   * If any Echoes `.utc` (creature) or `.uti` (item) explicitly references an `appearance.2da` or `upgrade.2da` row number, update that GFF field to point to the newly assigned row index.
4. Run `verify_2da.py` on all live tables to confirm zero blank-row corruption.

---

### Phase 3: Core Scripting & PartySwap Integration

#### 3.1 Galaxy Map Script (`a_galaxymap.nss` & `k_sup_galaxymap.nss`)
1. Edit `a_galaxymap.nss`:
   * Remove lines 162–165:
     ```c
     // REMOVE M4-78 CHECK:
     // if ((GetGlobalNumber("000_M478_Found") == 0)) {
     //     SetPlanetAvailable(PLANET_M4_78, FALSE);
     //     SetPlanetSelectable(PLANET_M4_78, FALSE);
     // }
     ```
   * Explicitly set `SetPlanetAvailable(PLANET_M4_78, FALSE);` and `SetPlanetSelectable(PLANET_M4_78, FALSE);` permanently so M4-78 never appears on the map.
2. Edit `k_sup_galaxymap.nss`:
   * Remove or comment out `case PLANET_M4_78:` transition logic (lines 305–311).
   * Preserve all Echoes destinations: Coruscant (`PLANET_LIVE_02`), Yavin (`PLANET_LIVE_03`), Manaan (`PLANET_LIVE_04`), Tatooine (`PLANET_LIVE_05`), and Sleheyron.
3. Compile both scripts to `.ncs` using `pykotor`'s internal script compiler.

#### 3.2 Ebon Hawk On-Enter (`k_003ebo_enter.nss` & `k_inc_hawk.nss`)
1. Extract PartySwap's modified `k_003ebo_enter.nss` from `01_extracted/mod98_partyswap`.
2. Inspect Echoes' `k_003ebo_enter.nss` (source already acquired in `/tmp/eotr_map`).
3. Merge the two files:
   * Graft PartySwap's companion spawning routines (`DoHandmaidenSpawnIn`, `DoDiscipleSpawnIn`) into `k_inc_hawk.nss`.
   * Graft PartySwap's puppet swap handling, female Exile Handmaiden support, and romance state checks into Echoes' `main()` and `DoCutscenes()` functions.
   * Preserve Echoes' Visas/Mandalore banters, Handmaiden Jedi training follow-up quest, and `SetBackground()` calls.
4. Compile to `k_003ebo_enter.ncs` and deploy to `override/`.

---

### Phase 4: Base Game Module Surgical Merges (7 Capsules)
Write and execute `scripts/merge_echoes_modules.py`:
1. **Target Modules:**
   * `601DAN.mod` (Khoonda / Dantooine Outskirts)
   * `710kor.mod` (Shyrack Caves / Secret Tomb)
   * `701KOR.mod` (Valley of the Dark Lords)
   * `302nar.mod` (Nar Shaddaa Refugee Quad)
   * `303NAR.mod` (Nar Shaddaa Docks)
   * `204tel.mod` (Citadel Station Residential)
   * `221tel.mod` (Citadel Station Czerka / Restored)
2. **Surgical Procedure per Module:**
   * Unpack the live patched `.mod` from `Knights of the Old Republic II/Modules/<name>.mod`.
   * Unpack Echoes' `.mod` from `staging_echoes/Modules/<name>.mod`.
   * **Merge Game Instance Trees (`.git`):**
     * Load live `*.git` and Echoes `*.git` using `pykotor.resource.formats.gff`.
     * Scan Echoes' `Door List`, `Placeable List`, and `Trigger List`.
     * Identify new instances added by Echoes (e.g. the Grove transition door in `601dan.git` and the temple passages in `710kor.git`).
     * Append these instances into the live `*.git` tree, checking that coordinates do not collide with objects placed by K2CP (#12) or Jedi Masters (#107).
   * **Merge Area Definitions (`.are` / `.ifo`):**
     * Keep the live community build `.are` (preserves fog, lighting, and camera fixes from K2CP).
     * Inject any new transition hooks or script parameters defined by Echoes.
   * **Re-pack Capsule:**
     * Pack all resources back into `Modules/<name>.mod` using `pykotor.resource.formats.erf`.

---

### Phase 5: Companion Dialogue Splice (6 Trees) & Journal Merge
Write and execute `scripts/merge_echoes_dialog.py`:
1. **Target Files:**
   * `atton.dlg`, `handmaiden.dlg`, `kreia.dlg`, `mandalore.dlg`, `mira.dlg`, `visasmarr.dlg`.
2. **Splicing Algorithm:**
   * Load the live patched DLG (containing PartySwap / For Mandalore / Kreia Fall changes).
   * Load Echoes' DLG.
   * Identify all new top-level `StartingList` entries and branch subtrees in Echoes:
     * Visas: Personal quest dialogue & prestige class reward conversation.
     * Handmaiden: Jedi training holocron quest dialogue.
     * Mandalore: War story dialogues & Visas banter triggers.
     * Mira: Manaan arrival banter.
   * Copy the new entry nodes, replies, and script conditionals into the live DLG structure.
   * Re-link reply pointers so existing PartySwap conversation paths remain accessible.
3. **Journal Merge (`global.jrl`):**
   * Load live `override/global.jrl` and Echoes `staging_echoes/Override/global.jrl`.
   * Find all quest categories in Echoes with tag `Plot_*` or custom quest prefixes not in vanilla/TSLRCM.
   * Append those quest categories and entry stages into the live `global.jrl`.

---

### Phase 6: Deployment & File Layout
Execute `apply_echoes_compatch.py`:
1. Copy Echoes' **completely new modules** (over 120 standalone `.mod` files for Manaan, Coruscant, Tatooine, Kashyyyk, Yavin, Rakata Prime, Sleheyron) into `Modules/`.
2. Copy Echoes' **audio assets** into `StreamMusic/` and `StreamVoice/`.
3. Copy Echoes' **movies** into `Movies/`.
4. Copy Echoes' **non-colliding models, textures, and items** into `override/`.
5. Deploy our **patched files**:
   * Spliced `.2da` tables $\rightarrow$ `override/`
   * Spliced `.dlg` files $\rightarrow$ `override/`
   * Merged `global.jrl` $\rightarrow$ `override/`
   * Recompiled `a_galaxymap.ncs`, `k_sup_galaxymap.ncs`, `k_003ebo_enter.ncs` $\rightarrow$ `override/`
   * Surgically merged 7 base `.mod` capsules $\rightarrow$ `Modules/`

---

## 4. In-Game Verification & Testing Matrix

| Test Checkpoint | Location / Condition | What to Validate | Failure Indicator & Fix |
| :--- | :--- | :--- | :--- |
| **Checkpoint 1: 2DA Integrity** | Character Creation $\rightarrow$ Peragus Start | Game boots past loading bar; character creation models load; Peragus module loads. | Divide-by-zero or crash at loading bar ~1/3 $\rightarrow$ check `appearance.2da` / `heads.2da` via `verify_2da.py`. |
| **Checkpoint 2: Texture / Area Load** | Peragus $\rightarrow$ Citadel Station (Telos) | Docking at Citadel Station succeeds; residential modules (`204tel`, `221tel`) load without crash. | Crash on module transition $\rightarrow$ TGA/TPC collision; check `dedup_tga_tpc.py` logs and `~/steam-208580.log`. |
| **Checkpoint 3: Ebon Hawk & PartySwap** | Boarding Ebon Hawk after Telos Polar Academy | Both Handmaiden and Disciple spawn; cutscenes play; cockpit window renders space/hyperspace cleanly. | Handmaiden or Disciple missing $\rightarrow$ `k_003ebo_enter.ncs` PartySwap loop issue. Black cockpit window $\rightarrow$ skybox texture clash. |
| **Checkpoint 4: Galaxy Map Operation** | Using Galaxy Map console on Ebon Hawk | M4-78 is **not** on the map; Coruscant, Manaan, Tatooine, Kashyyyk, Yavin, Sleheyron are selectable. | M4-78 visible or clicking crashes $\rightarrow$ `a_galaxymap.ncs` / `k_sup_galaxymap.ncs` destination switch statement mismatch. |
| **Checkpoint 5: Dantooine Grove Transition** | Khoonda (`601DAN`) $\rightarrow$ Kinrath Cave path | Path previously blocked in vanilla is open; walking through transitions to Grove module (`680dan.mod`). | Invisible barrier or door missing $\rightarrow$ `601dan.git` door/trigger injection issue. |
| **Checkpoint 6: Korriban Caves Transition** | Shyrack Caves (`710kor`) $\rightarrow$ Cave Back | Back passage transitions to new temple modules (`740KOR.mod`, etc.). | Transition trigger does not fire $\rightarrow$ `710kor.git` trigger hook missing. |

---

## 5. Rollback Procedure

> **Corrected 2026-09-24.** The steps that used to be here (`rm -rf` the game
> directory, then extract `backups/live_pre_echoes_*.tar`) must not be used. No
> such tarball exists (the Phase 0 backup was taken as the directory
> `backups/live_pre_echoes_compatch/`), and `Modules/` has since been rebuilt
> (pykotor bug #6 repair) with this merge re-applied, so restoring a pre-Echoes
> `Modules/` would also undo that rebuild. See the "Rollback" section of
> `kotor2_echoes_community_guide.md` for the file-by-file procedure.
