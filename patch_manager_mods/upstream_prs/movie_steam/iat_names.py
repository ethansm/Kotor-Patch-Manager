#!/usr/bin/env python3
"""Name the imported function behind IAT slots on Steam (from the exe import tables) and GOG (slot value = RVA of hint/name in the raw .rdata dump)."""
import struct, sys
from hookcheck import *
def cstr(read, va):
    b = read(va, 64); return b.split(b"\0")[0].decode()
def slot_name(read, slot):
    v = struct.unpack("<I", read(slot, 4))[0]
    return v, (cstr(read, base + v + 2) if 0 < v < 0x800000 else None)
for label, slots in [("BinkBufferBlit/BinkCopyToBuffer (+neighbours)", None)]: pass
G = {0x9854BC:"hook-blit",0x9854C8:"copy",0x9854C0:"?",0x9854C4:"?",0x9854B4:"?",0x9854B8:"?"}
S = {0x9864A0:"hook-blit",0x986494:"copy",0x98649C:"?",0x986498:"?",0x9864A8:"?",0x9864A4:"?"}
print("GOG"); 
for a in sorted(G): print(f"  {a:#x} -> {slot_name(gog_read, a)}")
print("STEAM")
for a in sorted(S): print(f"  {a:#x} -> {slot_name(steam_read, a)}")
