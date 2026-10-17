// EnemyStrip.cpp -- Enemy Effects Strip 0.2.0-probe (Steam Aspyr build 6A522E71...). PROBE ONLY: no drawing, no engine writes, no hooks.
// Hook-free consumer of hud-host (k2.hud@1). ctx->target and ctx->leader are CLIENT creatures; the effect list, the icon cache and
// the stats live on the SERVER creature, so every read goes through party::serverCreature (research 39 wave 10-17, R3).
// On a target change it logs one block: the target menu panel fields (menu +0x1BAC..+0x1AA4, raw int and float, units unproven),
// the server effect list (spells.2da icon matches, remaining-duration ratio, SetState id), the icon cache and the leader's skill
// ranks (stats +0x174 is a POINTER to the int8 base array; idx 3 = Awareness, UNPROVEN). While a target is set, a 250 ms throttled
// signature of the server effect list logs 'effects CHANGED' followed by a dump. Log: enemystrip_log.txt (fallback) and the game's mod log.
// [Strip] Verbose (enemy_strip.ini, default 1): 0 keeps only the one-line target change and the 'effects CHANGED' headers.
#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "KpLog.h"
#include "SafeRead.h"
#include "HudHost.h"
#include "K2Addr.h"
#include "Party.h"
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
constexpr unsigned S_EffArr = SiteBase + 0x06;   // server creature effect array ptr, count
constexpr unsigned S_EffPtr = SiteBase + 0x07;   // effect array slot i (uint32 pointer)
constexpr unsigned S_Eff    = SiteBase + 0x08;   // effect type / subtype / creator / spell id
constexpr unsigned S_EffTm  = SiteBase + 0x09;   // effect expiry day / ms of day
constexpr unsigned S_LdrSt  = SiteBase + 0x0A;   // server leader +0x1198 stats ptr
constexpr unsigned S_Skill  = SiteBase + 0x0B;   // base skill-rank array: 8 int8 (read at the pointer stats+0x174)
constexpr unsigned S_CliVt  = SiteBase + 0x0C;   // client creature +0 vtable (guard before party::serverCreature)
constexpr unsigned S_CliF8  = SiteBase + 0x0D;   // client creature +0xF8 cached server pointer (cross-check only)
constexpr unsigned S_SigArr = SiteBase + 0x0E;   // signature: server effect array ptr, count
constexpr unsigned S_SigEff = SiteBase + 0x0F;   // signature: effect slot pointer
constexpr unsigned S_SigFld = SiteBase + 0x10;   // signature: effect type, subtype, spell id
constexpr unsigned S_SigIc  = SiteBase + 0x11;   // signature: icon-cache count
constexpr unsigned S_IcCnt  = SiteBase + 0x12;   // icon cache: data ptr (+0xFFC) and count (+0x1000)
constexpr unsigned S_IcSlot = SiteBase + 0x13;   // icon cache: entry pointer slot
constexpr unsigned S_IcEnt  = SiteBase + 0x14;   // icon cache: entry fields (0x24-byte struct)
constexpr unsigned S_StatPt = SiteBase + 0x15;   // stats +0x174: POINTER to the int8 base skill-rank array
constexpr unsigned S_IntCnt = SiteBase + 0x16;   // SetState effect: ints count (+0x30) and ints pointer (+0x34)
constexpr unsigned S_IntVal = SiteBase + 0x17;   // SetState effect: ints[0] (state id)

constexpr int MaxEffectsLogged = 24;      // effect lines per dump
constexpr int SigEffects = 64;            // effects hashed into the change signature
constexpr int MaxIconCount = 64;          // sanity limit on the icon-cache count
constexpr int MaxIconsLogged = 16;        // icon-cache entries per dump
constexpr int MaxDumpBlocks = 300;        // effect-dump blocks per session (target change or CHANGED); then 'log cap reached' once
constexpr DWORD SigIntervalMs = 250;      // signature check period while a target is set (never per frame)
constexpr uintptr_t InvalidObj = 0xFFFFFFFFu;
constexpr int EffType_SetState = 0x08;    // SetState: ints[0] is the state id (Horrified = 8, Stasis = 5, ...)
constexpr int Cli_ServerCache = 0xF8;     // client creature: cached server pointer
constexpr int Stats_SkillPtr = 0x174;     // server stats: POINTER to the int8 base skill-rank array
constexpr int Icon_Spell = 0x00, Icon_Buff = 0x14, Icon_Prio = 0x18, Icon_Valid = 0x1C, Icon_StrRef = 0x20;   // icon entry fields

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

// Session dump budget: target-change blocks and CHANGED blocks share it, so a bad signature cannot spam the log.
int g_blocks = 0;
bool g_capLogged = false;
bool takeBlock() {
    if (g_blocks < MaxDumpBlocks) { ++g_blocks; return true; }
    if (!g_capLogged) {
        g_capLogged = true;
        KP_LOGV(chTarget, KPLOG_WARN, "log cap reached (%d effect-dump blocks): no further effect dumps this session", MaxDumpBlocks);
    }
    return false;
}

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

// World time for the ratio column. ok false = no world timer (main menu, load): ratio "-".
struct WorldNow {
    bool ok = false;
    int day = 0, ms = 0;
};
WorldNow worldNow() {
    WorldNow w;
    w.ok = k2fx::currentTime(&w.day, &w.ms);
    return w;
}

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

// Client creature -> server creature, or 0. The client pointer must be non-zero, not the invalid id, and its vtable readable
// before the engine accessor is called (MemberServerCreature thiscall, through Party.h).
uintptr_t resolveServer(uintptr_t client) {
    if (!client || client == InvalidObj) return 0;
    uint32_t vt = 0;
    if (!saferead::readAt(S_CliVt, client, 0, &vt) || !vt) return 0;
    return party::serverCreature(client);
}

// SetState (type 0x08): ints[0] is the state id. Writes "?" when the ints array is unreadable.
void stateOf(uintptr_t eff, int oIntCount, int oInts, char* out, size_t cap) {
    int32_t nInts = 0;
    uint32_t pInts = 0;
    int32_t st = 0;
    if (saferead::readAt(S_IntCnt, eff, oIntCount, &nInts) && nInts >= 1 && nInts <= 64 &&
        saferead::readAt(S_IntCnt, eff, oInts, &pInts) && pInts && saferead::readAt(S_IntVal, pInts, 0, &st))
        snprintf(out, cap, "%d", static_cast<int>(st));
    else
        snprintf(out, cap, "?");
}

// The server effect list, capped at MaxEffectsLogged. Reads its own offsets (k2addr table) through the site ids above.
void logEffects(uintptr_t srvTarget, uintptr_t srvLeader, const WorldNow& wt) {
    const int oArr = k2addr::off(&K2AddrTable::Creature_EffectArray);
    const int oCnt = k2addr::off(&K2AddrTable::Creature_EffectCount);
    const int oType = k2addr::off(&K2AddrTable::Effect_Type);
    const int oSub = k2addr::off(&K2AddrTable::Effect_Subtype);
    const int oDur = k2addr::off(&K2AddrTable::Effect_Duration);
    const int oExpDay = k2addr::off(&K2AddrTable::Effect_ExpiryDay);
    const int oExpMs = k2addr::off(&K2AddrTable::Effect_ExpiryTime);
    const int oCreator = k2addr::off(&K2AddrTable::Effect_Creator);
    const int oSpell = k2addr::off(&K2AddrTable::Effect_SpellId);
    const int oIntCount = k2addr::off(&K2AddrTable::Effect_IntCount);
    const int oInts = k2addr::off(&K2AddrTable::Effect_Ints);
    if (!oArr || !oCnt || !oType || !oSub || !oDur || !oExpDay || !oExpMs || !oCreator || !oSpell || !oIntCount || !oInts) {
        KP_LOGV(chTarget, KPLOG_WARN, "effects: offsets unknown for this exe, not read");
        return;
    }
    uint32_t arr = 0;
    int32_t count = 0;
    const bool readOk = saferead::readAt(S_EffArr, srvTarget, oArr, &arr) && saferead::readAt(S_EffArr, srvTarget, oCnt, &count);
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
        uint16_t type = 0, sub = 0;
        const bool typeOk = saferead::readAt(S_Eff, e, oType, &type);
        const bool subOk = saferead::readAt(S_Eff, e, oSub, &sub);
        const Fld dur = rdFld(S_Eff, e, oDur);
        uint32_t creator = 0;
        const bool creatorOk = saferead::readAt(S_Eff, e, oCreator, &creator);
        int32_t spell = -1;
        const bool spellOk = saferead::readAt(S_Eff, e, oSpell, &spell);
        const Fld expDay = rdFld(S_EffTm, e, oExpDay);
        const Fld expMs = rdFld(S_EffTm, e, oExpMs);

        char ratio[16] = "-";
        if (wt.ok && dur.ok && dur.f > 0.0f && expDay.ok && expMs.ok)
            snprintf(ratio, sizeof(ratio), "%.3f", static_cast<double>(k2fx::remainingRatio(expDay.i, expMs.i, dur.f, wt.day, wt.ms)));

        bool itemBuff = false, itemDebuff = false;
        const char* buff = spellOk ? k2fx::iconForSpell(spell, false, &itemBuff) : nullptr;
        const char* debuff = spellOk ? k2fx::iconForSpell(spell, true, &itemDebuff) : nullptr;

        char stateS[16] = "-";
        if (typeOk && type == EffType_SetState) stateOf(e, oIntCount, oInts, stateS, sizeof(stateS));

        char durS[40];
        fmtFld(durS, sizeof(durS), dur);
        const int creatorIsLeader = (creatorOk && srvLeader && creator == static_cast<uint32_t>(srvLeader)) ? 1 : 0;   // UNPROVEN: creator may be an object id
        KP_LOGV(chTarget, KPLOG_INFO, "eff[%d] type=%d sub=%d spell=%d dur=%s creator=%08x creatorIsLeader=%d ratio=%s buff=%s debuff=%s state=%s",
                i, typeOk ? (int)type : -1, subOk ? (int)sub : -1, spellOk ? (int)spell : -1, durS, creatorOk ? (unsigned)creator : 0u,
                creatorIsLeader, ratio, buff ? buff : "-", debuff ? debuff : "-", stateS);
    }
    if (count > MaxEffectsLogged) KP_LOGV(chTarget, KPLOG_INFO, "effects: %d more not logged", (int)count - MaxEffectsLogged);
}

// Icon cache of the server creature: count at +0x1000, data pointer at +0xFFC (array of pointers to 0x24-byte entries).
void logIconCache(uintptr_t srvTarget) {
    const int oData = k2addr::off(&K2AddrTable::Creature_IconCacheData);
    const int oCnt = k2addr::off(&K2AddrTable::Creature_IconCacheCount);
    if (!oData || !oCnt) {
        KP_LOGV(chTarget, KPLOG_WARN, "iconcache: offsets unknown for this exe, not read");
        return;
    }
    uint32_t data = 0;
    int32_t cnt = -1;
    const bool readOk = saferead::readAt(S_IcCnt, srvTarget, oData, &data) && saferead::readAt(S_IcCnt, srvTarget, oCnt, &cnt);
    if (!readOk || cnt < 0 || cnt > MaxIconCount) {
        KP_LOGV(chTarget, KPLOG_INFO, "iconcache count=%d not read (read=%d data=%08x)", (int)cnt, readOk ? 1 : 0, (unsigned)data);
        return;
    }
    KP_LOGV(chTarget, KPLOG_INFO, "iconcache count=%d", (int)cnt);
    if (cnt == 0) return;
    if (!data) {
        KP_LOGV(chTarget, KPLOG_WARN, "iconcache: data null with count=%d", (int)cnt);
        return;
    }
    const int logged = cnt < MaxIconsLogged ? static_cast<int>(cnt) : MaxIconsLogged;
    for (int i = 0; i < logged; ++i) {
        uint32_t ep = 0;
        if (!saferead::readAt(S_IcSlot, data, i * 4, &ep) || !ep) {
            KP_LOGV(chTarget, KPLOG_INFO, "ic[%d] slot unreadable (ptr=%08x)", i, (unsigned)ep);
            continue;
        }
        int16_t spell = -1;
        uint32_t buff = 0, valid = 0, strRef = 0;
        uint16_t prio = 0;
        const bool eOk = saferead::readAt(S_IcEnt, ep, Icon_Spell, &spell) && saferead::readAt(S_IcEnt, ep, Icon_Buff, &buff) &&
                         saferead::readAt(S_IcEnt, ep, Icon_Prio, &prio) && saferead::readAt(S_IcEnt, ep, Icon_Valid, &valid) &&
                         saferead::readAt(S_IcEnt, ep, Icon_StrRef, &strRef);
        if (!eOk) {
            KP_LOGV(chTarget, KPLOG_INFO, "ic[%d] entry unreadable (ptr=%08x)", i, (unsigned)ep);
            continue;
        }
        KP_LOGV(chTarget, KPLOG_INFO, "ic[%d] spell=%d buff=%u prio=%u valid=%u strref=%u", i, (int)spell, (unsigned)buff,
                (unsigned)prio, (unsigned)valid, (unsigned)strRef);
    }
    if (cnt > MaxIconsLogged) KP_LOGV(chTarget, KPLOG_INFO, "iconcache: %d more not logged", (int)cnt - MaxIconsLogged);
}

// Change signature of the server effect list: count, icon-cache count, and a rolling hash over up to SigEffects effects of
// (type, subtype, spell id). ok false = the list is unreadable or out of range (the fields keep their defaults).
struct EffSig {
    bool ok = false;
    uint32_t hash = 0;
    int count = -1;
    int icons = -1;
};

inline uint32_t mixIn(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u + 0x9E3779B9u; }   // FNV-style rolling step

bool sameSig(const EffSig& a, const EffSig& b) {
    return a.ok == b.ok && a.hash == b.hash && a.count == b.count && a.icons == b.icons;
}

EffSig effectSignature(uintptr_t srvTarget) {
    EffSig s;
    const int oArr = k2addr::off(&K2AddrTable::Creature_EffectArray);
    const int oCnt = k2addr::off(&K2AddrTable::Creature_EffectCount);
    const int oType = k2addr::off(&K2AddrTable::Effect_Type);
    const int oSub = k2addr::off(&K2AddrTable::Effect_Subtype);
    const int oSpell = k2addr::off(&K2AddrTable::Effect_SpellId);
    const int oIcCnt = k2addr::off(&K2AddrTable::Creature_IconCacheCount);
    if (!srvTarget || !oArr || !oCnt || !oType || !oSub || !oSpell || !oIcCnt) return s;
    uint32_t arr = 0;
    int32_t count = 0;
    if (!saferead::readAt(S_SigArr, srvTarget, oArr, &arr) || !saferead::readAt(S_SigArr, srvTarget, oCnt, &count)) return s;
    if (count < 0 || count > k2fx::MaxEffects) return s;
    int32_t icons = -1;
    if (!saferead::readAt(S_SigIc, srvTarget, oIcCnt, &icons)) icons = -1;
    uint32_t h = 2166136261u;
    h = mixIn(h, static_cast<uint32_t>(count));
    const int n = count < SigEffects ? static_cast<int>(count) : SigEffects;
    for (int i = 0; i < n; ++i) {
        uint32_t e = 0;
        if (!saferead::readAt(S_SigEff, arr, i * 4, &e) || !e) {
            h = mixIn(h, 0xBAD00000u | static_cast<uint32_t>(i));
            continue;
        }
        uint16_t type = 0, sub = 0;
        int32_t spell = -1;
        saferead::readAt(S_SigFld, e, oType, &type);   // an unreadable field hashes as its default
        saferead::readAt(S_SigFld, e, oSub, &sub);
        saferead::readAt(S_SigFld, e, oSpell, &spell);
        h = mixIn(h, static_cast<uint32_t>(type));
        h = mixIn(h, static_cast<uint32_t>(sub));
        h = mixIn(h, static_cast<uint32_t>(spell));
    }
    h = mixIn(h, static_cast<uint32_t>(icons));
    s.ok = true;
    s.hash = h;
    s.count = count;
    s.icons = icons;
    return s;
}

// Leader skill ranks: 8 int8 at the base array that the server stats pointer (stats +0x174) points to. idx 3 = Awareness in
// skills.2da order: UNPROVEN, log only.
void logLeaderSkills(uintptr_t srvLeader) {
    const int oStats = k2addr::off(&K2AddrTable::Creature_Stats);
    uint32_t stats = 0, base = 0;
    const bool statsOk = srvLeader && oStats && saferead::readAt(S_LdrSt, srvLeader, oStats, &stats) && stats;
    const bool baseOk = statsOk && saferead::readAt(S_StatPt, stats, Stats_SkillPtr, &base) && base;
    int8_t r[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    bool skillsOk = baseOk;
    for (int i = 0; skillsOk && i < 8; ++i) skillsOk = saferead::readAt(S_Skill, base, i, &r[i]);
    if (skillsOk) {
        KP_LOGV(chTarget, KPLOG_INFO, "leader skills base=%08x raw=[%d,%d,%d,%d,%d,%d,%d,%d] awareness(idx3)=%d (UNPROVEN index)",
                (unsigned)base, (int)r[0], (int)r[1], (int)r[2], (int)r[3], (int)r[4], (int)r[5], (int)r[6], (int)r[7], (int)r[3]);
    } else {
        KP_LOGV(chTarget, KPLOG_INFO, "leader skills: awareness=? (srvLeader=%08x stats=%08x base=%08x)", (unsigned)srvLeader,
                (unsigned)stats, (unsigned)base);
    }
}

// K2HUD_TARGET_MENU_POST: never per frame. A target change logs one block; while a target is set the server effect list is
// re-signed every SigIntervalMs and logs 'effects CHANGED' + dump only when the signature differs. Target 0 / 0xFFFFFFFF = none.
void KPATCH_CALL onTargetMenuPost(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, target) + sizeof(ctx->target)) return;
    saferead::beginScope();
    static uintptr_t lastTarget = 0;
    static DWORD lastCheck = 0;
    static EffSig lastSig;
    const uintptr_t target = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->target));
    const uintptr_t leader = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->leader));
    const DWORD now = GetTickCount();

    if (target != lastTarget) {
        lastTarget = target;
        lastCheck = now;
        lastSig = EffSig();
        if (target == 0) {
            KP_LOGV(chTarget, KPLOG_INFO, "target cleared");
            return;
        }
        if (target == InvalidObj) {
            KP_LOGV(chTarget, KPLOG_WARN, "target invalid (ffffffff), skipped");
            return;
        }
        const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
        const uintptr_t srvTarget = resolveServer(target);
        const uintptr_t srvLeader = resolveServer(leader);
        if (srvTarget) lastSig = effectSignature(srvTarget);   // baseline: the next CHANGED is relative to this list

        uint32_t f8 = 0;
        const bool f8Ok = saferead::readAt(S_CliF8, target, Cli_ServerCache, &f8);
        char f8s[16] = "?";
        int f8Match = -1;   // -1 unreadable, 0 differs from the call result, 1 same
        if (f8Ok) {
            snprintf(f8s, sizeof(f8s), "%08x", (unsigned)f8);
            f8Match = (static_cast<uintptr_t>(f8) == srvTarget) ? 1 : 0;
        }
        KP_LOGV(chTarget, KPLOG_INFO, "target=%08x srv=%08x cli+F8=%s f8match=%d leader=%08x srvLeader=%08x menu=%08x targetType=%d",
                (unsigned)target, (unsigned)srvTarget, f8s, f8Match, (unsigned)leader, (unsigned)srvLeader, (unsigned)menu,
                (int)ctx->targetType);
        if (!g_cfg.verbose) return;

        const WorldNow wt = worldNow();
        if (wt.ok) KP_LOGV(chTarget, KPLOG_INFO, "world time day=%d ms=%d", wt.day, wt.ms);
        else KP_LOGV(chTarget, KPLOG_INFO, "world time: unknown (ratio -)");
        logMenu(menu);
        if (!srvTarget) {
            KP_LOGV(chTarget, KPLOG_WARN, "server creature unresolved (target=%08x): effects and icon cache skipped", (unsigned)target);
        } else if (takeBlock()) {
            logEffects(srvTarget, srvLeader, wt);
            logIconCache(srvTarget);
        }
        logLeaderSkills(srvLeader);
        return;
    }

    if (!target || target == InvalidObj || now - lastCheck < SigIntervalMs) return;
    lastCheck = now;
    const uintptr_t srvTarget = resolveServer(target);
    if (!srvTarget) return;
    const EffSig s = effectSignature(srvTarget);
    if (sameSig(s, lastSig)) return;
    lastSig = s;
    if (!takeBlock()) return;
    KP_LOGV(chTarget, KPLOG_INFO, "effects CHANGED sig=%08x count=%d", (unsigned)s.hash, s.count);
    if (!g_cfg.verbose) return;
    const uintptr_t srvLeader = resolveServer(leader);
    logEffects(srvTarget, srvLeader, worldNow());
    logIconCache(srvTarget);
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_hud.init(api, "enemy-effects-strip");
    loadIni();
    KP_LOGV(chMain, KPLOG_INFO, "enemy-effects-strip 0.2.0-probe loaded (Verbose=%d)", g_cfg.verbose ? 1 : 0);
    g_hud.add(K2HUD_TARGET_MENU_POST, 100, onTargetMenuPost);
    g_hud.connect();
}

} // extern "C"
