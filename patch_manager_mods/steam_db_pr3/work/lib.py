import csv, re, sqlite3, os, bisect, collections
BASE='/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/'
W=BASE+'steam_db_pr3/work/'
DUMP=os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/')
GOGDB='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-contributing/AddressDatabases/kotor2_gog_aspyr.db'
STEAMDB='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-contributing/AddressDatabases/kotor2_steam_aspyr.db'
KDB=BASE+'ghidra_knowledge/kotor2_ghidra_knowledge.db'

def hx(s): return int(s,16)
def inv():
    return list(csv.DictReader(open(BASE+'steam_db_pr3/inventory.tsv'),delimiter='\t'))
def steam_funcs():
    """addr -> dict(size,retn,name,ns,source,thunk)"""
    d={}
    for r in csv.DictReader(open(DUMP+'inputs/steam_functions.tsv'),delimiter='\t'):
        d[hx(r['addr'])]=dict(size=int(r['size']),retn=int(r['retn']),name=r['name'],ns=r['namespace'],source=r['source'],thunk=r['thunk']=='true')
    return d
def gog_funcs():
    db=sqlite3.connect(GOGDB)
    rows=db.execute('select class_name,function_name,address,notes,calling_convention,param_size_bytes from functions order by address').fetchall()
    return rows
def gog_offsets():
    db=sqlite3.connect(GOGDB)
    return db.execute('select class_name,member_name,offset,notes from offsets').fetchall()
def mapper():
    """steam_addr(int) -> (gog_addr, name, tag, retn)"""
    m={}
    for l in open(BASE+'steam_db_pr3/mapper_out.tsv'):
        f=l.rstrip('\n').split('\t')
        if len(f)<5: continue
        m[hx(f[0])]=(hx(f[1]),f[2],f[3],f[4])
    return m
def name_map():
    db=sqlite3.connect(KDB)
    d=collections.defaultdict(list)
    for a,n,s,c,e,dt in db.execute('select * from name_map'):
        d[hx(a)].append((n,s,c,e))
    return d
_hdr={}
def hdr(addr):
    """parse '// FN addr name size= ret= callers= callees=' -> dict or None"""
    p=f'{DUMP}decomp/{addr:08x}'[:-0]  # placeholder
    p=f"{DUMP}decomp/{addr:08x}"
    bb=f'{addr:08x}'[2:4]
    path=f"{DUMP}decomp/{bb}/{addr:08x}.c"
    if not os.path.exists(path): return None
    with open(path,errors='replace') as f: first=f.readline()
    m=re.match(r'// FN (\w+) (\S+) size=(\d+) ret=(\d+) callers=(\S*) callees=(\S*)',first)
    if not m: return None
    return dict(name=m.group(2),size=int(m.group(3)),ret=int(m.group(4)),
        callers=[hx(x) for x in m.group(5).split(',') if x],callees=[hx(x) for x in m.group(6).split(',') if x])
def body(addr):
    bb=f'{addr:08x}'[2:4]
    path=f"{DUMP}decomp/{bb}/{addr:08x}.c"
    return open(path,errors='replace').read() if os.path.exists(path) else None
def asm(addr):
    bb=f'{addr:08x}'[2:4]
    path=f"{DUMP}decomp/{bb}/{addr:08x}.asm"
    return open(path,errors='replace').read().splitlines() if os.path.exists(path) else None
