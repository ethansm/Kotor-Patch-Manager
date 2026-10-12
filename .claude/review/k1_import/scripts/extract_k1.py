import sys
from pathlib import Path

from pykotor.extract.installation import Installation
from pykotor.resource.formats.mdl.mdl_auto import read_mdl
from pykotor.resource.type import ResourceType

K1_PATH = "/home/ethansm/.local/share/Steam/steamapps/common/swkotor"
OUT = Path("/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2c3f74c1-88fa-49a4-8101-3e63d340c7c0/scratchpad/K1a")

inst = Installation(K1_PATH)
print("GAME", inst.game())

for name in sys.argv[1:]:
    mdl_res = inst.resource(name, ResourceType.MDL)
    mdx_res = inst.resource(name, ResourceType.MDX)
    if mdl_res is None:
        print("MISSING_MDL", name)
        continue
    mdl_b = mdl_res.data
    mdx_b = mdx_res.data if mdx_res is not None else None
    if mdx_b is None:
        print("MISSING_MDX", name)
        mdx_b = b""
    (OUT / f"{name}.mdl").write_bytes(mdl_b)
    (OUT / f"{name}.mdx").write_bytes(mdx_b)
    print("SAVED", name, "mdl_bytes", len(mdl_b), "mdx_bytes", len(mdx_b))
    try:
        mdl = read_mdl(mdl_b, 0, None, mdx_b, 0, 0)
        print("PARSED", name, "supermodel_raw", repr(mdl.supermodel), "root", repr(mdl.root.name))
    except Exception as exc:  # noqa: BLE001
        print("PARSE_FAIL", name, type(exc).__name__, exc)
sys.stdout.flush()
