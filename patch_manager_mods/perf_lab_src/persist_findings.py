#!/usr/bin/env python3
# Persist a static batch report into the knowledge DB (run from patch_manager_mods/):
#   python3 perf_lab_src/persist_findings.py U2 7054ce,705559   (2nd arg = instruction addresses Opus verified; rows quoting them get confidence high)
# Parses the "| addr | name | confidence | purpose | evidence |" table of engine_research_findings/<batch>.md; inserts name_map
# (upgrades an existing row only to high when verified) and functions rows (only when the address has none). Source = <batch>-Phase4.
import sqlite3,re,sys
H='6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef'
u=sys.argv[1]; ver=sys.argv[2].split(',')
c=sqlite3.connect('ghidra_knowledge/kotor2_ghidra_knowledge.db')
st={'new':0,'upd':0,'skip':0,'fn':0}
for l in open(f'engine_research_findings/{u}.md'):
    m=re.match(r'^\|\s*(0x)?([0-9A-Fa-f]{6,8})\s*\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*$',l)
    if not m: continue
    a=int(m.group(2),16); name,conf,purp,ev=m.group(3),m.group(4).lower(),m.group(5),m.group(6)
    v=any(x in ev.lower() for x in ver)
    conf='high' if v else ('low' if conf.startswith('low') else 'medium')
    key=f'0x{a:08x}'; evid=(purp+' | '+ev)[:2000]
    old=c.execute('select name,source,confidence from name_map where steam_addr=?',(key,)).fetchone()
    if not old: c.execute('insert into name_map values(?,?,?,?,?,?)',(key,name,f'{u}-Phase4',conf,evid,'2026-09-30')); st['new']+=1
    elif conf=='high' and old[2]!='high': c.execute('update name_map set name=?,source=?,confidence=?,evidence=?,date=? where steam_addr=?',(name,f'{u}-Phase4',conf,f'(was {old[0]} / {old[1]}) '+evid,'2026-09-30',key)); st['upd']+=1
    else: st['skip']+=1
    if not c.execute('select 1 from functions where binary_hash=? and address=?',(H,f'{a:08x}')).fetchone():
        c.execute('insert into functions values(?,?,?,?,?,?,?)',(H,f'{a:08x}',name,None,purp,f'{u}-Phase4 ({conf}{", Opus-verified" if v else ""}): {ev}'[:2000],'2026-09-30')); st['fn']+=1
c.commit(); print(u,st)
