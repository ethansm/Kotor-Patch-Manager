#!/usr/bin/env bash
set -e

GHIDRA_DIR="/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC"
SERVER="ghidra.openkotor.com"
PORT=13100
REPO="Odyssey"
USER="Ethansm"
VAULT_DIR="$HOME/.config/ghidra"
CRED_FILE="$VAULT_DIR/.server_credentials"

echo "========================================================"
echo " OpenKotOR Ghidra Server Credential Vault Setup"
echo " Server: $SERVER:$PORT ($REPO)"
echo " User:   $USER"
echo "========================================================"
echo ""

read -s -p "Enter Ghidra Server password for user '$USER': " PASS
echo ""

if [ -z "$PASS" ]; then
    echo "ERROR: Password cannot be empty."
    exit 1
fi

echo "Verifying credentials with $SERVER:$PORT..."

TMP_TEST=$(mktemp /tmp/test_vault_XXXXXX.jshell)
cat << 'EOF' > "$TMP_TEST"
import ghidra.framework.client.*;
String user = System.getenv("GHIDRA_USER");
String pass = System.getenv("GHIDRA_PASS");
String server = System.getenv("GHIDRA_SERVER");
int port = Integer.parseInt(System.getenv("GHIDRA_PORT"));
try {
    ClientUtil.setClientAuthenticator(new PasswordClientAuthenticator(user, pass));
    RepositoryServerAdapter rsa = ClientUtil.getRepositoryServer(server, port, true);
    rsa.connect();
    System.out.println("VAULT_AUTH_OK");
} catch (Exception e) {
    System.err.println("AUTH_ERROR: " + e.getMessage());
    System.exit(1);
}
/exit
EOF

AUTH_RESULT=$(GHIDRA_USER="$USER" GHIDRA_PASS="$PASS" GHIDRA_SERVER="$SERVER" GHIDRA_PORT="$PORT" \
    "$GHIDRA_DIR/support/jshellRun" "$TMP_TEST" 2>&1 | grep "VAULT_AUTH_OK" || true)

rm -f "$TMP_TEST"

if [ "$AUTH_RESULT" != "VAULT_AUTH_OK" ]; then
    echo "ERROR: Authentication failed on $SERVER:$PORT. Please verify your password."
    exit 1
fi

echo "Authentication successful!"

mkdir -p "$VAULT_DIR"
cat << EOF > "$CRED_FILE"
# OpenKotOR Ghidra Server Credentials
# Mode: 0600 (Strictly restricted to user: $USER)
export GHIDRA_SERVER="$SERVER"
export GHIDRA_PORT="$PORT"
export GHIDRA_REPO="$REPO"
export GHIDRA_USER="$USER"
export GHIDRA_PASS="$PASS"
EOF

chmod 600 "$CRED_FILE"

# If secret-tool is installed, also sync into system keyring
if command -v secret-tool >/dev/null 2>&1; then
    printf "%s" "$PASS" | secret-tool store --label="OpenKotOR Ghidra Server" server "$SERVER" user "$USER" 2>/dev/null || true
    echo "Synced to Linux Desktop Keyring via secret-tool."
fi

unset PASS

echo ""
echo "========================================================"
echo " SUCCESS: Credential vault configured at:"
echo "   $CRED_FILE (permissions: 0600)"
echo " Autonomous agent queries to the server API are now enabled"
echo " with zero plaintext exposure in transcripts or chats."
echo "========================================================"
