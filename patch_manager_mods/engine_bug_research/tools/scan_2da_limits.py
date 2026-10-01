#!/usr/bin/env python3
"""Read-only scan of the live KOTOR II install's 2DA tables against engine limits (doc 15 leads L9/L10, candidate TD).

Run with the repo venv (needs pykotor only to enumerate/read resources):
    .venv/bin/python patch_manager_mods/engine_bug_research/tools/scan_2da_limits.py [GAME_DIR] > output.txt

Effective table = Override/<name>.2da if present, else the 2DA from the BIFs (what the engine's resman serves).
The checks follow engine_research_findings/V6.md section F4 (parser 0x71C3D0, tokenizer 0x71C180, magic check 0x729FD0):
  * text `2DA V2.0`: tokens (header, row labels, cells) with length >= 256 overflow the 256-byte stack buffer; odd `"` count.
  * binary `2DA V2.b`: only COLUMN NAMES are copied to the stack (>= 256 overflows); u16 offsets (header / label region);
    space, quote or empty column names desync the column count.
Caps: K2-verified widths from V6 (table CAPS_K2); the community table docs/2da_row_limits.md describes K1 and is shown only
as context. Nothing is written; no game file is modified.
"""
import logging
import os
import re
import struct
import sys

logging.disable(logging.CRITICAL)
from pykotor.extract.installation import Installation  # noqa: E402
from pykotor.resource.type import ResourceType  # noqa: E402

GAME = os.path.expanduser('~/.steam/steam/steamapps/common/Knights of the Old Republic II')
COMMUNITY = os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/docs/2da_row_limits.md')

# K2 Steam caps verified in V6 (max usable rows before the engine misbehaves). None = no hard cap found.
CAPS_K2 = {
    'appearance': (65536, 'u16 appearance id (creature+0x194)'),
    'portraits': (65534, 'u16 id, ids >= 0xFFFE are sentinels (0x57F360)'),
    'soundset': (65535, 'row count truncated to u16: 65536 rows load as 0 (0x85F0DF)'),
    'classes': (255, 'byte count at rules+0x121: 256 rows load as 0 (0x69FF93)'),
    'heads': (254, 'byte-path head id, 0xFF sentinel, head 0 remapped to 1 (0x85D6AB)'),
}


def community_caps():
    caps = {}
    try:
        for line in open(COMMUNITY):
            c = [x.strip() for x in line.split('|')]
            if len(c) > 4 and c[1].endswith('.2da') and c[3].isdigit():
                caps[c[1][:-4].lower()] = int(c[3])
    except OSError:
        pass
    return caps


def tokens_text(data):
    """Emulate the text tokenizer closely enough for lengths: whitespace separated, "quoted" tokens (length between quotes).
    Returns (list of lines, each a list of token lengths, odd_quote_flag)."""
    s = data.decode('latin-1')
    odd = s.count('"') % 2 == 1
    lines = s.split('\n')
    out = []
    for ln in lines:
        toks = []
        i, n = 0, len(ln)
        while i < n:
            if ln[i] in ' \t\r':
                i += 1
            elif ln[i] == '"':
                j = ln.find('"', i + 1)
                if j < 0:
                    j = n
                toks.append(j - i - 1)
                i = j + 1
            else:
                j = i
                while j < n and ln[j] not in ' \t\r':
                    j += 1
                toks.append(j - i)
                i = j
        out.append(toks)
    return lines, out, odd


def scan_text(data):
    res = {'fmt': 'text V2.0', 'issues': []}
    lines, toks, odd = tokens_text(data)
    if odd:
        res['issues'].append('odd number of double quotes (a quoted token can swallow following lines)')
    body = [t for t in toks[1:] if t]
    if not body:
        res['issues'].append('no header/rows (load fails)')
        return res
    ncols = len(body[0])
    res['cols'] = ncols
    res['rows'] = max(0, len(body) - 1)
    res['longest_col'] = max(body[0], default=0)
    res['longest_label'] = max((t[0] for t in body[1:]), default=0)
    res['longest_cell'] = max((x for t in body[1:] for x in t[1:1 + ncols]), default=0)
    res['longest_token'] = max(res['longest_col'], res['longest_label'], res['longest_cell'])
    res['danger_token'] = res['longest_token'] >= 256
    return res


def scan_binary(data):
    res = {'fmt': 'binary V2.b', 'issues': []}
    p = 9  # "2DA V2.b\n"
    hdr_end = data.find(b'\x00', p)
    if hdr_end < 0:
        res['issues'].append('no NUL after column names')
        return res
    names = data[p:hdr_end].split(b'\t')
    if names and names[-1] == b'':
        names = names[:-1]
    res['cols'] = len(names)
    res['longest_col'] = max((len(x) for x in names), default=0)
    hdr = hdr_end - p
    if hdr > 0xFFFF:
        res['issues'].append('column-name header %d B exceeds u16 offsets' % hdr)
    for x in names:
        if not x or b' ' in x or b'"' in x:
            res['issues'].append('column name %r has space/quote/is empty (column-count desync)' % x[:30])
            break
    q = hdr_end + 1
    if q + 4 > len(data):
        res['issues'].append('truncated before row count')
        return res
    rows = struct.unpack_from('<I', data, q)[0]
    res['rows'] = rows
    q += 4
    labels_start = q
    ends = 0
    for _ in range(rows):
        e = data.find(b'\t', q)
        if e < 0:
            res['issues'].append('row label without TAB terminator')
            return res
        ends = max(ends, e - labels_start)
        q = e + 1
    res['longest_label'] = 0  # labels are never copied to the stack on the binary path
    region = hdr + 1 + 4 + (q - labels_start)
    if region > 0xFFFF:
        res['issues'].append('label region end %d B exceeds u16 offsets' % region)
    off_bytes = 2 * rows * len(names)
    q += off_bytes
    if q + 2 > len(data):
        res['issues'].append('truncated in offset table')
        return res
    dsize = struct.unpack_from('<H', data, q)[0]
    q += 2
    if q + dsize != len(data):
        res.setdefault('notes', []).append('size word %d != actual data %d (benign: the engine reads the u16 into a stack local and never uses it, 0x71C8D2)' % (dsize, len(data) - q))
    cell_len = 0
    blob = data[q:]
    for off in struct.unpack_from('<%dH' % (rows * len(names)), data, q - 2 - off_bytes) if rows * len(names) else ():
        e = blob.find(b'\x00', off)
        if e >= 0:
            cell_len = max(cell_len, e - off)
    res['longest_cell'] = cell_len
    res['longest_token'] = res['longest_col']
    res['danger_token'] = res['longest_col'] >= 256
    res['header_bytes'] = hdr
    res['label_region_end'] = region
    return res


def main():
    game = sys.argv[1] if len(sys.argv) > 1 else GAME
    inst = Installation(game)
    eff = {}
    for r in inst.chitin_resources():
        if r.restype() == ResourceType.TwoDA:
            eff[r.resname().lower()] = ('bif', r)
    for r in inst.override_resources():
        if r.restype() == ResourceType.TwoDA:
            eff[r.resname().lower()] = ('override', r)
    comm = community_caps()
    print('# scan_2da_limits: %s' % game)
    print('# tables: %d (override %d)' % (len(eff), sum(1 for v in eff.values() if v[0] == 'override')))
    print('# danger_token = token >= 256 chars (stack overflow, V6 F2); WARN = token >= 200 or rows >= 90% of K2 cap; NOTE = informational')
    cols = ['table', 'src', 'fmt', 'rows', 'cols', 'k2_cap', 'headroom', 'k1_community_cap', 'longest_col', 'longest_label',
            'longest_cell', 'flags']
    print('\t'.join(cols))
    nflag = 0
    for name in sorted(eff):
        src, r = eff[name]
        data = r.data()
        magic = data[:8]
        if magic == b'2DA V2.b':
            res = scan_binary(data)
        elif magic == b'2DA V2.0':
            res = scan_text(data)
        else:
            res = {'fmt': 'BAD MAGIC %r' % magic, 'issues': ['engine rejects this table']}
        flags = list(res.get('issues', []))
        notes = res.get('notes', [])
        rows = res.get('rows')
        cap = CAPS_K2.get(name)
        headroom = ''
        if cap and rows is not None:
            headroom = cap[0] - rows
            if rows >= 0.9 * cap[0]:
                flags.append('WARN rows >= 90%% of K2 cap (%s)' % cap[1])
        if res.get('danger_token'):
            flags.append('DANGER token >= 256')
        elif res.get('longest_token', 0) >= 200:
            flags.append('WARN token >= 200')
        if flags:
            nflag += 1
        print('\t'.join(str(x) for x in [
            name, src, res['fmt'], rows if rows is not None else '', res.get('cols', ''),
            cap[0] if cap else '', headroom, comm.get(name, ''), res.get('longest_col', ''), res.get('longest_label', ''),
            res.get('longest_cell', ''), '; '.join(flags + ['NOTE ' + n for n in notes])]))
    print('# tables with flags: %d' % nflag)
    print('# caps used (K2 verified, V6): ' + '; '.join('%s=%d (%s)' % (k, v[0], v[1]) for k, v in sorted(CAPS_K2.items())))


if __name__ == '__main__':
    main()
