#!/usr/bin/env python3
"""Game-folder install of the party AI script set (17 override .ncs). Default = dry run (prints what it would do).

  apply_party_ai_scripts.py [--apply] [--tag HHMM]

Source: ~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/out_party_ai/ (built by
patch_manager_mods/ranged_ai_src/script/build_party_ai_scripts.py; manifest.json lists name, md5, size).

--apply (game CLOSED, user go):
  1. refuses if swkotor2.exe is running, if the manifest or any built file is missing or its md5 differs from the manifest,
     or if any target in $G/override/ already exists and is not ours. Allowed existing: k_ai_master.ncs with md5 94db8304
     (the Mod 8 0.2.0 build) or the new build; the 16 routers only if byte-identical to the new build (a re-run).
  2. backs up every existing target to ~/Games/KOTOR2_Mod_Workspace/backups_party_ai_<tag>/ (refuses if that dir exists, so a
     re-run never overwrites an earlier backup) and writes BEFORE.txt there: the pre-install md5 of every target, or "-" if absent
  3. copies the 17 files to $G/override/ (the real dir is lowercase `override`; `Override` is a symlink to it)
  4. re-md5s every installed file against the manifest (sibling sessions overwrite installs)
  5. prints the lines for ~/Games/KOTOR2_Mod_Workspace/replay/README.md and the rollback command, built from BEFORE.txt:
     restore only what existed before (k_ai_master.ncs), delete only what was new. If k_ai_master.ncs was already our build
     (a re-run), the restore points at the earlier backup dir whose BEFORE.txt holds the Mod 8 build (94db8304).
  A failed copy restores the pre-install state by itself (new files removed, backups copied back).
Partial rollbacks are safe: restoring only k_ai_master keeps the routing (the routers always run whatever k_ai_master is
live); deleting only k_ai_master gives vanilla behaviour (the routers then run the BIF k_ai_master).
"""
import argparse, datetime, glob, hashlib, json, os, shutil, subprocess, sys

G = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
W = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace")
OUT = os.path.join(W, "mod8_kite_script", "out_party_ai")
SRC_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ranged_ai_src", "script")
SRC_INPUTS = ("party_ai_patches.py", "kite_routine.nss", "party_ai_fixes.nss")   # recorded in manifest.json["inputs"]
MASTER = "k_ai_master.ncs"
MOD8_MD5 = "94db830439f1d33572ba911174e20a04"   # live k_ai_master.ncs (Mod 8 0.2.0 build)


def md5(p):
    return hashlib.md5(open(p, "rb").read()).hexdigest()


def find_all_ci(d, name):
    if not os.path.isdir(d):
        return []
    return [os.path.join(d, f) for f in os.listdir(d) if f.lower() == name.lower()]


def read_before(path):
    """BEFORE.txt -> {name: md5 or None}"""
    out = {}
    for l in open(path).read().splitlines():
        parts = l.split()
        if len(parts) >= 2:
            out[parts[1]] = None if parts[0] == "-" else parts[0]
    return out


def mod8_backup():
    """The backup dir (newest first) whose BEFORE.txt shows k_ai_master.ncs as the Mod 8 build, or None."""
    for d in sorted(glob.glob(os.path.join(W, "backups_party_ai_*")), reverse=True):
        b = os.path.join(d, "BEFORE.txt")
        if os.path.exists(b) and read_before(b).get(MASTER) == MOD8_MD5 and os.path.exists(os.path.join(d, MASTER)):
            return d
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--tag", default=datetime.datetime.now().strftime("%H%M"))
    a = ap.parse_args()

    running = subprocess.run(["pgrep", "-f", r"^S:.*swkotor2\.exe"], capture_output=True).stdout.strip()
    if running:
        if a.apply:
            sys.exit("swkotor2.exe is running: close the game first")
        print("NOTE: swkotor2.exe is running; --apply would refuse")
    ov = os.path.join(G, "override")
    if not os.path.isdir(ov):
        sys.exit("no override dir")
    mp = os.path.join(OUT, "manifest.json")
    if not os.path.exists(mp):
        sys.exit("missing " + mp + " (run patch_manager_mods/ranged_ai_src/script/build_party_ai_scripts.py)")
    man = json.load(open(mp))
    files = man["files"]
    if len(files) != 17:
        sys.exit(f"manifest lists {len(files)} files, expected 17")
    if not str(man.get("arg_order", "")).startswith("0 script(s) compiled with swapped"):
        sys.exit("manifest arg_order is not the clean result: " + str(man.get("arg_order")))
    stale = [n for n, m in man["inputs"].items() if n in SRC_INPUTS and md5(os.path.join(SRC_DIR, n)) != m]
    if stale:
        sys.exit("the repo sources changed since the build (" + ", ".join(stale) + "): rebuild with build_party_ai_scripts.py")
    bak = os.path.join(W, f"backups_party_ai_{a.tag}")
    if os.path.exists(bak):
        sys.exit(f"{bak} already exists: use another --tag (a backup is never overwritten)")

    # 1. the build must be intact
    for e in files:
        src = os.path.join(OUT, e["name"])
        if not os.path.exists(src):
            sys.exit("missing built file " + src)
        if md5(src) != e["md5"] or os.path.getsize(src) != e["size"]:
            sys.exit(f"{src} does not match manifest.json (md5 {md5(src)} != {e['md5']}): rebuild")
    new_master = next(e["md5"] for e in files if e["name"] == MASTER)

    # 2. nothing foreign in the way
    plan, foreign, before = [], [], {}
    for e in files:
        hits = find_all_ci(ov, e["name"])
        if len(hits) > 1:
            foreign.append((", ".join(hits), "several case variants"))
            continue
        dst = hits[0] if hits else None
        if dst is None:
            plan.append((e, None, "new"))
            before[e["name"]] = None
            continue
        m = md5(dst)
        before[e["name"]] = m
        if e["name"] == MASTER and m in (MOD8_MD5, new_master):
            plan.append((e, dst, "replace Mod 8 build" if m == MOD8_MD5 else "already ours"))
        elif m == e["md5"]:
            plan.append((e, dst, "already ours"))
        else:
            foreign.append((dst, m))
    print(f"mode: {'APPLY' if a.apply else 'dry run'}; source {OUT}; override {ov}; backups {bak}")
    print(f"manifest built {man.get('built')}; {len(files)} files; arg-order: {man.get('arg_order')}")
    for e, dst, what in plan:
        print(f"  {e['name']:24s} {e['size']:7d}  {e['md5'][:8]}  -> {what}")
    if foreign:
        for dst, m in foreign:
            print(f"REFUSE: {dst} exists, md5 {m}, and is not ours (another mod owns it; merge by hand)")
        sys.exit(1)
    if not a.apply:
        print("\n(dry run) nothing written. --apply --tag HHMM would back up the existing targets to the backup dir, "
              "write BEFORE.txt, copy the files above to", ov, "and re-check every md5.")
        print_tail(files, bak, a.tag, before, dry=True)
        return

    # 3. back up, record the pre-install state, copy, verify (restore everything if a copy fails)
    os.makedirs(bak)
    rec = []
    for e, dst, what in plan:
        if dst:
            shutil.copy2(dst, os.path.join(bak, e["name"]))
            rec.append(f"{before[e['name']]}  {e['name']}  (backed up)")
        else:
            rec.append(f"-  {e['name']}  (absent before the install)")
    open(os.path.join(bak, "BEFORE.txt"), "w").write("\n".join(rec) + "\n")
    try:
        for e, dst, what in plan:
            src = os.path.join(OUT, e["name"])
            target = dst or os.path.join(ov, e["name"])
            shutil.copy2(src, target)
            if md5(target) != e["md5"]:
                raise RuntimeError(f"copy of {e['name']} failed the md5 re-check")
            print("copied", target, e["md5"][:8])
        bad = [e["name"] for e in files if md5(find_all_ci(ov, e["name"])[0]) != e["md5"]]
        if bad:
            raise RuntimeError("md5 re-check failed after the copy: " + ", ".join(bad))
    except Exception as ex:
        print("INSTALL FAILED:", ex, "- restoring the pre-install state")
        for e, dst, what in plan:
            target = dst or os.path.join(ov, e["name"])
            if before[e["name"]] is None:
                if os.path.exists(target):
                    os.remove(target)
            else:
                shutil.copy2(os.path.join(bak, e["name"]), target)
        sys.exit(1)
    print(f"all {len(files)} installed files re-checked against manifest.json")
    print_tail(files, bak, a.tag, before, dry=False)


def print_tail(files, bak, tag, before, dry):
    d = datetime.date.today()
    master = next(e for e in files if e["name"] == MASTER)
    routers = [e["name"] for e in files if e["name"] != MASTER]
    was = before.get(MASTER)
    print("\nAdd to ~/Games/KOTOR2_Mod_Workspace/replay/README.md" + (" (after a real install):" if dry else ":"))
    print(f"- override/k_ai_master.ncs (party AI script fixes + Mod 8 stance 16 v2, {d} {tag}): md5 {master['md5'][:8]} "
          f"(was {(was or 'absent')[:8]}), from `patch_manager_mods/mods678_research/apply_party_ai_scripts.py`. "
          f"Rollback: see backups_party_ai_{tag}/BEFORE.txt.")
    print(f"- override/ {len(routers)} routers (E2: the 13 k_hen_<name>dlg route shouts to k_ai_master event 2004; a_atkonend, "
          f"a_force_combat, k_combat_rnd to event 3001), {d} {tag}: " +
          ", ".join(f"{e['name']} {e['md5'][:8]}" for e in files if e["name"] != MASTER) +
          ". Rollback: delete them.")
    print("\nRollback" + (" (as it would read)" if dry else "") + f", built from the pre-install state (BEFORE.txt):")
    ovd = os.path.join(G, "override")
    new_files = [n for n, m in before.items() if m is None]
    if new_files:
        print(f'  cd "{ovd}" && rm -f ' + " ".join(new_files))
    ours = [n for n, m in before.items() if m is not None and n != MASTER]
    if ours:
        print("  (already ours before this run, left alone: " + ", ".join(ours) + ")")
    if was is None:
        print(f'  # {MASTER} was absent: nothing to restore')
    else:
        src_dir = bak if was == MOD8_MD5 else mod8_backup()
        if was == MOD8_MD5 or src_dir:
            print(f'  cp "{os.path.join(src_dir, MASTER)}" "{os.path.join(ovd, MASTER)}" && md5sum "{os.path.join(ovd, MASTER)}"   '
                  f"# expect {MOD8_MD5}")
        else:
            print(f"  # {MASTER} was already the party build ({was[:8]}) and no backup dir holds the Mod 8 build: "
                  f"rebuild it with build_kite_script.py and copy mod8_kite_script/out/{MASTER}")
    print("  (partial: restoring only k_ai_master keeps the routing; deleting only k_ai_master = vanilla behaviour)")


if __name__ == "__main__":
    main()
