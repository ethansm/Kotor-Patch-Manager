#!/usr/bin/env bash
# Run one Reduce session (Sonnet 5.5) for a domain.
# Usage: run_reduce.sh [--dry-run] <domain-name> <wp ids comma-separated>
set -euo pipefail

DRY_RUN=0
ARGS=()
for a in "$@"; do
  case "$a" in
    --dry-run) DRY_RUN=1 ;;
    *) ARGS+=("$a") ;;
  esac
done
if [ "${#ARGS[@]}" -ne 2 ]; then
  echo "usage: run_reduce.sh [--dry-run] <domain-name> <wp1,wp2,...>" >&2
  exit 2
fi
DOMAIN="${ARGS[0]}"
WPS="${ARGS[1]}"
[[ "$DOMAIN" =~ ^[A-Za-z0-9_.-]+$ ]] || { echo "bad domain name: $DOMAIN" >&2; exit 2; }
[[ "$WPS" =~ ^[A-Za-z0-9_]+(,[A-Za-z0-9_]+)*$ ]] || { echo "bad wp list: $WPS" >&2; exit 2; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVIEW_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$REVIEW_DIR/../.." && pwd)"
REVIEWS_DIR="$REPO_ROOT/.claude/reviews"
RUNS_DIR="$REVIEW_DIR/runs"
WORKTREE="${KOTOR2_REVIEW_HOME:-$HOME/.cache/kotor2_review}/worktree"

# --- gates -------------------------------------------------------------------------------
[ -f "$REVIEW_DIR/canary.json" ] || { echo "refusing: $REVIEW_DIR/canary.json missing" >&2; exit 1; }
python3 -I - "$REVIEW_DIR/canary.json" <<'PY' || { echo "refusing: canary.json does not have \"pass\": true" >&2; exit 1; }
import json, sys
d = json.load(open(sys.argv[1], encoding="utf-8"))
sys.exit(0 if isinstance(d, dict) and d.get("pass") is True else 1)
PY
(cd "$REPO_ROOT" && python3 -I "$REVIEW_DIR/prep.py" verify-snapshot) \
  || { echo "refusing: snapshot verification failed" >&2; exit 1; }
[ -f "$REVIEW_DIR/withheld_leads.md" ] || { echo "abort: $REVIEW_DIR/withheld_leads.md missing" >&2; exit 1; }
[ -f "$REVIEW_DIR/worker_settings.json" ] || { echo "abort: $REVIEW_DIR/worker_settings.json missing" >&2; exit 1; }
[ -f "$REVIEW_DIR/reduce_prompt.md" ] && [ -f "$REVIEW_DIR/reduce_schema.json" ] \
  || { echo "abort: reduce_prompt.md or reduce_schema.json missing" >&2; exit 1; }
[ -d "$WORKTREE" ] || { echo "abort: sanitized worktree not found: $WORKTREE" >&2; exit 1; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# --- bundle + prompt -----------------------------------------------------------------------
python3 -I "$REVIEW_DIR/merge.py" --review-dir "$REVIEW_DIR" --emit-bundle "$WPS" > "$TMP/bundle.json"

python3 -I - "$REVIEW_DIR/reduce_prompt.md" "$REVIEW_DIR/withheld_leads.md" "$TMP/system_prompt.md" <<'PY'
import sys
prompt = open(sys.argv[1], encoding="utf-8").read()
leads = open(sys.argv[2], encoding="utf-8").read().strip()
if "{{WITHHELD_LEADS}}" not in prompt:
    sys.exit("placeholder {{WITHHELD_LEADS}} missing from reduce_prompt.md")
open(sys.argv[3], "w", encoding="utf-8").write(prompt.replace("{{WITHHELD_LEADS}}", leads) + "\n")
PY
{ printf 'Domain: %s\n\n## Findings\n' "$DOMAIN"; cat "$TMP/bundle.json"; } > "$TMP/user_prompt.md"

# isolation follows the variant the canary proved (A = --restricted --safe-mode, B = --restricted)
VARIANT="$(python3 -I -c 'import json,sys; print(json.load(open(sys.argv[1])).get("variant","A"))' "$REVIEW_DIR/canary.json")"
ISOLATION=(--restricted --safe-mode); [ "$VARIANT" = "B" ] && ISOLATION=(--restricted)
CMD=(timeout 1800 claude -p --model claude-sonnet-5-5 "${ISOLATION[@]}"
  --settings "$REVIEW_DIR/worker_settings.json" --tools "Read,Grep,Glob"
  --permission-mode dontAsk --max-budget-usd "${REDUCE_BUDGET_USD:-3.00}"
  --json-schema "$(cat "$REVIEW_DIR/reduce_schema.json")"
  --strict-mcp-config --mcp-config '{"mcpServers":{}}'
  --disable-slash-commands --no-session-persistence --output-format json
  --append-system-prompt "$(cat "$TMP/system_prompt.md")")

if [ "$DRY_RUN" -eq 1 ]; then
  echo "cwd: $WORKTREE"
  echo "command: timeout 1800 claude -p --model claude-sonnet-5-5 --restricted --safe-mode" \
       "--settings $REVIEW_DIR/worker_settings.json --tools Read,Grep,Glob --permission-mode dontAsk" \
       "--max-budget-usd ${REDUCE_BUDGET_USD:-3.00}" \
       "--json-schema <$(wc -c < "$REVIEW_DIR/reduce_schema.json") bytes: reduce_schema.json>" \
       "--strict-mcp-config --mcp-config '{\"mcpServers\":{}}' --disable-slash-commands" \
       "--no-session-persistence --output-format json" \
       "--append-system-prompt <$(wc -c < "$TMP/system_prompt.md") bytes: rendered reduce_prompt.md>" \
       "< user prompt on stdin"
  echo "system prompt bytes: $(wc -c < "$TMP/system_prompt.md")"
  echo "user prompt (bundle) bytes: $(wc -c < "$TMP/user_prompt.md")"
  echo "bundle findings: $(python3 -I -c 'import json,sys;print(json.load(open(sys.argv[1]))["count"])' "$TMP/bundle.json")"
  echo "would write: $RUNS_DIR/reduce_$DOMAIN.json, $REVIEWS_DIR/domain_$DOMAIN.json, $REVIEWS_DIR/domain_$DOMAIN.md"
  exit 0
fi

mkdir -p "$RUNS_DIR" "$REVIEWS_DIR"
RAW="$RUNS_DIR/reduce_$DOMAIN.json"
rc=0
(cd "$WORKTREE" && "${CMD[@]}" < "$TMP/user_prompt.md" > "$TMP/raw.json") || rc=$?
if [ -s "$TMP/raw.json" ]; then cp "$TMP/raw.json" "$RAW"; fi
if [ "$rc" -ne 0 ]; then
  echo "claude exited with $rc (raw output, if any, saved to $RAW)" >&2
  exit "$rc"
fi

# --- extract structured output -----------------------------------------------------------
OUT_JSON="$REVIEWS_DIR/domain_$DOMAIN.json"
python3 -I - "$RAW" "$OUT_JSON" "$DOMAIN" <<'PY'
import json, re, sys
raw_path, out_path, domain = sys.argv[1:4]
raw = json.load(open(raw_path, encoding="utf-8"))
if isinstance(raw, list):  # event-stream style output: take the final result event
    raw = next((e for e in reversed(raw) if isinstance(e, dict) and e.get("type") == "result"), {})
data = raw.get("structured_output") if isinstance(raw, dict) else None
if data is None and isinstance(raw, dict):
    res = raw.get("result")
    if isinstance(res, str):
        s = res.strip()
        s = re.sub(r"^```(?:json)?\s*|\s*```$", "", s)
        data = json.loads(s)
    elif isinstance(res, dict):
        data = res
if not isinstance(data, dict) or not isinstance(data.get("verdicts"), list):
    sys.exit("no usable structured output in " + raw_path)
if data.get("domain") != domain:
    sys.stderr.write(f"warning: output domain {data.get('domain')!r} != {domain!r}\n")
with open(out_path, "w", encoding="utf-8") as fh:
    fh.write(json.dumps(data, sort_keys=True, indent=1, ensure_ascii=False) + "\n")
PY
python3 -I "$REVIEW_DIR/merge.py" --render-domain "$OUT_JSON" > "$REVIEWS_DIR/domain_$DOMAIN.md"
echo "reduce $DOMAIN done: $OUT_JSON"
