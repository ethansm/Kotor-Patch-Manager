// EnemyStrip.cpp -- Enemy Effects Strip 0.4.1-dev (Steam Aspyr build 6A522E71...). Draws the target's status-effect icons as a bare row of
// GUI labels centred just above the target menu's name label. No engine hooks, no engine writes beyond the label setters in GuiKit.h.
// Hook-free consumer of hud-host (k2.hud@1): BUILD (prio 300) creates the label pool while the HUD layout is open, TARGET_MENU_POST (prio 100)
// reads the target's effects (every 250 ms or on a target change), picks the visible icons by the leader's Awareness tier (EnemyStripLogic.h)
// and places the labels (change-only setters). ctx->target and ctx->leader are CLIENT creatures; the effect list, the icon cache and
// the stats live on the SERVER creature, so every read goes through party::serverCreature (research 39 wave 10-17, R3).
// [Strip] Verbose (enemy_strip.ini, default 0): 1 additionally runs the 0.2.0 probe: on a target change one block with the target menu fields,
// the server effect list (spells.2da icon matches, remaining-duration ratio, SetState id), the icon cache and the leader's skill ranks;
// 'effects CHANGED' headers + dumps while a target is set. Log: enemystrip_log.txt (fallback) and the game's mod log.
// Optional settings page (0.4.0, doc 39 item 4; staged edits and sliders 0.4.3): when mod-config (k2.modconfig@1 + k2.menuui@1) is present the mod
// registers one entry under Options -> Mod Configuration. mod-config is NOT in `requires`: the connect is lazy (KPatchInit, then retried from the
// HUD frame until it works) and a missing provider is silent after one log line. The page edits a PENDING copy of the settings (toggles, number
// steppers, alpha sliders); Save writes the changed keys to enemy_strip.ini one at a time (this mod writes the INI itself, mod-config never
// does) and applies them at once, Reset goes back to the saved values and Defaults loads the defaults (then Save). 0.4.4: every setting applies live,
// because the label pool is always built at full size. ForceTier, Verbose and the Tier* thresholds are INI-only (read once at DLL load).
// With a provider that has SetDescSplit the page also shows a live example of the strip (mock name bar, pill, five sample icons with drain
// overlays; esl::previewLayoutZoomed, drawn zoomed so it is readable) in the top 75 % of the right half, redrawn from the pending edits every tick;
// the description keeps the rest.
#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
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
#include "ModConfigApi.h"
#include "EnemyStripLogic.h"
#include "SliderArt.h"

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
    float drainAlpha = 0.0f;   // effective alpha of the drain overlay; 0 = no drain shading (default)
    bool backing = false;      // pill behind the row
    float backingAlpha = 0.55f;
    int backingPad = 4;
    int forceTier = -1;        // -1 = from Awareness, 0..3 forces
} g_cfg;

const char* const IniPath = ".\\enemy_strip.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

// Raw reads only; every range rule (and the tier ordering) lives in esl::sanitize so the host test covers it.
int iniInt(const char* key, int def) {
    return static_cast<int>(GetPrivateProfileIntA("Strip", key, def, IniPath));
}
float iniFloat(const char* key, float def) {
    char dflt[32], buf[32];
    snprintf(dflt, sizeof(dflt), "%g", static_cast<double>(def));
    GetPrivateProfileStringA("Strip", key, dflt, buf, sizeof(buf), IniPath);
    return esl::parseFloatStrict(buf, def);   // trailing junk ("0.65abc") -> default
}

// The sanitized values -> g_cfg (one place, shared by the INI load and the settings page).
void storeCfg(const esl::CfgRaw& c) {
    g_cfg.enabled = c.enabled != 0;
    g_cfg.verbose = c.verbose != 0;
    g_cfg.maxIcons = c.maxIcons;
    g_cfg.iconSize = c.iconSize;
    g_cfg.gap = c.gap;
    g_cfg.spacing = c.spacing;
    g_cfg.tierObvious = c.tierObvious;
    g_cfg.tierOwn = c.tierOwn;
    g_cfg.tierFull = c.tierFull;
    g_cfg.drainAlpha = c.drainAlpha;
    g_cfg.backing = c.backing ? true : false;
    g_cfg.backingAlpha = c.backingAlpha;
    g_cfg.backingPad = c.backingPad;
    g_cfg.forceTier = c.forceTier;
}

// g_cfg as a CfgRaw, with `maxIcons` supplied by the caller (the settings page edits a pending MaxIcons, not the built pool size).
esl::CfgRaw currentRaw(int maxIcons) {
    esl::CfgRaw r;
    r.enabled = g_cfg.enabled ? 1 : 0;
    r.verbose = g_cfg.verbose ? 1 : 0;
    r.maxIcons = maxIcons;
    r.iconSize = g_cfg.iconSize;
    r.gap = g_cfg.gap;
    r.spacing = g_cfg.spacing;
    r.tierObvious = g_cfg.tierObvious;
    r.tierOwn = g_cfg.tierOwn;
    r.tierFull = g_cfg.tierFull;
    r.drainAlpha = g_cfg.drainAlpha;
    r.backing = g_cfg.backing ? 1 : 0;
    r.backingAlpha = g_cfg.backingAlpha;
    r.backingPad = g_cfg.backingPad;
    r.forceTier = g_cfg.forceTier;
    return r;
}

void loadIni() {
    const esl::CfgRaw d;
    esl::CfgRaw r;
    r.enabled = iniInt("Enabled", d.enabled);
    r.verbose = iniInt("Verbose", d.verbose);
    r.maxIcons = iniInt("MaxIcons", d.maxIcons);
    r.iconSize = iniInt("IconSize", d.iconSize);
    r.gap = iniInt("Gap", d.gap);
    r.spacing = iniInt("Spacing", d.spacing);
    r.tierObvious = iniInt("TierObvious", d.tierObvious);
    r.tierOwn = iniInt("TierOwn", d.tierOwn);
    r.tierFull = iniInt("TierFull", d.tierFull);
    r.drainAlpha = iniFloat("DrainAlpha", d.drainAlpha);
    r.backing = iniInt("Backing", d.backing);
    r.backingAlpha = iniFloat("BackingAlpha", d.backingAlpha);
    r.backingPad = iniInt("BackingPad", d.backingPad);
    r.forceTier = iniInt("ForceTier", d.forceTier);
    storeCfg(esl::sanitize(r));
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
    // Order matters: no widget write (hideAll included) before the stale-HUD check. Enabled can now be 0 while g_built is true (live toggle),
    // and a rebuilt HUD may have freed the old labels. An unbuilt (or half-built) pool was never shown, so there is nothing to hide.
    if (!g_built) return;
    if (static_cast<int32_t>(ctx->hud) != g_panel) {
        // Stale HUD: the old labels may be freed, so drop every pointer without writing to them (a new BUILD makes a new pool).
        g_built = false; g_panel = 0; g_nSw = 0; g_pill = Wid();
        for (SlotW& s : g_sw) s = SlotW();
        g_nSlots = 0; g_lastTarget = 0;
        return;
    }
    if (!g_cfg.enabled) { hideAll(); return; }
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
    const bool drainOn = esl::overlayVisible(g_cfg.drainAlpha);
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
        const int oh = (drainOn && esl::drainAt(g_tier) && sl.ratio >= 0.0f) ? esl::overlayH(r.h, sl.ratio) : 0;
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

KP_CHANNEL(chCfg, "config");
void embedLog(const char* fmt, ...) {   // EmbedTex.h's logger-neutral sink
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    KP_LOGV(chCfg, KPLOG_INFO, "EMBED %s", buf);
}

// Creates one hidden, click-through label from `tmpl` (draw order = creation order). HUD labels (onHud, template LBL_QUEUE1) are appended to
// the panel here; settings-page preview labels (template LBL_TITLE in the Options ctor) are NOT: mod-config lists them through AddCustom, and
// since the HUD template carries fill style 2 (stretch) and LBL_TITLE does not, it is set here.
bool makeWid(uintptr_t panel, Wid& w, const char* what, int k, const char* tmpl = "LBL_QUEUE1", bool onHud = true) {
    int why = guikit::CF_None;
    uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, tmpl, &why, &bv);
    if (!c) {
        if (onHud && !g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: %s %d create failed (why=%d vtable=%08x): drawing disabled for this HUD", what, k, why, (unsigned)bv); }
        if (!onHud) { KP_LOGV(chCfg, KPLOG_WARN, "preview: %s %d create failed (why=%d vtable=%08x): no preview", what, k, why, (unsigned)bv); }
        return false;
    }
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (onHud) {
        if (!guikit::appendToPanel(panel, c)) {
            if (!g_buildFailLogged) { g_buildFailLogged = true; KP_LOGV(chMain, KPLOG_WARN, "BUILD: %s %d append failed (control %08x left hidden): drawing disabled for this HUD", what, k, (unsigned)c); }
            return false;
        }
    } else {
        guikit::setFillStyle(c, 2);
    }
    w = Wid();
    w.c = c;
    w.vis = 0;
    return true;
}

// K2HUD_BUILD (prio 300): the HUD layout is open, so labels can be created from the LBL_QUEUE1 template. A new HUD = a new pool. The pool is ALWAYS
// built at full size (PoolSlots, hidden) whatever Enabled / MaxIcons say, so the settings page can change both live: MaxIcons only caps how many
// of the pool are used, Enabled only decides whether anything is shown.
void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    g_built = false; g_panel = 0; g_nSw = 0; g_buildFailLogged = false;
    g_pill = Wid();
    for (SlotW& s : g_sw) s = SlotW();
    g_nSlots = 0; g_tier = 0; g_lastTarget = 0; g_lastRefresh = 0;
    g_awSrv = 0; g_awAt = 0; g_awVal = 0; g_awValid = false;
    g_sigSet = false; g_sigTarget = 0; g_sigTier = -1; g_sigList[0] = 0;
    memset(g_imgs, 0, sizeof(g_imgs)); g_nImgs = 0; g_imgNext = 0;
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel) || !ctx->panel) { KP_LOGV(chMain, KPLOG_WARN, "BUILD: no panel in the context"); return; }
    saferead::beginScope();
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    const bool pillOk = makeWid(panel, g_pill, "pill", 0);   // first: behind everything else
    int n = 0;
    for (int k = 0; pillOk && k < PoolSlots; ++k) {
        SlotW& s = g_sw[k];
        if (!makeWid(panel, s.disc, "disc", k) || !makeWid(panel, s.icon, "icon", k) || !makeWid(panel, s.over, "overlay", k)) break;
        ++n;
    }
    g_nSw = n;
    g_built = pillOk && n > 0;   // a partial pool is usable: draw / hideAll / the refresh clamp are all bound by g_nSw
    if (g_built) g_panel = static_cast<int32_t>(ctx->panel);
    KP_LOGV(chMain, g_built ? KPLOG_INFO : KPLOG_WARN, "BUILD: slots=%d/%d pill=%d (MaxIcons=%d Enabled=%d)", n, PoolSlots, pillOk ? 1 : 0, g_cfg.maxIcons, g_cfg.enabled ? 1 : 0);
}

// ---------------------------------------------------------------- mod-config settings page (0.4.0, doc 39 item 4)
// Hookless and optional. Rows (top to bottom = esl::Setting order, 8 rows = the 8 vanilla column rows): a toggle row is one button; a number
// row is a label (name: value) plus '<' '>' buttons. Items are placed in cbEnter on the COLUMN pane (stripSlot ratios: slot h = col.h*32/284,
// pitch = col.h*36/284, as holster-editor). All callbacks run on the main thread.
// 0.4.1: when the provider has SetDescSplit the right half is split: the top 75 % is a live example of the strip (preview, below), the
// bottom 25 % the description text.

k2mc::ConfigClient g_mcCfg;
k2mc::MenuClient g_mcMenu;
bool g_mcRegistered = false;         // Register has been answered (accepted or refused): stop polling
int32_t g_mcToken = 0;
esl::CfgRaw g_saved;                 // the settings as stored in the INI (and applied to g_cfg)
esl::CfgRaw g_edit;                  // the page's pending copy: rows, sliders and the example work on it; Save writes it, Reset / Defaults only move it
bool g_layoutWarned = false;

struct SettingsPage {
    void* panel = nullptr;
    uint32_t page = 0;
    int32_t text[esl::S_Count];         // toggle: the button; number / slider: the label
    int32_t prev[esl::S_Count];         // number: '<'; slider: the groove (a provider-owned label, the framed track)
    int32_t next[esl::S_Count];         // number: '>'
    int32_t fill[esl::S_Count];         // slider: the filled part of the track (a provider-owned label)
    int32_t thumb[esl::S_Count];        // slider: the marker at the value (a provider-owned label)
    esl::RowRect track[esl::S_Count];   // slider track rect in engine px, set in cbEnter (w == 0: not placed)
    int32_t save = -1, rst = -1, defs = -1;   // the button row: Save, Reset, Defaults
    int dirtyShown = -1;                // Save button text on screen: -1 unknown, 0 "Save", 1 "Save *"
    int drag = -1;                      // slider row being dragged, -1 none
    bool lprev = false;                 // left button state last tick (press edge)
    void reset() {
        panel = nullptr; page = 0;
        for (int i = 0; i < esl::S_Count; ++i) { text[i] = prev[i] = next[i] = fill[i] = thumb[i] = -1; track[i] = { 0, 0, 0, 0 }; }
        save = rst = defs = -1; dirtyShown = -1; drag = -1; lprev = false;
    }
} g_pg;

// ---- preview (0.4.1): an example of the strip in the PREVIEW pane, drawn from the page's PENDING edits (g_edit, 0.4.3) every tick (change-only setters) ----
// 12 labels cloned from LBL_TITLE, listed on the page with AddCustom(MUI_NOFREE), so this mod OWNS them and frees them in cbDestroy once
// mod-config reports them unlinked (Control() == nullptr). Creation order = draw order: bar, pill, then icon k / drain overlay k.
constexpr int PvN = esl::kPreviewControls;
constexpr int PvBar = 0, PvPill = 1;
constexpr int pvIcon(int k) { return 2 + 2 * k; }
constexpr int pvOver(int k) { return 3 + 2 * k; }
constexpr int kPreviewPermille = 750;            // preview share of the description rect; the description keeps the bottom 25 %
const char* const kPreviewBarText = "Sith Assassin";
struct Preview {
    Wid w[PvN];
    int32_t item[PvN];
    int n = 0;                  // controls listed on the page (the first n of w[])
    bool on = false;            // every control listed and the description split is active: the preview draws
    bool paneWarned = false;
    void reset() {
        for (Wid& x : w) x = Wid();
        for (int32_t& i : item) i = -1;
        n = 0; on = false; paneWarned = false;
    }
} g_pv;

void pvHideAll() { for (int i = 0; i < g_pv.n; ++i) widHide(g_pv.w[i]); }

// mod-config shows every page item on entry (setVisible(true) also clears click-through): forget a visibility cache that no longer matches.
void pvResync() {
    for (int i = 0; i < g_pv.n; ++i) {
        Wid& x = g_pv.w[i];
        if (x.c && x.vis >= 0 && guikit::isVisible(x.c) != (x.vis == 1)) x.vis = -1;
    }
}

void refreshPreview() {
    if (!g_pv.n || !g_pg.page || !g_pg.panel || !g_mcMenu.ok()) return;
    pvResync();
    K2MuiRect r = { 0, 0, 0, 0 };
    if (!g_pv.on) {
        pvHideAll();
    } else if (!g_mcMenu.p->PaneRect(g_pg.panel, MUI_PANE_PREVIEW, &r) || r.w <= 0 || r.h <= 0) {
        if (!g_pv.paneWarned) { g_pv.paneWarned = true; KP_LOGV(chCfg, KPLOG_WARN, "preview: no PREVIEW pane rect (%d,%d,%d,%d): preview hidden", r.x, r.y, r.w, r.h); }
        pvHideAll();
    } else {
        const esl::RowRect pane = { r.x, r.y, r.w, r.h };
        const esl::PreviewLayout pl = esl::previewLayoutZoomed(pane, g_edit.enabled ? 1 : 0, g_edit.maxIcons, g_edit.iconSize, g_edit.gap, g_edit.spacing,
                                                          g_edit.backingPad, g_edit.backing != 0);
        if (!pl.ok) {
            pvHideAll();
        } else {
            void* overImg = imageCached(TexOverlay);
            Wid& bar = g_pv.w[PvBar];
            widFill(bar, overImg); widTint(bar, TintBlack); widAlpha(bar, 1.0f); widRect(bar, pl.name); widShow(bar);
            void* pillImg = pl.pill ? imageCached(TexPill) : nullptr;
            Wid& pill = g_pv.w[PvPill];
            if (pillImg) { widFill(pill, pillImg); widTint(pill, TintGrey); widAlpha(pill, g_edit.backingAlpha); widRect(pill, pl.row.pill); widShow(pill); }
            else widHide(pill);
            const float overA = esl::overlayAlpha(g_edit.drainAlpha);
            for (int k = 0; k < esl::kPreviewMaxIcons; ++k) {
                Wid& ic = g_pv.w[pvIcon(k)];
                Wid& ov = g_pv.w[pvOver(k)];
                const esl::PreviewSample& sm = esl::previewSample(k);
                void* img = k < pl.n ? imageCached(sm.resref) : nullptr;
                if (!img) { widHide(ic); widHide(ov); continue; }   // beyond the shown count, or this icon is missing
                const esl::RowRect& rc = pl.row.icon[k];
                widFill(ic, img); widTint(ic, TintWhite); widAlpha(ic, 1.0f); widRect(ic, rc); widShow(ic);
                const int oh = esl::overlayVisible(g_edit.drainAlpha) ? esl::overlayH(rc.h, sm.ratio) : 0;
                if (oh > 0 && overImg) {
                    const esl::RowRect orc = { rc.x, rc.y, rc.w, oh };
                    widFill(ov, overImg); widTint(ov, TintBlack); widAlpha(ov, overA); widRect(ov, orc); widShow(ov);
                } else {
                    widHide(ov);
                }
            }
        }
    }
    for (int i = 0; i < g_pv.n; ++i) guikit::setClickable(g_pv.w[i].c, false);   // the preview must never swallow clicks
}

// Options ctor window, after the rows: creates and lists the preview labels, then asks for the description split. Without SetDescSplit
// (older mod-config) nothing is created and the page works as before.
void buildPreview(void* panel, uint32_t page) {
    g_pv.reset();
    const K2MenuUiApi* ui = g_mcMenu.p;
    if (!g_mcMenu.hasDescSplit()) { KP_LOGV(chCfg, KPLOG_INFO, "preview skipped: provider has no SetDescSplit (older mod-config); page works without it"); return; }
    const uintptr_t pn = reinterpret_cast<uintptr_t>(panel);
    Wid tmp[PvN];
    int made = 0;
    for (; made < PvN; ++made) {
        const char* what = made == PvBar ? "bar" : (made == PvPill ? "pill" : ((made - 2) % 2 == 0 ? "icon" : "overlay"));
        if (!makeWid(pn, tmp[made], what, made, "LBL_TITLE", false)) break;
        if (made != PvBar) guikit::setText(tmp[made].c, "");   // the clone carries the Options title text ("OPTIONS"): blank it so it never shows over an icon
    }
    int listed = 0;
    if (made == PvN) {
        for (; listed < PvN; ++listed) {
            const int id = ui->AddCustom(page, reinterpret_cast<void*>(tmp[listed].c), MUI_NOFREE);
            if (id < 0) break;
            g_pv.w[listed] = tmp[listed];
            g_pv.item[listed] = id;
        }
        g_pv.n = listed;
    }
    for (int j = g_pv.n; j < made; ++j) guikit::releaseUnlinkedLabel(tmp[j].c, guikit::LabelVtable);   // created but never listed
    if (made != PvN) { KP_LOGV(chCfg, KPLOG_WARN, "preview skipped: %d/%d labels created", made, PvN); return; }
    if (g_pv.n != PvN) { KP_LOGV(chCfg, KPLOG_WARN, "preview skipped: AddCustom listed %d/%d labels (listed ones stay hidden)", g_pv.n, PvN); return; }
    guikit::setAlign(g_pv.w[PvBar].c, 18);
    guikit::setText(g_pv.w[PvBar].c, kPreviewBarText);
    if (!ui->SetDescSplit(page, kPreviewPermille)) { KP_LOGV(chCfg, KPLOG_WARN, "preview skipped: SetDescSplit(%u, %d) refused (listed labels stay hidden)", page, kPreviewPermille); return; }
    g_pv.on = true;
    KP_LOGV(chCfg, KPLOG_INFO, "preview BUILD ok: %d labels, split %d permille", PvN, kPreviewPermille);
}

// Options dtor, after mod-config unlinked its controls: free the preview labels it listed MUI_NOFREE. A label whose Control() still answers
// is still in the panel list (the unlink was refused): it is leaked, never freed.
void destroyPreview() {
    int freed = 0, kept = 0;
    if (g_pv.n && g_mcMenu.ok() && g_pg.page) {
        for (int i = 0; i < g_pv.n; ++i) {
            if (!g_pv.w[i].c) continue;
            if (g_mcMenu.p->Control(g_pg.page, g_pv.item[i])) { ++kept; continue; }
            guikit::releaseUnlinkedLabel(g_pv.w[i].c, guikit::LabelVtable);
            ++freed;
        }
        KP_LOGV(chCfg, KPLOG_INFO, "preview DESTROY: freed=%d kept=%d", freed, kept);
    }
    g_pv.reset();
}

const char* const kPageDescPreview =
    "Example of the icon row above a target's name, drawn with your edits. Drag or click the opacity bars. Nothing changes in the game until you press Save. Reset undoes unsaved edits; Defaults loads the default values (then press Save).";
const char* const kPageDescPlain =
    "Icon row above the target name. Drag or click the opacity bars. Nothing changes in the game until you press Save. Reset undoes unsaved edits; Defaults loads the default values.";

const char* const kRowDesc[esl::S_Count] = {
    "Show or hide the icon row.",
    "Most icons shown at once (1-8). The example above shows at most 5.",
    "Icon edge in pixels. Auto uses the height of the target name label.",
    "Pixels between the icon row and the name label.",
    "Pixels between neighbouring icons.",
    "Draw a pill behind the icon row.",
    "Opacity of the pill behind the icon row. Drag the bar or click on it.",   // (a label row has no hover text; the page description covers it)
    "Opacity of the dark remaining-time drain shading on the icons. 0 = off (default). Drag the bar or click on it.",
};
const char* const kSaveDesc = "Write the edits to enemy_strip.ini and apply them to the game now.";
const char* const kResetDesc = "Throw away unsaved edits: go back to the saved values.";
const char* const kDefaultsDesc = "Load the default value of every row. They apply when you press Save.";

// Every setting applies live since 0.4.4 (the label pool is always built at full size): no row needs a restart.
bool rowNeedsRestart(int, const esl::CfgRaw&) { return false; }

void rowText(int s, char* out, int cap) {
    esl::formatRowText(s, g_edit, rowNeedsRestart(s, g_edit), out, cap);
}

void refreshRow(int s) {
    if (!g_mcMenu.ok() || !g_pg.page || g_pg.text[s] < 0) return;
    char b[64];
    rowText(s, b, static_cast<int>(sizeof b));
    g_mcMenu.p->SetText(g_pg.page, g_pg.text[s], b);
}

// ---- slider rows (alpha): three labels in draw order: the groove (the framed pill art, so it looks like the page's buttons), the fill bar and the
// marker. mod-config hands out pre-cloned controls (all buttons before all labels), so anything drawn ON TOP of a button must be a label, which
// is why the track is not a button. The labels are provider-owned (AddLabel); their engine pointers come from Control() and are re-read on every
// page enter. Dragging is polled here: a press edge inside the track rect starts it, the cursor x then sets the value on the 0.05 grid until the
// button is released. No hover sound or highlight: the fill turns brighter while dragging.
Wid g_slGroove[esl::S_Count], g_slFill[esl::S_Count], g_slThumb[esl::S_Count];
void clearSliderWids() { for (int s = 0; s < esl::S_Count; ++s) { g_slGroove[s] = Wid(); g_slFill[s] = Wid(); g_slThumb[s] = Wid(); } }   // the labels belong to the provider and die with the panel
bool inTrack(const esl::RowRect& r, int x, int y) { return r.w > 0 && r.h > 0 && x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h; }

void refreshSlider(int s) {
    if (!g_mcMenu.ok() || !g_pg.page || g_pg.fill[s] < 0) return;
    const esl::RowRect& tr = g_pg.track[s];
    Wid& gr = g_slGroove[s];
    Wid& f = g_slFill[s];
    Wid& th = g_slThumb[s];
    if (tr.w <= 0 || !gr.c || !f.c || !th.c) return;
    guikit::setClickable(gr.c, false);   // the labels must never swallow a click meant for the rows around them
    guikit::setClickable(f.c, false);
    guikit::setClickable(th.c, false);
    // The holster editor's slider art (frame outline, wedge, marker): embedded textures that exist a few ticks after the page opens. Until then the
    // groove is the plain pill and the fill / marker labels stay empty.
    void* frameImg = hse::sliderArtImage(hse::SA_Frame);
    void* wedgeImg = hse::sliderArtImage(hse::SA_Wedge);
    void* markImg = hse::sliderArtImage(hse::SA_Mark);
    if (!frameImg || !wedgeImg || !markImg) {
        void* pillImg = imageCached(TexPill);
        if (pillImg) { widFill(gr, pillImg); widTint(gr, TintTeal); widAlpha(gr, 1.0f); widRect(gr, tr); }
        return;
    }
    const esl::RowRect inner = { tr.x + 6, tr.y, tr.w - 12 > 0 ? tr.w - 12 : 1, tr.h };
    const int fillPx = esl::sliderFillPx(esl::settingFloat(g_edit, s), inner.w);   // the same mapping the drag uses (sliderHundredths)
    const int markW = 8;
    int center = inner.x + fillPx;
    if (center < tr.x + markW / 2) center = tr.x + markW / 2;
    if (center > tr.x + tr.w - markW / 2) center = tr.x + tr.w - markW / 2;
    widFill(gr, frameImg); widAlpha(gr, 1.0f); widRect(gr, tr);
    widFill(f, wedgeImg); widAlpha(f, 1.0f);
    const int wedgeW = center - tr.x;   // the wedge is the track cut at the marker centre: its tall end always meets the marker
    widRect(f, wedgeW > 0 ? esl::RowRect{ tr.x, tr.y, wedgeW, tr.h } : esl::RowRect{ -9999, -9999, 1, 1 });
    widFill(th, markImg); widAlpha(th, 1.0f); widRect(th, { center - markW / 2, tr.y, markW, tr.h });
}

void refreshSaveButton() {
    if (!g_mcMenu.ok() || !g_pg.page || g_pg.save < 0) return;
    const int dirty = esl::pageDiffers(g_saved, g_edit) ? 1 : 0;
    if (dirty == g_pg.dirtyShown) return;
    g_mcMenu.p->SetText(g_pg.page, g_pg.save, dirty ? "Save *" : "Save");
    g_pg.dirtyShown = dirty;
}

void refreshAll() {
    for (int s = 0; s < esl::S_Count; ++s) {
        refreshRow(s);
        if (esl::settingIsSlider(s)) refreshSlider(s);
    }
    refreshSaveButton();
    refreshPreview();   // the example redraws from the pending edits at once (and again every tick)
}

// One step on setting s of the PENDING edits only (nothing is written or applied until Save).
void editSetting(int s, int dir) {
    const esl::CfgRaw nxt = esl::stepSetting(g_edit, s, dir);
    if (esl::sameSetting(g_edit, nxt, s)) return;   // at a limit
    g_edit = nxt;
    char val[32];
    esl::formatIniValue(s, g_edit, val, static_cast<int>(sizeof val));
    KP_LOGV(chCfg, KPLOG_INFO, "edit: %s=%s (unsaved)", esl::settingKey(s), val);
}

// Save: write every changed key (one key at a time) and apply what was written. A key whose INI write fails stays unsaved (the page keeps showing
// "Save *"). The live values follow through g_saved (already sanitized by stepSetting / setSliderValue); the label pool is always built at full
// size, so every setting including MaxIcons and Enabled applies at once. g_lastRefresh = 0 re-runs the effect read and the change-only caches
// re-send what differs.
void saveEdits() {
    int wrote = 0, failed = 0;
    for (int s = 0; s < esl::S_Count; ++s) {
        if (esl::sameSetting(g_saved, g_edit, s)) continue;
        char val[32];
        esl::formatIniValue(s, g_edit, val, static_cast<int>(sizeof val));
        if (WritePrivateProfileStringA("Strip", esl::settingKey(s), val, IniPath)) {
            g_saved = esl::copySetting(g_saved, g_edit, s);
            ++wrote;
            KP_LOGV(chCfg, KPLOG_INFO, "cfg: saved %s=%s", esl::settingKey(s), val);
        } else {
            ++failed;
            KP_LOGV(chCfg, KPLOG_WARN, "cfg: INI write FAILED for %s=%s (left unsaved)", esl::settingKey(s), val);
        }
    }
    if (wrote) {
        storeCfg(g_saved);
        g_lastRefresh = 0;
    }
    KP_LOGV(chCfg, KPLOG_INFO, "cfg: SAVE wrote=%d failed=%d", wrote, failed);
    refreshAll();
}

void resetEdits() {
    g_edit = g_saved;
    KP_LOGV(chCfg, KPLOG_INFO, "edit: RESET to saved values");
    refreshAll();
}

void defaultEdits() {
    g_edit = esl::pageDefaults(g_saved);
    KP_LOGV(chCfg, KPLOG_INFO, "edit: DEFAULTS loaded (unsaved)");
    refreshAll();
}

// Options ctor window: creates the page items (valid only here).
void KPATCH_CALL cbBuild(void* panel, void*) {
    g_pg.reset();
    clearSliderWids();
    if (!panel || !g_mcMenu.ok()) { KP_LOGV(chCfg, KPLOG_WARN, "page BUILD skipped panel=%p menuui=%d", panel, g_mcMenu.ok() ? 1 : 0); return; }
    const K2MenuUiApi* ui = g_mcMenu.p;
    const uint32_t page = ui->NewPage(panel, "Enemy Effects Strip");
    if (!page) { KP_LOGV(chCfg, KPLOG_WARN, "page BUILD: NewPage failed"); return; }
    g_edit = g_saved;   // the page always starts from the saved values
    KP_LOGV(chCfg, KPLOG_INFO, "slider art textures ensure=%d", hse::sliderArtEnsure() ? 1 : 0);
    bool ok = true;
    for (int s = 0; s < esl::S_Count; ++s) {
        char b[64];
        rowText(s, b, static_cast<int>(sizeof b));
        if (esl::settingIsToggle(s)) {
            g_pg.text[s] = ui->AddButton(page, b, kRowDesc[s]);
        } else if (esl::settingIsSlider(s)) {
            g_pg.text[s] = ui->AddLabel(page, b);
            g_pg.fill[s] = ui->AddLabel(page, "");   // draw order: wedge, frame outline, marker
            g_pg.prev[s] = ui->AddLabel(page, "");   // the groove / frame
            g_pg.thumb[s] = ui->AddLabel(page, "");
            if (g_pg.prev[s] < 0 || g_pg.fill[s] < 0 || g_pg.thumb[s] < 0) ok = false;
        } else {
            g_pg.text[s] = ui->AddLabel(page, b);
            g_pg.prev[s] = ui->AddButton(page, "<", kRowDesc[s]);
            g_pg.next[s] = ui->AddButton(page, ">", kRowDesc[s]);
            if (g_pg.prev[s] < 0 || g_pg.next[s] < 0) ok = false;
        }
        if (g_pg.text[s] < 0) ok = false;
    }
    g_pg.save = ui->AddButton(page, "Save", kSaveDesc);
    g_pg.rst = ui->AddButton(page, "Reset", kResetDesc);
    g_pg.defs = ui->AddButton(page, "Defaults", kDefaultsDesc);
    if (g_pg.save < 0 || g_pg.rst < 0 || g_pg.defs < 0) ok = false;
    if (!ok) { KP_LOGV(chCfg, KPLOG_WARN, "page BUILD: an item failed (page %u left inert)", page); g_pg.reset(); return; }
    g_pg.panel = panel;
    g_pg.page = page;
    g_pg.dirtyShown = 0;
    KP_LOGV(chCfg, KPLOG_INFO, "page BUILD ok page=%u rows=%d", page, static_cast<int>(esl::S_Count));
    buildPreview(panel, page);
}

void KPATCH_CALL cbEnter(uint32_t page, void*) {
    if (!g_pg.panel || page != g_pg.page || !g_mcMenu.ok()) return;
    saferead::beginScope();
    const K2MenuUiApi* ui = g_mcMenu.p;
    g_edit = g_saved;   // unsaved edits are dropped when the page is left
    g_pg.drag = -1;
    g_pg.lprev = false;
    g_pg.dirtyShown = -1;
    K2MuiRect col = { 0, 0, 0, 0 };
    if (ui->PaneRect(g_pg.panel, MUI_PANE_COLUMN, &col) && col.w > 0 && col.h > 0) {
        const int slotH = col.h * 32 / 284, pitch = col.h * 36 / 284;
        const int gap = 6;
        int bw = col.w * 20 / 100;
        int lw = col.w - 2 * bw - 2 * gap;
        if (lw < 0) { lw = 0; bw = (col.w - 2 * gap) / 2; if (bw < 0) bw = 0; }
        for (int s = 0; s < esl::S_Count; ++s) {
            const int y = col.y + s * pitch;
            if (esl::settingIsToggle(s)) {
                const K2MuiRect r = { col.x, y, col.w, slotH };
                ui->SetRect(g_pg.page, g_pg.text[s], &r);
            } else if (esl::settingIsSlider(s)) {
                const K2MuiRect rl = { col.x, y, lw, slotH };
                const K2MuiRect rt = { col.x + lw + gap, y, 2 * bw + gap, slotH };
                ui->SetRect(g_pg.page, g_pg.text[s], &rl);
                ui->SetRect(g_pg.page, g_pg.prev[s], &rt);
                g_pg.track[s] = { rt.x, rt.y, rt.w, rt.h };
                g_slGroove[s] = Wid(); g_slFill[s] = Wid(); g_slThumb[s] = Wid();   // fresh caches: the controls may be new objects
                g_slGroove[s].c = reinterpret_cast<uintptr_t>(ui->Control(g_pg.page, g_pg.prev[s]));
                g_slFill[s].c = reinterpret_cast<uintptr_t>(ui->Control(g_pg.page, g_pg.fill[s]));
                g_slThumb[s].c = reinterpret_cast<uintptr_t>(ui->Control(g_pg.page, g_pg.thumb[s]));
                KP_LOGV(chCfg, KPLOG_INFO, "slider %s: groove=%08x fill=%08x thumb=%08x track=%d,%d,%d,%d", esl::settingKey(s), static_cast<unsigned>(g_slGroove[s].c),
                        static_cast<unsigned>(g_slFill[s].c), static_cast<unsigned>(g_slThumb[s].c), rt.x, rt.y, rt.w, rt.h);
                if (g_slGroove[s].c) guikit::setFillStyle(g_slGroove[s].c, 2);   // fill mode 2: the label draws its fill image, as for the example labels
                if (g_slFill[s].c) guikit::setFillStyle(g_slFill[s].c, 2);
                if (g_slThumb[s].c) guikit::setFillStyle(g_slThumb[s].c, 2);
            } else {
                const K2MuiRect rl = { col.x, y, lw, slotH };
                const K2MuiRect rp = { col.x + lw + gap, y, bw, slotH };
                const K2MuiRect rn = { col.x + lw + gap + bw + gap, y, bw, slotH };
                ui->SetRect(g_pg.page, g_pg.text[s], &rl);
                ui->SetRect(g_pg.page, g_pg.prev[s], &rp);
                ui->SetRect(g_pg.page, g_pg.next[s], &rn);
            }
        }
        // The button row under the rows: Save | Reset | Defaults, a quarter pitch below the last row.
        const int by = col.y + esl::S_Count * pitch + pitch / 4;
        int w3 = (col.w - 2 * gap) / 3;
        if (w3 < 0) w3 = 0;
        const K2MuiRect rs = { col.x, by, w3, slotH };
        const K2MuiRect rr = { col.x + w3 + gap, by, w3, slotH };
        const K2MuiRect rd = { col.x + 2 * (w3 + gap), by, w3, slotH };
        ui->SetRect(g_pg.page, g_pg.save, &rs);
        ui->SetRect(g_pg.page, g_pg.rst, &rr);
        ui->SetRect(g_pg.page, g_pg.defs, &rd);
    } else if (!g_layoutWarned) {
        g_layoutWarned = true;
        KP_LOGV(chCfg, KPLOG_WARN, "page ENTER: no COLUMN pane rect (%d,%d,%d,%d): items stay unplaced", col.x, col.y, col.w, col.h);
    }
    ui->SetDesc(g_pg.page, g_pv.on ? kPageDescPreview : kPageDescPlain);
    refreshAll();   // mod-config just showed every page item: hide what must stay hidden, keep the rest click-through
}

bool leftDown() {
    return guikit::g_gk.gameForeground && guikit::g_gk.lbuttonDown && guikit::g_gk.gameForeground() && guikit::g_gk.lbuttonDown();
}

// Every Options draw while the page is shown: Clicked() is 1 once per click, so each item is asked every tick.
void KPATCH_CALL cbTick(uint32_t page, void*) {
    if (!g_pg.panel || page != g_pg.page || !g_mcMenu.ok()) return;
    saferead::beginScope();
    embtex::heal();   // the slider art textures survive a GL reset
    const K2MenuUiApi* ui = g_mcMenu.p;
    for (int s = 0; s < esl::S_Count; ++s) {
        if (esl::settingIsSlider(s)) continue;   // the track is driven by the polled press / drag below
        int dir = 0;
        if (esl::settingIsToggle(s)) {
            if (ui->Clicked(g_pg.page, g_pg.text[s])) dir = 1;
        } else {
            const bool dn = ui->Clicked(g_pg.page, g_pg.prev[s]) != 0;
            const bool up = ui->Clicked(g_pg.page, g_pg.next[s]) != 0;
            dir = up ? 1 : (dn ? -1 : 0);
        }
        if (dir) editSetting(s, dir);
    }
    // Sliders: press edge over a track starts a drag; while held the cursor x sets the value (nothing is saved until Save).
    const bool down = leftDown();
    int cx = 0, cy = 0;
    const bool haveCur = guikit::cursorEngine(&cx, &cy);
    if (!down) {
        g_pg.drag = -1;
    } else if (!g_pg.lprev && haveCur) {
        g_pg.drag = -1;
        for (int s = 0; s < esl::S_Count; ++s) {
            if (esl::settingIsSlider(s) && inTrack(g_pg.track[s], cx, cy)) { g_pg.drag = s; break; }
        }
    }
    if (g_pg.drag >= 0 && down && haveCur) {
        const esl::RowRect& tr = g_pg.track[g_pg.drag];
        const int inX = tr.x + 6, inW = tr.w - 12;   // the same inner span the fill bar uses
        const esl::CfgRaw nxt = esl::setSliderValue(g_edit, g_pg.drag, esl::sliderHundredths(cx, inX, inW));
        if (!esl::sameSetting(g_edit, nxt, g_pg.drag)) {
            g_edit = nxt;
            KP_LOGV(chCfg, KPLOG_INFO, "edit: %s=%.2f (unsaved, drag)", esl::settingKey(g_pg.drag), static_cast<double>(esl::settingFloat(g_edit, g_pg.drag)));
        }
    }
    g_pg.lprev = down;
    if (ui->Clicked(g_pg.page, g_pg.save)) saveEdits();
    if (ui->Clicked(g_pg.page, g_pg.rst)) resetEdits();
    if (ui->Clicked(g_pg.page, g_pg.defs)) defaultEdits();
    refreshAll();   // the example follows a pane rect change (resolution, split asserted after enter); rows and sliders are placed once per enter
}

void KPATCH_CALL cbDestroy(void* panel, void*) {
    if (!panel || panel != g_pg.panel) return;
    clearSliderWids();   // the slider labels are provider-owned: never touch them after the panel is gone
    destroyPreview();   // mod-config frees its own controls; this mod owns the MUI_NOFREE preview labels (mod-config unlinked them already)
    g_pg.reset();
}

K2ModConfigEntry g_mcEntry;   // Register copies it; the strings are static

// Lazy connect: KPatchInit tries once, the HUD frame retries (poll() = the first call, then every 256th) until both interfaces answer.
// Returns true once Register has been answered. Absent mod-config = no page, no error.
bool tryRegisterPage() {
    if (g_mcRegistered) return true;
    g_mcCfg.poll();
    g_mcMenu.poll();
    if (!g_mcCfg.ok() || !g_mcMenu.ok()) return false;
    g_mcEntry = K2ModConfigEntry();
    g_mcEntry.struct_size = sizeof(K2ModConfigEntry);
    g_mcEntry.id = "enemy-effects-strip";
    g_mcEntry.name = "Enemy Effects Strip";
    g_mcEntry.desc = "Icon row above the target name: size, spacing, backing and drain overlay. Edit, then press Save to write enemy_strip.ini and apply at once.";
    g_mcEntry.user = nullptr;
    g_mcEntry.build = cbBuild;
    g_mcEntry.enter = cbEnter;
    g_mcEntry.leave = nullptr;
    g_mcEntry.tick = cbTick;
    g_mcEntry.destroy = cbDestroy;
    g_mcToken = g_mcCfg.p->Register(&g_mcEntry);
    g_mcRegistered = true;
    if (g_mcToken > 0) KP_LOGV(chCfg, KPLOG_INFO, "page REGISTER ok token=%d", static_cast<int>(g_mcToken));
    else KP_LOGV(chCfg, KPLOG_WARN, "page REGISTER refused (mod-config table full or bad entry)");
    return true;
}

void KPATCH_CALL onTargetMenuPost(const K2HudContext* ctx, void*) {
    if (!g_mcRegistered) tryRegisterPage();
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, targetType) + sizeof(ctx->targetType)) { if (g_built) hideAll(); return; }
    saferead::beginScope();
    if (g_cfg.verbose) probeTargetMenuPost(ctx);
    drawPost(ctx);
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    embtex::g_log = embedLog;
    g_hud.init(api, "enemy-effects-strip");
    loadIni();
    g_saved = currentRaw(g_cfg.maxIcons);
    g_edit = g_saved;
    g_pg.reset();
    g_mcCfg.init(api);
    g_mcMenu.init(api);
    KP_LOGV(chMain, KPLOG_INFO, "enemy-effects-strip 0.4.6-dev loaded (Enabled=%d Verbose=%d MaxIcons=%d)", g_cfg.enabled ? 1 : 0, g_cfg.verbose ? 1 : 0, g_cfg.maxIcons);
    g_hud.add(K2HUD_BUILD, 300, onHudBuild);
    g_hud.add(K2HUD_TARGET_MENU_POST, 100, onTargetMenuPost);
    g_hud.connect();
    if (!tryRegisterPage()) KP_LOGV(chCfg, KPLOG_INFO, "page skipped for now: k2.modconfig@1 / k2.menuui@1 not provided (mod-config absent or later); retrying from the HUD frame");
}

} // extern "C"
