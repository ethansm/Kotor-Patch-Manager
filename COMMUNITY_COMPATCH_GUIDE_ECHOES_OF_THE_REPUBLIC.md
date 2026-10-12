# Compatibility Patch Guide: Echoes of the Republic (v0.93a)
## For the r/kotor Full Community Mod Build (154 Mods, Non-M4-78 Stack)

*Created by Antigravity AI & Ethan SM for the KOTOR modding community.*  
*Compatible with Linux (Proton / Steam Deck), macOS, and Windows.*

---

## 1. Executive Summary

**Echoes of the Republic** (v0.93a by vault13deathclaw) is an unofficial expansion for *Star Wars: Knights of the Old Republic II – The Sith Lords*, introducing over **128 new standalone modules**, 5 fully explorable planets (Coruscant, Manaan, Tatooine, Sleheyron, Yavin), 97 new quest categories, new Force powers, companion dialogue subtrees, and custom music/movies.

However, Echoes is distributed as a **1.01 GB monolithic loose-file overwrite archive**. In modern mod builds—such as the **154-mod r/kotor Community Mod Build** curated by Snigaroo—dropping this monolithic archive directly into the game folder causes:
1. **Critical Mod Breakage:** Overwrites *PartySwap* (breaking dual Handmaiden & Disciple mechanics), wipes out *Extended Enclave*, and erases community bugfixes (Mira, Lootra, Korriban fixes).
2. **Immediate Engine Crashes:** Drops 35 loose `.tga` textures that collide with existing `.tpc` textures, triggering an instant OpenGL buffer crash on Linux/Wine/Proton.
3. **Dialogue & Quest Corruption:** Overwrites `dialog.tlk` at index 136375, overwriting custom strings installed by prior mods.
4. **Quest Dead Ends:** Assumes the *M4-78 Enhancement Project* is installed, causing game-breaking quest dead ends and crashes on builds that deliberately exclude M4-78.

This guide provides the complete technical explanation and an **automated, turnkey compatch installer** that merges Echoes seamlessly into the 154-mod Community Build with **zero broken mods, zero texture crashes, and full PartySwap preservation**.

---

## 2. What Was Pruned & Why

To achieve 100% stability, our compatch preserves every piece of Echoes’ new planets and stories while surgically pruning destructive overwrites:

### A. M4-78 Decoupling (Community Build Non-M4-78 Stack)
* **Galaxy Map:** Decoupled `a_galaxymap.ncs` and `k_sup_galaxymap.ncs` to remove the M4-78 destination button, quest checks (`000_M478_Found`), and transit cutscenes.
* **Quest Log:** Excluded M4-78 quest entries from `LostJedi` (Master Vash on M4-78) and `fuel` (trading with M4-78 droids).
* **Dialogue:** Excluded Lieutenant Grenn’s reference to M4-78 droid fuel during the Battle of Telos.

### B. PartySwap & Bugfix Protection
* **Loose `k_003ebo_enter.ncs`:** Echoes shipped a loose script in `Override/`. If deployed, it overrides the PartySwap hook inside `Modules/003EBO.mod`, permanently deleting either Disciple or Handmaiden. We deleted the loose script to preserve PartySwap.
* **Module Overwrites (7 Base Capsules):** Echoes included raw `.mod` files for `204tel`, `221tel`, `302nar`, `303nar`, `601dan`, `701kor`, and `710kor`. Overwriting them would erase 13 community fix files in `303nar.mod` (Lootra & Mira fixes). We surgically extracted Echoes’ new doors, triggers, and waypoints and injected them into the live patched `.git` trees.
* **`dialog.tlk` Offset Remapping:** PartySwap and Extended Enclave claimed StrRefs 136375..136513. We dynamically appended Echoes’ strings starting at **136514**, updating all 2DA and item pointers.

### C. 4K Visual Protection
* **Low-Res Texture Suppression:** 35 loose `.tga` files that collided with 4K `.tpc` textures from the *Ultimate High Resolution Texture Pack* were suppressed, preventing engine crashes while keeping 4K graphics.
* **4K Skyboxes:** Preserved the 11MB & 12.5MB 4K cockpit skyboxes from Mod #85 (*HD Cockpit Skyboxes*) rather than downgrading to Echoes' 2.5MB vanilla textures.
* **Bundled Mod Deduplication:** Pruned 79 identical duplicate models and textures from *Droid Animations Fix* (Mod #123) and *HD Female Twi'leks* that were re-bundled inside Echoes.

---

## 3. The 7-Phase Compatch Architecture

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
         │                  │                       │                       │                  │
         └──────────────────┴───────────────────────┼───────────────────────┴──────────────────┘
                                                    ▼
                                    ┌───────────────────────────────┐
                                    │ Phase 6: Final Deployment     │
                                    │ (128 Modules, 12.7K Assets)   │
                                    └───────────────────────────────┘
                                                    │
                                                    ▼
                                    ┌───────────────────────────────┐
                                    │ Phase 7: Verification & Audit │
                                    │ (100% Pass Sanity & Regression│
                                    └───────────────────────────────┘
```

1. **Phase 1: Texture Sanitization (`01_sanitize_textures.py`)**  
   Resolves 59 internal duplicates and 35 cross-directory collisions between `.tga` and `.tpc`.
2. **Phase 2: Dynamic 2DA & TLK Reconciliation (`02_merge_2da_and_tlk.py`)**  
   Reconciles 12 tables (`appearance`, `heads`, `spells`, `placeables`, `modulesave`, `genericdoors`, `planetary`, etc.) and remaps custom strings cleanly onto StrRef 136514+.
3. **Phase 3: Core Scripting (`03_compile_galaxy_map.py`)**  
   Decouples M4-78 and re-compiles `a_galaxymap.ncs` and `k_sup_galaxymap.ncs` headlessly using `pykotor`. Deletes loose `k_003ebo_enter.ncs`.
4. **Phase 4: Base Game Module Surgical Merges (`04_merge_modules.py`)**  
   Unpacks 7 colliding `.mod` capsules, injects Echoes’ instances into the `.git` trees, and repacks:
   - `204TEL.mod`: Medical contact droid `corteldc` injected.
   - `221TEL.MOD`: Captain Tarkus spawn waypoint `tark_spwn1` injected; Grenn dialogue hooked.
   - `302NAR.mod`: IG-12 assassin door 19 unlocked; 10 waypoints and Talon gang injected.
   - `303nar.mod`: Darth Talon spawn trigger injected; **13 community fix files preserved**.
   - `601DAN.mod`: Dantooine Grove transition trigger and return waypoint `from_680` injected.
   - `701kor.mod`: Valley tomb placeables injected; `701kor.are` OnEnter hooked to `new_701area_ent`.
   - `710KOR.MOD`: Ancient Sith Temple transition placeable `kortempclick` and waypoint `from_770` injected.
5. **Phase 5: Companion Dialogue Splice & Journal / GUI Merge (`05_merge_dlgs_and_jrl.py`)**  
   - Spliced Echoes dialogue subtrees into `atton.dlg`, `handmaiden.dlg` (holocron training enabled), `kreia.dlg`, `mandalore.dlg`, `mira.dlg`, and `visasmarr.dlg`.
   - Merged 97 new quest categories into `global.jrl` (expanding it to 217 categories).
   - Injected 4 custom planet buttons into `galaxymap_p.gui` while preserving Malachor V and widescreen positioning.
6. **Phase 6: Final Deployment (`06_deploy_assets.py`)**  
   Copies 128 new standalone `.mod` capsules, 24 music tracks, 14 cutscenes, 1,008 voice files, and 12,704 unique assets. Skips 131 duplicate/managed files.
7. **Phase 7: End-to-End Regression Audit (`verify_all_compatch.py`)**  
   Automated verification of TLK bounds, 2DA corruptions, module struct counts, dialogue node counts, and GUI controls.

---

## 4. Turnkey Installation (One Command)

### Prerequisites
- Python 3.10+
- The **patched** pykotor venv described in `pykotor_patches/README.md`. Don't use
  plain `pip install pykotor`. Stock 2.3.12 reads some shipped 2DA tables as blank,
  and the merge would write those blanks back into your game. The installer checks
  for this and refuses to run on an unpatched pykotor.
- Echoes' galaxy map `.nss` sources (for `--map-src`)
- A TSLPatcher `nwnnsscomp.exe` (most mods ship one in `tslpatchdata/`) and
  Proton's Wine (`$KOTOR2_PROTON_WINE`, default Proton Experimental) to compile
  the galaxy map scripts. The installer stops before changing anything if either
  is missing.

### Running the Installer
To install the compatch automatically with pre-flight safety backup and verification:

```bash
# Clone or download the repository
cd KOTOR-II

# Run the master installer with the patched venv's Python
/path/to/patched/venv/bin/python3 scripts/install_echoes_compatch.py \
  --archive "/path/to/KOTOR 2 Echoes of the Republic.zip"
```

### CLI Options
- `--game-dir <path>`: Specify custom game directory (defaults to `$KOTOR2_ROOT`, else the standard Steam Proton path). Every phase patches this directory.
- `--staging-dir <path>`: The extracted Echoes files (default `$ECHOES_STAGING`, else `~/Games/KOTOR2_Mod_Workspace/staging_echoes`).
- `--map-src <path>`: Echoes' galaxy map `.nss` sources for phase 3 (default `$ECHOES_MAP_SRC`, else `~/Games/KOTOR2_Mod_Workspace/00_downloads/KOTOR 2 Echoes of the Republic Galaxy Map Source Files`).
- `--archive <path>`: Path to the Echoes `.zip` file (extracted into the staging dir if that is missing or empty).
- `--backup-dir <path>`, `--no-backup`: Where to write the safety tarball of `override/`, `Modules/` and `dialog.tlk`, or skip it.
- `--verify-only`: Run the verification suite without altering files.
- `--skip-pykotor-check`: Run even if pykotor fails the patched-2DA-reader check. Only use this if you know why the check fails.
- `--nwnnsscomp <path>`: The TSLPatcher `nwnnsscomp.exe` phase 3 compiles the galaxy map scripts with, the way Echoes and PartySwap did (default `$KOTOR2_NWNNSSCOMP`, else Repair HK-47's copy in `~/Games/KOTOR2_Mod_Workspace/01_extracted/mod127_repair_hk47/tslpatchdata/`).
- `--allow-builtin-nss`: Run without nwnnsscomp/Wine and let phase 3 use pykotor's built-in compiler. Don't: it swaps the arguments of engine calls, so the galaxy map it builds doesn't work in the game.

The phase scripts read the same environment variables (`KOTOR2_ROOT`,
`KOTOR2_WORKSPACE`, `ECHOES_STAGING`, `ECHOES_MAP_SRC`, `KOTOR2_NWNNSSCOMP`,
`KOTOR2_PROTON_WINE`, `KOTOR2_ALLOW_BUILTIN_NSS=1`) when you run them one by one.

**Phase 2 runs only once.** It appends rows and strings, so on a second run it
exits with "phase 2 already applied" and writes nothing. To run the whole
compatch again, restore the backup tarball first.

### Unified Toolset CLI (`kotor2_toolset.py`)
```bash
# Check installation health, module counts, and resolution
python3 scripts/kotor2_toolset.py status

# Run full 2DA, texture, and compatch verification
python3 scripts/kotor2_toolset.py verify

# List .tpc files that share a basename with a .tga/.dds (same rule as dedup_tga_tpc.py);
# add --fix to delete them
python3 scripts/kotor2_toolset.py dedup-textures

# Create a full game state snapshot
python3 scripts/kotor2_toolset.py backup
```

---

## 5. In-Game Testing Checkpoints

| Checkpoint | Location / Condition | What to Validate |
| :--- | :--- | :--- |
| **1. Character Creation & Peragus** | New Game Start | Boots past loading bar; custom appearance heads render properly without crash. |
| **2. Citadel Station (Telos)** | Residential & Czerka | `204tel` has medical droid `corteldc`; speaking to Lieutenant Grenn during the battle spawns Captain Tarkus in `221tel`. |
| **3. Refugee Quad (Nar Shaddaa)** | Sector 302 | Door 19 opens to the IG-12 assassin droid encounter; Talon gang spawns. |
| **4. Ebon Hawk & PartySwap** | Boarding Hawk post-Telos | Both Handmaiden and Disciple are present; companion dialogue trees offer new Echoes dialogue options. |
| **5. Galaxy Map Console** | Cockpit Console | Galaxy Map shows Sleheyron, Manaan, Coruscant, Tatooine, and Yavin; M4-78 is **not** on the map; Malachor V remains functional. |
| **6. Dantooine Grove Path** | Khoonda Plains (`601DAN`) | The transition trigger to the Dantooine Grove (`680dan.mod`) is open and walkable. |
| **7. Korriban Secret Temple** | Shyrack Caves (`710kor`) | Ancient Sith Temple placeable (`kortempclick`) warps player to the new tomb complex (`770kor.mod`). |

### If a checkpoint crashes to desktop
Get the real exception before changing anything. Set the game's Steam launch
option to `PROTON_LOG=1 %command%`, reproduce the crash, and read
`~/steam-208580.log`:

```bash
grep -n "dispatch_exception code=" ~/steam-208580.log | tail
```

Then run `kotor2_toolset.py verify`. `kotor2_mod_build.md`, "Getting a crash
log under Proton", covers what doesn't work (output redirects, running wine
outside Steam), how to read the log, and how this build's `c0000094`
divide-by-zero was traced to a corrupted `appearance.2da`.

---

## 6. Rollback

> **Corrected 2026-09-24.** An earlier version of this section said to `rm -rf`
> the whole game directory and then extract `backups/live_pre_echoes_compatch.tar`.
> **That tarball was never created** (the pre-compatch backup is a directory),
> so those steps would have deleted the game with nothing to restore. Don't use them.

What exists on disk:
- `~/Games/KOTOR2_Mod_Workspace/backups/live_pre_echoes_compatch/`: live
  `override/`, `dialog.tlk` and `swkotor2.ini` just before the compatch.
- `~/Games/KOTOR2_Mod_Workspace/backups/Modules_pre_swap_20260924/Modules/`:
  live `Modules/` with Echoes, before the capsule rebuild was swapped in.

**Since 2026-09-24, `Modules/` is the rebuilt one** (pykotor bug #6 repair,
see `kotor2_mod_build.md`, "Repair: replay capsule edits"). It already
contains Echoes' 7-module merge and 128 modules. **Don't restore `Modules/`
from any pre-Echoes backup**: that would also undo the rebuild. The 2DA fixes
made after the swap (`backups/override_2da_pre_a6_20260924/`) are also newer
than `live_pre_echoes_compatch/`.

To remove Echoes, restore only what it changed, file by file, never with
`rm -rf` on the game dir:
- `override/`: delete the files Echoes added (those not in
  `live_pre_echoes_compatch/override/`, but check the extras' files
  first; see `kotor2_mod_build.md`), and put back the 27 it replaced, plus `dialog.tlk`, from `live_pre_echoes_compatch/`.
  Then re-apply the post-swap 2DA fixes.
- `Modules/`: delete the 128 Echoes-only modules, and restore the 7 merged
  capsules from `~/Games/KOTOR2_Mod_Workspace/replay/pre_echoes_merge/` (the
  rebuilt capsules just before Echoes' merge was re-applied). Then re-run
  Visas' Dancer `Dance_Part_2`, which installed two scripts into `303NAR.mod`
  after the swap.
