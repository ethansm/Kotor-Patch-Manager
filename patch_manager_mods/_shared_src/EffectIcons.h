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

// Opt-in icon for the poison row; nullptr (default) = untracked, the engine draws its debuff arrow. A mod sets it only once the texture exists
// (Buff HUD: embedded bh_poison, PoisonIconTex.inc), so a consumer without that texture never gets the resref.
inline const char* g_trapPoisonIcon = nullptr;

// Harmful effecticon.2da row (good == 0) carried by a type-0x43 effect -> its icon, else nullptr. Row 0 is the "none" row.
inline const char* iconForEffectIconEffect(int eff, bool* item) {
    *item = false;
    unsigned short type = 0; int n = 0, ip = 0, row = 0;
    if (!rd(eff, Effect_Type, &type) || type != EffType_Icon) return nullptr;
    if (!rd(eff, Effect_IntCount, &n) || n <= 0 || !rd(eff, Effect_Ints, &ip) || !ip || !rd(ip, 0, &row)) return nullptr;
    if (row == EffIconRow_TrapPoison && g_trapPoisonIcon) return g_trapPoisonIcon;
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
        out[n++] = { resref, effectRatio(eff, day, ms, haveTime, minRatio), item };
    }
    return n;
}

} // namespace k2fx
