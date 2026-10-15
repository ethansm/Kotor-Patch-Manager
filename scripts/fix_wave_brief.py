#!/usr/bin/env python3
"""Fix-wave brief writer: the `brief` subcommand. Writes one batch of a fix wave and adds its Haiku variant units.

usage: python3 scripts/fix_wave_brief.py brief <W> --batch <B> [--dry-run] [--force] [--waves-root DIR] [--repo-root DIR]

Inputs (WR = <repo>/.claude/waves unless --waves-root):
  WR/<W>/batches.tsv  columns batch, wave_id, units, agents, test_usd, ids (ids ';'-joined)
  WR/<W>/items.tsv    one fix unit per row (id, kind, size, group, after, fids, write, tests, mods, entries, anchors,
                      constraint, context, doc; tab-separated, no quoting)
  WR/<W>/plan.json    optional findings: {"findings": path, "cols": [...]} (first col = key column)
Templates: <repo>/.claude/skills/variant-fix-wave/{variants.md, consolidate.md, review.md}.

Writes WR/<W>-<B>/ with items.tsv, units.tsv, <id>/pack.md, <id>/consolidate.md, <id>/review.brief.md, and adds one
Haiku recon unit per variant through `scripts/wave_tools.py new|add`. Renders everything in memory first, so input
errors write nothing. --dry-run writes nothing at all.

Exit codes: 0 written (or the batch dir already exists: prints `exists: <W>-<B> (use --force)`, no write),
2 refused (bad input, missing file, or wave_tools new failed), 3 some wave_tools adds were rejected (listed by uid and
label; the others were added; fix the cause and rerun with --force).
"""
import argparse
import csv
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parent
TEMPLATE_DIR = Path(".claude") / "skills" / "variant-fix-wave"
TEMPLATES = ("variants.md", "consolidate.md", "review.md")
SCRATCH_BASE = "/tmp/claude-1000"
MOD = "patch_manager_mods/"
NAME_RE = r"[A-Za-z0-9][A-Za-z0-9._-]*"
VARIANT_LABELS = ("A", "B", "C", "T1", "T2")
PROOF = {
    "bug": "red-on-BASE (revert only the production hunk(s), keep the tests, each new test FAILS)",
    "gap": "mutation kill (apply the named mutation to the code under test; each new test FAILS under it; for a production change "
           "in this unit, also red-on-BASE)",
}


class Refused(Exception):
    """Bad input or a missing file: the message goes to stderr and the run exits 2."""


def ent(p):
    """Repo-relative entry path: patch_manager_mods/ is prepended unless the path already starts with it or scripts/."""
    return p if p.startswith(MOD) or p.startswith("scripts/") else MOD + p


def lst(s):
    """Split a ';'-list cell, dropping empty items."""
    return [x.strip() for x in (s or "").split(";") if x.strip()]


def variants(kind, size):
    """Variant labels for one fix unit: bug S -> A; bug M/L -> A,B,C; gap S/M -> T2; gap L -> T1,T2 (empty size = M)."""
    table = {("bug", "S"): ["A"], ("bug", "M"): ["A", "B", "C"], ("bug", "L"): ["A", "B", "C"],
             ("gap", "S"): ["T2"], ("gap", "M"): ["T2"], ("gap", "L"): ["T1", "T2"]}
    key = (kind, size or "M")
    if key not in table:
        raise ValueError(f"kind {kind!r} size {size!r}")
    return list(table[key])


def read_tsv(path):
    """(header, rows) of a tab-separated file. Rows are dicts with stripped string values."""
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE)
        header = list(reader.fieldnames or [])
        rows = [{k: (v or "").strip() for k, v in r.items() if k} for r in reader]
    return header, rows


def write_tsv(path, header, rows):
    lines = ["\t".join(header)] + ["\t".join(str(r.get(h) or "") for h in header) for r in rows]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def last_line(out):
    lines = (out or "").strip().splitlines()
    return lines[-1] if lines else "(no output)"


def scratch_dir(wave, iid):
    """Private build root for one item (the design's /tmp/claude-1000/fixwave-<W>-<id>)."""
    return f"{SCRATCH_BASE}/fixwave-{wave}-{iid}"


def check_names(wave, batch):
    if not re.fullmatch(NAME_RE, wave):
        raise Refused(f"invalid wave id {wave!r}: use [A-Za-z0-9._-], starting with a letter or digit")
    if not re.fullmatch(r"[A-Z]+", batch):
        raise Refused(f"invalid batch letter {batch!r}: use capital letters, e.g. A")


def load_batch(wr, wave, batch):
    path = wr / wave / "batches.tsv"
    if not path.is_file():
        raise Refused(f"missing {path}: run the plan step first")
    _, rows = read_tsv(path)
    for r in rows:
        if r.get("batch") == batch:
            ids = lst(r.get("ids"))
            if not ids:
                raise Refused(f"batch {batch} in {path} has no ids")
            return ids
    have = ", ".join(sorted({r.get("batch", "") for r in rows})) or "none"
    raise Refused(f"unknown batch {batch!r} in {path} (batches: {have})")


def load_items(wr, wave, ids):
    """(header, rows) of the batch's items in items.tsv order, validated."""
    path = wr / wave / "items.tsv"
    if not path.is_file():
        raise Refused(f"missing {path}: run the plan step first")
    header, rows = read_tsv(path)
    seen = set()
    for r in rows:
        if r.get("id") in seen:
            raise Refused(f"duplicate item id {r.get('id')} in {path}")
        seen.add(r.get("id"))
    missing = [i for i in ids if i not in seen]
    if missing:
        raise Refused(f"batch ids not in {path}: {', '.join(missing)}")
    items = [r for r in rows if r.get("id") in ids]
    for r in items:
        iid = r.get("id")
        try:
            variants(r.get("kind", ""), r.get("size", ""))
        except ValueError as e:
            raise Refused(f"row {iid}: {e} (kind bug|gap, size S|M|L or empty)")
        for col in ("write", "tests", "entries"):
            if not lst(r.get(col)):
                raise Refused(f"row {iid}: empty {col}")
    return header, items


def load_ctx(wr, wave, repo):
    """Findings context from WR/<W>/plan.json (both keys optional). Relative findings paths resolve against the repo."""
    path = wr / wave / "plan.json"
    try:
        plan = json.loads(path.read_text(encoding="utf-8")) if path.is_file() else {}
    except ValueError as e:
        raise Refused(f"bad JSON in {path}: {e}")
    cols, findings = list(plan.get("cols") or []), None
    src = plan.get("findings")
    if src:
        fpath = Path(src) if Path(src).is_absolute() else repo / src
        if not fpath.is_file():
            raise Refused(f"findings file not found: {fpath}")
        header, rows = read_tsv(fpath)
        cols = cols or header
        if not cols:
            raise Refused(f"findings file has no columns: {fpath}")
        findings = {r.get(cols[0], ""): r for r in rows}
    return {"repo_root": repo, "findings": findings, "cols": cols}


def load_templates(repo):
    d = repo / TEMPLATE_DIR
    missing = [n for n in TEMPLATES if not (d / n).is_file()]
    if missing:
        raise Refused(f"missing templates in {d}: {', '.join(missing)}")
    return {n: (d / n).read_text(encoding="utf-8") for n in TEMPLATES}


def _section_lines(lines, heading):
    """Block headed `## <heading>` or `### <heading>` (whole word match) as one string, heading line included.
    The block ends at the next heading of the same or a higher level, or at a `ROW\\t` line. None if absent."""
    if not heading:
        return None
    pat = re.compile(r"^(#{2,3}) " + re.escape(heading) + r"(?![\w-])")
    for i, line in enumerate(lines):
        m = pat.match(line)
        if not m:
            continue
        level = len(m.group(1))
        body = []
        for nxt in lines[i + 1:]:
            if nxt.startswith("ROW\t"):
                break
            hm = re.match(r"^(#{1,3}) ", nxt)
            if hm and len(hm.group(1)) <= level:
                break
            body.append(nxt)
        return "\n".join([line] + body).strip()
    return None


def section(path, heading):
    """Text of the `## heading` (or `### heading`) block in a markdown file, stopping at the next heading of the same or a
    higher level. Missing file or heading -> `(section <h> not found in <path>)`."""
    p = Path(path)
    block = _section_lines(p.read_text(encoding="utf-8").splitlines(), heading) if p.is_file() else None
    return block if block is not None else f"(section {heading} not found in {path})"


def load_variants(text):
    """(label -> variant text, FORMAT text) from variants.md. FORMAT runs to end of file because it has its own `## ` lines."""
    lines = text.splitlines()
    cut = next((i for i, l in enumerate(lines) if l == "## FORMAT"), None)
    if cut is None:
        raise Refused("variants.md has no '## FORMAT' section")
    body = lines[:cut]
    var = {}
    for lab in VARIANT_LABELS:
        block = _section_lines(body, lab)
        txt = block.split("\n", 1)[1].strip() if block and "\n" in block else ""
        if not txt:
            raise Refused(f"variants.md has no text under '## {lab}'")
        var[lab] = txt
    fmt = "\n".join(lines[cut + 1:]).strip("\n")
    if not fmt:
        raise Refused("variants.md '## FORMAT' section is empty")
    return var, fmt


def is_test(path):
    n = Path(path).name
    return n.endswith("_test.cpp") or (n.startswith("test_") and n.endswith(".py"))


def tail(repo, path, n=8):
    p = repo / path
    if not p.is_file() or not is_test(path):
        return []
    lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
    lang = "cpp" if path.endswith(".cpp") else "python"
    lo = max(1, len(lines) - n + 1)
    return [f"### tail of {path} (lines {lo}-{len(lines)})", f"```{lang}", *lines[-n:], "```", ""]


def anchors(pats, files, repo):
    present = [f for f in files if (repo / f).exists()]
    out = []
    for p in pats:
        hits = []
        if present:
            try:
                r = subprocess.run(["rg", "-n", "-m", "4", "-e", p, *present], cwd=str(repo), capture_output=True, text=True)
                hits = [":".join(l.split(":", 2)[:2]) for l in r.stdout.splitlines()[:4]]
            except OSError:
                hits = ["(rg unavailable)"]
        out.append(f"- `{p}`: " + ("; ".join(hits) if hits else "(no hit)"))
    return out


def findings_block(fids, ctx):
    findings, cols = ctx["findings"], ctx["cols"]
    if findings is None:
        return ["## Findings rows (unresolved: no findings file in plan.json)", ", ".join(fids), ""]
    found = [findings[f] for f in fids if f in findings]
    missing = [f for f in fids if f not in findings]
    lines = ["## Findings rows", "| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
    lines += ["| " + " | ".join(r.get(c, "").replace("|", "/") for c in cols) + " |" for r in found]
    if missing:
        lines += ["", f"(not in findings file: {', '.join(missing)})"]
    return lines + [""]


def _note(repo, ref, title):
    """Section for a `path#heading` cell, or a placeholder when the cell is empty."""
    if not ref:
        return [f"## {title}", "(none)", ""]
    path, _, head = ref.partition("#")
    return [f"## {title} ({ref})", section(repo / path, head), ""]


def pack(row, ctx):
    """Context pack for one fix unit (markdown text)."""
    repo, iid = ctx["repo_root"], row["id"]
    write = [ent(f) for f in lst(row.get("write"))]
    tests = lst(row.get("tests"))
    entries = [ent(e) for e in lst(row.get("entries"))]
    fids = lst(row.get("fids"))
    out = [f"# {iid} fix wave pack (kind {row['kind']}, group {row.get('group') or '-'})", ""]
    if fids:
        out += findings_block(fids, ctx)
    else:
        out += ["## Origin", "No findings row (no fids): the Context notes below are the defect record.", ""]
    out += _note(repo, row.get("doc", ""), "Doc section")
    out += _note(repo, row.get("context", ""), "Context notes")
    out += ["## Item constraint (from the plan; binding)", row.get("constraint") or "(none)", "",
            "## Write scope", *[f"- {f}" for f in write], "",
            "## Test ids", *[f"- {t}" for t in tests], "",
            "## rg anchors (at brief time)",
            *anchors(lst(row.get("anchors")), sorted({e.split(":")[0] for e in entries} | set(write)), repo), ""]
    for f in write:
        out += tail(repo, f)
    return "\n".join(out) + "\n"


def compat_text(write):
    sh = [f for f in write if "/_shared_src/" in f and not is_test(f)]
    if not sh:
        return ""
    return (f"\n   Touched _shared_src: also `python3 scripts/compat_check.py {' '.join(sh)}` and "
            "`python3 scripts/check_shared_headers.py` (check only, never --sync).")


def gates(row, wave, iid):
    """Gate sentence for the consolidator and reviewer: host tests, plus the Windows compile gate when the item has mods."""
    tests = lst(row.get("tests"))
    text = f"`python3 scripts/run_tests.py {' '.join(tests)}` from the worktree root"
    mods = lst(row.get("mods"))
    if mods:
        sd = scratch_dir(wave, iid)
        flags = " ".join(f"--mod {m}" for m in mods)
        text += (f", and the Windows compile gate `python3 scripts/build_check.py {flags} --quiet --build-root {sd} "
                 f"--kpm-dir {sd}/kpm` (the host tests do NOT compile the mod .cpp; this does, with -Werror)")
    if any("/k1_port/" in t for t in tests):
        text += "; k1 tests need the k1port venv (outside the repo): a test that prints SKIP is NOT a pass, report it as SKIP"
    return text


def plan_units(items):
    """[(uid, label, item row, variant)] in batch order; uids restart at u01 for every batch."""
    plan = []
    for it in items:
        for v in variants(it["kind"], it.get("size", "")):
            plan.append((f"u{len(plan) + 1:02d}", f"{it['id']}-{v}", it, v))
    return plan


def haiku_task(uid, label, it, var_text, pack_text, fmt):
    """(task text, first three entries for wave_tools). Entries past the third become named read windows in the task."""
    entries = [ent(e) for e in lst(it["entries"])]
    tests = ", ".join(lst(it["tests"]))
    task = (f"Unit {uid} = {label}.\nPropose a fix for {it['id']} (kind {it['kind']}). Test ids: {tests}.\n"
            f"{var_text}\n\n{pack_text}\n{fmt}")
    if len(entries) > 3:
        task += "\n\nAlso read these windows (Read with offset/limit, nothing more): " + ", ".join(entries[3:])
    return task, entries[:3]


def render(template, name, **fields):
    try:
        return template.format(**fields)
    except (KeyError, IndexError, ValueError) as e:
        raise Refused(f"{name} template: bad placeholder {e}")


def contract_text(repo, role):
    """Output of `agent_contract.py print <role>`: the repo copy first, then this script's sibling."""
    for cand in (Path(repo) / "scripts" / "agent_contract.py", SCRIPTS / "agent_contract.py"):
        if cand.is_file():
            r = subprocess.run([sys.executable, str(cand), "print", role], capture_output=True, text=True)
            if r.returncode == 0 and r.stdout.strip():
                return r.stdout.strip()
    return f"(agent_contract.py print {role} unavailable: copy its block here before launching)"


def git_base(repo):
    try:
        r = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=str(repo), capture_output=True, text=True)
    except OSError:
        return "unknown"
    return r.stdout.strip() or "unknown"


def run_wave_tools(args, repo_root):
    """Run scripts/wave_tools.py with args in repo_root. Returns (returncode, stdout + stderr). Tests replace this."""
    cmd = [sys.executable, str(Path(repo_root) / "scripts" / "wave_tools.py"), *args]
    r = subprocess.run(cmd, cwd=str(repo_root), capture_output=True, text=True)
    return r.returncode, r.stdout + r.stderr


def build(waves_root, repo_root, wave, batch, dry_run, force, run_wave_tools=run_wave_tools):
    """Write batch <wave>-<batch> and add its units. Returns the exit code (0, 2 or 3); see the module docstring."""
    wr, repo = Path(waves_root).resolve(), Path(repo_root).resolve()
    bname = f"{wave}-{batch}"
    bdir = wr / bname
    try:
        check_names(wave, batch)
        ids = load_batch(wr, wave, batch)
        if (bdir / "manifest.json").exists() and not force:
            print(f"exists: {bname} (use --force)")
            return 0
        header, items = load_items(wr, wave, ids)
        ctx = load_ctx(wr, wave, repo)
        tmpl = load_templates(repo)
        var, fmt = load_variants(tmpl["variants.md"])
        plan = plan_units(items)
        base, worker, review = git_base(repo), contract_text(repo, "worker"), contract_text(repo, "review")
        packs = {it["id"]: pack(it, ctx) for it in items}
        tasks = []
        for uid, label, it, v in plan:
            task, ents = haiku_task(uid, label, it, var[v], packs[it["id"]], fmt)
            tasks.append((uid, label, task, ents))
        files = {}
        for it in items:
            iid = it["id"]
            write = [ent(f) for f in lst(it["write"])]
            props = ", ".join(f"{bdir}/{uid}.md ({label})" for uid, label, x, _ in plan if x["id"] == iid)
            fields = dict(u=iid, w_id=bname, base=base, root=str(repo), wave_dir=str(bdir), scope=", ".join(write),
                          gates=gates(it, wave, iid), proof=PROOF[it["kind"]], scratch=scratch_dir(wave, iid),
                          compat=compat_text(write), props=props, contract=worker)
            files[iid] = {
                "pack.md": packs[iid],
                "consolidate.md": render(tmpl["consolidate.md"], "consolidate.md", **fields),
                "review.brief.md": render(tmpl["review.md"], "review.md", **{**fields, "contract": review}),
            }
    except Refused as e:
        print(f"error: {e}", file=sys.stderr)
        return 2

    if dry_run:
        for uid, label, task, _ in tasks:
            print(f"DRY {uid} {label} {len(task)}")
        print(f"{len(plan)} units; {len(items)} items (dry run: nothing written)")
        return 0

    if force and bdir.exists():
        shutil.rmtree(bdir)
    rc, out = run_wave_tools(["new", "--kind", "recon", bname], repo)
    if rc != 0:
        print(f"error: wave_tools new {bname} failed: {last_line(out)}", file=sys.stderr)
        return 2
    if not (bdir / "manifest.json").is_file():
        print(f"error: wave_tools new did not create {bdir}/manifest.json (waves root must be <repo>/.claude/waves)",
              file=sys.stderr)
        return 2

    write_tsv(bdir / "items.tsv", header, items)
    (bdir / "units.tsv").write_text("uid\tlabel\n" + "".join(f"{uid}\t{label}\n" for uid, label, _, _ in plan), encoding="utf-8")
    for iid, fs in files.items():
        d = bdir / iid
        d.mkdir(parents=True, exist_ok=True)
        for name, text in fs.items():
            (d / name).write_text(text, encoding="utf-8")

    rejected = []
    for uid, label, task, ents in tasks:
        rc, out = run_wave_tools(["add", bname, "--id", uid, "--task", task, "--entries", *ents], repo)
        if rc != 0:
            rejected.append((uid, label, rc, out))
    for uid, label, rc, out in rejected:
        print(f"REJECTED {uid} {label} (rc {rc}): {last_line(out)}")
    print(f"{len(plan)} units; {len(items)} items; {len(plan) - len(rejected)} added; batch {bdir}")
    if rejected:
        print(f"fix the cause, then rerun: fix_wave_brief.py brief {wave} --batch {batch} --force")
        return 3
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description="Write one fix-wave batch's packs and briefs and add its Haiku units.",
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    pb = sub.add_parser("brief", help="write a batch's pack and briefs, add its Haiku variant units")
    pb.add_argument("wave", help="wave name: WR/<wave>/ holds batches.tsv, items.tsv, plan.json")
    pb.add_argument("--batch", required=True, metavar="B", help="batch letter from batches.tsv")
    pb.add_argument("--dry-run", action="store_true", help="print the units and sizes; write nothing")
    pb.add_argument("--force", action="store_true", help="replace an existing batch dir <wave>-<B>")
    pb.add_argument("--waves-root", default=None, metavar="DIR", help="default <repo>/.claude/waves")
    pb.add_argument("--repo-root", default=None, metavar="DIR", help="default: the repo this script lives in")
    a = ap.parse_args(argv)
    repo = Path(a.repo_root) if a.repo_root else SCRIPTS.parent
    waves = Path(a.waves_root) if a.waves_root else repo / ".claude" / "waves"
    return build(waves, repo, a.wave, a.batch, a.dry_run, a.force)


if __name__ == "__main__":
    sys.exit(main())
