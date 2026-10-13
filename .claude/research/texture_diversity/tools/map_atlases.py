#!/usr/bin/env python3
"""Map the 51 wave-5 atlases to the creature spawns that use them (deterministic, no model tokens).

KOTOR body textures resolve as texa/racetex base + 2-digit UTC variant (e.g. texa N_CzerkaOff + 01 -> n_czerkaoff01),
and a blank texa falls back to the model's own texture. The registry has no per-spawn variant, so users are counted per
FAMILY (atlas name minus trailing digits); racetex values are exact atlas names. Evidence kinds per appearance row:
  racetex = exact atlas name, texa = family base, model = race/modela base with blank texa (model-default guess).
Writes w5/atlas_users.tsv, w5/atlas_users.md, w5/faction_probe.tsv. Run from texture_diversity dir:
  ../../../.venv/bin/python -I tools/map_atlases.py
"""
import json, logging, collections, re
from pathlib import Path
logging.disable(logging.CRITICAL)
from pykotor.resource.formats.twoda import read_2da

REPO = Path(__file__).resolve().parents[4]
GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
app = read_2da(str(GAME / "override/appearance.2da"))
g = lambda r, c: r.get_string(c).strip().lower()
atlases = [l.split("\t")[0] for l in open("w5/inventory.tsv").read().splitlines()[1:] if l]
fam = lambda a: re.sub(r"\d+$", "", a)
fams = collections.defaultdict(list)
for a in atlases:
    fams[fam(a)].append(a)
reg = [json.loads(l) for l in open(REPO / ".claude/npc_div/registry.jsonl")]
by_app = collections.defaultdict(list)
for r in reg:
    if r.get("app") is not None:
        by_app[r["app"]].append(r)

hits = collections.defaultdict(list)  # family -> [(row, kind, label)]
for i in range(app.get_height()):
    r = app.get_row(i)
    rt, ta, race, ma = g(r, "racetex"), g(r, "texa"), g(r, "race"), g(r, "modela")
    lab = r.get_string("label")
    if rt in atlases:
        hits[fam(rt)].append((i, "racetex:" + rt, lab))
    elif ta and ta in fams:
        hits[ta].append((i, "texa", lab))
    elif not ta and not rt:
        for m in (ma, race):
            if m and m in fams:
                hits[m].append((i, "model", lab)); break
# n_ond_off / ond model guess: model N_OndOffM1 is the officer body; flagged below
hits.setdefault("n_ond_off", [])
for i in range(app.get_height()):
    r = app.get_row(i)
    if g(r, "modela") == "n_ondoffm1" and not g(r, "texa"):
        hits["n_ond_off"].append((i, "model-guess", r.get_string("label")))

out = ["family\tatlases\tapp_rows\tkinds\tspawns\tnames\tfactions\tmodules"]
md = ["# Atlas family -> creature users", "", "Source: override/appearance.2da x npc_div/registry.jsonl. Variant digit unknown per spawn, so counts are per family.", ""]
for f in sorted(fams):
    rows = hits.get(f, [])
    sp = [s for (i, k, l) in rows for s in by_app.get(i, [])]
    names = collections.Counter(s.get("name", "?") for s in sp)
    facs = collections.Counter(s.get("faction") for s in sp)
    mods = collections.Counter(s.get("module") for s in sp)
    kinds = sorted({k.split(":")[0] for (_, k, _) in rows})
    out.append("\t".join([f, ",".join(x.replace(f, "") or x for x in fams[f]), ",".join(str(i) for (i, _, _) in rows), ",".join(kinds),
                          str(len(sp)), "; ".join(f"{k}({v})" for k, v in names.most_common(6)),
                          ",".join(f"{k}:{v}" for k, v in facs.most_common(3)), ",".join(f"{k}:{v}" for k, v in mods.most_common(4))]))
    md.append(f"- **{f}** ({', '.join(fams[f])}): {len(sp)} spawns via rows {[i for (i,_,_) in rows][:6]} [{','.join(kinds) or 'NO ROW'}]: " + (", ".join(f"{k} x{v}" for k, v in names.most_common(5)) or "none"))
open("w5/atlas_users.tsv", "w").write("\n".join(out) + "\n")
open("w5/atlas_users.md", "w").write("\n".join(md) + "\n")
# faction probe: name keywords -> appearance row -> texture base
pro = ["keyword\tspawns\tapp_rows\tmodel/texa"]
for kw in ("vaklu", "covenant", "exchange", "czerka", "tsf", "telos security", "onderon", "khoonda", "mandalorian", "sith"):
    sp = [s for s in reg if kw in (s.get("name", "") + " " + s.get("tag", "")).lower()]
    rows = collections.Counter(s.get("app") for s in sp)
    desc = []
    for i, n in rows.most_common(6):
        if i is None: continue
        r = app.get_row(i)
        desc.append(f"{i}x{n}:{g(r,'modela') or g(r,'race')}/{g(r,'texa') or g(r,'racetex') or '-'}")
    pro.append(f"{kw}\t{len(sp)}\t{len(rows)}\t{' '.join(desc)}")
open("w5/faction_probe.tsv", "w").write("\n".join(pro) + "\n")
print("\n".join(out)); print(); print("\n".join(pro))
