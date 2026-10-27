// ChareffDecode.h - the pure, engine-free part of CharEffectsPanel.cpp (doc 20 Phase 5 #4): the effect stat decoder
// (effect type + ints[] -> short stat form + long tooltip line) and the pixel-width text layer (textPx / fitText /
// marqueeText / wrapAppend). Moved verbatim out of the DLL source so the host T1 test (chareff_decode_test.cpp) can run
// the exact code against the Python oracle scripts/check_effect_decode.py. Nothing here touches engine memory, g_cfg or
// Slot: the text functions take the three values they used from them (sy, text_k10, text_scale) as a TextCtx and the DLL
// keeps thin Slot* wrappers. Includes EffectNameTable.inc (generated, global scope) for the forceshields rows, so the .cpp
// must NOT include that file itself. The text layer now lives in the shared TextFit.h (textfit::); cfx re-exports it via using.
#pragma once
#include <stdio.h>
#include <string.h>
#include "EffectNameTable.inc"   // scripts/gen_effect_name_table.py: spells/effecticon names + icons, forceshields
#include "TextFit.h"   // shared text layer (font widths + textPx/fitText/marqueeText/wrapAppend); Patches/Common / -I../_shared_src

namespace cfx {

constexpr int MaxLines = 8, LineLen = 96;   // long lines are word-wrapped in the tooltip, never cut
struct Entry {
    bool section, harmful, gap, item, perm;
    int key;              // dedup key: 0x10000 + effecticon row, or spells.2da row
    int spell;            // spells.2da row of the effect that made the entry (-1 none): stat sibling match + description
    double remMs;         // < 0 = unknown
    char name[64]; char icon[17];
    char stat[96];        // stat column: short forms joined with ", " (cut with "..." at layout time)
    char val[24];   // Stats tab value text; empty for effect rows
    char lines[MaxLines][LineLen]; int nLines;   // tooltip: one long form per stat
    const char* desc;     // tooltip description (kSpellDesc) or a fixed text, nullptr = none
};
// ---- stat decoder (research R3 + R3b): effect type + ints[] -> short form (stat column) + long form (tooltip line) ----
// Decrease types store positive magnitudes; the sign comes from the type. Effects of one entry that differ only in the
// damage type (e.g. Energy Resistance = 5 DamageResistance effects) merge into one atom with the OR of the type masks.
enum AtomKind { A_NONE, A_ABIL, A_AC, A_ATT, A_DMG, A_RES, A_DR, A_IMMD, A_SAVE, A_SKILL, A_THP, A_REGEN, A_SPEED, A_IMM,
                A_FR, A_DEFL, A_FSHIELD, A_STATE, A_TFP, A_CONCEAL, A_DSHIELD, A_POISON, A_INVIS, A_ATTACKS, A_FPREG, A_VPREG };
struct Atom { int kind, a, b, mask, limit; };
const char* const kAbilShort[6] = {"Str", "Dex", "Con", "Int", "Wis", "Cha"};
const char* const kAbilLong[6] = {"Strength", "Dexterity", "Constitution", "Intelligence", "Wisdom", "Charisma"};
const char* const kSkillShort[8] = {"Comp", "Demo", "Stealth", "Aware", "Persuade", "Repair", "Security", "Treat Inj"};
const char* const kSkillLong[8] = {"Computer Use", "Demolitions", "Stealth", "Awareness", "Persuade", "Repair", "Security", "Treat Injury"};
// damage flag bit i (nwscript DAMAGE_TYPE_*, = iprp_damagetype row i): 4096 is the in-game "Energy" (blaster)
const char* const kDmgShort[13] = {"Bludg", "Pierce", "Slash", "Univ", "Acid", "Cold", "LS", "Elec", "Fire", "DS", "Sonic", "Ion", "Energy"};
const char* const kDmgLong[13] = {"Bludgeoning", "Piercing", "Slashing", "Universal", "Acid", "Cold", "Light Side", "Electrical",
                                  "Fire", "Dark Side", "Sonic", "Ion", "Energy"};
const char* const kSaveShort[4] = {"Saves", "Fort", "Ref", "Will"};
const char* const kSaveLong[4] = {"All saves", "Fortitude", "Reflex", "Will"};
const char* const kSaveType[19] = {nullptr, "acid", "sneak attack", "cold", "death", "disease", "Light Side", "electrical", "fear",
                                   "fire", "mind-affecting", "Dark Side", "poison", "sonic", "traps", "Force powers", "ion",
                                   "blaster", "paralysis"};
const char* const kImmunity[34] = {"none", "Mind", "Poison", "Disease", "Fear", "Traps", "Paralysis", "Blindness", "Deafness",
                                   "Slow", "Entangle", "Silence", "Stun", "Sleep", "Charm", "Dominate", "Confusion", "Curse",
                                   "Daze", "Ability loss", "Attack loss", "Damage loss", "Immunity loss", "Defense loss",
                                   "Slow", "Save loss", "FR loss", "Skill loss", "Knockdown", "Neg. level", "Sneak attack",
                                   "Critical hits", "Death", "Droid confusion"};
const char* const kAcType[5] = {"dodge", "natural", "armour", "shield", "deflection"};
inline const char* stateName(int st) {
    switch (st) {
    case 1: return "Confused"; case 2: return "Frightened"; case 3: return "Droid stun"; case 4: return "Stunned";
    case 5: return "Paralyzed"; case 6: return "Asleep"; case 7: return "Choked"; case 8: return "Horrified";
    case 10: return "Whirlwind"; case 15: return "Crushed"; case 16: return "Droid confused"; case 18: return "Mind trick";
    case 19: return "Droid scrambled"; default: return nullptr;
    }
}
// damage-type mask: one name, a known group, up to three names "Fire/Ion/Energy", or "N types"; long = every name
inline void maskName(int mask, bool lng, char* out, int n) {
    mask &= 0x1FFF;
    int bits = 0; for (int i = 0; i < 13; ++i) bits += (mask >> i) & 1;
    out[0] = 0;
    if (!bits) { snprintf(out, n, "%s", lng ? "no" : "?"); return; }
    if (!lng) {
        if (mask == 0x1FFF) { snprintf(out, n, "All"); return; }
        if (mask == 7) { snprintf(out, n, "Phys"); return; }
        if (mask == (4096 | 128)) { snprintf(out, n, "Energy"); return; }               // blaster + electrical (energy shields)
        if (mask == (4096 | 128 | 32 | 256 | 1024)) { snprintf(out, n, "Elem"); return; }   // Energy Resistance: + cold, fire, sonic
        if (bits > 3) { snprintf(out, n, "%d types", bits); return; }   // "<first> +N" read like a bonus
    } else if (mask == 0x1FFF) { snprintf(out, n, "all"); return; }
    int k = 0, len = 0;
    for (int i = 0; i < 13; ++i) {
        if (!((mask >> i) & 1)) continue;
        int w = snprintf(out + len, n - len, "%s%s", k ? (lng ? ", " : "/") : "", lng ? kDmgLong[i] : kDmgShort[i]);
        if (w < 0 || w >= n - len) return;
        len += w; ++k;
    }
}
inline bool atomOf(int type, const int* v, int n, Atom* at) {
    auto I = [&](int i) { return i < n ? v[i] : 0; };
    Atom x = {A_NONE, 0, 0, 0, 0};
    int sg = 1;
    switch (type) {
    case 0x25: sg = -1; /* fallthrough */
    case 0x24: if (I(0) < 0 || I(0) > 5) return false; x = {A_ABIL, sg * I(1), I(0), 0, 0}; break;
    case 0x31: sg = -1; /* fallthrough */
    case 0x30: x = {A_AC, sg * I(1), I(0), 0, 0}; break;
    case 0x0b: sg = -1; /* fallthrough */
    case 0x0a: x = {A_ATT, sg * I(0), I(1), 0, 0}; break;
    case 0x0e: sg = -1; /* fallthrough */
    case 0x0d: x = {A_DMG, sg * I(0), 0, I(1), 0}; break;   // a = DAMAGE_BONUS_* enum 1..10
    case 0x02: x = {A_RES, I(1), 0, I(0), I(2)}; break;
    case 0x0c: x = {A_DR, I(0), I(1), 0, I(2)}; break;
    case 0x11: sg = -1; /* fallthrough */
    case 0x10: x = {A_IMMD, sg * I(1), 0, I(0), 0}; break;
    case 0x1b: sg = -1; /* fallthrough */
    case 0x1a: x = {A_SAVE, sg * I(0), I(1), 0, I(2)}; break;   // limit = save type (0 all)
    case 0x38: sg = -1; /* fallthrough */
    case 0x37: if (I(0) < 0 || I(0) > 7) return false; x = {A_SKILL, sg * I(1), I(0), 0, 0}; break;
    case 0x0f: x = {A_THP, I(0), 0, 0, 0}; break;
    case 0x07: x = {A_REGEN, I(0), I(1), 0, 0}; break;
    // increase: the apply handler 0x59e970 stores v+100 when v < 100 (script 99 -> 199) and multiplies the rate by
    // ints[0]/100, clamped to [0.125, 1.5] (0x5766a0); decrease 0x59eb80 stores v raw, rate *= 1 - v/100 (R3c)
    case 0x1d: x = {A_SPEED, -I(0), 0, 0, 0}; break;
    case 0x1c: x = {A_SPEED, I(0) >= 100 ? I(0) - 100 : I(0), 0, 0, 0}; break;
    case 0x16: x = {A_IMM, 0, I(0), 0, 0}; break;
    case 0x22: sg = -1; /* fallthrough */
    case 0x21: x = {A_FR, sg * I(0), 0, 0, 0}; break;
    case 0x5d: sg = -1; /* fallthrough */
    case 0x5c: x = {A_DEFL, sg * I(0), 0, 0, 0}; break;
    case 0x6b: if (I(0) < 0 || I(0) >= kForceShieldCount) return false; x = {A_FSHIELD, kForceShield[I(0)].amount, I(0), kForceShield[I(0)].flags, 0}; break;
    case 0x08: if (!stateName(I(0))) return false; x = {A_STATE, 0, I(0), 0, 0}; break;
    case 0x5b: x = {A_TFP, I(0), 0, 0, 0}; break;
    case 0x4c: x = {A_CONCEAL, I(0), 0, 0, 0}; break;
    case 0x3d: x = {A_DSHIELD, I(0), I(1), I(2), 0}; break;
    case 0x23: x = {A_POISON, 0, I(0), 0, 0}; break;
    case 0x2f: x = {A_INVIS, 0, I(0), 0, 0}; break;
    case 0x2c: x = {A_ATTACKS, I(0), 0, 0, 0}; break;
    case 0x70: x = {A_FPREG, I(0), 0, 0, 0}; break;
    case 0x71: x = {A_VPREG, I(0), 0, 0, 0}; break;
    default: return false;   // 0x1e visual, 0x43 icon, 0x44 innate, link, heal/damage instants, ...: nothing to show
    }
    *at = x; return true;
}
inline const char* dmgBonus(int e, char* buf, int n) {   // DAMAGE_BONUS_* 1..5 = +1..+5, 6..10 = 1d4 1d6 1d8 1d10 2d6
    static const char* const dice[5] = {"1d4", "1d6", "1d8", "1d10", "2d6"};
    int m = e < 0 ? -e : e;
    if (m >= 6 && m <= 10) snprintf(buf, n, "%c%s", e < 0 ? '-' : '+', dice[m - 6]); else snprintf(buf, n, "%+d", e);
    return buf;
}
inline void fmtAtom(const Atom& x, char* sh, int shn, char* lg, int lgn) {
    char m[64], m2[160], d[16];
    sh[0] = lg[0] = 0;
    switch (x.kind) {
    case A_ABIL: snprintf(sh, shn, "%+d %s", x.a, kAbilShort[x.b]); snprintf(lg, lgn, "%+d %s", x.a, kAbilLong[x.b]); break;
    case A_AC: snprintf(sh, shn, "Def %+d", x.a); snprintf(lg, lgn, "Defense %+d%s%s%s", x.a, x.b > 0 && x.b < 5 ? " (" : "", x.b > 0 && x.b < 5 ? kAcType[x.b] : "", x.b > 0 && x.b < 5 ? ")" : ""); break;
    case A_ATT: snprintf(sh, shn, "Att %+d", x.a); snprintf(lg, lgn, "Attack %+d%s", x.a, x.b == 1 ? " (main hand)" : x.b == 2 ? " (off hand)" : ""); break;
    case A_DMG: maskName(x.mask, true, m2, sizeof m2); dmgBonus(x.a, d, sizeof d);
        snprintf(sh, shn, "Dmg %s", d); snprintf(lg, lgn, "Damage %s %s", d, m2); break;
    case A_RES: maskName(x.mask, false, m, sizeof m); maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Res %d %s", x.a, m);
        if (x.limit > 0) snprintf(lg, lgn, "Resist %d %s (up to %d)", x.a, m2, x.limit); else snprintf(lg, lgn, "Resist %d %s", x.a, m2); break;
    case A_DR: snprintf(sh, shn, "DR %d", x.a);
        if (x.limit > 0) snprintf(lg, lgn, "Damage reduction %d (up to %d)", x.a, x.limit); else snprintf(lg, lgn, "Damage reduction %d", x.a); break;
    case A_IMMD: maskName(x.mask, false, m, sizeof m); maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Imm %d%% %s", x.a, m); snprintf(lg, lgn, "%+d%% immunity: %s", x.a, m2); break;
    case A_SAVE: { int w = x.b >= 0 && x.b < 4 ? x.b : 0; const char* vs = x.limit > 0 && x.limit < 19 ? kSaveType[x.limit] : nullptr;
        snprintf(sh, shn, "%s %+d", kSaveShort[w], x.a);
        snprintf(lg, lgn, "%s %+d%s%s", kSaveLong[w], x.a, vs ? " vs " : "", vs ? vs : ""); break; }
    case A_SKILL: snprintf(sh, shn, "%s %+d", kSkillShort[x.b], x.a); snprintf(lg, lgn, "%s %+d", kSkillLong[x.b], x.a); break;
    case A_THP: snprintf(sh, shn, "+%d VP", x.a); snprintf(lg, lgn, "+%d temporary Vitality", x.a); break;
    case A_REGEN: { int sec = x.b > 0 ? (int)(((long long)x.b + 500) / 1000) : 0;
        snprintf(sh, shn, "Regen %d/%ds", x.a, sec); snprintf(lg, lgn, "Regenerates %d Vitality every %d s", x.a, sec); break; }
    case A_SPEED: snprintf(sh, shn, "Speed %+d%%", x.a); snprintf(lg, lgn, "Movement speed %+d%%%s", x.a, x.a > 50 ? " (engine cap +50%)" : ""); break;
    case A_IMM: { const char* nm = x.b >= 0 && x.b < 34 ? kImmunity[x.b] : "?";
        snprintf(sh, shn, "Imm: %s", nm); snprintf(lg, lgn, "Immune: %s", nm); break; }
    case A_FR: snprintf(sh, shn, "FR %+d", x.a); snprintf(lg, lgn, "Force resistance %+d", x.a); break;
    case A_DEFL: snprintf(sh, shn, "Defl %+d", x.a); snprintf(lg, lgn, "Blaster deflection %+d", x.a); break;
    case A_FSHIELD: maskName(x.mask, false, m, sizeof m); maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Shield %d %s", x.a, m); snprintf(lg, lgn, "Absorbs %d: %s", x.a, m2); break;
    case A_STATE: snprintf(sh, shn, "%s", stateName(x.b)); snprintf(lg, lgn, "%s", stateName(x.b)); break;
    case A_TFP: snprintf(sh, shn, "+%d FP", x.a); snprintf(lg, lgn, "+%d temporary Force points", x.a); break;
    case A_CONCEAL: snprintf(sh, shn, "Conceal %d%%", x.a); snprintf(lg, lgn, "%d%% concealment", x.a); break;
    case A_DSHIELD: maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Dmg shield %d", x.a); snprintf(lg, lgn, "Damage shield %d %s", x.a, m2); break;
    case A_POISON: snprintf(sh, shn, "Poisoned"); snprintf(lg, lgn, "Poisoned"); break;
    case A_INVIS: snprintf(sh, shn, "Invisible"); snprintf(lg, lgn, "Invisible"); break;
    case A_ATTACKS: snprintf(sh, shn, "%+d attacks", x.a); snprintf(lg, lgn, "%+d attack(s) per round", x.a); break;
    case A_FPREG: snprintf(sh, shn, "FP regen %+d%%", x.a); snprintf(lg, lgn, "Force point regeneration %+d%%", x.a); break;
    case A_VPREG: snprintf(sh, shn, "VP regen %+d%%", x.a); snprintf(lg, lgn, "Vitality regeneration %+d%%", x.a); break;
    default: break;
    }
}
// per entry: atoms in effect-array order, merged by (kind, a, b, limit) with OR-ed masks; formatted when collect ends
constexpr int MaxAtoms = 12;
struct AtomSet { Atom at[MaxAtoms]; int n; };
inline void addAtom(AtomSet& set, const Atom& x) {
    for (int i = 0; i < set.n; ++i) {
        Atom& y = set.at[i];
        if (y.kind == x.kind && y.a == x.a && y.b == x.b && y.limit == x.limit) { y.mask |= x.mask; return; }
    }
    if (set.n < MaxAtoms) set.at[set.n++] = x;
}
inline void formatStats(Entry& e, const AtomSet& set) {
    bool shield = false; for (int i = 0; i < set.n; ++i) shield |= set.at[i].kind == A_FSHIELD;
    e.stat[0] = 0; e.nLines = 0; int len = 0;
    for (int i = 0; i < set.n; ++i) {
        const Atom& x = set.at[i];
        if (shield && x.kind == A_RES && x.limit > 0) continue;   // a force shield's own absorb pieces: the Shield line says it
        char sh[48], lg[LineLen];
        fmtAtom(x, sh, sizeof sh, lg, sizeof lg);
        if (!sh[0]) continue;
        bool dup = false; for (int k = 0; k < e.nLines; ++k) if (!strcmp(e.lines[k], lg)) { dup = true; break; }
        if (dup) continue;
        if (e.nLines < MaxLines) { snprintf(e.lines[e.nLines], LineLen, "%s", lg); ++e.nLines; }
        int w = snprintf(e.stat + len, sizeof e.stat - len, "%s%s", len ? ", " : "", sh);
        if (w > 0 && w < (int)sizeof e.stat - len) len += w; else { e.stat[len] = 0; }
    }
}

// ---- pixel-width text layer: moved to the shared TextFit.h (textfit::); cfx keeps the old names so callers are unchanged ----
using textfit::Font; using textfit::F16; using textfit::F10; using textfit::TextCtx;
using textfit::textPx; using textfit::fitText; using textfit::marqueeText; using textfit::wrapAppendW; using textfit::wrapAppend;
using textfit::TextK; using textfit::kFontTexW; using textfit::kFont10TexW; using textfit::kFont10LineH;

} // namespace cfx
