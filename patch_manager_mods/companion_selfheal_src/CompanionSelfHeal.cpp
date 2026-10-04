// Companion Self-Heal 0.1.0-probe -- LOG ONLY (doc 27, CS2). Never calls an engine function (the only lookups are the pure object-table
// reads inside itemread::serverItemFor, the same ones the action hotbar makes) and never writes engine memory.
// Logs: P1 (HUD panel found by the StopLoadFromLayout hook), P3 (portrait slot -> HUD mapping, every 2 s), DUMP (Ctrl+Shift+ProbeKey):
// per companion vitals / skills / healing-item candidates / pick / route A args / auto decision.
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

KP_LOG_PATCH("companion-selfheal");

namespace {

constexpr unsigned SiteBase = 0x6E00;                    // saferead site id
constexpr uintptr_t HudVtable = gameaddr::HudVtable;             // CSWGuiInGame (HUD) RTTI vtable
constexpr uintptr_t ServerIdMask = gameaddr::ServerIdMask;       // global: client id & ~[mask] = server id (ClientToServerObjectId)
constexpr uintptr_t UseItemRouteA = gameaddr::UseItemRouteA;     // logged only, never called
constexpr int Panel_Ctl = 0x24, Panel_CtlCount = 0x28, Panel_Open = 0x30, Panel_Flags = 0x48;
constexpr int SlotFromHud = 0x1F34, SlotStride = 0xDE4;  // portrait slot k = hud + 0x1F34 + k*0xDE4
constexpr int Client_ObjectId = 4;
constexpr int Server_Commandable = 0xEC, Server_Stats = 0x1198;
constexpr int Stats_Skills = 0x174, Stats_Abilities = 0xED, Skill_Repair = 5, Skill_TreatInjury = 7, Abil_Int = 3, Abil_Wis = 4;
constexpr int ItemGroup = 1;                              // HUD item group
constexpr int MaxCands = 64;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chHud, "hud");
KP_CHANNEL(chDump, "dump");

k2ps::Client g_ps;
uintptr_t g_hud = 0;
bool g_logOn = true;
int g_probeKey = 0x48;
DWORD g_lastPollMs = 0;
bool g_keyPrev = false;

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

void dumpMember(int idx, const party::Vitals& m, const std::vector<itemread::Entry>& items) {
    uint32_t cmd = 0, stats = 0, clientId = 0;
    bool haveCmd = m.server && rd<uint32_t>(m.server, Server_Commandable, &cmd);
    bool haveId = m.client && rd<uint32_t>(m.client, Client_ObjectId, &clientId);
    KP_LOG(chDump, "member %d client=%08x server=%08x ok=%d hp=%d/%d down=%d commandable(+0xEC)=%s%u", idx, (unsigned)m.client, (unsigned)m.server, (int)m.ok, m.cur, m.max, (int)m.down, haveCmd ? "" : "?", (unsigned)cmd);
    int ti = 0, rep = 0, wis = 0, intel = 0;
    if (m.server && rd<uint32_t>(m.server, Server_Stats, &stats) && stats) {
        uint8_t b = 0;
        if (rd<uint8_t>(stats, Stats_Skills + Skill_TreatInjury, &b)) ti = b;
        if (rd<uint8_t>(stats, Stats_Skills + Skill_Repair, &b)) rep = b;
        if (rd<uint8_t>(stats, Stats_Abilities + Abil_Wis, &b)) wis = b;
        if (rd<uint8_t>(stats, Stats_Abilities + Abil_Int, &b)) intel = b;
        KP_LOG(chDump, "  skills stats=%08x TreatInjury=%d Repair=%d WIS=%d INT=%d", (unsigned)stats, ti, rep, wis, intel);
    } else KP_LOGV(chDump, KPLOG_WARN, "  skills: stats pointer unreadable (server=%08x)", (unsigned)m.server);

    heal::HealCand cands[MaxCands]; int idxOf[MaxCands]; int n = 0;
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
        KP_LOG(chDump, "  cand \"%s\" key=%08x client=%08x server=%08x item=%08x props=%d type=%d row=%d heal=%d count=%d usable=%d", e.name.c_str(), (unsigned)e.key, (unsigned)clientItem, (unsigned)srv, (unsigned)itemPtr, p.count, p.type, row, est, count, (int)usable);
        if (n < MaxCands) { cands[n] = heal::HealCand{ e.key, row, est, count, 0, usable }; idxOf[n] = (int)(&e - &items[0]); ++n; }
    }
    int missing = m.max - m.cur;
    int pick = heal::pickHealItem(missing, cands, n, 1);
    if (pick >= 0) {
        const itemread::Entry& e = items[(size_t)idxOf[pick]];
        uint32_t clientItem = (uint32_t)e.key & ~(uint32_t)itemread::ItemKeyFlag, srv = 0;
        itemread::serverItemFor(clientItem, &srv);
        KP_LOG(chDump, "  pick missing=%d -> \"%s\" row=%d heal=%d count=%d", missing, e.name.c_str(), cands[pick].row, cands[pick].heal, cands[pick].count);
        uint32_t selfServer = serverIdOf(clientId);
        KP_LOG(chDump, "  routeA (not called): ecx=%08x fn=%08x args(0,0,self=%08x,0,item=%08x,0,0,0) clientId=%08x idOk=%d", (unsigned)m.server, (unsigned)UseItemRouteA, (unsigned)selfServer, (unsigned)srv, (unsigned)clientId, (int)haveId);
    } else KP_LOG(chDump, "  pick missing=%d -> none (%d candidates)", missing, n);

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

void KPATCH_CALL onFrameBegin(const K2PsContext*, void*) {
    if (!g_logOn || !g_hud) return;
    saferead::beginScope();
    DWORD now = GetTickCount();
    if (now - g_lastPollMs >= 2000) { g_lastPollMs = now; logSlotMap(); }
    bool down = foreground() && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_SHIFT) & 0x8000) && (GetAsyncKeyState(g_probeKey) & 0x8000);
    if (down && !g_keyPrev) dumpAll();
    g_keyPrev = down;
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    loadIni();
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
    if (!g_logOn) return;
    uint32_t f30 = 0, f48 = 0, ctl = 0, cnt = 0;
    rd<uint32_t>(panel, Panel_Open, &f30); rd<uint32_t>(panel, Panel_Flags, &f48); rd<uint32_t>(panel, Panel_Ctl, &ctl); rd<uint32_t>(panel, Panel_CtlCount, &cnt);
    bool layoutOpen = f30 != 0 && (f48 & 2);
    KP_LOG(chHud, "P1 hud built panel=%08x +0x30=%08x +0x48=%08x layoutOpen=%d ctlList=%08x ctlCount=%u", (unsigned)panel, (unsigned)f30, (unsigned)f48, (int)layoutOpen, (unsigned)ctl, (unsigned)cnt);
}

}
