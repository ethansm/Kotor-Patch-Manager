import re,sqlite3,glob,os,collections,csv,sys
R='/home/ethansm/orca/workspaces/local-setup/KOTOR-II/'
P=R+'patch_manager_mods/ghidra_knowledge/'
c=sqlite3.connect('file:'+P+'kotor2_ghidra_knowledge.db?mode=ro',uri=True)
norm=lambda s:int(s,16)
known=set()
for (a,) in c.execute("select steam_addr from name_map where confidence!='wrong'"):
    try: known.add(norm(a))
    except: pass
for t in [r[0] for r in c.execute("select name from sqlite_master where type='table'")]:
    pass
for f in glob.glob(P+'rea_findings/*.tsv'):
    for row in csv.DictReader(open(f),delimiter='\t'):
        try: known.add(norm(row['addr']))
        except: pass
# function starts from static dump
starts={}
for row in csv.reader(open(P+'steam_functions.tsv'),delimiter='\t'):
    try: starts[norm(row[0])]=1
    except: pass
srt=sorted(starts)
import bisect
def fn_of(a):
    i=bisect.bisect_right(srt,a)-1
    return srt[i] if i>=0 else None
groups={'research_notes':[R+'.claude/research/**/*.md',R+'patch_manager_mods/*research*/**/*.md',R+'patch_manager_mods/engine_research_findings/*.md',R+'patch_manager_mods/[0-9]*.md',R+'.claude/reviews/**/*.md'],
        'mod_code':[R+'patch_manager_mods/*_src/**/*.toml',R+'patch_manager_mods/*_src/**/*.h',R+'patch_manager_mods/*_src/**/*.cpp',R+'patch_manager_mods/*_src/**/*.inc']}
rx=re.compile(r'\b0x00([4-9][0-9a-fA-F]{5})\b')
for g,pats in groups.items():
    files=set(f for p in pats for f in glob.glob(p,recursive=True) if 'dump_excerpts' not in f)
    fns=collections.Counter(); src=collections.defaultdict(set)
    for f in files:
        try: t=open(f,errors='ignore').read()
        except: continue
        for m in rx.finditer(t):
            a=int(m.group(1),16)
            if a>0x980000: continue  # data/rdata, skip
            fn=fn_of(a)
            if fn is None: continue
            fns[fn]+=1; src[fn].add(os.path.relpath(f,R).split('/')[1] if f.startswith(R+'patch_manager_mods') else os.path.relpath(f,R).split('/')[2])
    unk=[f for f in fns if f not in known]
    print(f"{g}: files={len(files)} functions cited={len(fns)} unnamed/unledgered={len(unk)}")
    top=sorted(unk,key=lambda f:-fns[f])[:12]
    for f in top: print(f"  0x{f:08x} cites={fns[f]} in={sorted(src[f])[:4]}")
