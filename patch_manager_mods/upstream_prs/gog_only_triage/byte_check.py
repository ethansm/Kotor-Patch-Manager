#!/usr/bin/env python3
"""R3 triage: do the GOG hooks' original_bytes occur on the Steam exe (.text)?

Read-only. For each hook in the three GOG-only patches:
  exact   : number of exact occurrences of original_bytes in Steam .text (+ up to 4 VAs)
  same_va : do the bytes match at the identical VA
  masked  : same search with address/rel32 operands wildcarded (operands found by objdump
            decoding of the byte string; only meaningful when len >= 5 and decodes cleanly)
  ctx     : among the (exact|masked) hits, the unique one whose delta (steam-gog) lies within
            +-0x400 of the delta of the nearest uniquely-matched neighbouring hook (by GOG addr)
Usage: byte_check.py UPSTREAM_PATCHES_DIR STEAM_EXE OUT_TSV
"""
import re, struct, subprocess, sys, tempfile, os, tomllib, bisect

PATCHES, EXE, OUT = sys.argv[1:4]
data = open(EXE, 'rb').read()
pe = struct.unpack_from('<I', data, 0x3C)[0]
nsec = struct.unpack_from('<H', data, pe + 6)[0]
optsz = struct.unpack_from('<H', data, pe + 20)[0]
base = struct.unpack_from('<I', data, pe + 24 + 28)[0]
secs = []
for i in range(nsec):
    o = pe + 24 + optsz + 40 * i
    name = data[o:o+8].rstrip(b'\0').decode()
    vsz, va, rsz, raw = struct.unpack_from('<IIII', data, o + 8)
    secs.append((name, base + va, vsz, raw, rsz))
text = next(s for s in secs if s[0] == '.text')
tva, tvsz, traw, trsz = text[1], text[2], text[3], text[4]
tb = data[traw:traw + min(trsz, tvsz)]
def va2off(va):
    for n, sva, vsz, raw, rsz in secs:
        if sva <= va < sva + max(vsz, rsz): return raw + va - sva
    return None
def find_all(pat):
    out, i = [], tb.find(pat)
    while i != -1:
        out.append(tva + i); i = tb.find(pat, i + 1)
    return out

def mask_of(orig, addr):
    """wildcard map via objdump: True = wildcard byte"""
    with tempfile.NamedTemporaryFile(delete=False) as f: f.write(bytes(orig)); p = f.name
    try:
        r = subprocess.run(['objdump', '-D', '-b', 'binary', '-mi386', '-Mintel', f'--adjust-vma={addr:#x}', p],
                           capture_output=True, text=True).stdout
    finally: os.unlink(p)
    mask = [False] * len(orig); ok = True
    for ln in r.splitlines():
        m = re.match(r'\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(.*)', ln)
        if not m: continue
        a = int(m.group(1), 16) - addr; bs = m.group(2).split(); txt = m.group(3)
        if '(bad)' in txt: ok = False
        mn = txt.split()[0] if txt.split() else ''
        if mn in ('call', 'jmp') and len(bs) == 5 or (mn.startswith('j') and len(bs) == 6):
            for k in range(len(bs) - 4, len(bs)): mask[a + k] = True
        for h in re.findall(r'0x([0-9a-f]{6,8})\b', txt):
            v = int(h, 16)
            if 0x400000 <= v < 0xB00000:
                le = struct.pack('<I', v).hex(); s = ''.join(bs)
                j = s.find(le)
                if j >= 0 and j % 2 == 0:
                    for k in range(4): mask[a + j // 2 + k] = True
    return mask, ok

def masked_find(orig, mask):
    pat = bytes(b for b, m in zip(orig, mask) if not m)
    # build regex
    rx = b''.join(b'.' if m else re.escape(bytes([b])) for b, m in zip(orig, mask))
    return [tva + m.start() for m in re.finditer(b'(?s)' + rx, tb) if True] if False else \
           [tva + m.start() for m in re.finditer(b'(?=(' + b'(?s:' + rx + b')))', tb)]

rows = []
def load(patch, fn):
    d = tomllib.load(open(os.path.join(PATCHES, patch, fn), 'rb'))
    for i, h in enumerate(d['hooks']):
        rows.append(dict(patch=patch, file=fn, idx=i, addr=h['address'], type=h['type'], func=h.get('function', ''),
                         orig=bytes(h['original_bytes']), repl=h.get('replacement_bytes')))
load('PartyRosterLimit32K2', 'kotor2-gog-aspyr.hooks.toml')
load('K2ClassMenuClothes', 'hooks.toml')
load('Movie Patch', 'kotor2_hooks.toml')   # K2 hooks only; hooks.toml of Movie Patch targets K1 shas

for r in rows:
    o = r['orig']
    r['exact'] = find_all(o)
    off = va2off(r['addr'])
    r['same_va'] = (off is not None and data[off:off+len(o)] == o)
    r['masked'] = None; r['mask_desc'] = ''
    if len(o) >= 5:
        mask, ok = mask_of(o, r['addr'])
        if ok and any(mask):
            r['masked'] = masked_find(o, mask)
            r['mask_desc'] = ''.join('xx' if m else f'{b:02x}' for b, m in zip(o, mask))
        elif ok:
            r['masked'] = r['exact']
    r['mask_hits'] = r['masked'] if r['masked'] is not None else r['exact']

# neighbour-delta context
uniq = sorted((r['addr'], (r['exact'][0] if len(r['exact']) == 1 else r['mask_hits'][0]) - r['addr'])
              for r in rows if r['patch'] == 'PartyRosterLimit32K2' and len(r['exact']) == 1 or
              (r['patch'] == 'PartyRosterLimit32K2' and r['masked'] is not None and len(r['mask_hits']) == 1 and len(r['orig']) >= 8))
def ctx(r):
    hits = r['mask_hits'] if r['masked'] is not None else r['exact']
    if not hits: return []
    cand = [u for u in uniq if u[0] != r['addr']]
    if not cand: return []
    near = min(cand, key=lambda u: abs(u[0] - r['addr']))
    return [h for h in hits if abs((h - r['addr']) - near[1]) <= 0x400]
def classify(r):
    ne, nm = len(r['exact']), len(r['mask_hits'])
    if ne == 1: return 'exact-unique' + ('@same-va' if r['same_va'] else '@moved')
    if ne > 1: return 'exact-multiple'
    if r['masked'] is not None and nm == 1: return 'masked-unique'
    if r['masked'] is not None and nm > 1: return 'masked-multiple'
    return 'absent'
hdr = ['patch', 'idx', 'gog_addr', 'type', 'function', 'len', 'class', 'exact_n', 'same_va', 'masked_n', 'delta_if_unique', 'ctx_hit', 'exact_vas', 'mask_pattern', 'original_bytes']
with open(OUT, 'w') as f:
    f.write('\t'.join(hdr) + '\n')
    for r in rows:
        c = classify(r); ex = r['exact']; mk = r['mask_hits']
        uh = ex[0] if len(ex) == 1 else (mk[0] if r['masked'] is not None and len(mk) == 1 else None)
        cx = ctx(r)
        f.write('\t'.join(map(str, [r['patch'], r['idx'], f"{r['addr']:#010x}", r['type'], r['func'], len(r['orig']), c, len(ex),
                int(r['same_va']), '' if r['masked'] is None else len(mk), '' if uh is None else f'{uh - r["addr"]:+#x}',
                ','.join(f'{h:#x}' for h in cx[:3]) if len(cx) == 1 else (f'{len(cx)} cands' if cx else ''),
                ','.join(f'{h:#x}' for h in ex[:4]), r['mask_desc'], r['orig'].hex(' ')])) + '\n')
import collections
for p in dict.fromkeys(r['patch'] for r in rows):
    print(p, collections.Counter(classify(r) for r in rows if r['patch'] == p))
