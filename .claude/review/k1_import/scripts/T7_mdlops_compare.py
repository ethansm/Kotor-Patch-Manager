import sys,os,json
sys.path.insert(0,os.path.expanduser('~/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/k1_port'))
import k1_port_model as m
from pykotor.resource.formats.mdl import read_mdl
R=os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/mdlops_t7/mdl_run1/pass2/n_commm-ascii-k2-bin')
S=os.path.expanduser('~/Games/KOTOR2_Mod_Workspace/k1_port_stage/models/k1p_commm')
def load(p):
    mb=open(p+'.mdl','rb').read(); xb=open(p+'.mdx','rb').read()
    mdl=read_mdl(mb,0,None,source_ext=xb) if False else read_mdl(mb,source_ext=xb)
    return mb,xb,mdl
out={}
for k,p in (('mdlops',R),('ours',S)):
    mb,xb,mdl=load(p)
    st=m.binary_mdx_strides(None,mb,xb) if False else None
    try: st=m.binary_mdx_strides(__import__('types').SimpleNamespace(),mb,xb)
    except Exception as e: st=repr(e)
    out[k]=(m.summarize(mdl,st if isinstance(st,list) else None),st)
a,b=out['mdlops'][0],out['ours'][0]
print('strides mdlops',out['mdlops'][1]); print('strides ours  ',out['ours'][1])
def diff(x,y,path=''):
    if isinstance(x,dict) and isinstance(y,dict):
        for k in sorted(set(x)|set(y)):
            if k not in x or k not in y: print('KEY',path+'/'+k,'only in', 'ours' if k in y else 'mdlops')
            else: diff(x[k],y[k],path+'/'+k)
    elif isinstance(x,list) and isinstance(y,list):
        if len(x)!=len(y): print('LEN',path,len(x),len(y))
        for i,(p,q) in enumerate(zip(x,y)): diff(p,q,f'{path}[{i}]')
    elif x!=y:
        s=lambda v:(repr(v)[:70])
        print('DIFF',path,'mdlops=',s(x),'ours=',s(y))
diff(a,b)

print('=== differing key names (aggregate)')
import collections,re
import io,contextlib
buf=io.StringIO()
with contextlib.redirect_stdout(buf): diff(a,b)
c=collections.Counter(re.sub(r'\[\d+\]','[]',l.split()[1]) if l.startswith('DIFF') else l[:60] for l in buf.getvalue().splitlines())
for k,v in c.most_common(40): print(v,k)
k1=m.summarize(read_mdl(open(os.path.expanduser(B:='~/Games/KOTOR2_Mod_Workspace/mdlops_t7/inputs/n_commm.mdl'),'rb').read(),source_ext=open(os.path.expanduser(B[:-4]+'.mdx'),'rb').read()))
print('vertex counts k1  ',[x['vertex_count'] for x in k1['meshes']])
print('vertex counts ours',[x['vertex_count'] for x in b['meshes']])
print('vertex counts mdlo',[x['vertex_count'] for x in a['meshes']])
print('nodes equal', a.get('nodes')==b.get('nodes'), 'textures', [x.get('texture') for x in a['meshes']][:4],[x.get('texture') for x in b['meshes']][:4])

print('=== detail')
n=0
for l in buf.getvalue().splitlines():
    if any(t in l for t in ('qbones','tbones','bonemap','texture_1','model_name','/nodes[')) and n<14:
        print(l[:200]); n+=1
import math
mx=0;cnt=0;sign=0
for i,(x,y) in enumerate(zip(a['meshes'],b['meshes'])):
    sx,sy=x.get('skin'),y.get('skin')
    if not sx: continue
    for key in ('qbones','tbones'):
        for p,q in zip(sx[key],sy[key]):
            for u,v in zip(p,q):
                d=abs(u-v) if isinstance(u,(int,float)) else 0
                mx=max(mx,d) if key=='tbones' else mx
    print(i,'bonemap equal',sx['bonemap']==sy['bonemap'],len(sx['bonemap']),len(sy['bonemap'])) if sx['bonemap']!=sy['bonemap'] else None
print('max tbones abs diff',mx)

print('=== bonemap/qbones')
for i in (42,43):
    x=a['meshes'][i]['skin']['bonemap']; y=b['meshes'][i]['skin']['bonemap']
    print(i,[(j,p,q) for j,(p,q) in enumerate(zip(x,y)) if p!=q][:8])
mq=0;sg=0
for x,y in zip(a['meshes'],b['meshes']):
    if not x.get('skin'): continue
    for p,q in zip(x['skin']['qbones'],y['skin']['qbones']):
        d=min(max(abs(u-v) for u,v in zip(p,q)), max(abs(u+v) for u,v in zip(p,q)))
        mq=max(mq,d)
print('max qbones diff (sign-insensitive)',mq)

print('=== k1 vs ours bonemap')
for i in (42,43):
    print(i,'k1==ours',k1['meshes'][i]['skin']['bonemap']==b['meshes'][i]['skin']['bonemap'])
names=[n['name'] for n in k1['nodes']]
import collections
d=[k for k,v in collections.Counter(n.lower() for n in names).items() if v>1]
print('dup node names in K1 tree:',d[:6])
print('all skin K1==ours', all(x['skin']==y['skin'] for x,y in zip(k1['meshes'],b['meshes']) if x.get('skin')))
