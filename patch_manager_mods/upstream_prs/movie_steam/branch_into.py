#!/usr/bin/env python3
"""Usage: branch_into.py <steam|gog> <fn_start_hex> <fn_size> <hook_va_hex>:<len> ... ; lists branch targets landing strictly inside a hook's stolen bytes, and whether each hook VA is a decoded instruction start."""
import re, subprocess, sys
which, start, size = sys.argv[1], int(sys.argv[2],16), int(sys.argv[3])
out = subprocess.run(["python3","/home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/disasm_va.py",which,f"{start:X}","--len",str(size)],capture_output=True,text=True).stdout
insns = {}
targets = []
for line in out.splitlines():
    m = re.match(r"\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(.*)", line)
    if m:
        va = int(m.group(1),16); insns[va] = (len(m.group(2).split()), m.group(3))
        t = re.search(r"\b(?:j\w+|call)\s+0x([0-9a-f]+)\s*$", m.group(3))
        if t: targets.append((va, int(t.group(1),16)))
for h in sys.argv[4:]:
    a, n = h.split(":"); a = int(a,16); n = int(n)
    # walk instructions from a
    cur, ok = a, a in insns
    while ok and cur < a+n:
        cur += insns[cur][0]; ok = cur in insns or cur >= a+n
    inside = [f"{s:X}->{t:X}" for s,t in targets if a < t < a+n]
    print(f"hook {a:X}+{n}: starts_on_insn={a in insns} ends_on_boundary={cur==a+n} branch_into_interior={inside or 'none'}")
