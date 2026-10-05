// HealItems.h - pure healing-item choice + auto-use trigger for the companion self-heal mod (doc 27 s3/s3b, evidence R3_item_choice).
// Header-only, no Windows/engine includes. Heal amounts mirror k_sup_healing.nss: heal = base + skillMul*skill + abilityMod.
#pragma once
#include <cstdint>
#include <string>

namespace heal {

enum class Family { None, Organic, Droid };
struct Formula { int row; Family fam; int base, skillMul; };

// spells.2da rows. Organic: skill=TreatInjury, ability=WIS. Droid: skill=Repair, ability=INT.
// Row 67 (antidote) and 129 (squad recovery stim) have no entry on purpose: they never heal the user.
inline const Formula* formulaFor(int row) {
    static const Formula tab[] = {
        {64, Family::Organic, 10, 1}, {65, Family::Organic, 20, 2}, {66, Family::Organic, 30, 3},
        {84, Family::Droid, 15, 1}, {127, Family::Droid, 25, 2}, {128, Family::Droid, 35, 3},
    };
    for (const Formula& f : tab) if (f.row == row) return &f;
    return nullptr;
}

inline int abilityMod(int score) { return (score >= 10) ? (score - 10) / 2 : -((11 - score) / 2); }   // floor((s-10)/2)

// Estimated HP restored by one use, from the USER's own stats. -1 if the row has no heal formula.
inline int healEstimate(int row, int treatInjury, int repair, int wis, int intel) {
    const Formula* f = formulaFor(row);
    if (!f) return -1;
    if (f->fam == Family::Organic) return f->base + f->skillMul * treatInjury + abilityMod(wis);
    return f->base + f->skillMul * repair + abilityMod(intel);
}

struct HealCand { int key; int row; int heal; int count; int cost; bool usable; };

// Pick the item to use for `missingHp`. Returns an index into c[0..n) or -1.
// minCount is the caller's job: 1 for a manual click, keep+1 for auto-use (so `keep` items stay in the stack).
// Rules: drop !usable / count<minCount / heal<=0 / no formula; smallest heal that covers missing (heal>=missing),
// else largest heal; ties -> lower cost, larger count, lower key.
inline int pickHealItem(int missingHp, const HealCand* c, int n, int minCount) {
    if (missingHp <= 0 || !c) return -1;
    int best = -1;
    bool bestCovers = false;
    for (int i = 0; i < n; ++i) {
        const HealCand& x = c[i];
        if (!x.usable || x.count < minCount || x.heal <= 0 || !formulaFor(x.row)) continue;
        bool covers = x.heal >= missingHp;
        if (best < 0) { best = i; bestCovers = covers; continue; }
        const HealCand& b = c[best];
        bool better;
        if (covers != bestCovers) better = covers;
        else if (x.heal != b.heal) better = covers ? (x.heal < b.heal) : (x.heal > b.heal);
        else if (x.cost != b.cost) better = x.cost < b.cost;
        else if (x.count != b.count) better = x.count > b.count;
        else better = x.key < b.key;
        if (better) { best = i; bestCovers = covers; }
    }
    return best;
}

struct AutoCfg { bool on = false; int hpPct = 35; int keep = 2; };
struct AutoState { uint32_t lastUseMs = 0; bool pending = false; };
constexpr uint32_t AutoCooldownMs = 6000, AutoEvalMs = 250;

// True when a companion at cur/max HP should auto-use a healing item now (strictly below hpPct percent).
inline bool shouldAutoUse(const AutoCfg& cfg, int cur, int max, bool alive, bool commandable, bool gameplay,
                          const AutoState& st, uint32_t nowMs) {
    if (!cfg.on || !alive || !commandable || !gameplay || max <= 0) return false;
    if (!((int64_t)cur * 100 < (int64_t)cfg.hpPct * max)) return false;
    if (st.pending) return false;
    return st.lastUseMs == 0 || (uint32_t)(nowMs - st.lastUseMs) >= AutoCooldownMs;
}

// shared; hotbar uses it via `using` (CS7, HotbarLogic.h isHealingRow). usertype 4, range "T", category 0x1408 medpac / 0x1404 repair / 0x1048 cure.
inline bool isHealingRow(int usertype, const std::string& range, int category) {
    return usertype == 4 && range == "T" && (category == 0x1408 || category == 0x1404 || category == 0x1048);
}

} // namespace heal
