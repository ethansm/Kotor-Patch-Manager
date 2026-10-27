// Quick Loot probe 0.0.1-probe (doc 52 QL0). LOG ONLY: reads the game through SafeRead and a few pure object-table lookups, never writes game
// memory, every handler lets the original run (QlShowContainer returns 0). The three hooks have the final QL1 shape (ShowContainerGui with a
// consumed exit), so QL1 is a DLL hot-swap, not a reinstall. Questions it answers (patch_manager_mods/52_quick_loot.md):
//   1. does loot that OnOpen generates (queued event 0x16) reach the container before the player's Close?   (SHOW n vs CLOSE n, EVT 0x16 dt)
//   2. timing: ms between Show, event 0x16 and Close.                                                        (dt= on EVT / CLOSE lines)
//   3. does take-all (Close flag 1) fire OnInvDisturbed (event 0x1B)?                                         (EVT id=1b after CLOSE flag=1)
// Hooks read ECX / [ESP+4..] (KPM passes the game's stack: [esp+0] = return address). Log lines: QuickLootLogic.h fmt*.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "KpLog.h"
#include "QuickLootLogic.h"
#include "../_shared/SafeRead.h"
#include "../_shared/GameAddr.h"

KP_LOG_PATCH("quick-loot");

extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }

namespace {

constexpr const char* Version = "0.0.1-probe";
constexpr unsigned Site = 0x7100;   // saferead failure-ring site id
// Steam addresses (static RE, doc 52 / .claude/research/quick_loot/README.md); AppGlobal and ClientToServerObjectId come from GameAddr.h.
constexpr uintptr_t GetPlaceableByGameObjectID = 0x0051C160;   // thiscall(serverApp, serverId) RET 4 -> CSWSPlaceable* (0 = none)
constexpr uintptr_t ItemListGetItem = 0x00606100;              // thiscall(repo, index) RET 4 -> CSWSItem* (0 = none)
constexpr uintptr_t EventClassGlobal = 0x0099244C;             // DAT_0099244c: the EventHandler's first argument is compared with it
constexpr int Plc_Open = 0x390, Plc_Repo = 0x3C4;              // open flag (byte), item repository pointer
constexpr int Repo_Array = 0x0C, Repo_Count = 0x10;            // object-id array, count
constexpr int Item_Stack = 0x2CC;                              // CSWSItem stack size (int)

KP_CHANNEL(chMain, "main");

qlogic::Tracker g_t;
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

}   // namespace

// H1. ECX = gui, [esp+4] = container object id (client), [esp+8] = flag. Returns 0 always: the original runs.
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
    return 0;
}

// H2. ECX = placeable, [esp+4] = a1, [esp+8] = a2, [esp+0xC] = a3 (event record pointer). Runs on every placeable heartbeat: filter first.
extern "C" void __cdecl QlPlaceableEvent(uintptr_t plc, int a1, int a2, int a3) {
    uint32_t now = nowMs();
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
