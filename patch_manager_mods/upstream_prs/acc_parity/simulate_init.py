#!/usr/bin/env python3
"""Simulate every GameAPI wrapper init (upstream/master sources) against address DBs.

Read-only: DBs are opened with file:...?mode=ro URIs; nothing is written except stdout.

Model of GameVersion (Patches/Common/GameAPI/GameVersion.cpp, upstream/master):
  GetFunctionAddress / GetOffset / GetClassSize  throw GameVersionException on a miss
  ResolveFunction                                 non-throwing, leaves the pointer null (optional)
  GetClassVtable, GetGlobalPointer                non-throwing
  HasOffset(A,B) gate + GetOffset(A,B)            optional (only looked up when present)
Inside `try { ... } catch { ... }` the first throwing miss abandons the rest of that try block;
statics assigned before the miss keep their values. If the catch body `return`s, the rest of the
init function is skipped too and the class' *Initialized flag stays false (re-tried on every
construction). A catch without `return` (CVirtualMachine) falls through and sets the flag anyway.

Usage: simulate_init.py <GameAPI dir> <label=db> [<label=db> ...]
"""
import re, os, sys, glob, sqlite3, collections

def strip_comments(s):
    s = re.sub(r'/\*.*?\*/', lambda m: ' ' * len(m.group(0)), s, flags=re.S)
    s = re.sub(r'//[^\n]*', lambda m: ' ' * len(m.group(0)), s)
    return s

def match_brace(src, i):
    """src[i] == '{' -> index just past the matching '}'"""
    depth = 0
    while i < len(src):
        c = src[i]
        if c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0: return i + 1
        i += 1
    return len(src)

LOOK = re.compile(
    r'GameVersion::(GetFunctionAddress|GetOffset)\s*\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)'
    r'|GameVersion::ResolveFunction\s*\(\s*\w+\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)'
    r'|GameVersion::(GetClassSize|GetClassVtable)\s*\(\s*"([^"]+)"\s*\)'
    r'|GameVersion::HasOffset\s*\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)')
TRY = re.compile(r'\btry\s*\{')
CATCH = re.compile(r'\bcatch\s*\([^)]*\)\s*\{')

def parse_init(body):
    """-> (items, base_calls). item = dict(kind, cls, name, throws, try_id, abort_return)"""
    tries = []  # (start,end,catch_returns)
    for m in TRY.finditer(body):
        end = match_brace(body, m.end() - 1)
        cm = CATCH.match(body, end) or re.compile(r'\s*catch\s*\([^)]*\)\s*\{').match(body, end)
        creturn = False
        if cm:
            cend = match_brace(body, cm.end() - 1)
            creturn = bool(re.search(r'\breturn\b', body[cm.end():cend]))
        tries.append((m.start(), end, creturn))
    gates = {(m.group(1), m.group(2)) for m in re.finditer(
        r'GameVersion::HasOffset\s*\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)', body)}
    items = []
    for m in LOOK.finditer(body):
        tid, cret = None, False
        for k, (s, e, cr) in enumerate(tries):
            if s <= m.start() < e:
                tid, cret = k, cr
        if m.group(1):
            kind = 'F' if m.group(1) == 'GetFunctionAddress' else 'O'
            cls, name = m.group(2), m.group(3)
            throws = tid is not None
            if kind == 'O' and (cls, name) in gates:
                throws = False
            items.append(dict(kind=kind, cls=cls, name=name, throws=throws, tid=tid, cret=cret, opt=(kind == 'O' and (cls, name) in gates)))
        elif m.group(5):
            items.append(dict(kind='R', cls=m.group(4), name=m.group(5), throws=False, tid=None, cret=False, opt=True))
        elif m.group(6):
            if m.group(6) == 'GetClassSize':
                items.append(dict(kind='S', cls=m.group(7), name='<classes.size>', throws=tid is not None, tid=tid, cret=cret, opt=False))
            else:
                items.append(dict(kind='V', cls=m.group(7), name='<classes.vtable>', throws=False, tid=None, cret=False, opt=True))
        elif m.group(8):
            pass  # gate itself, no lookup
    bases = re.findall(r'\b(\w+)::Initialize(Functions|Offsets)\s*\(\s*\)\s*;', body)
    flag_in_try = any(s <= m.start() < e for m in re.finditer(r'Initialized\s*=\s*true', body) for (s, e, _) in tries)
    any_catch_return = any(cr for (_, _, cr) in tries)
    return items, bases, dict(flag_in_try=flag_in_try, any_catch_return=any_catch_return)

def collect(gdir):
    out = collections.OrderedDict()
    for cpp in sorted(glob.glob(os.path.join(gdir, '*.cpp'))):
        src = strip_comments(open(cpp, encoding='utf-8', errors='replace').read())
        for m in re.finditer(r'void\s+(\w+)::(InitializeFunctions|InitializeOffsets)\s*\(\s*\)\s*\{', src):
            end = match_brace(src, m.end() - 1)
            items, bases, meta = parse_init(src[m.end():end])
            out[(m.group(1), m.group(2))] = (os.path.basename(cpp), items, bases, meta)
    return out

class DB:
    def __init__(self, path):
        self.c = sqlite3.connect('file:%s?mode=ro' % path, uri=True)
        self.F = {(a, b) for a, b in self.c.execute('select class_name,function_name from functions')}
        self.O = {(a, b) for a, b in self.c.execute('select class_name,member_name from offsets')}
        self.C = {a for (a,) in self.c.execute('select class_name from classes')}
        self.Cv = {a for (a,) in self.c.execute('select class_name from classes where vtable is not null')} \
            if any(r[1] == 'vtable' for r in self.c.execute('pragma table_info(classes)')) else set()
        self.G = {a for (a,) in self.c.execute('select pointer_name from global_pointers')}
    def has(self, it):
        k = it['kind']
        if k in 'FR': return (it['cls'], it['name']) in self.F
        if k == 'O': return (it['cls'], it['name']) in self.O
        if k == 'S': return it['cls'] in self.C
        if k == 'V': return it['cls'] in self.Cv
        raise ValueError(k)

def simulate(items, db, meta):
    """-> dict(resolved=[...], first_missing=item|None, missing=[...], flag_set=bool)"""
    resolved, missing = [], []
    aborted_tid = {}
    first = None
    stop_all = False
    for it in items:
        present = db.has(it)
        if not present: missing.append(it)
        if stop_all: continue
        if it['tid'] is not None and it['tid'] in aborted_tid:
            continue  # rest of this try block never runs
        if present:
            resolved.append(it)
        elif it['throws']:
            if first is None: first = it
            aborted_tid[it['tid']] = True
            if it['cret']: stop_all = True
    # *Initialized flag: set when nothing threw; after a throw it is still set only when the
    # assignment sits outside the try and the catch does not return (CVirtualMachine pattern).
    flag_set = first is None or (not meta['flag_in_try'] and not meta['any_catch_return'])
    return dict(resolved=resolved, first_missing=first, missing=missing, flag_set=flag_set)

def label(it):
    return '%s::%s' % (it['cls'], it['name'])

def main():
    gdir = sys.argv[1]
    dbs = [(a.split('=', 1)[0], DB(a.split('=', 1)[1])) for a in sys.argv[2:]]
    inits = collect(gdir)
    n_lookups = sum(len(v[1]) for v in inits.values())
    print('# GameAPI dir:', gdir)
    print('# init functions parsed: %d, lookups: %d' % (len(inits), n_lookups))
    kinds = collections.Counter(i['kind'] for v in inits.values() for i in v[1])
    print('# lookup kinds:', dict(kinds))
    for lab, db in dbs:
        print('\n' + '=' * 100)
        print('DB %s: %d functions, %d offsets, %d globals, %d classes rows' % (lab, len(db.F), len(db.O), len(db.G), len(db.C)))
        print('=' * 100)
        print('%-26s %-19s %5s %7s %-5s %6s %5s  %s' % ('class', 'init', 'looks', 'assign', 'full', 'miss', 'flag', 'first missing (throwing) | notes'))
        for (cls, fn), (cpp, items, bases, meta) in inits.items():
            if not items: continue
            r = simulate(items, db, meta)
            full = r['first_missing'] is None
            notes = []
            if r['first_missing']:
                notes.append(label(r['first_missing']))
            opt_miss = [label(i) for i in r['missing'] if not i['throws'] and i['kind'] != 'V']
            if opt_miss: notes.append('optional-miss: ' + ','.join(opt_miss[:4]) + ('...' if len(opt_miss) > 4 else ''))
            if bases: notes.append('calls base ' + ','.join('%s::Initialize%s' % b for b in bases))
            print('%-26s %-19s %5d %7d %-5s %6d %5s  %s' % (cls, fn, len(items), len(r['resolved']),
                  'yes' if full else 'NO', len(r['missing']), 'set' if r['flag_set'] else 'unset', ' | '.join(notes)))
    # ---- parity: Steam branch DB vs GOG, every init function --------------------------
    names = [l for l, _ in dbs]
    if 'branch' in names and 'gog' in names:
        bdb = dict(dbs)['branch']; gdb = dict(dbs)['gog']
        print('\n' + '=' * 100)
        print('PARITY: init functions where branch Steam DB assigns FEWER lookups than GOG, or differs in full-init')
        print('=' * 100)
        worse = diff = same = 0
        for (cls, fn), (cpp, items, bases, meta) in inits.items():
            if not items: continue
            rb = simulate(items, bdb, meta); rg = simulate(items, gdb, meta)
            ab, ag = len(rb['resolved']), len(rg['resolved'])
            fb, fg = rb['first_missing'] is None, rg['first_missing'] is None
            if ab < ag or fb != fg:
                worse += 1
                print('%-26s %-19s steam assigned %3d/%d full=%s first-miss=%s | gog assigned %3d full=%s first-miss=%s' % (
                    cls, fn, ab, len(items), fb, label(rb['first_missing']) if rb['first_missing'] else '-', ag, fg,
                    label(rg['first_missing']) if rg['first_missing'] else '-'))
            elif ab == ag: same += 1
            else: diff += 1
        print('init functions: %d total with lookups; equal-or-better than GOG: %d; steam assigns MORE than GOG: %d; worse: %d' % (
            worse + same + diff, same, diff, worse))
    # ---- ACC commands ------------------------------------------------------------------
    print('\n' + '=' * 100)
    print('ACC COMMANDS: requirement check per DB (requirement = lookup assigned before any abort)')
    print('=' * 100)
    def R(cls, fn, kind, lcls, name): return ('lookup', cls, fn, kind, lcls, name)
    def GP(name): return ('global', name)
    APP = [GP('APP_MANAGER_PTR')]
    SERVER = APP + [R('CAppManager', 'InitializeOffsets', 'O', 'CAppManager', 'Server'),
                    R('CServerExoApp', 'InitializeFunctions', 'F', 'CServerExoApp', 'GetPlayerCreatureId'),
                    R('CServerExoApp', 'InitializeFunctions', 'F', 'CServerExoApp', 'GetCreatureByGameObjectID')]
    CLIENT = APP + [R('CAppManager', 'InitializeOffsets', 'O', 'CAppManager', 'Client'),
                    R('CClientExoApp', 'InitializeFunctions', 'F', 'CClientExoApp', 'GetClientOptions'),
                    R('CClientOptions', 'InitializeFunctions', 'R', 'CClientOptions', 'SetCameraMode')]
    def CF(t): return [R('ConsoleFunc', 'InitializeFunctions', 'F', 'ConsoleFunc', t)]
    commands = collections.OrderedDict([
        ('runscript <resref> (string)', CF('StringConstructor') + SERVER + [GP('VIRTUAL_MACHINE_PTR'),
            R('CVirtualMachine', 'InitializeFunctions', 'F', 'CVirtualMachine', 'RunScript'),
            R('CExoString', 'InitializeFunctions', 'F', 'CExoString', 'CStrConstructor'),
            R('CExoString', 'InitializeOffsets', 'S', 'CExoString', '<classes.size>')]),
        ('teleport "x y" (string)', CF('StringConstructor') + SERVER + [
            R('CSWSObject', 'InitializeOffsets', 'O', 'CSWSObject', 'Position'),
            R('CSWSObject', 'InitializeOffsets', 'O', 'CSWSObject', 'Orientation'),
            R('CSWSObject', 'InitializeOffsets', 'O', 'CSWSObject', 'AreaId'),
            R('CSWSObject', 'InitializeFunctions', 'F', 'CSWSObject', 'AddActionToFront')]),
        ('freecam', CF('NoParamConstructor') + CLIENT),
        ('addfeat <feat> (int)', CF('IntConstructor') + SERVER + [
            R('CSWSCreature', 'InitializeOffsets', 'O', 'CSWSCreature', 'CreatureStats'),
            R('CSWSCreatureStats', 'InitializeFunctions', 'F', 'CSWSCreatureStats', 'AddFeat')]),
        ('walkmeshrender', CF('NoParamConstructor') + [GP('RENDER_AABB')]),
        ('guirender', CF('NoParamConstructor') + [GP('RENDER_GUI')]),
        ('wireframerender', CF('NoParamConstructor') + [GP('RENDER_WIREFRAME')]),
        ('triggersrender', CF('NoParamConstructor') + [GP('RENDER_QA_TRIGGERS'), GP('RENDER_TRIGGERS')]),
        ('personalspacerender', CF('NoParamConstructor') + [GP('RENDER_PERSONAL_SPACE')]),
        ('boundingboxesrender', CF('NoParamConstructor') + [GP('RENDER_GOB_BBS')]),
    ])
    for lab, db in dbs:
        print('\n-- DB %s' % lab)
        resolved_cache = {}
        def resolved(cls, fn):
            k = (cls, fn)
            if k not in resolved_cache:
                _, items, _, meta = inits[k]
                resolved_cache[k] = {(i['kind'], i['cls'], i['name']) for i in simulate(items, db, meta)['resolved']}
            return resolved_cache[k]
        for cmd, reqs in commands.items():
            miss = []
            for rq in reqs:
                if rq[0] == 'global':
                    if rq[1] not in db.G: miss.append('global ' + rq[1])
                else:
                    _, cls, fn, kind, lcls, name = rq
                    if (kind, lcls, name) not in resolved(cls, fn): miss.append('%s::%s [%s::%s]' % (lcls, name, cls, fn))
            print('  %-30s %s' % (cmd, 'OK' if not miss else 'FAIL: ' + '; '.join(miss)))
    # ---- GOG-missing names (upstream bug candidate) ------------------------------------
    if 'gog' in names:
        gdb = dict(dbs)['gog']
        print('\n' + '=' * 100)
        print('GOG DB: lookups issued by GameAPI inits that the GOG DB lacks (all wrappers), grouped by init function')
        print('  (first = first THROWING miss, i.e. where the try block abandons the rest)')
        print('=' * 100)
        tot = 0
        for (cls, fn), (cpp, items, bases, meta) in inits.items():
            miss = [i for i in items if not gdb.has(i)]
            if not miss: continue
            r = simulate(items, gdb, meta)
            tot += len(miss)
            first = label(r['first_missing']) if r['first_missing'] else '(none throwing)'
            print('%-24s %-18s missing %2d of %2d; first=%s' % (cls, fn, len(miss), len(items), first))
            print('      ' + ', '.join(label(i) + ('' if i['throws'] else '(opt)') for i in miss[:60]) + (' ...' if len(miss) > 60 else ''))
        print('total missing lookups (GOG): %d' % tot)
    # ---- markdown coverage table for the ACC-reachable wrappers --------------------------
    print('\n' + '=' * 100)
    print('MARKDOWN: ACC-reachable wrapper classes (per init function); cell = assigned/lookups, full?, first throwing miss, total missing')
    print('=' * 100)
    reach = ['ConsoleFunc', 'CExoString', 'CVirtualMachine', 'CAppManager', 'CServerExoApp', 'CClientExoApp', 'CClientOptions',
             'CGameObject', 'CSWSObject', 'CSWSCreature', 'CSWSCreatureStats']
    print('| Class / init | Lookups | ' + ' | '.join('%s (assigned, full, first miss, total missing)' % l for l in names) + ' |')
    print('| --- | --- | ' + ' | '.join('---' for _ in names) + ' |')
    for cls in reach:
        for fn in ('InitializeFunctions', 'InitializeOffsets'):
            if (cls, fn) not in inits or not inits[(cls, fn)][1]: continue
            _, items, _, meta = inits[(cls, fn)]
            cells = []
            for lab, db in dbs:
                r = simulate(items, db, meta)
                full = r['first_missing'] is None
                fm = ('first miss `%s`' % label(r['first_missing'])) if r['first_missing'] else '-'
                cells.append('%d/%d, %s, %s, %d missing' % (len(r['resolved']), len(items), 'full' if full else 'ABORTS', fm, len(r['missing'])))
            print('| %s %s | %d | %s |' % (cls, 'fns' if fn == 'InitializeFunctions' else 'offsets', len(items), ' | '.join(cells)))
    # per-lookup presence matrix for classes of interest (appendix)
    print('\n' + '=' * 100)
    print('APPENDIX: per-lookup presence (classes used by AdditionalConsoleCommands), source order')
    print('=' * 100)
    interest = ['ConsoleFunc', 'CExoString', 'CVirtualMachine', 'CAppManager', 'CServerExoApp', 'CClientExoApp',
                'CClientOptions', 'CGameObject', 'CSWSObject', 'CSWSCreature', 'CSWSCreatureStats']
    for cls in interest:
        for fn in ('InitializeFunctions', 'InitializeOffsets'):
            if (cls, fn) not in inits: continue
            cpp, items, bases, meta = inits[(cls, fn)]
            if not items: continue
            print('\n-- %s::%s (%s)  base calls: %s' % (cls, fn, cpp, bases))
            for n, it in enumerate(items, 1):
                flags = ''.join('Y' if d.has(it) else '.' for _, d in dbs)
                print('  %2d %s %-1s %-48s %s' % (n, it['kind'], 'T' if it['throws'] else 'o', label(it), flags))
    print('\n(columns after name = presence in DBs in the order: %s ; Y present, . missing; T throwing-in-try, o optional)' % ', '.join(l for l, _ in dbs))
    # global pointer lookups in non-init code
    print('\nGlobal pointers read outside init functions (GetGlobalPointer):')
    for cpp in sorted(glob.glob(os.path.join(gdir, '*.cpp'))):
        src = strip_comments(open(cpp, encoding='utf-8', errors='replace').read())
        for m in re.finditer(r'GetGlobalPointer\s*\(\s*"([^"]+)"', src):
            print('  %-24s %-20s %s' % (os.path.basename(cpp), m.group(1), ' '.join('%s=%s' % (l, 'Y' if m.group(1) in d.G else '.') for l, d in dbs)))

if __name__ == '__main__':
    main()
