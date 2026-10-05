#!/usr/bin/env bash
set -euo pipefail

HANDOFF_FILE=".claude/session_handoff.md"
mkdir -p .claude
# Keep the previous handoff (its scratchpad/dead-ends are hand-written and not regenerable)
if [ -s "$HANDOFF_FILE" ]; then cp "$HANDOFF_FILE" "${HANDOFF_FILE%.md}.prev.md"; fi

# Sibling repos where most mod/DB code actually lives (CLAUDE.md)
EXTRA_REPOS=(
  "$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"
  "$HOME/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db"
)

BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "detached")
COMMIT=$(git log -n 1 --oneline 2>/dev/null || echo "no commits")
WORKTREE=$(pwd)

# Rebase / Merge state detection
GIT_STATE="clean"
# .git is a file in worktrees, so resolve paths via git itself
gp() { git rev-parse --git-path "$1" 2>/dev/null; }
if [ -d "$(gp rebase-apply)" ] || [ -d "$(gp rebase-merge)" ]; then
  GIT_STATE="mid-rebase"
elif [ -f "$(gp MERGE_HEAD)" ]; then
  GIT_STATE="mid-merge"
elif [ -f "$(gp CHERRY_PICK_HEAD)" ]; then
  GIT_STATE="mid-cherry-pick"
fi

# Filter out sensitive patterns per Credential Isolation Policy
DIRTY_RAW=$(git status -s 2>/dev/null || true)
if [ -n "$DIRTY_RAW" ]; then
  DIRTY_COUNT=$(echo "$DIRTY_RAW" | wc -l | tr -d ' ')
  DIRTY_FILES=$(echo "$DIRTY_RAW" | grep -v -E "credential|secret" || true)
else
  DIRTY_COUNT=0
  DIRTY_FILES="(none)"
fi

DIFF_STAT=$(git diff HEAD --stat 2>/dev/null | grep -v -E "credential|secret" || true)
if [ -z "$DIFF_STAT" ]; then
  DIFF_STAT="(no uncommitted diffs)"
fi

EXTRA=""
for r in "${EXTRA_REPOS[@]}"; do
  [ -d "$r" ] || continue
  rb=$(git -C "$r" rev-parse --abbrev-ref HEAD 2>/dev/null || echo "?")
  rh=$(git -C "$r" log -n 1 --oneline 2>/dev/null || echo "?")
  rd=$(git -C "$r" status -s 2>/dev/null | grep -v -E "credential|secret" || true)
  rn=$(printf '%s' "$rd" | grep -c . || true)
  EXTRA+=$'\n'"### \`$r\`"$'\n'"- Branch \`$rb\`, HEAD \`$rh\`, $rn dirty"$'\n'
  [ "$rn" -gt 0 ] && EXTRA+=$'```\n'"$rd"$'\n```\n'
done
[ -z "$EXTRA" ] && EXTRA="(none found)"

cat <<EOF > "$HANDOFF_FILE"
# Session Handoff & State Snapshot
- **Timestamp**: $(date -Iseconds)
- **Branch**: \`$BRANCH\`
- **HEAD**: \`$COMMIT\`
- **Worktree**: \`$WORKTREE\`
- **Git Lifecycle State**: \`$GIT_STATE\`

---

## 1. Uncommitted & Dirty Files ($DIRTY_COUNT files)
\`\`\`
$DIRTY_FILES
\`\`\`

## 2. Diff Summary
\`\`\`
$DIFF_STAT
\`\`\`

## 2b. Sibling Repos
$EXTRA

## 3. Active Scratchpad & Mental Model
<!-- Filled by Lead Orchestrator -->

## 4. Dead Ends & Anti-Patterns (Do NOT Attempt)
<!-- Filled by Lead Orchestrator -->

## 5. Verification Commands
<!-- Filled by Lead Orchestrator -->
EOF

echo "Workspace snapshot written to $HANDOFF_FILE ($DIRTY_COUNT dirty files, state: $GIT_STATE)."
