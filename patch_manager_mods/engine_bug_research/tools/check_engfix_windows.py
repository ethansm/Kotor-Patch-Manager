#!/usr/bin/env python3
"""Window check for the doc-15 Phase 2 engine fixes (L1 dialogue letterbox, L4 wrap measure), read-only.

Reuses mods678_research/check_windows.py (instruction boundaries, rel branches, branch targets into the window,
overlap with the 09-30g snapshot AND the live patch_config.toml, TOML cross-check against the repo mirrors) with
its WINDOWS/OWNERS replaced by the two windows below, then adds a +-8 byte proximity check against every live
hook (check_windows.py only flags strict overlap).

L4 is a `simple` byte write (no JMP is placed), checked as a whole instruction for clarity; the < 5 bytes rule does
not apply to it, but the window is 6 bytes anyway.
2026-10-03: added the five log-only probe windows W1-W5 of k2-dialogue-letterbox-fix 1.0.1-probe (V8b (c)). The owner of the
"L1" prefix is now the probe mirror dialogue_letterbox_fix_probe_src (it carries the three 1.0.0 L1 hooks unchanged + the five
probe hooks, all with the same patch id; the live 1.0.0 hooks of that id are therefore never counted as "near"). A final block
checks that the 1.0.0 mirror dialogue_letterbox_fix_src is a hook-for-hook subset of the probe mirror.
Usage: check_engfix_windows.py
"""
import os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "..", "mods678_research"))
import check_windows as cw  # noqa: E402

cw.OWNERS = {"L1": ("k2-dialogue-letterbox-fix", "dialogue_letterbox_fix_probe_src"),
             "L4": ("k2-wrap-measure-fix", "wrap_measure_fix_src")}
# drop owners whose mirror does not exist yet (before S2/S3 wrote them)
cw.OWNERS = {k: v for k, v in cw.OWNERS.items()
             if os.path.exists(os.path.join(cw.SRC, v[1], "kotor2-steam-aspyr.hooks.toml"))} or {"--": (None, None)}
cw.WINDOWS = [
    # L1: CSWGuiManager::SetScreenSize 0x411A80, after both W/H shorts and CALL 0x413DD0 (V7): MOV EDX,[EBP-0xC]; CMP [EDX+0x7C],0
    cw.W("L1 SetScreenSize post-store", 0x411AC1, 7, 0x411A80),
    # L1 log-only: last store of CSWGuiDialogLetterbox::SetTop / SetBottom (V1 (c)), ECX = this: MOV [ECX+0x88],1 / ,2
    cw.W("L1 SetTop rect log", 0x8BAD7C, 10, 0x8BACA0),
    cw.W("L1 SetBottom rect log", 0x8BAF01, 10, 0x8BADA0),
    # L4: CAurGUIStringInternal::WrapStrings 0x47AF30: FADD double ptr [0x0098c1d8] (0.25) -> [0x00987F08] (0.5)
    cw.W("L4 WrapStrings glyph FADD", 0x47B232, 6, 0x47AF30),
    # L1 probe (1.0.1-probe, V8b (c)); owner = probe mirror (prefix "L1"). Function starts: letterbox Draw 0x8BB260, base panel Draw 0x40EC90,
    # CGuiInGame::SetGlobalDialogState 0x7CEB80.
    cw.W("L1 probe W1 Draw top after SetExtent", 0x8BB461, 6, 0x8BB260),      # MOV EAX,[EBP-0x34] ; MOV ECX,[EAX+0x10]
    cw.W("L1 probe W2 Draw bottom after SetExtent", 0x8BB52B, 6, 0x8BB260),   # MOV ECX,[EBP-0x34] ; MOV EDX,[ECX+8]
    cw.W("L1 probe W3 Draw pre-CALL base Draw", 0x8BB538, 9, 0x8BB260),       # FLD [EBP+8] ; FSTP [ESP] ; MOV ECX,[EBP-0x34] (x87 pair)
    cw.W("L1 probe W4 base Draw after fill", 0x40ED9F, 5, 0x40EC90),          # ADD ESP,0x1C ; TEST EAX,EAX (HOT)
    cw.W("L1 probe W5 SetGlobalDialogState arg", 0x7CEB95, 7, 0x7CEB80),      # CMP [EBP+8],1 ; SETZ DL
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
    # the 1.0.0 mirror must be a hook-for-hook subset of the probe mirror (same address, bytes, function, parameters, exclusions)
    import tomllib
    def hooks(d):
        t = tomllib.load(open(os.path.join(cw.SRC, d, "kotor2-steam-aspyr.hooks.toml"), "rb"))
        return {int(h["address"]): (h.get("function"), tuple(h["original_bytes"]), tuple((q["source"], q["type"]) for q in h.get("parameters", [])),
                                    tuple(h.get("exclude_from_restore", []))) for h in t["hooks"]}
    base, probe = hooks("dialogue_letterbox_fix_src"), hooks("dialogue_letterbox_fix_probe_src")
    bad = [f"{a:#x}" for a, v in base.items() if probe.get(a) != v]
    print(f"\n1.0.0 mirror ({len(base)} hooks) subset of probe mirror ({len(probe)} hooks): " + ("OK" if not bad else "MISMATCH " + ", ".join(bad)))
