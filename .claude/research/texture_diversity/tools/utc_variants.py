#!/usr/bin/env python3
"""Per-spawn texture variant: read TextureVar from each creature UTC and resolve the body texture (read-only, no model tokens).

Texture = racetex (exact) else texa base + '%02d' % max(TextureVar,1) (no *00 file exists in the texture packs, so var 0 is read as 01; inferred). Writes
w5/utc_variants.tsv (module/template/app/var/texture/spawns) and w5/atlas_exact.tsv (per atlas spawns).
Run: ../../../.venv/bin/python -I tools/utc_variants.py
"""
import sys, json, logging, collections
from pathlib import Path
logging.disable(logging.CRITICAL)
REPO = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(REPO / "scripts"))
import npc_div_registry as R
from pykotor.resource.formats.gff import read_gff
from pykotor.resource.formats.twoda import read_2da

app = read_2da(str(R.GAME / "override/appearance.2da"))
g = lambda r, c: r.get_string(c).strip().lower()
reg = [json.loads(l) for l in open(REPO / ".claude/npc_div/registry.jsonl")]
need = collections.defaultdict(collections.Counter)  # module -> template -> spawns
for r in reg:
    if r.get("template") and not r.get("unresolved"):
        need[r["module"]][r["template"]] += 1
files = R.module_files(R.GAME / "modules")
atlases = {l.split("\t")[0] for l in open("w5/inventory.tsv").read().splitlines()[1:] if l}
rows, per_atlas = [], collections.defaultdict(lambda: collections.Counter())
miss = 0
for stem, tpls in sorted(need.items()):
    if stem not in files:
        miss += sum(tpls.values()); continue
    try:
        _git, utc = R.module_resources(files[stem])
    except Exception:
        miss += sum(tpls.values()); continue
    for tpl, n in tpls.items():
        if tpl not in utc:
            miss += n; continue
        u = read_gff(utc[tpl]).root
        a = int(R._get(u, "Appearance_Type", -1)); var = int(R._get(u, "TextureVar", 0) or 0)
        ar = app.get_row(a) if 0 <= a < app.get_height() else None
        tex = ""
        if ar is not None:
            rt, ta = g(ar, "racetex"), g(ar, "texa")
            tex = rt or (f"{ta}{max(var, 1):02d}" if ta else "")  # var 0 -> 01: no *00 texture exists in the packs
        rows.append((stem, tpl, a, var, tex, n))
        if tex in atlases:
            per_atlas[tex][(stem)] += n
with open("w5/utc_variants.tsv", "w") as f:
    f.write("module\ttemplate\tapp\ttexvar\ttexture\tspawns\n")
    for r in rows: f.write("\t".join(map(str, r)) + "\n")
tot = collections.Counter()
for r in rows: tot[r[4]] += r[5]
with open("w5/atlas_exact.tsv", "w") as f:
    f.write("atlas\tspawns\ttop_modules\n")
    for a in sorted(atlases):
        f.write(f"{a}\t{sum(per_atlas[a].values())}\t{','.join(f'{k}:{v}' for k,v in per_atlas[a].most_common(4))}\n")
print("unresolved spawns", miss, "templates", len(rows))
print(open("w5/atlas_exact.tsv").read())
# validate formula on commoners
print({k: v for k, v in sorted(tot.items()) if k.startswith("n_commm")}.__len__(), "commm textures seen; sample", sorted(tot.items(), key=lambda x: -x[1])[:5])
