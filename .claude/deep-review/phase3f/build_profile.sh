#!/usr/bin/env bash
# Builds profile_fix_kotor2.toml = the p2c review profile + fix_section_kotor2.toml, installs the compat adapter into the
# gate-visible cache dir and validates the result. $0 (no model calls).
set -euo pipefail
D="$(cd "$(dirname "$0")" && pwd)"
B="$HOME/.claude/skills/deep-haiku-review/bundle"
mkdir -p "$HOME/.cache/deep_haiku_review/kotor2-adapters"
install -m 0644 "$D/kotor2_compat_adapter.py" "$HOME/.cache/deep_haiku_review/kotor2-adapters/kotor2_compat_adapter.py"
cat "$D/../phase3b/profile_p2c.toml" "$D/fix_section_kotor2.toml" > "$D/profile_fix_kotor2.toml"
python3 -I "$B/engine/dr_profile.py" check "$D/profile_fix_kotor2.toml"
