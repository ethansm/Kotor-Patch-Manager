"""
06_deploy_assets.py - Phase 6: Final Deployment & Asset Copy

Deploys sanitized, non-colliding assets from staging into the live KOTOR 2 game directory:
1. 128 new standalone .mod planet capsules into Modules/
   (excluding the 7 colliding modules which were merged in Phase 4)
2. 24 new ambient music tracks into StreamMusic/
3. 14 new cinematic movies into Movies/
4. 1,008 voice files into StreamVoice/ (preserving relative tree)
5. 12,704 non-colliding override files into override/
   (strictly skipping the 131 colliding files to protect merged 2DAs,
    spliced dialogues, global.jrl, galaxymap_p.gui, and high-res Community Build textures)
6. Phase 3's galaxy map scripts into override/, replacing the build's copy only
   when it is a known one whose changes Echoes' source already has
"""
import hashlib

import os
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from _common import GAME_DIR, STAGING_DIR  # noqa: E402

COLLIDING_MODULES = {
    "204tel.mod",
    "221tel.mod",
    "302nar.mod",
    "303nar.mod",
    "601dan.mod",
    "701kor.mod",
    "710kor.mod",
}

COLLIDING_OVERRIDE_FILES = {
    "ambientmusic.2da",
    "appearance.2da",
    "atton.dlg",
    "c_drdminelg.mdl",
    "c_drdminelg.mdx",
    "c_drdminesm.mdl",
    "c_drdminesm.mdx",
    "c_mk1_drd.mdl",
    "c_mk1_drd.mdx",
    "c_mk2_drd.mdl",
    "c_mk2_drd.mdx",
    "c_tankdroid.mdl",
    "c_tankdroid.mdx",
    "desktop.ini",
    "ebo_sky01.tga",
    "ebo_sky02.tga",
    "effecticon.2da",
    "galaxymap_p.gui",
    "genericdoors.2da",
    "global.jrl",
    "globalcat.2da",
    "handmaiden.dlg",
    "heads.2da",
    "k_003ebo_enter.ncs",
    "kreia.dlg",
    "llk_dahnisb.mdl",
    "llk_dahnisb.mdx",
    "llk_dahnisb01.tpc",
    "llk_dahnish.mdl",
    "llk_dahnish.mdx",
    "llk_dahnish.tpc",
    "mandalore.dlg",
    "mira.dlg",
    "modulesave.2da",
    "n_repcptmw.mdl",
    "n_repcptmw.mdx",
    "n_twiass.mdl",
    "n_twiass.mdx",
    "n_twiassblue01.tpc",
    "n_twiassblued01.tpc",
    "n_twiassoga01.tpc",
    "n_twiassogb01.tpc",
    "n_twiassred01.tpc",
    "n_twidance.mdl",
    "n_twidance.mdx",
    "n_twidance_boot.mdl",
    "n_twidance_boot.mdx",
    "n_twidanceb01.tpc",
    "n_twidancebd01.tpc",
    "n_twidancec01.tpc",
    "n_twidancecd01.tpc",
    "n_twiund.mdl",
    "n_twiund.mdx",
    "n_twiundblue01.tpc",
    "n_twiundblued01.tpc",
    "n_twiundgreenr01.tpc",
    "n_twiundgreerd01.tpc",
    "n_twiundred01.tpc",
    "p_twi1.tpc",
    "p_twi1b.tpc",
    "p_twi1d1.tpc",
    "p_twi1d2.tpc",
    "p_twi2.tpc",
    "p_twi2b.tpc",
    "p_twi2d1.tpc",
    "p_twi2d2.tpc",
    "p_twi3.tpc",
    "p_twi3b.tpc",
    "p_twi3d1.tpc",
    "p_twi3d2.tpc",
    "p_twi5.tpc",
    "p_twi5b.tpc",
    "p_twi5d1.tpc",
    "p_twi5d2.tpc",
    "p_twi6.tpc",
    "p_twi6b.tpc",
    "p_twi6d1.tpc",
    "p_twi6d2.tpc",
    "p_twi7.tpc",
    "p_twi7b.tpc",
    "p_twi7d1.tpc",
    "p_twi7d2.tpc",
    "p_twiassblue.tpc",
    "p_twiassblueb.tpc",
    "p_twiassblued1.tpc",
    "p_twiassblued2.tpc",
    "p_twiasswhite.tpc",
    "p_twiasswhiteb.tpc",
    "p_twiasswhited1.tpc",
    "p_twiasswhited2.tpc",
    "p_twiblack.tpc",
    "p_twiblackb.tpc",
    "p_twiblackd1.tpc",
    "p_twiblackd2.tpc",
    "p_twifyuthura.tpc",
    "p_twifyuthurab.tpc",
    "p_twifyuthurad1.tpc",
    "p_twifyuthurad2.tpc",
    "p_twihngreen.mdl",
    "p_twihngreen.mdx",
    "p_twihpc2.mdl",
    "p_twihpc2.mdx",
    "p_twihpc3.mdl",
    "p_twihpc3.mdx",
    "p_twihtea.mdl",
    "p_twihtea.mdx",
    "p_twinblue.tpc",
    "p_twinblued1.tpc",
    "p_twinblued2.tpc",
    "p_twingreener.tpc",
    "p_twingreenerd1.tpc",
    "p_twingreenerd2.tpc",
    "p_twired.tpc",
    "p_twiredd1.tpc",
    "p_twiredd2.tpc",
    "pfbltwi.mdl",
    "pfbltwi.mdx",
    "pfbltwibd01.tpc",
    "pfbltwiblue01.tpc",
    "pfbltwiblued01.tpc",
    "pfbltwig01.tpc",
    "pfbltwigd01.tpc",
    "pfbltwigr01.tpc",
    "pfbltwigrd01.tpc",
    "pfbltwir01.tpc",
    "placeables.2da",
    "planetary.2da",
    "soundset.2da",
    "spells.2da",
    "upgrade.2da",
    "visasmarr.dlg",
}

# Compiled by phase 3 from Echoes' sources. Deployed by deploy_galaxy_map(), not
# deploy_override(): the build usually has its own k_sup_galaxymap.ncs, and
# skipping it like other collisions left the game with a travel script that
# never stores Echoes' destinations (12-15, and 0/2/9 reused), which
# check_party_gui.ncs then can't route.
# a_galaxy_map.ncs is Echoes' own prologue (001EBO) script, deployed as shipped. It is
# NOT a copy of a_galaxymap.ncs; deploy_galaxy_map() refuses a staging copy that is.
GALAXY_MAP_SCRIPTS = ["k_sup_galaxymap.ncs", "a_galaxymap.ncs", "a_galaxy_map.ncs"]

# sha256 of build copies that phase 3's scripts may replace, and why that is safe.
# Any other copy stops the deploy: its mod's changes would be lost.
REPLACEABLE_GALAXY_MAP_SCRIPTS = {
    "544ef0c79165e591c28a9c52133f6448c7297d102ad1789d3fb4343d1744b0be":
        "PartySwap 1.5 k_sup_galaxymap.ncs (Handmaiden & Disciple Romances ships the same file). "
        "Its one change to vanilla, the Handmaiden cutscene condition, is in Echoes' source.",
    "8bfb2878426023a246aa5ca0db5fedf07903d39e314de9217f6b82f45a307885":
        "a_galaxymap.ncs from an earlier phase 3 run (pykotor's built-in compiler); that run "
        "also wrote it as a_galaxy_map.ncs.",
    "975f55e0627e43623ebd428f2cc6de5ec3f2e581c152fcaf0a785d4b509125a6":
        "k_sup_galaxymap.ncs from an earlier phase 3 run (pykotor's built-in compiler).",
}

def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def existing_spellings(d):
    """{lower-case name: name} of the real files already in d. Linux keeps "Foo.mod" and
    "foo.mod" as two files and Wine then loads either one, so a deploy must reuse the
    existing spelling or skip (see ../case_dedup.py)."""
    if not d.is_dir():
        return {}
    return {p.name.lower(): p.name for p in d.iterdir() if p.is_file() and not p.is_symlink()}


def deploy_modules():
    print("\n--- Deploying Standalone Modules ---")
    stage_mod_dir = STAGING_DIR / "Modules"
    live_mod_dir = GAME_DIR / "Modules"
    
    have = existing_spellings(live_mod_dir)
    copied = 0
    skipped = 0
    for mod_file in stage_mod_dir.iterdir():
        if not mod_file.is_file():
            continue
        if mod_file.name.lower() in COLLIDING_MODULES:
            skipped += 1
            continue

        # a re-run overwrites its own module under the spelling already there
        target = live_mod_dir / have.get(mod_file.name.lower(), mod_file.name)
        shutil.copy2(mod_file, target)
        copied += 1
        
    print(f"  Copied {copied} new standalone modules to {live_mod_dir.name}/")
    print(f"  Skipped {skipped} colliding modules (preserved Phase 4 surgical merges)")

def deploy_stream_music():
    print("\n--- Deploying StreamMusic ---")
    stage_dir = STAGING_DIR / "StreamMusic"
    live_dir = GAME_DIR / "StreamMusic"
    live_dir.mkdir(parents=True, exist_ok=True)
    
    copied = 0
    for f in stage_dir.iterdir():
        if f.is_file():
            target = live_dir / f.name
            shutil.copy2(f, target)
            copied += 1
            
    print(f"  Copied {copied} music tracks to {live_dir.name}/")

def deploy_movies():
    print("\n--- Deploying Movies ---")
    stage_dir = STAGING_DIR / "Movies"
    live_dir = GAME_DIR / "Movies"
    live_dir.mkdir(parents=True, exist_ok=True)
    
    copied = 0
    for f in stage_dir.iterdir():
        if f.is_file():
            target = live_dir / f.name
            shutil.copy2(f, target)
            copied += 1
            
    print(f"  Copied {copied} cinematic movies to {live_dir.name}/")

def deploy_stream_voice():
    print("\n--- Deploying StreamVoice ---")
    stage_dir = STAGING_DIR / "StreamVoice"
    live_dir = GAME_DIR / "StreamVoice"
    
    copied = 0
    skipped = 0
    for root, dirs, files in os.walk(stage_dir):
        rel_root = os.path.relpath(root, stage_dir)
        target_dir = live_dir / rel_root
        target_dir.mkdir(parents=True, exist_ok=True)
        
        for f in files:
            src_file = Path(root) / f
            dst_file = target_dir / f
            if not dst_file.exists():
                shutil.copy2(src_file, dst_file)
                copied += 1
            else:
                skipped += 1
                
    print(f"  Copied {copied} voice audio files to {live_dir.name}/")
    if skipped > 0:
        print(f"  Skipped {skipped} existing voice files")

def deploy_override():
    print("\n--- Deploying Override Assets ---")
    stage_dir = STAGING_DIR / "Override"
    live_dir = GAME_DIR / "override"
    
    # Pre-build lowercase collision set
    collisions_lower = {f.lower() for f in COLLIDING_OVERRIDE_FILES}
    # Anything already in override/ under any spelling also collides: the build's
    # file wins, as for the list above. Checked on disk so it holds for any build,
    # and so a re-run can't leave "Foo.tga" next to "foo.tga".
    have = existing_spellings(live_dir)

    copied = 0
    skipped = 0
    present = 0
    for f in stage_dir.iterdir():
        if not f.is_file():
            continue
        fname_lower = f.name.lower()
        if fname_lower in GALAXY_MAP_SCRIPTS:
            continue  # deploy_galaxy_map()
        if fname_lower in collisions_lower or fname_lower in ["desktop.ini", "thumbs.db"]:
            skipped += 1
            continue
        if fname_lower in have:
            present += 1
            continue

        target = live_dir / fname_lower  # the copy_loose.py convention
        shutil.copy2(f, target)
        copied += 1

    print(f"  Copied {copied} new unique assets to {live_dir.name}/")
    print(f"  Skipped {skipped} colliding/managed files (protected community build integrity)")
    print(f"  Skipped {present} files already in {live_dir.name}/ under some spelling")

def deploy_galaxy_map():
    print("\n--- Deploying Galaxy Map Scripts ---")
    stage = existing_spellings(STAGING_DIR / "Override")
    live_dir = GAME_DIR / "override"
    have = existing_spellings(live_dir)
    for name in GALAXY_MAP_SCRIPTS:
        if name not in stage:
            sys.exit(f"Missing {name} in {STAGING_DIR / 'Override'}: run phase 3 first.")
    travel = sha256(STAGING_DIR / "Override" / stage["a_galaxymap.ncs"])
    if sha256(STAGING_DIR / "Override" / stage["a_galaxy_map.ncs"]) == travel:
        sys.exit(
            f"{STAGING_DIR / 'Override' / stage['a_galaxy_map.ncs']} is a copy of a_galaxymap.ncs (an old "
            "phase 3 wrote both). Restore Echoes' own Override/a_galaxy_map.ncs from its zip: it is the "
            "prologue's galaxy map script, and the copy makes that map never open."
        )
    for name in GALAXY_MAP_SCRIPTS:
        src = STAGING_DIR / "Override" / stage[name]
        if name not in have:
            shutil.copy2(src, live_dir / name)
            print(f"  Copied {name}")
            continue
        target = live_dir / have[name]
        digest = sha256(target)
        live_travel = sha256(live_dir / have["a_galaxymap.ncs"]) if "a_galaxymap.ncs" in have else None
        if digest == sha256(src):
            print(f"  {have[name]} already up to date")
        elif name == "a_galaxy_map.ncs" and digest in (travel, live_travel):
            # an earlier phase 3 copied the travel script over Echoes' prologue script
            shutil.copy2(src, target)
            print(f"  Replaced {have[name]}: a copy of a_galaxymap.ncs, not the prologue script")
        elif digest in REPLACEABLE_GALAXY_MAP_SCRIPTS:
            shutil.copy2(src, target)
            print(f"  Replaced {have[name]}: {REPLACEABLE_GALAXY_MAP_SCRIPTS[digest]}")
        else:
            sys.exit(
                f"{target} (sha256 {digest}) is not a copy this compatch knows. Replacing it would "
                f"drop its mod's changes. Merge them into Echoes' source, recompile (phase 3), and add "
                f"the sha256 to REPLACEABLE_GALAXY_MAP_SCRIPTS in {Path(__file__).name}."
            )

def main():
    print("=" * 60)
    print("Phase 6: Final Deployment of Assets")
    print("=" * 60)
    
    deploy_modules()
    deploy_stream_music()
    deploy_movies()
    deploy_stream_voice()
    deploy_override()
    deploy_galaxy_map()
    
    print("\n" + "=" * 60)
    print("Phase 6 Deployment Complete!")
    print("=" * 60)

if __name__ == "__main__":
    main()
