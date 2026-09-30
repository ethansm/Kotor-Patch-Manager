#!/usr/bin/env python3
"""Animation inventory for creature body models (Mod 5: Equipment 3D character, research R5).

Reads the animation list straight from the binary MDL headers and follows the supermodel chain, because the
engine looks a name up in the model's own animation list and then in its supermodel's (Gob::PlayAnimation
004e3270: `param_1[0x24]` first, then `param_1[0x21]`; see research/R4a).

Binary layout (K2, offsets relative to the data start = file offset 12; same base as inv3d_model_fit.py):
  geometry header 80 B (name char[32] @8), model header: animation array (ptr,count,count2) @88,
  supermodel name char[32] @136. Animation header: geometry header 80 B (name char[32] @8), length f32 @80,
  transition f32 @84.

Usage: list_creature_anims.py [--game DIR] [--out FILE] [--rows N,N,..]   (needs the repo .venv: pykotor)
Output: per supermodel chain the animation set with lengths; candidate flourish/idle presence per appearance row.
"""
import argparse, logging, struct, sys, collections
from pathlib import Path

# animations.2da names (plan section "What is already known"): looping idles and one-shot flourishes.
IDLES = ['pause1', 'pause2', 'pause3', 'cpause1', 'cpause2', 'standstill', 'neutral', 'good', 'evil']
FLOURISH = ['pausesh', 'scanning', 'salute', 'victory', 'bow', 'greeting', 'taunt', 'powerup', 'touchheart',
            'rolleyes', 'shrug', 'nodyes', 'point', 'itemequip', 'equip']


def _cstr(b):
    return b.split(b'\0', 1)[0].decode('latin1')


def parse_anims(mdl):
    """-> (model name, supermodel name, {anim name: length}) from raw MDL bytes."""
    d = mdl[12:]
    name = _cstr(d[8:40])
    sup = _cstr(d[136:168])
    ptr, cnt = struct.unpack_from('<II', d, 88)
    anims = {}
    if cnt < 4096:
        offs = struct.unpack_from('<%dI' % cnt, d, ptr)
        for o in offs:
            if o + 88 > len(d):
                continue
            anims[_cstr(d[o + 8:o + 40])] = struct.unpack_from('<f', d, o + 80)[0]
    return name, sup, anims


def chain(inst, mdlname, rt, cache):
    """Follow the supermodel chain. -> list of (model, {anims}); missing links stop the chain."""
    out, seen = [], set()
    cur = mdlname.lower()
    while cur and cur not in seen and cur != 'null':
        seen.add(cur)
        if cur not in cache:
            r = inst.resource(cur, rt)
            cache[cur] = parse_anims(r.data) if r is not None else None
        v = cache[cur]
        if v is None:
            out.append((cur, None))
            break
        out.append((cur, v[2]))
        cur = v[1].lower()
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--game', default=str(Path.home() / '.local/share/Steam/steamapps/common/Knights of the Old Republic II'))
    ap.add_argument('--out', default='-')
    ap.add_argument('--rows', default='', help='appearance rows to detail (default: K2 party rows, see K2_PARTY)')
    a = ap.parse_args()
    logging.disable(logging.CRITICAL)
    from pykotor.extract.installation import Installation
    from pykotor.resource.type import ResourceType
    from pykotor.resource.formats.twoda import read_2da
    inst = Installation(a.game)
    app = read_2da(inst.resource('appearance', ResourceType.TwoDA).data)
    anm = read_2da(inst.resource('animations', ResourceType.TwoDA).data)
    two = {anm.get_row(i)._data['name'].lower(): anm.get_row(i)._data for i in range(anm.get_height())}
    out = open(a.out, 'w') if a.out != '-' else sys.stdout
    P = lambda *x: print(*x, file=out)
    rt = ResourceType.MDL
    cache = {}
    # default: the KOTOR 2 party (appearance.2da also keeps K1 leftovers such as Bastila/Carth/Cand/Jolee/Juhani/Mission/
    # Zaalbar, which are not in the K2 party) + the PC test row 0
    K2_PARTY = [0, 2, 3, 451, 452, 453, 454, 455, 456, 457, 462, 511]
    want = [int(x) for x in a.rows.split(',') if x] if a.rows else K2_PARTY
    rows = []
    for i in range(app.get_height()):
        r = app.get_row(i)._data
        lab = r['label']
        if want is not None:
            if i not in want:
                continue
        elif not (lab.startswith('Party_') or lab.startswith('Player') or 'PMB' in r['race'].upper() or 'PFB' in r['race'].upper()):
            continue
        rows.append((i, lab, r))
    P('# Creature animation inventory (R5). candidates: idles=%s flourishes=%s' % (IDLES, FLOURISH))
    P('# animations.2da flags for flourishes: (fireforget, hideequippeditems, stationary)')
    for n in FLOURISH:
        d = two.get(n)
        P('#   %-10s %s' % (n, ('ff=%s hide=%s stat=%s loop=%s' % (d['fireforget'], d['hideequippeditems'], d['stationary'], d['looping'])) if d else 'NOT IN animations.2da'))
    bysup = collections.OrderedDict()
    P('\n## Per appearance row (model chosen = race column, else modela)')
    for i, lab, r in rows:
        m = r['race'] or r['modela']
        # for B rows the loaded body is armour dependent; sample race + modela + modelb
        cands = [x for x in dict.fromkeys([r['race'], r['modela'], r['modelb'], r['modelc']]) if x]
        for mn in cands[:3]:
            ch = chain(inst, mn, rt, cache)
            allan = {}
            for mdl, an in ch:
                if an:
                    for k, v in an.items():
                        allan.setdefault(k.lower(), v)
            key = ' > '.join(x for x, _ in ch)
            bysup.setdefault(key, set()).update(allan)
            idl = [x for x in IDLES if x in allan]
            fl = ['%s(%.1fs)' % (x, allan[x]) for x in FLOURISH if x in allan]
            P('row %3d %-28s type=%s model=%-14s chain=[%s] n_anims=%d' % (i, lab, r['modeltype'], mn, key, len(allan)))
            P('        idles: %s' % ', '.join(idl))
            P('        flourish: %s' % ', '.join(fl))
    P('\n## Per supermodel chain: union of candidate names and other stance-like names')
    for key, s in bysup.items():
        st = sorted(x for x in s if x[:1] in 'cgh' and len(x) <= 8 and x[1:2].isdigit() or x.startswith('cpause'))
        P('chain [%s]: %d anims; idles=%s; flourish=%s' % (key, len(s), [x for x in IDLES if x in s], [x for x in FLOURISH if x in s]))
        P('     stance-like: %s' % ', '.join(st[:40]))
    if out is not sys.stdout:
        out.close()


if __name__ == '__main__':
    main()
