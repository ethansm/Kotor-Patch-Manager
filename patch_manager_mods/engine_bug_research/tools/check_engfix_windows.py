#!/usr/bin/env python3
"""Window check for the doc-15 Phase 2 engine fixes (L1 dialogue letterbox, L4 wrap measure), read-only.

Reuses mods678_research/check_windows.py (instruction boundaries, rel branches, branch targets into the window,
overlap with the 09-30g snapshot AND the live patch_config.toml, TOML cross-check against the repo mirrors) with
its WINDOWS/OWNERS replaced by the two windows below, then adds a +-8 byte proximity check against every live
hook (check_windows.py only flags strict overlap).

L4 is a `simple` byte write (no JMP is placed), checked as a whole instruction for clarity; the < 5 bytes rule does
not apply to it, but the window is 6 bytes anyway.
Usage: check_engfix_windows.py
"""
import os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mods678_research"))
import check_windows as cw  # noqa: E402

cw.OWNERS = {"L1": ("k2-dialogue-letterbox-fix", "dialogue_letterbox_fix_src"),
             "L4": ("k2-wrap-measure-fix", "wrap_measure_fix_src")}
# drop owners whose mirror does not exist yet (before S2/S3 wrote them)
cw.OWNERS = {k: v for k, v in cw.OWNERS.items()
             if os.path.exists(os.path.join(cw.SRC, v[1], "kotor2-steam-aspyr.hooks.toml"))} or {"--": (None, None)}
cw.WINDOWS = [
    # L1: CSWGuiManager::SetScreenSize 0x411A80, after both W/H shorts and CALL 0x413DD0 (V7): MOV EDX,[EBP-0xC]; CMP [EDX+0x7C],0
    cw.W("L1 SetScreenSize post-store", 0x411AC1, 7, 0x411A80),
    # L4: CAurGUIStringInternal::WrapStrings 0x47AF30: FADD double ptr [0x0098c1d8] (0.25) -> [0x00987F08] (0.5)
    cw.W("L4 WrapStrings glyph FADD", 0x47B232, 6, 0x47AF30),
]

PAD = 8


def proximity():
    hooks = cw.live_windows() + cw.live_config_windows()[0]
    ok = True
    print(f"\n+-{PAD} byte proximity vs {len(hooks)} live hooks (snapshot + live config):")
    for name, start, length, *_ in cw.WINDOWS:
        own = cw.owner_of(name)[0]
        near = sorted({(a, n, pid) for a, n, pid, _ in hooks
                       if pid != own and a < start + length + PAD and start - PAD < a + n})
        print(f"  {name}: {start:#x}+{length}: " + ("none" if not near else
              ", ".join(f"{a:#x}+{n} ({pid})" for a, n, pid in near)))
        ok &= not near
    return ok


if __name__ == "__main__":
    cw.main()
    print("\nPROXIMITY OK" if proximity() else "\nPROXIMITY PROBLEMS")
