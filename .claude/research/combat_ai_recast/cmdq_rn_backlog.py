"""O1: the combat-round node list (QS rn=) as a cast backlog. Per actor: spell nodes (sub 9) pending in rn at
each QS point, a histogram of the backlog depth at cmd0 (script queue time), and whether a CLR of that actor
ever shrinks rn (ClearAllActions vs round nodes)."""
import re, os, sys
from collections import Counter, defaultdict
G = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/")
path = sys.argv[1] if len(sys.argv) > 1 else G + "cmdq-probe_log.txt"
qs = re.compile(r"T=([\d.]+) .*QS at=(\w+) actor=(\w+) n=(\d+) q=(\S*) rn=(\S*) cur=(\S+)")
clr = re.compile(r"T=([\d.]+) .*CLR victim=(\w+) caller=(\w+)")
depth = defaultdict(Counter); last = {}; clr_eff = Counter(); pend_clr = {}
for ln in open(path, errors="replace"):
    m = qs.search(ln)
    if m:
        a, at = m[3], m[2]
        nodes = [x for x in m[6].split(",") if x]
        sp = [x for x in nodes if x.startswith("9:")]
        depth[a, at][len(sp)] += 1
        if a in pend_clr:
            before = pend_clr.pop(a)
            clr_eff["shrunk" if len(sp) < before else "same/grew"] += 1
        last[a] = len(sp)
        continue
    m = clr.search(ln)
    if m and last.get(m[2], 0) > 0:
        pend_clr[m[2]] = last[m[2]]
for (a, at), c in sorted(depth.items()):
    print(a, at, "spell nodes pending:", dict(sorted(c.items())))
print("CLR of an actor with pending spell nodes -> next QS:", dict(clr_eff))
