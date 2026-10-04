import re
from xref import *
def cstr(read, va):
    try: b = read(va, 40)
    except SystemExit: return None
    s = b.split(b"\0")[0]
    return s.decode() if 2 <= len(s) < 32 and re.fullmatch(rb"[ -~]+", s) else None
def names(buf, base, read, ctor, load, getf, dtor):
    res = {}
    for a in callers(buf, base, ctor):
        # find push imm32 in previous 40 bytes whose imm points at a string
        seg = buf[a-base-40:a-base]
        found = None
        for i in range(len(seg)-4):
            if seg[i] == 0x68:
                v = struct.unpack_from("<I", seg, i+1)[0]
                s = cstr(read, v)
                if s: found = s
        res.setdefault(found, []).append(a)
    return res
g = names(GT, GBASE, gog_read, 0x6207E0, 0, 0, 0)
s = names(TEXT, TBASE, steam_read, 0x71AD40, 0, 0, 0)
common = sorted(k for k in g if k and k in s and len(g[k]) == 1 and len(s[k]) == 1)
print("unique-name callers matched:", len(common))
for k in common[:12]:
    print(k, hex(g[k][0]), hex(s[k][0]))
# show first: 2DA names containing 'classes'/'appearance'
