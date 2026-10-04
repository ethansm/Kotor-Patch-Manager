import os,glob,logging
logging.disable(logging.CRITICAL)
from pykotor.resource.generics.uti import read_uti
from pykotor.resource.formats.twoda import read_2da
from pykotor.resource.formats.tlk import read_tlk
S=os.environ['S']+'/x'
sp=read_2da(open(S+'/2da/spells.2da','rb').read())
bi=read_2da(open(S+'/2da/baseitems.2da','rb').read())
tlk=read_tlk(open('/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/dialog.tlk','rb').read())
# spells rows with healing-ish scripts / any item use
hs={i for i in range(sp.get_height()) if sp.get_row(i).get_string('impactscript') in('k_sup_healing',)}
print('healing spell rows',sorted(hs))
# all item-cast spells (usertype 4) for reference
print('usertype4 rows',[(i,sp.get_row(i).get_string('label'),sp.get_row(i).get_string('impactscript')) for i in range(sp.get_height()) if sp.get_row(i).get_string('usertype')=='4'])
rows=[]
for f in sorted(glob.glob(S+'/uti/*.uti')):
    u=read_uti(open(f,'rb').read())
    for p in u.properties:
        if p.property_name==10 and p.subtype in hs:
            nm=tlk.get(u.name.stringref).text if u.name.stringref>=0 else u.name.get(0,0) if hasattr(u.name,'get') else ''
            r=bi.get_row(u.base_item)
            rows.append((os.path.basename(f)[:-4],nm,u.base_item,r.get_string('label'),p.subtype,u.stack_size,u.cost,u.charges))
for r in rows: print(r)
