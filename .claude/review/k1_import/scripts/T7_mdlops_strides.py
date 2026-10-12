import sys,os
sys.path.insert(0,os.path.expanduser('~/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/k1_port'))
import k1_port_model as m
pk=m.load_pykotor()
P={'mdlops':'~/Games/KOTOR2_Mod_Workspace/mdlops_t7/mdl_run1/pass2/n_commm-ascii-k2-bin','ours':'~/Games/KOTOR2_Mod_Workspace/k1_port_stage/models/k1p_commm','k1':'~/Games/KOTOR2_Mod_Workspace/mdlops_t7/inputs/n_commm'}
r={}
for k,p in P.items():
    p=os.path.expanduser(p); r[k]=m.binary_mdx_strides(pk,open(p+'.mdl','rb').read(),open(p+'.mdx','rb').read())
for k in r: print(k,sorted(set(r[k])),len(r[k]))
print('mdlops==ours',r['mdlops']==r['ours'],' ours==k1',r['ours']==r['k1'])
print([ (i,x,y,z) for i,(x,y,z) in enumerate(zip(r['mdlops'],r['ours'],r['k1'])) if not x==y][:6])
