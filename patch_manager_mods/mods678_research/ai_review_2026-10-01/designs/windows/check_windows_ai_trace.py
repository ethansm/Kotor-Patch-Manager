#!/usr/bin/env python3
"""Validate proposed splice windows for Mods 6-8 against the full-program dump (lesson 166) and the live hooks.

For each window (name, start, length, function entry) it checks, from the dump's <fn>.asm:
  * the window starts and ends on whole instruction boundaries (length is the sum of whole instructions)
  * length >= 5
  * no relative branch/call (E8/E9/EB/7x/0F 8x) INSIDE the window (a re-executed copy would land wrong),
    unless the window is a skip_original_bytes=true window (bytes never re-run) -> reported as 'skip-only'
  * no branch from OUTSIDE the window targets an address strictly inside it (start is fine, it lands on our jump)
  * a RET may only be the LAST stolen instruction (the wrapper restores ESP exactly, so a trailing RET returns normally)
  * no overlap with any live hook window (live_hooks_2026-09-30g.tsv) or Mod 1's inventory hooks
Prints the original bytes as a TOML-ready list.

Usage: check_windows.py            (runs the built-in table of proposed windows)
"""
import re, sys, os

DUMP = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp")
HERE = "/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/mods678_research"
LIVE = os.path.join(HERE, "live_hooks_2026-09-30g.tsv")
# ranged-ai 0.2.0 windows (not in the live TSV yet; must not be overlapped either)
EXTRA_LIVE = [(0x6D83EA,12,"ranged-ai:RangedAiHold"),(0x6D7C79,6,"ranged-ai:RangedAiRetarget"),(0x6D850F,6,"ranged-ai:RangedAiStanceRules"),
              (0x77AA5E,7,"ranged-ai:RangedAiStanceClamp"),(0x77ACBC,9,"ranged-ai:RangedAiStanceRow"),(0x77D8D4,6,"ranged-ai:RangedAiStanceClick"),(0x74DBDD,7,"ranged-ai:RangedAiHudStance")]
CONSUMED = {"T2 PrintString capture", "X2 A1 fix (review option 1)"}   # regenerated 2026-09-30 23:58 from the live patch_config.toml (120 hooks: sibling added level-up-override, perf-lab 0.3.0, load-hang-probe 1.4.0)

# (name, start, length, function entry, skip_original)
WINDOWS = [
    # AI trace: VM command handlers (perf-lab owns 0x668FD0 ExecuteCommand; these are per-command sites)
    ("T1 ShipBuild value",          0x692C50, 10, 0x692C40, False),  # MOV [0xA7E114],1 ; handler overwrites [ebp-4] (pushed by 0x692C5A)
    ("T2 PrintString capture",      0x68F301, 7,  0x68F260, False),  # MOV [EBP-0x10],0 (argc<2 path, all 512 vanilla calls); consumed exit 0x68F39F
    ("T3 AurPostString capture",    0x6F62F2, 7,  0x6F6200, False),  # MOV [EBP-0x34],0 after the 4 pops succeeded
    ("T4 SendMessageToPC (optional)", 0x690BC8, 7, 0x690A90, False), # MOV [EBP-0x38],0 (success tail; not used by any AI script)
    # AI trace: engine log points (review_ai_engine.md)
    ("E1 A1 attack entry (+0x570 reset)", 0x6D740F, 10, 0x6D7390, False),  # MOV [EAX+0x570],0xBB8, EAX = attacker
    ("E1b A1 approach-branch exit", 0x6D8BE2, 5,  0x6D7390, False),  # MOV EAX,[0x9944B8] (return 2)
    ("E2 A2 slot-6 fallback fires", 0x569A08, 6,  0x569320, False),  # MOV ECX,[EBP-0x194]; +0x570<=0, alive
    ("E3 P1 block-2 UpdatePerception(1)", 0x56BDBD, 8, 0x56B8A0, False),  # PUSH 1; MOV ECX,[EBP-0xD4]
    ("E4 FM1 bump ClearAllActions", 0x586F43, 5,  0x586B70, False),  # PUSH 1; MOV ECX,[EBP+8]
    # Future companion-ai fix windows (NOT part of ai-trace v0.1; validated so the shared-DLL plan is sound)
    ("X1 FM1 fix variant (skip-only, incl. CALL)", 0x586F43, 10, 0x586B70, True),  # PUSH 1; MOV ECX,[EBP+8]; CALL 0x541080
    ("X2 A1 fix (review option 1)", 0x6D84FC, 6, 0x6D7390, False),   # MOV [EBP-0x384],ECX ; consumed exit 0x6D83FF
    ("X3 P1 fix (store stamp)",     0x56BDE3, 6, 0x56B8A0, False),   # MOV [EAX+0x39C],EDX
]


def load_fn(entry):
    bb = "%02x" % ((entry >> 16) & 0xff)
    path = os.path.join(DUMP, bb, "%08x.asm" % entry)
    ins = []
    for l in open(path).read().splitlines():
        m = re.match(r"([0-9a-f]{8})\s+([0-9a-f]+)\s+(.*)", l)
        if m:
            ins.append((int(m.group(1), 16), bytes.fromhex(m.group(2)), m.group(3)))
    return ins


def live_windows():
    w = []
    for l in open(LIVE):
        a, n, ty, f, p = l.rstrip("\n").split("\t")
        w.append((int(a, 16), int(n), p + ":" + f))
    return w


def is_rel(b, txt):
    return bool(re.match(r"(J\w+|CALL|LOOP\w*)\s+0x", txt))


def main():
    live = live_windows()
    ok = True
    live = live + EXTRA_LIVE
    for name, start, length, entry, skip in WINDOWS:
        ins = load_fn(entry)
        addrs = [a for a, _, _ in ins]
        if start not in addrs:
            print(f"[FAIL] {name}: {start:#x} is not an instruction start in fn {entry:#x}")
            ok = False
            continue
        if length == 0:
            continue
        # walk whole instructions
        total, inside = 0, []
        for a, b, t in ins:
            if a >= start and total < length:
                inside.append((a, b, t))
                total += len(b)
        end = start + total
        problems = []
        if total != length:
            problems.append(f"length {length} is not on an instruction boundary (whole insns = {total})")
        if length < 5:
            problems.append("window < 5 bytes")
        rets = [i for i, (a, b, t) in enumerate(inside) if t.startswith("RET")]
        if rets and rets != [len(inside) - 1]:
            problems.append("RET inside the window is not the last stolen instruction")
        rel_in = [(a, t) for a, b, t in inside if is_rel(b, t)]
        if rel_in and not skip:
            problems.append("relative branch inside window: " + ", ".join(f"{a:#x} {t}" for a, t in rel_in))
        # external targets into the window interior
        for a, b, t in ins:
            m = re.match(r"(J\w+|LOOP\w*)\s+0x([0-9a-f]+)", t)
            if m:
                tg = int(m.group(2), 16)
                if start < tg < end and not (start <= a < end):
                    problems.append(f"external branch {a:#x} -> {tg:#x} lands inside window")
        for la, ln, lname in live:
            if name.startswith("M6") and "inventory-sort-filter" in lname:
                continue  # Mod 6's own installed hooks (window == hook)
            if la < end and start < la + ln:
                problems.append(f"overlaps live hook {la:#x}+{ln} ({lname})")
        if name in CONSUMED and not skip:
            eaxuse = [f"{a:#x} {t}" for a, b, t in inside if re.search(r"\b(EAX|AX|AL|AH)\b", t)]
            if eaxuse:
                print(f"[WARN] {name}: stolen bytes touch EAX (framework runs them after the handler with EAX = handler result; "
                      f"unsafe for consumed_exit_address hooks): " + "; ".join(eaxuse))
        blob = b"".join(b for _, b, _ in inside)
        status = "OK  " if not problems else "FAIL"
        if problems:
            ok = False
        print(f"[{status}] {name}: {start:#010x} len {total}{' (skip-only)' if skip else ''}  bytes = [{', '.join('0x%02X' % x for x in blob)}]")
        for p in problems:
            print("        ! " + p)
        for a, b, t in inside:
            print(f"        {a:08x}  {b.hex():22} {t}")
    print("\nALL OK" if ok else "\nPROBLEMS FOUND")


if __name__ == "__main__":
    main()
