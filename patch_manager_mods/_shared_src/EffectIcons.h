// EffectIcons.h -- spells.2da row -> buff / debuff icon resref tables + the per-creature icon list the Buff HUD draws.
// Header-only. The two tables are GENERATED (scripts/gen_buff_hud_icon_table.py, scripts/gen_debuff_icon_table.py); they were
// byte-identical copies in buff_duration_hud_src and active_effects_panel_src (doc 19b B1) and now live here once.
// Include as "../_shared/EffectIcons.h".
#pragma once
#include "Effects.h"
#include "EffectIconTable.inc"   // kEffectIcons[kEffectIconTableSize]: buff icon or nullptr
#include "DebuffIconTable.inc"   // kDebuffIcons[kDebuffIconTableSize]: { resref, item } for hostile rows

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

// Distinct tracked spell ids on the creature, in application order, each with its remaining-duration ratio.
inline int collectBuffs(uintptr_t creature, int day, int ms, bool haveTime, Buff* out, int maxOut, bool debuff = false, float minRatio = 0.0f) {
    int arr = 0, count = 0;
    if (!effectListNonEmpty(creature, &arr, &count)) return 0;
    int seen[MaxEffects]; int nSeen = 0; int n = 0;
    for (int i = 0; i < count && n < maxOut; ++i) {
        int eff, spell;
        if (!rd(arr, i * 4, &eff) || !rd(eff, Effect_SpellId, &spell)) continue;
        bool item = false;
        const char* resref = iconForSpell(spell, debuff, &item);
        if (!resref) continue;
        bool dup = false;
        for (int k = 0; k < nSeen; ++k) if (seen[k] == spell) { dup = true; break; }
        if (dup) continue;
        seen[nSeen++] = spell;
        out[n++] = { resref, effectRatio(eff, day, ms, haveTime, minRatio), item };
    }
    return n;
}

} // namespace k2fx
