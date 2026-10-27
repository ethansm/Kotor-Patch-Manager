// EffectIcons.h -- spells.2da row -> buff / debuff icon resref tables + the per-creature icon list the Buff HUD draws.
// Header-only. The two tables are GENERATED (scripts/gen_buff_hud_icon_table.py, scripts/gen_debuff_icon_table.py); they were
// byte-identical copies in buff_duration_hud_src and active_effects_panel_src (doc 19b B1) and now live here once.
// Include as "../_shared/EffectIcons.h".
#pragma once
#include "Effects.h"
#include "EffectIconTable.inc"   // kEffectIcons[kEffectIconTableSize]: buff icon or nullptr
#include "DebuffIconTable.inc"   // kDebuffIcons[kDebuffIconTableSize]: { resref, item } for hostile rows
#include "EffectIconRowTable.inc" // kEffectIconRows[kEffectIconRowCount]: effecticon.2da { resref, good } for script EffectIcon (type 0x43) debuffs
#include <string.h>

namespace k2fx {

struct Buff { const char* resref; float ratio; bool item; };

// Buff (or debuff) icon for a spells.2da row; nullptr = not tracked. *item = draw on the round item-ability disc.
inline const char* iconForSpell(int spell, bool debuff, bool* item) {
    *item = false;
    if (spell < 0 || spell >= kEffectIconTableSize) return nullptr;
    if (debuff) { *item = kDebuffIcons[spell].item; return kDebuffIcons[spell].resref; }
    *item = isItemAbility(spell);
    return kEffectIcons[spell];
}

constexpr int EffType_Icon = 0x43;   // script SetEffectIcon: ints[0] = effecticon.2da row
constexpr int EffIconRow_TrapPoison = 110;   // TRAP_EFFECT_POISON: the 2da names the placeholder ip_resistenergy (no texture), so the table holds nullptr

// Opt-in icon for the poison row: called only when an effect carries that row, returns the resref once its texture exists, else nullptr.
// nullptr hook (default) or nullptr result = untracked, the engine draws its debuff arrow. Lazy so the texture is registered only in a real game
// with a poisoned creature (Buff HUD: embedded bh_poison, PoisonIconTex.inc); a consumer without that texture never sets the hook.
inline const char* (*g_trapPoisonIcon)() = nullptr;

// effecticon.2da row of a type-0x43 effect (ints[0]), else -1.
inline int effectIconRow(int eff) {
    unsigned short type = 0; int n = 0, ip = 0, row = 0;
    if (!rd(eff, Effect_Type, &type) || type != EffType_Icon) return -1;
    if (!rd(eff, Effect_IntCount, &n) || n <= 0 || !rd(eff, Effect_Ints, &ip) || !ip || !rd(ip, 0, &row)) return -1;
    return row;
}

// EffectPoison (type 0x23), as CSWSEffectListHandler::OnApplyPoison (Steam 0x59fd20) fills it on a failed save: ints[1]/[2] = world (day, ms) at
// apply, ints[5] = poison.2da DURATION (s), ints[6] = PERIOD (s); +0x0c holds DURATION * 1000 (ms, not s), so effectRatio does not fit it.
constexpr int EffType_Poison = 0x23;
struct PoisonTiming { int startDay, startMs, durSec, period; };
inline bool readPoison(int eff, PoisonTiming* t) {
    unsigned short type = 0; int n = 0, ip = 0;
    if (!rd(eff, Effect_Type, &type) || type != EffType_Poison) return false;
    if (!rd(eff, Effect_IntCount, &n) || n < 7 || !rd(eff, Effect_Ints, &ip) || !ip) return false;
    return rd(ip, 4, &t->startDay) && rd(ip, 8, &t->startMs) && rd(ip, 20, &t->durSec) && rd(ip, 24, &t->period) && t->durSec > 0;
}
// Remaining share of the longest-running poison in the effect array; 1.0 without a clock or a readable poison (draws fully bright, as before).
inline float poisonRatio(int arr, int count, int day, int ms, bool haveTime, float minRatio = 0.0f) {
    if (!haveTime) return 1.0f;
    float best = -1.0f;
    for (int i = 0; i < count; ++i) {
        int eff = 0; PoisonTiming t;
        if (!rd(arr, i * 4, &eff) || !readPoison(eff, &t)) continue;
        float r = remainingRatio(t.startDay, t.startMs + t.durSec * 1000, float(t.durSec), day, ms, minRatio);
        if (r > best) best = r;
    }
    return best < 0.0f ? 1.0f : best;
}

// Harmful effecticon.2da row (good == 0) carried by a type-0x43 effect -> its icon, else nullptr. Row 0 is the "none" row.
inline const char* iconForEffectIconEffect(int eff, bool* item) {
    *item = false;
    const int row = effectIconRow(eff);
    if (row == EffIconRow_TrapPoison && g_trapPoisonIcon) { if (const char* r = g_trapPoisonIcon()) return r; }
    if (row <= 0 || row >= kEffectIconRowCount || kEffectIconRows[row].good != 0 || !kEffectIconRows[row].resref) return nullptr;
    const char* r = kEffectIconRows[row].resref;
    *item = !strncmp(r, "ii_", 3) || !strncmp(r, "iw_", 3);
    return r;
}

// Distinct tracked spell ids on the creature, in application order, each with its remaining-duration ratio.
inline int collectBuffs(uintptr_t creature, int day, int ms, bool haveTime, Buff* out, int maxOut, bool debuff = false, float minRatio = 0.0f) {
    int arr = 0, count = 0;
    if (!effectListNonEmpty(creature, &arr, &count)) return 0;
    int seen[MaxEffects]; int nSeen = 0; int n = 0;
    const char* iconSeen[MaxEffects]; int nIcon = 0;   // icons added from type-0x43 rows (debuff only)
    for (int i = 0; i < count && n < maxOut; ++i) {
        int eff, spell;
        if (!rd(arr, i * 4, &eff) || !rd(eff, Effect_SpellId, &spell)) continue;
        bool item = false;
        const char* resref = iconForSpell(spell, debuff, &item);
        bool viaIcon = false;
        if (!resref && debuff) { resref = iconForEffectIconEffect(eff, &item); viaIcon = resref != nullptr; }   // script-applied debuff (Slow): no mappable spell id
        if (!resref) continue;
        bool dup = false;
        if (viaIcon) { for (int k = 0; k < n; ++k) if (!strcmp(out[k].resref, resref)) { dup = true; break; } }   // spell -1 repeats: dedupe by icon
        else {
            for (int k = 0; k < nSeen; ++k) if (seen[k] == spell) { dup = true; break; }
            for (int k = 0; !dup && k < nIcon; ++k) if (!strcmp(iconSeen[k], resref)) dup = true;               // same icon already drawn via an EffectIcon row
        }
        if (dup) continue;
        if (viaIcon) { if (nIcon < MaxEffects) iconSeen[nIcon++] = resref; } else seen[nSeen++] = spell;
        const bool poison = viaIcon && effectIconRow(eff) == EffIconRow_TrapPoison;   // the icon effect has no duration: drain by the poison itself
        out[n++] = { resref, poison ? poisonRatio(arr, count, day, ms, haveTime, minRatio) : effectRatio(eff, day, ms, haveTime, minRatio), item };
    }
    return n;
}

} // namespace k2fx
