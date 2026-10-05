// Companion Self-Heal 0.5.0-dev (doc 27, CS4 restyle + CS5 + CS8/CS9). The HUD gets, from hud-host's K2HUD_BUILD phase (k2.hud@1, source 0x0040F5A0, HUD panel only; the only own hook is the RMB camera swallow 0x7B3817), 14 runtime
// labels (2 dark discs under the buttons, 2 cross-icon buttons, a popover background, 5 popover items, 2 value labels, 2 gold auto rings drawn on top; the cross, disc + ring art are
// embedded textures, see namespace tex; fallback = the text "+"). Per frame (K2PS_FRAME_BEGIN, only in gameplay HUD):
// buttons follow the companion cards (slot+0x694 vit rect); LMB = use the picked healing item now (route A), RMB = popover
// (Auto on/off, use-below %, keep reserve; persisted to companion_selfheal.ini); Auto mode uses items by itself (HealItems.h rules).
// Route A is reachable ONLY through useNow(): HUD gate + commandable (+0xEC) + CanUseItem + per-member 2 s debounce. The popover never calls it.
// Engine writes: label setters + the ProjectionFlag clear/restore around the creates + our own texture objects. Everything else is SafeRead.
// Logs: P1/P3 (hook, slot map), DUMP (Ctrl+Shift+ProbeKey), BUILD, MAP, CLICK/USE/AUTO/DONE/REFUSE, CFG.
// Opt-in (companion_selfheal.ini [probe] call=1): Ctrl+Shift+CallKey (default 'J') = useNow(first companion with a pick, "key") + POST/RESULT window.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "KpLog.h"
#include "SafeRead.h"
#include "K2Addr.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "HudHost.h"
#include "HealItems.h"
#include "ItemRead.h"
#include "GuiKit.h"
#include "GuiTip.h"
#include "FileWatch.h"
#include "SelfHealUi.h"

KP_LOG_PATCH("companion-selfheal");

namespace {

constexpr unsigned SiteBase = 0x6E00;                    // saferead site id
constexpr uintptr_t HudVtable = gameaddr::HudVtable;             // CSWGuiInGame (HUD) RTTI vtable
constexpr uintptr_t ServerIdMask = gameaddr::ServerIdMask;       // global: client id & ~[mask] = server id (ClientToServerObjectId)
constexpr uintptr_t UseItemRouteA = gameaddr::UseItemRouteA;     // logged; called only with [probe] call=1
constexpr uintptr_t CanUseItem = gameaddr::CanUseItem;           // thiscall(server creature; itemPtr,0,1) RET 0xC
constexpr int Panel_Ctl = 0x24, Panel_CtlCount = 0x28, Panel_Open = 0x30, Panel_Flags = 0x48;
constexpr int SlotFromHud = 0x1F34, SlotStride = 0xDE4;  // portrait slot k = hud + 0x1F34 + k*0xDE4
constexpr int Client_ObjectId = 4;
constexpr int Server_Commandable = 0xEC, Server_Stats = 0x1198;
constexpr int Stats_Race = 0xE0;                          // short, creature race (R5 probe)
constexpr int Stats_Skills = 0x174, Stats_Abilities = 0xED, Skill_Repair = 5, Skill_TreatInjury = 7, Abil_Int = 3, Abil_Wis = 4;
constexpr int ItemGroup = 1;                              // HUD item group
constexpr int MaxCands = 64;
constexpr int Slot_VitRect = 0x694;                       // K2Addr Slot_VitRect: 4 ints x,y,w,h (screen px)
constexpr int Hud_Manager = 0x1C, Mgr_Panels = 0x88, Mgr_PanelCount = 0x8C, Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;
constexpr int Cr_Tag = 0x18, PanelIgnoredByInput = 0x600;
// Panels that do not block gameplay hotkeys (RTTI vtables): messages, fade, pause, auto-pause
constexpr uintptr_t PanelWhitelist[] = { gameaddr::MessagesPanelVtable, gameaddr::FadePanelVtable, gameaddr::PausePanelVtable, gameaddr::AutoPausePanelVtable };
constexpr int NWidgets = 14;
constexpr DWORD CacheMs = 250;
constexpr DWORD DebounceMs = 2000, PostMs = 10000, PostStepMs = 500;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chHud, "hud");
KP_CHANNEL(chDump, "dump");
KP_CHANNEL(chCall, "call");
KP_CHANNEL(chUi, "ui");
KP_CHANNEL(chCfg, "cfg");

k2ps::Client g_ps;
k2hud::Client g_hudc;
uintptr_t g_hud = 0;
bool g_logOn = true;
int g_probeKey = 0x48;
DWORD g_lastPollMs = 0;
bool g_keyPrev = false;
int g_probeCall = 0, g_callKey = 0x4A;
bool g_callPrev = false;
DWORD g_lastCallMs = 0;
struct Post { bool active = false; DWORD start = 0, next = 0; int member = 0; uint32_t itemKey = 0; int preHp = 0, preStack = 0; uintptr_t server = 0; } g_post;

template <typename T> bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(SiteBase, base, off, out); }

bool foreground() {
    HWND w = GetForegroundWindow(); DWORD pid = 0;
    if (!w) return false;
    GetWindowThreadProcessId(w, &pid);
    return pid == GetCurrentProcessId();
}

// ".\\" prefix: GetPrivateProfile*A with a bare file name looks in the Windows directory, not the game dir (CWD).
const char* const IniPath = ".\\companion_selfheal.ini";

void loadIni() {
    g_logOn = GetPrivateProfileIntA("General", "Log", 1, "companion_selfheal.ini") != 0;
    g_probeKey = GetPrivateProfileIntA("General", "ProbeKey", 0x48, IniPath);
    if (g_probeKey <= 0 || g_probeKey > 0xFF) g_probeKey = 0x48;
    g_probeCall = GetPrivateProfileIntA("probe", "call", 0, "companion_selfheal.ini") != 0;
    g_callKey = GetPrivateProfileIntA("probe", "CallKey", 0x4A, IniPath);
    if (g_callKey <= 0 || g_callKey > 0xFF) g_callKey = 0x4A;
}

void onHudConnect(bool ok, const char* why) { if (g_logOn) KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "k2.hud: %s (%s)", ok ? "connected" : "NOT connected", why ? why : ""); }
void onConnect(bool ok, const char* why) { if (g_logOn) KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "portraitslots: %s (%s)", ok ? "connected" : "NOT connected", why ? why : ""); }

// P3: which of the 3 portrait slots belong to the HUD panel (slot - 0x1F34 - k*0xDE4 == hud).
void logSlotMap() {
    for (int i = 0; i < K2PS_MAX_SLOTS; ++i) {
        int32_t slot = 0, cr = 0;
        if (!g_ps.slotAt(i, &slot, &cr)) continue;
        int match = -1;
        for (int k = 0; k < 3; ++k) if ((uintptr_t)slot - SlotFromHud - (uintptr_t)k * SlotStride == g_hud) { match = k; break; }
        if (match >= 0) KP_LOG(chHud, "P3 slot[%d]=%08x creature=%08x hud=%08x k=%d", i, (unsigned)slot, (unsigned)cr, (unsigned)g_hud, match);
        else KP_LOG(chHud, "P3 slot[%d]=%08x creature=%08x hud=%08x k=none", i, (unsigned)slot, (unsigned)cr, (unsigned)g_hud);
    }
}

uint32_t serverIdOf(uint32_t clientId) {
    uint32_t mask = 0;
    if (!rd<uint32_t>(ServerIdMask, 0, &mask)) return 0;
    return clientId & ~mask;
}

struct Pick {
    int pick = -1, row = -1, heal = -1, count = 0, missing = 0, ncands = 0;
    uint32_t key = 0, clientItem = 0, srv = 0;
    uintptr_t itemPtr = 0;
    std::string name;
    int ti = 0, rep = 0, wis = 0, intel = 0;
};

int canUseStrict(uintptr_t server, uintptr_t itemPtr) { return reinterpret_cast<int(__thiscall*)(uintptr_t, uintptr_t, int, int)>(CanUseItem)(server, itemPtr, 0, 1); }

// Skills + candidate building + pickHealItem for one member. log=true emits the DUMP skills/cand lines (byte-identical to 0.1.0).
Pick analyzeMember(const party::Vitals& m, const std::vector<itemread::Entry>& items, bool log, int minCount = 1) {
    Pick pk;
    uint32_t stats = 0;
    int& ti = pk.ti; int& rep = pk.rep; int& wis = pk.wis; int& intel = pk.intel;
    if (m.server && rd<uint32_t>(m.server, Server_Stats, &stats) && stats) {
        uint8_t b = 0;
        if (rd<uint8_t>(stats, Stats_Skills + Skill_TreatInjury, &b)) ti = b;
        if (rd<uint8_t>(stats, Stats_Skills + Skill_Repair, &b)) rep = b;
        if (rd<uint8_t>(stats, Stats_Abilities + Abil_Wis, &b)) wis = b;
        if (rd<uint8_t>(stats, Stats_Abilities + Abil_Int, &b)) intel = b;
        if (log) KP_LOG(chDump, "  skills stats=%08x TreatInjury=%d Repair=%d WIS=%d INT=%d", (unsigned)stats, ti, rep, wis, intel);
    } else if (log) KP_LOGV(chDump, KPLOG_WARN, "  skills: stats pointer unreadable (server=%08x)", (unsigned)m.server);

    pk.missing = m.max - m.cur;
    heal::HealCand cands[MaxCands]; int idxOf[MaxCands]; uintptr_t ptrOf[MaxCands]; int n = 0;
    for (const itemread::Entry& e : items) {
        if (!(e.key & itemread::ItemKeyFlag)) continue;
        uint32_t clientItem = (uint32_t)e.key & ~(uint32_t)itemread::ItemKeyFlag, srv = 0;
        uintptr_t itemPtr = itemread::serverItemFor(clientItem, &srv);
        itemread::ItemProp p = itemread::readItemProp(itemPtr);
        int count = itemread::stackCount(e.key, e.name); if (count <= 0) count = 1;
        bool isSpell = p.count > 0 && p.type == 10;
        int row = isSpell ? p.row : -1;
        int est = row >= 0 ? heal::healEstimate(row, ti, rep, wis, intel) : -1;
        bool usable = isSpell && p.usable != 0;
        // R5 (a): only items the engine accepts for THIS member (strict). Skipped at full HP: strict refuses medpacs there (full-HP gate), the FullHp state must still resolve.
        int can = -1;
        if (usable && itemPtr && m.server && pk.missing > 0) { can = canUseStrict(m.server, itemPtr); usable = can != 0; }
        if (log) KP_LOG(chDump, "  cand \"%s\" key=%08x client=%08x server=%08x item=%08x props=%d type=%d row=%d heal=%d count=%d usable=%d can=%d", e.name.c_str(), (unsigned)e.key, (unsigned)clientItem, (unsigned)srv, (unsigned)itemPtr, p.count, p.type, row, est, count, (int)usable, can);
        if (n < MaxCands) { cands[n] = heal::HealCand{ e.key, row, est, count, 0, usable }; idxOf[n] = (int)(&e - &items[0]); ptrOf[n] = itemPtr; ++n; }
    }
    pk.ncands = n;
    pk.pick = heal::pickHealItem(pk.missing, cands, n, minCount);
    if (pk.pick >= 0) {
        const itemread::Entry& e = items[(size_t)idxOf[pk.pick]];
        pk.key = (uint32_t)e.key;
        pk.clientItem = (uint32_t)e.key & ~(uint32_t)itemread::ItemKeyFlag;
        pk.itemPtr = ptrOf[pk.pick];
        itemread::serverItemFor(pk.clientItem, &pk.srv);
        pk.name = e.name; pk.row = cands[pk.pick].row; pk.heal = cands[pk.pick].heal; pk.count = cands[pk.pick].count;
    }
    return pk;
}

void dumpMember(int idx, const party::Vitals& m, const std::vector<itemread::Entry>& items) {
    uint32_t cmd = 0, clientId = 0;
    bool haveCmd = m.server && rd<uint32_t>(m.server, Server_Commandable, &cmd);
    bool haveId = m.client && rd<uint32_t>(m.client, Client_ObjectId, &clientId);
    KP_LOG(chDump, "member %d client=%08x server=%08x ok=%d hp=%d/%d down=%d commandable(+0xEC)=%s%u", idx, (unsigned)m.client, (unsigned)m.server, (int)m.ok, m.cur, m.max, (int)m.down, haveCmd ? "" : "?", (unsigned)cmd);
    Pick pk = analyzeMember(m, items, true);
    if (pk.pick >= 0) {
        KP_LOG(chDump, "  pick missing=%d -> \"%s\" row=%d heal=%d count=%d", pk.missing, pk.name.c_str(), pk.row, pk.heal, pk.count);
        uint32_t selfServer = serverIdOf(clientId);
        KP_LOG(chDump, "  routeA (not called): ecx=%08x fn=%08x args(0,0,self=%08x,0,item=%08x,0,0,0) clientId=%08x idOk=%d", (unsigned)m.server, (unsigned)UseItemRouteA, (unsigned)selfServer, (unsigned)pk.srv, (unsigned)clientId, (int)haveId);
    } else KP_LOG(chDump, "  pick missing=%d -> none (%d candidates)", pk.missing, pk.ncands);

    heal::AutoCfg cfg; cfg.on = true;
    heal::AutoState st;
    bool alive = m.ok && m.cur > 0;
    bool want = heal::shouldAutoUse(cfg, m.cur, m.max, alive, haveCmd && cmd != 0, true, st, GetTickCount());
    KP_LOG(chDump, "  auto(on, %d%%, keep %d): shouldAutoUse=%d alive=%d commandable=%d", cfg.hpPct, cfg.keep, (int)want, (int)alive, (int)(haveCmd && cmd != 0));
}

void dumpAll() {
    saferead::beginScope();
    KP_LOG(chDump, "DUMP begin hud=%08x", (unsigned)g_hud);
    if (!g_hud) { KP_LOGV(chDump, KPLOG_WARN, "DUMP: no HUD panel seen yet"); return; }
    std::vector<itemread::Entry> items;
    int n = itemread::readGroup(g_hud, ItemGroup, &items);
    KP_LOG(chDump, "item group %d: %d entries", ItemGroup, n);
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    KP_LOG(chDump, "party members=%d", cnt);
    for (int i = 1; i < cnt; ++i) dumpMember(i, v[i], items);
    KP_LOG(chDump, "DUMP end");
}

// ---- gate: copy of hotbar gameplayHudActive; Common candidate (19b section D) ----
int rdInt(uintptr_t base, int off, int dflt = 0) { int v; return rd<int>(base, off, &v) ? v : dflt; }
bool isIgnored(uintptr_t panel) { return (rdInt(panel, Panel_Flags) & PanelIgnoredByInput) != 0; }
bool isWhitelisted(uintptr_t panel) {
    uintptr_t vt; if (!rd<uintptr_t>(panel, 0, &vt)) return false;
    for (uintptr_t w : PanelWhitelist) if (vt == w) return true;
    return false;
}
bool gameplayHudActive(uintptr_t hud) {
    uintptr_t mgr; if (!hud || !rd<uintptr_t>(hud, Hud_Manager, &mgr) || !mgr) return false;
    int modals = rdInt(mgr, Mgr_Modals), modalCount = rdInt(mgr, Mgr_ModalCount);
    if (modalCount < 0 || modalCount > 64) return false;
    for (int i = modalCount - 1; i >= 0; --i) {
        uintptr_t p; if (!rd<uintptr_t>((uintptr_t)modals, i * 4, &p) || !p) return false;
        if (!isIgnored(p)) return false;
    }
    int panels = rdInt(mgr, Mgr_Panels), panelCount = rdInt(mgr, Mgr_PanelCount);
    if (panelCount < 0 || panelCount > 256) return false;
    for (int i = panelCount - 1; i >= 0; --i) {
        uintptr_t p; if (!rd<uintptr_t>((uintptr_t)panels, i * 4, &p) || !p) return false;
        if (isIgnored(p)) continue;
        if (p == hud) return true;
        if (!isWhitelisted(p)) return false;
    }
    return false;
}

// Tag of a creature (CExoString {char*, len} at +0x18); false = unreadable or empty (the PC has an empty tag).
bool readCreatureTag(uintptr_t cr, char* out, int cap) {
    out[0] = 0;
    uint32_t str = 0, len = 0;
    if (!rd<uint32_t>(cr, Cr_Tag, &str) || !rd<uint32_t>(cr, Cr_Tag + 4, &len)) return false;
    if (!str || len == 0 || len >= static_cast<uint32_t>(cap) || !saferead::probeRead(SiteBase + 4, reinterpret_cast<const void*>(static_cast<uintptr_t>(str)), len)) return false;
    memcpy(out, reinterpret_cast<const void*>(static_cast<uintptr_t>(str)), len);
    out[len] = 0;
    return true;
}

bool menuInfo(const K2PsContext* ctx) { return ctx && ctx->struct_size >= offsetof(K2PsContext, inMenu) + sizeof(int32_t); }
bool gateOpen(const K2PsContext* ctx) { return menuInfo(ctx) && ctx->inMenu == 0 && gameplayHudActive(g_hud) && foreground(); }

// ---- per-member state ----
DWORD g_lastUseMs[3] = {};
shui::Pending g_pend[3];
uint32_t g_pendKey[3] = {};
heal::AutoState g_auto[3];

struct UseInfo { Pick pk; int hp = 0; uintptr_t server = 0; };

#define REFUSE(...) do { KP_LOGV(chCall, KPLOG_WARN, __VA_ARGS__); return; } while (0)
#define REFUSEF(...) do { KP_LOGV(chCall, KPLOG_WARN, __VA_ARGS__); return false; } while (0)

// THE only path to route A: gate -> per-member debounce -> commandable -> ids -> CanUseItem -> route A once. src = click|auto|key.
bool useNow(const K2PsContext* ctx, DWORD now, int mi, int minCount, const char* src, UseInfo* out = nullptr) {
    if (mi < 1 || mi > 2) REFUSEF("REFUSE bad member %d (%s)", mi, src);
    if (!g_hud) REFUSEF("REFUSE no hud (%s)", src);
    if (!menuInfo(ctx)) REFUSEF("REFUSE no menu info in K2PsContext (%s)", src);
    if (ctx->inMenu) REFUSEF("REFUSE in menu (ctx.inMenu=1) (%s)", src);
    if (!gameplayHudActive(g_hud)) REFUSEF("REFUSE hud gate not active (%s)", src);
    if (g_lastUseMs[mi] && now - g_lastUseMs[mi] < DebounceMs) REFUSEF("REFUSE debounce member=%d (%lu ms since the last use) (%s)", mi, (unsigned long)(now - g_lastUseMs[mi]), src);
    std::vector<itemread::Entry> items;
    itemread::readGroup(g_hud, ItemGroup, &items);
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    if (mi >= cnt) REFUSEF("REFUSE member %d not in party (members=%d) (%s)", mi, cnt, src);
    const party::Vitals& m = v[mi];
    Pick pk = analyzeMember(m, items, false, minCount);
    if (pk.pick < 0) REFUSEF("REFUSE no pick member=%d minCount=%d (%s)", mi, minCount, src);
    uint32_t cmd = 0, clientId = 0;
    if (!m.server) REFUSEF("REFUSE server=0 member=%d", mi);
    if (!rd<uint32_t>(m.server, Server_Commandable, &cmd) || !cmd) REFUSEF("REFUSE not commandable member=%d (+0xEC=%u)", mi, (unsigned)cmd);
    if (!m.client || !rd<uint32_t>(m.client, Client_ObjectId, &clientId)) REFUSEF("REFUSE ids: client object id unreadable member=%d", mi);
    uint32_t selfServerId = serverIdOf(clientId), itemServerId = pk.srv;
    if (!selfServerId || !itemServerId || !pk.itemPtr) REFUSEF("REFUSE ids self=%08x item=%08x itemPtr=%08x", (unsigned)selfServerId, (unsigned)itemServerId, (unsigned)pk.itemPtr);
    int can = canUseStrict(m.server, pk.itemPtr);
    if (can == 0) {
        // R5 probe: race short at stats+0xE0; base-item type (+0xAC) needs the 0x6D5200 lookup, not a GameAddr constant -> -1
        uint32_t st = 0; int16_t race = -1; int type = -1;
        if (rd<uint32_t>(m.server, Server_Stats, &st) && st) { int16_t r = 0; if (rd<int16_t>(st, Stats_Race, &r)) race = r; }
        KP_LOGV(chCall, KPLOG_WARN, "REFUSE canuse=0 member=%d item=%08x ptr=%08x name=\"%s\" type=%d race=%d", mi, (unsigned)itemServerId, (unsigned)pk.itemPtr, pk.name.c_str(), type, (int)race);
        return false;
    }
    g_lastUseMs[mi] = now;   // only real uses debounce
    KP_LOG(chCall, "USE member=%d item=%08x \"%s\" heal=%d hp=%d/%d src=%s", mi, (unsigned)itemServerId, pk.name.c_str(), pk.heal, m.cur, m.max, src);
    g_pend[mi] = shui::Pending(); g_pend[mi].on = true; g_pend[mi].since = now; g_pend[mi].preHp = m.cur; g_pend[mi].preStack = pk.count;
    g_pendKey[mi] = pk.key;
    g_auto[mi].lastUseMs = now; g_auto[mi].pending = true;
    if (out) { out->pk = pk; out->hp = m.cur; out->server = m.server; }
    reinterpret_cast<void(__thiscall*)(uintptr_t, int, int, uint32_t, int, uint32_t, int, int, int)>(UseItemRouteA)(m.server, 0, 0, selfServerId, 0, itemServerId, 0, 0, 0);
    KP_LOG(chCall, "CALL routeA returned member=%d", mi);
    return true;
}

// Ctrl+Shift+CallKey: useNow on the first companion that has a pick (opt-in, [probe] call=1), then the POST window.
void doCall(const K2PsContext* ctx, DWORD now) {
    if (now - g_lastCallMs < DebounceMs) REFUSE("REFUSE debounce (%lu ms since the last call)", (unsigned long)(now - g_lastCallMs));
    if (g_post.active) REFUSE("REFUSE post window active");
    g_lastCallMs = now;
    if (!g_hud) REFUSE("REFUSE no hud");
    std::vector<itemread::Entry> items;
    itemread::readGroup(g_hud, ItemGroup, &items);
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    int mi = -1; Pick pk;
    for (int i = 1; i < cnt && mi < 0; ++i) { pk = analyzeMember(v[i], items, false); if (pk.pick >= 0) mi = i; }
    if (mi < 0) REFUSE("REFUSE no member with a pick (members=%d)", cnt);
    UseInfo ui;
    if (!useNow(ctx, now, mi, 1, "key", &ui)) return;
    g_post = Post();
    g_post.member = mi; g_post.itemKey = ui.pk.key; g_post.preHp = ui.hp; g_post.preStack = ui.pk.count; g_post.server = ui.server;
    g_post.start = now; g_post.next = now + PostStepMs; g_post.active = true;
}

void doPost(DWORD now) {
    if (!g_post.active || now < g_post.next) return;
    DWORD t = now - g_post.start;
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    int hp = (g_post.member < cnt) ? v[g_post.member].cur : -1;
    std::vector<itemread::Entry> items;
    int stack = -1;
    if (g_hud && itemread::readGroup(g_hud, ItemGroup, &items) >= 0)
        for (const itemread::Entry& e : items) if ((uint32_t)e.key == g_post.itemKey) { stack = itemread::stackCount(e.key, e.name); if (stack <= 0) stack = 1; break; }
    uint32_t cmd = 0;
    rd<uint32_t>(g_post.server, Server_Commandable, &cmd);
    KP_LOG(chCall, "POST t=%dms hp=%d stack=%d cmd=%u", (int)t, hp, stack, (unsigned)cmd);
    g_post.next += PostStepMs;
    if (t >= PostMs) {
        int gone = stack < 0;
        KP_LOG(chCall, "RESULT hp%+d stack%+d (gone=%d)", hp - g_post.preHp, (gone ? 0 : stack) - g_post.preStack, gone);
        g_post.active = false;
    }
}

// ---- INI (companion_selfheal.ini): [defaults] / [companions] via SelfHealUi.h; everything else preserved ----
shui::CfgTable g_cfg;
std::string g_cfgText;
filewatch::ReloadPoll g_cfgPoll;

void loadCfg() {
    std::ifstream f(IniPath, std::ios::binary);
    g_cfgText.clear();
    if (f) { std::stringstream ss; ss << f.rdbuf(); g_cfgText = ss.str(); }
    g_cfg = shui::parseFile(g_cfgText);
    KP_LOG(chCfg, "CFG reload n=%d", (int)g_cfg.rows.size());
}

bool saveIni(const std::string& key, const shui::CompCfg& c) {
    std::string text = shui::serializeFile(g_cfg, g_cfgText);
    bool ok = filewatch::atomicWrite(IniPath, text, true);
    if (ok) { g_cfgText = text; filewatch::accept(g_cfgPoll, IniPath); }
    KP_LOG(chCfg, "CFG save tag=%s auto=%d hp=%d keep=%d ok=%d", key.c_str(), (int)c.autoOn, c.hpPct, c.keep, (int)ok);
    return ok;
}

// ---- embedded textures (cross + 8 ring frames) ----
// Copy of hotbar embedded-texture recipe; Common candidate (19b section D).
// A CAurTextureBasic registered under our name (like the engine's own "NormCubeMap") with the decoded pixels already in place and +0xE4
// "constructed" set, so the engine never asks the resource manager for it; the texture tick uploads it and frees the buffer.
namespace tex {
#include "SelfHealTextures.inc"
constexpr int TexBasicSize = 0xF4;
constexpr int Tex_Pixels = 0x44, Tex_MipBytes = 0x5C, Tex_W = 0x68, Tex_H = 0x6C, Tex_Bpp = 0x74, Tex_Format = 0x78, Tex_Gl = 0x58;
constexpr int Tex_Cube = 0xDF, Tex_Constructed = 0xE4, Tex_Missing = 0xE5, Tex_FromRes = 0xE7, Tex_Tpc = 0xE9;
constexpr int MaxDim = 256;
constexpr int DiscIdx = 1 + shui::kRingFrames;
constexpr int Count = 2 + shui::kRingFrames;   // csh_cross, csh_ring0..7, csh_disc
using NewFn = void*(__cdecl*)(unsigned);
using PtrListAddFn = void(__thiscall*)(void*, void*);
struct Emb { unsigned char* basic = nullptr; int w = 0, h = 0, heals = 0; };
Emb g_tex[Count]; bool g_texInit = false;
void* g_crossImg = nullptr; void* g_discImg = nullptr; void* g_ringImg[shui::kRingFrames] = {};
static_assert(sizeof kSelfHealTextures / sizeof kSelfHealTextures[0] == Count, "csh_cross + csh_ring0..7 + csh_disc");

int rdI(const unsigned char* b, int off) { int v = 0; return rd<int>(reinterpret_cast<uintptr_t>(b), off, &v) ? v : 0; }
uint8_t rdB(const unsigned char* b, int off) { uint8_t v = 0; return rd<uint8_t>(reinterpret_cast<uintptr_t>(b), off, &v) ? v : 0; }
uintptr_t rdP(const unsigned char* b, int off) { uintptr_t v = 0; return rd<uintptr_t>(reinterpret_cast<uintptr_t>(b), off, &v) ? v : 0; }

// Minimal TGA check: uncompressed true-colour (type 2), 32 bpp, no colour map, w,h in 1..MaxDim, size >= 18 + idlen + w*h*4.
bool tgaOk(const unsigned char* d, unsigned size, int* w, int* h, unsigned* off) {
    if (size < 18 || d[1] != 0 || d[2] != 2 || d[16] != 32) return false;
    int ww = d[12] | d[13] << 8, hh = d[14] | d[15] << 8; unsigned o = 18u + d[0];
    if (ww <= 0 || hh <= 0 || ww > MaxDim || hh > MaxDim) return false;
    if ((uint64_t)o + (uint64_t)ww * (uint64_t)hh * 4u > (uint64_t)size) return false;
    *w = ww; *h = hh; *off = o;
    return true;
}

bool fillTexture(int i, const char* why) {
    const SelfHealEmbeddedTexture& src = kSelfHealTextures[i]; Emb& t = g_tex[i];
    int w = 0, h = 0; unsigned off = 0;
    if (!tgaOk(src.data, src.size, &w, &h, &off)) { KP_LOGV(chUi, KPLOG_WARN, "TEX %s: embedded TGA invalid (not 32-bit type 2, truncated or > %d px), skipped", src.resref, MaxDim); return false; }
    unsigned n = (unsigned)(w * h * 4);
    unsigned char* px = static_cast<unsigned char*>(reinterpret_cast<NewFn>(gameaddr::EngineNewArr)(n));
    if (!px) { KP_LOGV(chUi, KPLOG_WARN, "TEX %s: engine new[](%u) failed", src.resref, n); return false; }
    for (unsigned j = 0; j < n; j += 4) { px[j] = src.data[off + j + 2]; px[j + 1] = src.data[off + j + 1]; px[j + 2] = src.data[off + j]; px[j + 3] = src.data[off + j + 3]; }   // BGRA -> RGBA, rows as stored (bottom-up, = CResTGA)
    unsigned char* b = t.basic;
    *reinterpret_cast<unsigned char**>(b + Tex_Pixels) = px;
    *reinterpret_cast<int*>(b + Tex_W) = w; *reinterpret_cast<int*>(b + Tex_H) = h; *reinterpret_cast<int*>(b + Tex_Bpp) = 4;
    *reinterpret_cast<int*>(b + Tex_Format) = 0; *reinterpret_cast<int*>(b + Tex_MipBytes) = 0;
    b[Tex_Cube] = 0; b[Tex_FromRes] = 0; b[Tex_Tpc] = 0; b[Tex_Missing] = 0; b[Tex_Constructed] = 1;
    reinterpret_cast<PtrListAddFn>(gameaddr::TexListAdd)(reinterpret_cast<void*>(gameaddr::TexPendingList), b);
    t.w = w; t.h = h;
    KP_LOG(chUi, "TEX %s %s (%dx%d)", src.resref, why, w, h);
    return true;
}
void ensureTextures() {   // once per process; the textures outlive every HUD (never freed)
    if (g_texInit) return;
    g_texInit = true;
    for (int i = 0; i < Count; ++i) {
        unsigned char* b = static_cast<unsigned char*>(reinterpret_cast<NewFn>(gameaddr::New)(TexBasicSize));
        if (!b) { KP_LOGV(chUi, KPLOG_WARN, "TEX %s: allocation failed", kSelfHealTextures[i].resref); continue; }
        memset(b, 0, TexBasicSize);
        reinterpret_cast<void*(__thiscall*)(void*, const char*, const char*)>(gameaddr::TexBasicCtor)(b, kSelfHealTextures[i].resref, kSelfHealTextures[i].resref);
        g_tex[i].basic = b;
        if (fillTexture(i, "queued")) {
            void* im = guikit::imageFor(kSelfHealTextures[i].resref);   // a pooled GUI handle that is never released keeps the texture alive
            if (i == 0) g_crossImg = im; else if (i == DiscIdx) g_discImg = im; else g_ringImg[i - 1] = im;
        }
    }
}
// A GL reset (texture ResetAll / SetMem) Resets every registered texture (+0xE4 = 0, pixels and GL names freed) and
// re-queues all of them on the work list. If we see it first, refill (the duplicate list entry is a no-op: +0xE4 = 1, +0x44 = NULL after
// the first build). If the engine rebuilt first, its resource lookup missed our name (+0xE5 = 1): Reset and refill. DB lesson 290.
void healTextures() {
    for (int i = 0; i < Count; ++i) {
        Emb& t = g_tex[i]; unsigned char* b = t.basic;
        if (!b) continue;
        bool built = rdB(b, Tex_Constructed) != 0; uintptr_t px = rdP(b, Tex_Pixels); int w = rdI(b, Tex_W);
        if (built && !px && !rdB(b, Tex_Missing) && rdI(b, Tex_Gl)) { t.heals = 0; continue; }   // uploaded: healthy
        if (t.heals >= 20) continue;   // give up after 20 consecutive failed refills (art missing, labels still work)
        if (!built && !px) { ++t.heals; fillTexture(i, "refilled"); }
        else if (built && !px && (rdB(b, Tex_Missing) || w != t.w)) {
            ++t.heals;
            reinterpret_cast<void(__fastcall*)(void*)>(gameaddr::TexReset)(b);
            fillTexture(i, "refilled");
        }
        if (t.heals == 20) KP_LOGV(chUi, KPLOG_WARN, "TEX %s giving up after 20 failed refills (built=%d missing=%d pixels=%08x); art missing, labels still work", kSelfHealTextures[i].resref, (int)rdB(b, Tex_Constructed), (int)rdB(b, Tex_Missing), (unsigned)rdP(b, Tex_Pixels));   // crossed once: the guard above skips at >= 20
    }
}
}  // namespace tex

// ---- widgets ----
struct W {
    uintptr_t c = 0;
    shui::Rect r{ 0, 0, 0, 0 };
    bool rectSet = false;
    int vis = -1, clk = -1;
    std::string txt;
    float alpha = -1.f;
    float tr = -1.f, tg = -1.f, tb = -1.f;   // last tint / text colour sent
    int frame = -1;                          // ring: last texture frame sent
    bool filled = false, fb = false;         // cross fill applied / fell back to the "+" text
    std::string tip;
    bool hover = false;
};
enum { W_Disc0 = 0, W_Disc1, W_Btn0, W_Btn1, W_PopBg, W_Auto, W_HpM, W_HpP, W_KeepM, W_KeepP, W_HpVal, W_KeepVal, W_Ring0, W_Ring1 };   // discs first (under the buttons), rings last: appended after the buttons, drawn on top
W g_w[NWidgets];

void wRect(W& w, const shui::Rect& r) { if (!w.c || (w.rectSet && w.r == r)) return; if (guikit::setRect(w.c, r.x, r.y, r.w, r.h)) { w.r = r; w.rectSet = true; } }
bool isPopText(const W& w) { int i = static_cast<int>(&w - g_w); return i == W_Auto || i == W_HpM || i == W_HpP || i == W_KeepM || i == W_KeepP || i == W_HpVal || i == W_KeepVal; }
void popStyle(W& w) { if (!w.c || !isPopText(w)) return; guikit::setTextColor(w.c, 0.85f, 0.95f, 0.85f); guikit::setAlpha(w.c, 1.0f); w.alpha = 1.0f; w.tr = 0.85f; w.tg = 0.95f; w.tb = 0.85f; }   // popover text was invisible (CS8 R-pop): explicit light colour + alpha 1 on build and every show
void wShow(W& w, bool on) {
    if (!w.c) return;
    if (on) { if (w.vis != 1) popStyle(w); if (w.clk != 1) { guikit::setClickable(w.c, true); w.clk = 1; } if (w.vis != 1) { guikit::setVisible(w.c, true); w.vis = 1; } }
    else { if (w.clk != 0) { guikit::setClickable(w.c, false); w.clk = 0; } if (w.vis != 0) { guikit::setVisible(w.c, false); w.vis = 0; } w.hover = false; w.alpha = -1.f; w.tr = w.tg = w.tb = -1.f; w.frame = -1; w.filled = false; if (isPopText(w)) w.txt.clear(); }   // popover text re-applied on the next show (button "+"/"" state is tracked separately)
}
void wVis(W& w, bool on) { if (!w.c || w.vis == (on ? 1 : 0)) return; guikit::setVisible(w.c, on); w.vis = on ? 1 : 0; }   // visibility only: rings are NEVER clickable
void wText(W& w, const std::string& t) { if (!w.c || w.txt == t) return; if (guikit::setText(w.c, t.c_str())) w.txt = t; }
bool g_iconFbLogged[3] = {};
void wAlpha(W& w, float a) { if (w.alpha != a) { guikit::setAlpha(w.c, a); w.alpha = a; } }
// Button look: the cross image tinted rgb (change-gated); if the image is missing or setFill is refused, the "+" text coloured via setTextColor.
void wIcon(W& w, int member, const shui::Look& L) {
    if (!w.c) return;
    bool colourChanged = w.tr != L.r || w.tg != L.g || w.tb != L.b;
    if (!w.fb && (colourChanged || !w.filled)) {
        float rgb[3] = { L.r, L.g, L.b };
        if (tex::g_crossImg && guikit::setFill(w.c, tex::g_crossImg, rgb)) { w.filled = true; w.tr = L.r; w.tg = L.g; w.tb = L.b; wText(w, ""); }
        else { w.fb = true; if (!g_iconFbLogged[member]) { g_iconFbLogged[member] = true; KP_LOGV(chUi, KPLOG_WARN, "ICON fallback member=%d (%s)", member, tex::g_crossImg ? "setFill refused" : "no cross image"); } colourChanged = true; }
    }
    if (w.fb && colourChanged) { wText(w, "+"); guikit::setTextColor(w.c, L.r, L.g, L.b); w.tr = L.r; w.tg = L.g; w.tb = L.b; }
    wAlpha(w, L.alpha);
}
// Ring look: texture frame (change-gated); false = no image for this frame (caller hides the ring).
bool wRing(W& w, const shui::Look& L) {
    if (!w.c) return false;
    void* img = tex::g_ringImg[L.ringFrame];
    if (!img) return false;
    if (w.frame != L.ringFrame) { float rgb[3] = { L.r, L.g, L.b }; if (!guikit::setFill(w.c, img, rgb)) return false; w.frame = L.ringFrame; }
    return true;
}
// Disc backdrop: csh_disc untinted (change-gated). False = no image / fill refused (caller hides the disc).
bool wDisc(W& w) {
    if (!w.c || !tex::g_discImg) return false;
    if (!w.filled) { float rgb[3] = { 1.f, 1.f, 1.f }; if (!guikit::setFill(w.c, tex::g_discImg, rgb)) return false; w.filled = true; }
    return true;
}
void wTip(W& w, const std::string& t) { if (!w.c || w.tip == t || !guitip::nativeSupported()) return; if (guitip::setNative(w.c, t.c_str())) w.tip = t; }

int g_popMi = 0;                       // member whose popover is open (0 = closed)
guikit::ClickEdge g_lmb, g_rmb;
DWORD g_cacheMs = 0, g_cfgCheckMs = 0;
bool g_cacheValid = false;
bool g_mapLogged[3] = {}, g_warnTag[3] = {};

struct Card {
    bool have = false;
    party::Vitals m;
    Pick pk, apk;
    std::string tag;
    shui::CompCfg cfg;
    int k = -1;
    bool seen = false;                 // matched to a slot this frame
    shui::Rect vit{ 0, 0, 0, 0 };
    shui::Rect btn{ 0, 0, 0, 0 };
    shui::BtnState st = shui::BtnState::Hidden;
};
Card g_card[3];

std::string keyFor(int mi) {
    if (!g_card[mi].tag.empty()) return g_card[mi].tag;
    if (!g_warnTag[mi]) {
        uintptr_t sv = g_card[mi].m.server; char hex[64] = {}; int hn = 0;   // CS8 investigation: raw bytes of the CExoString field {char*, len, ...} at server+0x18
        for (int i = 0; i < 16; ++i) { uint8_t b = 0; bool ok = sv && rd<uint8_t>(sv, Cr_Tag + i, &b); hn += snprintf(hex + hn, sizeof hex - hn, ok ? "%02x" : "??", b); }
        uint32_t tp = 0; rd<uint32_t>(sv, Cr_Tag, &tp);
        KP_LOG(chCfg, "TAG member=%d raw=%s ptr=%08x server=%08x", mi, hex, (unsigned)tp, (unsigned)sv);
    }
    if (!g_warnTag[mi]) { g_warnTag[mi] = true; KP_LOGV(chCfg, KPLOG_WARN, "WARN member=%d has an empty tag, INI key slot%d", mi, g_card[mi].k >= 0 ? g_card[mi].k : mi); }
    return "slot" + std::to_string(g_card[mi].k >= 0 ? g_card[mi].k : mi);
}

// Screen rects (engine px) of the currently shown selfheal widgets; read by the RMB hook (same main thread). R6.
shui::Rect g_shown[8]; int g_nShown = 0;
void hideAll() {
    g_nShown = 0;
    for (W& w : g_w) wShow(w, false);
    g_popMi = 0;
    for (Card& c : g_card) { c.seen = false; c.st = shui::BtnState::Hidden; }
}

// pending tracker: HP up / stack down / timeout -> DONE (runs every frame while a use is pending, also in menus)
void pendingTick(DWORD now) {
    bool any = false;
    for (int i = 1; i <= 2; ++i) any = any || g_pend[i].on;
    if (!any || !g_hud) return;
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    std::vector<itemread::Entry> items;
    bool haveItems = itemread::readGroup(g_hud, ItemGroup, &items) >= 0;
    for (int i = 1; i <= 2; ++i) {
        if (!g_pend[i].on) continue;
        int hp = i < cnt ? v[i].cur : g_pend[i].preHp;
        int stack = -1;
        if (haveItems) { for (const itemread::Entry& e : items) if ((uint32_t)e.key == g_pendKey[i]) { stack = itemread::stackCount(e.key, e.name); if (stack <= 0) stack = 1; break; } }
        else stack = g_pend[i].preStack;   // group unreadable: no stack evidence, wait for HP or the timeout
        if (!shui::pendingDone(g_pend[i], now, hp, stack)) continue;
        KP_LOG(chCall, "DONE member=%d hp%+d stack%+d timeout=%d", i, hp - g_pend[i].preHp, (stack < 0 ? 0 : stack) - g_pend[i].preStack, (int)shui::pendingTimedOut(g_pend[i], now));
        g_pend[i].on = false; g_auto[i].pending = false;
    }
}

void refreshCache(const K2PsContext* ctx, DWORD now) {
    std::vector<itemread::Entry> items;
    itemread::readGroup(g_hud, ItemGroup, &items);
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    for (int i = 1; i <= 2; ++i) {
        Card& c = g_card[i];
        c.have = i < cnt && v[i].ok;
        if (!c.have) { c.pk = Pick(); c.apk = Pick(); continue; }
        c.m = v[i];
        char tag[64]; readCreatureTag(c.m.server, tag, sizeof tag);
        c.tag = tag;
        c.cfg = shui::lookup(g_cfg, keyFor(i));
        c.pk = analyzeMember(c.m, items, false, 1);
        c.apk = c.cfg.autoOn ? analyzeMember(c.m, items, false, c.cfg.keep + 1) : Pick();
        // CS5 auto: same route A as a click (useNow), at most every 250 ms (this cache cadence).
        if (c.cfg.autoOn && c.apk.pick >= 0) {
            heal::AutoCfg ac; ac.on = true; ac.hpPct = c.cfg.hpPct; ac.keep = c.cfg.keep;
            uint32_t cmd = 0;
            bool haveCmd = rd<uint32_t>(c.m.server, Server_Commandable, &cmd);
            bool alive = c.m.ok && c.m.cur > 0;
            if (heal::shouldAutoUse(ac, c.m.cur, c.m.max, alive, haveCmd && cmd != 0, g_card[i].seen, g_auto[i], now)) {
                KP_LOG(chCall, "AUTO member=%d hp=%d/%d pick=\"%s\" keep=%d", i, c.m.cur, c.m.max, c.apk.name.c_str(), c.cfg.keep);
                if (!useNow(ctx, now, i, c.cfg.keep + 1, "auto")) g_auto[i].lastUseMs = now;   // refused: back off one cooldown, no log spam
            }
        }
    }
    g_cacheMs = now; g_cacheValid = true;
}

std::string tipFor(const Card& c) {
    char b[160];
    switch (c.st) {
        case shui::BtnState::FullHp: return "Full health";
        case shui::BtnState::NoItem: return "No healing item";
        case shui::BtnState::AutoOn: snprintf(b, sizeof b, "Auto %d%% keep %d", c.cfg.hpPct, c.cfg.keep); return b;
        default: snprintf(b, sizeof b, "Heal: %s (+%d)", c.pk.name.c_str(), c.pk.heal); return b;
    }
}

// One-shot DIAG after the first popover show: what the engine holds for each text item (CS8 invisible-text probe).
bool g_popDiagDone = false;
void popDiag(bool popOpen) {
    if (g_popDiagDone || !popOpen) return;
    g_popDiagDone = true;
    const int items[7] = { W_Auto, W_HpM, W_HpP, W_KeepM, W_KeepP, W_HpVal, W_KeepVal };
    for (int i = 0; i < 7; ++i) {
        W& w = g_w[items[i]]; float rgb[3] = { -1.f, -1.f, -1.f }, a = -1.f;
        bool gc = w.c && guikit::getTextColor(w.c, rgb), ga = w.c && guikit::getAlpha(w.c, &a);
        KP_LOG(chUi, "DIAG pop item=%d rgb=%.2f,%.2f,%.2f alpha=%.2f txt=\"%s\" (rgbOk=%d alphaOk=%d)", items[i], rgb[0], rgb[1], rgb[2], a, w.txt.c_str(), (int)gc, (int)ga);
    }
}

void uiFrame(const K2PsContext* ctx, DWORD now) {
    // INI hot reload (1 s throttle inside changed())
    if (filewatch::changed(g_cfgPoll, IniPath, now)) loadCfg();
    pendingTick(now);
    tex::healTextures();   // every frame, before the gate: a GL reset can happen in menus too
    bool ledge = guikit::clickEdge(g_lmb), redge = guikit::rclickEdge(g_rmb);   // polled every frame so a held press never fires on reopen
    if (!gateOpen(ctx)) { hideAll(); return; }
    if (!g_cacheValid || now - g_cacheMs >= CacheMs) refreshCache(ctx, now);

    // slot -> card mapping + vit rects
    for (Card& c : g_card) c.seen = false;
    for (int i = 0; i < K2PS_MAX_SLOTS; ++i) {
        int32_t slot = 0, cr = 0;
        if (!g_ps.slotAt(i, &slot, &cr)) continue;
        int k = -1;
        for (int kk = 0; kk < 3; ++kk) if ((uintptr_t)slot - SlotFromHud - (uintptr_t)kk * SlotStride == g_hud) { k = kk; break; }
        if (k < 0) continue;
        int vr[4] = {};
        bool ok = true;
        for (int j = 0; j < 4; ++j) ok = rd<int>((uintptr_t)slot, Slot_VitRect + 4 * j, &vr[j]) && ok;
        if (!ok) continue;
        shui::Rect vit{ vr[0], vr[1], vr[2], vr[3] };
        bool isLeader = shui::isLeaderVit(vit);   // was `vit.w < 70` (inverted: live small-card vits are 47 wide, doc 30 finding)
        for (int mi = 1; mi <= 2; ++mi) {
            Card& c = g_card[mi];
            if (!c.have) continue;
            bool byClient = c.m.client && (uint32_t)cr == (uint32_t)c.m.client, byServer = c.m.server && (uint32_t)cr == (uint32_t)c.m.server;
            if (!byClient && !byServer) continue;
            c.k = k; c.vit = vit; c.seen = !isLeader;
            if (!g_mapLogged[mi]) { g_mapLogged[mi] = true; KP_LOG(chUi, "MAP member=%d slot=%d k=%d via=%s vit=[%d,%d,%d,%d] leader=%d", mi, i, k, byClient ? "client" : "server", vit.x, vit.y, vit.w, vit.h, (int)isLeader); }
        }
    }
    int screenW = 0;   // popover right clamp; 0 = unknown (no clamp)
    if (guikit::g_gk.screenWAddr) guikit::detail::rdInt(guikit::g_gk.screenWAddr, 0, &screenW);

    // button state + look
    for (int mi = 1; mi <= 2; ++mi) {
        Card& c = g_card[mi];
        W& b = g_w[W_Btn0 + mi - 1];
        c.btn = c.seen ? shui::buttonRect(c.vit) : shui::Rect{ 0, 0, 0, 0 };
        c.st = shui::resolveState(c.have && c.seen, false, c.m.cur, c.m.max, c.pk.pick >= 0, g_pend[mi].on, c.cfg.autoOn);
        if (c.btn.w <= 0) c.st = shui::BtnState::Hidden;
        W& ring = g_w[W_Ring0 + mi - 1];
        W& disc = g_w[W_Disc0 + mi - 1];
        if (c.st == shui::BtnState::Hidden) { wShow(b, false); wShow(ring, false); wShow(disc, false); if (g_popMi == mi) g_popMi = 0; continue; }
        wRect(b, c.btn);
        wRect(disc, shui::ringRect(c.btn));
        wVis(disc, wDisc(disc));
        const shui::Look L = shui::lookFor(c.st, c.cfg.autoOn, now);
        wIcon(b, mi, L);
        if (L.ring) {
            wRect(ring, shui::ringRect(c.btn));
            if (wRing(ring, L)) wVis(ring, true); else wVis(ring, false);
        } else wVis(ring, false);
        wTip(b, tipFor(c));
        wShow(b, true);
    }

    // input
    int cx = 0, cy = 0;
    bool haveCur = guikit::cursorEngine(&cx, &cy);
    int onBtn = 0;
    if (haveCur) {
        for (int mi = 1; mi <= 2; ++mi) {
            W& b = g_w[W_Btn0 + mi - 1];
            bool h = g_card[mi].st != shui::BtnState::Hidden && guikit::hit(b.c, cx, cy);
            if (h) onBtn = mi;
            if (h && !b.hover) guikit::playHoverSound();
            b.hover = h;
        }
    }
    shui::PopLayout L{};
    bool popOpen = g_popMi != 0 && g_card[g_popMi].btn.w > 0;
    if (!popOpen) g_popMi = 0;
    if (popOpen) L = shui::popoverLayout(g_card[g_popMi].btn, screenW);
    bool inPop = popOpen && haveCur && cx >= L.r[0].x && cx < L.r[0].x + L.r[0].w && cy >= L.r[0].y && cy < L.r[0].y + L.r[0].h;
    if (haveCur && ledge) {
        if (inPop) {   // popover click: edit config only, NEVER useNow
            shui::PopItem it = shui::popoverHit(L, cx, cy);
            if (it != shui::PopItem::None) {
                if (filewatch::changed(g_cfgPoll, IniPath, now, true)) loadCfg();   // pick up a hand edit before overwriting
                std::string key = keyFor(g_popMi);
                shui::CompCfg nc = shui::applyPop(shui::lookup(g_cfg, key), it);
                shui::upsert(g_cfg, key, nc);
                g_card[g_popMi].cfg = nc;
                saveIni(key, nc);
            }
        } else if (onBtn) {
            Card& c = g_card[onBtn];
            if (shui::clickAllowed(c.st)) {
                if (c.st == shui::BtnState::NoItem) KP_LOG(chCall, "CLICK member=%d noitem", onBtn);
                else useNow(ctx, now, onBtn, shui::clickMinCount(c.st), "click");
            }
        } else if (popOpen) g_popMi = 0;
    } else if (haveCur && redge) {
        if (onBtn) g_popMi = (g_popMi == onBtn) ? 0 : onBtn;
        else if (popOpen && !inPop) g_popMi = 0;
    }

    // popover widgets
    popOpen = g_popMi != 0;
    if (popOpen) {
        L = shui::popoverLayout(g_card[g_popMi].btn, screenW);
        const shui::CompCfg& cfg = g_card[g_popMi].cfg;
        wRect(g_w[W_PopBg], L.r[0]);
        const int items[5] = { W_Auto, W_HpM, W_HpP, W_KeepM, W_KeepP };
        for (int i = 0; i < 5; ++i) { wRect(g_w[items[i]], L.r[i + 1]); wText(g_w[items[i]], shui::popLabel(static_cast<shui::PopItem>(i + 1), cfg)); }
        wRect(g_w[W_HpVal], shui::popValueRect(L, 2)); wText(g_w[W_HpVal], shui::popValueLabel(2, cfg));
        wRect(g_w[W_KeepVal], shui::popValueRect(L, 3)); wText(g_w[W_KeepVal], shui::popValueLabel(3, cfg));
    }
    for (int i = W_PopBg; i < W_Ring0; ++i) wShow(g_w[i], popOpen);
    popDiag(popOpen);
    int ns = 0;   // rects the RMB camera hook must not turn into a camera drag: buttons, rings, popover bg
    for (int mi = 1; mi <= 2; ++mi) if (g_card[mi].st != shui::BtnState::Hidden && g_card[mi].btn.w > 0) { g_shown[ns++] = g_card[mi].btn; g_shown[ns++] = shui::ringRect(g_card[mi].btn); }   // disc and ring share ringRect (outer extent)
    if (popOpen) g_shown[ns++] = L.r[0];
    g_nShown = ns;
}

void KPATCH_CALL onFrameBegin(const K2PsContext* ctx, void*) {
    if (!g_hud) return;
    saferead::beginScope();
    DWORD now = GetTickCount();
    if (g_logOn) {
        if (now - g_lastPollMs >= 2000) { g_lastPollMs = now; logSlotMap(); }
        bool down = foreground() && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000) && (GetAsyncKeyState(g_probeKey) & 0x8000);
        if (down && !g_keyPrev) dumpAll();
        g_keyPrev = down;
        if (g_probeCall) {
            bool cdown = foreground() && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000) && (GetAsyncKeyState(g_callKey) & 0x8000);
            if (cdown && !g_callPrev) doCall(ctx, now);
            g_callPrev = cdown;
            doPost(now);
        }
    }
    uiFrame(ctx, now);
}

// RAII: ProjectionFlag cleared for the label binds, restored on EVERY path (incl. failures).
struct ProjGuard {
    int* flag; int saved = 0; bool cleared = false;
    ProjGuard() : flag(reinterpret_cast<int*>(gameaddr::ProjectionFlag)) {
        bool readOk = saferead::probeRead(SiteBase + 1, flag, 4);
        saved = readOk ? *flag : 0;
        if (saved) { if (saferead::probeWrite(SiteBase + 2, flag, 4)) { *flag = 0; cleared = true; } else KP_LOGV(chHud, KPLOG_WARN, "BUILD: projection flag not writable, labels bind with the aspect projection"); }
    }
    ~ProjGuard() { if (cleared) { if (saferead::probeWrite(SiteBase + 3, flag, 4)) *flag = saved; else KP_LOGV(chHud, KPLOG_WARN, "BUILD: projection flag restore failed (saved=%d)", saved); } }
};

// Creates one hidden + click-through label; null on failure (only that widget is disabled).
uintptr_t makeLabel(uintptr_t panel, const char* tmpl, const char* text, bool font, int* whyOut) {
    int why = guikit::CF_None; uint32_t badVt = 0;
    uintptr_t c = guikit::createLabelFromTemplate(panel, tmpl, &why, &badVt);
    *whyOut = why;
    if (!c) return 0;
    if (font) { guikit::setFont(c, "dialogfont10x10"); guikit::setAlign(c, 18); }
    if (text) guikit::setText(c, text);
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (!guikit::appendToPanel(panel, c)) {   // no destroy helper in GuiKit: the control stays hidden + click-through and unreferenced
        *whyOut = guikit::CF_Count;
        KP_LOGV(chHud, KPLOG_WARN, "BUILD append failed, control %08x left hidden + click-through", (unsigned)c);
        return 0;
    }
    return c;
}

// 12 widgets while the HUD layout is still open (before StopLoadFromLayout).
void buildWidgets(uintptr_t panel) {
    for (W& w : g_w) w = W();
    tex::ensureTextures();
    g_popMi = 0; g_lmb = guikit::ClickEdge(); g_rmb = guikit::ClickEdge();
    for (Card& c : g_card) c.seen = false;
    ProjGuard pg;
    int n = 0;
    std::string whys;
    auto add = [&](int idx, const char* tmpl, const char* text, bool font, const char* name) {
        int why = 0;
        uintptr_t c = makeLabel(panel, tmpl, text, font, &why);
        if (!c && !strcmp(tmpl, "LBL_MOULDING3")) { c = makeLabel(panel, "LBL_QUEUE1", text, font, &why); if (c) whys += std::string(name) + ":moulding3 fallback ";}
        g_w[idx].c = c; g_w[idx].txt = text ? text : "";
        if ((idx >= W_Ring0 || idx <= W_Disc1) && c) { g_w[idx].vis = 0; g_w[idx].clk = 0; }   // discs + rings start hidden + click-through and stay non-clickable
        if (c) ++n; else whys += std::string(name) + ":" + std::to_string(why) + " ";
    };
    add(W_Disc0, "LBL_QUEUE1", nullptr, false, "disc0");   // before the buttons: z-order disc < button < ring
    add(W_Disc1, "LBL_QUEUE1", nullptr, false, "disc1");
    add(W_Btn0, "LBL_QUEUE1", "+", true, "btn0");   // "+" = the fallback glyph until the cross fill is applied
    add(W_Btn1, "LBL_QUEUE1", "+", true, "btn1");
    add(W_PopBg, "LBL_MOULDING3", nullptr, false, "popbg");
    add(W_Auto, "LBL_QUEUE1", "Auto", true, "auto");
    add(W_HpM, "LBL_QUEUE1", "-", true, "hpm");
    add(W_HpP, "LBL_QUEUE1", "+", true, "hpp");
    add(W_KeepM, "LBL_QUEUE1", "-", true, "keepm");
    add(W_KeepP, "LBL_QUEUE1", "+", true, "keepp");
    add(W_HpVal, "LBL_QUEUE1", "", true, "hpval");
    add(W_KeepVal, "LBL_QUEUE1", "", true, "keepval");
    add(W_Ring0, "LBL_QUEUE1", nullptr, false, "ring0");   // after the buttons: drawn on top
    add(W_Ring1, "LBL_QUEUE1", nullptr, false, "ring1");
    for (W& w : g_w) popStyle(w);
    KP_LOGV(chHud, n == NWidgets ? KPLOG_INFO : KPLOG_WARN, "BUILD n=%d/%d why=%s", n, NWidgets, whys.empty() ? "ok" : whys.c_str());
}

// K2HUD_BUILD callback (hud-host, source 0x0040F5A0, HUD panel only, layout still open). The vtable check is a cheap guard.
void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel)) return;
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    saferead::beginScope();
    g_ps.poll();
    uint32_t vt = 0;
    if (!panel || !rd<uint32_t>(panel, 0, &vt) || vt != HudVtable) return;
    g_hud = panel;
    buildWidgets(panel);
    if (!g_logOn) return;
    uint32_t f30 = 0, f48 = 0, ctl = 0, cnt = 0;
    rd<uint32_t>(panel, Panel_Open, &f30); rd<uint32_t>(panel, Panel_Flags, &f48); rd<uint32_t>(panel, Panel_Ctl, &ctl); rd<uint32_t>(panel, Panel_CtlCount, &cnt);
    bool layoutOpen = f30 != 0 && (f48 & 2);
    KP_LOG(chHud, "P1 hud built panel=%08x +0x30=%08x +0x48=%08x layoutOpen=%d ctlList=%08x ctlCount=%u", (unsigned)panel, (unsigned)f30, (unsigned)f48, (int)layoutOpen, (unsigned)ctl, (unsigned)cnt);
}

} // namespace

extern "C" {

// R6: detour at gameaddr::RButtonDownHoldSite (PerformRButtonDownAction 0x7B3810), skip_original_bytes: replaces MOV [RmbHoldFlag],1.
// Vanilla (write 1) unless the cursor is over a shown selfheal rect, so RMB on our buttons/popover never drags the camera.
// TODO(CS8): when Mouse Look (options bit1) is set the flag semantics invert (0078c350 XORs it); the options object is not exposed in GameAddr, so that case is not handled (ini = 0 here).
void __cdecl SelfHealRmbHold() {
    int cx = 0, cy = 0;
    if (g_nShown > 0 && guikit::cursorEngine(&cx, &cy)) {
        for (int i = 0; i < g_nShown; ++i) if (shui::detail::inRect(g_shown[i], cx, cy)) return;
    }
    *reinterpret_cast<volatile int*>(gameaddr::RmbHoldFlag) = 1;
}

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    loadIni();
    loadCfg();
    filewatch::accept(g_cfgPoll, IniPath);
    KP_LOG(chMain, "0.5.0-dev: probe call=%d CallKey=0x%02X cfg rows=%d", g_probeCall, g_callKey, (int)g_cfg.rows.size());
    g_ps.init(api, "companion-selfheal");
    g_ps.onConnect = onConnect;
    g_ps.add(K2PS_FRAME_BEGIN, 300, onFrameBegin);
    g_ps.connect();
    g_hudc.init(api, "companion-selfheal");
    g_hudc.onConnect = onHudConnect;
    g_hudc.add(K2HUD_BUILD, 200, onHudBuild);
    g_hudc.connect();
}

}
