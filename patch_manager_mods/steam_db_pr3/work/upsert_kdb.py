import sys,json,sqlite3,re; sys.path.insert(0,'.')
from lib import *
from offsets_final import OE
DRY = '--apply' not in sys.argv
rows=json.load(open('final_rows.json'))
db=sqlite3.connect(KDB)
DATE='2026-10-02'
SRC='steam-db-pr3-phase2'
MAPPER_SRC=('gog_cluster_delta_map.py','R9-Phase2 (GOG delta/size + mapper, Opus spot-checked)','gog_delta+0xF66A0')
ins=upd=keep=dele=0; conflicts=[]
for d in rows:
    nm=f"{d['cls']}::{d['name']}"
    # stale mapper row at the rejected/original candidate
    if d['cand_orig'] and d['cand_orig']!=d['steam_addr']:
        r=db.execute('select name,source,confidence from name_map where steam_addr=?',(d['cand_orig'],)).fetchone()
        if r and r[0].split(' ')[0]==nm and (r[1] in MAPPER_SRC or r[1].startswith('gog_')):
            dele+=1
            if not DRY: db.execute('delete from name_map where steam_addr=?',(d['cand_orig'],))
    if d['verdict'] not in ('verified','likely') or not d['steam_addr']: continue
    conf='high' if d['verdict']=='verified' else 'medium'
    ev=f"[{d['verdict']}; {d['cc']} retn={d['steam_retn']}; GOG {d['gog']}] "+d['evidence'][:420]
    r=db.execute('select name,source,confidence,evidence from name_map where steam_addr=?',(d['steam_addr'],)).fetchone()
    if r is None:
        ins+=1
        if not DRY: db.execute('insert into name_map values(?,?,?,?,?,?)',(d['steam_addr'],nm,SRC,conf,ev,DATE))
    else:
        same=r[0].split(' ')[0].split('(')[0]==nm or r[0].startswith(nm)
        if same:
            if r[1] in MAPPER_SRC or r[2]!='high' or conf=='high' and r[2]!='high':
                upd+=1
                if not DRY: db.execute('update name_map set source=?,confidence=?,evidence=?,date=? where steam_addr=?',(SRC,conf,ev,DATE,d['steam_addr']))
            else: keep+=1
        else:
            if r[1] in MAPPER_SRC or r[1].startswith('gog_'):
                upd+=1; conflicts.append((d['steam_addr'],r[0],'->',nm,'replaced mapper row'))
                if not DRY: db.execute('update name_map set name=?,source=?,confidence=?,evidence=?,date=? where steam_addr=?',(nm,SRC,conf,ev,DATE,d['steam_addr']))
            else:
                keep+=1; conflicts.append((d['steam_addr'],r[0],r[1],'vs mine',nm,'KEPT existing (non-mapper source)'))
print('DRY' if DRY else 'APPLIED','insert',ins,'update',upd,'keep',keep,'delete_stale',dele)
for c in conflicts: print(c)
# offsets -> struct_fields
oi=ou=0
for (cls,name),(v,ev) in OE.items():
    import csv
    g=[r for r in inv() if r['kind']=='O' and r['class']==cls and r['name']==name][0]['gog']
    off=int(g,16)
    r=db.execute('select name,source from struct_fields where class=? and offset=?',(cls,off)).fetchone()
    if r: ou+=1; print('struct_fields exists',cls,hex(off),r); continue
    oi+=1
    if not DRY: db.execute('insert into struct_fields values(?,?,?,?,?,?,?,?,?,?,?)',(cls,off,None,None,name,'high' if v=='verified' else 'medium',ev[:500],None,None,'steam-db-pr3-phase3',DATE))
print('struct_fields insert',oi,'exists',ou)
if not DRY: db.commit()
