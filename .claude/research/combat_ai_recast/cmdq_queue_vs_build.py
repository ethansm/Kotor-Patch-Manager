import re,os,sys
from collections import Counter,defaultdict
G=os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/")
from pykotor.resource.formats.twoda import read_2da
t=read_2da(G+"override/spells.2da")
def lab(i): return t.get_row(i).get_string("label").replace("FORCE_POWER_","") if 0<=i<t.get_height() else str(i)
def hostile(i): return 0<=i<t.get_height() and t.get_row(i).get_string("hostilesetting")=="1"
rx=re.compile(r"T=([\d.]+) .*CMD kind=spell actor=(\w+) id=(\w+) tgt=(\w+) real=(\d)")
q=defaultdict(list); c=Counter(); ex=[]
for ln in open(G+"cmdq-probe_log.txt",errors="replace"):
    m=rx.search(ln)
    if not m: continue
    ts,a,s,r=float(m[1])/1000,m[2],int(m[3],16),int(m[5])
    if s==0xffffffff: s=-1
    if r==0: q[a].append((ts,s)); continue
    hist=q[a]
    if not hist: c["build-no-queue"]+=1; continue
    cur=hist[-1]; prev=hist[-2] if len(hist)>1 else None
    if ts-cur[0]<0.5 and cur[1]==s: c["build=this round's queue"]+=1
    elif prev and prev[1]==s: c["build=PREVIOUS queue (stale)"]+=1; ex.append((round(ts,1),a,"queued",lab(cur[1]),"built",lab(s),"hostile" if hostile(s) else "friendly"))
    else: c["build=other"]+=1; ex.append((round(ts,1),a,"queued",lab(cur[1]),"built",lab(s),"other"))
print(dict(c))
for e in ex[:14]: print(e)
