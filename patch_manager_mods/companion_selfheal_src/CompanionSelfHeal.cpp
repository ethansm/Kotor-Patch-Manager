// Companion Self-Heal 0.3.0-dev (doc 27, CS4 + CS5). The HUD gets, from ONE hook (StopLoadFromLayout 0x0040F5A0, HUD vtable only), 10 runtime
// labels (2 "H" buttons, a popover background, 5 popover items, 2 value labels). Per frame (K2PS_FRAME_BEGIN, only in gameplay HUD):
// buttons follow the companion cards (slot+0x694 vit rect); LMB = use the picked healing item now (route A), RMB = popover
// (Auto on/off, use-below %, keep reserve; persisted to companion_selfheal.ini); Auto mode uses items by itself (HealItems.h rules).
// Route A is reachable ONLY through useNow(): HUD gate + commandable (+0xEC) + CanUseItem + per-member 2 s debounce. The popover never calls it.
// Engine writes: label setters + the ProjectionFlag clear/restore around the creates. Everything else is SafeRead.
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
constexpr int Stats_Skills = 0x174, Stats_Abilities = 0xED, Skill_Repair = 5, Skill_TreatInjury = 7, Abil_Int = 3, Abil_Wis = 4;
constexpr int ItemGroup = 1;                              // HUD item group
constexpr int MaxCands = 64;
constexpr int Slot_VitRect = 0x694;                       // K2Addr Slot_VitRect: 4 ints x,y,w,h (screen px)
constexpr int Hud_Manager = 0x1C, Mgr_Panels = 0x88, Mgr_PanelCount = 0x8C, Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;
constexpr int Cr_Tag = 0x18, PanelIgnoredByInput = 0x600;
constexpr uintptr_t PanelWhitelist[] = { 0x0099E5EC, 0x009A044C, 0x009A7BAC, 0x009A8FCC };
constexpr int NWidgets = 10;
constexpr DWORD CacheMs = 250;
constexpr DWORD DebounceMs = 2000, PostMs = 10000, PostStepMs = 500;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chHud, "hud");
KP_CHANNEL(chDump, "dump");
KP_CHANNEL(chCall, "call");
KP_CHANNEL(chUi, "ui");
KP_CHANNEL(chCfg, "cfg");

k2ps::Client g_ps;
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
        if (log) KP_LOG(chDump, "  cand \"%s\" key=%08x client=%08x server=%08x item=%08x props=%d type=%d row=%d heal=%d count=%d usable=%d", e.name.c_str(), (unsigned)e.key, (unsigned)clientItem, (unsigned)srv, (unsigned)itemPtr, p.count, p.type, row, est, count, (int)usable);
        if (n < MaxCands) { cands[n] = heal::HealCand{ e.key, row, est, count, 0, usable }; idxOf[n] = (int)(&e - &items[0]); ptrOf[n] = itemPtr; ++n; }
    }
    pk.ncands = n;
    pk.missing = m.max - m.cur;
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
    g_lastUseMs[mi] = now;
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
    int can = reinterpret_cast<int(__thiscall*)(uintptr_t, uintptr_t, int, int)>(CanUseItem)(m.server, pk.itemPtr, 0, 1);
    if (can == 0) REFUSEF("REFUSE canuse=0 member=%d item=%08x", mi, (unsigned)itemServerId);
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

// ---- widgets ----
struct W {
    uintptr_t c = 0;
    shui::Rect r{ 0, 0, 0, 0 };
    bool rectSet = false;
    int vis = -1, clk = -1;
    std::string txt;
    int look = -1;
    float alpha = -1.f;
    std::string tip;
    bool hover = false;
};
enum { W_Btn0 = 0, W_Btn1, W_PopBg, W_Auto, W_HpM, W_HpP, W_KeepM, W_KeepP, W_HpVal, W_KeepVal };
W g_w[NWidgets];

void wRect(W& w, const shui::Rect& r) { if (!w.c || (w.rectSet && w.r == r)) return; if (guikit::setRect(w.c, r.x, r.y, r.w, r.h)) { w.r = r; w.rectSet = true; } }
void wShow(W& w, bool on) {
    if (!w.c) return;
    if (on) { if (w.clk != 1) { guikit::setClickable(w.c, true); w.clk = 1; } if (w.vis != 1) { guikit::setVisible(w.c, true); w.vis = 1; } }
    else { if (w.clk != 0) { guikit::setClickable(w.c, false); w.clk = 0; } if (w.vis != 0) { guikit::setVisible(w.c, false); w.vis = 0; } w.hover = false; }
}
void wText(W& w, const std::string& t) { if (!w.c || w.txt == t) return; if (guikit::setText(w.c, t.c_str())) w.txt = t; }
void wLook(W& w, int look, float alpha, float r, float g, float b) {
    if (!w.c) return;
    if (w.look != look) { float rgb[3]; if (guikit::getTextColor(w.c, rgb)) guikit::setTextColor(w.c, r, g, b); w.look = look; }
    if (w.alpha != alpha) { guikit::setAlpha(w.c, alpha); w.alpha = alpha; }
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
    if (!g_warnTag[mi]) { g_warnTag[mi] = true; KP_LOGV(chCfg, KPLOG_WARN, "WARN member=%d has an empty tag, INI key slot%d", mi, g_card[mi].k >= 0 ? g_card[mi].k : mi); }
    return "slot" + std::to_string(g_card[mi].k >= 0 ? g_card[mi].k : mi);
}

void hideAll() {
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

void uiFrame(const K2PsContext* ctx, DWORD now) {
    // INI hot reload (1 s throttle inside changed())
    if (filewatch::changed(g_cfgPoll, IniPath, now)) loadCfg();
    pendingTick(now);
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
        bool isLeader = vit.w < 70;
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
        if (c.st == shui::BtnState::Hidden) { wShow(b, false); if (g_popMi == mi) g_popMi = 0; continue; }
        wRect(b, c.btn);
        switch (c.st) {
            case shui::BtnState::FullHp: wLook(b, 1, 0.4f, 1.f, 1.f, 1.f); break;
            case shui::BtnState::NoItem: wLook(b, 2, 0.6f, 1.f, 0.3f, 0.3f); break;
            case shui::BtnState::Pending: wLook(b, ((now / 250) % 2) ? 3 : 4, ((now / 250) % 2) ? 1.0f : 0.5f, 1.f, 1.f, 1.f); break;
            case shui::BtnState::AutoOn: wLook(b, 5, 1.0f, 0.3f, 1.f, 0.3f); break;
            default: wLook(b, 0, 1.0f, 1.f, 1.f, 1.f); break;
        }
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
    for (int i = W_PopBg; i < NWidgets; ++i) wShow(g_w[i], popOpen);
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

// 10 widgets while the HUD layout is still open (before StopLoadFromLayout).
void buildWidgets(uintptr_t panel) {
    for (W& w : g_w) w = W();
    g_popMi = 0; g_lmb = guikit::ClickEdge(); g_rmb = guikit::ClickEdge();
    for (Card& c : g_card) c.seen = false;
    ProjGuard pg;
    int n = 0;
    std::string whys;
    auto add = [&](int idx, const char* tmpl, const char* text, bool font, const char* name) {
        int why = 0;
        uintptr_t c = makeLabel(panel, tmpl, text, font, &why);
        if (!c && !strcmp(tmpl, "LBL_MOULDING3")) { c = makeLabel(panel, "LBL_QUEUE1", text, font, &why); if (c) whys += std::string(name) + ":moulding3 fallback ";}
        g_w[idx].c = c;
        if (c) ++n; else whys += std::string(name) + ":" + std::to_string(why) + " ";
    };
    add(W_Btn0, "LBL_QUEUE1", "H", true, "btn0");
    add(W_Btn1, "LBL_QUEUE1", "H", true, "btn1");
    add(W_PopBg, "LBL_MOULDING3", nullptr, false, "popbg");
    add(W_Auto, "LBL_QUEUE1", "Auto", true, "auto");
    add(W_HpM, "LBL_QUEUE1", "-", true, "hpm");
    add(W_HpP, "LBL_QUEUE1", "+", true, "hpp");
    add(W_KeepM, "LBL_QUEUE1", "-", true, "keepm");
    add(W_KeepP, "LBL_QUEUE1", "+", true, "keepp");
    add(W_HpVal, "LBL_QUEUE1", "", true, "hpval");
    add(W_KeepVal, "LBL_QUEUE1", "", true, "keepval");
    KP_LOGV(chHud, n == NWidgets ? KPLOG_INFO : KPLOG_WARN, "BUILD n=%d/%d why=%s", n, NWidgets, whys.empty() ? "ok" : whys.c_str());
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    loadIni();
    loadCfg();
    filewatch::accept(g_cfgPoll, IniPath);
    KP_LOG(chMain, "0.3.0-dev: probe call=%d CallKey=0x%02X cfg rows=%d", g_probeCall, g_callKey, (int)g_cfg.rows.size());
    g_ps.init(api, "companion-selfheal");
    g_ps.onConnect = onConnect;
    g_ps.add(K2PS_FRAME_BEGIN, 300, onFrameBegin);
    g_ps.connect();
}

// 0x0040F5A0 entry, ecx = panel. Only the HUD (gameaddr::HudVtable) is of interest; everything else returns at once.
void __cdecl SelfHealLayoutClosing(uintptr_t panel) {
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

}
