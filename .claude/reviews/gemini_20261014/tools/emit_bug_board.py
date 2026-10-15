"""G2.4 step 1: emit g2/bug_board_fragment.md (doc 37 table rows BG26.. + detail sections) from validated.tsv. Run from repo root.

One row per real=yes defect whose impact is not test-gap/tooling; fids sharing one root cause (GROUPS) share a row.
Severity is the skeptic's (high|med|low -> high|medium|low); see gate.md for the Opus-checked rows.
"""
import csv, re
from pathlib import Path

D = Path(".claude/reviews/gemini_20261014")
FIRST = 26
DATE = "2026-10-14"
rd = lambda p: list(csv.DictReader(open(p, newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
V = rd(D / "validated.tsv")
GROUPS = [["CO-2d-09", "QH-2a-11", "SH-3a-06"], ["SH-3a-07", "HB-1c-02"]]  # EmbedTex give-up log; GuiKit label leak
OWNER = {"queue-hud": "queue-hud (doc 30)", "selfheal": "selfheal (doc 27)", "hotbar": "hotbar (doc 07/31)", "k1": "k1 (doc 38)",
         "holster": "holster (doc 42)", "npc": "npc (doc 34/40)", "perf-lab": "perf-lab (doc 10)", "companion-ai": "companion-ai (owner session)",
         "shared": "-", "scripts": "-"}
SEV = {"high": "high", "med": "medium", "low": "low"}
ORDER = {"high": 0, "med": 1, "low": 2}

defects = [r for r in V if r["real"] == "yes" and r["impact"] not in ("test-gap", "tooling")]
by = {r["fid"]: r for r in defects}
rows, used = [], set()
for g in GROUPS:
    members = [by[f] for f in g if f in by]
    if members:
        rows.append(members)
        used |= {m["fid"] for m in members}
rows += [[r] for r in defects if r["fid"] not in used]
rows.sort(key=lambda m: (ORDER[m[0]["severity"]], m[0]["owner_doc"], m[0]["fid"]))


def first_loc(ev):
    m = re.search(r"([\w./-]+\.\w+):(\d+)", ev)
    return (m.group(1), int(m.group(2))) if m else (None, None)


def quote(path, line):
    try:
        return Path(path).read_text(errors="replace").splitlines()[line - 1].strip()[:160]
    except (OSError, IndexError, TypeError):
        return ""


def one_line(s, n=170):
    s = re.sub(r"\s+", " ", s).strip()
    return s if len(s) <= n else s[: n - 3] + "..."


table, details, summary = [], [], []
for i, members in enumerate(rows):
    lead = members[0]
    bid = f"BG{FIRST + i}"
    path, line = first_loc(lead["evidence"])
    loc = f"{path}:{line}" if path else lead["file"]
    fids = ", ".join(m["fid"] for m in members)
    sev = SEV[lead["severity"]]
    claim = one_line(lead["title"] + "; " + lead["why"])
    owner = OWNER.get(lead["owner_doc"], lead["owner_doc"])
    shared = path and "_shared_src" in path
    nxt = one_line(("Write the failing test: " + lead["failing_test"] if lead["failing_test"] not in ("", "-") else "Fix: " + lead["fix_sketch"])
                   + (" (compat_check.py required)" if shared else ""), 200)
    table.append(f"| {bid} | **{sev}** `gem-{lead['fid']}` {loc} ({lead['impact']}): {claim} | - | {owner} | OPEN | | {nxt} |")
    summary.append((bid, sev, fids, lead["owner_doc"], one_line(lead["title"], 90)))
    ev_lines = []
    for m in members:
        for p, ln in re.findall(r"([\w./-]+\.\w+):(\d+)", m["evidence"])[:3]:
            q = quote(p, int(ln))
            ev_lines.append(f"  - `{p}:{ln}` `{q}`" if q else f"  - `{p}:{ln}`")
    details += [
        f"### {bid} — gem-{lead['fid']} ({sev}, G2-validated, {lead['category']}/{lead['impact']})",
        f"- **Where**: `{loc}`" + (f" (same root cause: {fids})" if len(members) > 1 else ""),
        f"- **Claim** (Gemini 2026-10-14, untrusted until checked): {lead['title']}",
        f"- **Still real ({DATE} G2)**: G1 {lead['g1']} -> v2 {lead['v2']} -> skeptic real=yes, {lead['impact']}, {sev}. {one_line(lead['why'], 300)}"
        + (" Opus gate read the lines (g2/gate.md)." if lead["gate"] == "opus" else ""),
        "- **Audit evidence**:", *dict.fromkeys(ev_lines),
        f"- **Check steps**: {lead['failing_test'] if lead['failing_test'] != '-' else 'Re-read the evidence lines on HEAD; reproduce from the claim.'}"
        + (" Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change." if shared else ""),
        f"- **Investigate**: fix sketch: {lead['fix_sketch'].rstrip('.')}. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` ({fids}).",
        "- **Resolution**:", ""]

out = ["<!-- G2 fragment: table rows go under '## Batch table' after BG25; detail sections go at the end of the doc. -->", "## Table rows", "", *table,
       "", "## Detail sections", "", *details]
(D / "g2" / "bug_board_fragment.md").write_text("\n".join(out) + "\n")
for s in summary:
    print(*s, sep=" | ")
print(len(summary), "rows")
