import re,sys,os
from collections import defaultdict
sys.path.insert(0,os.getcwd()+"/.venv/lib/python3.12/site-packages")
from pykotor.resource.formats.twoda import read_2da
G=os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/")
t=read_2da(G+"override/spells.2da")
lab={i:t.get_row(i).get_string("label") for i in range(t.get_height())}
FAM={}
fams={"speed":(8,34,37),"armor":(17,18,24),"valor":(22,33,36),"eres":(42,40,133),"barrier":(135,136,137),"bmed":(138,139,140),"fury":(164,165,166),"heal":(),"revit":(173,174,175)}
for k,v in fams.items():
    for s in v: FAM[s]=k
DUR={"speed":36,"armor":20,"valor":20,"eres":60,"barrier":30,"bmed":20,"fury":30,"revit":0}  # vanilla base sec (approx; min of tiers)
rx=re.compile(r"T=([\d.]+) .*CMD kind=spell actor=(\w+) id=(\w+) tgt=(\w+) real=(\d)")
ev=[]
for ln in open(G+"cmdq-probe_log.txt",errors="replace"):
    m=rx.search(ln)
    if m: ev.append((float(m[1])/1000,m[2],int(m[3],16),int(m[5])))
for real in (0,1):
    last={}; hits=defaultdict(list); n=defaultdict(int)
    for ts,a,s,r in ev:
        if r!=real: continue
        f=FAM.get(s)
        if not f or not DUR.get(f): continue
        n[f]+=1
        k=(a,f)
        if k in last and ts-last[k]<DUR[f]: hits[f].append((a,lab[s],round(ts-last[k],1)))
        last[k]=ts
    print("real=%d"%real, {f:(n[f],len(hits[f])) for f in n})
    for f,h in hits.items(): print("  ",f,h[:6])
