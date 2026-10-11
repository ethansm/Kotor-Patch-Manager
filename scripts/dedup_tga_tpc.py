#!/usr/bin/env python3
"""Delete any .tpc in override/ that shares a basename with a .tga or .dds.

Replicates the build guide's "Remove Duplicate TGA/TPC" step (Flachzangen's
.bat / Malacand's .sh), whose instructions say to delete the **TPC** side of
each pair. When a texture exists as both, the engine can load the .tpc
instead of the .tga. That .tpc is usually a stale copy, from vanilla or an
earlier texture pack, of a texture a later mod meant to replace with its .tga
(often paired with a .txi that only matches the new .tga). The guide warns
that letting the .tpc win can crash the game. So the .tpc goes, and the
mod-supplied .tga/.dds stays.

Run it LAST, after every other mod install, because later texture packs can
create new pairs. Use --dry-run first to see what would be deleted. Deletion
is not reversible except by reinstalling the mod that shipped the file.

Usage: dedup_tga_tpc.py [--dry-run]
"""
import argparse
import os

# Override with KOTOR2_ROOT if the game isn't at the default Steam-on-Linux
# path (a different Steam library, GOG, a manual copy, etc).
KOTOR2_ROOT = os.environ.get(
    "KOTOR2_ROOT",
    os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"),
)
OVERRIDE = os.path.join(KOTOR2_ROOT, "override")


def dedup_override(override_dir=OVERRIDE, dry_run=False):
    """Scan override_dir for conflicting .tpc files sharing basenames with .tga/.dds,
    and remove the .tpc files (unless dry_run is True). Returns list of deleted/flagged filenames.
    """
    competing_basenames = {
        os.path.splitext(f.lower())[0]
        for f in os.listdir(override_dir)
        if f.lower().endswith((".tga", ".dds"))
    }

    deleted = []
    for f in os.listdir(override_dir):
        lf = f.lower()
        if not lf.endswith(".tpc"):
            continue
        if os.path.splitext(lf)[0] in competing_basenames:
            if not dry_run:
                os.remove(os.path.join(override_dir, f))
            deleted.append(f)
    return deleted


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--dry-run", action="store_true", help="List what would be deleted without deleting it.")
    parser.add_argument("override_dir", nargs="?", default=OVERRIDE, help=f"Path to override directory (default: {OVERRIDE})")
    args = parser.parse_args()

    deleted = dedup_override(args.override_dir, dry_run=args.dry_run)
    verb = "Would delete" if args.dry_run else "Deleted"
    print(f"{verb} {len(deleted)} .tpc file(s) that had a same-basename .tga/.dds")
    for d in deleted:
        print("  ", d)
    return len(deleted)


if __name__ == "__main__":
    main()
