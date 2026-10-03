import csv,sys,glob,os
# usage: merge_ov.py B<n> : merge B<n>/opus/overrides_*.tsv -> B<n>/overrides.tsv (later parts win; consolidation file 'overrides_z' wins last)
b=sys.argv[1]; out={}
for f in sorted(glob.glob(f"{b}/opus/overrides_*.tsv")):
    tag=os.path.basename(f)[10:-4]
    for r in csv.DictReader(open(f),delimiter="\t"):
        g=r["gog_addr"].strip().lower()
        if not g: continue
        if g in out and out[g]["steam_addr"]!=r["steam_addr"]: print("CONFLICT",g,out[g]["steam_addr"],out[g]["final_verdict"],"vs",tag,r["steam_addr"],r["final_verdict"])
        note=r["note"] if r["note"].startswith("[") else f"[{tag}] {r['note']}"
        out[g]=dict(gog_addr=g,steam_addr=r["steam_addr"].strip().lower(),final_verdict=r["final_verdict"].strip(),note=note.replace("\t"," "))
with open(f"{b}/overrides.tsv","w") as fh:
    fh.write("gog_addr\tsteam_addr\tfinal_verdict\tnote\n")
    for g in sorted(out): r=out[g]; fh.write("\t".join([r["gog_addr"],r["steam_addr"],r["final_verdict"],r["note"]])+"\n")
from collections import Counter; print(b,"overrides",len(out),Counter(r["final_verdict"] for r in out.values()))
