"""precheck_import.py B<n>: before `import-ghidra --mode append`, list functions.csv rows whose name the branch DB holds
at another address (append would move it) or whose address the branch DB holds under another name (duplicate address)."""
import csv
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bdlib as bd  # noqa: E402

b = sys.argv[1]
p = bd.get_paths()
con = bd.connect_ro(Path(sys.argv[2]) if len(sys.argv) > 2 else p.branchdb)
by_name, by_addr = {}, {}
for c, f, a in con.execute("select class_name,function_name,address from functions"):
    a = bd.parse_addr(a) if isinstance(a, str) else a
    by_name[(c, f)] = a
    by_addr.setdefault(a, []).append("%s::%s" % (c, f))
rows = list(csv.DictReader(open(p.batches / b / "functions.csv")))
new = same = 0
bad = []
for r in rows:
    k, a = (r["class_name"], r["function_name"]), bd.parse_addr(r["address"])
    if k in by_name:
        if by_name[k] == a:
            same += 1
        else:
            bad.append("NAME MOVES  %s::%s branch 0x%08x -> 0x%08x" % (k[0], k[1], by_name[k], a))
    else:
        new += 1
    held = [n for n in by_addr.get(a, []) if n != "%s::%s" % k]
    if held:
        bad.append("ADDR HELD   0x%08x %s::%s, branch has %s" % (a, k[0], k[1], ",".join(held)))
dups = [a for a in {bd.parse_addr(r["address"]) for r in rows} if sum(bd.parse_addr(r["address"]) == a for r in rows) > 1]
bad += ["DUP IN CSV  0x%08x" % a for a in dups]
print("%s: %d rows, new %d, same-address confirms %d, problems %d" % (b, len(rows), new, same, len(bad)))
for x in bad:
    print("  " + x)
sys.exit(1 if bad else 0)
