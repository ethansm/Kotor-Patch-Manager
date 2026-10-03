import sys,os,glob
sys.path.insert(0,os.path.dirname(__file__))
from utcs import summ
from pykotor.resource.formats.erf import read_erf
from pykotor.resource.formats.gff import read_gff
S=os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/cloudsaves/62703561")
for d in sorted(glob.glob(S+'/0*')):
    try:
        nfo=read_gff(open(d+'/savenfo.res','rb').read()).root
        pt=read_gff(open(d+'/PARTYTABLE.res','rb').read()).root
        mem=[(m.acquire('PT_MEMBER_ID',-9),m.acquire('PT_IS_LEADER',0)) for m in pt.get_list('PT_MEMBERS')]
        outer=read_erf(d+'/SAVEGAME.sav')
        avail={}
        mods=[]
        for r in outer:
            if r.restype.extension=='utc' and r.resref.lower().startswith('availnpc'):
                c=read_gff(r.data).root; s=summ(c); avail[r.resref]=(s['xp'],s['cls'])
            if r.restype.extension=='sav': mods.append(str(r.resref))
        last=nfo.acquire('LASTMODULE','')
        gitnpc=[]
        for r in outer:
            if r.restype.extension=='sav' and str(r.resref).lower()==last.lower():
                for rr in read_erf(r.data):
                    if rr.restype.extension=='git':
                        for c in read_gff(rr.data).root.get_list('Creature List'):
                            t=c.acquire('Tag','')
                            if t in('atton','kreia','t3m4','3cfd','bao-dur','visas','handmaiden','g0t0','hk47','mira','disciple','hanharr','mand'): gitnpc.append((t,c.acquire('Experience',0)))
        print(os.path.basename(d),'|',nfo.acquire('SAVEGAMENAME',''),nfo.acquire('AREANAME',''),'|members',mem,'|avail',avail,'|git',gitnpc,'|xp pool',pt.acquire('PT_XP_POOL',0))
    except Exception as e: print(d,'ERR',e)
