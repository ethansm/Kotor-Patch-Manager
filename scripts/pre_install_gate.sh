#!/usr/bin/env bash
# Pre-install gate (doc 20 §4.2): see scripts/pre_install_gate.py.
exec "$(dirname "$0")/../.venv/bin/python" "$(dirname "$0")/pre_install_gate.py" "$@"
