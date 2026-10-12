import sys
sys.path.insert(0, sys.argv[1])
from pykotor.resource.formats.mdl import MDL, MDLNode, MDLMesh, bytes_mdl, read_mdl
from pykotor.resource.formats.mdl.mdl_types import MDLNodeType

def length(name, dirt):
    m = MDL()
    c = MDLNode()
    c.name = name
    c.node_type = MDLNodeType.TRIMESH
    c.mesh = MDLMesh()
    c.mesh.dirt_texture = dirt
    m.root.children.append(c)
    data = bytes_mdl(m)
    back = read_mdl(data).root.children[0].mesh.dirt_texture
    return len(data), back

for name in ("tri", "probe_mesh"):
    k1, _ = length(name, 1)
    k2, back = length(name, 2)
    print(f"name={name!r} K1={k1} K2={k2} delta={k2-k1} round_trip_dirt={back}")
