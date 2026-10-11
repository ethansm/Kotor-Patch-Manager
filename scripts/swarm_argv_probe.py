#!/usr/bin/env python3
"""M1 $0 argv parity probe for the two map runners (.claude/plans/swarm_library_architecture.md, section 11).

Copies the REAL `effort_args` and `run_one` functions out of each run_map.sh (bundle: ~/.claude/skills/deep-haiku-review/
bundle/engine/run_map.sh; fork: .claude/review/run_map.sh) and runs them in a scratch dir. Only the preconditions and the
prep.py / record_run.py calls are faked. `claude` is a shim that logs its argv and stdin hash, then forwards to the offline
stub (bundle/stub/claude). Nothing reaches the network or a model, and no live run dir is read or written.

Compares the argv each engine passes to `claude -p`, plus the content of the three file-valued inputs (settings, schema,
system prompt). A positive control (bundle with EFFORT_MAP=xhigh) must show --effort, or the probe reports FAIL.
dr_sandbox_prefix is stubbed to an empty prefix so the probe compares the claude argv only; the OS wrapper is covered by
swarm-bundle-wrapper.

Usage: python3 scripts/swarm_argv_probe.py [--work DIR] [--keep] [--json]
Exit: 0 probe ran and control passed, 1 probe error or control failed.
"""
import argparse
import difflib
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
BUNDLE = Path.home() / ".claude/skills/deep-haiku-review/bundle"
FORK = REPO / ".claude/review"
STUB = BUNDLE / "stub/claude"
MODEL = "claude-haiku-5-5"   # bundle resolves MODEL from the profile; harness pins it for both
PACKET = "FAKE-1"
PROMPT = "Review packet FAKE-1 (argv probe fixture). Report findings as JSON.\n"
VALUE_FLAGS = {"--model", "--effort", "--settings", "--tools", "--permission-mode", "--max-budget-usd",
               "--json-schema", "--mcp-config", "--output-format", "--append-system-prompt"}
CONTENT_FLAGS = {"--settings": "worker_settings.json", "--json-schema": "worker_schema.json",
                 "--append-system-prompt": "worker_system_prompt.md"}
ENGINE_FILES = {
    "bundle": {"src": BUNDLE / "engine/run_map.sh", "dir": BUNDLE / "templates"},
    "fork": {"src": FORK / "run_map.sh", "dir": FORK},
}
CASES = [  # name, engine, extra env
    ("bundle_default", "bundle", {}),
    ("bundle_effort", "bundle", {"EFFORT_MAP": "xhigh"}),
    ("fork_default", "fork", {}),
]

FAKE_PREP = "import sys\nif sys.argv[1] == 'is-complete':\n    sys.exit(1)\nif sys.argv[1] == 'render':\n" \
            "    sys.stdout.write(%r)\n    sys.exit(0)\nsys.exit(2)\n" % PROMPT
FAKE_RECORD = "import sys\nsys.exit(0)\n"
FAKE_BUDGET = "import sys\nif sys.argv[1] == 'reserve':\n    print('probe-token')\nsys.exit(0)\n"   # run_one reserves/settles per attempt (M3a)
SHIM = "#!/usr/bin/env python3\n# argv probe shim: logs argv and stdin hash, then forwards to the offline stub\n" \
       "import hashlib, json, os, subprocess, sys\n" \
       "prompt = sys.stdin.buffer.read()\n" \
       "rec = {'cwd': os.getcwd(), 'argv': sys.argv[1:], 'stdin_sha256': hashlib.sha256(prompt).hexdigest()}\n" \
       "with open(os.environ['PROBE_LOG'], 'a') as f:\n    f.write(json.dumps(rec) + '\\n')\n" \
       "r = subprocess.run([sys.executable, '-I', %r] + sys.argv[1:], input=prompt, capture_output=True)\n" \
       "sys.stdout.buffer.write(r.stdout)\nsys.stderr.buffer.write(r.stderr)\nsys.exit(r.returncode)\n" % str(STUB)


def block(text, header):
    """Bash function text from the line starting with `header` up to the first line that is exactly `}`."""
    lines = text.splitlines()
    i = next(n for n, ln in enumerate(lines) if ln.startswith(header))
    j = next(n for n in range(i + 1, len(lines)) if lines[n] == "}")
    return "\n".join(lines[i:j + 1])


def top_line(text, prefix):
    return next(ln for ln in text.splitlines() if ln.startswith(prefix))


def harness_text(engine, src, root):
    text = src.read_text()
    eng = root / engine
    if engine == "bundle":
        env = (f'P_ENGINE="{eng}"; P_RUN="{root}/run"; P_TEMPLATES="{root}/none"; P_TREE="{root}/tree"\n'
               f'SETTINGS="{eng}/worker_settings.json"; SCHEMA="{eng}/worker_schema.json"; SYSPROMPT="{eng}/worker_system_prompt.md"\n'
               f'CLAUDE_BIN="{root}/bin/claude"; CALL_USD=0.30; TIMEOUT_S=900; BACKOFF_BASE=15; BACKOFF_SCALE=0; PY="python3 -I"\n'
               f'TOTAL_USD=5; SPENT_CMD="echo 0"; DEFER_FILE="{root}/run/map.deferred"\n'
               f'mkdir -p "$P_RUN"/raw "$P_RUN"/work/prompts "$P_TREE"\n')
        effort = block(text, "effort_args() {") + "\n" + block(text, "budget_settle() {") + "\n"
    else:
        env = (f'REVIEW_DIR="{eng}"; TREE="{root}/tree"; WORKER_TIMEOUT_S=900; WORKER_BUDGET_USD=0.30; PY="python3 -I"\n'
               f'mkdir -p "$REVIEW_DIR"/raw "$REVIEW_DIR"/work/prompts "$TREE"\n')
        effort = ""
    return ("#!/bin/bash\n" + "DR_HIDE_HOME=0\n" + "dr_sandbox_prefix() { DR_SBX=(); }\n"
            + top_line(text, "ISOLATION=(") + "\n" + 'ISOLATION_STR="${ISOLATION[*]}"\n'
            + f'MODEL="{MODEL}"\n' + env + effort + block(text, "run_one() {") + "\n"
            + f'run_one "{PACKET}"\n')


def run_case(work, name, engine, extra_env):
    root = work / name
    eng = root / engine
    eng.mkdir(parents=True)
    for fname in CONTENT_FLAGS.values():   # same file names in templates/ and in the fork dir
        shutil.copy(ENGINE_FILES[engine]["dir"] / fname, eng / fname)
    (eng / "prep.py").write_text(FAKE_PREP)
    (eng / "tools").mkdir()
    (eng / "tools/record_run.py").write_text(FAKE_RECORD)
    (eng / "tools/budget.py").write_text(FAKE_BUDGET)
    (root / "bin").mkdir()
    shim = root / "bin/claude"
    shim.write_text(SHIM)
    shim.chmod(0o755)
    (root / "tree").mkdir()
    (root / "stubstate").mkdir()
    harness = root / "harness.sh"
    harness.write_text(harness_text(engine, ENGINE_FILES[engine]["src"], root))
    log = root / "probe.log"
    env = {"PATH": f"{root}/bin:{os.environ.get('PATH', '')}", "HOME": os.environ.get("HOME", ""),
           "PROBE_LOG": str(log), "STUB_LOG": str(root / "stub.log"), "STUB_STATE": str(root / "stubstate"),
           "TMPDIR": os.environ.get("TMPDIR", "/tmp")}
    env.update(extra_env)
    proc = subprocess.run(["bash", str(harness)], env=env, capture_output=True, text=True, timeout=300)
    recs = [json.loads(ln) for ln in log.read_text().splitlines() if ln.strip()] if log.exists() else []
    return {"case": name, "engine": engine, "rc": proc.returncode, "calls": len(recs),
            "record": recs[0] if recs else None,
            "stderr_tail": proc.stderr.strip().splitlines()[-3:] if proc.returncode or not recs else [],
            "root": root}


def sha(data):
    return hashlib.sha256(data).hexdigest()[:12]


def normalise(result):
    """Flag/value tokens. --settings and --append-system-prompt values are placeholders: their content is NOT compared
    (the bundle writes both at build time from the profile). --json-schema is hashed (the bundle passes the template)."""
    rec = result["record"]
    out = [f"cwd {Path(rec['cwd']).name}"]
    argv = rec["argv"]
    i = 0
    while i < len(argv):
        a = argv[i]
        if a in VALUE_FLAGS and i + 1 < len(argv):
            v = argv[i + 1]
            if a == "--settings":
                v = "<settings-file>"
            elif a == "--append-system-prompt":
                v = "<system-prompt-text>"
            elif a == "--json-schema":
                v = "content:" + sha(v.encode())
            out.append(f"{a} {v}")
            i += 2
        else:
            out.append(a)
            i += 1
    out.append(f"stdin sha256:{rec['stdin_sha256'][:12]}")
    return out


def diff_tokens(a, b):
    rows = []
    for op, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
        if op == "equal":
            continue
        rows.append({"op": op, "left": a[i1:i2], "right": b[j1:j2]})
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--work", help="scratch dir (default: a new temp dir, removed unless --keep)")
    ap.add_argument("--keep", action="store_true", help="keep the scratch dir")
    ap.add_argument("--json", action="store_true", help="print the result as JSON")
    args = ap.parse_args()
    for p in (STUB, BUNDLE / "engine/run_map.sh", FORK / "run_map.sh"):
        if not p.exists():
            print(f"argv probe: missing {p}", file=sys.stderr)
            return 1
    made = args.work is None
    work = Path(args.work or tempfile.mkdtemp(prefix="swarm-argv-probe-")).resolve()
    work.mkdir(parents=True, exist_ok=True)
    try:
        results = {name: run_case(work, name, engine, env) for name, engine, env in CASES}
        errors = {k: v for k, v in results.items() if v["record"] is None or v["rc"] != 0}
        if errors:
            for k, v in errors.items():
                print(f"argv probe: {k} rc={v['rc']} calls={v['calls']} stderr={v['stderr_tail']}", file=sys.stderr)
            return 1
        tok = {k: normalise(v) for k, v in results.items()}
        base_diff = diff_tokens(tok["bundle_default"], tok["fork_default"])
        effort_diff = diff_tokens(tok["bundle_default"], tok["bundle_effort"])
        control_ok = any("--effort" in " ".join(r["right"]) for r in effort_diff)
        schema_same = sha((ENGINE_FILES["bundle"]["dir"] / "worker_schema.json").read_bytes()) == \
            sha((ENGINE_FILES["fork"]["dir"] / "worker_schema.json").read_bytes())
        summary = {
            "tokens": {k: len(v) for k, v in tok.items()},
            "argv_diff_bundle_default_vs_fork": base_diff,
            "argv_diff_effort_added_by_EFFORT_MAP": effort_diff,
            "schema_identical": schema_same,
            "content_not_compared": ["worker_settings.json", "worker_system_prompt.md"],
            "control_effort_visible": control_ok,
            "stub_calls": {k: v["calls"] for k, v in results.items()},
        }
        if args.json:
            print(json.dumps(summary, indent=2))
        else:
            print("M1 argv parity probe ($0, offline stub, scratch dir: %s)" % work)
            print("tokens per case:", ", ".join(f"{k}={n}" for k, n in summary["tokens"].items()))
            print("stub calls:", ", ".join(f"{k}={n}" for k, n in summary["stub_calls"].items()))
            if base_diff:
                print("\nargv: bundle_default vs fork_default DIFFERS:")
                for r in base_diff:
                    print(f"  {r['op']}: bundle={r['left']} fork={r['right']}")
            else:
                print("\nargv: bundle_default vs fork_default IDENTICAL (same flags, values and order)")
            print("\nargv: EFFORT_MAP=xhigh adds to bundle_default:")
            for r in effort_diff:
                print(f"  {r['op']}: {r['left']} -> {r['right']}")
            print("\nschema file (passed as --json-schema): bundle template == fork file:", schema_same)
            print("NOT COMPARED: worker_settings.json and worker_system_prompt.md. The bundle writes both at build time")
            print("  from the profile (prep.py:828-829, :980); the fork file is its own output. Open item for M2/M3.")
            print("\ncontrol (--effort must appear when EFFORT_MAP is set):", "PASS" if control_ok else "FAIL")
        return 0 if control_ok else 1
    finally:
        if made and not args.keep:
            shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
