import json, re
T = ".claude/deep-review/p2c/triage"
pf = json.load(open(f"{T}/prefilter.json"))
H = {  # id: (verdict, evidence, source)
 "8ea1731f8816": ("STALE-FIXED", "df26bd3 (QL1 identity guard) QueueHud.cpp:1095-1144; skip[k] blocks dispatch when buffer owner is in no slot; residual: address reuse (ABA) unverified", "haiku+orchestrator"),
 "d682bb833922": ("STILL-REAL", "QueueHud.cpp:1634-1635 `if (fb) memberK = 0;` non-party caster still mapped to member 0", "haiku+orchestrator"),
 "9059eb31f144": ("STILL-REAL", "QueueHud.cpp:1162-1167 actionInQueue includes T_CAST_SPELL (spec in 30_queue_hud.md may intend it)", "haiku"),
 "f77ae8184a09": ("STILL-REAL", "QueueHudLogic.h:674-679 shouldReInject returns hasFollowLeader, no completion signal", "haiku"),
 "e606f41d177d": ("STILL-REAL", "scripts/02_merge_2da_and_tlk.py:116,234-236 (untracked file)", "haiku+orchestrator"),
 "119182a14fc2": ("STILL-REAL", "scripts/plan_manager.py:531 bare-cell match, scoped 'NN:Tn' ident (:348) never matches; fallback append :578-580", "haiku+orchestrator"),
 "bc58247fd299": ("STILL-REAL", "ActionHotbar.cpp:385 OID-only match; needs live evidence a non-Exile shares sentinel 0xFFFFFFFF", "haiku"),
 "d74365c9d82d": ("STILL-REAL", "CompanionSelfHeal.cpp:770,852 leader `have` never cleared (moved from 696/777)", "haiku"),
 "822afff6842c": ("STILL-REAL", "generate_ultrawide_gui_overrides.py:694-696 probe swallows exception -> False (untracked)", "haiku"),
 "6ccefb375bd5": ("STILL-REAL", "generate_ultrawide_gui_overrides.py:1681-1683 check cannot MISMATCH; result discarded :1835", "haiku"),
 "6a2684bd832c": ("STILL-REAL", "neocities_mods_detailed.json:10 still advises AV override for unsigned exe ('HERE' link not re-checked)", "haiku"),
 "843bd1103025": ("UNCLEAR", "LevelUpOverride.cpp:164-180 logging moved to KpLog core; the Common KpLog.h source is not in this repo, so cap/flush behaviour cannot be checked", "haiku+orchestrator"),
 "d2c66b77d42f": ("STILL-REAL", "research/companion_levelup/tools/parity/scenario_test.cpp:418-419 logcap branch still empty (moved from 408-409)", "haiku"),
 "5a447a280525": ("UNCLEAR", "stub/LuoLogHost.h:9 remove+KpLog_SetFallbackFile; stale-fd behaviour depends on KpLog.h, absent from repo", "haiku+orchestrator"),
}
out = []
for o in pf:
    if o["id"] in H: v, ev, src = H[o["id"]]
    else:
        q = o["evidence_quotes"][0]
        v, ev, src = "STILL-REAL", f"{q['file']}:{q['line']} quote verbatim present; no commits or edits since 7020d54", "prefilter"
    claim = re.split(r"(?<=[.!?])\s", o["claim"].replace("\n", " "), 1)[0][:220]
    out.append(dict(id=o["id"], tier=o["tier"], severity=o["final_severity"], file=o["file"], lines=o["lines"], claim=claim, verdict=v, evidence=ev, source=src))
json.dump(out, open(f"{T}/triage.json", "w"), indent=1)
from collections import Counter
c = Counter(x["verdict"] for x in out); print(len(out), dict(c))
sev = {"critical":0,"high":1,"medium":2,"low":3}
def tab(rows, cols):
    s = "| " + " | ".join(cols) + " |\n|" + "---|"*len(cols) + "\n"
    for r in rows: s += "| " + " | ".join(str(r[k]).replace("|","/") for k in cols) + " |\n"
    return s
for x in out:
    x["fl"] = f"{x['file']}:{x['lines']}"
    x["flag"] = " **needs human judgment before any fix**" if x["tier"].startswith("CONTESTED") else ""
    x["claim2"] = x["claim"] + x["flag"]
real = sorted([x for x in out if x["verdict"]=="STILL-REAL"], key=lambda x: sev.get(x["severity"],9))
stale = [x for x in out if x["verdict"].startswith("STALE")]
unc = [x for x in out if x["verdict"]=="UNCLEAR"]
md = f"""# p2c findings triage (read-only; $0 dr.py spend)
Tree: HEAD efc5335 + working tree, vs audit pin 8ba3029 (earliest pin 7020d54). Verdicts: prefilter (quote check + git log), Haiku recon x4, Opus spot-checks.

## 1. Summary
28 representatives = **{c['STILL-REAL']} STILL-REAL + {len(stale)} STALE + {len(unc)} UNCLEAR**. The 13 REFUTED findings were not triaged. Of the {c['STILL-REAL']} real: {sum(1 for x in real if x['source']=='prefilter')} by deterministic prefilter (file untouched since the pin, quotes verbatim), {sum(1 for x in real if x['source']!='prefilter')} by Haiku recon.

## 2. STILL-REAL (by severity)
{tab(real, ['id','severity','tier','fl','claim2','evidence'])}
## 3. STALE (fixed by other sessions)
{tab(stale, ['id','severity','fl','evidence'])}Who fixed: df26bd3 "feat(queue-hud 0.5.0-dev): pause restore, ghost cur, identity guard, combat hand-off (QL1)" (2026-10-11).

## 4. UNCLEAR
{tab(unc, ['id','severity','tier','fl','evidence'])}Both depend on the Common `KpLog.h` core, which is not in this worktree; resolve by reading it where it lives, or ask the human.

## 5. Audit gaps that still apply
- refute-audit `batch_02` never ran.
- 26 queued verify units never verified (waived via `[coverage] allow_unverified`).
- Medium-and-below findings beyond the 60 queued were not verified.
- 2 map packets waived (`other-024`, `other-035`).

## 6. Before the fix half
- Add a `[fix]` section to the p2c profile (model: `.claude/deep-review/pilot/profile.toml`).
- Add `hook-asm`, `cpp-logic`, `gate-integrity` to `specialty_names`.
- `python3 $DR fix_intake.py --stats` is the free find-only check; `fix-go` and `findings-skim` are the user's gates.
"""
open(f"{T}/triage.md","w").write(md)
