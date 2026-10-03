#!/usr/bin/env python3
"""C1b insert: R10 level-up UI identities -> patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db.

DRY-RUN BY DEFAULT.  `--write` commits.  Idempotent: add-only (never updates/overwrites an existing row; a name conflict
is reported and skipped), a second --write adds 0 rows.  Run from anywhere.

Gating (RE_AGENT_GUIDE sec 1: a Haiku `match` alone never promotes a row):
  --verdicts FILE   Haiku output: JSON list (or JSONL) of {"card": "C012", "verdict": "match|mismatch|unsure", ...}
  --gate FILE       Opus/lead decisions: {"0x0090C230": {"decision": "approve", "confidence": "high", "name": "optional override",
                    "tier": "opus|reviewer"}, ...}.  `decision` approve | reject.
A candidate is written only when: its REAL card verdict is `match`, BOTH decoy cards are NOT `match`, AND the gate approves it
(a gate approval may override a Haiku miss by setting "sonnet_override": true).  Without --gate, --write is refused.

Tables written per approved function candidate (all add-only):
  name_map(steam_addr '0x%08x', name, source, confidence, evidence, date)
  functions(binary_hash steam pristine, address '%08x', symbol, size, purpose, notes, discovered_date)
  steam_validated(ledger mark: verdict verified, tier, batch C1b, fp_sha '' (server fp.tsv not on disk here), evidence_ref)
Per approved vtable card: classes + class_methods (all 28 slots from R10_out_vtables.txt).
Per approved site card: hooks_ported (planned, not installed).  Plus one lessons row.
Never touched: any existing row (0x77D800 is reported as a name conflict, see candidates.NOT_NOMINATED).
"""
import argparse, json, os, re, sqlite3, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "C1b_scripts"))
import candidates as C
REPO = os.path.abspath(os.path.join(HERE, "../../.."))
DB = os.path.join(REPO, "patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db")
DUMP = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/")

ap = argparse.ArgumentParser()
ap.add_argument("--write", action="store_true"); ap.add_argument("--db", default=DB)
ap.add_argument("--verdicts"); ap.add_argument("--gate")
ap.add_argument("--key", default=os.path.join(HERE, "C1b_answer_key.json"))
a = ap.parse_args()

def h8(x): return "0x%08x" % x
def f8(x): return "%08x" % x
def load_json(p):
    t = open(p).read().strip()
    try: return json.loads(t)
    except ValueError: return [json.loads(l) for l in t.splitlines() if l.strip()]

key = load_json(a.key)["cards"]
verd = {}
if a.verdicts:
    v = load_json(a.verdicts); v = v if isinstance(v, list) else list(v.values())
    verd = {x["card"]: x["verdict"] for x in v}
gate = {k.lower(): v for k, v in (load_json(a.gate) if a.gate else {}).items()}
if a.write and not a.gate: sys.exit("REFUSED: --write needs --gate (Opus/lead decisions); Haiku match alone never promotes a row")

# per candidate address: real card id + decoy ids
cand_cards = {}
for cid, c in key.items():
    d = cand_cards.setdefault(c["cand_addr"].lower(), {"real": None, "decoys": []})
    (d.__setitem__("real", cid) if c["kind"] == "real" else d["decoys"].append(cid))

def status(addr):
    """-> (ok, why).  addr int."""
    cc = cand_cards.get(h8(addr))
    if not cc or not cc["real"]: return False, "no card"
    g = gate.get(h8(addr), {})
    if g.get("decision") == "reject": return False, "gate rejected"
    if not verd: haiku = None
    else: haiku = verd.get(cc["real"]) == "match" and all(verd.get(d) != "match" for d in cc["decoys"]) and len(cc["decoys"]) >= 2
    if g.get("decision") == "approve" and (haiku or g.get("sonnet_override")): return True, "gate approved, haiku %s" % ("pass" if haiku else "override")
    if g.get("decision") == "approve": return False, "gate approved but haiku/decoy check failed (set sonnet_override to force)"
    if haiku is None: return False, "no verdicts yet"
    return False, "haiku %s, awaiting gate" % ("pass" if haiku else "FAIL (needs Sonnet)")

db = sqlite3.connect(a.db)
db.execute("BEGIN")
added = {"name_map": 0, "functions": 0, "steam_validated": 0, "classes": 0, "class_methods": 0, "hooks_ported": 0, "lessons": 0}
skipped = []
def one(sql, args=()): return db.execute(sql, args).fetchone()

def dump_size(addr):
    p = os.path.join(DUMP, "decomp", f8(addr)[:2], f8(addr) + ".c")
    try: m = re.search(r"size=(\d+)", open(p).readline()); return int(m.group(1)) if m else None
    except OSError: return None

def call_note(addr):
    out = []
    for k in C.CALLS:
        if k["parent"] == addr and status(k["addr"])[0]:
            out.append("call site 0x%08X: %s" % (k["addr"], k["role"]))
    return " | ".join(out)

print("DB:", a.db, "| mode:", "WRITE" if a.write else "DRY-RUN", "| verdicts:", bool(verd), "| gate:", bool(gate))
for c in C.FUNCS:
    ok, why = status(c["addr"]); ad = h8(c["addr"])
    row = one("select name, source from name_map where steam_addr=?", (ad,))
    if row:
        msg = "EXISTS name_map '%s' (%s)%s" % (row[0], row[1], "" if row[0] == c["name"] else " -- NAME CONFLICT with proposed %s, not written" % c["name"])
        skipped.append((ad, msg)); print("SKIP", ad, msg); continue
    g = gate.get(ad.lower(), {})
    name = g.get("name", c["name"]); conf = g.get("confidence", c["conf"])
    if c["conf"] == "medium" or (conf == "high" and c["conf"] == "medium"): conf = "medium"
    ev = "%s [R10 C1b: %s; anchors: %s; Haiku card %s + %d decoys; %s; class prefix of grid/panel names is a label, verify via RTTI if ever needed] %s" % (
        c["purpose"], c["ev"], c["anchor"], cand_cards.get(ad.lower(), {}).get("real"), len(cand_cards.get(ad.lower(), {}).get("decoys", [])), why, call_note(c["addr"]))
    print(("ADD " if ok else "HOLD"), ad, name, conf, "-", why)
    if not ok: continue
    db.execute("insert into name_map values(?,?,?,?,?,?)", (ad, name, C.SRC, conf, ev.strip(), C.DATE)); added["name_map"] += 1
    if not one("select 1 from functions where binary_hash=? and address=?", (C.BIN, f8(c["addr"]))):
        db.execute("insert into functions values(?,?,?,?,?,?,?)", (C.BIN, f8(c["addr"]), name, dump_size(c["addr"]), c["purpose"], "C1b R10: " + c["anchor"] + " (" + c["ev"] + ")", C.DATE)); added["functions"] += 1
    if not one("select 1 from steam_validated where steam_addr=? and name=?", (ad, name)):
        db.execute("insert into steam_validated(steam_addr,name,verdict,tier,gog_addr,batch,fp_sha,evidence_ref,date) values(?,?,?,?,?,?,?,?,?)",
                   (ad, name, "verified", g.get("tier", "reviewer"), "", "C1b", "", C.R10, C.DATE)); added["steam_validated"] += 1

# vtables -> classes + class_methods (structural dump of R10_out_vtables.txt; only slot 15 gets a purpose)
vt_txt = open(os.path.join(HERE, "R10_out_vtables.txt")).read().split("## vtable ")[1:]
vtd = {}
for b in vt_txt:
    vtd[int(b.split()[0], 16)] = [(int(m.group(1)), int(m.group(2), 16)) for m in re.finditer(r"slot (\d+) \+0x[0-9A-F]+ = 0x([0-9A-F]+)", b)]
for ctor, vt, cls, _w in C.VTABLES:
    ok, why = status(ctor)
    if one("select 1 from classes where binary_hash=? and class_name=?", (C.BIN, cls)) or one("select 1 from classes where binary_hash=? and vftable_address=?", (C.BIN, f8(vt))):
        print("SKIP class", cls, "exists"); continue
    print(("ADD " if ok else "HOLD"), "class", cls, h8(vt), "-", why)
    if not ok: continue
    db.execute("insert into classes values(?,?,?,?,?)", (C.BIN, cls, f8(vt), None, "Constructor 0x%08X writes vftable 0x%08X (C1b R10 sec 0; Haiku-verified)." % (ctor, vt))); added["classes"] += 1
    for idx, ad in vtd[vt]:
        if one("select class_name from class_methods where binary_hash=? and address=?", (C.BIN, f8(ad))):
            continue  # shared boilerplate slot already recorded under another class (UNIQUE binary_hash+address); first class keeps it
        db.execute("insert into class_methods values(?,?,?,?,?,?,?,?)", (C.BIN, cls, idx, f8(ad), dump_size(ad), "FUN_" + f8(ad),
                   "input/event switch (case table 0x27/0x2D accept, 0x28/0x2E back, 0x29 click, 0x2A recommended, arrows)" if idx == 15 else None, "C1b R10_out_vtables.txt")); added["class_methods"] += 1

for s in C.SITES:
    ok, why = status(s["addr"]); notes = "PLANNED research site, NOT installed (R10 sec e, no overlap +-16 vs repo/live hooks, live==vanilla bytes). %s. Consumed-exit 0x%08X = %s. Do not hook the 0x%08X call instead." % (s["role"], s["exit"], s["exit_role"], 0)
    notes = notes.replace("Do not hook the 0x00000000 call instead.", "").strip()
    if one("select 1 from hooks_ported where patch_id=? and hook_function=?", ("pc-levelup-filter", s["id"])):
        print("SKIP site", s["id"], "exists"); continue
    print(("ADD " if ok else "HOLD"), "site", s["id"], h8(s["addr"]), "-", why)
    if not ok: continue
    db.execute("insert into hooks_ported values(?,?,?,?,?,?,?,?,?)", ("pc-levelup-filter", s["id"], None, None, C.BIN, h8(s["addr"]), s["bytes"].upper(), notes, C.DATE)); added["hooks_ported"] += 1

topic = "level-up feat/power/skill grids: grey vs hide, accept trap (C1b/R10)"
if not one("select 1 from lessons where topic=?", (topic,)) and any(v for v in added.values()):
    db.execute("insert into lessons(scope,topic,note,date) values(?,?,?,?)", ("level-up UI", topic,
        "Feats/powers use a 3-column icon grid (rows built once per open: 0x89B600 feats, 0x89AAA0 powers) plus a separate CHOOSABLE vector (0x90C230 via CanChooseFeat; 0x9098D0 via 0x848660). "
        "Slot state 3 = dimmed: in-grid-but-not-choosable shows greyed, hidden = no grid row. Selection is by object id, never index. Accept refuses when picks left and choosable count >= 1 "
        "(popups 0xBE5A / 0x1EBA5), so a hide filter must act on grid row AND choosable vector. The Recommended button (0x90B6B0 -> AddDefaultPackageFeats 0x8489F0) bypasses the filter. "
        "R10 typo: skills slot-15 handler is 0x90E850, not 0x90E860. 0x77D800 (takes ecx=client creature, 0x7F2540 then vt+0x30) kept its older name; see C1b report. Source: " + C.R10, C.DATE)); added["lessons"] += 1

print("\nSKIPPED existing:", len(skipped), skipped)
print("WOULD ADD" if not a.write else "ADDED", added)
ic = db.execute("pragma integrity_check").fetchone()[0]; print("integrity_check:", ic)
if a.write and ic == "ok": db.commit(); print("committed")
else:
    db.rollback(); print("rolled back (dry-run)" if not a.write else "ROLLED BACK (integrity)")
