"""R10: for each candidate window (va:len) verify (1) it starts and ends on instruction boundaries in a linear disasm of its function,
(2) no intra-function jmp/jcc/loop target lands strictly inside the window (after its first byte), (3) the stolen bytes are EAX/flags-free summary.
usage: R10_window_check.py FUNC_LO FUNC_HI va:len [va:len ...]"""
import sys,os
sys.path.insert(0,os.path.dirname(__file__))
import R10_pe as P, capstone
from capstone.x86 import *
lo=int(sys.argv[1],16); hi=int(sys.argv[2],16)
md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32); md.detail=True
ins=list(md.disasm(P.rd(lo,hi-lo),lo)); starts={i.address for i in ins}
tg={}
for i in ins:
    if X86_GRP_JUMP in i.groups and i.operands and i.operands[0].type==X86_OP_IMM: tg.setdefault(i.operands[0].imm,[]).append(i.address)
for w in sys.argv[3:]:
    a,n=w.split(':'); a=int(a,16); n=int(n); e=a+n
    ok_start=a in starts; ok_end=(e in starts) or e==hi
    inside=[(hex(t),[hex(x) for x in f]) for t,f in tg.items() if a<t<e]
    win=[i for i in ins if a<=i.address<e]
    eax=any('eax' in (i.op_str or '') or 'al' in (i.op_str or '').split(',')[0:1] for i in win)
    print(f'0x{a:X}:{n} start_on_boundary={ok_start} end_on_boundary={ok_end} inbound_jumps_into_middle={inside or "none"} target_of_jump_at_start={[hex(x) for x in tg.get(a,[])] or "none"}')
    for i in win: print(f'     {i.address:08x} {i.bytes.hex(" "):<24} {i.mnemonic} {i.op_str}')
    print('     mentions eax/al in window:',eax)
