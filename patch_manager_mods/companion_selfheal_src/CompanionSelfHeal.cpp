// Companion Self-Heal 0.7.0-dev (doc 27, CS4 restyle + CS5 + CS8/CS9; doc 24 CS10 strip slot + CS11 kit frame). The HUD gets, from hud-host's K2HUD_BUILD phase (k2.hud@1, source 0x0040F5A0, HUD panel only; the only own hook is the RMB camera swallow 0x7B3817), 14 runtime
// labels (2 dark discs under the buttons, 2 cross-icon buttons, a popover background, 5 popover items, 2 value labels, 2 gold auto rings drawn on top; the cross, disc + ring art are
// embedded textures, see namespace tex / EmbedTex.h; fallback = the text "+"). Per frame (K2PS_FRAME_BEGIN, only in gameplay HUD):
// buttons follow the companion cards (slot+0x694 vit rect); LMB = use the picked healing item now (route A), RMB = popover
// (Auto on/off, use-below %, keep reserve; persisted to companion_selfheal.ini); Auto mode uses items by itself (HealItems.h rules).
// Route A is reachable ONLY through useNow(): HUD gate + commandable (+0xEC) + CanUseItem + per-member 2 s debounce. The popover never calls it.
// Engine writes: label setters + the ProjectionFlag clear/restore around the creates + our own texture objects. Everything else is SafeRead.
// Logs: P1/P3 (hook, slot map), LEADER gone/back (leader party entry lost/regained), DUMP (Ctrl+Shift+ProbeKey), BUILD, MAP, CLICK/USE/AUTO/DONE/REFUSE, CFG, KICK, QDIAG (queue trace, INI [queue] trace=0 to silence).
// CS12 (0.6.0-dev): a follower's queue is [0x3D FollowLeader] which re-adds itself at the head, so the 0x3F item use appended by route A starves.
// kickTick() rotates the queue (pop head + append tail, k times) when the R8 gates hold; useNow() refuses a second use while one is queued; the pending
// timeout only runs over a not-queued stretch. INI [queue] kick=1 (0 = no rotate). Never ClearAllActions, never 0x5648D0.
// Opt-in (companion_selfheal.ini [probe] call=1): Ctrl+Shift+CallKey (default 'J') = useNow(first companion with a pick, "key") + POST/RESULT window.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
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
#include "ActQueue.h"
#include "EmbedTex.h"
#include "HudKit.h"
#include "TextFit.h"

KP_LOG_PATCH("companion-selfheal");

namespace {

constexpr unsigned SiteBase = 0x6E00;                    // saferead site id (+0..4)
constexpr unsigned SiteQueue = 0x6E10, SiteRound = 0x6E20, SitePause = 0x6E40, SiteKick = 0x6E50;   // CS12: actq queue reads +0..5, round reads +0..12, pause chain +0..3, kick gate reads +0..3
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
constexpr int Slot_CardRect = 0x54C;                      // portrait card rect (screen px)
constexpr int Slot_VitRect = 0x694;                       // K2Addr Slot_VitRect: 4 ints x,y,w,h (screen px)
constexpr int Hud_Manager = 0x1C, Mgr_Panels = 0x88, Mgr_PanelCount = 0x8C, Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;
constexpr int Cr_Tag = 0x18, PanelIgnoredByInput = 0x600, GateLogCap = 64;
// Panels that do not block gameplay hotkeys (RTTI vtables): messages, fade, pause, auto-pause
constexpr uintptr_t PanelWhitelist[] = { gameaddr::MessagesPanelVtable, gameaddr::FadePanelVtable, gameaddr::PausePanelVtable, gameaddr::AutoPausePanelVtable };
constexpr DWORD CacheMs = 250;
constexpr DWORD DebounceMs = 2000, PostMs = 10000, PostStepMs = 500;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chHud, "hud");
KP_CHANNEL(chDump, "dump");
KP_CHANNEL(chCall, "call");
KP_CHANNEL(chUi, "ui");
KP_CHANNEL(chCfg, "cfg");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("companion-selfheal_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

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
bool g_kick = true;   // [queue] kick=1 (CS12 rotate); 0 = 0.5.0 behaviour (dedupe + pendingDone2 stay)
bool g_queueTrace = true;   // [queue] trace=1 (code default): QDIAG lines (own INFO lines, edge-triggered); 0 = off
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
    g_logOn = GetPrivateProfileIntA("General", "Log", 1, IniPath) != 0;
    g_probeKey = GetPrivateProfileIntA("General", "ProbeKey", 0x48, IniPath);
    if (g_probeKey <= 0 || g_probeKey > 0xFF) g_probeKey = 0x48;
    g_probeCall = GetPrivateProfileIntA("probe", "call", 0, IniPath) != 0;
    g_kick = GetPrivateProfileIntA("queue", "kick", 1, IniPath) != 0;
    g_queueTrace = GetPrivateProfileIntA("queue", "trace", 1, IniPath) != 0;
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
// Diagnostic only: why the gate is open/closed. Filled by gateEval, logged on change by gateLog (never per frame).
struct GateInfo { bool open = false; const char* why = "other"; uintptr_t hud = 0, top = 0; uint32_t f48 = 0, topF48 = 0; bool f48Bad = false; };
// A genuine panel flags dword is small (0x45/0x47/0x145); the first HUD build of a process can carry stale heap garbage (0xFF8FB4xx) that keeps bit 0x400 set forever.
bool hudFlagsPlausible(uint32_t f48) { return (f48 >> 12) == 0; }
void gateLog(const GateInfo& g) {
    static GateInfo last; static bool have = false; static int lines = 0;
    if (have && last.open == g.open && strcmp(last.why, g.why) == 0 && last.hud == g.hud && last.f48 == g.f48 && last.f48Bad == g.f48Bad && last.top == g.top && last.topF48 == g.topF48) return;
    last = g; have = true;
    if (lines > GateLogCap) return;
    if (lines == GateLogCap) { ++lines; KP_LOG(chHud, "GATE cap reached"); return; }
    if (g.f48Bad) { ++lines; KP_LOG(chHud, "GATE hud flags implausible f48=%08x -> treated as not ignored", (unsigned)g.f48); if (lines >= GateLogCap) return; }
    ++lines;
    KP_LOG(chHud, "GATE open=%d why=%s hud=%08x f48=%08x bits=%03x top=%08x topf48=%08x", (int)g.open, g.why, (unsigned)g.hud, (unsigned)g.f48, (unsigned)(g.f48 & PanelIgnoredByInput), (unsigned)g.top, (unsigned)g.topF48);
}
bool gateEval(uintptr_t hud, GateInfo& g) {
    g = GateInfo(); g.hud = hud;
    if (!hud) { g.why = "no-hud"; return false; }
    g.f48 = (uint32_t)rdInt(hud, Panel_Flags);
    uintptr_t mgr; if (!rd<uintptr_t>(hud, Hud_Manager, &mgr) || !mgr) return false;
    int modals = rdInt(mgr, Mgr_Modals), modalCount = rdInt(mgr, Mgr_ModalCount);
    if (modalCount < 0 || modalCount > 64) return false;
    for (int i = modalCount - 1; i >= 0; --i) {
        uintptr_t p; if (!rd<uintptr_t>((uintptr_t)modals, i * 4, &p) || !p) return false;
        if (!isIgnored(p)) { g.why = "modal-open"; g.top = p; g.topF48 = (uint32_t)rdInt(p, Panel_Flags); return false; }
    }
    int panels = rdInt(mgr, Mgr_Panels), panelCount = rdInt(mgr, Mgr_PanelCount);
    if (panelCount < 0 || panelCount > 256) return false;
    bool sawHudIgnored = false;
    for (int i = panelCount - 1; i >= 0; --i) {
        uintptr_t p; if (!rd<uintptr_t>((uintptr_t)panels, i * 4, &p) || !p) return false;
        if (p == hud && !hudFlagsPlausible(g.f48)) g.f48Bad = true;   // garbage flags dword: do not trust the ignore bits (read-only; no engine write)
        else if (isIgnored(p)) { if (p == hud) sawHudIgnored = true; continue; }
        if (!g.top) { g.top = p; g.topF48 = (uint32_t)rdInt(p, Panel_Flags); }   // first non-ignored panel from the top
        if (p == hud) { g.why = "open"; g.open = true; return true; }
        if (!isWhitelisted(p)) { g.why = "other-panel-on-top"; g.top = p; g.topF48 = (uint32_t)rdInt(p, Panel_Flags); return false; }
    }
    g.why = sawHudIgnored ? "hud-ignored" : "not-found";
    return false;
}
bool gameplayHudActive(uintptr_t hud) {
    GateInfo g; bool r = gateEval(hud, g);
    gateLog(g);
    return r;
}

// Tag of a creature (CExoString {char*, len} at +0x18); false = unreadable (the PC has an empty tag, recognized as "player").
bool readCreatureTag(uintptr_t cr, char* out, int cap) {
    out[0] = 0;
    if (!cr || cap < 1) return false;
    uint32_t str = 0, len = 0;
    if (!rd<uint32_t>(cr, Cr_Tag, &str) || !rd<uint32_t>(cr, Cr_Tag + 4, &len)) return false;
    if (!str || len == 0) {
        // Recognize the Exile / player character (empty tag when leader control is switched to a follower)
        if (cap > 6) {
            strncpy(out, "player", cap);
            out[cap - 1] = 0;
            return true;
        }
        return false;
    }
    if (len >= static_cast<uint32_t>(cap) || !saferead::probeRead(SiteBase + 4, reinterpret_cast<const void*>(static_cast<uintptr_t>(str)), len)) return false;
    memcpy(out, reinterpret_cast<const void*>(static_cast<uintptr_t>(str)), len);
    out[len] = 0;
    return true;
}

bool menuInfo(const K2PsContext* ctx) { return ctx && ctx->struct_size >= offsetof(K2PsContext, inMenu) + sizeof(int32_t); }
bool gateOpen(const K2PsContext* ctx) { return menuInfo(ctx) && ctx->inMenu == 0 && gameplayHudActive(g_hud) && foreground(); }

// ---- per-member state ----
DWORD g_lastUseMs[3] = {};
shui::Pending g_pend[3];
uint32_t g_unqueuedSince[3] = {};   // pendingDone2 not-queued stretch start (0 = not running); zeroed when pending is armed
// kick state per member (CS12); reset when a pending is armed or ends
int g_kickReason[3] = {};           // last logged skip gate (0 = none) so the log is edge-triggered
DWORD g_starvedSince[3] = {};       // start of the current starved + unpaused stretch (0 = none)
bool g_starveLogged[3] = {};        // QDIAG edge flag: starved onset logged; its cleared edge logs and resets it; a pause keeps it; arm + DONE reset it
bool g_rotNoopLogged[3] = {};       // QDIAG one-shot per pending: no-op rotate logged (reset on arm + DONE)
bool g_readFail[3] = {};            // QDIAG edge flag: kick actq::read failing for this member
const char* g_kickGate[3] = {};     // gate name of the last kickSkip (for the give-up QDIAG line)
bool g_kickGaveUp[3] = {};
uint32_t g_pendKey[3] = {};
heal::AutoState g_auto[3];
// Member 0 (leader) state
shui::Pending g_pend0;
uint32_t g_unqueuedSince0 = 0;
int g_kickReason0 = 0;
DWORD g_starvedSince0 = 0;
bool g_kickGaveUp0 = false;
uint32_t g_pendKey0 = 0;
heal::AutoState g_auto0;

struct UseInfo { Pick pk; int hp = 0; uintptr_t server = 0; };

#define REFUSE(...) do { KP_LOGV(chCall, KPLOG_WARN, __VA_ARGS__); return; } while (0)
#define REFUSEF(...) do { KP_LOGV(chCall, KPLOG_WARN, __VA_ARGS__); return false; } while (0)

// QDIAG (trace only; callers check g_queueTrace). Own INFO lines, never appended to REFUSE/KICK/USE/DONE/TIMEOUT.
// Types: hex, at most 8 shown, then ",+N" for the rest.
void qdiagTypes(const actq::Snapshot& q, char* buf, size_t cap) {
    size_t n = 0; buf[0] = 0;
    for (int j = 0; j < q.ne && j < 8 && n + 8 < cap; ++j) n += snprintf(buf + n, cap - n, "%s%x", j ? "," : "", q.e[j].type & 0xFF);
    if (q.ne > 8) snprintf(buf + n, cap - n, ",+%d", q.ne - 8);
}
void qdiagSnap(const char* tag, int mi, const actq::Snapshot& q, bool paused) {
    char ty[64];
    qdiagTypes(q, ty, sizeof ty);
    KP_LOG(chCall, "QDIAG %s member=%d queue=%d types=[%s] rounds=%d paused=%d", tag, mi, q.ne, ty, q.nrn, (int)paused);
}
// The hud-gate refuse runs before the member lookup, so read the member here (pure reads).
void qdiagHudRefuse(int mi) {
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    if (mi < 0 || mi >= cnt || !v[mi].server) { KP_LOG(chCall, "QDIAG hud-gate member=%d no server (members=%d)", mi, cnt); return; }
    actq::Snapshot q;
    actq::read(v[mi].server, &q, SiteQueue, SiteRound);
    qdiagSnap("hud-gate", mi, q, actq::playerPaused(SitePause));
}

// THE only path to route A: gate -> per-member debounce -> commandable -> ids -> CanUseItem -> route A once. src = click|auto|key.
bool useNow(const K2PsContext* ctx, DWORD now, int mi, int minCount, const char* src, UseInfo* out = nullptr) {
    if (mi < 0 || mi > 2) REFUSEF("REFUSE bad member %d (%s)", mi, src);
    if (!g_hud) REFUSEF("REFUSE no hud (%s)", src);
    if (!menuInfo(ctx)) REFUSEF("REFUSE no menu info in K2PsContext (%s)", src);
    if (ctx->inMenu) REFUSEF("REFUSE in menu (ctx.inMenu=1) (%s)", src);
    if (!gameplayHudActive(g_hud)) { if (g_queueTrace) qdiagHudRefuse(mi); REFUSEF("REFUSE hud gate not active (%s)", src); }
    if (mi >= 0 && mi <= 2 && g_lastUseMs[mi] && now - g_lastUseMs[mi] < DebounceMs) REFUSEF("REFUSE debounce member=%d (%lu ms since the last use) (%s)", mi, (unsigned long)(now - g_lastUseMs[mi]), src);
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
    {   // CS12 dedupe: a second use while one is still queued would stack behind the stalled first
        actq::Snapshot qs;
        if (!actq::read(m.server, &qs, SiteQueue, SiteRound) && g_queueTrace) KP_LOG(chCall, "QDIAG dedupe queue read failed member=%d", mi);
        shui::UseRefusal ur = shui::useAllowed(actq::useQueued(qs));
        if (ur != shui::UseRefusal::None) { if (g_queueTrace) qdiagSnap("dedupe", mi, qs, actq::playerPaused(SitePause)); REFUSEF("REFUSE %s member=%d (%s)", shui::useRefusalText(ur), mi, src); }
    }
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
    if (mi >= 0 && mi <= 2) g_lastUseMs[mi] = now;   // only real uses debounce
    KP_LOG(chCall, "USE member=%d item=%08x \"%s\" heal=%d hp=%d/%d src=%s", mi, (unsigned)itemServerId, pk.name.c_str(), pk.heal, m.cur, m.max, src);
    if (mi >= 1 && mi <= 2) { g_pend[mi] = shui::Pending(); g_pend[mi].on = true; g_pend[mi].since = now; g_pend[mi].preHp = m.cur; g_pend[mi].preStack = pk.count; g_unqueuedSince[mi] = 0; g_kickReason[mi] = 0; g_starvedSince[mi] = 0; g_kickGaveUp[mi] = false; g_pendKey[mi] = pk.key; g_auto[mi].lastUseMs = now; g_auto[mi].pending = true; g_starveLogged[mi] = false; g_rotNoopLogged[mi] = false; }
    else if (mi == 0) { g_pend0 = shui::Pending(); g_pend0.on = true; g_pend0.since = now; g_pend0.preHp = m.cur; g_pend0.preStack = pk.count; g_unqueuedSince0 = 0; g_kickReason0 = 0; g_starvedSince0 = 0; g_kickGaveUp0 = false; g_pendKey0 = pk.key; g_auto0.lastUseMs = now; g_auto0.pending = true; }
    if (out) { out->pk = pk; out->hp = m.cur; out->server = m.server; }
    if (g_queueTrace) {
        actq::Snapshot qb;
        bool qok = actq::read(m.server, &qb, SiteQueue, SiteRound);
        KP_LOG(chCall, "QDIAG routeA t=%lu member=%d queue=%d read=%d", (unsigned long)now, mi, qb.ne, (int)qok);
    }
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

// ---- embedded textures (cross + circular disc (normal/hover) + 8 ring frames) ----
// Copy of hotbar embedded-texture recipe; Common candidate (19b section D).
// A CAurTextureBasic registered under our name (like the engine's own "NormCubeMap") with the decoded pixels already in place and +0xE4
// "constructed" set, so the engine never asks the resource manager for it; the texture tick uploads it and frees the buffer.
namespace tex {
#include "SelfHealTextures.inc"
constexpr int Count = 7 + shui::kRingFrames;   // csh_cross, csh_disc, csh_disc_hi, csh_tog_off, csh_tog_off_hi, csh_tog_on, csh_tog_on_hi, csh_ring0..7
bool g_texInit = false;
void* g_crossImg = nullptr; void* g_discImg[2] = {}; void* g_togImg[4] = {}; void* g_ringImg[shui::kRingFrames] = {};   // g_togImg: off, off_hi, on, on_hi
void* g_arrowNeg = nullptr; void* g_arrowPos = nullptr;   // vanilla stepper glyphs uibit_arrowneg_p / uibit_arrowpos_p (engine textures, not embedded)
static_assert(sizeof kSelfHealTextures / sizeof kSelfHealTextures[0] == Count, "csh_cross + csh_disc + csh_disc_hi + 4 csh_tog_* + csh_ring0..7");

void logLine(const char* fmt, ...) {   // embtex::g_log sink
    char buf[256]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    KP_LOG(chUi, "%s", buf);
}
void ensureTextures() {   // once per process; the textures outlive every HUD (never freed)
    if (g_texInit) return;
    embtex::g_log = logLine;
    const bool ensured = embtex::ensure(kSelfHealTextures, Count);   // create + fill + queue (idempotent); heal() below refills after a GL reset
    g_texInit = ensured;   // not ready / partial: retried on the next HUD build (was set unconditionally, a not-ready first call left the discs null forever)
    for (int i = 0; i < Count; ++i) {
        void* im = guikit::imageFor(kSelfHealTextures[i].resref);   // a pooled GUI handle that is never released keeps the texture alive
        if (i == 0) g_crossImg = im; else if (i <= 2) g_discImg[i - 1] = im; else if (i <= 6) g_togImg[i - 3] = im; else g_ringImg[i - 7] = im;
    }
    g_arrowNeg = guikit::imageFor("uibit_arrowneg_p");   // vanilla stepper glyphs (r1.md); pooled, never released
    g_arrowPos = guikit::imageFor("uibit_arrowpos_p");
    KP_LOG(chUi, "TEXSET ensure=%d supported=%d ready=%d cross=%08x disc=%08x dischi=%08x tog=%08x,%08x,%08x,%08x ring0=%08x slots=%d/%d", (int)ensured, (int)embtex::supported(), (int)embtex::detail::engineReady(),
           (unsigned)reinterpret_cast<uintptr_t>(g_crossImg), (unsigned)reinterpret_cast<uintptr_t>(g_discImg[0]), (unsigned)reinterpret_cast<uintptr_t>(g_discImg[1]), (unsigned)reinterpret_cast<uintptr_t>(g_togImg[0]), (unsigned)reinterpret_cast<uintptr_t>(g_togImg[1]), (unsigned)reinterpret_cast<uintptr_t>(g_togImg[2]), (unsigned)reinterpret_cast<uintptr_t>(g_togImg[3]), (unsigned)reinterpret_cast<uintptr_t>(g_ringImg[0]), embtex::slotCount(), (int)embtex::MaxTex);
    KP_LOG(chUi, "ARROW neg=%08x pos=%08x", (unsigned)reinterpret_cast<uintptr_t>(g_arrowNeg), (unsigned)reinterpret_cast<uintptr_t>(g_arrowPos));
}
// Vanilla arrow handles: ensureTextures resolves them once; if the engine had no handle yet (null), retry at most 10 times, 500 ms apart, on popover show.
void resolveArrows(DWORD now) {
    static DWORD last = 0; static int tries = 0;
    if ((g_arrowNeg && g_arrowPos) || tries >= 10 || (last && now - last < 500)) return;
    last = now; ++tries;
    if (!g_arrowNeg) g_arrowNeg = guikit::imageFor("uibit_arrowneg_p");
    if (!g_arrowPos) g_arrowPos = guikit::imageFor("uibit_arrowpos_p");
    KP_LOG(chUi, "ARROW retry=%d neg=%08x pos=%08x", tries, (unsigned)reinterpret_cast<uintptr_t>(g_arrowNeg), (unsigned)reinterpret_cast<uintptr_t>(g_arrowPos));
}
void healTextures() { embtex::heal(); }   // every frame (GL reset + missing rebuild, 20-refill give-up per texture; DB lesson 290)
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
enum {
    W_Disc0 = 0, W_Disc1, W_DiscLeader,
    W_Frame0, W_Frame1, W_FrameLeader,
    W_Btn0, W_Btn1, W_BtnLeader,
    W_PopBg, W_AutoPill, W_Auto, W_HpM, W_HpP, W_KeepM, W_KeepP, W_HpVal, W_KeepVal,
    W_Ring0, W_Ring1, W_RingLeader,
    NWidgets
};
W g_w[NWidgets];
// Phase G bisect (HUD +0x48 = 0xFF8FB44F hides hotbar + self-heal): false = the 4 SH2 leader widgets are not created. true = leader auto toggle button (outside, left of the portrait).
constexpr bool kLeaderWidgets = true;

// First failure per (widget, op) is logged once (setRect / setText / setFill retry every frame otherwise silently).
void setFail(const W& w, int op, const char* name) {
    static unsigned char seen[NWidgets][3];
    const int i = static_cast<int>(&w - g_w);
    if (i < 0 || i >= NWidgets || seen[i][op]) return;
    seen[i][op] = 1;
    KP_LOGV(chUi, KPLOG_WARN, "SETFAIL op=%s item=%d", name, i);
}
void wRect(W& w, const shui::Rect& r) { if (!w.c || (w.rectSet && w.r == r)) return; if (guikit::setRect(w.c, r.x, r.y, r.w, r.h)) { w.r = r; w.rectSet = true; } else setFail(w, 0, "rect"); }
bool isPopText(const W& w) { int i = static_cast<int>(&w - g_w); return i == W_Auto || i == W_HpM || i == W_HpP || i == W_KeepM || i == W_KeepP || i == W_HpVal || i == W_KeepVal; }
// Vanilla KOTOR II button colours (r1.md): text and arrows teal, hover cream; Auto-heal caption when OFF dim teal-grey (readable).
constexpr float kTeal[3] = { 0.102f, 0.698f, 0.549f };
constexpr float kCream[3] = { 0.800f, 0.800f, 0.698f };
constexpr float kTealDim[3] = { 0.45f, 0.58f, 0.53f };
void popStyle(W& w) {
    if (!w.c || !isPopText(w)) return;
    guikit::setTextColor(w.c, kTeal[0], kTeal[1], kTeal[2]);
    guikit::setAlpha(w.c, 1.0f);
    w.alpha = 1.0f;
    float rgb[3] = {};
    if (guikit::getTextColor(w.c, rgb)) { w.tr = rgb[0]; w.tg = rgb[1]; w.tb = rgb[2]; }
    else { w.tr = kTeal[0]; w.tg = kTeal[1]; w.tb = kTeal[2]; }
}   // popover text was invisible (CS8 R-pop): explicit colour + alpha 1 on build and every show
void wShow(W& w, bool on) {
    if (!w.c) return;
    if (on) { if (w.vis != 1) popStyle(w); if (w.clk != 1) { guikit::setClickable(w.c, true); w.clk = 1; } if (w.vis != 1) { guikit::setVisible(w.c, true); w.vis = 1; } }
    else { if (w.clk != 0) { guikit::setClickable(w.c, false); w.clk = 0; } if (w.vis != 0) { guikit::setVisible(w.c, false); w.vis = 0; } w.hover = false; w.alpha = -1.f; w.tr = w.tg = w.tb = -1.f; w.frame = -1; w.filled = false; if (isPopText(w)) w.txt.clear(); }   // popover text re-applied on the next show (button "+"/"" state is tracked separately)
}
void wVis(W& w, bool on) { if (!w.c || w.vis == (on ? 1 : 0)) return; guikit::setVisible(w.c, on); w.vis = on ? 1 : 0; }   // visibility only: rings are NEVER clickable
void wText(W& w, const std::string& t) { if (!w.c || w.txt == t) return; if (guikit::setText(w.c, t.c_str())) w.txt = t; else setFail(w, 1, "text"); }
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
    if (w.frame != L.ringFrame) { float rgb[3] = { shui::kRingR, shui::kRingG, shui::kRingB }; if (!guikit::setFill(w.c, img, rgb)) return false; w.frame = L.ringFrame; }
    return true;
}
// Circular button backdrop: csh_disc / csh_disc_hi (beige rim + halo on hover), untinted (change-gated on hover; w.frame = 0 / 1).
// A missing hover image falls back to the normal disc. False = no image / fill refused (caller hides the disc).
bool wDisc(W& w, bool hover) {
    if (!w.c) return false;
    int want = (hover && tex::g_discImg[1]) ? 1 : 0;
    if (w.frame == want) return true;
    void* img = tex::g_discImg[want];
    float rgb[3] = { 1.f, 1.f, 1.f };
    const bool ok = img && guikit::setFill(w.c, img, rgb);
    static int lines = 0;   // edge-gated (hover swap, first draw): proves the circular disc + hover halo fill
    if (lines < 96) { ++lines; const int i = static_cast<int>(&w - g_w); KP_LOG(chUi, "DISC m=%d hover=%d img=%08x setfill=%d", i == W_DiscLeader ? 0 : i - W_Disc0 + 1, want, (unsigned)reinterpret_cast<uintptr_t>(img), (int)ok); }
    if (!ok) { setFail(w, 2, "fill"); return false; }
    w.frame = want;
    return true;
}
// Auto-heal toggle pill: csh_tog_off/on (+ _hi when the row is hovered), untinted, change-gated (w.frame = 0..3 = g_togImg index). A missing image keeps the last fill.
void wPill(W& w, bool on, bool hover) {
    if (!w.c) return;
    const int want = (on ? 2 : 0) + (hover ? 1 : 0);
    if (w.frame == want) return;
    void* img = tex::g_togImg[want] ? tex::g_togImg[want] : tex::g_togImg[on ? 2 : 0];
    float rgb[3] = { 1.f, 1.f, 1.f };
    const bool ok = img && guikit::setFill(w.c, img, rgb);
    static int lines = 0;
    if (lines < 48) { ++lines; KP_LOG(chUi, "PILL on=%d hover=%d img=%08x setfill=%d", (int)on, (int)hover, (unsigned)reinterpret_cast<uintptr_t>(img), (int)ok); }
    if (!ok) { setFail(w, 2, "fill"); return; }
    w.frame = want;
}
void wTextColor(W& w, float r, float g, float b) { if (!w.c || (w.tr == r && w.tg == g && w.tb == b)) return; guikit::setTextColor(w.c, r, g, b); w.tr = r; w.tg = g; w.tb = b; }
void wTip(W& w, const std::string& t) { if (!w.c || w.tip == t || !guitip::nativeSupported()) return; if (guitip::setNative(w.c, t.c_str())) w.tip = t; }

// Unconscious button: not clickable (the engine neither hovers nor tooltips it); wShow(true) restores clickability the frame it wakes.
void wInert(W& w) { if (w.c && w.clk != 0) { guikit::setClickable(w.c, false); w.clk = 0; } w.hover = false; }

// Native GUI sound by guisounds.2da index (0 gui_click, 1 gui_scroll = hover, 3 gui_check). False when the sound manager is null.
bool playGuiIdx(int idx) {
    uint32_t mgr = 0;
    if (!guikit::g_gk.playSound || !guikit::g_gk.guiSoundMgrAddr || !guikit::detail::rd32(guikit::g_gk.guiSoundMgrAddr, 0, &mgr) || !mgr) return false;
    guikit::g_gk.playSound(reinterpret_cast<void*>(static_cast<uintptr_t>(mgr)), idx);
    return true;
}
bool playClickSound() { return playGuiIdx(0); }
int g_popHover = -1;                   // popover row under the cursor (shui::PopRow, -1 = none), edge-gated hover sound/log
int g_popMi = -1;                      // member whose popover is open (-1 = closed, 0 = leader, 1..2 = companions)
guikit::ClickEdge g_lmb, g_rmb;
DWORD g_cacheMs = 0, g_cfgCheckMs = 0;
bool g_cacheValid = false;
bool g_mapLogged[3] = {}, g_warnTag[3] = {};
// Stepper arrow (vanilla BTN_DIFFLEFT/RIGHT look, r1.md): uibit_arrowneg_p (-) / uibit_arrowpos_p (+) white glyph tinted teal, cream on hover, alpha 0.35 at the bound.
// Change-gated on the state edge: w.frame = hover, w.tr/tg/tb = tint sent (wShow(false) resets both). Fill refused or no image: the "-"/"+" text in the same colour.
void wArrow(W& w, void* img, bool hover, bool dim) {
    if (!w.c) return;
    const int i = static_cast<int>(&w - g_w);
    const float* rgb = hover ? kCream : kTeal;
    const int want = hover ? 1 : 0;
    if (w.frame != want || w.tr != rgb[0] || w.tg != rgb[1] || w.tb != rgb[2]) {
        float tint[3] = { rgb[0], rgb[1], rgb[2] };
        const bool ok = img && guikit::setFill(w.c, img, tint);
        if (ok) {
            w.tr = rgb[0]; w.tg = rgb[1]; w.tb = rgb[2];
            if (guikit::setText(w.c, "")) w.txt.clear(); else setFail(w, 1, "text");   // direct: wShow(false) clears the text cache, not the control
        } else {
            wText(w, (i == W_HpM || i == W_KeepM) ? "-" : "+");
            wTextColor(w, rgb[0], rgb[1], rgb[2]);
        }
        w.frame = want;
        static int logs = 0;   // edge-gated (state change / show), capped
        if (logs < 24) { ++logs; KP_LOG(chUi, "ARROW fill m=%d item=%d ok=%d img=%08x", g_popMi, i, (int)ok, (unsigned)reinterpret_cast<uintptr_t>(img)); }
    }
    wAlpha(w, dim ? 0.35f : 1.0f);
}


struct Card {
    bool have = false;
    party::Vitals m;
    Pick pk, apk;
    std::string tag;
    shui::CompCfg cfg;
    int k = -1;
    bool seen = false;                 // matched to a slot this frame
    shui::Rect vit{ 0, 0, 0, 0 };
    shui::Rect card{ 0, 0, 0, 0 };
    shui::Rect btn{ 0, 0, 0, 0 };
    shui::BtnState st = shui::BtnState::Hidden;
};
Card g_card[3];
Card g_card0;   // Member 0 (leader)

// Popover size from the actual text: dialogfont10x10 widths (TextFit.h) at the current screen height, so "Use below 95%" never wraps.
constexpr float kPopTextK10 = 1.03f;   // CharEffectsPanel default text_k10 (measured for dialogfont10x10)
struct PopMetrics { int unit = 0, panelW = 0, widest = 0, line = 0, margin = 0, pillW = 0, pillH = 0, screenW = 0, screenH = 0; };
PopMetrics g_popMet;
shui::PopLayout popLayoutFor(const Card& pc, int screenW, int screenH) {
    const float sy = screenH > 0 ? screenH / 600.f : 1.f;
    const textfit::TextCtx tc{ sy, kPopTextK10, 1.f };
    auto px = [&](const char* t) { return textfit::textPx(tc, t, (int)strlen(t), textfit::F10); };
    const float wHp = px("Use below 95%"), wKeep = px("Keep 20"), wCap = px(shui::popAutoCaption());
    const float wVal = wHp > wKeep ? wHp : wKeep, wMax = wVal > wCap ? wVal : wCap;
    const float lineEng = textfit::kFont10LineH * kPopTextK10 * sy;
    PopMetrics m;
    m.unit = shui::popUnitForText(wMax, lineEng, shui::popUnit(pc.btn));
    m.margin = m.unit / 4 > 4 ? m.unit / 4 : 4;
    m.pillH = m.unit - 4; m.pillW = m.pillH * 2;
    m.widest = (int)std::ceil(wMax);
    m.line = (int)std::ceil(lineEng);
    int w = 8 * m.unit;
    if (w < m.widest + 2 * m.unit) w = m.widest + 2 * m.unit;
    const int wv = (int)std::ceil(wVal) + 2 * m.unit + 2 * m.margin;   // stepper squares + value text with a margin each side
    if (w < wv) w = wv;
    const int wc = (int)std::ceil(wCap) + m.pillW + 3 * m.margin;      // caption + pill on the toggle row
    if (w < wc) w = wc;
    m.panelW = w; m.screenW = screenW; m.screenH = screenH;
    g_popMet = m;
    return shui::popoverLayoutW(shui::ringRect(pc.btn), screenW, m.unit, m.panelW);
}

std::string keyFor(int mi) {
    if (mi == 0) return "slot0";
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
    g_popMi = -1;
    for (Card& c : g_card) { c.seen = false; c.st = shui::BtnState::Hidden; }
    g_card0.seen = false;
    g_card0.st = shui::BtnState::Hidden;
}

// pending tracker: HP up / stack down / timeout -> DONE (runs every frame while a use is pending, also in menus)
void pendingTick(DWORD now) {
    bool any = false;
    for (int i = 1; i <= 2; ++i) any = any || g_pend[i].on;
    any = any || g_pend0.on;
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
        actq::Snapshot qs;
        if (i < cnt) actq::read(v[i].server, &qs, SiteQueue, SiteRound);
        bool queued = actq::useQueued(qs);
        if (!shui::pendingDone2(g_pend[i], now, hp, stack, queued, &g_unqueuedSince[i])) continue;
        bool progress = hp > g_pend[i].preHp || (stack >= 0 && stack < g_pend[i].preStack) || (stack < 0 && g_pend[i].preStack > 0);
        KP_LOG(chCall, "DONE member=%d hp%+d stack%+d queued=%d", i, hp - g_pend[i].preHp, (stack < 0 ? 0 : stack) - g_pend[i].preStack, (int)queued);
        if (!progress) KP_LOGV(chCall, KPLOG_WARN, "TIMEOUT unqueued member=%d", i);
        g_pend[i].on = false; g_auto[i].pending = false; g_unqueuedSince[i] = 0; g_kickReason[i] = 0; g_starvedSince[i] = 0; g_kickGaveUp[i] = false; g_starveLogged[i] = false; g_rotNoopLogged[i] = false;
    }
    // Member 0 (leader)
    if (!g_pend0.on) return;
    int hp0 = 0 < cnt ? v[0].cur : g_pend0.preHp;
    int stack0 = -1;
    if (haveItems) { for (const itemread::Entry& e : items) if ((uint32_t)e.key == g_pendKey0) { stack0 = itemread::stackCount(e.key, e.name); if (stack0 <= 0) stack0 = 1; break; } }
    else stack0 = g_pend0.preStack;
    actq::Snapshot qs0;
    if (0 < cnt) actq::read(v[0].server, &qs0, SiteQueue, SiteRound);
    bool queued0 = actq::useQueued(qs0);
    if (!shui::pendingDone2(g_pend0, now, hp0, stack0, queued0, &g_unqueuedSince0)) return;
    bool progress0 = hp0 > g_pend0.preHp || (stack0 >= 0 && stack0 < g_pend0.preStack) || (stack0 < 0 && g_pend0.preStack > 0);
    KP_LOG(chCall, "DONE member=0 hp%+d stack%+d queued=%d", hp0 - g_pend0.preHp, (stack0 < 0 ? 0 : stack0) - g_pend0.preStack, (int)queued0);
    if (!progress0) KP_LOGV(chCall, KPLOG_WARN, "TIMEOUT unqueued member=0");
    g_pend0.on = false; g_auto0.pending = false; g_unqueuedSince0 = 0; g_kickReason0 = 0; g_starvedSince0 = 0; g_kickGaveUp0 = false;
}

// CS12 route B (R8): rotate the starved follower queue so the 0x3F reaches the head. Game thread, our draw callback (outside RunActions).
// Gates (all required): cr+0x20C==0, cr+0x11AC!=0, round!=0, round+0xAA8==0, round+0xAB8<1, RoundHasPending(round)!=0, not paused.
void typesStr(const actq::Snapshot& q, char* buf, size_t cap) {
    size_t n = 0; buf[0] = 0;
    for (int j = 0; j < q.ne && n + 6 < cap; ++j) n += snprintf(buf + n, cap - n, "%s%x", j ? "," : "", q.e[j].type & 0xFF);
}
void kickSkip(int mi, int reason, const char* gate, long long val) {
    if (g_kickReason[mi] == reason) return;
    g_kickReason[mi] = reason;
    g_kickGate[mi] = gate;
    KP_LOG(chCall, "KICK skipped member=%d (%s=%lld)", mi, gate, val);
}
void kickTick(DWORD now) {
    if (!g_kick || !g_hud) return;
    bool any = false;
    for (int i = 1; i <= 2; ++i) any = any || g_pend[i].on;
    if (!any) return;
    party::Vitals v[3];
    int cnt = party::members(v, 3);
    for (int i = 1; i <= 2 && i < cnt; ++i) {   // never member 0 (the controlled leader)
        if (!g_pend[i].on || g_kickGaveUp[i] || !v[i].server) continue;
        uintptr_t cr = v[i].server;
        actq::Snapshot q;
        bool qok = actq::read(cr, &q, SiteQueue, SiteRound);
        if (!qok && g_queueTrace && !g_readFail[i]) KP_LOG(chCall, "QDIAG kick queue read failed member=%d cr=%08x (treated as empty)", i, (unsigned)cr);
        g_readFail[i] = !qok;
        if (!actq::starved(q)) {
            if (g_starveLogged[i]) { g_starveLogged[i] = false; if (g_queueTrace) KP_LOG(chCall, "QDIAG starved cleared member=%d queue=%d", i, q.ne); }
            g_starvedSince[i] = 0; g_kickReason[i] = 0; continue;
        }
        bool paused = actq::playerPaused(SitePause);
        if (paused) { g_starvedSince[i] = 0; kickSkip(i, 1, "paused", 1); continue; }
        if (!g_starveLogged[i]) { g_starveLogged[i] = true; if (g_queueTrace) qdiagSnap("starved onset", i, q, false); }
        if (!g_starvedSince[i]) g_starvedSince[i] = now ? now : 1u;
        else if (now - g_starvedSince[i] > 5000) {
            KP_LOGV(chCall, KPLOG_WARN, "KICK starved member=%d", i);
            if (g_queueTrace) {
                char ty[64]; qdiagTypes(q, ty, sizeof ty);
                KP_LOG(chCall, "QDIAG giveup member=%d elapsed=%lu ms last=%s reason=%d types=[%s]", i, (unsigned long)(now - g_starvedSince[i]),
                       g_kickReason[i] ? g_kickGate[i] : "none", g_kickReason[i], ty);
            }
            g_kickGaveUp[i] = true; continue;
        }
        uint32_t running = 0, c11ac = 0, round = 0;
        int aa8 = 0, ab8 = 0;
        if (!saferead::readAt(SiteKick, cr, actq::CrRunning, &running)) { kickSkip(i, 2, "running", -1); continue; }
        if (running != 0) { kickSkip(i, 2, "running", running); continue; }
        if (!saferead::readAt(SiteKick + 1, cr, actq::Cr11AC, &c11ac)) { kickSkip(i, 3, "cr11ac", -1); continue; }
        if (c11ac == 0) { kickSkip(i, 3, "cr11ac", 0); continue; }
        round = q.round;
        if (!round) { kickSkip(i, 4, "round", 0); continue; }
        if (!saferead::readAt(SiteKick + 2, round, actq::RdAA8, &aa8)) { kickSkip(i, 5, "aa8", -1); continue; }
        if (aa8 != 0) { kickSkip(i, 5, "aa8", aa8); continue; }
        if (!saferead::readAt(SiteKick + 3, round, actq::RdAB8, &ab8)) { kickSkip(i, 6, "ab8", -1); continue; }
        if (ab8 >= 1) { kickSkip(i, 6, "ab8", ab8); continue; }
        uint32_t pending = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::RoundHasPending)(round);
        if (pending == 0) { kickSkip(i, 7, "pending", 0); continue; }
        int k = actq::firstIndex(q, actq::T_USE_ITEM);
        char before[96], after[96];
        typesStr(q, before, sizeof before);
        int done = 0;
        bool removeZero = false;
        for (int n = 0; n < k; ++n) {
            uint32_t d = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::ActListRemoveHead)(cr + actq::CrQueue);
            if (d == 0) { removeZero = true; break; }
            reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail)(cr + actq::CrQueue, d);
            ++done;
        }
        actq::Snapshot q2;
        actq::read(cr, &q2, SiteQueue, SiteRound);
        typesStr(q2, after, sizeof after);
        KP_LOG(chCall, "KICK member=%d before=[%s] after=[%s] k=%d route=rotate", i, before, after, done);
        if (g_queueTrace && !g_rotNoopLogged[i] && (removeZero || !strcmp(before, after))) {
            g_rotNoopLogged[i] = true;
            KP_LOG(chCall, "QDIAG rotate no-op member=%d why=%s k=%d done=%d queue=%d", i, removeZero ? "remove-head-0" : "before==after", k, done, q2.ne);
        }
        g_kickReason[i] = 0;
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
    // Member 0 (leader)
    Card& c0 = g_card0;
    c0.have = 0 < cnt && v[0].ok;
    static bool leaderGone = false;
    if (!c0.have != leaderGone) { leaderGone = !c0.have; KP_LOG(chUi, "LEADER %s (members=%d ok=%d)", leaderGone ? "gone" : "back", cnt, cnt > 0 ? (int)v[0].ok : 0); }
    if (!c0.have) { c0.pk = Pick(); c0.apk = Pick(); c0.m = party::Vitals(); c0.cfg = shui::CompCfg(); } else {
        c0.m = v[0];
        char tag[64]; readCreatureTag(c0.m.server, tag, sizeof tag);
        c0.tag = tag;
        c0.cfg = shui::lookup(g_cfg, keyFor(0));
        c0.pk = analyzeMember(c0.m, items, false, 1);
        c0.apk = c0.cfg.autoOn ? analyzeMember(c0.m, items, false, c0.cfg.keep + 1) : Pick();
        // Auto-heal for leader (member 0)
        if (c0.cfg.autoOn && c0.apk.pick >= 0) {
            heal::AutoCfg ac; ac.on = true; ac.hpPct = c0.cfg.hpPct; ac.keep = c0.cfg.keep;
            uint32_t cmd = 0;
            bool haveCmd = rd<uint32_t>(c0.m.server, Server_Commandable, &cmd);
            bool alive = c0.m.ok && c0.m.cur > 0;
            if (heal::shouldAutoUse(ac, c0.m.cur, c0.m.max, alive, haveCmd && cmd != 0, g_card0.seen, g_auto0, now)) {
                KP_LOG(chCall, "AUTO member=0 hp=%d/%d pick=\"%s\" keep=%d", c0.m.cur, c0.m.max, c0.apk.name.c_str(), c0.cfg.keep);
                if (!useNow(ctx, now, 0, c0.cfg.keep + 1, "auto")) g_auto0.lastUseMs = now;
            }
        }
    }
    g_cacheMs = now; g_cacheValid = true;
}

std::string tipFor(const Card& c) {
    char b[160];
    if (&c == &g_card0) {   // leader: click heals self, right-click opens the options popover
        switch (c.st) {
            case shui::BtnState::FullHp: return "Full health";
            case shui::BtnState::NoItem: return "No healing item";
            default: return "Heal self - right-click for options";
        }
    }
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

// Popover geometry + per-row readback on every closed->open edge or member change (cap 16): proves or refutes the invisible-text theory
// (engine Draw 0x47B500 drops a vertically-centred single line when box h < line h) and the align-10 fix. ext = engine px, tobj/rend = text object gate + renderer ptr.
shui::Rect g_dbgBtn{ 0, 0, 0, 0 }; int g_dbgUnit = 0;
void popRowLog(bool popOpen, int mi, int screenW) {
    static int lastMi = -2, lines = 0;
    const int cur = popOpen ? mi : -1;
    if (cur == lastMi) return;
    lastMi = cur;
    if (!popOpen || lines >= 16) return;
    ++lines;
    int screenH = 0;
    if (guikit::g_gk.screenHAddr) guikit::detail::rdInt(guikit::g_gk.screenHAddr, 0, &screenH);
    KP_LOG(chUi, "POP open m=%d screen=%dx%d unit=%d btn=[%d,%d,%d,%d] align=10", mi, screenW, screenH, g_dbgUnit, g_dbgBtn.x, g_dbgBtn.y, g_dbgBtn.w, g_dbgBtn.h);
    KP_LOG(chUi, "POPSIZE m=%d unit=%d panelW=%d widest=%d line=%d screen=%dx%d", mi, g_popMet.unit, g_popMet.panelW, g_popMet.widest, g_popMet.line, screenW, screenH);
    static const char* const names[8] = { "bg", "auto", "hpm", "hpp", "keepm", "keepp", "hpval", "keepval" };
    const int items[8] = { W_PopBg, W_Auto, W_HpM, W_HpP, W_KeepM, W_KeepP, W_HpVal, W_KeepVal };
    const int oTx = guikit::detail::off(&K2AddrTable::Label_Text), oRn = guikit::detail::off(&K2AddrTable::Text_Renderer);
    for (int i = 0; i < 8; ++i) {
        const W& w = g_w[items[i]];
        if (!w.c) { KP_LOG(chUi, "POPROW m=%d item=%s null", mi, names[i]); continue; }
        int e[4] = { -1, -1, -1, -1 }; const bool ge = guikit::getExtent(w.c, &e[0], &e[1], &e[2], &e[3]);
        float rgb[3] = { -1.f, -1.f, -1.f }; const bool gc = guikit::getTextColor(w.c, rgb);
        uint32_t rend = 0; if (oTx && oRn) guikit::detail::rd32(w.c + oTx, oRn, &rend);
        KP_LOG(chUi, "POPROW m=%d item=%s ext=[%d,%d,%d,%d] extOk=%d vis=%d txt=\"%s\" tobj=%d rend=%08x rgb=%.2f,%.2f,%.2f rgbOk=%d rectSet=%d",
               mi, names[i], e[0], e[1], e[2], e[3], (int)ge, (int)guikit::isVisible(w.c), w.txt.c_str(), (int)guikit::detail::textObjOk(w.c), rend, rgb[0], rgb[1], rgb[2], (int)gc, (int)w.rectSet);
    }
}

const char* stName(shui::BtnState st) {
    switch (st) {
        case shui::BtnState::Hidden: return "Hidden"; case shui::BtnState::FullHp: return "FullHp"; case shui::BtnState::NoItem: return "NoItem";
        case shui::BtnState::Pending: return "Pending"; case shui::BtnState::AutoOn: return "AutoOn"; case shui::BtnState::Normal: return "Normal";
        case shui::BtnState::Down: return "Down";
    }
    return "?";
}
// Edge-triggered (state changes only, capped): proves Down / wake transitions in the play log.
void stateLog(int mi, shui::BtnState st, const Card& c, float lookAlpha) {
    static shui::BtnState last[3] = { shui::BtnState::Hidden, shui::BtnState::Hidden, shui::BtnState::Hidden };
    static int lines[3] = {};   // per member so a busy companion cannot starve the others
    if (last[mi] == st) return;
    const shui::BtnState old = last[mi]; last[mi] = st;
    if (lines[mi] >= 24) return;
    ++lines[mi];
    KP_LOG(chUi, "STATE member=%d %s->%s hp=%d/%d lookA=%.2f", mi, stName(old), stName(st), c.m.cur, c.m.max, lookAlpha);
}
// One line per distinct leader rect (placement proof for the next play log).
void leaderRectLog(const shui::Rect& btn, const shui::Rect& card, const shui::Rect& vit, int diameter, int reserve) {
    static shui::Rect last{ -1, -1, -1, -1 }; static int lines = 0;
    if (btn == last || lines >= 16) return;
    last = btn; ++lines;
    const shui::Rect rg = shui::ringRect(btn);
    KP_LOG(chUi, "LEADERBTN rect=[%d,%d,%d,%d] ring=[%d,%d,%d,%d] card=[%d,%d,%d,%d] vit=[%d,%d,%d,%d] disc=%d reserve=%d", btn.x, btn.y, btn.w, btn.h, rg.x, rg.y, rg.w, rg.h, card.x, card.y, card.w, card.h, vit.x, vit.y, vit.w, vit.h, diameter, reserve);
}

void uiFrame(const K2PsContext* ctx, DWORD now) {
    // INI hot reload (1 s throttle inside changed())
    if (filewatch::changed(g_cfgPoll, IniPath, now)) loadCfg();
    pendingTick(now);
    kickTick(now);
    tex::healTextures();   // every frame, before the gate: a GL reset can happen in menus too
    bool ledge = guikit::clickEdge(g_lmb), redge = guikit::rclickEdge(g_rmb);   // polled every frame so a held press never fires on reopen
    if (!gateOpen(ctx)) { hideAll(); return; }
    if (!g_cacheValid || now - g_cacheMs >= CacheMs) refreshCache(ctx, now);

    // slot -> card mapping + vit rects
    for (Card& c : g_card) c.seen = false;
    g_card0.seen = false;
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
        // Member 0 (leader): shown only while refreshCache found a readable party entry (c0.have)
        if (k == 0 && g_card0.have) {
            g_card0.k = k;
            g_card0.vit = vit;
            g_card0.seen = true;
            int cd[4] = {};
            bool okCd = true;
            for (int j = 0; j < 4; ++j) okCd = rd<int>((uintptr_t)slot, Slot_CardRect + 4 * j, &cd[j]) && okCd;
            if (okCd && cd[2] > 0 && cd[3] > 0) g_card0.card = shui::Rect{ cd[0], cd[1], cd[2], cd[3] };
            if (!g_mapLogged[0]) { g_mapLogged[0] = true; KP_LOG(chUi, "MAP member=0 slot=%d k=%d vit=[%d,%d,%d,%d] card=[%d,%d,%d,%d]", i, k, vit.x, vit.y, vit.w, vit.h, cd[0], cd[1], cd[2], cd[3]); }
        }
    }
    int screenW = 0;   // popover right clamp; 0 = unknown (no clamp)
    if (guikit::g_gk.screenWAddr) guikit::detail::rdInt(guikit::g_gk.screenWAddr, 0, &screenW);
    int screenH = 0;   // popover text sizing (px per texel = screenH / 600)
    if (guikit::g_gk.screenHAddr) guikit::detail::rdInt(guikit::g_gk.screenHAddr, 0, &screenH);

    // button state + look
    for (int mi = 1; mi <= 2; ++mi) {
        Card& c = g_card[mi];
        W& b = g_w[W_Btn0 + mi - 1];
        c.btn = c.seen ? shui::buttonRect(c.vit) : shui::Rect{ 0, 0, 0, 0 };
        if (c.btn.w > 0) {   // hud-host strip slot order 0 (doc 24 CS10): registration only, the button is still drawn at shui::buttonRect
            K2HudRect sr{};
            g_hudc.stripSlot("companion-selfheal", 0, K2HudRect{ c.vit.x, c.vit.y, c.vit.w, c.vit.h }, shui::stripReserve(c.vit), c.btn.h, &sr);
        }
        c.st = shui::resolveState(c.have && c.seen, false, c.m.cur, c.m.max, c.pk.pick >= 0, g_pend[mi].on, c.cfg.autoOn, c.m.down);
        if (c.btn.w <= 0) c.st = shui::BtnState::Hidden;
        stateLog(mi, c.st, c, shui::lookFor(c.st, c.cfg.autoOn, now).alpha);
        if (c.st == shui::BtnState::Down && g_popMi == mi) g_popMi = -1;   // unconscious: no popover
        W& ring = g_w[W_Ring0 + mi - 1];
        W& disc = g_w[W_Disc0 + mi - 1];
        W& frm = g_w[W_Frame0 + mi - 1];
        if (c.st == shui::BtnState::Hidden) { wShow(b, false); wShow(ring, false); wShow(disc, false); wShow(frm, false); if (g_popMi == mi) g_popMi = -1; continue; }
        wRect(b, c.btn);
        wRect(disc, shui::ringRect(c.btn));
        wVis(disc, wDisc(disc, b.hover));
        const shui::Look L = shui::lookFor(c.st, c.cfg.autoOn, now);
        if (disc.c) wAlpha(disc, L.alpha);   // disc dims with the cross
        wIcon(b, mi, L);
        if (L.ring) {
            wRect(ring, shui::ringRect(c.btn));
            if (wRing(ring, L)) wVis(ring, true); else wVis(ring, false);
        } else wVis(ring, false);
        wTip(b, tipFor(c));
        wShow(b, true);
        if (c.st == shui::BtnState::Down) wInert(b);
    }
    // Member 0 (leader) button
    Card& c0 = g_card0;
    W& b0 = g_w[W_BtnLeader];
    // Geometry source: the portrait card rect; fallback (card unreadable) = the vit rect through the same formula.
    const shui::Rect geo0 = c0.card.w > 0 ? c0.card : c0.vit;
    // Disc size = the companion disc (queue icon size); the leader's own vit.h (208) would hit the 72 px cap.
    const int leaderDiam = (g_card[1].seen && g_card[1].vit.h > 0) ? shui::discDiameter(g_card[1].vit.h) : (g_card[2].seen && g_card[2].vit.h > 0 ? shui::discDiameter(g_card[2].vit.h) : shui::kDefaultDisc);
    c0.btn = c0.seen ? shui::leaderButtonRect(geo0, c0.vit, leaderDiam) : shui::Rect{ 0, 0, 0, 0 };
    if (c0.btn.w > 0) {   // hud-host strip slot order 0 (doc 24 CS10): registration only, the button is still drawn at shui::leaderButtonRect
        K2HudRect sr{};
        g_hudc.stripSlot("companion-selfheal", 0, K2HudRect{ geo0.x, geo0.y, geo0.w, geo0.h }, shui::leaderStripReserve(geo0, c0.vit, leaderDiam), c0.btn.h, &sr);
        leaderRectLog(c0.btn, geo0, c0.vit, leaderDiam, shui::leaderStripReserve(geo0, c0.vit, leaderDiam));
    }
    c0.st = shui::resolveState(c0.have && c0.seen, false, c0.m.cur, c0.m.max, c0.pk.pick >= 0, g_pend0.on, c0.cfg.autoOn, c0.m.down);
    if (c0.btn.w <= 0) c0.st = shui::BtnState::Hidden;
    stateLog(0, c0.st, c0, shui::lookFor(c0.st, c0.cfg.autoOn, now).alpha);
    if (c0.st == shui::BtnState::Down && g_popMi == 0) g_popMi = -1;   // unconscious: no popover
    W& ring0 = g_w[W_RingLeader];
    W& disc0 = g_w[W_DiscLeader];
    W& frm0  = g_w[W_FrameLeader];
    if (c0.st == shui::BtnState::Hidden) { wShow(b0, false); wShow(ring0, false); wShow(disc0, false); wShow(frm0, false); if (g_popMi == 0) g_popMi = -1; }
    else {
        wRect(b0, c0.btn);
        wRect(disc0, shui::ringRect(c0.btn));
        wVis(disc0, wDisc(disc0, b0.hover));
        const shui::Look L0 = shui::lookFor(c0.st, c0.cfg.autoOn, now);
        if (disc0.c) wAlpha(disc0, L0.alpha);
        wIcon(b0, 0, L0);
        if (L0.ring) {
            wRect(ring0, shui::ringRect(c0.btn));
            if (wRing(ring0, L0)) wVis(ring0, true); else wVis(ring0, false);
        } else wVis(ring0, false);
        wTip(b0, tipFor(c0));
        wShow(b0, true);
        if (c0.st == shui::BtnState::Down) wInert(b0);
    }

    // input
    int cx = 0, cy = 0;
    bool haveCur = guikit::cursorEngine(&cx, &cy);
    int onBtn = -1;
    shui::PopLayout L{};
    Card& popCard = (g_popMi == 0) ? g_card0 : g_card[g_popMi >= 0 ? g_popMi : 1];
    bool popOpen = g_popMi >= 0 && popCard.btn.w > 0;
    if (!popOpen) g_popMi = -1;
    if (popOpen) L = popLayoutFor(popCard, screenW, screenH);
    bool inPop = popOpen && haveCur && cx >= L.r[0].x && cx < L.r[0].x + L.r[0].w && cy >= L.r[0].y && cy < L.r[0].y + L.r[0].h;
    // A cursor inside the open popover never hovers a button beneath it (no sound, no halo, no onBtn).
    static bool s_occLogged[3] = { false, false, false };
    static int s_occPop = -1;
    if (s_occPop != g_popMi) { s_occPop = g_popMi; s_occLogged[0] = s_occLogged[1] = s_occLogged[2] = false; }
    static int s_occCount = 0;
    auto noteOcclusion = [&](int mi) {
        if (inPop && !s_occLogged[mi]) {
            s_occLogged[mi] = true;
            if (s_occCount < 24) { ++s_occCount; KP_LOG(chUi, "POPOCCLUDE m=%d btn=%d", g_popMi, mi); }
        }
    };
    if (haveCur) {
        for (int mi = 1; mi <= 2; ++mi) {
            W& b = g_w[W_Btn0 + mi - 1];
            const bool hitUnder = g_card[mi].st != shui::BtnState::Hidden && g_card[mi].st != shui::BtnState::Down && guikit::hit(b.c, cx, cy);   // Down: no hover sound, tooltip, click or popover
            if (hitUnder) noteOcclusion(mi);
            const bool h = hitUnder && !inPop;
            if (h) onBtn = mi;
            if (h && !b.hover) guikit::playHoverSound();
            b.hover = h;
        }
        // Member 0 (leader) button
        W& b0 = g_w[W_BtnLeader];
        const bool hitUnder0 = g_card0.st != shui::BtnState::Hidden && g_card0.st != shui::BtnState::Down && guikit::hit(b0.c, cx, cy);
        if (hitUnder0) noteOcclusion(0);
        const bool h0 = hitUnder0 && !inPop;
        if (h0) onBtn = 0;
        if (h0 && !b0.hover) guikit::playHoverSound();
        b0.hover = h0;
    }
    if (haveCur && ledge) {
        if (inPop) {   // popover click: edit config only, NEVER useNow
            shui::PopItem it = shui::popoverHit(L, cx, cy);
            if (it != shui::PopItem::None) {
                if (filewatch::changed(g_cfgPoll, IniPath, now, true)) loadCfg();   // pick up a hand edit before overwriting
                std::string key = keyFor(g_popMi);
                const shui::CompCfg oldCfg = shui::lookup(g_cfg, key);   // copy: upsert below may move the row
                shui::CompCfg nc = shui::applyPop(oldCfg, it);
                shui::upsert(g_cfg, key, nc);
                if (g_popMi == 0) g_card0.cfg = nc;
                else g_card[g_popMi].cfg = nc;
                saveIni(key, nc);
                if (it == shui::PopItem::AutoToggle) { if (!playGuiIdx(3) && !playGuiIdx(0)) guikit::playHoverSound(); }   // vanilla checkbox sound on the pill
                else if (!(nc == oldCfg)) { if (!playClickSound()) guikit::playHoverSound(); }   // stepper: gui_click only when the value moved (a bound is silent)
                if (it == shui::PopItem::AutoToggle) KP_LOG(chUi, "TOGGLE member=%d auto=%d", g_popMi, (int)nc.autoOn);
                static int clicks = 0;
                if (clicks < 96) { ++clicks; KP_LOG(chUi, "POPCLICK m=%d row=%d", g_popMi, static_cast<int>(it)); }
            }
        } else if (onBtn >= 0) {
            if (onBtn == 0) {   // leader button: LMB uses a medpack on the leader (auto toggle lives in the RMB popover)
                const shui::BtnState st0 = g_card0.st;
                if (st0 == shui::BtnState::FullHp) KP_LOG(chCall, "CLICK member=0 noneed");
                else if (st0 == shui::BtnState::NoItem) KP_LOG(chCall, "CLICK member=0 noitem");
                else if (shui::clickAllowed(st0)) { KP_LOG(chCall, "CLICK member=0 ok"); useNow(ctx, now, 0, shui::clickMinCount(st0), "click"); }
            } else if (Card& c = g_card[onBtn]; shui::clickAllowed(c.st)) {
                if (c.st == shui::BtnState::NoItem) KP_LOG(chCall, "CLICK member=%d noitem", onBtn);
                else useNow(ctx, now, onBtn, shui::clickMinCount(c.st), "click");
            }
        } else if (popOpen) g_popMi = -1;
    } else if (haveCur && redge) {
        if (onBtn >= 0) g_popMi = (g_popMi == onBtn) ? -1 : onBtn;   // 0 = leader, 1..2 = companions
        else if (popOpen && !inPop) g_popMi = -1;
    }

    // popover widgets
    popOpen = g_popMi >= 0;
    int hoverRow = -1;
    if (popOpen) {
        Card& pc = (g_popMi == 0) ? g_card0 : g_card[g_popMi];
        L = popLayoutFor(pc, screenW, screenH);
        g_dbgBtn = pc.btn; g_dbgUnit = g_popMet.unit;
        const shui::CompCfg& cfg = pc.cfg;
        if (haveCur) { const int r = static_cast<int>(shui::popoverRowAt(L, cx, cy)); hoverRow = r != 0 ? r : -1; }
        wRect(g_w[W_PopBg], L.r[0]);
        const shui::Rect& ra = L.r[1];
        const int margin = g_popMet.margin, pillW = g_popMet.pillW, pillH = g_popMet.pillH;
        wRect(g_w[W_AutoPill], shui::Rect{ ra.x + ra.w - pillW - margin, ra.y + (ra.h - pillH) / 2, pillW, pillH });
        wRect(g_w[W_Auto], shui::Rect{ ra.x + margin, ra.y, ra.w - pillW - 3 * margin, ra.h });
        wText(g_w[W_Auto], shui::popAutoCaption());
        const int items[4] = { W_HpM, W_HpP, W_KeepM, W_KeepP };
        for (int i = 0; i < 4; ++i) wRect(g_w[items[i]], L.r[i + 2]);   // stepper look (arrow fill / text fallback) is set by wArrow after the wShow loop
        wRect(g_w[W_HpVal], shui::popValueRect(L, 2)); wText(g_w[W_HpVal], shui::popValueLabel(2, cfg));
        wRect(g_w[W_KeepVal], shui::popValueRect(L, 3)); wText(g_w[W_KeepVal], shui::popValueLabel(3, cfg));
        wPill(g_w[W_AutoPill], cfg.autoOn, hoverRow == shui::Row_Auto);
    }
    for (int i = W_PopBg; i < W_Ring0; ++i) wShow(g_w[i], popOpen);
    if (popOpen) {   // after wShow: popStyle resets the text colour on every show
        const shui::CompCfg& cfg = (g_popMi == 0 ? g_card0 : g_card[g_popMi]).cfg;
        if (cfg.autoOn) wTextColor(g_w[W_Auto], kTeal[0], kTeal[1], kTeal[2]); else wTextColor(g_w[W_Auto], kTealDim[0], kTealDim[1], kTealDim[2]);
        const int rowOf[4] = { shui::Row_HpM, shui::Row_HpP, shui::Row_KeepM, shui::Row_KeepP };
        const int itemOf[4] = { W_HpM, W_HpP, W_KeepM, W_KeepP };
        tex::resolveArrows(now);
        const bool dimOf[4] = { cfg.hpPct <= 5, cfg.hpPct >= 95, cfg.keep <= 0, cfg.keep >= 20 };   // at the bound: dimmed arrow
        void* const imgOf[4] = { tex::g_arrowNeg, tex::g_arrowPos, tex::g_arrowNeg, tex::g_arrowPos };
        for (int i = 0; i < 4; ++i) wArrow(g_w[itemOf[i]], imgOf[i], hoverRow == rowOf[i], dimOf[i]);
    }
    if (hoverRow != g_popHover) {   // edge-gated hover sound + log (the engine does not hover our rects; we hit-test ourselves)
        g_popHover = hoverRow;
        if (hoverRow > 0) {
            guikit::playHoverSound();
            static int hovers = 0;
            if (hovers < 48) { ++hovers; KP_LOG(chUi, "POPHOVER m=%d row=%d", g_popMi, hoverRow); }
        }
    }
    popDiag(popOpen);
    popRowLog(popOpen, g_popMi, screenW);
    int ns = 0;   // rects the RMB camera hook must not turn into a camera drag: buttons, rings, popover bg
    for (int mi = 1; mi <= 2; ++mi) if (g_card[mi].st != shui::BtnState::Hidden && g_card[mi].btn.w > 0) { g_shown[ns++] = g_card[mi].btn; g_shown[ns++] = shui::ringRect(g_card[mi].btn); }   // disc and ring share ringRect (outer extent)
    if (g_card0.st != shui::BtnState::Hidden && g_card0.btn.w > 0) { g_shown[ns++] = g_card0.btn; g_shown[ns++] = shui::ringRect(g_card0.btn); }
    if (popOpen) g_shown[ns++] = L.r[0];
    g_nShown = ns;
}

void KPATCH_CALL onFrameBegin(const K2PsContext* ctx, void*) {
    if (!g_hud) return;
    saferead::beginScope();
    DWORD now = GetTickCount();
    if (g_logOn) {
        if (now - g_lastPollMs >= 2000) {
            g_lastPollMs = now;
            static uint32_t s_lastSlotSig = 0;
            static bool s_sigInit = false;
            uint32_t slotSig = 17;
            for (int i = 0; i < K2PS_MAX_SLOTS; ++i) {
                int32_t sl = 0, cr = 0;
                if (g_ps.slotAt(i, &sl, &cr)) slotSig = slotSig * 31 + (uint32_t)sl ^ (uint32_t)cr;
            }
            if (!s_sigInit || slotSig != s_lastSlotSig) {
                s_sigInit = true;
                s_lastSlotSig = slotSig;
                logSlotMap();
            }
        }
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
uintptr_t makeLabel(uintptr_t panel, const char* tmpl, const char* text, bool font, int* whyOut, unsigned align = 18) {
    int why = guikit::CF_None; uint32_t badVt = 0;
    uintptr_t c = guikit::createLabelFromTemplate(panel, tmpl, &why, &badVt);
    *whyOut = why;
    if (!c) return 0;
    const bool fOk = !font || guikit::setFont(c, "dialogfont10x10"), aOk = !font || guikit::setAlign(c, align), tOk = !text || guikit::setText(c, text);
    if (!(fOk && aOk && tOk)) KP_LOGV(chHud, KPLOG_WARN, "SETFAIL op=build tmpl=%s font=%d align=%d text=%d", tmpl, (int)fOk, (int)aOk, (int)tOk);
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (!guikit::appendToPanel(panel, c)) {   // no destroy helper in GuiKit: the control stays hidden + click-through and unreferenced
        *whyOut = guikit::CF_Count;
        KP_LOGV(chHud, KPLOG_WARN, "BUILD append failed, control %08x left hidden + click-through", (unsigned)c);
        return 0;
    }
    return c;
}

// 16 widgets while the HUD layout is still open (before StopLoadFromLayout).
void buildWidgets(uintptr_t panel) {
    for (W& w : g_w) w = W();
    tex::ensureTextures();
    g_popMi = -1; g_lmb = guikit::ClickEdge(); g_rmb = guikit::ClickEdge();
    for (Card& c : g_card) c.seen = false;
    g_card0.seen = false;
    ProjGuard pg;
    int n = 0;
    std::string whys;
    auto add = [&](int idx, const char* tmpl, const char* text, bool font, const char* name) {
        if (!kLeaderWidgets && (idx == W_DiscLeader || idx == W_FrameLeader || idx == W_BtnLeader || idx == W_RingLeader)) { ++n; return; }
        int why = 0;
        // Popover rows are 16-24 px; engine Draw 0x47B500 drops a vertically-centred (0x10) single line when box h < line h (V3.md F3, SPECULATIVE at this px/unit), so they are top-aligned (10 = 0x08 top | 2 centre-h; 17 = 0x11 still has the 0x10 bit). POP/POPROW log lines prove or refute this.
        const unsigned al = idx == W_Auto ? 9u : ((idx > W_Auto && idx <= W_KeepVal) ? 10u : 18u);   // Auto-heal caption left-aligned (9 = top|left), other popover text 10 = top|centre-h
        uintptr_t c = makeLabel(panel, tmpl, text, font, &why, al);
        if (!c && !strcmp(tmpl, "LBL_MOULDING3")) { c = makeLabel(panel, "LBL_QUEUE1", text, font, &why, al); if (c) whys += std::string(name) + ":moulding3 fallback ";}
        g_w[idx].c = c; g_w[idx].txt = text ? text : "";
        if ((idx >= W_Ring0 || idx <= W_FrameLeader) && c) { g_w[idx].vis = 0; g_w[idx].clk = 0; }   // discs + frames + rings start hidden + click-through and stay non-clickable
        if (c) ++n; else whys += std::string(name) + ":" + std::to_string(why) + " ";
    };
    add(W_Disc0, "LBL_QUEUE1", nullptr, false, "disc0");   // before the buttons: z-order disc < button < ring
    add(W_Disc1, "LBL_QUEUE1", nullptr, false, "disc1");
    add(W_DiscLeader, "LBL_QUEUE1", nullptr, false, "discLeader");
    add(W_Frame0, "LBL_QUEUE1", nullptr, false, "frame0");   // z-order disc < frame < button (cross) < ring
    add(W_Frame1, "LBL_QUEUE1", nullptr, false, "frame1");
    add(W_FrameLeader, "LBL_QUEUE1", nullptr, false, "frameLeader");
    add(W_Btn0, "LBL_QUEUE1", "+", true, "btn0");   // "+" = the fallback glyph until the cross fill is applied
    add(W_Btn1, "LBL_QUEUE1", "+", true, "btn1");
    add(W_BtnLeader, "LBL_QUEUE1", "+", true, "btnLeader");   // Leader button (member 0)
    add(W_PopBg, "LBL_MOULDING3", nullptr, false, "popbg");
    add(W_AutoPill, "LBL_QUEUE1", nullptr, false, "autopill");   // toggle pill texture under the Auto-heal caption
    add(W_Auto, "LBL_QUEUE1", "Auto", true, "auto");
    add(W_HpM, "LBL_QUEUE1", "-", true, "hpm");
    add(W_HpP, "LBL_QUEUE1", "+", true, "hpp");
    add(W_KeepM, "LBL_QUEUE1", "-", true, "keepm");
    add(W_KeepP, "LBL_QUEUE1", "+", true, "keepp");
    add(W_HpVal, "LBL_QUEUE1", "-", true, "hpval");
    add(W_KeepVal, "LBL_QUEUE1", "-", true, "keepval");
    add(W_Ring0, "LBL_QUEUE1", nullptr, false, "ring0");   // after the buttons: drawn on top
    add(W_Ring1, "LBL_QUEUE1", nullptr, false, "ring1");
    add(W_RingLeader, "LBL_QUEUE1", nullptr, false, "ringLeader");   // Leader ring
    KP_LOGV(chHud, n == NWidgets ? KPLOG_INFO : KPLOG_WARN, "BUILD n=%d/%d why=%s proj=%d cleared=%d", n, (int)NWidgets, whys.c_str(), pg.saved, (int)pg.cleared);
    for (W& w : g_w) popStyle(w);
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
    uint32_t f48pre = 0;
    rd<uint32_t>(panel, Panel_Flags, &f48pre);
    buildWidgets(panel);
    if (!g_logOn) return;
    KP_LOGV(chHud, KPLOG_DEBUG, "P0 bisect leaderWidgets=%d hud +0x48 before build=%08x", (int)kLeaderWidgets, (unsigned)f48pre);
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
    KP_LOG(chMain, "0.8.2-dev ui=popsize+togglepill+leaderclick+vanillaarrows+pillnarrow+popocclude: probe call=%d CallKey=0x%02X kick=%d cfg rows=%d", g_probeCall, g_callKey, (int)g_kick, (int)g_cfg.rows.size());
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
