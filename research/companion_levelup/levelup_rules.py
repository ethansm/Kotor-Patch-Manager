"""Load KOTOR II rule tables (feat/spells/featgain/classpowergain/skills/classes/exptable) from the LIVE install
so path specs can be validated. pykotor from ../../.venv; run with stderr suppressed (it is very chatty)."""
import os
from pykotor.extract.installation import Installation
from pykotor.resource.type import ResourceType
from pykotor.resource.formats.twoda import read_2da

GAME = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
SKILLS = ["computer_use", "demolitions", "stealth", "awareness", "persuade", "repair", "security", "treat_injury"]  # skills.2da rows 0-7
CLASS_ABBR = {0: "sol", 1: "sct", 2: "scd", 3: "jgd", 4: "jcn", 5: "jsn", 6: "drc", 7: "drx", 9: "tec"}
JEDI = {3: "jgd", 4: "jcn", 5: "jsn"}
JEDI_PWR_COL = {3: "guardian", 4: "consular", 5: "sentinel"}  # spells.2da columns
CLASS_ID = {"Soldier": 0, "Scout": 1, "Scoundrel": 2, "JediGuardian": 3, "JediConsular": 4, "JediSentinel": 5, "TechSpecialist": 9, "CombatDroid": 6, "ExpertDroid": 7}


def _int(s, d=None):
    try:
        return int(s)
    except (ValueError, TypeError):
        return d


class Rules:
    def __init__(self, game=GAME):
        inst = Installation(game)
        T = lambda n: read_2da(inst.resource(n, ResourceType.TwoDA).data)
        self.tlk = inst.talktable()
        self.feat, self.spells, self.classes = T("feat"), T("spells"), T("classes")
        self.featgain, self.pwrgain, self.skills = T("featgain"), T("classpowergain"), T("skills")
        self.exptable = T("exptable")

    def cell(self, t, row, col, d=""):
        try:
            return t.get_row(row).get_string(col)
        except Exception:
            return d

    def feat_label(self, i):
        n = _int(self.cell(self.feat, i, "name"))
        s = self.tlk.string(n) if n is not None else None
        return f"{i}:{s or self.cell(self.feat, i, 'label')}"

    def spell_label(self, i):
        n = _int(self.cell(self.spells, i, "name"))
        s = self.tlk.string(n) if n is not None else None
        return f"{i}:{s or self.cell(self.spells, i, 'label')}"

    # --- feats ---
    def feat_selectable(self, feat, cls):
        """feat.2da <cls>_list: 0/1 = selectable, 3 = class-granted, 4 = not available."""
        return self.cell(self.feat, feat, CLASS_ABBR[cls] + "_list") in ("0", "1")

    def feat_granted_level(self, feat, cls):
        return _int(self.cell(self.feat, feat, CLASS_ABBR[cls] + "_granted"), -1)

    def feat_slots(self, cls, class_level):
        return _int(self.cell(self.featgain, class_level - 1, CLASS_ABBR[cls] + "_reg"), 0) + \
               _int(self.cell(self.featgain, class_level - 1, CLASS_ABBR[cls] + "_bon"), 0)

    def feat_prereqs(self, feat):
        g = lambda c: _int(self.cell(self.feat, feat, c), 0)
        req = [g("prereqfeat1"), g("prereqfeat2")]
        req = [r for r in req if r > 0]
        # prereqfeat "0" is a legitimate feat id only for feat 0 (Armour prof?) - treated as none, as the engine table uses ****/blank
        ors = [x for x in (_int(self.cell(self.feat, feat, f"orreqfeat{k}"), -1) for k in range(5)) if x > 0]
        return dict(minchar=g("mincharlevel"), minattack=g("minattackbonus"), str=g("minstr"), dex=g("mindex"),
                    int=g("minint"), wis=g("minwis"), req=req, orreq=ors, reqskill=self.cell(self.feat, feat, "reqskill"),
                    gainmultiple=self.cell(self.feat, feat, "gainmultiple"))

    # --- powers ---
    def power_min_class_level(self, spell, cls):
        """spells.2da guardian/consular/sentinel: minimum CHARACTER level (Kreia gets Heal=6 at Consular 6; Atton takes Heal at char lvl 20 as Sentinel 1); 99 / -1 = never selectable."""
        return _int(self.cell(self.spells, spell, JEDI_PWR_COL[cls]), -1)

    def power_prereqs(self, spell):
        """spells.2da 'prerequisites': spell ids joined by '_' (e.g. Master Heal '10_28'); '' / '****' = none."""
        v = self.cell(self.spells, spell, "prerequisites")
        return [int(x) for x in v.split("_") if x.isdigit()]

    def power_slots(self, cls, class_level):
        return _int(self.cell(self.pwrgain, class_level - 1, JEDI[cls]), 0)

    # --- skills ---
    def skill_is_class(self, skill_idx, cls):
        return self.cell(self.skills, skill_idx, CLASS_ABBR[cls] + "_class") == "1"

    def skill_points(self, cls, int_score):
        base = _int(self.cell(self.classes, cls, "skillpointbase"), 1)
        return max(1, base + (int_score - 10) // 2)
