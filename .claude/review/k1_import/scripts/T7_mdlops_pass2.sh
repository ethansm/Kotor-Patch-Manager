#!/bin/bash
B=$HOME/Games/KOTOR2_Mod_Workspace/mdlops_t7; RUN=$B/mdl_run1
PROTON="/home/ethansm/.local/share/Steam/steamapps/common/Proton - Experimental/proton"
W=$(printf 'Z:%s' "${RUN//\//\\}")
export STEAM_COMPAT_CLIENT_INSTALL_PATH=$HOME/.local/share/Steam STEAM_COMPAT_DATA_PATH="$RUN/prefix"
cd "$RUN/pass2"
timeout 900 "$PROTON" run "$W\\tools\\mdlops.exe" -k2 "$W\\pass2\\n_commm-ascii.mdl" > "$RUN/pass2.log" 2>&1
echo "rc=$?" >> "$RUN/pass2.log"
