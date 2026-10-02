import sys; sys.path.insert(0,'/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_pr3/work')
from lib import *
from offsets_final import OE
import json,csv,collections,sqlite3
rows=json.load(open('final_rows.json'))
SD='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-contributing/AddressDatabases/kotor2_steam_aspyr.db'
sdb=sqlite3.connect(SD)
steam_f={(c,n) for c,n in sdb.execute('select class_name,function_name from functions')}
steam_o={(c,n) for c,n in sdb.execute('select class_name,member_name from offsets')}
gdb=sqlite3.connect(GOGDB)
gog_f={(c,n) for c,n in gdb.execute('select class_name,function_name from functions')}
gog_o={(c,n) for c,n in gdb.execute('select class_name,member_name from offsets')}
inv_all=inv()
# ---- verified.tsv
cols=['kind','class','name','gog','steam_addr','verdict','calling_convention','param_size_bytes','evidence','cand_orig','cand_status','content_level','wrapper_issue','steam_size','gog_param_size']
out=[]
for d in rows:
    ev=d['evidence']
    out.append(['F',d['cls'],d['name'],d['gog'],d['steam_addr'],d['verdict'],d['cc'],d['param'],ev.replace('\t',' ').replace('\n',' '),d['cand_orig'],d['status'],d['content'],d['wrapper_issue'],d['steam_size'],d['gog_param']])
offrows=[]
for r in inv_all:
    if r['kind']=='O' and r['gog']:
        k=(r['class'],r['name']); v,ev=OE[k]
        offrows.append(['O',r['class'],r['name'],r['gog'],r['gog'],v,'','',ev,'','confirmed' ,'','', '',''])
out+=offrows
with open('../verified.tsv','w') as f:
    f.write('\t'.join(cols)+'\n')
    for o in out: f.write('\t'.join(str(x) for x in o)+'\n')
fc=collections.Counter(d['verdict'] for d in rows); oc=collections.Counter(o[5] for o in offrows)
print('F',fc,'O',oc)
# ---- coverage
vd={(d['cls'],d['name']):d['verdict'] for d in rows}
od={(o[1],o[2]):o[5] for o in offrows}
inv_oos={(r['kind'],r['class'],r['name']) for r in inv_all if not r['gog']}
W=list(csv.DictReader(open('wrapper_lookups.tsv'),delimiter='\t'))
inv_classes={r['class'] for r in inv_all}
groups=collections.OrderedDict()
for w in W:
    groups.setdefault((w['file'],w['class'],w['initfn']),[]).append(w)
def status(w):
    k=w['kind']; c=w['lookup_class']; n=w['name']
    if k=='F':
        if (c,n) in steam_f: return 'present'
        if (c,n) in vd: return vd[(c,n)]
        if ('F',c,n) in inv_oos: return 'gogmissing'
        return 'gogmissing' if (c,n) not in gog_f else 'notinventoried'
    if k=='O':
        if (c,n) in steam_o: return 'present'
        if (c,n) in od: return od[(c,n)]
        if ('O',c,n) in inv_oos: return 'gogmissing'
        return 'gogmissing' if (c,n) not in gog_o else 'notinventoried'
    return 'gogmissing'   # classes table is empty in both DBs
cov=[]
for (file,cls,initfn),L in groups.items():
    sts=[status(w) for w in L]
    if not any(( (w['kind']=='F' and (w['lookup_class'],w['name']) in vd) or (w['kind']=='O' and (w['lookup_class'],w['name']) in od)) for w in L): continue
    n=len(L)
    def cnt(x): return sum(1 for s in sts if s==x)
    inscope=cnt('verified')+cnt('likely')+cnt('rejected')+cnt('unresolved')
    blocking=[f"{w['lookup_class']}.{w['name']}({s})" for w,s in zip(L,sts) if s in('rejected','unresolved','likely')]
    gogmiss=[f"{w['lookup_class']}.{w['name']}" for w,s in zip(L,sts) if s=='gogmissing']
    ok_v=lambda s: s in('present','verified')
    ok_vl=lambda s: s in('present','verified','likely')
    def prefix(pred):
        for i,s in enumerate(sts):
            if not pred(s): return i
        return n
    gog_prefix=prefix(lambda s: s!='gogmissing')   # lookups GOG would assign before its first miss
    pv=prefix(ok_v); pvl=prefix(ok_vl)
    full_v='Y' if pv==n else 'N'; full_vl='Y' if pvl==n else 'N'
    par_v='Y' if pv>=gog_prefix else 'N'; par_vl='Y' if pvl>=gog_prefix else 'N'
    firstblock_v=next((f"{w['lookup_class']}.{w['name']}({s})" for w,s in zip(L,sts) if not ok_v(s)),'')
    cov.append([f"{cls}::{initfn}",file,n,inscope,cnt('present'),cnt('verified'),cnt('likely'),cnt('rejected'),cnt('unresolved'),cnt('gogmissing'),full_v,full_vl,gog_prefix,pv,pvl,par_v,par_vl,firstblock_v,';'.join(blocking[:12]),';'.join(gogmiss[:8])])
hdr=['wrapper_init','file','total_lookups','in_scope(gog+steam-missing)','already_in_steam_db','verified','likely','rejected','unresolved','gog_missing(out_of_scope)','full_init_verified_only','full_init_verified+likely','gog_assigns_before_first_miss','steam_assigns_verified_only','steam_assigns_verified+likely','gog_parity_verified_only','gog_parity_verified+likely','first_blocking_row_verified_only','blocking_rows(likely/rejected/unresolved)','gog_missing_rows']
with open('../class_coverage.tsv','w') as f:
    f.write('\t'.join(hdr)+'\n')
    for c in cov: f.write('\t'.join(str(x) for x in c)+'\n')
for c in cov: print(c[0],'tot',c[2],'inscope',c[3],'v',c[5],'l',c[6],'rej',c[7],'unr',c[8],'gogmiss',c[9],'full',c[10],c[11],'prefix gog/v/vl',c[12],c[13],c[14],'parity',c[15],c[16],'|',c[17])

print('--- notinventoried')
for (file,cls,initfn),L in groups.items():
    if not any(((w['kind']=='F' and (w['lookup_class'],w['name']) in vd) or (w['kind']=='O' and (w['lookup_class'],w['name']) in od)) for w in L): continue
    for w in L:
        if status(w)=='notinventoried': print(file,initfn,w['kind'],w['lookup_class'],w['name'])
