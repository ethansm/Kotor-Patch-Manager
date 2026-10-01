#!/usr/bin/env python3
"""Game-folder install of the party AI script set (17 override .ncs). Default = dry run (prints what it would do).

  apply_party_ai_scripts.py [--apply] [--tag HHMM]

Source: ~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/out_party_ai/ (built by
patch_manager_mods/ranged_ai_src/script/build_party_ai_scripts.py; manifest.json lists name, md5, size).

--apply (game CLOSED, user go):
  1. refuses if swkotor2.exe is running, if the manifest or any built file is missing or its md5 differs from the manifest,
     or if any target in $G/override/ already exists and is not ours. Allowed existing: k_ai_master.ncs with md5 94db8304
     (the Mod 8 0.2.0 build) or the new build; the 16 routers only if byte-identical to the new build (a re-run).
  2. backs up every existing target (and records the absent ones) to ~/Games/KOTOR2_Mod_Workspace/backups_party_ai_<tag>/
  3. copies the 17 files to $G/override/ (the real dir is lowercase `override`; `Override` is a symlink to it)
  4. re-md5s every installed file against the manifest (sibling sessions overwrite installs)
  5. prints the lines for ~/Games/KOTOR2_Mod_Workspace/replay/README.md and the rollback command.
Rollback: delete the 16 new files and restore k_ai_master.ncs (94db8304) from the backup; printed at the end.
Partial rollbacks are safe: restoring only k_ai_master keeps the routing (the routers always run whatever k_ai_master is
live); deleting only k_ai_master gives vanilla behaviour (the routers then run the BIF k_ai_master).
"""
import argparse, datetime, hashlib, json, os, shutil, subprocess, sys

G = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
W = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace")
OUT = os.path.join(W, "mod8_kite_script", "out_party_ai")
MASTER = "k_ai_master.ncs"
MOD8_MD5 = "94db830439f1d33572ba911174e20a04"   # live k_ai_master.ncs (Mod 8 0.2.0 build)


def md5(p):
    return hashlib.md5(open(p, "rb").read()).hexdigest()


def find_ci(d, name):
    if not os.path.isdir(d):
        return None
    for f in os.listdir(d):
        if f.lower() == name.lower():
            return os.path.join(d, f)
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
    bak = os.path.join(W, f"backups_party_ai_{a.tag}")

    # 1. the build must be intact
    for e in files:
        src = os.path.join(OUT, e["name"])
        if not os.path.exists(src):
            sys.exit("missing built file " + src)
        if md5(src) != e["md5"] or os.path.getsize(src) != e["size"]:
            sys.exit(f"{src} does not match manifest.json (md5 {md5(src)} != {e['md5']}): rebuild")
    new_master = next(e["md5"] for e in files if e["name"] == MASTER)

    # 2. nothing foreign in the way
    plan, foreign = [], []
    for e in files:
        dst = find_ci(ov, e["name"])
        if dst is None:
            plan.append((e, None, "new"))
            continue
        m = md5(dst)
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
              "copy the files above to", ov, "and re-check every md5.")
        print_tail(files, bak, a.tag, dry=True)
        return

    # 3. back up, copy, verify
    os.makedirs(bak, exist_ok=True)
    rec = []
    for e, dst, what in plan:
        if dst:
            shutil.copy2(dst, os.path.join(bak, e["name"]))
            rec.append(f"{md5(dst)}  {e['name']}  (backed up)")
        else:
            rec.append(f"-  {e['name']}  (absent before the install)")
    open(os.path.join(bak, "BEFORE.txt"), "w").write("\n".join(rec) + "\n")
    for e, dst, what in plan:
        src = os.path.join(OUT, e["name"])
        target = dst or os.path.join(ov, e["name"])
        shutil.copy2(src, target)
        if md5(target) != e["md5"]:
            sys.exit(f"copy of {e['name']} failed the md5 re-check")
        print("copied", target, e["md5"][:8])
    bad = [e["name"] for e in files if md5(find_ci(ov, e["name"])) != e["md5"]]
    if bad:
        sys.exit("md5 re-check failed after the copy: " + ", ".join(bad))
    print(f"all {len(files)} installed files re-checked against manifest.json")
    print_tail(files, bak, a.tag, dry=False)


def print_tail(files, bak, tag, dry):
    d = datetime.date.today()
    names = [e["name"] for e in files]
    master = next(e for e in files if e["name"] == MASTER)
    routers = [n for n in names if n != MASTER]
    print("\nAdd to ~/Games/KOTOR2_Mod_Workspace/replay/README.md" + (" (after a real install):" if dry else ":"))
    print(f"- override/k_ai_master.ncs (party AI script fixes + Mod 8 stance 16 v2, {d} {tag}): md5 {master['md5'][:8]} "
          f"(was Mod 8 0.2.0 {MOD8_MD5[:8]}), from `patch_manager_mods/mods678_research/apply_party_ai_scripts.py`. "
          f"Rollback: restore {MOD8_MD5[:8]} from backups_party_ai_{tag}/.")
    print(f"- override/ {len(routers)} routers (E2: the 13 k_hen_<name>dlg route shouts to k_ai_master event 2004; a_atkonend, "
          f"a_force_combat, k_combat_rnd to event 3001), {d} {tag}: " +
          ", ".join(f"{e['name']} {e['md5'][:8]}" for e in files if e["name"] != MASTER) +
          ". Rollback: delete them.")
    print("\nRollback" + (" (as it would read)" if dry else "") + ":")
    print(f'  cd "{os.path.join(G, "override")}" && rm -f ' + " ".join(routers))
    print(f'  cp "{os.path.join(bak, MASTER)}" "{os.path.join(G, "override", MASTER)}" && md5sum {MASTER}   '
          f"# expect {MOD8_MD5}")
    print("  (partial: restoring only k_ai_master keeps the routing; deleting only k_ai_master = vanilla behaviour)")


if __name__ == "__main__":
    main()
