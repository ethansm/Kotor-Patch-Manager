#!/usr/bin/env python3
"""Extract every vanilla .nss source from the game's BIFs (Scripts.bif has 638, incl.
k_ai_master, k_inc_generic, k_inc_gensupport, nwscript, k_inc_treas_k2).

Run with the repo venv: ../../.venv/bin/python extract_vanilla_nss.py OUT_DIR
Note: the copies in ~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/build.*/ of
k_inc_generic/k_inc_gensupport are byte-identical to vanilla (they came from Mines Overhaul's
Optional_NPCsUseMines, whose k_ai_master differs by 21 lines). override/k_inc_treas_k2.nss is
modded (adds VP_RandomGenSaber); the BIF copy is vanilla.
"""
import logging, sys
from pathlib import Path

logging.disable(logging.CRITICAL)
from pykotor.extract.installation import Installation  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

GAME = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
n = 0
for res in Installation(GAME).chitin_resources():
    if res.restype() == ResourceType.NSS:
        (out / f"{res.resname()}.nss").write_bytes(res.data())
        n += 1
print(n, "nss written to", out)
