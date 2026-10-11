import json,re
def load(p):
    d=json.load(open(p)); out=[]
    for t,items in d["tiers"].items():
        for i in items:
            ls=i.get("line_start"); le=i.get("line_end")
            if ls is None:
                m=re.match(r"(\d+)-?(\d+)?",str(i.get("lines","")));
                if m: ls=int(m[1]); le=int(m[2] or m[1])
            out.append((t,i.get("file"),ls,le or ls,i.get("id"),i.get("title") or i.get("summary") or ""))
    return out
new=load(".claude/deep-review/p1/verified_findings.json")
old=load(".claude/reviews/verified_findings.v2.json")
files={n[1] for n in new}
oldf=[o for o in old if o[1] in files]
print("old findings in same files:",len(oldf),"| new:",len(new))
def ov(a,b): return a[1]==b[1] and a[2] is not None and b[2] is not None and a[2]<=b[3]+3 and b[2]<=a[3]+3
for n in new:
    m=[o for o in oldf if ov(n,o)]
    print("NEW",n[0],n[1].split('/')[-1],n[2],n[3],"->", [(o[0],o[2],o[3]) for o in m] or "NOT IN OLD")
print("--- old items not rediscovered (in the pilot's changed range only can matter):")
for o in oldf:
    if not any(ov(n,o) for n in new): print(" ",o[0],o[1].split('/')[-1],o[2],o[3],o[5][:60])
