#!/usr/bin/env python3
"""List GUI-usable textures (TexturePacks ERFs + override) matching a regex; optionally render PNGs.
usage: .venv/bin/python3 list_icons.py [--filter REGEX] [--max N] [--render resref,...] [--out DIR]
Read-only on the game dir."""
import argparse, logging, re
from pathlib import Path
logging.disable(logging.CRITICAL)
from PIL import Image
from pykotor.extract.installation import Installation
from pykotor.resource.formats.tpc import TPCTextureFormat
from pykotor.resource.formats.erf import read_erf

GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
DEFAULT = r"opt|gear|cog|setting|config|lvl|level|up|ip_|i_"

def entries():
    for erf in sorted((GAME / "TexturePacks").glob("*.erf")):
        for r in read_erf(erf):
            if str(r.restype).lower().endswith(("tpc", "tga")) or r.restype.extension in ("tpc", "tga"):
                yield str(r.resref), erf.name
    for p in sorted((GAME / "override").iterdir()):
        if p.suffix.lower() in (".tpc", ".tga"):
            yield p.stem, "override"

def load(inst, name):
    t = inst.texture(name)
    if t is None:
        return None
    t.convert(TPCTextureFormat.RGBA)
    m = t.get(0, 0)
    return Image.frombytes("RGBA", (m.width, m.height), bytes(m.data)).transpose(Image.FLIP_TOP_BOTTOM)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--filter", default=DEFAULT)
    ap.add_argument("--max", type=int, default=60)
    ap.add_argument("--render", default="")
    ap.add_argument("--out", default=str(Path(__file__).parent))
    a = ap.parse_args()
    rx = re.compile(a.filter, re.I)
    inst = Installation(GAME)
    seen, hits = set(), []
    for name, src in entries():
        if name.lower() in seen or not rx.search(name):
            continue
        seen.add(name.lower()); hits.append((name, src))
    print(f"{len(hits)} matches for /{a.filter}/")
    for name, src in hits[:a.max]:
        im = load(inst, name)
        print(f"{name:28s} {('%dx%d' % im.size) if im else '?':>9s}  {src}")
    if len(hits) > a.max:
        print(f"... {len(hits) - a.max} more (raise --max or narrow --filter)")
    for name in filter(None, a.render.split(",")):
        im = load(inst, name)
        if im:
            im.save(Path(a.out) / f"{name}.png"); print("rendered", name)

if __name__ == "__main__":
    main()
