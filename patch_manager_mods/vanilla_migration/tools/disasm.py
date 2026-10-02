#!/usr/bin/env python3
"""disasm.py START_VA END_VA [vanilla|j0o]  (needs capstone from repo .venv)"""
import sys
from j0o_diff import *
from capstone import *
a=int(sys.argv[1],16); b=int(sys.argv[2],16); which=sys.argv[3] if len(sys.argv)>3 else 'vanilla'
d=v if which=='vanilla' else j
md=Cs(CS_ARCH_X86,CS_MODE_32)
f=va2f(a)
for i in md.disasm(d[f:f+(b-a)],a):
    print('%08x  %-24s %s %s'%(i.address,i.bytes.hex(' '),i.mnemonic,i.op_str))
