#!/usr/bin/env bash
# install_echoes_compatch.sh - Turnkey Linux shell wrapper for Echoes of the Republic compatch
#
# Runs install_echoes_compatch.py with the python of a venv that has the PATCHED
# pykotor (../pykotor_patches/README.md). It never installs pykotor itself: stock
# pykotor 2.3.12 from pip blanks some 2DA tables when it reads them, and phase 2
# would write those blanks into the game.
#
#   KOTOR2_PY=/path/to/venv/bin/python3 ./install_echoes_compatch.sh [options]
#
# Without KOTOR2_PY it uses $VENV_DIR/bin/python3 (VENV_DIR defaults to ../.venv).
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(dirname "$SCRIPT_DIR")"
VENV_DIR="${VENV_DIR:-$REPO_DIR/.venv}"

echo "=== Star Wars Knights of the Old Republic II: Echoes of the Republic Compatch ==="

if [ -n "${KOTOR2_PY:-}" ]; then
    PY="$KOTOR2_PY"
elif [ -x "$VENV_DIR/bin/python3" ]; then
    PY="$VENV_DIR/bin/python3"
else
    echo "[ERROR] No patched pykotor venv found at $VENV_DIR." >&2
    echo "        Set one up as described in $REPO_DIR/pykotor_patches/README.md, then run with" >&2
    echo "        KOTOR2_PY=/path/to/that/venv/bin/python3 (or VENV_DIR=/path/to/that/venv)." >&2
    exit 1
fi

# The installer also checks that this pykotor is the patched one before any phase.
exec "$PY" "$SCRIPT_DIR/install_echoes_compatch.py" "$@"
