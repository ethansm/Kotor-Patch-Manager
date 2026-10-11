#!/usr/bin/env python3
"""Create a lowercase symlink next to every mixed/upper-case directory under the
KOTOR2 install (e.g. "modules" -> "Modules", "movies" -> "Movies").

The game doesn't need these. It runs under Wine, which already resolves paths
case-insensitively. The symlinks are for the Linux-side tooling: shell
commands, these scripts, and pykotor code paths that build a plain Path
instead of a CaseAwarePath. All of those resolve "modules/602dan.mod"
literally, so without the symlink the lookup fails with FileNotFoundError.

Idempotent. Re-run it whenever a mod creates a new directory under the game
root.
"""
import os

# Override with KOTOR2_ROOT if the game isn't at the default Steam-on-Linux
# path (a different Steam library, GOG, a manual copy, etc).
KOTOR2_ROOT = os.environ.get(
    "KOTOR2_ROOT",
    os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"),
)


SKIP_TREES = {"reshade-shaders", "cloudsaves"}


def create_case_symlinks(root_dir=KOTOR2_ROOT):
    """Create lowercase symlinks for any mixed/uppercase directories in root_dir.
    Returns the count of new symlinks created.
    """
    created = 0
    # Prune subtrees that must not have case-alias symlinks:
    # - reshade-shaders: ReShade's recursive ** effect search follows symlinks, causing double compilation
    # - cloudsaves: game and Steam Cloud enumerate subdirectories directly
    skip_lower = {s.lower() for s in SKIP_TREES}

    # followlinks=False so the walk doesn't descend into the symlinks this
    # creates (or ones from a previous run) and visit every directory twice.
    for root, dirs, _files in os.walk(root_dir, followlinks=False):
        if os.path.abspath(root) == os.path.abspath(root_dir):
            dirs[:] = [d for d in dirs if d.lower() not in skip_lower]
        for d in dirs:
            low_d = d.lower()
            if low_d == d:
                continue
            low_path = os.path.join(root, low_d)
            # lexists, not exists: a dangling symlink already at this name
            # would make os.symlink raise.
            if not os.path.lexists(low_path):
                os.symlink(d, low_path)
                created += 1
                print("symlinked:", low_path, "->", d)
    return created


def main():
    created = create_case_symlinks(KOTOR2_ROOT)
    print(f"Created {created} new case-insensitive symlinks")


if __name__ == "__main__":
    main()
