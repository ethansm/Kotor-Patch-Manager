#!/bin/bash
# asm.sh FUNC_ADDR LO HI : asm lines of function FUNC_ADDR between LO and HI (hex, no 0x)
D=~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp
f=$(printf "%08x" 0x$1); bb=${f:2:2}
python3 -c "
import sys
lo,hi=int('$2',16),int('$3',16)
for l in open('$D/$bb/$f.asm'):
    t=l.split(None,1)
    try: a=int(t[0],16)
    except: continue
    if lo<=a<=hi: print(l.rstrip())
"
