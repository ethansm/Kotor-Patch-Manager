"""Fold cross-batch duplicate topics in topics.tsv and write topics.md. Run from the review dir."""
import csv
from collections import Counter

rows = list(csv.DictReader(open("topics.tsv", newline=""), delimiter="\t", quoting=csv.QUOTE_NONE))
by = {r["tid"]: r for r in rows}

MERGES = [
    ("X-T01", ["HB-T10", "CO-T01", "SH-T01"], [], "P1",
     "Do guikit::detail::wr (GuiKit.h) and CompanionAI.cpp wr<T> guard the store with only a readability probe (probeRead / rd<T> / readAt), with no probeWrite or writable-protection check, so a committed read-only target would fault?",
     "patch_manager_mods/_shared_src/GuiKit.h:100-140;patch_manager_mods/_shared_src/SafeRead.h:60-135;patch_manager_mods/companion_ai_src/CompanionAI.cpp:95-110"),
    ("X-T02", ["HB-T03", "SH-T02", "CO-T11", "HN-T05"], ["QH-2a-10"], "P1",
     "Does saferead::readAt compute base+off in 32-bit uintptr_t with no wraparound check, so a high (corrupt) base plus positive off wraps to a low address that passes probeRead's checks and reads an unrelated location?",
     "patch_manager_mods/_shared_src/SafeRead.h:60-135;patch_manager_mods/holster_src/Holster.cpp:150-182"),
    ("X-T03", ["CO-T12", "SH-T05"], ["QH-2a-11"], "P2",
     "In embtex::heal, is ++s.heals done before detail::fill and the MaxHeals give-up message logged when heals==MaxHeals even if that same fill succeeded, and does embtex_test.cpp only count the line without checking it follows a successful fill?",
     "patch_manager_mods/_shared_src/EmbedTex.h:120-163;patch_manager_mods/_shared_src/embtex_test.cpp:60-130"),
    ("X-T04", ["HB-T11", "SH-T06"], [], "P2",
     "Do the failure returns in createLabelFromTemplate (CF_Vtable/CF_Template/CF_Count/CF_Flags) and bindByTag (CF_Vtable before initControl) return 0 after g_gk.alloc (+ labelCtor) without destroying or freeing the allocated control?",
     "patch_manager_mods/_shared_src/GuiKit.h:150-215;patch_manager_mods/_shared_src/GuiKit.h:536-600"),
    ("X-T05", ["HB-T15", "SH-T08"], [], "P2",
     "Answer each separately: (a) WindowHook::install returns true when old is set without checking h==hwnd or IsWindow(hwnd); (b) guikit wheelInstall's static subOk is never reset by wheelUninstall, so it reinstalls on a stale hwnd; (c) k2input::Client::connect calls onConnect(false,...) on every attempt with no reportedMissing guard?",
     "patch_manager_mods/_shared_src/WindowHook.h:20-56;patch_manager_mods/_shared_src/GuiKit.h:650-705;patch_manager_mods/_shared_src/GuiInput.h:76-135"),
]

drop, new, log = set(), [], []
for tid, src, extra, pri, q, ent in MERGES:
    fids = []
    for s in src:
        fids += [x.strip() for x in by[s]["fids"].split(",")]
        drop.add(s)
    fids += extra
    new.append({"batch": "cross", "tid": tid, "fids": ",".join(fids), "priority": pri, "question": q, "entries": ent})
    log.append(f"- **{tid}** ({pri}) <- {', '.join(src)}{' + ' + ', '.join(extra) if extra else ''}")
# QH-T09 split: its two fids went to X-T02 and X-T03
by["QH-T09"]["fids"] = ""
drop.add("QH-T09")
log.append("- QH-T09 split: QH-2a-10 -> X-T02, QH-2a-11 -> X-T03")

out = [r for r in rows if r["tid"] not in drop] + new
cols = ["batch", "tid", "fids", "priority", "question", "entries"]
with open("topics.tsv", "w", newline="") as f:
    w = csv.DictWriter(f, cols, delimiter="\t", quoting=csv.QUOTE_NONE, escapechar="\\", lineterminator="\n")
    w.writeheader()
    w.writerows(out)

fc = Counter()
for r in out:
    for x in r["fids"].split(","):
        fc[x] += 1
fids = [r["fid"] for r in csv.DictReader(open("findings.tsv", newline=""), delimiter="\t", quoting=csv.QUOTE_NONE)]
bad = [x for x in fids if fc[x] != 1] + [x for x in fc if x not in fids]
order = {"P1": 0, "P2": 1, "P3": 2}
live = sorted([r for r in out if r["tid"] != "dropped"], key=lambda r: (order.get(r["priority"], 9), r["tid"]))
with open("topics.md", "w") as f:
    f.write("# Gemini 2026-10-14: investigation topics\n\n")
    f.write(f"{len(fids)} findings -> {len(live)} topics ({Counter(r['priority'] for r in live)}). "
            "Source rows: `findings.tsv`, `topics.tsv`; anchor check `anchors.tsv`. "
            "Per-batch notes (in-batch duplicates, out-of-bundle citations, not-assessed backlog): `triage/<batch>.topics.md`.\n\n")
    f.write("## Cross-batch merges\n" + "\n".join(log) + "\n\n## Topics\n| tid | pri | fids | question |\n|---|---|---|---|\n")
    for r in live:
        f.write(f"| {r['tid']} | {r['priority']} | {r['fids'].replace(',', ', ')} | {r['question'].replace('|', '/')} |\n")
print("topics", len(live), Counter(r["priority"] for r in live), "BAD" if bad else "OK", bad)
