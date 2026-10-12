"""Reproduce the pykotor 2.3.12 K2 trimesh write failure (doc 38, section 2).

Setup (scratch venv, not the repo):
    python3 -m venv <scratch>/venv_pk
    <scratch>/venv_pk/bin/pip install pykotor==2.3.12
Run:
    <scratch>/venv_pk/bin/python -I probe_pykotor_k2_write.py

Expected on 2.3.12 unmodified:
    control dirt_texture=1 -> OK (K1 layout, 724 bytes)
    probe   dirt_texture=2 -> ValueError: _TrimeshHeader.write wrote 348 bytes, expected 340
"""

import traceback

from pykotor.resource.formats.mdl import MDL, MDLNode, MDLMesh, bytes_mdl
from pykotor.resource.formats.mdl.mdl_types import MDLNodeType


def build(dirt_texture):
    mdl = MDL()
    node = MDLNode()
    node.name = "probe_mesh"
    node.node_type = MDLNodeType.TRIMESH
    node.mesh = MDLMesh()
    node.mesh.dirt_texture = dirt_texture
    mdl.root.children.append(node)
    return mdl


for label, dt in (("control dirt_texture=1 (K1 path expected)", 1), ("probe dirt_texture=2 (K2 path expected)", 2)):
    try:
        data = bytes_mdl(build(dt))
        print(label, "-> OK, bytes_mdl length", len(data))
    except Exception as exc:
        print(label, "-> EXC", type(exc).__name__, str(exc)[:200])
        traceback.print_exc(limit=2)
