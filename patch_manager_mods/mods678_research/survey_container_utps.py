#!/usr/bin/env python3
"""Survey every container placeable template (UTP with HasInventory=1) in modules/*.mod|rim and
override/*.utp: script slots, static item count, plot/trap/lock flags. Produced
utp_container_survey.json and utp_container_script_slots.txt (2026-09-30: 850 containers).

Run: ../../.venv/bin/python survey_container_utps.py OUT_DIR
Classification used in 12_empty_container_indicators.md: treasure scripts match
^(k_plc_tre|k_plc_treas|k_plc_it_|k_plev_cons|k_ptar_conttr|k_punk_treas|k_def_.*trea); they sit in
OnHeartbeat (276) or OnOpen (11, all kasm22ab). Limitation: templates only; GIT instances and
scripts that CreateItemOnObject from elsewhere are not covered.
"""
import collections, glob, json, logging, os, sys

logging.disable(logging.CRITICAL)
from pykotor.resource.formats.erf import read_erf  # noqa: E402
from pykotor.resource.formats.rim import read_rim  # noqa: E402
from pykotor.resource.formats.gff import read_gff  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

G = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
SLOTS = ["OnHeartbeat", "OnOpen", "OnUsed", "OnInvDisturbed", "OnClosed", "OnDisarm", "OnTrapTriggered",
         "OnUserDefined", "OnDeath", "OnMeleeAttacked", "OnDamaged", "OnSpellCastAt", "OnLock", "OnUnlock",
         "OnEndDialogue"]
seen = {}


def handle(name, data, src):
    try:
        r = read_gff(data).root
    except Exception:
        return
    if not (r.exists("HasInventory") and r.get_uint8("HasInventory")):
        return
    d = {s: str(r.get_resref(s)) for s in SLOTS if r.exists(s) and str(r.get_resref(s))}
    d["_items"] = len(r.get_list("ItemList")) if r.exists("ItemList") else 0
    for k, f in (("_plot", "Plot"), ("_trap", "TrapFlag"), ("_locked", "Locked"), ("_bodybag", "BodyBag")):
        d[k] = r.get_uint8(f) if r.exists(f) else 0
    seen[f"{src}/{name}"] = d


for f in glob.glob(G + "/modules/*"):
    fl = f.lower()
    try:
        cap = read_erf(f) if fl.endswith(".mod") else read_rim(f) if fl.endswith(".rim") else None
    except Exception:
        continue
    if cap:
        mod = os.path.basename(fl).split(".")[0].replace("_s", "")
        for res in cap:
            if res.restype == ResourceType.UTP:
                handle(str(res.resref).lower(), res.data, mod)
for f in glob.glob(G + "/override/*.utp"):
    handle(os.path.basename(f)[:-4].lower(), open(f, "rb").read(), "override")

out = sys.argv[1]
json.dump(seen, open(os.path.join(out, "utp_container_survey.json"), "w"), indent=0)
c = collections.Counter((s, v[s]) for v in seen.values() for s in SLOTS if s in v)
with open(os.path.join(out, "utp_container_script_slots.txt"), "w") as fh:
    fh.write(f"containers: {len(seen)}\n")
    for (s, v), n in c.most_common(60):
        fh.write(f"{n} {s} {v}\n")
print(len(seen), "containers")
