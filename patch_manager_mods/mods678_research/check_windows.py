#!/usr/bin/env python3
"""Validate proposed splice windows for Mods 6-8 against the full-program dump (lesson 166) and the live hooks.

For each window (name, start, length, function entry) it checks, from the dump's <fn>.asm:
  * the window starts and ends on whole instruction boundaries (length is the sum of whole instructions)
  * length >= 5
  * no relative branch/call (E8/E9/EB/7x/0F 8x) INSIDE the window (a re-executed copy would land wrong),
    unless the window is a skip_original_bytes=true window (bytes never re-run) -> reported as 'skip-only'
  * no branch from OUTSIDE the window targets an address strictly inside it (start is fine, it lands on our jump)
  * a RET may only be the LAST stolen instruction (the wrapper restores ESP exactly, so a trailing RET returns normally)
  * no overlap with any live hook window (live_hooks_2026-09-30e.tsv) or Mod 1's inventory hooks
Prints the original bytes as a TOML-ready list.

Usage: check_windows.py            (runs the built-in table of proposed windows)
"""
import re, sys, os

DUMP = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp")
HERE = os.path.dirname(os.path.abspath(__file__))
LIVE = os.path.join(HERE, "live_hooks_2026-09-30e.tsv")   # regenerated 2026-09-30 evening from the live patch_config.toml (82 hooks, same set as -b)

# (name, start, length, function entry, skip_original)
WINDOWS = [
    # Mod 6
    ("M6 tab click entry",         0x8A8CF0, 6,  0x8A8CF0, False),
    ("M6 backpack row buffer",     0x8A7E22, 79, 0x8A7BA0, True),
    ("M6 flush before finalise",   0x8A7E76, 22, 0x8A7BA0, False),
    ("M6 row-select epilogue (re-tint)", 0x8A8478, 6, 0x8A83D0, False),  # H6d 0.2.0: MOV ESP,EBP; POP EBP; RET 4 (RET is the last stolen insn)
    # Mod 7
    ("M7 SetMouseCursor entry",    0x7B3870, 7,  0x7B3870, False),
    ("M7 hilite placeable colour", 0x7AA747, 10, 0x7AA460, False),
    ("M7 client update tick",      0x7AC470, 5,  0x7AC470, False),
    # Mod 8 engine
    ("M8 hold-on-blocked-LOS",     0x6D83EA, 12, 0x6D7390, False),
    ("M8 retarget gate (IsPC)",    0x6D7C79, 6,  0x6D7390, False),   # moved from 0x6D7C73 (stolen MOV ECX,[EAX+..] ran with EAX=handler result)
    ("M8 ranged-stance rules local", 0x6D850F, 6, 0x6D7390, True),  # E-H3 0.2.0: MOV [EBP-0x174],EAX (skip-only; handler stores 11 for stance 16)
    ("M8 stance clamp",            0x77AA5E, 7,  0x77A9E0, False),
    ("M8 stance row fill",         0x77ACBC, 9,  0x77A9E0, False),   # re-sited 2026-09-30: was 15 bytes ending in MOV EAX,[EBP-0xF8] (EAX rule)
    ("M8 stance click",            0x77D8D4, 6,  0x77D890, False),   # re-sited 2026-09-30: was 14 bytes incl. MOV EAX,..; SUB EAX,0x410 (EAX rule)
    ("M8 HUD stance default branch", 0x74DBDD, 7, 0x74DA40, True),   # S-H4: MOV [ebp-0x1c],-1 (skip-only; handler writes 0x415 or -1)
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
        if name.startswith("M8") and not skip:
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
