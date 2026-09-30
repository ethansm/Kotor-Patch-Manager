#!/bin/bash
# copy_dump_excerpts.sh ADDR... : copy decomp (.c) + disasm (.asm) of Steam functions from the full-program dump
# (~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71, see lesson 166) into ./dump_excerpts/ so evidence lives in the repo.
D=~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp
OUT="$(dirname "$0")/dump_excerpts"; mkdir -p "$OUT"
for a in "$@"; do
  a=${a#0x}; a=$(printf '%08x' 0x$a); bb=${a:2:2}
  for ext in c asm; do [ -f "$D/$bb/$a.$ext" ] && cp "$D/$bb/$a.$ext" "$OUT/$a.$ext" || echo "missing $a.$ext"; done
done
