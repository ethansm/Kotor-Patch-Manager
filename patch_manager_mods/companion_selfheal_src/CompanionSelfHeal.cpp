// Companion Self-Heal 0.2.0-probe (doc 27, CS3b). With the default INI it behaves exactly like 0.1.0: LOG ONLY, never calls an engine
// function (the only lookups are the pure object-table reads inside itemread::serverItemFor, the same ones the action hotbar makes)
// and never writes engine memory.
// Logs: P1 (HUD panel found by the StopLoadFromLayout hook), P3 (portrait slot -> HUD mapping, every 2 s), DUMP (Ctrl+Shift+ProbeKey):
// per companion vitals / skills / healing-item candidates / pick / route A args / auto decision.
// Opt-in (companion_selfheal.ini), both default 0:
//   [probe] call=1   Ctrl+Shift+CallKey (default 'J') makes ONE real call: CanUseItem, then UseItemRouteA(first non-leader member that has a
//                    pick, item -> self), 2 s debounce, refused outside gameplay; then logs POST hp/stack/commandable for 10 s + RESULT.
//   [probe] label=1  creates ONE test label (template LBL_QUEUE1, text "TEST") on the HUD panel while its layout is open (P2); P4/P5 are
//                    not implemented (logged once).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "KpLog.h"
#include "SafeRead.h"
#include "K2Addr.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "HealItems.h"
#include "ItemRead.h"
#include "GuiKit.h"

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
constexpr DWORD DebounceMs = 2000, PostMs = 10000, PostStepMs = 500;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chHud, "hud");
KP_CHANNEL(chDump, "dump");
KP_CHANNEL(chCall, "call");

k2ps::Client g_ps;
uintptr_t g_hud = 0;
bool g_logOn = true;
int g_probeKey = 0x48;
DWORD g_lastPollMs = 0;
bool g_keyPrev = false;
int g_probeCall = 0, g_probeLabel = 0, g_callKey = 0x4A;
bool g_callPrev = false;
DWORD g_lastCallMs = 0;
uintptr_t g_labelPanel = 0, g_label = 0;
bool g_p45Logged = false;
struct Post { bool active = false; DWORD start = 0, next = 0; int member = 0; uint32_t itemKey = 0; int preHp = 0, preStack = 0; uintptr_t server = 0; } g_post;

template <typename T> bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(SiteBase, base, off, out); }

bool foreground() {
    HWND w = GetForegroundWindow(); DWORD pid = 0;
    if (!w) return false;
    GetWindowThreadProcessId(w, &pid);
    return pid == GetCurrentProcessId();
}

void loadIni() {
    g_logOn = GetPrivateProfileIntA("General", "Log", 1, "companion_selfheal.ini") != 0;
    g_probeKey = GetPrivateProfileIntA("General", "ProbeKey", 0x48, "companion_selfheal.ini");
    if (g_probeKey <= 0 || g_probeKey > 0xFF) g_probeKey = 0x48;
    g_probeCall = GetPrivateProfileIntA("probe", "call", 0, "companion_selfheal.ini") != 0;
    g_probeLabel = GetPrivateProfileIntA("probe", "label", 0, "companion_selfheal.ini") != 0;
    g_callKey = GetPrivateProfileIntA("probe", "CallKey", 0x4A, "companion_selfheal.ini");
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
Pick analyzeMember(const party::Vitals& m, const std::vector<itemread::Entry>& items, bool log) {
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
    pk.pick = heal::pickHealItem(pk.missing, cands, n, 1);
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

#define REFUSE(...) do { KP_LOGV(chCall, KPLOG_WARN, __VA_ARGS__); return; } while (0)

// Ctrl+Shift+CallKey: one real CanUseItem + UseItemRouteA call (opt-in, [probe] call=1).
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
    const party::Vitals& m = v[mi];
    uint32_t cmd = 0, clientId = 0;
    if (!m.server) REFUSE("REFUSE server=0 member=%d", mi);
    if (!rd<uint32_t>(m.server, Server_Commandable, &cmd) || !cmd) REFUSE("REFUSE not commandable member=%d (+0xEC=%u)", mi, (unsigned)cmd);
    bool ctxMenu = ctx && ctx->struct_size >= offsetof(K2PsContext, inMenu) + sizeof(int32_t);
    if (!ctxMenu) REFUSE("REFUSE no menu info in K2PsContext");
    if (ctx->inMenu) REFUSE("REFUSE in menu (ctx.inMenu=1)");
    if (!m.client || !rd<uint32_t>(m.client, Client_ObjectId, &clientId)) REFUSE("REFUSE ids: client object id unreadable member=%d", mi);
    uint32_t selfServerId = serverIdOf(clientId), itemServerId = pk.srv;
    if (!selfServerId || !itemServerId || !pk.itemPtr) REFUSE("REFUSE ids self=%08x item=%08x itemPtr=%08x", (unsigned)selfServerId, (unsigned)itemServerId, (unsigned)pk.itemPtr);
    int can = reinterpret_cast<int(__thiscall*)(uintptr_t, uintptr_t, int, int)>(CanUseItem)(m.server, pk.itemPtr, 0, 1);
    if (can == 0) REFUSE("REFUSE canuse=0 member=%d item=%08x", mi, (unsigned)itemServerId);
    KP_LOG(chCall, "CALL routeA member=%d item=%08x pre hp=%d stack=%d", mi, (unsigned)itemServerId, m.cur, pk.count);
    g_post = Post();
    g_post.member = mi; g_post.itemKey = pk.key; g_post.preHp = m.cur; g_post.preStack = pk.count; g_post.server = m.server;
    g_post.start = now; g_post.next = now + PostStepMs; g_post.active = true;
    reinterpret_cast<void(__thiscall*)(uintptr_t, int, int, uint32_t, int, uint32_t, int, int, int)>(UseItemRouteA)(m.server, 0, 0, selfServerId, 0, itemServerId, 0, 0, 0);
    KP_LOG(chCall, "CALL routeA returned member=%d", g_post.member);
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
        KP_LOG(chCall, "RESULT hp%+d stack%+d", hp - g_post.preHp, stack - g_post.preStack);
        g_post.active = false;
    }
}

void KPATCH_CALL onFrameBegin(const K2PsContext* ctx, void*) {
    if (!g_logOn || !g_hud) return;
    saferead::beginScope();
    DWORD now = GetTickCount();
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
    if (g_label && !g_p45Logged) { g_p45Logged = true; KP_LOG(chHud, "P4/P5 not implemented in 0.2.0"); }
}

// P2: one test label from the HUD's LBL_QUEUE1 template, created while the HUD layout is still open (before StopLoadFromLayout).
void createTestLabel(uintptr_t panel) {
    if (panel == g_labelPanel) return;
    g_labelPanel = panel; g_label = 0; g_p45Logged = false;
    int* projFlag = reinterpret_cast<int*>(gameaddr::ProjectionFlag);
    bool flagRead = saferead::probeRead(SiteBase + 1, projFlag, 4);
    int saved = flagRead ? *projFlag : 0;
    bool cleared = false;
    if (saved) { if (saferead::probeWrite(SiteBase + 2, projFlag, 4)) { *projFlag = 0; cleared = true; } else KP_LOGV(chHud, KPLOG_WARN, "P2: projection flag not writable, label binds with the aspect projection"); }
    int why = guikit::CF_None; uint32_t badVt = 0;
    uintptr_t c = guikit::createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &badVt);
    if (c) {
        guikit::setText(c, "TEST");
        guikit::setVisible(c, true);
        if (guikit::appendToPanel(panel, c)) {
            g_label = c;
            int x = 0, y = 0, w = 0, h = 0; guikit::getExtent(c, &x, &y, &w, &h);
            KP_LOG(chHud, "P2 label=%08x why=%d extent=[%d,%d,%d,%d] (template rect, not repositioned)", (unsigned)c, why, x, y, w, h);
        } else { why = guikit::CF_Count; KP_LOGV(chHud, KPLOG_WARN, "P2 label=%08x why=%d (append to the panel list failed, not used)", (unsigned)c, why); }
    } else KP_LOGV(chHud, KPLOG_WARN, "P2 label=%08x why=%d badVt=%08x", 0u, why, (unsigned)badVt);
    if (cleared) { if (saferead::probeWrite(SiteBase + 3, projFlag, 4)) *projFlag = saved; else KP_LOGV(chHud, KPLOG_WARN, "P2: projection flag restore failed (saved=%d)", saved); }
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    loadIni();
    KP_LOG(chMain, "0.2.0-probe: probe call=%d label=%d CallKey=0x%02X", g_probeCall, g_probeLabel, g_callKey);
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
    if (g_probeLabel) createTestLabel(panel);
    if (!g_logOn) return;
    uint32_t f30 = 0, f48 = 0, ctl = 0, cnt = 0;
    rd<uint32_t>(panel, Panel_Open, &f30); rd<uint32_t>(panel, Panel_Flags, &f48); rd<uint32_t>(panel, Panel_Ctl, &ctl); rd<uint32_t>(panel, Panel_CtlCount, &cnt);
    bool layoutOpen = f30 != 0 && (f48 & 2);
    KP_LOG(chHud, "P1 hud built panel=%08x +0x30=%08x +0x48=%08x layoutOpen=%d ctlList=%08x ctlCount=%u", (unsigned)panel, (unsigned)f30, (unsigned)f48, (int)layoutOpen, (unsigned)ctl, (unsigned)cnt);
}

}
