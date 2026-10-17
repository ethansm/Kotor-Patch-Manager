// EnemyStrip.cpp -- Enemy Effects Strip 0.3.0-dev (Steam Aspyr build 6A522E71...). Draws the target's status-effect icons as a bare row of
// GUI labels centred just above the target menu's name label. No engine hooks, no engine writes beyond the label setters in GuiKit.h.
// Hook-free consumer of hud-host (k2.hud@1): BUILD (prio 300) creates the label pool while the HUD layout is open, TARGET_MENU_POST (prio 100)
// reads the target's effects (every 250 ms or on a target change), picks the visible icons by the leader's Awareness tier (EnemyStripLogic.h)
// and places the labels (change-only setters). ctx->target and ctx->leader are CLIENT creatures; the effect list, the icon cache and
// the stats live on the SERVER creature, so every read goes through party::serverCreature (research 39 wave 10-17, R3).
// [Strip] Verbose (enemy_strip.ini, default 0): 1 additionally runs the 0.2.0 probe: on a target change one block with the target menu fields,
// the server effect list (spells.2da icon matches, remaining-duration ratio, SetState id), the icon cache and the leader's skill ranks;
// 'effects CHANGED' headers + dumps while a target is set. Log: enemystrip_log.txt (fallback) and the game's mod log.
#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "KpLog.h"
#include "SafeRead.h"
#include "HudHost.h"
#include "K2Addr.h"
#include "Party.h"
#include "Effects.h"
#include "EffectIcons.h"
#include "GuiKit.h"
#include "EnemyStripLogic.h"

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
constexpr unsigned S_Draw   = SiteBase + 0x18;   // draw path: menu geometry (origin + name rect)
constexpr unsigned S_Id     = SiteBase + 0x19;   // draw path: object id at creature +4 (party creator match)
constexpr unsigned S_Eff2   = SiteBase + 0x1A;   // draw path: effect ints[2] and ratio inputs

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
    bool enabled = true;       // [Strip] Enabled
    bool verbose = false;      // [Strip] Verbose: 1 = the 0.2.0 probe blocks as well
    int maxIcons = 5;          // 1..8
    int iconSize = 0;          // px, 0 = name label height
    int gap = 4;               // px between the row and the name label
    int spacing = 4;           // px between icons
    int tierObvious = 5, tierOwn = 12, tierFull = 20;   // Awareness thresholds
    float drainAlpha = 0.65f;  // effective alpha of the drain overlay
    bool backing = false;      // pill behind the row
    float backingAlpha = 0.55f;
    int backingPad = 4;
    int forceTier = -1;        // -1 = from Awareness, 0..3 forces
} g_cfg;

const char* const IniPath = ".\\enemy_strip.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

int iniInt(const char* key, int def, int lo, int hi) {
    return clampi(static_cast<int>(GetPrivateProfileIntA("Strip", key, def, IniPath)), lo, hi);
}
float iniFloat(const char* key, float def, float lo, float hi) {
    char dflt[32], buf[32];
    snprintf(dflt, sizeof(dflt), "%g", static_cast<double>(def));
    GetPrivateProfileStringA("Strip", key, dflt, buf, sizeof(buf), IniPath);
    char* end = nullptr;
    const double v = strtod(buf, &end);
    if (end == buf || v != v) return def;
    return clampf(static_cast<float>(v), lo, hi);
}

void loadIni() {
    g_cfg.enabled = iniInt("Enabled", 1, 0, 1) != 0;
    g_cfg.verbose = iniInt("Verbose", 0, 0, 1) != 0;
    g_cfg.maxIcons = iniInt("MaxIcons", 5, 1, esl::kMaxIcons);
    g_cfg.iconSize = iniInt("IconSize", 0, 0, 256);
    g_cfg.gap = iniInt("Gap", 4, 0, 200);
    g_cfg.spacing = iniInt("Spacing", 4, 0, 100);
    g_cfg.tierObvious = iniInt("TierObvious", 5, 0, 127);
    g_cfg.tierOwn = iniInt("TierOwn", 12, 0, 127);
    g_cfg.tierFull = iniInt("TierFull", 20, 0, 127);
    g_cfg.drainAlpha = iniFloat("DrainAlpha", 0.65f, 0.0f, 1.0f);
    g_cfg.backing = iniInt("Backing", 0, 0, 1) != 0;
    g_cfg.backingAlpha = iniFloat("BackingAlpha", 0.55f, 0.0f, 1.0f);
    g_cfg.backingPad = iniInt("BackingPad", 4, 0, 32);
    const int ft = iniInt("ForceTier", -1, -1, 3);
    g_cfg.forceTier = ft;
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

// Probe (Verbose=1 only): never per frame. A target change logs one block; while a target is set the server effect list is
// re-signed every SigIntervalMs and logs 'effects CHANGED' + dump only when the signature differs. Target 0 / 0xFFFFFFFF = none.
void probeTargetMenuPost(const K2HudContext* ctx) {
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
    const uintptr_t srvLeader = resolveServer(leader);
    logEffects(srvTarget, srvLeader, worldNow());
    logIconCache(srvTarget);
}


// ===================================================================== draw path (0.3.0)
// Target menu fields (menu = ctx->menu = hud + 0xCC), display px: origin x/y, name label rect (panel-local) x/y/w/h, flags bit0 = shown.
constexpr int Menu_Flags = 0x1BAC;
constexpr int Menu_OriginX = 0x1648, Menu_OriginY = 0x164C;
constexpr int Menu_NameX = 0x166C, Menu_NameY = 0x1670, Menu_NameW = 0x1674, Menu_NameH = 0x1678;
constexpr int Obj_Id = 0x04;                  // CGameObject object id (doc 39 R3: cid = *(T+4)); client ids carry bit 31
constexpr int MaxEff = 64;                    // effects read per refresh
constexpr int PoolSlots = esl::kMaxIcons;
constexpr DWORD RefreshMs = 250;              // effect list refresh period while a target is shown (also on a target change)
constexpr DWORD AwareMs = 1000;               // Awareness cache
constexpr int MaxStripLines = 300;            // 'strip:' lines per session
constexpr int ImgCacheN = 16;

const char* const TexOverlay = "uibit_fill_2bt";   // solid black, baked alpha 0.78
const char* const TexPill = "lbl_miscroll_f";
const char* const TexDisc = "buffhud_circle";      // a DISC: item-ability icons only, never a rectangle background
const float TintWhite[3] = { 1.0f, 1.0f, 1.0f };
const float TintBlack[3] = { 0.0f, 0.0f, 0.0f };
const float TintGrey[3] = { 0.5f, 0.5f, 0.5f };
const float TintTeal[3] = { 0.035f, 0.415f, 0.34f };

// One engine label with change-only setters (cache = last value written). Same pattern as combat_hotbar_src/CombatHotbar.cpp.
struct Wid {
    uintptr_t c = 0;
    int vis = -1;                 // -1 unknown, 0 hidden, 1 shown
    esl::RowRect rect = { 0, 0, 0, 0 };
    bool rectSet = false;
    void* fill = nullptr;
    bool fillSet = false;
    float alpha = -1.f;
    float tint[3] = { 0.f, 0.f, 0.f };
    bool tintSet = false;
};
struct SlotW { Wid disc, icon, over; };

void widHide(Wid& w) {
    if (!w.c || w.vis == 0) return;
    guikit::setVisible(w.c, false); guikit::setClickable(w.c, false);
    w.vis = 0;
}
void widShow(Wid& w) {
    if (!w.c || w.vis == 1) return;
    guikit::setVisible(w.c, true); guikit::setClickable(w.c, false);   // shown labels must stay click-through
    w.vis = 1;
}
void widRect(Wid& w, const esl::RowRect& r) {
    if (!w.c) return;
    if (!w.rectSet || w.rect.x != r.x || w.rect.y != r.y || w.rect.w != r.w || w.rect.h != r.h) { guikit::setRect(w.c, r.x, r.y, r.w, r.h); w.rect = r; w.rectSet = true; }
}
void widFill(Wid& w, void* img) {
    if (!w.c) return;
    if (!w.fillSet || w.fill != img) { guikit::setFill(w.c, img); w.fill = img; w.fillSet = true; }
}
void widAlpha(Wid& w, float a) {
    if (!w.c) return;
    if (w.alpha != a) { guikit::setAlpha(w.c, a); w.alpha = a; }
}
void widTint(Wid& w, const float* rgb) {
    if (!w.c) return;
    if (!w.tintSet || w.tint[0] != rgb[0] || w.tint[1] != rgb[1] || w.tint[2] != rgb[2]) {
        guikit::setBorderTint(w.c, rgb);
        w.tint[0] = rgb[0]; w.tint[1] = rgb[1]; w.tint[2] = rgb[2]; w.tintSet = true;
    }
}

Wid g_pill;
SlotW g_sw[PoolSlots];
int g_nSw = 0;              // complete slots built in the last BUILD
bool g_built = false;       // pool complete for g_panel
int32_t g_panel = 0;        // HUD panel the pool belongs to (identity guard against a stale HUD)
bool g_buildFailLogged = false;

// resref -> image, 16 entries (round-robin replacement). A null image is cached too and warns once per entry creation.
struct ImgEnt { char name[20]; void* img; };
ImgEnt g_imgs[ImgCacheN];
int g_nImgs = 0, g_imgNext = 0;
// Resrefs already warned about (survives FIFO eviction of g_imgs); once full, no further warns are added or issued.
constexpr int WarnedN = 32;
char g_warned[WarnedN][20];
int g_nWarned = 0;

bool takeWarn(const char* resref) {
    for (int i = 0; i < g_nWarned; ++i) if (strcmp(g_warned[i], resref) == 0) return false;
    if (g_nWarned >= WarnedN) return false;
    strncpy(g_warned[g_nWarned], resref, sizeof(g_warned[0]) - 1);
    g_warned[g_nWarned][sizeof(g_warned[0]) - 1] = 0;
    ++g_nWarned;
    return true;
}

void* imageCached(const char* resref) {
    if (!resref || !*resref) return nullptr;
    for (int i = 0; i < g_nImgs; ++i) if (strcmp(g_imgs[i].name, resref) == 0) return g_imgs[i].img;
    void* im = guikit::imageFor(resref);
    ImgEnt* e;
    if (g_nImgs < ImgCacheN) e = &g_imgs[g_nImgs++];
    else { e = &g_imgs[g_imgNext]; g_imgNext = (g_imgNext + 1) % ImgCacheN; }
    memset(e, 0, sizeof(*e));
    strncpy(e->name, resref, sizeof(e->name) - 1);
    e->img = im;
    if (!im && takeWarn(resref)) KP_LOGV(chMain, KPLOG_WARN, "image '%s' not found: its icon is not drawn", resref);
    return im;
}

void hideAll() {
    widHide(g_pill);
    for (int i = 0; i < g_nSw; ++i) { widHide(g_sw[i].disc); widHide(g_sw[i].icon); widHide(g_sw[i].over); }
}

// ---- refresh state (target / Awareness / slots) ----
esl::Slot g_slots[PoolSlots];
int g_nSlots = 0;
int g_tier = 0;
uintptr_t g_lastTarget = 0;
DWORD g_lastRefresh = 0;
uintptr_t g_awSrv = 0;
DWORD g_awAt = 0;
int g_awVal = 0;
bool g_awValid = false;
uintptr_t g_sigTarget = 0;
int g_sigTier = -1;
char g_sigList[200] = { 0 };
bool g_sigSet = false;
int g_stripLines = 0;
bool g_stripCapLogged = false;

// Server creature object ids / pointers of the party: an effect's creator (+0x18) is matched against any of them (form unproven).
struct PartyIds {
    uint32_t v[64];
    int n = 0;
    void add(uint32_t x) {
        if (x == 0 || x == 0xFFFFFFFFu || n >= 64) return;
        for (int i = 0; i < n; ++i) if (v[i] == x) return;
        v[n++] = x;
    }
    bool has(uint32_t x) const {
        if (x == 0) return false;
        for (int i = 0; i < n; ++i) if (v[i] == x) return true;
        return false;
    }
};

void gatherParty(PartyIds& p) {
    p.n = 0;
    const uintptr_t t = party::table();
    int cnt = party::partyCount(t);
    if (cnt > 8) cnt = 8;
    for (int i = 0; i < cnt; ++i) {
        const uintptr_t client = party::member(t, i);
        if (!client || client == InvalidObj) continue;
        const uintptr_t srv = resolveServer(client);
        p.add(static_cast<uint32_t>(client));
        if (srv) p.add(static_cast<uint32_t>(srv));
        uint32_t id = 0;
        if (srv && saferead::readAt(S_Id, srv, Obj_Id, &id)) p.add(id);
        id = 0;
        if (saferead::readAt(S_Id, client, Obj_Id, &id)) { p.add(id); p.add(id & 0x7FFFFFFFu); }
    }
}

// Reads up to cap effects of the server creature into EffIn rows (icons are static-table resrefs, valid for the process lifetime).
int readEffects(uintptr_t srv, const PartyIds& pids, esl::EffIn* out, int cap) {
    const int oArr = k2addr::off(&K2AddrTable::Creature_EffectArray);
    const int oCnt = k2addr::off(&K2AddrTable::Creature_EffectCount);
    const int oType = k2addr::off(&K2AddrTable::Effect_Type);
    const int oSub = k2addr::off(&K2AddrTable::Effect_Subtype);
    const int oDur = k2addr::off(&K2AddrTable::Effect_Duration);
    const int oCreator = k2addr::off(&K2AddrTable::Effect_Creator);
    const int oSpell = k2addr::off(&K2AddrTable::Effect_SpellId);
    const int oIntCount = k2addr::off(&K2AddrTable::Effect_IntCount);
    const int oInts = k2addr::off(&K2AddrTable::Effect_Ints);
    if (!srv || !oArr || !oCnt || !oType || !oSub || !oDur || !oCreator || !oSpell || !oIntCount || !oInts) return 0;
    uint32_t arr = 0;
    int32_t count = 0;
    if (!saferead::readAt(S_EffArr, srv, oArr, &arr) || !saferead::readAt(S_EffArr, srv, oCnt, &count)) return 0;
    if (!arr || count <= 0 || count > k2fx::MaxEffects) return 0;
    const int n = count < cap ? static_cast<int>(count) : cap;
    int day = 0, ms = 0;
    const bool haveTime = k2fx::currentTime(&day, &ms);
    int m = 0;
    for (int i = 0; i < n; ++i) {
        uint32_t e = 0;
        if (!saferead::readAt(S_EffPtr, arr, i * 4, &e) || !e) continue;
        uint16_t type = 0, sub = 0;
        if (!saferead::readAt(S_Eff2, e, oType, &type)) continue;
        saferead::readAt(S_Eff2, e, oSub, &sub);
        int32_t spell = -1;
        if (!saferead::readAt(S_Eff2, e, oSpell, &spell)) spell = -1;
        uint32_t creator = 0;
        const bool creatorOk = saferead::readAt(S_Eff2, e, oCreator, &creator);

        int state = -1, int2 = 0;
        if (type == EffType_SetState || type == k2fx::EffType_Pool) {
            int32_t nInts = 0;
            uint32_t pInts = 0;
            if (saferead::readAt(S_IntCnt, e, oIntCount, &nInts) && nInts >= 1 && nInts <= 64 &&
                saferead::readAt(S_IntCnt, e, oInts, &pInts) && pInts) {
                int32_t v = 0;
                if (type == EffType_SetState && saferead::readAt(S_IntVal, pInts, 0, &v)) state = static_cast<int>(v);
                v = 0;
                if (type == k2fx::EffType_Pool && nInts >= 3 && saferead::readAt(S_IntVal, pInts, 8, &v)) int2 = static_cast<int>(v);
            }
        }

        bool itD = false, itB = false;
        const char* deb = k2fx::iconForSpell(spell, true, &itD);
        if (!deb && type == k2fx::EffType_Icon) deb = k2fx::iconForEffectIconEffect(static_cast<int>(e), &itD);   // harmful effecticon row
        const char* buf = k2fx::iconForSpell(spell, false, &itB);
        if (buf && deb && strcmp(buf, deb) == 0) buf = nullptr;

        float ratio = -1.0f;   // permanent / unknown
        float dur = 0.0f;
        if (haveTime && saferead::readAt(S_Eff2, e, oDur, &dur) && dur > 0.0f) ratio = k2fx::effectRatio(e, day, ms, true);

        esl::EffIn& o = out[m++];
        o.type = static_cast<int>(type);
        o.sub = static_cast<int>(sub);
        o.spell = static_cast<int>(spell);
        o.state = state;
        o.ratio = ratio;
        o.byParty = creatorOk && pids.has(creator);
        o.debuffIcon = deb;
        o.buffIcon = buf;
        o.item = deb ? itD : itB;
        o.int2 = int2;
    }
    return m;
}

// Leader Awareness = base rank int8 at (*(int8**)(stats+0x174))[3] (skills.2da row 3), cached AwareMs. Unreadable -> 0.
int leaderAwareness(uintptr_t srvLeader, DWORD now) {
    if (!srvLeader) return 0;
    if (g_awValid && srvLeader == g_awSrv && now - g_awAt < AwareMs) return g_awVal;
    const int oStats = k2addr::off(&K2AddrTable::Creature_Stats);
    uint32_t stats = 0, base = 0;
    int8_t r3 = 0;
    int aw = 0;
    if (oStats && saferead::readAt(S_LdrSt, srvLeader, oStats, &stats) && stats &&
        saferead::readAt(S_StatPt, stats, Stats_SkillPtr, &base) && base && saferead::readAt(S_Skill, base, 3, &r3))
        aw = static_cast<int>(r3);
    g_awSrv = srvLeader; g_awAt = now; g_awVal = aw; g_awValid = true;
    return aw;
}

// One INFO line per change of (target, tier, resref list); capped at MaxStripLines per session.
void logSignature(uintptr_t target) {
    char list[sizeof(g_sigList)];
    size_t used = 0;
    list[used++] = '[';
    for (int i = 0; i < g_nSlots; ++i) {
        const int w = snprintf(list + used, sizeof(list) - used, "%s%s", i ? "," : "", g_slots[i].resref);
        if (w < 0 || static_cast<size_t>(w) >= sizeof(list) - used - 1) break;
        used += static_cast<size_t>(w);
    }
    list[used++] = ']';
    list[used] = 0;
    if (g_sigSet && g_sigTarget == target && g_sigTier == g_tier && strcmp(list, g_sigList) == 0) return;
    g_sigSet = true; g_sigTarget = target; g_sigTier = g_tier;
    memcpy(g_sigList, list, used + 1);
    if (g_stripLines < MaxStripLines) {
        ++g_stripLines;
        KP_LOGV(chTarget, KPLOG_INFO, "strip: target=%08x tier=%d slots=%d %s", (unsigned)target, g_tier, g_nSlots, list);
    } else if (!g_stripCapLogged) {
        g_stripCapLogged = true;
        KP_LOGV(chTarget, KPLOG_INFO, "strip: log cap reached (%d lines): no further strip lines this session", MaxStripLines);
    }
}

void refresh(uintptr_t target, uintptr_t leader, DWORD now) {
    g_nSlots = 0;
    const uintptr_t srvT = resolveServer(target);
    const uintptr_t srvL = resolveServer(leader);
    esl::Tiers t;
    t.obvious = g_cfg.tierObvious; t.own = g_cfg.tierOwn; t.full = g_cfg.tierFull;
    g_tier = g_cfg.forceTier >= 0 ? g_cfg.forceTier : esl::tierFor(leaderAwareness(srvL, now), t);
    if (srvT) {
        PartyIds pids;
        if (g_tier == 2) gatherParty(pids);   // creator only matters for the 'own debuffs' tier
        esl::EffIn in[MaxEff];
        const int n = readEffects(srvT, pids, in, MaxEff);
        const int maxIcons = g_cfg.maxIcons < g_nSw ? g_cfg.maxIcons : g_nSw;
        g_nSlots = esl::buildSlots(in, n, g_tier, g_slots, PoolSlots, maxIcons);
    }
    logSignature(target);
}

// TARGET_MENU_POST: runs before every drawn HUD frame (also with no target), so everything here is change-only.
void drawPost(const K2HudContext* ctx) {
    if (!g_cfg.enabled || !g_built) { hideAll(); return; }
    if (static_cast<int32_t>(ctx->hud) != g_panel) {
        // Stale HUD: the old labels may be freed, so drop every pointer without writing to them (a new BUILD makes a new pool).
        g_built = false; g_panel = 0; g_nSw = 0; g_pill = Wid();
        for (SlotW& s : g_sw) s = SlotW();
        g_nSlots = 0; g_lastTarget = 0;
        return;
    }
    const uintptr_t target = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->target));
    const uintptr_t leader = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->leader));
    const uintptr_t menu = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->menu));
    uint8_t flags = 0;
    const bool shown = target && target != InvalidObj && ctx->targetType >= 0 && ctx->targetType <= 3 && menu &&
                       saferead::readAt(S_Flags, menu, Menu_Flags, &flags) && (flags & 1);
    if (!shown) { g_lastTarget = 0; hideAll(); return; }

    const DWORD now = GetTickCount();
    if (target != g_lastTarget || now - g_lastRefresh >= RefreshMs) {
        g_lastTarget = target;
        g_lastRefresh = now;
        refresh(target, leader, now);
    }
    if (g_nSlots <= 0) { hideAll(); return; }

    int32_t mx = 0, my = 0, nx = 0, ny = 0, nw = 0, nh = 0;
    if (!saferead::readAt(S_Draw, menu, Menu_OriginX, &mx) || !saferead::readAt(S_Draw, menu, Menu_OriginY, &my) ||
        !saferead::readAt(S_Draw, menu, Menu_NameX, &nx) || !saferead::readAt(S_Draw, menu, Menu_NameY, &ny) ||
        !saferead::readAt(S_Draw, menu, Menu_NameW, &nw) || !saferead::readAt(S_Draw, menu, Menu_NameH, &nh)) { hideAll(); return; }
    const esl::Layout lay = esl::layoutRow(mx, my, nx, ny, nw, nh, g_nSlots, g_cfg.iconSize, g_cfg.gap, g_cfg.spacing, g_cfg.backingPad);
    if (!lay.ok) { hideAll(); return; }

    void* overImg = imageCached(TexOverlay);
    void* discImg = imageCached(TexDisc);
    const float overA = esl::overlayAlpha(g_cfg.drainAlpha);
    for (int k = 0; k < g_nSw; ++k) {
        SlotW& w = g_sw[k];
        void* img = k < lay.n ? imageCached(g_slots[k].resref) : nullptr;
        if (!img) { widHide(w.disc); widHide(w.icon); widHide(w.over); continue; }   // unused slot, or its icon is missing
        const esl::Slot& sl = g_slots[k];
        const esl::RowRect& r = lay.icon[k];
        if (sl.item && discImg) {
            widFill(w.disc, discImg); widTint(w.disc, TintTeal); widAlpha(w.disc, 1.0f); widRect(w.disc, r); widShow(w.disc);
        } else {
            widHide(w.disc);
        }
        widFill(w.icon, img); widTint(w.icon, TintWhite); widAlpha(w.icon, 1.0f); widRect(w.icon, r); widShow(w.icon);
        const int oh = (esl::drainAt(g_tier) && sl.ratio >= 0.0f) ? esl::overlayH(r.h, sl.ratio) : 0;
        if (oh > 0 && overImg) {
            const esl::RowRect orc = { r.x, r.y, r.w, oh };
            widFill(w.over, overImg); widTint(w.over, TintBlack); widAlpha(w.over, overA); widRect(w.over, orc); widShow(w.over);
        } else {
            widHide(w.over);
        }
    }
    void* pillImg = g_cfg.backing ? imageCached(TexPill) : nullptr;
    if (pillImg) {
        widFill(g_pill, pillImg); widTint(g_pill, TintGrey); widAlpha(g_pill, g_cfg.backingAlpha); widRect(g_pill, lay.pill); widShow(g_pill);
    } else {
        widHide(g_pill);
    }
}

// Creates one hidden, click-through label from LBL_QUEUE1 and appends it to the panel (draw order = creation order).
bool makeWid(uintptr_t panel, Wid& w, const char* what, int k) {
    int why = guikit::CF_None;
    uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &bv);
    if (!c) {
        if (!g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: %s %d create failed (why=%d vtable=%08x): drawing disabled for this HUD", what, k, why, (unsigned)bv); }
        return false;
    }
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (!guikit::appendToPanel(panel, c)) {
        if (!g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: %s %d append failed (control %08x left hidden): drawing disabled for this HUD", what, k, (unsigned)c); }
        return false;
    }
    w = Wid();
    w.c = c;
    w.vis = 0;
    return true;
}

// K2HUD_BUILD (prio 300): the HUD layout is open, so labels can be created from the LBL_QUEUE1 template. A new HUD = a new pool.
void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    g_built = false; g_panel = 0; g_nSw = 0; g_buildFailLogged = false;
    g_pill = Wid();
    for (SlotW& s : g_sw) s = SlotW();
    g_nSlots = 0; g_tier = 0; g_lastTarget = 0; g_lastRefresh = 0;
    g_awSrv = 0; g_awAt = 0; g_awVal = 0; g_awValid = false;
    g_sigSet = false; g_sigTarget = 0; g_sigTier = -1; g_sigList[0] = 0;
    memset(g_imgs, 0, sizeof(g_imgs)); g_nImgs = 0; g_imgNext = 0;
    if (!g_cfg.enabled) { KP_LOGV(chMain, KPLOG_INFO, "BUILD: disabled ([Strip] Enabled=0)"); return; }
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel) || !ctx->panel) { KP_LOGV(chMain, KPLOG_WARN, "BUILD: no panel in the context"); return; }
    saferead::beginScope();
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    const bool pillOk = makeWid(panel, g_pill, "pill", 0);   // first: behind everything else
    int n = 0;
    for (int k = 0; pillOk && k < g_cfg.maxIcons && k < PoolSlots; ++k) {
        SlotW& s = g_sw[k];
        if (!makeWid(panel, s.disc, "disc", k) || !makeWid(panel, s.icon, "icon", k) || !makeWid(panel, s.over, "overlay", k)) break;
        ++n;
    }
    g_nSw = n;
    g_built = pillOk && n == g_cfg.maxIcons;
    if (g_built) g_panel = static_cast<int32_t>(ctx->panel);
    KP_LOGV(chMain, g_built ? KPLOG_INFO : KPLOG_WARN, "BUILD: slots=%d/%d pill=%d", n, g_cfg.maxIcons, pillOk ? 1 : 0);
}

void KPATCH_CALL onTargetMenuPost(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, targetType) + sizeof(ctx->targetType)) { if (g_built) hideAll(); return; }
    saferead::beginScope();
    if (g_cfg.verbose) probeTargetMenuPost(ctx);
    drawPost(ctx);
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_hud.init(api, "enemy-effects-strip");
    loadIni();
    KP_LOGV(chMain, KPLOG_INFO, "enemy-effects-strip 0.3.0-dev loaded (Enabled=%d Verbose=%d MaxIcons=%d)", g_cfg.enabled ? 1 : 0, g_cfg.verbose ? 1 : 0, g_cfg.maxIcons);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);
    g_hud.add(K2HUD_TARGET_MENU_POST, 100, onTargetMenuPost);
    g_hud.connect();
}

} // extern "C"
