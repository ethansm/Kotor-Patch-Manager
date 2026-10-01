#!/usr/bin/env python3
"""Game-folder data for the Mods 6+8 0.2.0 combined install. Default = dry run (prints what it would do).

  apply_mods68_data.py [--apply] [--tag HHMM]

--apply (game CLOSED, user go; run AFTER the KPatchLauncher install):
  1. scripts/append_ranged_kite_tlk.py --apply  (backs up dialog.tlk, idempotent) -> ranged_ai_strrefs.json
  2. scripts/build_ranged_kite_2da.py --strrefs-json ... --out STAGE  (refuses over a foreign Override aiscripts.2da)
  3. copies STAGE/Override/aiscripts.2da and mod8_kite_script/out/k_ai_master.ncs to $G/Override/
     (refuses if a k_ai_master.ncs not made by us is already there; backs up any existing target first)
  4. writes $G/inventory_sort_filter.ini and $G/ranged_ai.ini from the repo sources (backs up the old ones),
     with kite_strref_msg set from the JSON
  5. prints the lines to add to ~/Games/KOTOR2_Mod_Workspace/replay/README.md
Rollback: printed at the end.
"""
import argparse, datetime, hashlib, json, os, re, shutil, subprocess, sys

G = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
W = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace")
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
PY = os.path.join(REPO, ".venv", "bin", "python")
NCS = os.path.join(W, "mod8_kite_script", "out", "k_ai_master.ncs")
JSON = os.path.join(W, "ranged_ai_strrefs.json")
INI6 = os.path.join(REPO, "patch_manager_mods", "inventory_sort_filter_src", "inventory_sort_filter.ini")
INI8 = os.path.join(REPO, "patch_manager_mods", "ranged_ai_src", "ranged_ai.ini")


def md5(p):
    return hashlib.md5(open(p, "rb").read()).hexdigest()


def find_ci(d, name):
    if not os.path.isdir(d):
        return None
    for f in os.listdir(d):
        if f.lower() == name:
            return os.path.join(d, f)
    return None


def run(cmd):
    print("$", " ".join(cmd))
    r = subprocess.run(cmd)
    if r.returncode != 0:
        sys.exit(f"step failed ({r.returncode})")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--tag", default=datetime.datetime.now().strftime("%H%M"))
    a = ap.parse_args()
    if subprocess.run(["pgrep", "-f", r"^S:.*swkotor2\.exe"], capture_output=True).stdout.strip():
        sys.exit("swkotor2.exe is running: close the game first")
    ov = os.path.join(G, "override")
    if not os.path.isdir(ov):
        sys.exit("no override dir")
    stage = os.path.join(W, f"stage_mods68_data_{a.tag}")
    bak = os.path.join(W, f"backups_mods68_data_{a.tag}")
    if not os.path.exists(NCS):
        sys.exit("missing " + NCS + " (run ranged_ai_src/script/build_kite_script.py)")
    existing_ncs = find_ci(ov, "k_ai_master.ncs")
    if existing_ncs and md5(existing_ncs) != md5(NCS):
        sys.exit(f"{existing_ncs} exists and is not our build: another mod owns k_ai_master; merge by hand")
    print(f"mode: {'APPLY' if a.apply else 'dry run'}; stage {stage}; backups {bak}")
    print(f"k_ai_master.ncs: {md5(NCS)} ({os.path.getsize(NCS)} bytes); Override copy: {existing_ncs or 'none'}")

    # 1. TLK
    run([PY, os.path.join(REPO, "scripts", "append_ranged_kite_tlk.py")] + (["--apply"] if a.apply else []))
    if not a.apply:
        print("\n(dry run) would build aiscripts.2da from the JSON, copy it + k_ai_master.ncs to", ov,
              "and write both INIs to", G)
        return
    s = json.load(open(JSON))
    # 2. 2DA
    run([PY, os.path.join(REPO, "scripts", "build_ranged_kite_2da.py"), "--strrefs-json", JSON, "--out", stage])
    os.makedirs(bak, exist_ok=True)
    lines = []
    # 3. Override files
    for src, name in ((os.path.join(stage, "Override", "aiscripts.2da"), "aiscripts.2da"), (NCS, "k_ai_master.ncs")):
        dst = find_ci(ov, name) or os.path.join(ov, name)
        if os.path.exists(dst):
            shutil.copy2(dst, os.path.join(bak, name))
        shutil.copy2(src, dst)
        assert md5(dst) == md5(src)
        lines.append(f"- override/{name} (Mod 8 ranged-ai stance 16, {datetime.date.today()} {a.tag}): md5 {md5(dst)[:8]}, "
                     f"from `patch_manager_mods/mods678_research/apply_mods68_data.py`. Rollback: delete it.")
        print("copied", dst, md5(dst)[:8])
    # 4. INIs
    for src in (INI6, INI8):
        dst = os.path.join(G, os.path.basename(src))
        if os.path.exists(dst):
            shutil.copy2(dst, os.path.join(bak, os.path.basename(src)))
        txt = open(src).read()
        if src == INI8:
            txt, n = re.subn(r"(?m)^kite_strref_msg=\d+", f"kite_strref_msg={s['message']}", txt)
            assert n == 1
        open(dst, "w", newline="\r\n").write(txt)
        print("wrote", dst)
    print("\nAdd to replay/README.md:\n" + "\n".join(lines))
    print("(plus the dialog.tlk line printed by append_ranged_kite_tlk.py above)")
    print(f"\nRollback: delete override/aiscripts.2da + override/k_ai_master.ncs; cp {s.get('backup')} \"$G/dialog.tlk\"; "
          f"old INIs in {bak}; remove the patch ids / reinstall the previous set.")


if __name__ == "__main__":
    main()
