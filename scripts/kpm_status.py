#!/usr/bin/env python3
"""One-shot install state of every repo mod: what is live, what needs a hot-swap, a rebuild, or a full reinstall.

Usage: python3 scripts/kpm_status.py [--game DIR] [--kpm DIR] [--json] [--only id,id]
Read-only (git status, file reads, zip reads; no compiler, no game-dir writes). Runs in well under a second.

Per repo mod (patch_manager_mods/*_src with a manifest) one line:  VERDICT  id  version  live/kpatch md5  detail
Verdicts, first match wins:
  NOT-LIVE     id not in the live patch_config.toml (library, retired probe, PR-only fix)
  DIRTY        uncommitted production source in the repo dir: commit first (tests/docs ignored)
  MIRROR       repo production files differ from Patches/<X>/ in KPM: copy them over, then rebuild
  REBUILD      kpatch older than its KPM code, or its manifest/hooks differ from the KPM files: run create-patch.py
  LIVE         live DLL md5 == kpatch DLL md5
  REINSTALL    kpatch hooks != the live patch_config hooks for the id: hot-swap NOT allowed (KPatchLauncher)
  LIVE-OTHER   live DLL differs but is newer than the kpatch (a gate/stage build of the same source); swap only on purpose
  SWAP         kpatch is fresh, hooks match live, DLL differs: ready for a DLL hot-swap
Ends with summary lines `SWAP: ...`, `REBUILD: ...` etc. that name the ids for the next step.
"""
import argparse
import difflib
import filecmp
import hashlib
import json
import pathlib
import re
import subprocess
import sys
import tomllib
import zipfile

REPO = pathlib.Path(__file__).resolve().parent.parent
MODS = REPO / "patch_manager_mods"
KPM_DEFAULT = pathlib.Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"
GAME_DEFAULT = pathlib.Path.home() / ".steam/steam/steamapps/common/Knights of the Old Republic II"
CODE_EXT = {".cpp", ".h", ".inc", ".def"}
ASSET_EXT = {".ini", ".toml", ".tpc", ".mdl", ".mdx", ".2da", ".tga"}  # production assets, incl. code_patches.toml and override/ trees
ORDER = ["NOT-LIVE", "DIRTY", "MIRROR", "REBUILD", "REINSTALL", "SWAP", "LIVE-OTHER", "LIVE"]


def md5(b):
    return hashlib.md5(b).hexdigest()[:8]


def is_production(name):
    """Files that go into the DLL or the kpatch (not tests, smokes, docs or the test config)."""
    p = pathlib.PurePath(name)
    if name == "test.toml" or name.endswith("_test.cpp") or name.startswith("smoke_"):
        return False
    return p.suffix in CODE_EXT or p.suffix in ASSET_EXT or name == "manifest.toml" or name.endswith(".hooks.toml")


def norm_hooks(hooks):
    """Canonical, order-free hook set: the fields that decide what gets patched. Defaults are dropped like the installer does."""
    out = []
    for h in hooks or []:
        out.append(json.dumps({
            "address": int(h["address"]),
            "type": h.get("type", "detour"),
            "function": h.get("function"),
            "original_bytes": list(h.get("original_bytes", [])),
            "consumed_exit_address": h.get("consumed_exit_address"),
            "exclude_from_restore": sorted(h.get("exclude_from_restore", []) or []),
            "params": [p.get("source") for p in h.get("parameters", []) or []],
        }, sort_keys=True))
    return sorted(out)


def steam_hooks_name(names):
    """The live game is Steam/Windows: pick kotor2-steam-aspyr.hooks.toml, never the GOG/linux/macos files next to it."""
    hooks = [n for n in names if n.endswith("hooks.toml")]
    steam = [n for n in hooks if "steam" in n and not any(x in n for x in ("linux", "macos", "mac"))]
    return (steam or [n for n in hooks if "gog" not in n] or [None])[0]


def git_dirty(src):
    r = subprocess.run(["git", "-C", str(REPO), "status", "--porcelain", "--", str(src)], capture_output=True, text=True)
    return [l[3:].split("/")[-1] for l in r.stdout.splitlines() if is_production(l[3:].split("/")[-1])]


class DuplicateKpmId(Exception):
    """Two Patches/<X> folders hold the same patch id (usually a casing twin such as save-events + SaveEvents)."""


def kpm_dirs(kpm):
    out = {}
    for m in sorted((kpm / "Patches").glob("*/manifest.toml")):
        try:
            with open(m, "rb") as f:
                pid = tomllib.load(f)["patch"]["id"]
        except (KeyError, tomllib.TOMLDecodeError, OSError):
            continue
        if pid in out and out[pid] != m.parent:
            raise DuplicateKpmId(f"FATAL: Duplicate KPM patch ID '{pid}' found in directories '{out[pid].name}' and '{m.parent.name}'")
        out[pid] = m.parent
    return out


def is_probe_manifest(m):
    """True when the manifest's version has a -probe suffix (a probe build dir, not the release dir)."""
    try:
        return "-probe" in str(tomllib.load(open(m, "rb"))["patch"].get("version", ""))
    except (KeyError, tomllib.TOMLDecodeError, OSError):
        return False


def repo_manifests():
    """Repo mod manifests: <x>_src/manifest.toml, plus nested ones at any depth such as perf_lab_src/perflab/manifest.toml."""
    return sorted(MODS.glob("*_src/**/manifest.toml"))


def pick_sources(manifests, kpm_map):
    """One repo dir per patch id. When several dirs share an id (an old probe dir next to the release dir), keep the one whose
    manifest.toml equals the KPM copy. Failing that, the newest non-probe dir by mtime; probe dirs only when no release dir exists."""
    by_id = {}
    for m in manifests:
        try:
            by_id.setdefault(tomllib.load(open(m, "rb"))["patch"]["id"], []).append(m)
        except (KeyError, tomllib.TOMLDecodeError, OSError):
            by_id.setdefault(m.parent.name, []).append(m)
    out = []
    for pid, ms in by_id.items():
        kd = kpm_map.get(pid)
        same = [m for m in ms if kd and (kd / "manifest.toml").exists() and m.read_bytes() == (kd / "manifest.toml").read_bytes()]
        release = [m for m in ms if not is_probe_manifest(m)] or ms
        out.append(same[0] if same else max(release, key=lambda m: m.stat().st_mtime))
    return sorted(out)


def status(mod_manifest, kpm_map, live, game):
    man = tomllib.load(open(mod_manifest, "rb"))["patch"]
    pid, ver, src = man["id"], man.get("version", "?"), mod_manifest.parent
    rel = str(src.relative_to(REPO)) if src.is_relative_to(REPO) else str(src)
    row = {"id": pid, "version": ver, "src": rel, "live": "", "kpatch": "", "detail": ""}
    if pid not in live:
        return {**row, "verdict": "NOT-LIVE"}
    dirty = git_dirty(src)
    if dirty:
        return {**row, "verdict": "DIRTY", "detail": ",".join(dirty[:5])}
    kd = kpm_map.get(pid)
    if not kd:
        return {**row, "verdict": "MIRROR", "detail": "no Patches/<X> dir with this id in KPM"}
    row["kpm"] = str(kd)
    drift = [f.name for f in sorted(src.iterdir()) if f.is_file() and is_production(f.name)
             and (not (kd / f.name).exists() or not filecmp.cmp(f, kd / f.name, shallow=False))]
    if drift:
        return {**row, "verdict": "MIRROR", "detail": ",".join(drift[:5])}
    kps = sorted(kd.glob("*.kpatch"))
    if not kps:
        return {**row, "verdict": "REBUILD", "detail": "no kpatch"}
    kp = kps[0]
    with zipfile.ZipFile(kp) as z:
        names = z.namelist()
        kdll = z.read("binaries/windows_x86.dll")
        kman = z.read("manifest.toml")
        hname = steam_hooks_name(names)
        khooks_raw = z.read(hname) if hname else b""
    row["kpatch"] = md5(kdll)
    kp_m = kp.stat().st_mtime
    newer = [f.name for f in kd.iterdir() if f.is_file() and f.suffix in CODE_EXT and f.stat().st_mtime > kp_m]
    if (kd / "manifest.toml").read_bytes() != kman:
        newer.append("manifest.toml(content)")
    if hname and (kd / hname).exists() and (kd / hname).read_bytes() != khooks_raw:
        newer.append(hname + "(content)")
    if newer:
        return {**row, "verdict": "REBUILD", "detail": "kpatch older than " + ",".join(sorted(newer)[:5])}
    dll = game / live[pid].get("dll", f"patches/{pid}.dll")
    if not dll.exists():
        return {**row, "verdict": "SWAP", "detail": "live DLL missing"}
    row["live"] = md5(dll.read_bytes())
    if row["live"] == row["kpatch"]:
        return {**row, "verdict": "LIVE"}
    khooks = tomllib.loads(khooks_raw.decode()).get("hooks", []) if hname else []
    kn, ln = norm_hooks(khooks), norm_hooks(live[pid].get("hooks", []))
    if kn != ln:
        return {**row, "verdict": "REINSTALL", "detail": f"hooks kpatch {len(kn)} vs live {len(ln)}, {len(set(kn) - set(ln))} differ (address/bytes/function/params)"}
    if dll.stat().st_mtime > kp_m:
        return {**row, "verdict": "LIVE-OTHER", "detail": "live DLL newer than kpatch (other build)"}
    return {**row, "verdict": "SWAP", "detail": kp.name}


def suggest(query, ids):
    """Closest known patch id to a mistyped query (case, '-' and '_' ignored), or None."""
    def key(s):
        return re.sub(r"[^a-z0-9]", "", s.lower())
    by_key = {key(i): i for i in ids}
    hit = difflib.get_close_matches(key(query), list(by_key), n=1, cutoff=0.6)
    return by_key[hit[0]] if hit else None


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--game", type=pathlib.Path, default=GAME_DEFAULT)
    ap.add_argument("--kpm", type=pathlib.Path, default=KPM_DEFAULT)
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--only", default="", help="comma-separated ids")
    a = ap.parse_args(argv)
    cfg = a.game / "patch_config.toml"
    if not cfg.exists():
        print(f"no {cfg}", file=sys.stderr)
        return 2
    live = {p["id"]: p for p in tomllib.load(open(cfg, "rb")).get("patches", [])}
    try:
        kmap = kpm_dirs(a.kpm)
    except DuplicateKpmId as e:
        print(str(e), file=sys.stderr)
        return 2
    only = {x for x in a.only.split(",") if x}
    rows, known = [], set()
    for m in pick_sources(repo_manifests(), kmap):
        try:
            r = status(m, kmap, live, a.game)
        except (KeyError, tomllib.TOMLDecodeError, zipfile.BadZipFile) as e:
            r = {"id": m.parent.name, "version": "?", "verdict": "REBUILD", "live": "", "kpatch": "", "detail": f"unreadable: {e}"}
        known.add(r["id"])
        if not only or r["id"] in only:
            rows.append(r)
    for x in sorted(only - known):
        hint = suggest(x, known)
        print(f"usage error: unknown id '{x}'" + (f"\nDid you mean: '{hint}'?" if hint else ""), file=sys.stderr)
        return 2
    rows.sort(key=lambda r: (ORDER.index(r["verdict"]), r["id"]))
    if a.json:
        print(json.dumps(rows, indent=1))
        return 0
    for r in rows:
        md = f"{r['live'] or '-'}/{r['kpatch'] or '-'}"
        print(f"{r['verdict']:<10} {r['id']:<28} {r['version']:<14} {md:<17} {r['detail']}")
    for v in ORDER[1:6]:
        ids = sorted({r["id"] for r in rows if r["verdict"] == v})
        if ids:
            print(f"{v}: {' '.join(ids)}")
    print(f"game_running={'yes' if subprocess.run(['pgrep', '-f', r'^S:.*swkotor2\.exe'], capture_output=True).returncode == 0 else 'no'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
