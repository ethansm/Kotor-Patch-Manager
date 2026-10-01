#!/usr/bin/env python3
"""K1 (swkotor.exe 1.03) address -> K2 function by name -> K2 GOG addr -> K2 Steam addr.  Read-only.

Chain: KPM kotor1_0_3.db (K1 class::function, nearest preceding named function) ->
       KPM kotor2_gog_aspyr.db (same class::function, GOG address) ->
       Steam address from the repo's GOG->Steam maps (engine_research_findings/*_map_*.tsv,
       ghidra_knowledge/gog_map_mods678.tsv) or the knowledge DB name_map table.

A bridged address is a HYPOTHESIS: confirm `ret N` and callees in the dump before relying on it.
Confidence: name-exact (same class::function in K2 and a Steam address resolved), gog-only (name matches in the K2 GOG DB but
no Steam mapping yet: run gog_cluster_delta_map.py for the class), class-only (class exists in K2 but not that method), none.

usage: k1_to_k2_bridge.py [--inventory FILE.md] [--addr 0x41B140 ...] [--out TSV]
"""
import argparse, bisect, csv, glob, os, re, sqlite3, sys

HERE = os.path.dirname(os.path.abspath(__file__))
PM = os.path.abspath(os.path.join(HERE, '..', '..'))
KPM = os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/AddressDatabases')
KDB = os.path.join(PM, 'ghidra_knowledge', 'kotor2_ghidra_knowledge.db')


def ro(path):
    return sqlite3.connect('file:%s?mode=ro' % path, uri=True)


def base_name(fn):
    return re.sub(r'_\d+$', '', fn)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--inventory', help='KMRP binary-inventory.md; every 0x004xxxxx..0x00Bxxxxx first-column address is bridged')
    ap.add_argument('--addr', nargs='*', default=[])
    ap.add_argument('--out', default=os.path.join(PM, 'engine_bug_research', 'k1_k2_bridge.tsv'))
    a = ap.parse_args()

    addrs = {}
    if a.inventory:
        for line in open(a.inventory):
            m = re.match(r'\|\s*`(0x00[4-9A-B][0-9A-Fa-f]{5})`\s*\|', line)
            if m:
                addrs.setdefault(int(m.group(1), 16), 'kmrp-inventory')
    for s in a.addr:
        addrs.setdefault(int(s, 16), 'cli')

    k1 = ro(os.path.join(KPM, 'kotor1_0_3.db'))
    k1f = sorted((r[0], r[1], r[2], r[3]) for r in k1.execute(
        'select address,class_name,function_name,param_size_bytes from functions'))
    k1keys = [r[0] for r in k1f]

    g = ro(os.path.join(KPM, 'kotor2_gog_aspyr.db'))
    gog_by_name, gog_classes = {}, set()
    for addr, cl, fn, ps in g.execute('select address,class_name,function_name,param_size_bytes from functions'):
        gog_by_name.setdefault((cl, base_name(fn)), []).append((addr, fn, ps))
        gog_classes.add(cl)

    # gog addr -> (steam addr, method, retn)
    gog2steam = {}
    files = glob.glob(os.path.join(PM, 'engine_research_findings', '*_map_*.tsv')) + [
        os.path.join(PM, 'ghidra_knowledge', 'gog_map_mods678.tsv')]
    for f in files:
        for row in csv.reader(open(f), delimiter='\t'):
            if len(row) >= 4 and row[0].startswith('0x') and row[1].startswith('0x'):
                gog2steam.setdefault(int(row[1], 16), []).append((int(row[0], 16), row[3], row[4] if len(row) > 4 else '', os.path.basename(f)))
    nm = ro(KDB)
    steam_by_name = {}
    for sa, name, conf in nm.execute('select steam_addr,name,confidence from name_map'):
        steam_by_name.setdefault(name, []).append((int(sa, 16), conf))
    steam_ret = {}
    for row in csv.reader(open(os.path.join(PM, 'ghidra_knowledge', 'steam_functions.tsv')), delimiter='\t'):
        if row[0].startswith('0x'):
            steam_ret[int(row[0], 16)] = row[2]

    cols = ['k1_addr', 'source', 'k1_function', 'k1_offset', 'k1_retn', 'confidence', 'gog_addr', 'gog_retn',
            'steam_addr', 'steam_retn', 'steam_via', 'retn_agree', 'notes']
    out = []
    for addr in sorted(addrs):
        i = bisect.bisect_right(k1keys, addr) - 1
        if i < 0:
            out.append([hex(addr), addrs[addr], '', '', '', 'none', '', '', '', '', '', '', 'before first K1 function']); continue
        faddr, cl, fn, ps = k1f[i]
        nxt = k1f[i + 1][0] if i + 1 < len(k1f) else None
        notes = []
        if nxt is not None and nxt - faddr > 0x1800:
            notes.append('gap to next named K1 fn %#x: container uncertain' % (nxt - faddr))
        cands = gog_by_name.get((cl, base_name(fn)), [])
        # exact-name candidates first (fn vs fn_2 variants)
        exact = [c for c in cands if c[1] == fn] or cands
        # K1 and K2 may number duplicate-named functions differently (Draw vs Draw_2): prefer the candidate whose
        # param size equals K1's; if none agree keep the name-exact one and flag it.
        same_ps = [c for c in cands if c[2] == ps]
        if same_ps and not [c for c in exact if c[2] == ps]:
            notes.append('K1 name %s matched GOG %s by param size' % (fn, same_ps[0][1]))
            exact = same_ps
        elif same_ps:
            exact = [c for c in exact if c[2] == ps]
        elif exact and ps is not None and exact[0][2] != ps:
            notes.append('K1 retn %s != GOG retn %s' % (ps, exact[0][2]))
        if not exact:
            conf = 'class-only' if cl in gog_classes else 'none'
            out.append([hex(addr), addrs[addr], '%s::%s' % (cl, fn), hex(addr - faddr), ps, conf, '', '', '', '', '', '', '; '.join(notes)]); continue
        if len(exact) > 1:
            notes.append('%d GOG candidates' % len(exact))
        gaddr, gfn, gps = exact[0]
        steam = gog2steam.get(gaddr, [])
        via = ''
        sa = ''
        if steam:
            sa, via = steam[0][0], '%s:%s:%s' % (steam[0][3], steam[0][1], steam[0][2])
        else:
            for nme in ('%s::%s' % (cl, fn), '%s::%s' % (cl, base_name(fn))):
                if nme in steam_by_name:
                    sa, via = steam_by_name[nme][0][0], 'name_map:%s' % steam_by_name[nme][0][1]; break
        sr = steam_ret.get(sa, '') if sa != '' else ''
        agree = ''
        if sr != '' and gps is not None:
            agree = 'yes' if str(gps) == sr else 'NO(gog %s vs steam %s)' % (gps, sr)
        conf = 'name-exact' if sa != '' else 'gog-only'
        if any(n.startswith('K1 retn') for n in notes):
            conf = 'name-weak' if sa != '' else 'gog-only'
        out.append([hex(addr), addrs[addr], '%s::%s' % (cl, fn), hex(addr - faddr), ps, conf, hex(gaddr), gps,
                    hex(sa) if sa != '' else '', sr, via, agree, '; '.join(notes)])
    with open(a.out, 'w', newline='') as fh:
        w = csv.writer(fh, delimiter='\t'); w.writerow(cols); w.writerows(out)
    from collections import Counter
    c = Counter(r[5] for r in out)
    print('wrote %s: %d rows; %s' % (a.out, len(out), dict(c)))


if __name__ == '__main__':
    main()
