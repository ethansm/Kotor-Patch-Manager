#!/bin/bash
# usage: prep_opus.sh B<n>  (run in steam_db_batches)
set -e
B=$1; S=/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/15a79d29-2bbc-45a2-9bd5-6dce1ab8b092/scratchpad
for f in $B/review/sonnet_c*.tsv; do c=${f##*_c}; c=${c%.tsv}; python3 tools/score_review.py $B/cards/key.tsv $f 2>&1 | grep -i "decision" | sed "s/^/c$c /"; done
python3 tools/batch.py finalize $B 2>&1 | tail -2
cp $B/evidence.tsv $S/${B}_ev_pre.tsv
python3 tools/batch.py gates $B 2>&1 | head -1
python3 tools/batch.py finalize $B 2>&1 | tail -2
python3 - $B $S <<'PY'
import csv,sys
b,s=sys.argv[1:3]
a={r["gog"]:r for r in csv.DictReader(open(f"{s}/{b}_ev_pre.tsv"),delimiter="\t")}
ev=list(csv.DictReader(open(f"{b}/evidence.tsv"),delimiter="\t"))
for r in ev:
    o=a[r["gog"]]
    if (o["gate_class"],o["steam_addr"])!=(r["gate_class"],r["steam_addr"]): print("  regate:",r["name"],o["gate_class"],o["steam_addr"],"->",r["gate_class"],r["steam_addr"])
rows=[r for r in ev if r["gate_class"] in ("REJECT","NOCAND")]
L=[f"# Opus gate {b} part r (REJECT/NOCAND rows, lead addition)","",
"- These rows have no shippable gate pick. For each, search the Steam binary for the true counterpart of the GOG body",
"  (whole-binary fp compare: size, retn, n_insn, callee set mapped via known names, disp/imm sets; neighbour runs).",
"- Ship (`verified`/`likely`) only a counterpart whose Steam body you checked against the GOG fp yourself (quote an asm line).",
"- If the GOG label contradicts the body, use `label_suspect` and name the Mac symbol.",
"- Otherwise `rejected` (gate pick wrong, none found) or `unresolved`. sample_errors does not apply to this part (write 0).","",
"| # | gate | Class::Name | GOG addr | gate pick | gog size/retn |","|---|---|---|---|---|---|"]
for i,r in enumerate(rows,1):
    L.append(f"| {i} | {r['gate_class']} | {r['class']}::{r['name']} | {r['gog']} | {r['steam_addr'] or '-'} | {r['gog_size']}/{r['gog_retn']} |")
import os; os.makedirs(f"{b}/opus",exist_ok=True)
if rows: open(f"{b}/opus/part_r.md","w").write("\n".join(L)+"\n")
print("part_r rows:",len(rows))
PY
python3 tools/batch.py opus-parts $B --rows 60 2>&1 | tail -1
