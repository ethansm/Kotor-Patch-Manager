#!/usr/bin/env python3
"""Account for every J0-o diff byte: which KPM patch (by id dir) has a hook whose range covers it, and
whether the patch's replacement bytes equal J0-o there. Also checks proposed ports for overlaps with k2_aspyr_shader_fixes hooks
and emits proposed_hooks_steam.toml (hook blocks generated from J0-o bytes)."""
import glob,tomllib
from j0o_diff import *
root='/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches'
diffs=sorted(i for i in range(len(v)) if v[i]!=j[i])
def hooks(pid):
    h=tomllib.load(open(f'{root}/{pid}/kotor2-steam-aspyr.hooks.toml','rb'))
    return [(k['address'],bytes(k['original_bytes']),bytes(k['replacement_bytes'])) for k in h['hooks'] if k.get('type')=='simple']
sets={p:hooks(p) for p in ('K2AspyrShaderFixes','K2AspyrColorCorrection','K2AspyrFogReflectionFix','K2AspyrMusicFix')}
def cover(pid):
    c=set()
    for a,o,r in sets[pid]:
        f=va2f(a)
        for i in range(f,f+len(r)): c.add(i)
    return c
cov={p:cover(p) for p in sets}
print('total diff bytes',len(diffs))
hdr=[i for i in diffs if i<0x400]; print('PE header diffs',[hex(i) for i in hdr])
body=[i for i in diffs if i>=0x400]
for p in sets: print('%-26s covers %d of %d body diff bytes'%(p,sum(1 for i in body if i in cov[p]),len(body)))
union=set().union(*cov.values())
print('uncovered by any of the 4 patches:',[hex(i) for i in body if i not in union][:20])
# overlap check for proposed ports
sf=[(va2f(a),va2f(a)+len(r)) for a,o,r in sets['K2AspyrShaderFixes']]
out=['# Proposed additions for k2_aspyr_shader_fixes (Steam Aspyr 6A522E...), generated from J0-o 3C-FD bytes\n']
for pid,desc in (('K2AspyrColorCorrection','Pre-gamma colour grade in the full-screen gamma pass: R,B x0.90 and G x0.85 (removes the Aspyr yellow tint)'),('K2AspyrMusicFix','Disable music/ambient ducking during dialogue (NOP the fmul by the duck factor)')):
    for a,o,r in sets[pid]:
        f=va2f(a)
        assert v[f:f+len(o)]==o and j[f:f+len(r)]==r[:len(r)] or pid=='K2AspyrMusicFix'
        ov=[(hex(x),hex(y)) for x,y in sf if x<f+len(r) and f<y]
        print(pid,hex(a),'len',len(r),'overlap with ShaderFixes hooks:',ov or 'none','| J0-o==repl on diff bytes:',all(j[i]==r[i-f] for i in diffs if f<=i<f+len(r)))
        out.append(f'# {desc}\n[[hooks]]\naddress = {hex(a)}\ntype = "simple"\noriginal_bytes = [\n'+''.join('    '+', '.join('0x%02X'%b for b in o[k:k+16])+(',\n' if k+16<len(o) else '\n') for k in range(0,len(o),16))+']\nreplacement_bytes = [\n'+''.join('    '+', '.join('0x%02X'%b for b in r[k:k+16])+(',\n' if k+16<len(r) else '\n') for k in range(0,len(r),16))+']\n')
open('proposed_hooks_steam.toml','w').write('\n'.join(out))
