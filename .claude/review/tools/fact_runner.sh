#!/usr/bin/env bash
# Optional fact runner: run the repo's read-only checkers in a SCRATCH, history-free copy of the tree and
# save their failures to .claude/review/facts/<mod>.json (attached to packets as provisional known facts).
# No model calls. Run only after the go-ahead. Never runs install/hot-swap/deploy paths, never in the repo.
#
# Guards: MASTER_LEDGER_PATH -> scratch; KOTOR2_ROOT -> a nonexistent scratch dir (no live game dir for
# run_tests); production .claude/master_ledger.jsonl hashed before/after and must be byte-identical.
# Skipped by design: agent_diff_check.py, compat_check.py (need git history), kpm_hotswap.py (install path),
# clef_dataset.py / shadow_replay.py (own DEFAULT_LEDGER, ignore MASTER_LEDGER_PATH),
# check-raw-addresses (needs the excluded tests/static/raw_addresses.toml: human-only gap).
set -euo pipefail

REVIEW_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REPO="$(cd "$REVIEW_DIR/../.." && pwd)"
RHOME="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}"
FR="$RHOME/factrun"; SCR="$FR/tree"; PY="python3 -I"
LEDGER="$REPO/.claude/master_ledger.jsonl"
rm -rf "$FR"; mkdir -p "$FR/build" "$FR/out"
before="$(sha256sum "$LEDGER" 2>/dev/null | cut -d' ' -f1 || echo absent)"

$PY "$REVIEW_DIR/prep.py" build-tree "$SCR" > "$FR/tree_info.json"
chmod -R u+w "$SCR"
export MASTER_LEDGER_PATH="$FR/scratch_ledger.jsonl"
export KOTOR2_ROOT="$FR/no_game_dir"            # deliberately missing
export TMPDIR="$FR/tmp"; mkdir -p "$TMPDIR"

mapfile -t MODS < <($PY - "$REVIEW_DIR/packets" <<'EOF'
import json, pathlib, sys
print("\n".join(sorted({m for p in pathlib.Path(sys.argv[1]).glob("*.json") for m in json.loads(p.read_text()).get("mods", [])})))
EOF
)

run() {   # name, logfile, cmd...
  local name="$1" log="$2"; shift 2
  set +e; (cd "$SCR" && timeout 900 "$@") > "$log" 2>&1; local rc=$?; set -e
  echo "$name rc=$rc"; echo "$rc" > "$log.rc"
}

run check_hook_spans "$FR/out/check_hook_spans.log" python3 scripts/check_hook_spans.py --repo "$SCR" --quiet
run check_hook_bytes "$FR/out/check_hook_bytes.log" python3 scripts/check_hook_bytes.py --quiet
for m in "${MODS[@]}"; do
  [[ "$m" == _shared_src ]] && continue
  run "build_check:$m" "$FR/out/build_$m.log" python3 scripts/build_check.py --repo "$SCR" --mod "$m" \
      --build-root "$FR/build" --quiet --jobs 2
  key="${m%_src}"
  run "run_tests:$m" "$FR/out/tests_$m.log" python3 scripts/run_tests.py -k "$key" --jobs 2
done

$PY - "$FR/out" "$REVIEW_DIR/facts" "${MODS[@]}" <<'EOF'
import json, pathlib, re, sys
out, facts = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2]); facts.mkdir(exist_ok=True)
def tail(p):
    t = p.read_text(errors="replace").splitlines() if p.exists() else []
    bad = [l for l in t if re.search(r"FAIL|ERROR|error:|Traceback|WARN", l)]
    return (bad or t)[-60:]
def rc(p):
    f = pathlib.Path(str(p) + ".rc")
    return int(f.read_text()) if f.exists() else None
shared = {n: {"rc": rc(out / f"{n}.log"), "lines": tail(out / f"{n}.log")}
          for n in ("check_hook_spans", "check_hook_bytes")}
for m in sys.argv[3:]:
    mid = m[:-4] if m.endswith("_src") else m
    rec = {"mod": m, "provisional": "until WP1 is reduced; scratch copy has no git history and no binaries",
           "depends_on_live_install": "check_hook_spans reads the live patch_config.toml; check_hook_bytes reads the vanilla exe",
           "skipped": {"agent_diff_check.py": "needs-git-history", "compat_check.py": "needs-git-history",
                       "check-raw-addresses": "needs excluded tests/static/raw_addresses.toml (human-only gap)"},
           "build_check": {"rc": rc(out / f"build_{m}.log"), "lines": tail(out / f"build_{m}.log")},
           "run_tests": {"rc": rc(out / f"tests_{m}.log"), "lines": tail(out / f"tests_{m}.log")},
           "hook_checkers_mentions": {n: [l for l in v["lines"] if mid.replace("_", "") in l.replace("_", "").replace("-", "")][:20]
                                      for n, v in shared.items()}}
    (facts / f"{m}.json").write_text(json.dumps(rec, indent=1, sort_keys=True) + "\n")
(facts / "_shared_checkers.json").write_text(json.dumps(shared, indent=1, sort_keys=True) + "\n")
print(f"facts written for {len(sys.argv) - 3} mods")
EOF

after="$(sha256sum "$LEDGER" 2>/dev/null | cut -d' ' -f1 || echo absent)"
if [[ "$before" != "$after" ]]; then
  echo "FACT RUNNER: production ledger CHANGED ($before -> $after). Treat facts as void; investigate." >&2
  exit 3
fi
echo "fact runner OK; production ledger unchanged ($before)"
