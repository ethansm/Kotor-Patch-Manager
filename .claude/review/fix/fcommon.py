#!/usr/bin/env python3
"""Shared helpers for the Fix pipeline (stdlib only, no LLM calls). Contract: ../INTERFACES.md, "Fix stage".

Layout (all overridable so self-tests never touch real outputs):
  CODE_DIR          this directory (.claude/review/fix): scripts, prompts, schemas, specialty_notes/
  FIX_DIR           data dir: raw/ results/ checks/ reviews/ integ/          (default = CODE_DIR)
  FIX_WORK          work dir: fix_queue.json human_list.json refuted_skim.json prompts/ batches/ calib_key.json
                    (default .claude/review/work/fix; when FIX_DIR is overridden: $FIX_DIR/work)
  KOTOR2_REVIEW_HOME  holds worktree/ (sanitized pinned snapshot) and fix/ (FIX_HOME)
  FIX_HOME          $KOTOR2_REVIEW_HOME/fix: base/ (buildable git copy) tasks/ slots/ integ/ canary/
  KOTOR2_FIX_OUT    patch series output (default .claude/fixes)
  KOTOR2_FIX_REPORT fix_report.{json,md} dir (default .claude/reviews)
  FIX_FINDINGS      input findings (default .claude/reviews/verified_findings.v2.json, else verified_findings.json)

CLI:
  fcommon.py paths                      print the resolved layout as JSON
  fcommon.py spent fix|review|integrate sum of CLI-reported cost_usd of recorded runs
  fcommon.py --self-test
"""
from __future__ import annotations

import fnmatch
import hashlib
import json
import os
import re
import sys
from pathlib import Path

CODE_DIR = Path(__file__).resolve().parent
REVIEW = CODE_DIR.parent                       # .claude/review
REPO = REVIEW.parent.parent                    # repo root (read-only for this pipeline)
for _p in (str(REVIEW / "tools"),):
    if _p not in sys.path:
        sys.path.append(_p)                    # append: never shadow stdlib (verify/select.py lesson)

FIX_DIR = Path(os.environ.get("FIX_DIR") or CODE_DIR).resolve()
if os.environ.get("FIX_WORK"):
    WORK = Path(os.environ["FIX_WORK"]).resolve()
elif FIX_DIR == CODE_DIR:
    WORK = REVIEW / "work" / "fix"
else:
    WORK = FIX_DIR / "work"
RHOME = Path(os.environ.get("KOTOR2_REVIEW_HOME") or (Path.home() / ".cache" / "kotor2_review")).resolve()
TREE = RHOME / "worktree"                      # sanitized pinned snapshot (prep.py build), read-only
FIX_HOME = RHOME / "fix"
BASE = FIX_HOME / "base"
TASKS = FIX_HOME / "tasks"
SLOTS = FIX_HOME / "slots"
INTEG_HOME = FIX_HOME / "integ"
OUT = Path(os.environ.get("KOTOR2_FIX_OUT") or (REPO / ".claude" / "fixes")).resolve()
REPORT_OUT = Path(os.environ.get("KOTOR2_FIX_REPORT") or (REPO / ".claude" / "reviews")).resolve()
_V2 = REPO / ".claude" / "reviews" / "verified_findings.v2.json"   # v1 builds a stale 135-task queue (bug 17)
FINDINGS = Path(os.environ.get("FIX_FINDINGS") or (_V2 if _V2.is_file() else REPO / ".claude" / "reviews" / "verified_findings.json")).resolve()
PRODUCTION_LEDGER = REPO / ".claude" / "master_ledger.jsonl"
PINNED_HEAD = "22b56b427125e12bb7b2914d96b820973d06c445"

QUEUE = WORK / "fix_queue.json"
HUMAN = WORK / "human_list.json"
SKIM = WORK / "refuted_skim.json"
CALIB_KEY = WORK / "calib_key.json"            # answer key: never rendered into any prompt / task copy
PROMPTS = WORK / "prompts"
BATCHES = WORK / "batches"
RAW = FIX_DIR / "raw"
RESULTS = FIX_DIR / "results"                  # fixer results  <task>.aN.json (N = fixer attempt 1|2)
PATCHES = FIX_DIR / "patches"                  # harness-made diffs <task>.aN.patch
CHECKS = FIX_DIR / "checks"                    # <task>.aN.check.json (fix_checks.py)
REVIEWS = FIX_DIR / "reviews"                  # <task>.aN.review.json (+ .quote.json)
INTEG = FIX_DIR / "integ"                      # <batch>.json (Sonnet) + <batch>.verify.json (script)

SPECIALTIES = ["hook-asm", "test-weakening", "gate-integrity", "python-tooling", "cpp-logic", "build-config"]
CLASSES = ["auto-fixable", "needs-review", "needs-design", "human-only"]
FIX_TIERS = ["CONFIRMED", "CONFIRMED-BY-SONNET-ONLY"]
TASK_ID_RE = re.compile(r"^T[0-9a-f]{10}$")
ID_RE = re.compile(r"^[A-Za-z0-9_.-]{1,40}$")
LINE_CAP = {"auto-fixable": 40, "needs-review": 60, "needs-design": 120}

# Paths no fixer may ever touch (deploy / install / ledger writers / address DB). Matched with fnmatch on
# the repo-relative posix path. A finding whose file matches is human-only.
NEVER_EDIT = ["scripts/kpm_hotswap.py", "scripts/pre_install_gate.py", "scripts/pre_install_gate.sh",
              "scripts/*install*", "scripts/*deploy*", "*.db", "*.sqlite", "tests/static/raw_addresses.toml",
              ".claude/*", ".agents/*", ".opencode/*", ".mcp.json", ".github/*", "AGENTS.md", "CLAUDE.md"]
# Scripts that ignore MASTER_LEDGER_PATH: fixable, but their tests run only inside the fixgate sandbox
# (real repo hidden), never on the host. fix_checks.py enforces this.
LEDGER_UNSAFE = ["scripts/shadow_replay.py", "scripts/clef_dataset.py"]


def dumps(obj) -> str:
    return json.dumps(obj, sort_keys=True, indent=1, ensure_ascii=False) + "\n"


def write_json(path: Path, obj) -> None:
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".tmp")
    tmp.write_text(dumps(obj), encoding="utf-8")
    os.replace(tmp, path)


def rj(path):
    try:
        with open(path, encoding="utf-8") as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return None


def sha_hex(s: str | bytes) -> str:
    return hashlib.sha256(s if isinstance(s, bytes) else s.encode("utf-8")).hexdigest()


def file_sha(path: Path) -> str | None:
    try:
        return hashlib.sha256(Path(path).read_bytes()).hexdigest()
    except OSError:
        return None


_GLOBS = [".env", "*.pem", ".credentials.json", "endpoint.env", "*credential*", "*secret*"]


def is_excluded(path: str) -> bool:
    """Secret-pattern test. Uses prep.is_excluded but ALSO behind a dummy dir, because prep strips "./"
    with lstrip and so turns a root-level ".env" into "env" (known prep bug; prep.py is not edited)."""
    p = str(path or "").replace("\\", "/")
    try:
        sys.path.append(str(REVIEW))
        import prep  # type: ignore
        if prep.is_excluded(p) or prep.is_excluded("x/" + p.lstrip("/")):
            return True
    except Exception:
        pass
    low = p.lower()
    parts = [x for x in low.split("/") if x]
    if any(fnmatch.fnmatch(x, g) for x in parts for g in _GLOBS):
        return True
    return low.endswith("tests/static/raw_addresses.toml") or "sunshine/credentials/" in "/" + low


def safe_rel(path: str) -> str | None:
    """Normalized relative posix path, or None when absolute, '..', NUL, home-relative or excluded."""
    p = str(path or "").strip().replace("\\", "/")
    if not p or p.startswith("/") or p.startswith("~") or re.match(r"^[A-Za-z]:", p) or "\x00" in p:
        return None
    parts = [x for x in p.split("/") if x not in ("", ".")]
    if not parts or ".." in parts:
        return None
    rel = "/".join(parts)
    return None if is_excluded(rel) else rel


def never_edit(rel: str) -> bool:
    return any(fnmatch.fnmatch(rel, g) for g in NEVER_EDIT)


def parse_lines(v) -> tuple[int, int]:
    nums = [int(x) for x in re.findall(r"\d+", str(v or ""))]
    return (min(nums), max(nums)) if nums else (0, 0)


def read_lines(rel: str, root: Path | None = None) -> list[str] | None:
    root = Path(root or TREE)
    r = safe_rel(rel)
    if r is None:
        return None
    fp = (root / r)
    try:
        real = fp.resolve()
        if root.resolve() not in real.parents or not real.is_file():
            return None
        data = real.read_bytes()
    except OSError:
        return None
    if b"\x00" in data[:4096]:
        return None
    return data.decode("utf-8", errors="replace").splitlines()


def mod_of(rel: str) -> str | None:
    """patch_manager_mods/<x>_src/... -> '<x>_src'; _shared_src counts as a mod key of its own."""
    parts = rel.split("/")
    if len(parts) > 2 and parts[0] == "patch_manager_mods":
        return parts[1]
    return None


def load_queue(path: Path | None = None) -> dict:
    q = rj(path or QUEUE)
    if not isinstance(q, dict) or not isinstance(q.get("tasks"), list):
        sys.exit(f"fcommon: queue missing or invalid: {path or QUEUE} (run fix_intake.py)")
    return q


def task_by_id(tid: str, path: Path | None = None) -> dict:
    for t in load_queue(path)["tasks"]:
        if t["task_id"] == tid:
            return t
    sys.exit(f"fcommon: unknown task {tid}")


def cost_sum(directory: Path, suffix: str) -> float:
    tot = 0.0
    if not Path(directory).is_dir():
        return 0.0
    for f in sorted(Path(directory).glob(f"*{suffix}")):
        r = rj(f)
        if isinstance(r, dict) and isinstance(r.get("cost_usd"), (int, float)):
            tot += float(r["cost_usd"])
    return round(tot, 6)


def spent(stage: str) -> float:
    return {"fix": lambda: cost_sum(RESULTS, ".json"),
            "review": lambda: cost_sum(REVIEWS, ".review.json"),
            "integrate": lambda: cost_sum(INTEG, ".run.json"),
            "integ": lambda: cost_sum(INTEG, ".run.json")}[stage]()


def paths() -> dict:
    return {k: str(v) for k, v in {"CODE_DIR": CODE_DIR, "FIX_DIR": FIX_DIR, "WORK": WORK, "RHOME": RHOME,
            "TREE": TREE, "FIX_HOME": FIX_HOME, "BASE": BASE, "OUT": OUT, "REPORT_OUT": REPORT_OUT,
            "FINDINGS": FINDINGS, "REPO": REPO}.items()}


def self_test() -> int:
    bad = []
    for p in (".env", "a/.env", ".ENV", "x/cakey.PEM", "sunshine/credentials/a", "foo_secret_bar.txt",
              "tests/static/raw_addresses.toml", "a/.credentials.json", "endpoint.env", "MyCredentials/x"):
        if not is_excluded(p):
            bad.append(f"not excluded: {p}")
    for p in ("scripts/run_tests.py", "patch_manager_mods/queue_hud_src/exports.def", "env", "README.md"):
        if is_excluded(p):
            bad.append(f"excluded: {p}")
    for p in ("/etc/passwd", "../x", "a/../../b", "~/.ssh/id", ".env", "C:/x"):
        if safe_rel(p) is not None:
            bad.append(f"unsafe accepted: {p}")
    if safe_rel("./scripts//a.py") != "scripts/a.py":
        bad.append("normalize")
    if parse_lines("1068-1116") != (1068, 1116) or parse_lines("6") != (6, 6):
        bad.append("parse_lines")
    if not never_edit("scripts/kpm_hotswap.py") or never_edit("scripts/compat_check.py"):
        bad.append("never_edit")
    if mod_of("patch_manager_mods/queue_hud_src/QueueHud.cpp") != "queue_hud_src" or mod_of("scripts/x.py"):
        bad.append("mod_of")
    print("fcommon self-test:", "PASS" if not bad else "FAIL " + "; ".join(bad))
    return 1 if bad else 0


def main(argv: list[str]) -> int:
    if argv[:1] == ["--self-test"]:
        return self_test()
    if argv[:1] == ["paths"]:
        print(dumps(paths()), end="")
        return 0
    if len(argv) == 2 and argv[0] == "spent":
        print(spent(argv[1]))
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
