#!/usr/bin/env python3
"""Drift check for the headers shared by patch DLLs (shared-logging Phase 0, doc 19).

Edit the headers in patch_manager_mods/_shared_src/ (canonical). The framework fork must carry
byte-identical copies in Patches/Common/ (on every patch's include path), and Patches/_shared/<h>
must be a forwarding header to ../Common/<h> so the existing "../_shared/<h>" includes keep
building. EXTRA headers live elsewhere in the repo and are copied (no forwarder) to Patches/Common/<rel>
(tests/include/TestHarness.h -> Patches/Common/test/TestHarness.h, doc 24 T9b). Other copies (the level-up parity stub) are reported but not fixed: their owners sync them.

  check_shared_headers.py           report; exit 1 on any drift
  check_shared_headers.py --sync    copy canonical -> Patches/Common and rewrite the forwarding headers

The framework clone is $KPM_DIR, default ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager.
"""
import argparse
import os
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
CANONICAL = REPO / "patch_manager_mods" / "_shared_src"
KPM = Path(os.environ.get("KPM_DIR", Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"))
# Copies owned by other work; drift is reported, never overwritten.
FOREIGN = [REPO / "research/companion_levelup/tools/parity/stub/_shared"]
# (canonical file, path under Patches/Common): byte-identical copy only, no _shared forwarder.
EXTRA = [(REPO / "tests/include/TestHarness.h", "test/TestHarness.h")]


def forwarder(name: str) -> str:
    return (f"// Forwarding header (shared-logging Phase 0): the canonical copy is Patches/Common/{name}, which is on every\n"
            f"// patch's include path, so new code writes #include \"{name}\". Existing patches include \"../_shared/{name}\"\n"
            f"// and keep working unchanged; this file goes away once they have switched.\n"
            f"#pragma once\n"
            f"#include \"../Common/{name}\"\n")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sync", action="store_true", help="fix Patches/Common and Patches/_shared from the canonical copies")
    args = ap.parse_args()

    common, shims = KPM / "Patches" / "Common", KPM / "Patches" / "_shared"
    if not common.is_dir():
        print(f"ERROR: framework Common dir not found: {common} (set KPM_DIR)")
        return 2
    # .inc = generated tables included by a shared header (EffectIcons.h -> EffectIconTable.inc / DebuffIconTable.inc)
    headers = sorted(list(CANONICAL.glob("*.h")) + list(CANONICAL.glob("*.inc")))
    targets = [(src, ((common / src.name, "copy"), (shims / src.name, "forwarder"))) for src in headers]
    targets += [(src, ((common / rel, "copy"),)) for src, rel in EXTRA]
    drift = 0
    for src, dests in targets:
        want = src.read_bytes()
        for path, label in dests:
            expect = want if label == "copy" else forwarder(src.name).encode()
            ok = path.is_file() and path.read_bytes() == expect
            if not ok and args.sync:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(expect)
                print(f"SYNCED  {label:9} {path}")
            elif not ok:
                drift += 1
                print(f"DRIFT   {label:9} {path}" + ("" if path.exists() else " (missing)"))
            else:
                print(f"OK      {label:9} {path}")
        for d in FOREIGN if src.parent == CANONICAL else ():
            other = d / src.name
            if other.is_file() and other.read_bytes() != want:
                print(f"WARN    foreign   {other} differs (owner syncs it)")
    # Headers that exist only in the framework would be edited where nobody looks.
    for path in sorted(list(shims.glob("*.h")) + list(shims.glob("*.inc"))):
        if not (CANONICAL / path.name).is_file():
            drift += 1
            print(f"DRIFT   orphan    {path} has no canonical copy in {CANONICAL}")
    print(f"{len(targets)} shared headers, {drift} drifted")
    return 1 if drift else 0


if __name__ == "__main__":
    sys.exit(main())
