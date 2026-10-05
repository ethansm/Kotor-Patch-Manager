#!/usr/bin/env python3
"""Stage the buff/shield split install set (doc 21 W5) without touching the game.

Builds a patch_config.toml from the LIVE one: the buff-duration-hud [[patches]] block (the single-DLL build, or an already split set)
is replaced by, in this order (provider first: KPatchInit runs in config order):
    portrait-slots   (always)
    buff-duration-hud   (variants both, buff)
    shield-bar          (variants both, shield)
Hook entries are generated from each patch's hooks.toml (addresses in decimal, as the installer writes them). Then it re-parses
the result with tomllib and fails on any duplicate address or overlapping [address, address+len(original_bytes)) span.

  stage_portrait_split.py --variant both|buff|shield|none [--out DIR]
Writes DIR/patch_config.toml, copies the DLLs from $KPM/split_build/<Folder>.kpatch (the split packages are kept out of Patches/,
whose BuffDurationHUD.kpatch stays the live single-DLL build until W5, so sibling reinstalls keep working) into DIR/patches/, writes DIR/STAGE_INFO.txt
(md5s, source commits). Installing is a separate, user-approved step (game closed, backup first).
"""
import argparse, hashlib, io, os, re, subprocess, sys, tomllib, zipfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
KPM = Path(os.environ.get("KPM_DIR", Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"))
PATCHES = {   # id -> (KPM folder, dll file name in the game's patches/)
    "portrait-slots": ("PortraitSlots", "portrait-slots.dll"),
    "buff-duration-hud": ("BuffDurationHUD", "buff-duration-hud.dll"),
    "shield-bar": ("ShieldBar", "shield-bar.dll"),
}
VARIANTS = {"both": ["portrait-slots", "buff-duration-hud", "shield-bar"], "buff": ["portrait-slots", "buff-duration-hud"],
            "shield": ["portrait-slots", "shield-bar"], "none": ["portrait-slots"]}
OURS = set(PATCHES)


def manifest_version(archive_path):
    """`version` from the manifest.toml inside a kpatch (zip) archive, or None if unreadable/absent."""
    try:
        with zipfile.ZipFile(archive_path) as z:
            v = tomllib.loads(z.read("manifest.toml").decode()).get("version")
        return None if v is None else str(v)
    except (OSError, KeyError, zipfile.BadZipFile, tomllib.TOMLDecodeError, UnicodeDecodeError):
        return None


def block_for(pid: str) -> str:
    folder, dll = PATCHES[pid]
    hooks = tomllib.loads(zipfile.ZipFile(KPM / "split_build" / f"{folder}.kpatch").read("kotor2-steam-aspyr.hooks.toml").decode())["hooks"]
    out = [f'[[patches]]\nid = "{pid}"\ndll = "patches/{dll}"\n']
    for h in hooks:
        out.append("[[patches.hooks]]\n")
        out.append(f"address = {int(h['address'])}\n")
        out.append(f"original_bytes = [{', '.join(str(b) for b in h['original_bytes'])}]\n")
        out.append(f'type = "{h["type"]}"\n')
        out.append(f'function = "{h["function"]}"\n')
        if h.get("exclude_from_restore"):
            out.append(f"exclude_from_restore = [{', '.join(repr(x).replace(chr(39), chr(34)) for x in h['exclude_from_restore'])}]\n")
        for p in h.get("parameters", []):
            out.append(f'[[patches.hooks.parameters]]\nsource = "{p["source"]}"\ntype = "{p["type"]}"\n')
    return "".join(out)


def split_blocks(text: str):
    """(preamble, [(id, block_text)]) split on top-level [[patches]] headers."""
    idx = [m.start() for m in re.finditer(r"^\[\[patches\]\]\s*$", text, re.M)]
    if not idx:
        sys.exit("no [[patches]] in live config")
    pre, blocks = text[:idx[0]], []
    for a, b in zip(idx, idx[1:] + [len(text)]):
        blk = text[a:b]
        m = re.search(r'^id = "([^"]+)"', blk, re.M)
        blocks.append((m.group(1) if m else "?", blk))
    return pre, blocks


def check(text: str) -> list:
    cfg = tomllib.loads(text)
    spans, errs = [], []
    for p in cfg.get("patches", []):
        for h in p.get("hooks", []):
            n = len(h.get("original_bytes", [])) or len(h.get("replacement_bytes", [])) or 1
            spans.append((int(h["address"]), int(h["address"]) + n, p["id"], h.get("function", h.get("type"))))
    spans.sort()
    for (a0, a1, i0, f0), (b0, b1, i1, f1) in zip(spans, spans[1:]):
        if b0 < a1:
            errs.append(f"overlap 0x{a0:08X}-0x{a1:08X} {i0}:{f0} <-> 0x{b0:08X}-0x{b1:08X} {i1}:{f1}")
    ids = [p["id"] for p in cfg.get("patches", [])]
    if "portrait-slots" in ids:
        for c in ("buff-duration-hud", "shield-bar"):
            if c in ids and ids.index(c) < ids.index("portrait-slots"):
                errs.append(f"{c} is listed before portrait-slots (its KPatchInit would miss the provider)")
    return errs


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--variant", choices=VARIANTS, required=True)
    ap.add_argument("--out", type=Path)
    a = ap.parse_args()
    out = a.out or KPM / f"split_stage_{a.variant}"
    live = (GAME / "patch_config.toml").read_text()
    pre, blocks = split_blocks(live)
    first = next((i for i, (pid, _) in enumerate(blocks) if pid in OURS), None)
    if first is None:
        sys.exit("live config has none of portrait-slots / buff-duration-hud / shield-bar")
    kept = [b for pid, b in blocks if pid not in OURS]
    new_ours = [block_for(pid) for pid in VARIANTS[a.variant]]
    others_before = [b for pid, b in blocks[:first] if pid not in OURS]
    others_after = kept[len(others_before):]
    text = pre + "".join(others_before) + "".join(new_ours) + "".join(others_after)
    errs = check(text)
    if errs:
        print("STAGE FAIL:\n  " + "\n  ".join(errs))
        return 1
    (out / "patches").mkdir(parents=True, exist_ok=True)
    (out / "patch_config.toml").write_text(text)
    info = [f"variant {a.variant}", f"live config md5 {hashlib.md5(live.encode()).hexdigest()}"]
    for pid in VARIANTS[a.variant]:
        folder, dll = PATCHES[pid]
        kp = KPM / "split_build" / f"{folder}.kpatch"
        data = zipfile.ZipFile(kp).read("binaries/windows_x86.dll")
        (out / "patches" / dll).write_bytes(data)
        info.append(f"{dll} {hashlib.md5(data).hexdigest()} version {manifest_version(kp) or '?'} (from {kp.name}, {kp.stat().st_mtime:.0f})")
    for repo in (REPO, KPM):
        info.append(f"{repo.name} HEAD {subprocess.run(['git', '-C', str(repo), 'rev-parse', '--short', 'HEAD'], capture_output=True, text=True).stdout.strip()}")
    (out / "STAGE_INFO.txt").write_text("\n".join(info) + "\n")
    print(f"STAGED {out}\n  " + "\n  ".join(info))
    return 0


if __name__ == "__main__":
    sys.exit(main())
