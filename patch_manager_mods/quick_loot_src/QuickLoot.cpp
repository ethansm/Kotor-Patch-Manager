// Quick Loot 0.1.0-dev (doc 52 QL1a): clicking a loot container takes everything at once, no container menu.
//   QlShowContainer consumes ShowContainerGui (return 1) when Enabled, hud-host is connected, flag == 1 and Shift is up; the container is then
//   "pending". QlPlaceableEvent notes its OnOpen (event 0x16). The hud-host FRAME tick sends the vanilla OK-button message (0x0087A550, Close
//   flag 1 = take all) 2 frames after OnOpen, or 1000 ms after Show when no OnOpen came. Close is never called from inside the EventHandler hook.
//   Anything else returns 0 (vanilla menu). Shift held = vanilla menu. Config: quick_loot.ini [QuickLoot] Enabled=1 (read once at init).
// Below: the QL0 probe description (all its log lines are kept). Questions it answered (patch_manager_mods/52_quick_loot.md):
//   1. does loot that OnOpen generates (queued event 0x16) reach the container before the player's Close?   (SHOW n vs CLOSE n, EVT 0x16 dt)
//   2. timing: ms between Show, event 0x16 and Close.                                                        (dt= on EVT / CLOSE lines)
//   3. does take-all (Close flag 1) fire OnInvDisturbed (event 0x1B)?                                         (EVT id=1b after CLOSE flag=1)
// Hooks read ECX / [ESP+4..] (KPM passes the game's stack: [esp+0] = return address). Log lines: QuickLootLogic.h fmt*.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "KpLog.h"
#include "HudHost.h"
#include "QuickLootLogic.h"
#include "../_shared/SafeRead.h"
#include "../_shared/GameAddr.h"

KP_LOG_PATCH("quick-loot");

namespace {

constexpr const char* Version = "0.1.0-dev";
constexpr unsigned Site = 0x7100;   // saferead failure-ring site id
// Steam addresses (static RE, doc 52 / .claude/research/quick_loot/README.md); AppGlobal and ClientToServerObjectId come from GameAddr.h.
constexpr uintptr_t GetPlaceableByGameObjectID = 0x0051C160;   // thiscall(serverApp, serverId) RET 4 -> CSWSPlaceable* (0 = none)
constexpr uintptr_t ItemListGetItem = 0x00606100;              // thiscall(repo, index) RET 4 -> CSWSItem* (0 = none)
constexpr uintptr_t EventClassGlobal = 0x0099244C;             // DAT_0099244c: the EventHandler's first argument is compared with it
constexpr int Plc_Open = 0x390, Plc_Repo = 0x3C4;              // open flag (byte), item repository pointer
constexpr int Repo_Array = 0x0C, Repo_Count = 0x10;            // object-id array, count
constexpr int Item_Stack = 0x2CC;                              // CSWSItem stack size (int)
// QL1a: vanilla OK button path (0x8B2850): msgObj = *(*(*(*AppGlobal + 4) + 4) + 0x150); thiscall(msgObj, clientObjId, closeFlag) RET 8.
constexpr uintptr_t SendContainerClose = 0x0087A550;
constexpr int App_Client = 4, Client_Inner = 4, Inner_MsgObj = 0x150;
const char* const IniPath = ".\\quick_loot.ini";   // ".\\" prefix: GetPrivateProfile*A with a bare name looks in the Windows dir

KP_CHANNEL(chMain, "main");

qlogic::Tracker g_t;
qlogic::Take g_take;
k2hud::Client g_hud;
bool g_enabled = true;
uint32_t g_frame = 0;   // last HUD frame counter seen by onHudFrame
bool g_started = false;
LARGE_INTEGER g_freq;
bool g_timeInit = false;

template <typename T> bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(Site, base, off, out); }

uint32_t nowMs() {
    if (!g_timeInit) { QueryPerformanceFrequency(&g_freq); g_timeInit = true; }
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return static_cast<uint32_t>(static_cast<unsigned long long>(t.QuadPart) * 1000ull / static_cast<unsigned long long>(g_freq.QuadPart));
}

void enter() {
    saferead::beginScope();
    if (!g_started) { g_started = true; if (KpLog_HasCore()) KP_LOG(chMain, "START quick-loot %s", Version); }
}

using LookupFn = uint32_t(__thiscall*)(uintptr_t self, uint32_t arg);

// *(*AppGlobal + 8) = the server app / object table (same hop as ItemRead.h serverItemFor).
uintptr_t serverApp() {
    uint32_t app = 0, srv = 0;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app || !rd<uint32_t>(app, 8, &srv)) return 0;
    return srv;
}

uintptr_t placeableFor(uint32_t clientId) {
    uintptr_t srv = serverApp();
    if (!srv) return 0;
    uint32_t serverId = reinterpret_cast<LookupFn>(gameaddr::ClientToServerObjectId)(srv, clientId);
    if (!serverId) return 0;
    return reinterpret_cast<LookupFn>(GetPlaceableByGameObjectID)(srv, serverId);
}

// Item count of the placeable's repository, -1 when unreadable. *repoOut receives the repository pointer.
int itemCount(uintptr_t plc, uintptr_t* repoOut = nullptr) {
    uint32_t repo = 0; int n = 0;
    if (!plc || !rd<uint32_t>(plc, Plc_Repo, &repo) || !repo || !rd<int>(repo, Repo_Count, &n)) return -1;
    if (repoOut) *repoOut = repo;
    return n;
}

int openFlag(uintptr_t plc) { uint8_t o = 0; return rd<uint8_t>(plc, Plc_Open, &o) ? static_cast<int>(o) : -1; }

void logLine(const char* s) { KP_LOG(chMain, "%s", s); }

using SendCloseFn = void(__thiscall*)(uintptr_t msgObj, uint32_t objId, int closeFlag);

// The client message object, SafeRead at each hop. 0 = a hop failed.
uintptr_t messageObject() {
    uint32_t app = 0, client = 0, inner = 0, msg = 0;
    if (!rd<uint32_t>(gameaddr::AppGlobal, 0, &app) || !app) return 0;
    if (!rd<uint32_t>(app, App_Client, &client) || !client) return 0;
    if (!rd<uint32_t>(client, Client_Inner, &inner) || !inner) return 0;
    if (!rd<uint32_t>(inner, Inner_MsgObj, &msg) || !msg) return 0;
    return msg;
}

// K2HUD_FRAME: send the pending take-all Close. Runs on the game thread, outside any EventHandler.
void KPATCH_CALL onHudFrame(const K2HudContext* ctx, void*) {
    if (ctx) g_frame = ctx->frame;
    if (!g_take.active) return;
    uint32_t now = nowMs();
    if (!qlogic::readyToSend(g_take.openSeen, g_frame, g_take.openFrame, now, g_take.showMs)) return;
    enter();
    char b[160];
    uintptr_t msg = messageObject();
    if (!msg) {
        uint32_t dt = qlogic::msSince(now, g_take.showMs);
        if (!g_take.errLogged) { g_take.errLogged = true; qlogic::fmtErr(b, sizeof b, "msgobj", dt); logLine(b); }
        if (qlogic::giveUp(now, g_take.showMs)) { qlogic::fmtErr(b, sizeof b, "giveup", dt); logLine(b); g_take.clear(); }
        return;
    }
    int n = itemCount(g_take.plc);
    uint32_t obj = g_take.obj;
    int openSeen = g_take.openSeen ? 1 : 0;
    uint32_t dt = qlogic::msSince(now, g_take.showMs);
    g_take.clear();   // before the call: a re-entrant Show must not see a stale pending take
    reinterpret_cast<SendCloseFn>(SendContainerClose)(msg, obj, 1);
    qlogic::fmtTakeSend(b, sizeof b, obj, dt, openSeen, n);
    logLine(b);
}

}   // namespace

extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    g_enabled = GetPrivateProfileIntA("QuickLoot", "Enabled", 1, IniPath) != 0;
    g_hud.init(api, "quick-loot");
    g_hud.add(K2HUD_FRAME, 100, onHudFrame);
    g_hud.connect();
    if (KpLog_HasCore()) KP_LOG(chMain, "INIT quick-loot %s enabled=%d hud=%d", Version, g_enabled ? 1 : 0, g_hud.connected ? 1 : 0);
}

// H1. ECX = gui, [esp+4] = container object id (client), [esp+8] = flag. Returns 1 to consume (take-all armed), 0 = the original menu runs.
extern "C" int __cdecl QlShowContainer(uintptr_t gui, uint32_t objId, int flag) {
    (void)gui;
    enter();
    uintptr_t plc = placeableFor(objId);
    uintptr_t repo = 0;
    int n = itemCount(plc, &repo);
    uint32_t now = nowMs();
    g_t.onShow(plc, now);
    char b[160];
    qlogic::fmtShow(b, sizeof b, objId, flag, plc, plc ? openFlag(plc) : -1, n);
    logLine(b);
    unsigned lines = qlogic::clampItems(n);
    uint32_t arr = 0;   // object-id array
    if (!repo || !rd<uint32_t>(repo, Repo_Array, &arr) || !arr) lines = 0;
    for (unsigned i = 0; i < lines; ++i) {
        uint32_t id = 0; int stack = -1;
        if (!rd<uint32_t>(arr, static_cast<int>(i * 4), &id)) break;
        uintptr_t item = reinterpret_cast<LookupFn>(ItemListGetItem)(repo, i);
        if (item) rd<int>(item, Item_Stack, &stack);
        qlogic::fmtItem(b, sizeof b, i, id, stack);
        logLine(b);
    }
    g_hud.poll();
    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    qlogic::Reason why = qlogic::shouldConsume(g_enabled, g_hud.connected, flag, shift, plc != 0, g_take.active);
    if (why != qlogic::Reason::Consume) {
        qlogic::fmtVanilla(b, sizeof b, why);
        logLine(b);
        return 0;
    }
    g_take.arm(objId, plc, now, g_frame);
    qlogic::fmtTakeArm(b, sizeof b, objId, plc, n);
    logLine(b);
    return 1;
}

// H2. ECX = placeable, [esp+4] = a1, [esp+8] = a2, [esp+0xC] = a3 (event record pointer). Runs on every placeable heartbeat: filter first.
extern "C" void __cdecl QlPlaceableEvent(uintptr_t plc, int a1, int a2, int a3) {
    uint32_t now = nowMs();
    if (g_take.active && plc == g_take.plc) {   // QL1a: note the container's OnOpen (the Close is sent from the HUD frame, never here)
        enter();
        uint16_t tid = 0;
        if (rd<uint16_t>(static_cast<uint32_t>(a3), 0, &tid)) g_take.onEvent(plc, static_cast<short>(tid), g_frame);
    }
    if (!g_t.matches(plc, now)) return;
    enter();
    int ref = 0;
    rd<int>(EventClassGlobal, 0, &ref);
    uint16_t id16 = 0;
    bool haveId = rd<uint16_t>(static_cast<uint32_t>(a3), 0, &id16);
    char b[160];
    if (g_t.wantRaw()) {
        qlogic::fmtEvtRaw(b, sizeof b, static_cast<unsigned>(plc), a1, a2, static_cast<unsigned>(a3), haveId ? id16 : -1, ref);
        logLine(b);
    }
    if (haveId && qlogic::isWatchedEvent(static_cast<short>(id16))) {
        qlogic::fmtEvt(b, sizeof b, static_cast<short>(id16), qlogic::msSince(now, g_t.showMs), itemCount(plc));
        logLine(b);
    }
}

// H3. ECX = placeable, [esp+4] = player object id, [esp+8] = flag (1 = take all).
extern "C" void __cdecl QlCloseInventory(uintptr_t plc, uint32_t player, int flag) {
    (void)player;
    uint32_t now = nowMs();
    if (!g_t.matches(plc, now)) return;
    enter();
    g_t.onClose(plc, now);
    char b[160];
    qlogic::fmtClose(b, sizeof b, flag, qlogic::msSince(now, g_t.showMs), itemCount(plc), openFlag(plc));
    logLine(b);
}
