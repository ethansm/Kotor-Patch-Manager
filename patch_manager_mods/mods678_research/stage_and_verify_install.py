#!/usr/bin/env python3
"""Stage + verify a Kotor Patch Manager install (procedure from doc 10 'Standing rules', automated for the read-only parts).

  stage_and_verify_install.py stage  STAGE_DIR [EXTRA.kpatch ...]   # never writes the game dir
      For every id in the live patch_config.toml, find the kpatch (under the kotor-patch-manager tree) whose DLL md5 equals the
      LIVE DLL (simple patches: newest kpatch), check its Steam Aspyr hook set is byte-identical to the live config, copy it into STAGE_DIR,
      add the EXTRA kpatches (an extra whose id is live replaces that live build), and print the id list to pass to KPatchLauncher.
  stage_and_verify_install.py verify STAGE_DIR                        # after the install
      md5 of every live DLL vs the DLL in the staged kpatch of that id; hook counts; consumed_exit_address hooks.

Install itself (game CLOSED, user-gated; `pgrep -f '^S:.*swkotor2\\.exe'` empty; back up kpm_install_state.json + patch_config.toml + patches/ first):
  KPATCH_NO_LAUNCH=1 ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches STAGE_DIR <ids>
Building a kpatch: cd Patches/<Name> && KPATCH_ADDRESS_DATABASES=<dir of symlinks to kotor2_*.db> python3 ../create-patch.py
(needed while AddressDatabases/kotor1_steam.db is a 0-byte file).
"""
import sys, os, glob, re, zipfile, hashlib, shutil, tomllib, collections

G = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
T = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager")
STEAM = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"


def kpatch_index():
    idx = collections.defaultdict(list)
    for f in glob.glob(T + "/**/*.kpatch", recursive=True):
        try:
            z = zipfile.ZipFile(f)
            m = z.read("manifest.toml").decode()
        except Exception:
            continue
        mm = re.search(r'^id\s*=\s*"([^"]+)"', m, re.M)
        if not mm:
            continue
        dll = [n for n in z.namelist() if n.startswith("binaries/") and n.endswith(".dll")]
        md5 = hashlib.md5(z.read(dll[0])).hexdigest() if dll else None
        idx[mm.group(1)].append((os.path.getmtime(f), f, md5))
    return idx


def norm(h):
    return (h["address"], h.get("function", ""), tuple(h["original_bytes"]), h.get("skip_original_bytes", False),
            h.get("consumed_exit_address", 0), tuple(p["source"] for p in h.get("parameters", [])))


def kpatch_hooks(z):
    out = []
    for n in z.namelist():
        if n.endswith(".toml") and n != "manifest.toml":
            t = tomllib.loads(z.read(n).decode())
            tv = [x.upper() for x in t.get("metadata", {}).get("target_versions", [])]
            if STEAM in tv or not tv:
                out += [norm(h) for h in t.get("hooks", [])]
    return sorted(out)


def stage(stage_dir, extras):
    live = tomllib.load(open(G + "/patch_config.toml", "rb"))
    idx = kpatch_index()
    os.makedirs(stage_dir, exist_ok=True)
    ids, bad = [], 0
    def kid(e):
        return re.search(r'^id\s*=\s*"([^"]+)"', zipfile.ZipFile(e).read("manifest.toml").decode(), re.M).group(1)
    extra_ids = {kid(e) for e in extras}
    for p in live["patches"]:
        i = p["id"]
        if i in extra_ids:   # upgraded by an EXTRA kpatch: the live build is replaced, nothing to match
            print(f"{i:32s} live -> replaced by extra")
            continue
        lp = G + "/" + p["dll"] if p.get("dll") else None
        lm = hashlib.md5(open(lp, "rb").read()).hexdigest() if lp and os.path.exists(lp) else None
        cs = sorted(idx.get(i, []), reverse=True)
        match = [c for c in cs if lm and c[2] == lm]
        pick = (match or cs[:1])
        if not pick:
            print("NO KPATCH for", i); bad += 1; continue
        z = zipfile.ZipFile(pick[0][1])
        same = kpatch_hooks(z) == sorted(norm(h) for h in p.get("hooks", []))
        print(f"{i:32s} dll={'match' if match else ('none' if not lm else 'NO-MATCH')} hooks={'SAME' if same else 'DIFF'} <- {pick[0][1].replace(T, '')}")
        bad += (not same) or (lm is not None and not match)
        shutil.copy2(pick[0][1], os.path.join(stage_dir, os.path.basename(pick[0][1]).replace(" ", "_")))
        ids.append(i)
    for e in extras:
        i = kid(e)
        shutil.copy2(e, stage_dir)
        if i not in ids:
            ids.append(i)
        print("extra:", i, e)
    print("PROBLEMS:", bad)
    print("IDS:", " ".join(ids))


def verify(stage_dir):
    live = tomllib.load(open(G + "/patch_config.toml", "rb"))
    kp = {}
    for f in glob.glob(stage_dir + "/*.kpatch"):
        z = zipfile.ZipFile(f)
        kp[re.search(r'^id\s*=\s*"([^"]+)"', z.read("manifest.toml").decode(), re.M).group(1)] = z
    bad = 0
    print("target", live["target_version_sha"][:8], "patches", len(live["patches"]), "hooks", sum(len(p.get("hooks", [])) for p in live["patches"]))
    for p in live["patches"]:
        i = p["id"]; z = kp.get(i)
        if z is None:
            print(i, "NOT STAGED"); bad += 1; continue
        d = [n for n in z.namelist() if n.startswith("binaries/") and n.endswith(".dll")]
        if d and p.get("dll"):
            ok = hashlib.md5(open(G + "/" + p["dll"], "rb").read()).hexdigest() == hashlib.md5(z.read(d[0])).hexdigest()
            bad += not ok
            print(f"{i:32s} dll md5 {'OK' if ok else 'MISMATCH'}")
        for h in p.get("hooks", []):
            if h.get("consumed_exit_address"):
                print(f"   consumed_exit hook {h['address']:#x} -> {h['consumed_exit_address']:#x} exclude={h.get('exclude_from_restore')}")
    print("mismatches", bad)


if __name__ == "__main__":
    if len(sys.argv) >= 3 and sys.argv[1] == "stage":
        stage(sys.argv[2], sys.argv[3:])
    elif len(sys.argv) == 3 and sys.argv[1] == "verify":
        verify(sys.argv[2])
    else:
        print(__doc__)
