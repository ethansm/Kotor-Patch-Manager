#!/usr/bin/env python3
"""Find and collapse case-variant duplicates in override/ ("TEL_SFtr.tpc" next to
"tel_sftr.tpc").

Wine resolves filenames case-insensitively, so when two spellings of one name
sit in override/, which one the game loads depends on directory order: in
effect, arbitrary. Linux keeps both, so this happens whenever one install
writes a name in a different case from an earlier one. pykotor keeps the case
used in a mod's ini, and mod archives use whatever case their author had. A
build of this size ended up with 79 such pairs. In 74 of them the two files
had different content, so the game could load the older texture, model or
dialog instead of the one the later mod installed.

The rule is the build guide's install precedence: the file the most recent
install wrote wins. It's kept under the lower-case name (the copy_loose.py
convention) and every other spelling is deleted.
  - copy_loose.py and pk_install.py call collapse() after every install, with the
    files that install just wrote as the winners, so new pairs can't form.
  - This CLI audits an existing folder. Pairs with identical content are collapsed
    automatically. Pairs that differ need a decision: pass --keep <name> (or
    --keep-file) naming the spelling to keep, usually the one from the mod later
    in the build order. Without one, the pair is reported and the exit code is 1.

Symlinks are ignored. They're the case aliases symlink_pass.py makes on purpose.
The CLI also deletes OS junk that mod archives carry (.DS_Store, Thumbs.db,
desktop.ini, macOS "._" resource forks); none of it is a game resource.

Usage: case_dedup.py [override_dir] [--dry-run] [--keep NAME ...] [--keep-file FILE]
"""
import argparse
import filecmp
import os
import sys
from collections import defaultdict

KOTOR2_ROOT = os.environ.get(
    "KOTOR2_ROOT",
    os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"),
)
OVERRIDE = os.path.join(KOTOR2_ROOT, "override")

JUNK_NAMES = {".ds_store", "thumbs.db", "desktop.ini"}


def is_junk(name):
    return name.lower() in JUNK_NAMES or name.startswith("._")


def case_groups(d):
    """{lower-case name: [spellings]} for every name with more than one real file in d."""
    groups = defaultdict(list)
    for f in os.listdir(d):
        p = os.path.join(d, f)
        if os.path.isfile(p) and not os.path.islink(p):
            groups[f.lower()].append(f)
    return {k: sorted(v) for k, v in groups.items() if len(v) > 1}


def collapse(d, winners, dry_run=False):
    """For each name in winners: delete every other-case spelling of it in d, then
    rename it to lower case. Returns [(winner, [deleted])] for names that had
    another spelling or weren't lower case; the winner now has the lower-case name."""
    done = []
    for w in winners:
        low = w.lower()
        others = [f for f in os.listdir(d) if f.lower() == low and f != w
                  and not os.path.islink(os.path.join(d, f))]
        if not others and w == low:
            continue
        if not dry_run:
            for f in others:
                os.remove(os.path.join(d, f))
            if w != low:
                os.rename(os.path.join(d, w), os.path.join(d, low))
        done.append((w, others))
    return done


def audit(d, keep=(), dry_run=False):
    """Delete OS junk, then collapse identical-content groups and the groups named in
    keep. Returns (junk [names], resolved [(winner, [deleted])], unresolved {lower: [spellings]})."""
    junk = sorted(f for f in os.listdir(d) if is_junk(f) and os.path.isfile(os.path.join(d, f)))
    if not dry_run:
        for f in junk:
            os.remove(os.path.join(d, f))
    keep_by_low = {k.lower(): k for k in keep}
    winners, unresolved = [], {}
    for low, names in sorted(case_groups(d).items()):
        if dry_run and is_junk(low):
            continue
        if low in keep_by_low:
            if keep_by_low[low] not in names:
                raise SystemExit(f"--keep {keep_by_low[low]}: not one of {names}")
            winners.append(keep_by_low[low])
        elif all(filecmp.cmp(os.path.join(d, names[0]), os.path.join(d, n), shallow=False) for n in names[1:]):
            winners.append(low if low in names else names[0])
        else:
            unresolved[low] = names
    return junk, collapse(d, winners, dry_run), unresolved


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("override_dir", nargs="?", default=OVERRIDE)
    ap.add_argument("--dry-run", action="store_true", help="report what would change without changing it")
    ap.add_argument("--keep", action="append", default=[], help="the spelling to keep for a differing pair")
    ap.add_argument("--keep-file", help="file with one --keep name per line (# comments allowed)")
    a = ap.parse_args()
    keep = list(a.keep)
    if a.keep_file:
        with open(a.keep_file) as f:
            keep += [ln.split("#")[0].strip() for ln in f if ln.split("#")[0].strip()]

    junk, resolved, unresolved = audit(a.override_dir, keep, a.dry_run)
    verb = "Would delete" if a.dry_run else "Deleted"
    print(f"{verb} {len(junk)} OS junk file(s): {junk}")
    verb = "Would collapse" if a.dry_run else "Collapsed"
    print(f"{verb} {len(resolved)} case-variant group(s) in {a.override_dir}")
    for winner, deleted in resolved:
        print(f"   keep {winner} as {winner.lower()}, delete {deleted}")
    if unresolved:
        print(f"\n{len(unresolved)} group(s) with differing content need --keep (the later mod's spelling):")
        for low, names in unresolved.items():
            sizes = [os.path.getsize(os.path.join(a.override_dir, n)) for n in names]
            print(f"   {low}: " + ", ".join(f"{n} ({s} bytes)" for n, s in zip(names, sizes)))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
