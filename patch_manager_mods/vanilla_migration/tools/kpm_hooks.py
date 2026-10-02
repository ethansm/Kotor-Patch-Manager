#!/usr/bin/env python3
"""Decode KPM hooks (steam-aspyr) and report: VA, section, whether original_bytes match vanilla, text decode, vs J0-o bytes at same VA."""
from j0o_diff import *
import sys
path=sys.argv[1] if len(sys.argv)>1 else KPM
h=tomllib.load(open(path,'rb'))
def dec(b): return bytes(b).decode('latin1')
for k in h['hooks']:
    a=k['address']; ob=bytes(k.get('original_bytes',[])); rb=bytes(k.get('replacement_bytes',[]))
    f=va2f(a)
    print('='*90); print('hook',hex(a),secname(a),k.get('type'),'orig',len(ob),'repl',len(rb),{x:y for x,y in k.items() if x not in('original_bytes','replacement_bytes','address','type')})
    print(' vanilla match:',v[f:f+len(ob)]==ob,' | J0-o bytes at VA equal vanilla:',j[f:f+len(ob)]==v[f:f+len(ob)], '| J0-o equals KPM repl:', j[f:f+len(rb)]==rb)
    if secname(a) in('.rdata','.data'):
        print('--- original'); print(dec(ob)); print('--- replacement'); print(dec(rb))
    else:
        print(' orig hex',ob.hex(' ')); print(' repl hex',rb.hex(' '))
