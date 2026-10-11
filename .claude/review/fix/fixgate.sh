#!/usr/bin/env bash
# fixgate.sh <root> <subcmd> [args]   -- the only command the integrator may run (via fixgate_<batch>.sh) and the
# gate runner of fix_checks.py. Contract: .claude/review/INTERFACES.md, "fixgate.sh".
#   apply <patch> | reset | diff [path] | status | build <mod_src> | test <filter> | compat [--list] <file>... | diffcheck
# <root> must be $FIX_HOME/slots/<n> or $FIX_HOME/integ/<batch> (a git clone of base). Every argument must match
# ^[A-Za-z0-9_.:/+-]+$, contain no "..", not start with "-" (except compat's literal --list) or "/"; else exit 2.
# Re-execs itself inside bwrap (FIX_SANDBOX=bwrap, default): real repo/HOME/orca/~/.config/~/.ssh/~/Games hidden,
# no network, scratch ledger. FIX_SANDBOX=none only for self-tests.
set -euo pipefail

die() { echo "fixgate: $*" >&2; exit 2; }

[[ $# -ge 2 ]] || die "usage: fixgate.sh <root> apply|reset|diff|status|build|test|compat|diffcheck [args]"
ROOT_ARG=$1
SUB=$2
shift 2

RE='^[A-Za-z0-9_.:/+-]+$'
bad_arg() {  # $1 = value
  [[ $1 =~ $RE ]] || return 0
  [[ $1 == *..* ]] && return 0
  return 1
}

bad_arg "$ROOT_ARG" && die "bad root '$ROOT_ARG'"
[[ -d $ROOT_ARG ]] || die "root '$ROOT_ARG' is not a directory"
ROOT=$(cd "$ROOT_ARG" && pwd -P)
[[ -d $ROOT/.git && ! -L $ROOT/.git ]] || die "root '$ROOT' is not a git clone"
PARENT=$(dirname "$ROOT")
FIX_HOME=$(dirname "$PARENT")
case $(basename "$PARENT") in slots | integ) ;; *) die "root must be \$FIX_HOME/slots/<n> or \$FIX_HOME/integ/<batch>" ;; esac
[[ $(basename "$FIX_HOME") == fix ]] || die "FIX_HOME '$FIX_HOME' is not .../fix"
NAME=$(basename "$ROOT")
[[ $NAME =~ ^[A-Za-z0-9_.-]+$ ]] || die "bad root name"
RH=$(dirname "$FIX_HOME")

for a in "$@"; do
  if [[ $SUB == compat && $a == --list ]]; then continue; fi
  bad_arg "$a" && die "bad argument '$a'"
  [[ $a == -* || $a == /* ]] && die "argument '$a' must be relative and not an option"
done

case $SUB in
  apply | build | test) [[ $# -eq 1 ]] || die "$SUB takes exactly one argument" ;;
  reset | status | diffcheck) [[ $# -eq 0 ]] || die "$SUB takes no arguments" ;;
  diff) [[ $# -le 1 ]] || die "diff takes at most one path" ;;
  compat) [[ $# -ge 1 ]] || die "compat needs at least one file" ;;
  *) die "unknown subcommand '$SUB'" ;;
esac
if [[ $SUB == apply ]]; then
  [[ -f $ROOT/$1 && ! -L $ROOT/$1 ]] || die "patch '$1' is not a regular file under the root"
fi

SELF=$(readlink -f "${BASH_SOURCE[0]}")
HERE=$(dirname "$SELF")
LEDGER_DIR=$FIX_HOME/ledger
TMPD=$FIX_HOME/tmp/$NAME

# ---- outer phase: set up scratch dirs, then re-exec inside the sandbox
if [[ ${FIX_SANDBOX_INNER:-0} != 1 ]]; then
  mkdir -p "$LEDGER_DIR" "$TMPD"
  : > "$LEDGER_DIR/$NAME.jsonl"
  if [[ ${FIX_SANDBOX:-bwrap} != none ]]; then
    command -v bwrap > /dev/null || die "bwrap not installed (FIX_SANDBOX=none is for self-tests only)"
    BW=()
    while IFS= read -r -d '' x; do BW+=("$x"); done < <(KOTOR2_REVIEW_HOME=$RH python3 -I "$HERE/fix_settings.py" bwrap-argv gate "$ROOT" "$LEDGER_DIR" "$TMPD")
    [[ ${#BW[@]} -gt 0 ]] || die "empty bwrap argv"
    exec bwrap "${BW[@]}" --ro-bind "$SELF" /tmp/.fixgate.sh -- env FIX_SANDBOX_INNER=1 FIX_SANDBOX=bwrap \
      bash /tmp/.fixgate.sh "$ROOT" "$SUB" "$@"
  fi
fi

# ---- inner phase: run the command with a scrubbed environment
G=(git -c core.hooksPath=/dev/null -c core.fsmonitor=false -c core.autocrlf=false -C "$ROOT")
GENV=(GIT_CONFIG_GLOBAL=/dev/null GIT_CONFIG_NOSYSTEM=1 XDG_CONFIG_HOME=/nonexistent-xdg GIT_TERMINAL_PROMPT=0
  GIT_AUTHOR_NAME=fix-pipeline GIT_AUTHOR_EMAIL=fix-pipeline@localhost GIT_COMMITTER_NAME=fix-pipeline
  GIT_COMMITTER_EMAIL=fix-pipeline@localhost)
TENV=(PATH="$PATH" HOME="${HOME:-/nonexistent}" LANG=C.UTF-8 TERM=dumb "TMPDIR=$TMPD"
  "MASTER_LEDGER_PATH=$LEDGER_DIR/$NAME.jsonl" MASTER_LEDGER_TEST=1 KOTOR2_ROOT=/nonexistent-kotor2
  PYTHONDONTWRITEBYTECODE=1 PYTHONNOUSERSITE=1)
if [[ -n ${KPM_DIR:-} ]]; then TENV+=("KPM_DIR=$KPM_DIR"); fi

gitc() { env "${GENV[@]}" "${G[@]}" "$@"; }
tool() { (cd "$ROOT" && env -i "${TENV[@]}" "$@"); }

rc=0
case $SUB in
  apply)
    gitc apply --3way --whitespace=nowarn "$ROOT/$1" || rc=$?
    ;;
  reset)
    base=$(gitc rev-list --max-parents=0 HEAD | tail -n 1)
    gitc checkout -f -q --detach "$base"
    gitc reset -q --hard "$base"
    gitc clean -fdxq -e _patches -e _resolved   # integrator inputs/outputs survive a reset (bug 13)
    ;;
  diff)
    if [[ $# -eq 1 ]]; then gitc diff HEAD --no-ext-diff --no-color -- "$1"; else gitc diff HEAD --no-ext-diff --no-color; fi
    ;;
  status)
    gitc status --short
    ;;
  build)
    mod=${1#patch_manager_mods/}
    rm -rf "$TMPD/build"
    tool python3 "$ROOT/scripts/build_check.py" --mod "$mod" --repo "$ROOT" --build-root "$TMPD/build" || rc=$?
    ;;
  test)
    rm -f "$TMPD/rt.json"
    tool python3 "$ROOT/scripts/run_tests.py" "$1" --jobs 4 --json "$TMPD/rt.json" || rc=$?
    if [[ -f $TMPD/rt.json ]]; then
      python3 -I -c 'import json,sys
d=json.load(open(sys.argv[1])); c=d.get("counts",{})
bad=[r["id"] for r in d.get("results",[]) if r.get("status") in ("FAIL","ERROR")]
print("fixgate-summary: counts=%s failing=%s" % (json.dumps(c, sort_keys=True), json.dumps(bad[:30])))' "$TMPD/rt.json" || true
    fi
    ;;
  compat)
    rm -f "$TMPD/compat.json"
    tool python3 "$ROOT/scripts/compat_check.py" --repo "$ROOT" --jobs 4 --json "$TMPD/compat.json" "$@" || rc=$?
    ;;
  diffcheck)
    tool python3 "$ROOT/scripts/agent_diff_check.py" --no-patch --json || rc=$?
    ;;
esac
exit "$rc"
