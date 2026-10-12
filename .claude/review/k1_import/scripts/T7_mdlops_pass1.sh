#!/bin/bash
B=$HOME/Games/KOTOR2_Mod_Workspace/mdlops_t7; RUN=$B/mdl_run1
PROTON="/home/ethansm/.local/share/Steam/steamapps/common/Proton - Experimental/proton"
W=$(printf 'Z:%s' "${RUN//\//\\}")
export STEAM_COMPAT_CLIENT_INSTALL_PATH=$HOME/.local/share/Steam STEAM_COMPAT_DATA_PATH="$RUN/prefix"
cd "$RUN/work"
timeout 900 "$PROTON" run "$W\\tools\\mdlops.exe" "$W\\work\\n_commm.mdl" > "$RUN/pass1.log" 2>&1
echo "rc=$?" >> "$RUN/pass1.log"
