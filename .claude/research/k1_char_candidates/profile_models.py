"""Profile K1-only party-member models: supermodel, node/anim counts, textures, K2 presence. Read-only."""
import logging, sys; logging.disable(logging.CRITICAL)
from pathlib import Path
from pykotor.extract.installation import Installation, SearchLocation
from pykotor.resource.formats.mdl import read_mdl
from pykotor.resource.type import ResourceType as RT
root = Path.home() / ".local/share/Steam/steamapps/common"
I1 = Installation(str(root / "swkotor")); I2 = Installation(str(root / "Knights of the Old Republic II"))
CH = [SearchLocation.CHITIN]

def get(inst, name, rt):
    r = inst.resource(name, rt, CH)
    return r.data if r else None

for n in sys.argv[1:]:
    d, x = get(I1, n, RT.MDL), get(I1, n, RT.MDX)
    print("==", n, "mdl", len(d) if d else None, "mdx", len(x) if x else None)
    if not d: continue
    try: m = read_mdl(d, source_ext=x)
    except Exception as e: print("  read fail", e); continue
    nodes = m.all_nodes()
    tex = sorted({str(nd.mesh.texture_1).lower() for nd in nodes if getattr(nd, "mesh", None) and nd.mesh and str(nd.mesh.texture_1).lower() not in ("", "null")})
    skin = sum(1 for nd in nodes if getattr(getattr(nd, "mesh", None), "skin", None))
    print("  supermodel", m.supermodel, "class", m.classification, "anims", len(m.anims), "nodes", len(nodes), "skinned", skin)
    print("  anims:", [a.name for a in m.anims][:10])
    print("  textures:", tex)
    print("  tex in K2 (tpc):", {t: (get(I2, t, RT.TPC) is not None) for t in tex})
    sm = str(m.supermodel).lower()
    print("  supermodel", sm, "in K2:", get(I2, sm, RT.MDL) is not None, "| K1:", get(I1, sm, RT.MDL) is not None)
