#!/usr/bin/env python
"""One-off generator for paths/<char>.toml (skill points allocated greedily by a priority list, capped at char_level+3).
Feat/power picks are the explicit tables below; edit them here (or the TOML directly) and re-run check_paths.py.
Usage: ../../.venv/bin/python gen_paths.py  (2>/dev/null)"""
import os
from levelup_rules import Rules, SKILLS, CLASS_ID, JEDI

R = Rules()
HERE = os.path.dirname(os.path.abspath(__file__))


def build(tag, name, utc, start_skills, start_int, levels, skill_prio, header):
    """levels: list of (char_level, class_id, feats, powers, attrs)"""
    sk = dict(start_skills)
    out = [header, "[meta]", f'tag = "{tag}"', f'name = "{name}"', f'utc = "{utc}"', ""]
    for cl, cls, feats, powers, attrs in levels:
        pts = R.skill_points(cls, start_int)
        alloc = {}
        prio = skill_prio(cl, cls)
        while pts > 0:
            for s in prio:
                idx = SKILLS.index(s)
                cost = 1 if R.skill_is_class(idx, cls) else 2
                if cost <= pts and sk[s] < cl + 3:
                    sk[s] += 1; alloc[s] = alloc.get(s, 0) + 1; pts -= cost
                    break
            else:
                break  # nothing affordable / under cap
        out.append("[[level]]")
        out.append(f"char_level = {cl}")
        out.append(f"class = {cls}   # {R.cell(R.classes, cls, 'label')}")
        if attrs:
            out.append("attrs = {" + ", ".join(f"{k} = {v}" for k, v in attrs.items()) + "}")
        out.append("feats = [" + ", ".join(map(str, feats)) + "]" + ("   # " + ", ".join(R.feat_label(f) for f in feats) if feats else ""))
        if powers or cls in JEDI:
            out.append("powers = [" + ", ".join(map(str, powers)) + "]" + ("   # " + ", ".join(R.spell_label(p) for p in powers) if powers else ""))
        out.append("skills = {" + ", ".join(f"{k} = {v}" for k, v in alloc.items()) + "}")
        out.append("")
    open(os.path.join(HERE, "paths", f"{tag}.toml"), "w").write("\n".join(out))


# ---------------- Atton: Scoundrel 3 -> Scoundrel 19 (char 4..19), Sentinel 1..9 (char 20..28) ----------------
SC, SN, CN = CLASS_ID["Scoundrel"], CLASS_ID["JediSentinel"], CLASS_ID["JediConsular"]
atton = []
scd_feats = {5: [20], 8: [77], 11: [3], 14: [9], 17: [85]}       # Improved/Master Sniper Shot, TWF, Improved TWF, Master TWF (elkripper)
dex = {4: 17, 8: 18, 12: 19, 16: 20, 20: 21, 24: 22, 28: 23}
for cl in range(4, 20):
    atton.append((cl, SC, scd_feats.get(cl, []), [], {"DEX": dex[cl]} if cl in dex else {}))
# Sentinel (elkripper): 1 Burst of Speed+Heal | 2 Knight Speed | 3 Master Speed | 4 Improved Heal | 5 Master Heal | ...
# This install's spells.2da gates by CHARACTER level (Heal 6, Knight Speed 9, Improved Heal 12, Master Speed 15, Master Heal 18) so those all fit at char 20+.
sn_feats = {1: [139], 3: [140], 6: [240], 7: [241], 9: [242]}      # Close Combat, Improved CC, Precise Shot I-III (last picks are placeholders per elkripper)
sn_pow = {1: [8, 10], 2: [34], 3: [37], 4: [28], 5: [134], 6: [162], 7: [163], 8: [46], 9: [29]}  # 8 Burst,10 Heal | 34 Knight Speed | 37 Master Speed | 28 Impr Heal | 134 Master Heal | 162 Deflection | 163 Redirection | 46 Stun | 29 Stasis
for k in range(1, 10):
    atton.append((19 + k, SN, sn_feats.get(k, []), sn_pow[k], {"DEX": dex[19 + k]} if (19 + k) in dex else {}))
build("atton", "Atton Rand", "p_atton.utc",
      dict(computer_use=3, demolitions=4, stealth=4, awareness=5, persuade=0, repair=0, security=5, treat_injury=0), 10, atton,
      lambda cl, cls: ["awareness", "stealth", "security", "demolitions"] if cls == SC else ["awareness", "stealth", "security", "treat_injury", "computer_use"],
      "# Atton: pre-conversion Scoundrel 4-19, convert at char 20 (Sentinel 1-9). Feats follow elkripper; skill points are a PROPOSAL (greedy by priority).\n# Jedi-conversion levels can only be live-tested once story progress allows.")

# ---------------- Kreia: Consular 4 .. 30 (elkripper caster build, 'Kreia beg' section) ----------------
kf = {6: [143], 9: [84], 12: [123], 15: [124], 18: [114], 21: [115], 24: [13], 27: [21], 30: [22]}
kp = {4: [22], 5: [18], 6: [10, 24], 7: [138], 8: [135], 9: [34, 40], 10: [33], 11: [136], 12: [28, 139], 13: [17], 14: [30],
      15: [37, 41], 16: [20], 17: [133], 18: [134, 140], 19: [137], 20: [36], 21: [31, 50], 22: [9], 23: [32], 24: [45, 15],
      25: [11], 26: [7], 27: [38, 153], 28: [154], 29: [27], 30: [155, 26]}
wis = {4: 17, 8: 18, 12: 19, 16: 20, 20: 21, 24: 22, 28: 23}
kreia = [(cl, CN, kf.get(cl, []), kp.get(cl, []), {"WIS": wis[cl]} if cl in wis else {}) for cl in range(4, 31)]
build("kreia", "Kreia", "p_kreia.utc",
      dict(computer_use=0, demolitions=0, stealth=6, awareness=6, persuade=5, repair=0, security=3, treat_injury=6), 14, kreia,
      lambda cl, cls: ["awareness", "treat_injury", "repair"],
      "# Kreia: Jedi Consular 4-30, elkripper caster build. Dark-side powers (Affliction, Plague, Drain*, Death Field, Kill...) reduced by Alignment Affects Force Powers if she is not dark.\n# Skill points are a PROPOSAL (greedy by priority). Test subject: live save has her at Consular 6.")
