#!/usr/bin/env python3
"""side-by-side function neighbours: seq.py STEAM_ADDR GOG_ADDR [before] [after]  (sizes/retn/names from the local server exports)"""
import sys, bisect
D='/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/'
def load(p):
    rows=[l.rstrip('\n').split('\t') for l in open(p)][1:]
    out=[(int(r[0],16),int(r[1]),int(r[2]),(r[4]+'::' if r[4] else '')+r[3]) for r in rows]
    out.sort(); return out
S=load(D+'k2_win_steam_aspyr_swkotor2.exe/swkotor2_aspyr.exe_functions.tsv')
G=load(D+'k2_win_gog_aspyr_swkotor2.exe/swkotor2.exe_functions.tsv')
def window(L,a,b,n):
    i=bisect.bisect_left([x[0] for x in L],a)
    return L[max(0,i-b):i+n+1]
if __name__=='__main__':
    s=int(sys.argv[1],16); g=int(sys.argv[2],16)
    b=int(sys.argv[3]) if len(sys.argv)>3 else 3; n=int(sys.argv[4]) if len(sys.argv)>4 else 4
    ws=window(S,s,b,n); wg=window(G,g,b,n)
    for i in range(max(len(ws),len(wg))):
        a=ws[i] if i<len(ws) else None; c=wg[i] if i<len(wg) else None
        f=lambda x: '%08x %5d r%-3d %s'%(x[0],x[1],x[2],x[3][:42]) if x else ' '*40
        print(f(a),' | ',f(c))
