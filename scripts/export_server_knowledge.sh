#!/usr/bin/env bash
set -e

GHIDRA_DIR="/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC"
SERVER="ghidra.openkotor.com"
PORT=13100
REPO="Odyssey"
USER="Ethansm"
SCRIPT_PATH="/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts"
OUTPUT_DIR="/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server"

echo "========================================================"
echo " OpenKotOR Server Knowledge Exporter & Auditor"
echo " Server: $SERVER:$PORT ($REPO)"
echo " User:   $USER"
echo "========================================================"
echo ""

mkdir -p "$OUTPUT_DIR"

CRED_FILE="$HOME/.config/ghidra/.server_credentials"
if [ -z "$GHIDRA_PASS" ] && [ -f "$CRED_FILE" ]; then
    # Load from protected config file (~/.config/ghidra/.server_credentials)
    source "$CRED_FILE"
elif [ -z "$GHIDRA_PASS" ] && command -v secret-tool >/dev/null 2>&1; then
    # Attempt lookup from GNOME / FreeDesktop Keyring
    GHIDRA_PASS=$(secret-tool lookup server "$SERVER" user "$USER" 2>/dev/null || true)
fi

if [ -z "$GHIDRA_PASS" ]; then
    read -s -p "Enter Ghidra Server password for user '$USER': " GHIDRA_PASS
    echo ""
fi

if [ -z "$GHIDRA_PASS" ]; then
    echo "ERROR: Password cannot be empty."
    exit 1
fi

echo "Connecting to ghidra://$SERVER:$PORT/$REPO/TSL/ and exporting symbols..."
echo "Target: k2_win_steam_aspyr_swkotor2.exe"

printf "%s\n" "$GHIDRA_PASS" | "$GHIDRA_DIR/support/analyzeHeadless" \
    "ghidra://$SERVER:$PORT/$REPO/TSL" \
    -process "k2_win_steam_aspyr_swkotor2.exe" \
    -readOnly -noanalysis \
    -connect "$USER" -p \
    -scriptPath "$SCRIPT_PATH" \
    -postScript ExportProgramSymbols.java "$OUTPUT_DIR"

echo ""
echo "Target: k2_win_gog_aspyr_swkotor2.exe"

printf "%s\n" "$GHIDRA_PASS" | "$GHIDRA_DIR/support/analyzeHeadless" \
    "ghidra://$SERVER:$PORT/$REPO/TSL" \
    -process "k2_win_gog_aspyr_swkotor2.exe" \
    -readOnly -noanalysis \
    -connect "$USER" -p \
    -scriptPath "$SCRIPT_PATH" \
    -postScript ExportProgramSymbols.java "$OUTPUT_DIR"

unset GHIDRA_PASS

echo ""
echo "========================================================"
echo " Symbols successfully exported to $OUTPUT_DIR"
echo " Running comparative audit against local knowledge base..."
echo "========================================================"
echo ""

python3 "$(dirname "$0")/audit_server_knowledge.py"
