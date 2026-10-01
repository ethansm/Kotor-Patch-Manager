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
LIVE = os.path.join(HERE, "live_hooks_2026-09-30g.tsv")   # regenerated 2026-09-30 23:58 from the live patch_config.toml (120 hooks: sibling added level-up-override, perf-lab 0.3.0, load-hang-probe 1.4.0)

# (name, start, length, function entry, skip_original, consumed_exit or None)
WINDOWS = [
    # companion-ai (new patch id) -- design_ai_engine_fixes.md
    ("CA A1 approach gate end/retarget",   0x6D853E, 10, 0x6D7390, False, 0x6D7D2F),  # MOV [EBP-0x16C],0x7F000000
    ("CA P1 throttle interval",            0x56BDB4, 7,  0x56B8A0, False, None),      # CMP [EBP-0x28],0xFA0
    ("CA P1 throttle stamp store",         0x56BDE3, 6,  0x56B8A0, False, None),      # MOV [EAX+0x39C],EDX
    ("CA FM1 bump clear replace",          0x586F43, 10, 0x586B70, True,  None),      # PUSH 1; MOV ECX,[EBP+8]; CALL ClearAllActions
    ("CA FM4 snapshot FollowInfo",         0x57451F, 6,  0x574500, False, None),      # MOV ECX,[EAX+0x11AC]
    ("CA FM4 restore FollowInfo",          0x575183, 7,  0x574500, False, None),      # MOV EAX,[EBP+0xC]; PUSH EAX; MOV ECX,[EBP+8]
    ("CA FM4 alt: apply=0 (mode 2)",       0x57452C, 6,  0x574500, False, None),      # MOV ECX,[EBP-0x1D4] (args already pushed)
    ("CA A4 NULL spell row",               0x6D9E2A, 15, 0x6D9500, True,  0x6D9EA4),  # MOV EDX,[EBP-0x68]; MOV EAX,[EDX+0x154]; MOV [EBP-0x15C],EAX
    ("CA C1 restart idx (0xC)",            0x5868B9, 7,  0x5867E0, True,  None),      # MOV [EBP-0x10],0
    ("CA C1 restart idx (0xF)",            0x5868F1, 7,  0x5867E0, True,  None),
    ("CA C1 restart idx (0x2E)",           0x586929, 7,  0x5867E0, True,  None),
    ("CA A3 dead-target exit (opt)",       0x6D78B7, 7,  0x6D7390, False, None),      # MOV [EBP-0x3C],0 (lock-clear variant, no consumed exit)
    ("CA A3 alt consumed (opt)",           0x6D78B7, 7,  0x6D7390, False, 0x6D77AC),  # same window, EndAttackAction variant
    ("CA FM3 sentinel probe (opt)",        0x51DBE7, 6,  0x51D790, False, None),      # MOV EDX,[0xA1B488]
    # ranged-ai 0.3.0 change: E-H3 handler only (window unchanged, listed here to re-verify)
]
# live ranged-ai hooks (not in the TSV) and the parallel AI-trace design's planned windows
EXTRA_LIVE = [
    (0x6D83EA, 12, "ranged-ai:RangedAiHold"), (0x6D7C79, 6, "ranged-ai:RangedAiRetarget"),
    (0x6D850F, 6, "ranged-ai:RangedAiStanceRules"), (0x77AA5E, 7, "ranged-ai:StanceClamp"),
    (0x77ACBC, 9, "ranged-ai:StanceRow"), (0x77D8D4, 6, "ranged-ai:StanceClick"), (0x74DBDD, 7, "ranged-ai:HudStance"),
    (0x692C50, 10, "PLANNED ai-trace:T1"), (0x68F301, 7, "PLANNED ai-trace:T2"), (0x6F62F2, 7, "PLANNED ai-trace:T3"),
    (0x690BC8, 7, "PLANNED ai-trace:T4"), (0x6D740F, 10, "PLANNED ai-trace:E1"), (0x6D8BE2, 5, "PLANNED ai-trace:E1b"),
    (0x569A08, 6, "PLANNED ai-trace:E2"), (0x56BDBD, 8, "PLANNED ai-trace:E3"), (0x586F43, 5, "PLANNED ai-trace:E4"),
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
    return w + [(a, n, l) for a, n, l in EXTRA_LIVE]


def eax_state(ins, at, limit=60):
    """Follow fall-through and Jcc/JMP targets from `at` (within the function): 'dead' only if on EVERY path EAX is
    fully written (MOV/MOVZX/LEA/POP/XOR EAX,EAX) or clobbered by a CALL/RET before any read."""
    pos = {a: i for i, (a, b, t) in enumerate(ins)}
    seen, verdicts = set(), []
    def walk(addr, depth):
        while True:
            if addr not in pos: verdicts.append(f"unknown ({addr:#x} outside fn)"); return
            if addr in seen: return
            seen.add(addr)
            i = pos[addr]; a, b, t = ins[i]
            m = re.match(r"(\w+)\s*(.*)", t); op, args = m.group(1), m.group(2)
            if op in ("CALL", "RET"):
                if re.search(r"\bEAX\b", args): verdicts.append(f"LIVE (read by {a:#x} {t})")
                return
            dst, _, src = args.partition(",")
            rd_ = re.search(r"\b(EAX|AX|AL|AH)\b", src) or (re.search(r"\b(EAX|AX|AL|AH)\b", dst) and "[" in dst) \
                  or (op not in ("MOV", "MOVZX", "MOVSX", "LEA", "POP", "XOR") and re.search(r"\b(EAX|AX|AL|AH)\b", dst))
            if op == "XOR" and dst.strip() == "EAX" and src.strip() != "EAX": rd_ = True
            if rd_: verdicts.append(f"LIVE (read by {a:#x} {t})"); return
            if dst.strip() == "EAX" and op in ("MOV", "MOVZX", "MOVSX", "LEA", "POP", "XOR"): return
            mj = re.match(r"(J\w+)\s+0x([0-9a-f]+)", t)
            if mj:
                if depth > 12: verdicts.append("unknown (depth)"); return
                walk(int(mj.group(2), 16), depth + 1)
                if mj.group(1) == "JMP": return
            if i + 1 >= len(ins): verdicts.append("unknown (end)"); return
            addr = ins[i + 1][0]
    walk(at, 0)
    if not verdicts: return "dead on all paths"
    return "; ".join(verdicts)


def is_rel(b, txt):
    return bool(re.match(r"(J\w+|CALL|LOOP\w*)\s+0x", txt))


def main():
    live = live_windows()
    ok = True
    for name, start, length, entry, skip, cexit in WINDOWS:
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
            if la < end and start < la + ln:
                problems.append(f"overlaps live hook {la:#x}+{ln} ({lname})")
        if cexit is not None and not skip:
            eaxuse = [f"{a:#x} {t}" for a, b, t in inside if re.search(r"\b(EAX|AX|AL|AH)\b", t)]
            if eaxuse:
                problems.append("consumed hook whose stolen bytes touch EAX: " + "; ".join(eaxuse))
        if cexit is not None:
            if cexit not in addrs:
                problems.append(f"consumed exit {cexit:#x} is not an instruction start in fn {entry:#x}")
            for label, at in (("resume", end), ("consumed exit", cexit)):
                v = eax_state(ins, at)
                print(f"        EAX at {label} {at:#x}: {v}")
                if not v.startswith("dead"):
                    problems.append(f"EAX may be live at {label} {at:#x} (EAX = handler result there)")
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
