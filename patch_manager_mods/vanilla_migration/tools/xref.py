#!/usr/bin/env python3
"""xref.py VA : find E8/E9 rel32 calls/jumps to VA in .text and absolute 32-bit pointers to VA anywhere"""
import sys,struct
from j0o_diff import *
t=int(sys.argv[1],16)
for n,va,vs,ro,rs in secs:
    base=0x400000+va
    if n=='.text':
        for i in range(ro,ro+rs-5):
            if v[i] in (0xe8,0xe9):
                r=struct.unpack_from('<i',v,i+1)[0]
                if base+(i-ro)+5+r==t: print('call' if v[i]==0xe8 else 'jmp', hex(base+i-ro))
    for i in range(ro,ro+rs-3):
        if struct.unpack_from('<I',v,i)[0]==t: print('ptr',n,hex(base+i-ro))
