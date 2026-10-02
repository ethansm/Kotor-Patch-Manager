#!/usr/bin/env python3
"""Python port of the CharEffectsPanel stat decoder (atomOf/fmtAtom/maskName/dmgBonus/addAtom/formatStats/wrapAppend)
with hand-derived test cases and a --samples checker for real DECODE log lines.

Source of truth: ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/CharEffectsPanel/CharEffectsPanel.cpp
Usage: check_effect_decode.py [--samples chareffects_log.txt] [--inc PATH_TO_EffectNameTable.inc]
"""
import argparse, os, re, sys

DEFAULT_INC = os.path.expanduser(
    "~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/CharEffectsPanel/EffectNameTable.inc")
LINE_LEN, SHORT_LEN, STAT_LEN, MAX_LINES, MAX_ATOMS = 96, 48, 96, 8, 12

(A_NONE, A_ABIL, A_AC, A_ATT, A_DMG, A_RES, A_DR, A_IMMD, A_SAVE, A_SKILL, A_THP, A_REGEN, A_SPEED, A_IMM,
 A_FR, A_DEFL, A_FSHIELD, A_STATE, A_TFP, A_CONCEAL, A_DSHIELD, A_POISON, A_INVIS, A_ATTACKS, A_FPREG, A_VPREG) = range(26)

ABIL_S = ["Str", "Dex", "Con", "Int", "Wis", "Cha"]
ABIL_L = ["Strength", "Dexterity", "Constitution", "Intelligence", "Wisdom", "Charisma"]
SKILL_S = ["Comp", "Demo", "Stealth", "Aware", "Persuade", "Repair", "Security", "Treat Inj"]
SKILL_L = ["Computer Use", "Demolitions", "Stealth", "Awareness", "Persuade", "Repair", "Security", "Treat Injury"]
DMG_S = ["Bludg", "Pierce", "Slash", "Univ", "Acid", "Cold", "LS", "Elec", "Fire", "DS", "Sonic", "Ion", "Energy"]
DMG_L = ["Bludgeoning", "Piercing", "Slashing", "Universal", "Acid", "Cold", "Light Side", "Electrical",
         "Fire", "Dark Side", "Sonic", "Ion", "Energy"]
SAVE_S = ["Saves", "Fort", "Ref", "Will"]
SAVE_L = ["All saves", "Fortitude", "Reflex", "Will"]
SAVE_T = [None, "acid", "sneak attack", "cold", "death", "disease", "Light Side", "electrical", "fear", "fire",
          "mind-affecting", "Dark Side", "poison", "sonic", "traps", "Force powers", "ion", "blaster", "paralysis"]
IMMUNITY = ["none", "Mind", "Poison", "Disease", "Fear", "Traps", "Paralysis", "Blindness", "Deafness", "Slow",
            "Entangle", "Silence", "Stun", "Sleep", "Charm", "Dominate", "Confusion", "Curse", "Daze", "Ability loss",
            "Attack loss", "Damage loss", "Immunity loss", "Defense loss", "Slow", "Save loss", "FR loss",
            "Skill loss", "Knockdown", "Neg. level", "Sneak attack", "Critical hits", "Death", "Droid confusion"]
AC_TYPE = ["dodge", "natural", "armour", "shield", "deflection"]
STATES = {1: "Confused", 2: "Frightened", 3: "Droid stun", 4: "Stunned", 5: "Paralyzed", 6: "Asleep", 7: "Choked",
          8: "Horrified", 10: "Whirlwind", 15: "Crushed", 16: "Droid confused", 18: "Mind trick", 19: "Droid scrambled"}


def load_force_shield(path):
    txt = open(path).read()
    m = re.search(r"kForceShield\[kForceShieldCount\]\s*=\s*\{(.*?)\n\};", txt, re.S)
    rows = [(int(a), int(b)) for a, b in re.findall(r"\{\s*(\d+)\s*,\s*(\d+)\s*\}", m.group(1))]
    cnt = int(re.search(r"kForceShieldCount\s*=\s*(\d+)", txt).group(1))
    assert len(rows) == cnt, "kForceShield parse: %d rows vs count %d" % (len(rows), cnt)
    return rows


FORCE_SHIELD = []


def trunc(s, n):  # snprintf into a buffer of n bytes
    return s[:n - 1]


def mask_name(mask, lng, n):
    mask &= 0x1FFF
    bits = bin(mask).count("1")
    if not bits:
        return trunc("no" if lng else "?", n)
    if not lng:
        if mask == 0x1FFF: return trunc("All", n)
        if mask == 7: return trunc("Phys", n)
        if mask == (4096 | 128): return trunc("Energy", n)
        if mask == (4096 | 128 | 32 | 256 | 1024): return trunc("Elem", n)
        if bits > 3: return trunc("%d types" % bits, n)
    elif mask == 0x1FFF:
        return trunc("all", n)
    out, k = "", 0
    for i in range(13):
        if not (mask >> i) & 1: continue
        s = (("," + " " if lng else "/") if k else "") + (DMG_L[i] if lng else DMG_S[i])
        if len(s) >= n - len(out):
            return out + s[:n - len(out) - 1]
        out += s
        k += 1
    return out


def atom_of(typ, v):
    """returns (kind,a,b,mask,limit) or None"""
    I = lambda i: v[i] if i < len(v) else 0
    sg = 1
    t = typ
    neg = {0x25: 0x24, 0x31: 0x30, 0x0b: 0x0a, 0x0e: 0x0d, 0x11: 0x10, 0x1b: 0x1a, 0x38: 0x37, 0x1d: 0x1c,
           0x22: 0x21, 0x5d: 0x5c}
    if t in neg:
        sg, t = -1, neg[t]
    if t == 0x24:
        if I(0) < 0 or I(0) > 5: return None
        return (A_ABIL, sg * I(1), I(0), 0, 0)
    if t == 0x30: return (A_AC, sg * I(1), I(0), 0, 0)
    if t == 0x0a: return (A_ATT, sg * I(0), I(1), 0, 0)
    if t == 0x0d: return (A_DMG, sg * I(0), 0, I(1), 0)
    if t == 0x02: return (A_RES, I(1), 0, I(0), I(2))
    if t == 0x0c: return (A_DR, I(0), I(1), 0, I(2))
    if t == 0x10: return (A_IMMD, sg * I(1), 0, I(0), 0)
    if t == 0x1a: return (A_SAVE, sg * I(0), I(1), 0, I(2))
    if t == 0x37:
        if I(0) < 0 or I(0) > 7: return None
        return (A_SKILL, sg * I(1), I(0), 0, 0)
    if t == 0x0f: return (A_THP, I(0), 0, 0, 0)
    if t == 0x07: return (A_REGEN, I(0), I(1), 0, 0)
    if t == 0x1c: return (A_SPEED, sg * I(0), 0, 0, 0)
    if t == 0x16: return (A_IMM, 0, I(0), 0, 0)
    if t == 0x21: return (A_FR, sg * I(0), 0, 0, 0)
    if t == 0x5c: return (A_DEFL, sg * I(0), 0, 0, 0)
    if t == 0x6b:
        if I(0) < 0 or I(0) >= len(FORCE_SHIELD): return None
        amt, fl = FORCE_SHIELD[I(0)]
        return (A_FSHIELD, amt, I(0), fl, 0)
    if t == 0x08:
        if I(0) not in STATES: return None
        return (A_STATE, 0, I(0), 0, 0)
    if t == 0x5b: return (A_TFP, I(0), 0, 0, 0)
    if t == 0x4c: return (A_CONCEAL, I(0), 0, 0, 0)
    if t == 0x3d: return (A_DSHIELD, I(0), I(1), I(2), 0)
    if t == 0x23: return (A_POISON, 0, I(0), 0, 0)
    if t == 0x2f: return (A_INVIS, 0, I(0), 0, 0)
    if t == 0x2c: return (A_ATTACKS, I(0), 0, 0, 0)
    if t == 0x70: return (A_FPREG, I(0), 0, 0, 0)
    if t == 0x71: return (A_VPREG, I(0), 0, 0, 0)
    return None


def dmg_bonus(e, n=16):
    dice = ["1d4", "1d6", "1d8", "1d10", "2d6"]
    m = -e if e < 0 else e
    if 6 <= m <= 10: s = ("-" if e < 0 else "+") + dice[m - 6]
    else: s = "%+d" % e
    return trunc(s, n)


def fmt_atom(x, shn=SHORT_LEN, lgn=LINE_LEN):
    kind, a, b, mask, limit = x
    sh = lg = ""
    if kind == A_ABIL:
        sh, lg = "%+d %s" % (a, ABIL_S[b]), "%+d %s" % (a, ABIL_L[b])
    elif kind == A_AC:
        par = 0 < b < 5
        sh, lg = "Def %+d" % a, "Defense %+d%s" % (a, (" (" + AC_TYPE[b] + ")") if par else "")
    elif kind == A_ATT:
        sh = "Att %+d" % a
        lg = "Attack %+d%s" % (a, " (main hand)" if b == 1 else " (off hand)" if b == 2 else "")
    elif kind == A_DMG:
        m2 = mask_name(mask, True, 160); d = dmg_bonus(a)
        sh, lg = "Dmg %s" % d, "Damage %s %s" % (d, m2)
    elif kind == A_RES:
        m = mask_name(mask, False, 64); m2 = mask_name(mask, True, 160)
        sh = "Res %d %s" % (a, m)
        lg = "Resist %d %s (up to %d)" % (a, m2, limit) if limit > 0 else "Resist %d %s" % (a, m2)
    elif kind == A_DR:
        sh = "DR %d" % a
        lg = "Damage reduction %d (up to %d)" % (a, limit) if limit > 0 else "Damage reduction %d" % a
    elif kind == A_IMMD:
        m = mask_name(mask, False, 64); m2 = mask_name(mask, True, 160)
        sh, lg = "Imm %d%% %s" % (a, m), "%+d%% immunity: %s" % (a, m2)
    elif kind == A_SAVE:
        w = b if 0 <= b < 4 else 0
        vs = SAVE_T[limit] if 0 < limit < 19 else None
        sh = "%s %+d" % (SAVE_S[w], a)
        lg = "%s %+d%s" % (SAVE_L[w], a, (" vs " + vs) if vs else "")
    elif kind == A_SKILL:
        sh, lg = "%s %+d" % (SKILL_S[b], a), "%s %+d" % (SKILL_L[b], a)
    elif kind == A_THP:
        sh, lg = "+%d VP" % a, "+%d temporary Vitality" % a
    elif kind == A_REGEN:
        sec = (b + 500) // 1000 if b > 0 else 0   # C int division; b>0 so floor == trunc
        sh, lg = "Regen %d/%ds" % (a, sec), "Regenerates %d Vitality every %d s" % (a, sec)
    elif kind == A_SPEED:
        sh, lg = "Speed %+d%%" % a, "Movement speed %+d%%" % a
    elif kind == A_IMM:
        nm = IMMUNITY[b] if 0 <= b < 34 else "?"
        sh, lg = "Imm: %s" % nm, "Immune: %s" % nm
    elif kind == A_FR:
        sh, lg = "FR %+d" % a, "Force resistance %+d" % a
    elif kind == A_DEFL:
        sh, lg = "Defl %+d" % a, "Blaster deflection %+d" % a
    elif kind == A_FSHIELD:
        m = mask_name(mask, False, 64); m2 = mask_name(mask, True, 160)
        sh, lg = "Shield %d %s" % (a, m), "Absorbs %d: %s" % (a, m2)
    elif kind == A_STATE:
        sh = lg = STATES[b]
    elif kind == A_TFP:
        sh, lg = "+%d FP" % a, "+%d temporary Force points" % a
    elif kind == A_CONCEAL:
        sh, lg = "Conceal %d%%" % a, "%d%% concealment" % a
    elif kind == A_DSHIELD:
        m2 = mask_name(mask, True, 160)
        sh, lg = "Dmg shield %d" % a, "Damage shield %d %s" % (a, m2)
    elif kind == A_POISON: sh = lg = "Poisoned"
    elif kind == A_INVIS: sh = lg = "Invisible"
    elif kind == A_ATTACKS:
        sh, lg = "%+d attacks" % a, "%+d attack(s) per round" % a
    elif kind == A_FPREG:
        sh, lg = "FP regen %+d%%" % a, "Force point regeneration %+d%%" % a
    elif kind == A_VPREG:
        sh, lg = "VP regen %+d%%" % a, "Vitality regeneration %+d%%" % a
    return trunc(sh, shn), trunc(lg, lgn)


def add_atom(atoms, x):
    for i, y in enumerate(atoms):
        if y[0] == x[0] and y[1] == x[1] and y[2] == x[2] and y[4] == x[4]:
            atoms[i] = (y[0], y[1], y[2], y[3] | x[3], y[4]); return
    if len(atoms) < MAX_ATOMS: atoms.append(x)


def format_stats(atoms):
    shield = any(a[0] == A_FSHIELD for a in atoms)
    stat, lines = "", []
    for x in atoms:
        if shield and x[0] == A_RES and x[4] > 0: continue
        sh, lg = fmt_atom(x)
        if not sh: continue
        if lg in lines: continue
        if len(lines) < MAX_LINES: lines.append(lg)
        s = (", " if stat else "") + sh
        if len(s) < STAT_LEN - len(stat): stat += s     # w>0 && w < sizeof-len; else stat[len]=0 (unchanged)
    return stat, lines


def decode_entry(effects):
    atoms = []
    for typ, ints in effects:
        a = atom_of(typ, ints)
        if a: add_atom(atoms, a)
    return format_stats(atoms)


def wrap_append(src, max_px, width, st, cap):
    """st = {'len': int, 'buf': str}; buf is what a C reader of `out` sees (incl. a truncated failed emit)."""
    lines = 0
    cur, cl = "", 0

    def emit():
        nonlocal cur, cl, lines
        s = ("\n" if st["len"] else "") + cur
        if len(s) < cap - st["len"]:
            st["buf"] = st["buf"][:st["len"]] + s
            st["len"] += len(s); lines += 1
        else:
            st["buf"] = st["buf"][:st["len"]]   # C: out[*len] = 0 on overflow (no clipped tail)
        cur, cl = "", 0

    words = src.split(" ")
    for w in words:
        if not w: continue
        w = w[:120]
        cand = (cur + (" " if cl else "") + w)[:255]
        if cl and width(cand) > max_px:
            emit(); cur = w[:255]
        else:
            cur = cand
        cl = len(cur)
    if cl: emit()
    return lines


def wrap(src, max_chars, cap=1024):
    st = {"len": 0, "buf": ""}
    n = wrap_append(src, max_chars, len, st, cap)
    return n, st["buf"]


# ---------------------------------------------------------------- tests (expected values derived by hand from the C)
VALOR = [(0x24, [0, 2]), (0x24, [1, 2]), (0x24, [2, 2]), (0x1a, [2, 0, 0])]
SKILLS8 = [(0x37, [i, 1]) for i in range(8)]
SKILL_SH = "Comp +1, Demo +1, Stealth +1, Aware +1, Persuade +1, Repair +1, Security +1, Treat Inj +1"  # 89 chars
SKILL_LG = ["Computer Use +1", "Demolitions +1", "Stealth +1", "Awareness +1", "Persuade +1", "Repair +1",
            "Security +1", "Treat Injury +1"]
TESTS = [
    ("force valor", VALOR, "+2 Str, +2 Dex, +2 Con, Saves +2",
     ["+2 Strength", "+2 Dexterity", "+2 Constitution", "All saves +2"]),
    ("force armor", [(0x30, [0, 6]), (0x1a, [6, 0, 0])], "Def +6, Saves +6", ["Defense +6", "All saves +6"]),
    ("energy resistance merge", [(0x02, [m, 20, 0]) for m in (32, 128, 256, 1024, 4096)], "Res 20 Elem",
     ["Resist 20 Cold, Electrical, Fire, Sonic, Energy"]),
    ("energy shield item (res dropped)", [(0x6b, [6]), (0x02, [4096, 40, 40]), (0x02, [128, 40, 40])],
     "Shield 40 Energy", ["Absorbs 40: Electrical, Energy"]),
    ("limit res w/o shield kept", [(0x02, [4096, 40, 40]), (0x02, [128, 40, 40])], "Res 40 Energy",
     ["Resist 40 Electrical, Energy (up to 40)"]),
    ("shield keeps limit-0 res", [(0x6b, [6]), (0x02, [32, 20, 0])], "Shield 40 Energy, Res 20 Cold",
     ["Absorbs 40: Electrical, Energy", "Resist 20 Cold"]),
    ("shield + limit res reorder", [(0x02, [32, 20, 0]), (0x6b, [6])], "Res 20 Cold, Shield 40 Energy",
     ["Resist 20 Cold", "Absorbs 40: Electrical, Energy"]),
    ("burst of speed", [(0x1c, [50]), (0x30, [0, 2])], "Speed +50%, Def +2", ["Movement speed +50%", "Defense +2"]),
    ("decreases", [(0x25, [1, 2]), (0x38, [2, 4]), (0x11, [256, 50])], "-2 Dex, Stealth -4, Imm -50% Fire",
     ["-2 Dexterity", "Stealth -4", "-50% immunity: Fire"]),
    ("dmg increase enum", [(0x0d, [7, 4096])], "Dmg +1d6", ["Damage +1d6 Energy"]),
    ("dmg plain +3", [(0x0d, [3, 128])], "Dmg +3", ["Damage +3 Electrical"]),
    ("dmg decrease plain", [(0x0e, [3, 128])], "Dmg -3", ["Damage -3 Electrical"]),
    ("dmg decrease dice", [(0x0e, [8, 128])], "Dmg -1d8", ["Damage -1d8 Electrical"]),
    ("dmg 11 not dice", [(0x0d, [11, 128])], "Dmg +11", ["Damage +11 Electrical"]),
    ("mask 3 bits", [(0x10, [1 | 32 | 256, 50])], "Imm 50% Bludg/Cold/Fire", ["+50% immunity: Bludgeoning, Cold, Fire"]),
    ("mask Phys", [(0x10, [7, 50])], "Imm 50% Phys", ["+50% immunity: Bludgeoning, Piercing, Slashing"]),
    ("mask 2 bits", [(0x10, [3, 50])], "Imm 50% Bludg/Pierce", ["+50% immunity: Bludgeoning, Piercing"]),
    ("mask all", [(0x10, [0x1FFF, 10])], "Imm 10% All", ["+10% immunity: all"]),
    ("mask high bits ignored", [(0x10, [0x2000 | 1, 10])], "Imm 10% Bludg", ["+10% immunity: Bludgeoning"]),
    ("mask zero", [(0x10, [0, 50])], "Imm 50% ?", ["+50% immunity: no"]),
    ("long line kept whole (LineLen 96)", [(0x10, [0x1FFE, 50])], "Imm 50% 12 types",
     ["+50% immunity: Piercing, Slashing, Universal, Acid, Cold, Light Side, Electrical, Fire, Dark Side, Sonic, Ion, Energy"[:95]]),
    ("state stunned", [(0x08, [4])], "Stunned", ["Stunned"]),
    ("state unknown", [(0x08, [9])], "", []),
    ("unknown type 0x1e", [(0x1e, [1])], "", []),
    ("duplicate effects", VALOR + VALOR, "+2 Str, +2 Dex, +2 Con, Saves +2",
     ["+2 Strength", "+2 Dexterity", "+2 Constitution", "All saves +2"]),
    ("long-form dedupe (DR b differs)", [(0x0c, [5, 1, 0]), (0x0c, [5, 2, 0])], "DR 5", ["Damage reduction 5"]),
    ("DR with limit", [(0x0c, [5, 1, 20])], "DR 5", ["Damage reduction 5 (up to 20)"]),
    ("regen", [(0x07, [2, 6000])], "Regen 2/6s", ["Regenerates 2 Vitality every 6 s"]),
    ("regen rounding", [(0x07, [1, 1500])], "Regen 1/2s", ["Regenerates 1 Vitality every 2 s"]),
    ("regen zero period", [(0x07, [1, 0])], "Regen 1/0s", ["Regenerates 1 Vitality every 0 s"]),
    ("empty entry", [], "", []),
    ("ability bad index", [(0x24, [6, 2])], "", []),
    ("skill bad index", [(0x37, [8, 2])], "", []),
    ("force shield bad row", [(0x6b, [21])], "", []),
    ("force shield neg row", [(0x6b, [-1])], "", []),
    ("ac typed", [(0x30, [2, 3])], "Def +3", ["Defense +3 (armour)"]),
    ("ac type 5 no paren", [(0x30, [5, 3])], "Def +3", ["Defense +3"]),
    ("ac decrease", [(0x31, [0, 2])], "Def -2", ["Defense -2"]),
    ("attack decrease main", [(0x0b, [2, 1])], "Att -2", ["Attack -2 (main hand)"]),
    ("attack off hand", [(0x0a, [2, 2])], "Att +2", ["Attack +2 (off hand)"]),
    ("save vs disease", [(0x1a, [3, 1, 5])], "Fort +3", ["Fortitude +3 vs disease"]),
    ("save decrease reflex", [(0x1b, [3, 2, 0])], "Ref -3", ["Reflex -3"]),
    ("save bad type ignored", [(0x1a, [3, 3, 19])], "Will +3", ["Will +3"]),
    ("save bad which -> all", [(0x1a, [3, 7, 0])], "Saves +3", ["All saves +3"]),
    ("immunity", [(0x16, [2])], "Imm: Poison", ["Immune: Poison"]),
    ("immunity out of range", [(0x16, [40])], "Imm: ?", ["Immune: ?"]),
    ("temp hp / fp", [(0x0f, [10]), (0x5b, [5])], "+10 VP, +5 FP", ["+10 temporary Vitality", "+5 temporary Force points"]),
    ("force resist / deflection", [(0x21, [5]), (0x22, [3]), (0x5c, [2]), (0x5d, [1])],
     "FR +5, FR -3, Defl +2, Defl -1",
     ["Force resistance +5", "Force resistance -3", "Blaster deflection +2", "Blaster deflection -1"]),
    ("misc singles", [(0x4c, [25]), (0x23, [1]), (0x2f, [1]), (0x2c, [1]), (0x70, [10]), (0x71, [20])],
     "Conceal 25%, Poisoned, Invisible, +1 attacks, FP regen +10%, VP regen +20%",
     ["25% concealment", "Poisoned", "Invisible", "+1 attack(s) per round", "Force point regeneration +10%",
      "Vitality regeneration +20%"]),
    ("damage shield", [(0x3d, [5, 0, 128])], "Dmg shield 5", ["Damage shield 5 Electrical"]),
    ("missing ints default 0", [(0x24, [])], "+0 Str", ["+0 Strength"]),
    ("shield mask 7 row 10", [(0x6b, [10])], "Shield 50 Phys", ["Absorbs 50: Bludgeoning, Piercing, Slashing"]),
    ("stat 96 buffer: exactly 95 chars fits", SKILLS8[:7] + [(0x3d, [1234567, 0, 128])],
     SKILL_SH[:-14] + ", Dmg shield 1234567", SKILL_LG[:7] + ["Damage shield 1234567 Electrical"]),
    ("stat 96 buffer: overflow atom dropped + MaxLines 8", SKILLS8[:7] + [(0x3d, [1234567, 0, 128]), (0x5b, [5])],
     SKILL_SH[:-14] + ", Dmg shield 1234567", SKILL_LG[:7] + ["Damage shield 1234567 Electrical"]),   # 9th line dropped: MaxLines 8
    ("stat 96 buffer: 89 + 7 does not fit", SKILLS8 + [(0x24, [0, 1])], SKILL_SH, SKILL_LG),
]

WRAP_TESTS = [
    # (name, src, maxChars, expected lines list)
    ("wrap basic", "aaa bbb ccc ddd", 7, ["aaa bbb", "ccc ddd"]),
    ("wrap exact fit", "aaa bbb", 7, ["aaa bbb"]),
    ("wrap spaces collapsed", "  aa   bb  ", 10, ["aa bb"]),
    ("wrap long word alone", "a bbbbbbbbbb c", 5, ["a", "bbbbbbbbbb", "c"]),
    ("wrap word cut at 120", "x" * 130, 1000, ["x" * 120]),
    ("wrap empty", "", 10, []),
]


def run_tests():
    rows = []
    for name, effs, es, el in TESTS:
        s, l = decode_entry(effs)
        ok = (s == es and l == el)
        rows.append((name, ok, "" if ok else "got stat=%r lines=%r\n        want stat=%r lines=%r" % (s, l, es, el)))
    for name, src, mx, exp in WRAP_TESTS:
        n, buf = wrap(src, mx)
        ok = (buf == "\n".join(exp) and n == len(exp))
        rows.append((name, ok, "" if ok else "got %d %r want %r" % (n, buf, exp)))
    # wrap: multi-emit with shared out buffer ('\n' between separate calls) and cap overflow
    st = {"len": 0, "buf": ""}
    n1 = wrap_append("aa bb", 10, len, st, 1024); n2 = wrap_append("cc", 10, len, st, 1024)
    ok = (n1, n2, st["buf"]) == (1, 1, "aa bb\ncc")
    rows.append(("wrap two calls share buffer", ok, "" if ok else repr(st)))
    st = {"len": 0, "buf": ""}
    n = wrap_append("aaaa bbbb", 4, len, st, 8)   # line1 "aaaa" (4) ok; line2 "\nbbbb" = 5 >= 8-4 -> fails
    ok = (n == 1 and st["len"] == 4 and st["buf"] == "aaaa")
    rows.append(("wrap cap overflow: line dropped, no tail", ok, "" if ok else "%d %r" % (n, st)))
    return rows


SAMPLE_RE = re.compile(r"DECODE: type=0x([0-9a-fA-F]+) ints=\[([^\]]*)\] -> (?:\(no stat shown\)|'(.*)' / '(.*)')\s*$")


def run_samples(path):
    rows = []
    for ln, line in enumerate(open(path, errors="replace"), 1):
        if "DECODE:" not in line: continue
        m = SAMPLE_RE.search(line)
        name = "sample L%d" % ln
        if not m:
            rows.append((name, False, "unparsable: " + line.strip())); continue
        typ = int(m.group(1), 16)
        ints = [int(t) for t in m.group(2).split(",") if t.strip()]
        a = atom_of(typ, ints)
        if m.group(3) is None:
            ok = a is None
            rows.append((name + " 0x%02x%s none" % (typ, ints), ok, "" if ok else "port decodes: %r" % (fmt_atom(a),)))
        else:
            if a is None:
                rows.append((name + " 0x%02x%s" % (typ, ints), False, "port: no atom; log: %r" % (m.group(3),))); continue
            sh, lg = fmt_atom(a)
            ok = (sh, lg) == (m.group(3), m.group(4))
            rows.append((name + " 0x%02x%s" % (typ, ints), ok,
                         "" if ok else "got %r / %r want %r / %r" % (sh, lg, m.group(3), m.group(4))))
    return rows


def main():
    global FORCE_SHIELD
    ap = argparse.ArgumentParser()
    ap.add_argument("--samples", help="chareffects_log.txt copy with DECODE: lines")
    ap.add_argument("--inc", default=DEFAULT_INC)
    args = ap.parse_args()
    FORCE_SHIELD = load_force_shield(args.inc)
    rows = run_tests()
    if args.samples: rows += run_samples(args.samples)
    bad = 0
    for name, ok, detail in rows:
        print("%-4s %s" % ("PASS" if ok else "FAIL", name))
        if not ok: bad += 1; print("        " + detail)
    print("\n%d/%d passed (kForceShield rows parsed: %d)" % (len(rows) - bad, len(rows), len(FORCE_SHIELD)))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
