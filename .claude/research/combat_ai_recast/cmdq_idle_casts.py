"""Per actor: spell CMDs (queue real=0 / build real=1), how many are the invalid talent id -1 (vanilla
TalentSpell(-1) wasted round), and the -1 build bursts (wall-clock span of back-to-back -1 builds)."""
import re, os, sys
from collections import defaultdict
G = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/")
path = sys.argv[1] if len(sys.argv) > 1 else G + "cmdq-probe_log.txt"
rx = re.compile(r"T=([\d.]+) .*CMD kind=(\w+) actor=(\w+) id=(\w+) tgt=(\w+) real=(\d)")
st = defaultdict(lambda: {"q": 0, "b": 0, "q-1": 0, "b-1": 0, "other": 0})
runs = defaultdict(list)
for ln in open(path, errors="replace"):
    m = rx.search(ln)
    if not m:
        continue
    ts, kind, a, sid, real = float(m[1]) / 1000, m[2], m[3], int(m[4], 16), m[6]
    s = st[a]
    if kind != "spell":
        s["other"] += 1
        continue
    k = "q" if real == "0" else "b"
    s[k] += 1
    if sid == 0xffffffff:
        s[k + "-1"] += 1
        if k == "b":
            r = runs[a]
            if r and ts - r[-1][1] < 8.0:
                r[-1] = (r[-1][0], ts, r[-1][2] + 1)
            else:
                r.append((ts, ts, 1))
for a, s in sorted(st.items(), key=lambda x: -x[1]["b"]):
    print(a, s, "bursts(start s, span s, n):", [(round(x, 1), round(y - x, 1), n) for x, y, n in runs[a] if n > 1][:6])
