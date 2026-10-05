#!/usr/bin/env python3
"""Stage + verify a Kotor Patch Manager install (procedure from doc 10 'Standing rules', automated for the read-only parts).

  stage_and_verify_install.py stage  STAGE_DIR [EXTRA.kpatch ...] [--allow-dirty]   # never writes the game dir
      For every id in the live patch_config.toml, find the kpatch (under the kotor-patch-manager tree) whose DLL md5 equals the
      LIVE DLL (simple patches: newest kpatch), check its Steam Aspyr hook set is byte-identical to the live config, copy it into STAGE_DIR,
      add the EXTRA kpatches (an extra whose id is live replaces that live build), and print the id list to pass to KPatchLauncher.
      Commit-before-install record: each staged id's source dir (Patches/<Dir> of the picked kpatch, else the manifest.toml id map;
      "?" = ambiguous/unknown -> warning only; extras outside T = "external") is checked with `git -C T status --porcelain -- <dir>`;
      `WARN DIRTY <id> <paths>` per dirty dir. Writes STAGE_DIR/STAGE_INFO.txt (branch=, head=, upstream=pushed|unpushed|no-upstream,
      date=, ids=, then `<id>\t<srcdir>\t<clean|DIRTY: paths|unchecked>` per id) and prints `STAGE_INFO: <path> head=<sha12>`.
      Exit code 2 if any DIRTY (after printing/writing everything) unless --allow-dirty (accepted anywhere after `stage`).
      RULE: commit the mod's source on ethansm/local-mods (explicit paths) BEFORE staging; note the STAGE_INFO head in the mod's doc.
  stage_and_verify_install.py verify STAGE_DIR                        # after the install
      prints the STAGE_INFO branch/head/date (or "none (pre-rule stage)"), then
      md5 of every live DLL vs the DLL in the staged kpatch of that id; hook counts; consumed_exit_address hooks.

Install itself (game CLOSED, user-gated; `pgrep -f '^S:.*swkotor2\\.exe'` empty; back up kpm_install_state.json + patch_config.toml + patches/ first):
  KPATCH_NO_LAUNCH=1 ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches STAGE_DIR <ids>
Building a kpatch: cd Patches/<Name> && KPATCH_ADDRESS_DATABASES=<dir of symlinks to kotor2_*.db> python3 ../create-patch.py
(needed while AddressDatabases/kotor1_steam.db is a 0-byte file).
"""
import sys, os, glob, re, zipfile, hashlib, shutil, tomllib, collections, subprocess, datetime

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


def git(*a):
    r = subprocess.run(["git", "-C", T, *a], capture_output=True, text=True)
    return r.returncode, r.stdout.rstrip("\n")


def dir_map():
    m = collections.defaultdict(set)
    for f in glob.glob(T + "/Patches/*/manifest.toml"):
        mm = re.search(r'^id\s*=\s*"([^"]+)"', open(f).read(), re.M)
        if mm:
            m[mm.group(1)].add("Patches/" + os.path.basename(os.path.dirname(f)))
    return m


def srcdir(path, i, dmap, extra=False):
    parts = os.path.relpath(path, T).split(os.sep)
    if parts[0] == "Patches" and len(parts) >= 3:
        return "Patches/" + parts[1]
    if extra:
        return "external"
    ds = dmap.get(i, set())
    return next(iter(ds)) if len(ds) == 1 else "?"


def copy_into(src, stage_dir, name=None):
    """Copy src into stage_dir (as `name`, default basename); no-op when dst already is the same file (re-staging a staged dir)."""
    dst = os.path.join(stage_dir, name or os.path.basename(src))
    if os.path.exists(dst) and os.path.samefile(src, dst):
        return dst
    shutil.copy2(src, dst)
    return dst


def under(path, d):
    d = os.path.realpath(d)
    return os.path.realpath(path).startswith(d + os.sep)


def stage(stage_dir, extras, allow_dirty=False):
    live = tomllib.load(open(G + "/patch_config.toml", "rb"))
    idx = kpatch_index()
    os.makedirs(stage_dir, exist_ok=True)
    ids, bad, src = [], 0, {}
    dmap = dir_map()
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
        cs = [c for c in cs if not under(c[1], stage_dir)] or cs   # prefer sources outside the stage dir (re-run safety)
        match = [c for c in cs if lm and c[2] == lm]
        pick = (match or cs[:1])
        if not pick:
            print("NO KPATCH for", i); bad += 1; continue
        z = zipfile.ZipFile(pick[0][1])
        same = kpatch_hooks(z) == sorted(norm(h) for h in p.get("hooks", []))
        print(f"{i:32s} dll={'match' if match else ('none' if not lm else 'NO-MATCH')} hooks={'SAME' if same else 'DIFF'} <- {pick[0][1].replace(T, '')}")
        bad += (not same) or (lm is not None and not match)
        copy_into(pick[0][1], stage_dir, os.path.basename(pick[0][1]).replace(" ", "_"))
        ids.append(i)
        src[i] = srcdir(pick[0][1], i, dmap)
    for e in extras:
        i = kid(e)
        copy_into(e, stage_dir)
        src[i] = srcdir(e, i, dmap, True)
        if i not in ids:
            ids.append(i)
        print("extra:", i, e)
    print("PROBLEMS:", bad)
    print("IDS:", " ".join(ids))
    dirty, rows = False, []
    for i in ids:
        d = src.get(i, "?")
        if d in ("?", "external"):
            if d == "?":
                print(f"WARN UNKNOWN-SRCDIR {i} (ambiguous or no Patches/ dir; not checked)")
            rows.append(f"{i}\t{d}\tunchecked"); continue
        rc, out = git("status", "--porcelain", "--", d)
        paths = " ".join(l[3:] for l in out.splitlines())
        if rc != 0:
            print(f"WARN GIT-STATUS-FAILED {i} {d}"); rows.append(f"{i}\t{d}\tunchecked"); continue
        if paths:
            dirty = True; print(f"WARN DIRTY {i} {paths}")
        rows.append(f"{i}\t{d}\t" + (f"DIRTY: {paths}" if paths else "clean"))
    head = git("rev-parse", "HEAD")[1]
    rc, up = git("rev-parse", "@{u}")
    upstream = "no-upstream" if rc != 0 else ("pushed" if up == head else "unpushed")
    info = os.path.join(stage_dir, "STAGE_INFO.txt")
    with open(info, "w") as f:
        f.write(f"branch={git('branch', '--show-current')[1]}\nhead={head}\nupstream={upstream}\ndate={datetime.datetime.now().isoformat(timespec='seconds')}\nids={' '.join(ids)}\n" + "".join(r + "\n" for r in rows))
    print(f"STAGE_INFO: {info} head={head[:12]}")
    if dirty and not allow_dirty:
        print("DIRTY source dirs: commit them before install (or pass --allow-dirty)"); sys.exit(2)


def verify(stage_dir):
    live = tomllib.load(open(G + "/patch_config.toml", "rb"))
    kp = {}
    si = os.path.join(stage_dir, "STAGE_INFO.txt")
    if os.path.exists(si):
        kv = dict(l.split("=", 1) for l in open(si).read().splitlines() if "=" in l and "\t" not in l)
        print(f"STAGE_INFO: branch={kv.get('branch')} head={kv.get('head', '')[:12]} date={kv.get('date')}")
    else:
        print("STAGE_INFO: none (pre-rule stage)")
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
        rest = sys.argv[3:]
        stage(sys.argv[2], [a for a in rest if a != "--allow-dirty"], "--allow-dirty" in rest)
    elif len(sys.argv) == 3 and sys.argv[1] == "verify":
        verify(sys.argv[2])
    else:
        print(__doc__)
