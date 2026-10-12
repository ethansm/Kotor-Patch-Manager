"""K1b: K1 -> K2 conversion path on real K1 NPC models (scratch only).

Reads K1a inputs (read-only), reads each K1 .mdl with the patched reader,
writes with the forced-K2 MDLBinaryWriter into K1b, reads the output back,
and compares summaries via B4/semantic_check.py. Also records the geometry
layout token pair (file offset 12 and 16) and the supermodel name.
"""

import json
import struct
import sys
from pathlib import Path

SCRATCH = Path("/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2c3f74c1-88fa-49a4-8101-3e63d340c7c0/scratchpad")
K1A = SCRATCH / "K1a"
K1B = SCRATCH / "K1b"
B3 = SCRATCH / "B3"
B4 = SCRATCH / "B4"
NAMES = ("p_bastilah", "p_carthh", "n_darthmalak")

sys.path.insert(0, str(B4))
sys.path.insert(0, str(B3 / "pk_force"))

import pykotor  # noqa: E402
import semantic_check as sc  # noqa: E402
from pykotor.common.misc import Game  # noqa: E402
from pykotor.resource.formats.mdl.io_mdl import MDLBinaryWriter  # noqa: E402
from pykotor.resource.formats.mdl.mdl_auto import read_mdl  # noqa: E402


def tokens(data: bytes) -> tuple[int, int]:
    # Geometry header starts at file offset 12 (after the 12-byte MDL file header).
    return struct.unpack_from("<II", data, 12)


def exc_text(exc: BaseException) -> str:
    return f"{type(exc).__name__}: {str(exc)[:120]}"


def main() -> None:
    print(f"pykotor={pykotor.__file__}", flush=True)
    K1B.mkdir(parents=True, exist_ok=True)
    results = {}
    for name in NAMES:
        r: dict = {}
        results[name] = r
        print(f"== {name}", flush=True)
        k1_mdl = (K1A / f"{name}.mdl").read_bytes()
        k1_mdx = (K1A / f"{name}.mdx").read_bytes()
        r["k1_len"] = len(k1_mdl)
        r["k1_tokens"] = tokens(k1_mdl)

        # Step 1a: read K1 .mdl alone (no MDX), record the outcome.
        try:
            read_mdl(k1_mdl)
            r["step1_mdl_only"] = "OK"
        except Exception as exc:  # noqa: BLE001
            r["step1_mdl_only"] = exc_text(exc)
        print(f"  step1 mdl-only: {r['step1_mdl_only']}", flush=True)

        # Step 1b: read K1 .mdl with its MDX as the ext source (needed for vertex data).
        try:
            k1 = read_mdl(k1_mdl, source_ext=k1_mdx, size_ext=len(k1_mdx))
        except Exception as exc:  # noqa: BLE001
            r["step1_with_mdx"] = exc_text(exc)
            print(f"  step1 with-mdx: {r['step1_with_mdx']}", flush=True)
            continue
        r["step1_with_mdx"] = "OK"
        r["k1_supermodel"] = k1.supermodel
        orig = sc.summarize(k1)
        print(f"  step1 with-mdx: OK supermodel={k1.supermodel!r}", flush=True)

        # Step 2: write as K2 with the forced-K2 writer.
        out_mdl_path = K1B / f"out_{name}.mdl"
        out_mdx_path = K1B / f"out_{name}.mdx"
        try:
            writer = MDLBinaryWriter(k1, out_mdl_path, out_mdx_path)
            writer.force_game = Game.K2
            writer.write()
        except Exception as exc:  # noqa: BLE001
            r["step2_write"] = exc_text(exc)
            print(f"  step2 write: {r['step2_write']}", flush=True)
            continue
        out_mdl = out_mdl_path.read_bytes()
        out_mdx = out_mdx_path.read_bytes()
        r["out_len"] = len(out_mdl)
        r["out_mdx_len"] = len(out_mdx)
        r["out_tokens"] = tokens(out_mdl)
        r["k1_supermodel_bytes_in_output"] = bool(k1.supermodel) and k1.supermodel.encode("ascii") in out_mdl
        r["k1_supermodel_bytes_in_input"] = bool(k1.supermodel) and k1.supermodel.encode("ascii") in k1_mdl
        print(f"  step2 out_len={len(out_mdl)} out_tokens={r['out_tokens']}", flush=True)

        # Step 3: read the output back and compare summaries.
        try:
            back = read_mdl(out_mdl, source_ext=out_mdx, size_ext=len(out_mdx))
            k2 = sc.summarize(back)
        except Exception as exc:  # noqa: BLE001
            r["step3_readback"] = exc_text(exc)
            print(f"  step3 read-back: {r['step3_readback']}", flush=True)
            continue
        r["step3_readback"] = "OK"
        r["out_supermodel"] = back.supermodel
        row, ok = sc.make_row(name, orig, k2)
        r["row"] = row
        r["match"] = ok
        print(f"  step3 {row}", flush=True)

    out = K1B / "k1b_results.json"
    out.write_text(json.dumps(results, indent=1, default=str))
    print(f"RESULTS_JSON={out}", flush=True)


if __name__ == "__main__":
    main()
