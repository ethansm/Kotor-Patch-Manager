"""G2.4 step 2: create dev work items for the G2-validated Gemini findings. Run from repo root.

usage: python3 emit_work_items.py [--dry-run]
Items: one per BG26.. row (from g2/bug_board_fragment.md order), test-gap/tooling clusters per owner (<= 4 files each),
one latent-hardening item, one verify-only item for runtime-unsure fids.
State: ready = free-gate owner (shared/scripts) with a failing test; hold = owner-gated mod or needs install/play/judgment.
"""
import csv, re, subprocess, sys
from collections import defaultdict
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
DRY = "--dry-run" in sys.argv
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
V = {r["fid"]: r for r in rd(D / "validated.tsv")}
FREE = {"shared", "scripts"}
SRC = "validated.tsv (.claude/reviews/gemini_20261014)"


def files_of(r, n=4):
    out = []
    for p in re.findall(r"([\w./-]+\.(?:h|cpp|py|java|ini|md|toml))", r["evidence"] + " " + r["failing_test"] + " " + r["file"]):
        if not Path(p).is_file():
            hit = [h for h in subprocess.run(["git", "ls-files", f"*/{Path(p).name}"], capture_output=True, text=True).stdout.split()
                   if Path(r["file"].split(",")[0]).parent.parts[:2] == Path(h).parent.parts[:2]]
            p = hit[0] if len(hit) == 1 else None
        if p and p not in out:
            out.append(p)
    return out[:n]


items = []
frag = (D / "g2" / "bug_board_fragment.md").read_text()
for bid, fid in re.findall(r"^\| (BG\d+) \| \*\*\w+\*\* `gem-([\w-]+)`", frag, re.M):
    r = V[fid]
    grp = re.search(rf"^### {bid} .*?\n- \*\*Where\*\*: [^\n]*?(?:same root cause: ([^)]+))?\)?$", frag, re.M | re.S)
    fids = grp.group(1) if grp and grp.group(1) else fid
    state = "ready" if r["owner_doc"] in FREE and r["failing_test"] not in ("", "-") else "hold"
    items.append(dict(title=f"{bid} {r['title'][:90]}", area=r["owner_doc"], files=files_of(r), state=state,
                      why=f"{r['impact']}/{r['severity']}: {r['why'][:160]} Fix: {r['fix_sketch'][:120]} Test: {r['failing_test'][:120]} Fids {fids}; {SRC}; doc 37 {bid}."))

clusters = defaultdict(list)
for r in V.values():
    if r["real"] == "yes" and r["impact"] in ("test-gap", "tooling"):
        clusters[r["owner_doc"]].append(r)
for owner, rows in sorted(clusters.items()):
    byfile = defaultdict(list)
    for r in rows:
        fs = files_of(r, 1)
        byfile[fs[0] if fs else r["file"]].append(r)
    keys = sorted(byfile)
    for i in range(0, len(keys), 4):
        chunk = keys[i:i + 4]
        rs = [r for k in chunk for r in byfile[k]]
        state = "ready" if owner in FREE and all(r["failing_test"] not in ("", "-") for r in rs) else "hold"
        items.append(dict(title=f"Gemini test/tooling gaps: {owner} ({', '.join(Path(k).name for k in chunk)})"[:120], area=owner, files=chunk, state=state,
                          why=f"{len(rs)} validated test-gap/tooling fids: " + "; ".join(f"{r['fid']}: {r['failing_test'][:90]}" for r in rs)[:600] + f" {SRC}."))

items.append(dict(title="Gemini latent hardening: GuiKit wr write-probe, CodePatch writeChecked order, TwoDA cells wrap", area="shared", state="hold",
                  files=["patch_manager_mods/_shared_src/GuiKit.h", "patch_manager_mods/_shared_src/CodePatch.h", "patch_manager_mods/action_hotbar_src/TwoDA.h", "patch_manager_mods/_shared_src/SafeRead.h"],
                  why="Real in code, unreachable today (g2/gate.md): X-T01 wr<T> uses probeRead (CO-2c-02, CO-2d-07, HB-1c-01, SH-3a-02); SH-3a-04 memcpy seen before commit check; "
                      "HB-1a-02 32-bit nrows*cols wrap; HB-1a-04 readAt base+off wrap. Owner decides whether hardening is worth a shared-lib change (compat_check.py required). " + SRC))
uns = [r for r in V.values() if r["real"] == "unsure"]
items.append(dict(title="Gemini runtime-unsure checks: " + ", ".join(r["fid"] for r in uns), area="shared", state="verify-only", files=[],
                  why="; ".join(f"{r['fid']}: {r['why'][:110]}" for r in uns) + f" Needs runtime logs or engine RE, not code reads. {SRC}."))

for it in items:
    cmd = [sys.executable, "scripts/wi_driver.py", "backlog", "add", "--title", it["title"], "--why", it["why"], "--area", it["area"],
           "--files", ";".join(it["files"]), "--origin", "gemini-20261014", "--state", it["state"]] + (["--dry-run"] if DRY else [])
    res = subprocess.run(cmd, capture_output=True, text=True)
    print(("OK  " if res.returncode == 0 else "FAIL"), it["state"], "|", it["title"][:80], "|", len(it["files"]), "files",
          "" if res.returncode == 0 else res.stderr.strip()[-200:])
    if res.returncode == 0 and not DRY:
        print("   ", res.stdout.strip().splitlines()[-1][:120] if res.stdout.strip() else "")
print(len(items), "items", "(dry-run)" if DRY else "")
