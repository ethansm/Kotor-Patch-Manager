#!/usr/bin/env python3
"""For each Steam hook site: confirm original_bytes vs Steam dump, find the aligned GOG address, compare bytes + print context."""
import sys, align
ss = align.parse(sys.argv[1]); gs = align.parse(sys.argv[2])
pairs = {k: (s, g) for k, s, g in align.load_pairs('pairs_r1.tsv') if k != 'TextColorAlt'}
HOOKS = [  # addr, original_bytes, enclosing pair key
 (0x74F25A, '89 85 44 FF FF FF', 'UpdatePortraits'),
 (0x74F8AB, '89 95 A4 FE FF FF', 'UpdatePortraits'),
 (0x745774, '89 4D FC 8B 45 FC', 'SlotDrawGate'),
 (0x745A84, '8B 4D E0 8B 11 8B 4D E0 8B 42 38 FF D0', 'RowDraw'),
 (0x745A97, '0F B6 4D EF 03 4D C4', 'RowDraw'),
 (0x745AEC, '8D 55 D0 52 8B 45 E0', 'RowDraw'),
 (0x744EEC, '8B 4D F4 64 89 0D 00 00 00 00', 'TooltipText'),
]
for addr, hexs, key in HOOKS:
    ob = bytes.fromhex(hexs.replace(' ', ''))
    s0, g0 = pairs[key]
    sf = align.fn_for(ss, s0); gf = align.fn_for(gs, g0)
    S, G = sf['ins'], gf['ins']
    idx = next((i for i, x in enumerate(S) if x[0] == addr), None)
    print('=== hook %08X  (%s %08x -> %08x)' % (addr, key, s0, g0))
    if idx is None: print('  NOT an instruction boundary in Steam dump'); continue
    # collect whole instructions covering len(ob)
    n = 0; j = idx; joined = b''
    while len(joined) < len(ob): joined += S[j][1]; j += 1
    print('  steam bytes@site: %s  == original_bytes: %s  (%d insn, %d bytes)' % (joined.hex(' '), joined[:len(ob)] == ob, j - idx, len(joined)))
    gj = b''; k = idx
    while len(gj) < len(ob): gj += G[k][1]; k += 1
    print('  GOG aligned addr: %08x  bytes: %s  byte-identical: %s' % (G[idx][0], gj.hex(' '), gj == joined))
    for i in range(max(0, idx - 4), min(len(S), j + 3)):
        mark = '>>' if idx <= i < j else '  '
        print('  %s S %08x %-22s %-40s | G %08x %-22s %s' % (mark, S[i][0], S[i][1].hex(' '), S[i][2][:40], G[i][0], G[i][1].hex(' '), G[i][2][:40]))
