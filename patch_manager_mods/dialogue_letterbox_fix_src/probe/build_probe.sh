#!/usr/bin/env bash
# Build k2-dialogue-letterbox-fix 0.1.0-probe (same source as the shipping fix, DLB_PROBE via probe/dlb_variant.h).
# The probe reuses the patch id on purpose, and validate-patches FAILs on two catalog dirs with one id, so the build dir
# is created under Patches/ (create-patch needs ../Common) and removed again on exit. Never leave it behind.
#   Usage: bash build_probe.sh            (F=<kotor-patch-manager checkout> overrides the default)
#   Output: $F/letterbox_probe_stage/0.1.0-probe/K2DialogueLetterboxFixProbe.kpatch
set -euo pipefail
F="${F:-$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"   # .../dialogue_letterbox_fix_src/probe
SRC="$(dirname "$HERE")"                               # .../dialogue_letterbox_fix_src
BUILD="$F/Patches/K2DialogueLetterboxFixProbe.build"
OUT="$F/letterbox_probe_stage/0.1.0-probe"

[ -d "$F/Patches/Common" ] || { echo "no $F/Patches/Common (set F=<kpm checkout>)" >&2; exit 1; }
[ ! -e "$BUILD" ] || { echo "$BUILD already exists (another build running?)" >&2; exit 1; }
trap 'rm -rf "$BUILD"' EXIT
mkdir "$BUILD"
cp "$SRC/K2DialogueLetterboxFix.cpp" "$BUILD/"
cp "$HERE/dlb_variant.h" "$HERE/manifest.toml" "$HERE/kotor2-steam-aspyr.hooks.toml" "$HERE/exports.def" "$BUILD/"
mkdir -p "$OUT"
# create-patch names the package after its positional argument (default: the build dir's name), so pass it explicitly.
# dlb_variant.h sits next to the .cpp, so `__has_include("dlb_variant.h")` finds it without any -I.
(cd "$BUILD" && python3 ../create-patch.py K2DialogueLetterboxFixProbe --targets windows_x86 -o "$OUT")
echo "built: $OUT/K2DialogueLetterboxFixProbe.kpatch"
