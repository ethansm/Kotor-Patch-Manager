#!/usr/bin/env bash
# Build a pykotor 2.3.12 venv that can WRITE KOTOR 2 (TSL) binary MDL files.
# Usage: patch_manager_mods/k1_port/setup_venv.sh
# Override the venv location with K1PORT_VENV=/path/to/venv.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PATCH_FILE="${SCRIPT_DIR}/pykotor_k2_mdl.patch"
VENV="${K1PORT_VENV:-$HOME/Games/KOTOR2_Mod_Workspace/venvs/k1port}"
PYKOTOR_VERSION="2.3.12"

if [[ ! -f "$PATCH_FILE" ]]; then
    echo "ERROR: patch file missing: $PATCH_FILE" >&2
    exit 1
fi

if [[ ! -x "$VENV/bin/python" ]]; then
    echo "Creating venv: $VENV"
    mkdir -p "$(dirname "$VENV")"
    python3 -m venv "$VENV"
fi

echo "Installing pykotor==${PYKOTOR_VERSION} and py7zr into $VENV"
"$VENV/bin/python" -m pip install --quiet "pykotor==${PYKOTOR_VERSION}" py7zr

SITE="$("$VENV/bin/python" -I -c 'import sysconfig; print(sysconfig.get_paths()["purelib"])')"

if patch -p1 --dry-run -R -d "$SITE" -i "$PATCH_FILE" >/dev/null 2>&1; then
    echo "Patch already applied, skipping: $SITE"
else
    patch -p1 --no-backup-if-mismatch -d "$SITE" -i "$PATCH_FILE"
    echo "Patch applied under $SITE: pykotor/resource/formats/mdl/io_mdl.py, mdl_data.py"
fi

echo "VENV_PYTHON=$VENV/bin/python"
