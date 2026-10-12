"""B3: read real TSL MDL, force the K2 writer, compare output bytes to input."""

import sys
from pathlib import Path

B3 = Path("/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2c3f74c1-88fa-49a4-8101-3e63d340c7c0/scratchpad/B3")
GAME_DIR = "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
sys.path.insert(0, str(B3 / "pk_force"))

import pykotor  # noqa: E402
from pykotor.common.misc import Game  # noqa: E402
from pykotor.extract.installation import Installation  # noqa: E402
from pykotor.resource.formats.mdl.io_mdl import MDLBinaryWriter  # noqa: E402
from pykotor.resource.formats.mdl.mdl_auto import read_mdl  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402


def first_diff(a: bytes, b: bytes) -> int | None:
    for i, (x, y) in enumerate(zip(a, b)):
        if x != y:
            return i
    if len(a) != len(b):
        return min(len(a), len(b))
    return None


print(f"pykotor={pykotor.__file__}")
inst = Installation(GAME_DIR)
for name in ("p_bastilah", "p_carthh", "p_hk47"):
    mdl_res = inst.resource(name, ResourceType.MDL)
    mdx_res = inst.resource(name, ResourceType.MDX)
    if mdl_res is None or mdx_res is None:
        print(f"{name}: MISSING mdl={mdl_res is not None} mdx={mdx_res is not None}")
        continue
    in_mdl: bytes = mdl_res.data
    in_mdx: bytes = mdx_res.data
    (B3 / f"in_{name}.mdl").write_bytes(in_mdl)
    (B3 / f"in_{name}.mdx").write_bytes(in_mdx)

    mdl = read_mdl(in_mdl, source_ext=in_mdx, size_ext=len(in_mdx))
    out_mdl_path = B3 / f"out_{name}.mdl"
    out_mdx_path = B3 / f"out_{name}.mdx"
    writer = MDLBinaryWriter(mdl, out_mdl_path, out_mdx_path)
    writer.force_game = Game.K2
    writer.write()

    out_mdl = out_mdl_path.read_bytes()
    out_mdx = out_mdx_path.read_bytes()
    d_mdl = first_diff(in_mdl, out_mdl)
    d_mdx = first_diff(in_mdx, out_mdx)
    print(
        f"{name}: mdl in_len={len(in_mdl)} out_len={len(out_mdl)} "
        f"identical={'yes' if d_mdl is None else 'no'} first_diff={d_mdl} | "
        f"mdx in_len={len(in_mdx)} out_len={len(out_mdx)} "
        f"identical={'yes' if d_mdx is None else 'no'} first_diff={d_mdx}"
    )
