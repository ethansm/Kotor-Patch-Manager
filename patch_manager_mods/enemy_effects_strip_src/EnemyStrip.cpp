// EnemyStrip.cpp -- Enemy Effects Strip 0.1.0-probe (Steam Aspyr build 6A522E71...). PROBE ONLY: no drawing, no engine writes, no hooks.
// Hook-free consumer of hud-host (k2.hud@1). On every K2HUD_TARGET_MENU_POST it compares the target with the last one and, only on
// a change, logs one block: the target menu panel fields (menu +0x1BAC..+0x1AA4, raw int and float, units unproven), the target's
// effect list with spells.2da icon matches and remaining-duration ratio (capped at 24 effects), and the leader's skill ranks with
// the Awareness guess (stats +0x174 idx 3, UNPROVEN). Log: enemystrip_log.txt (fallback) and the game's mod log.
// [Strip] Verbose (enemy_strip.ini, default 1): 0 keeps only the one-line target change.
#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "KpLog.h"
#include "SafeRead.h"
#include "HudHost.h"
#include "K2Addr.h"
#include "Effects.h"
#include "EffectIcons.h"

KP_LOG_PATCH("enemy-effects-strip");

namespace {

// saferead site ids: one per group of reads, so the failure ring says which field group was unreadable.
constexpr unsigned SiteBase = 0x7A00;
constexpr unsigned S_Flags  = SiteBase + 0x00;   // menu +0x1BAC (uint8)
constexpr unsigned S_Panel  = SiteBase + 0x01;   // menu +0x1648..+0x1654 origin + size
constexpr unsigned S_Clamp  = SiteBase + 0x02;   // menu +0x1658..+0x1664 clamp bounds
constexpr unsigned S_Name   = SiteBase + 0x03;   // menu +0x166C..+0x1678 name label rect
constexpr unsigned S_Bar    = SiteBase + 0x04;   // menu +0x1A44..+0x1A50 health bar rect
constexpr unsigned S_BarHp  = SiteBase + 0x05;   // menu +0x1AA0 bar max, +0x1AA4 bar cur
constexpr unsigned S_EffArr = SiteBase + 0x06;   // target +0x148 effect array ptr, +0x14C count
constexpr unsigned S_EffPtr = SiteBase + 0x07;   // effect array slot i (uint32 pointer)
constexpr unsigned S_Eff    = SiteBase + 0x08;   // effect type / duration / creator / spell id
constexpr unsigned S_EffTm  = SiteBase + 0x09;   // effect expiry day / ms of day
constexpr unsigned S_LdrSt  = SiteBase + 0x0A;   // leader +0x1198 stats ptr
constexpr unsigned S_Skill  = SiteBase + 0x0B;   // stats +0x174..+0x17B skill ranks (int8)

constexpr int MaxEffectsLogged = 24;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chTarget, "target");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("enemystrip_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

struct Config {
    bool verbose = true;   // [Strip] Verbose: 1 = full block per target change (probe default)
} g_cfg;

const char* const IniPath = ".\\enemy_strip.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

void loadIni() {
    g_cfg.verbose = GetPrivateProfileIntA("Strip", "Verbose", 1, IniPath) != 0;
}

k2hud::Client g_hud;

// One engine field read as int32 and as float (units unproven). ok = the read succeeded.
struct Fld {
    int32_t i = 0;
    float f = 0.0f;
    bool ok = false;
};

Fld rdFld(unsigned site, uintptr_t base, int off) {
    Fld r;
    uint32_t raw = 0;
    if (saferead::readAt(site, base, off, &raw)) {
        r.ok = true;
        r.i = static_cast<int32_t>(raw);
        memcpy(&r.f, &raw, sizeof(r.f));
    }
    return r;
}

void fmtFld(char* out, size_t cap, const Fld& f) {
    if (f.ok) snprintf(out, cap, "%d/%.2f", f.i, static_cast<double>(f.f));
    else snprintf(out, cap, "?");
}

// One log line: a head, then " name=int/float" per field ("?" for an unreadable one). KpLog caps a line at 512 bytes.
struct Line {
    char b[480] = {0};
    explicit Line(const char* head) { snprintf(b, sizeof(b), "%s", head); }
    void put(const char* name, const Fld& f) {
        char v[40];
        fmtFld(v, sizeof(v), f);
        const size_t used = strlen(b);
        snprintf(b + used, sizeof(b) - used, " %s=%s", name, v);
    }
};

// Target menu panel state (menu = ctx->menu = hud + 0xCC). Read only; the origin is final at POST time.
void logMenu(uintptr_t menu) {
    uint8_t flags = 0;
    char fs[24];
    if (saferead::readAt(S_Flags, menu, 0x1BAC, &flags)) snprintf(fs, sizeof(fs), "%02x bit0=%d", (unsigned)flags, flags & 1);
    else snprintf(fs, sizeof(fs), "?");
    KP_LOGV(chTarget, KPLOG_INFO, "menu +1BAC flags=%s", fs);

    Line panel("menu panel");
    panel.put("+1648", rdFld(S_Panel, menu, 0x1648));
    panel.put("+164C", rdFld(S_Panel, menu, 0x164C));
    panel.put("+1650", rdFld(S_Panel, menu, 0x1650));
    panel.put("+1654", rdFld(S_Panel, menu, 0x1654));
    KP_LOGV(chTarget, KPLOG_INFO, "%s", panel.b);

    Line clamp("menu clamp");
    clamp.put("+1658", rdFld(S_Clamp, menu, 0x1658));
    clamp.put("+165C", rdFld(S_Clamp, menu, 0x165C));
    clamp.put("+1660", rdFld(S_Clamp, menu, 0x1660));
    clamp.put("+1664", rdFld(S_Clamp, menu, 0x1664));
    KP_LOGV(chTarget, KPLOG_INFO, "%s", clamp.b);

    Line name("menu name rect");
    name.put("+166C", rdFld(S_Name, menu, 0x166C));
    name.put("+1670", rdFld(S_Name, menu, 0x1670));
    name.put("+1674", rdFld(S_Name, menu, 0x1674));
    name.put("+1678", rdFld(S_Name, menu, 0x1678));
    KP_LOGV(chTarget, KPLOG_INFO, "%s", name.b);

    Line bar("menu hp bar rect");
    bar.put("+1A44", rdFld(S_Bar, menu, 0x1A44));
    bar.put("+1A48", rdFld(S_Bar, menu, 0x1A48));
    bar.put("+1A4C", rdFld(S_Bar, menu, 0x1A4C));
    bar.put("+1A50", rdFld(S_Bar, menu, 0x1A50));
    KP_LOGV(chTarget, KPLOG_INFO, "%s", bar.b);

    Line hp("menu hp max/cur");
    hp.put("+1AA0", rdFld(S_BarHp, menu, 0x1AA0));
    hp.put("+1AA4", rdFld(S_BarHp, menu, 0x1AA4));
    KP_LOGV(chTarget, KPLOG_INFO, "%s", hp.b);
}

// The target's effect list, capped at MaxEffectsLogged. Reads its own offsets (k2addr table) through the site ids above.
// haveTime false (no world timer) leaves ratio "-".
void logEffects(uintptr_t target, uintptr_t leader, bool haveTime, int day, int ms) {
    const int oArr = k2addr::off(&K2AddrTable::Creature_EffectArray);
    const int oCnt = k2addr::off(&K2AddrTable::Creature_EffectCount);
    const int oType = k2addr::off(&K2AddrTable::Effect_Type);
    const int oDur = k2addr::off(&K2AddrTable::Effect_Duration);
    const int oExpDay = k2addr::off(&K2AddrTable::Effect_ExpiryDay);
    const int oExpMs = k2addr::off(&K2AddrTable::Effect_ExpiryTime);
    const int oCreator = k2addr::off(&K2AddrTable::Effect_Creator);
    const int oSpell = k2addr::off(&K2AddrTable::Effect_SpellId);
    if (!oArr || !oCnt || !oType || !oDur || !oExpDay || !oExpMs || !oCreator || !oSpell) {
        KP_LOGV(chTarget, KPLOG_WARN, "effects: offsets unknown for this exe, not read");
        return;
    }
    uint32_t arr = 0;
    int32_t count = 0;
    const bool readOk = saferead::readAt(S_EffArr, target, oArr, &arr) && saferead::readAt(S_EffArr, target, oCnt, &count);
    if (!readOk || !arr || count <= 0 || count > k2fx::MaxEffects) {
        KP_LOGV(chTarget, KPLOG_INFO, "effects: none (read=%d arr=%08x count=%d)", readOk ? 1 : 0, (unsigned)arr, (int)count);
        return;
    }
    const int logged = count < MaxEffectsLogged ? static_cast<int>(count) : MaxEffectsLogged;
    KP_LOGV(chTarget, KPLOG_INFO, "effects: count=%d logging=%d", (int)count, logged);
    for (int i = 0; i < logged; ++i) {
        uint32_t e = 0;
        if (!saferead::readAt(S_EffPtr, arr, i * 4, &e) || !e) {
            KP_LOGV(chTarget, KPLOG_INFO, "eff[%d] slot unreadable (ptr=%08x)", i, (unsigned)e);
            continue;
        }
        uint16_t type = 0;
        const bool typeOk = saferead::readAt(S_Eff, e, oType, &type);
        const Fld dur = rdFld(S_Eff, e, oDur);
        uint32_t creator = 0;
        const bool creatorOk = saferead::readAt(S_Eff, e, oCreator, &creator);
        int32_t spell = -1;
        const bool spellOk = saferead::readAt(S_Eff, e, oSpell, &spell);
        const Fld expDay = rdFld(S_EffTm, e, oExpDay);
        const Fld expMs = rdFld(S_EffTm, e, oExpMs);

        char ratio[16] = "-";
        if (haveTime && dur.ok && dur.f > 0.0f && expDay.ok && expMs.ok)
            snprintf(ratio, sizeof(ratio), "%.3f", static_cast<double>(k2fx::remainingRatio(expDay.i, expMs.i, dur.f, day, ms)));

        bool itemBuff = false, itemDebuff = false;
        const char* buff = spellOk ? k2fx::iconForSpell(spell, false, &itemBuff) : nullptr;
        const char* debuff = spellOk ? k2fx::iconForSpell(spell, true, &itemDebuff) : nullptr;

        char durS[40];
        fmtFld(durS, sizeof(durS), dur);
        const int creatorIsLeader = (creatorOk && leader && creator == static_cast<uint32_t>(leader)) ? 1 : 0;   // UNPROVEN: creator may be an object id
        KP_LOGV(chTarget, KPLOG_INFO, "eff[%d] type=%d spell=%d dur=%s creator=%08x creatorIsLeader=%d ratio=%s buff=%s debuff=%s",
                i, typeOk ? (int)type : -1, spellOk ? (int)spell : -1, durS, creatorOk ? (unsigned)creator : 0u, creatorIsLeader,
                ratio, buff ? buff : "-", debuff ? debuff : "-");
    }
    if (count > MaxEffectsLogged) KP_LOGV(chTarget, KPLOG_INFO, "effects: %d more not logged", (int)count - MaxEffectsLogged);
}

// Leader skill ranks: 8 int8 at stats+0x174 (skills.2da row order). idx 3 = Awareness in that order: UNPROVEN, log only.
void logLeaderSkills(uintptr_t leader) {
    const int oStats = k2addr::off(&K2AddrTable::Creature_Stats);
    uint32_t stats = 0;
    const bool statsOk = leader && oStats && saferead::readAt(S_LdrSt, leader, oStats, &stats) && stats;
    int8_t r[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    bool skillsOk = statsOk;
    for (int i = 0; skillsOk && i < 8; ++i) skillsOk = saferead::readAt(S_Skill, stats, 0x174 + i, &r[i]);
    if (skillsOk) {
        KP_LOGV(chTarget, KPLOG_INFO, "leader skills raw=[%d,%d,%d,%d,%d,%d,%d,%d] awareness(idx3)=%d (UNPROVEN index)",
                (int)r[0], (int)r[1], (int)r[2], (int)r[3], (int)r[4], (int)r[5], (int)r[6], (int)r[7], (int)r[3]);
    } else {
        KP_LOGV(chTarget, KPLOG_INFO, "leader skills: awareness=? (leader=%08x stats=%08x)", (unsigned)leader, (unsigned)stats);
    }
}

// K2HUD_TARGET_MENU_POST: one block per target change, never per frame. Target 0 = no target.
void KPATCH_CALL onTargetMenuPost(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, target) + sizeof(ctx->target)) return;
    saferead::beginScope();
    static uintptr_t lastTarget = 0;
    const uintptr_t target = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->target));
    if (target == lastTarget) return;
    lastTarget = target;
    if (!target) {
        KP_LOGV(chTarget, KPLOG_INFO, "target cleared");
        return;
    }
    const uintptr_t leader = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->leader));
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    KP_LOGV(chTarget, KPLOG_INFO, "target=%08x leader=%08x menu=%08x targetType=%d",
            (unsigned)target, (unsigned)leader, (unsigned)menu, (int)ctx->targetType);
    if (!g_cfg.verbose) return;

    int day = 0, ms = 0;
    const bool haveTime = k2fx::currentTime(&day, &ms);
    if (haveTime) KP_LOGV(chTarget, KPLOG_INFO, "world time day=%d ms=%d", day, ms);
    else KP_LOGV(chTarget, KPLOG_INFO, "world time: unknown (ratio -)");
    logMenu(menu);
    logEffects(target, leader, haveTime, day, ms);
    logLeaderSkills(leader);
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_hud.init(api, "enemy-effects-strip");
    loadIni();
    KP_LOGV(chMain, KPLOG_INFO, "enemy-effects-strip 0.1.0-probe loaded (Verbose=%d)", g_cfg.verbose ? 1 : 0);
    g_hud.add(K2HUD_TARGET_MENU_POST, 100, onTargetMenuPost);
    g_hud.connect();
}

} // extern "C"
