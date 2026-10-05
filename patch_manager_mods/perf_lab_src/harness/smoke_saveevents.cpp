// Smoke test for save-events (doc 22 section 4.1 / 6.2): the real save-events.dll (provider of "k2.saveevents" v1) loaded under Wine's real
// PE loader and driven through its five hook exports with the KPM hook calling convention (cdecl; SE_OnLoadRequest takes the [ebp-0x14]
// pointer), exactly as the detours would call them, over a fake engine = the zeroed .bss pad planted with the objects the hooks read.
// Build: i686-w64-mingw32-g++ -std=c++17 -O1 -I <harness> -I tests/include -I../../_shared_src -I$KPM/Patches/Common smoke_saveevents.cpp -static
//        (runner flags in smokes.toml). The pad must cover the three engine globals the DLL reads: AppGlobal 0xA1B4A4, Steam Cloud byte
//        0xA7EA88, cloud root CExoString 0xA7EABC (checked at startup, exit 2).
// Run:   smoke_saveevents.exe <wire|dispatch|inert> [skip-provider]   (dir needs save-events.dll).
//        Exit codes: 0 pass, 1 check failed / LoadLibrary failed, 2 setup failure, 3 missing export.
//        Red-proof: argv[2] == "skip-provider" or env SMOKE_SKIP_PROVIDER=1 skips the provider's KPatchInit; wire and dispatch must then FAIL.
//
// What is real: the DLL, its KPatchInit and hook exports, saferead (real VirtualQuery), the registry semantics (tests/include/FakeKPatchApi.h =
// KPM registry.cpp) and a fake "kpatch.log" sink. What is faked: engine memory. Nothing is written to the engine by the DLL (pad compare).
//
//   wire     provided + table complete; all five hooks produce the right event for local and Steam Cloud roots (kind, folder, cloud, struct_size),
//            NEW_GAME has a NULL folder, CurrentSaveFolder follows (and is cleared by NEW_GAME), one evt log line per event with the consumer
//            count, no WARN, pad untouched.
//   dispatch registry through the real DLL: kind-mask filtering, a callback that unregisters itself and registers another, the slow-consumer
//            WARN (a consumer that sleeps 9 ms; the next consumer still runs), Unregister accounting, refused registrations, table-full,
//            CurrentSaveFolder truncation through the table.
//   inert    hooks before KPatchInit (no crash, nothing dispatched), a KPatchApi without `require` (nothing provided), zero consumers (consumers=0),
//            and unreadable / implausible engine data (null app global, null / small load pointer, garbage folder): WARN, nothing dispatched,
//            CurrentSaveFolder unchanged, pad untouched.
#define __USE_MINGW_ANSI_STDIO 0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>

#include "FakeKPatchApi.h"
#include "SaveEvents.h"

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what) {
    ++g_checks;
    printf("saveevents: %s %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) ++g_fail;
}
static void checkf(bool ok, const char* fmt, int a = 0, int b = 0, int c = 0) {
    char buf[256]; snprintf(buf, sizeof buf, fmt, a, b, c); check(ok, buf);
}
[[noreturn]] static void die(int code, const char* msg) { printf("saveevents: %s\n", msg); fflush(stdout); exit(code); }

// ---------------------------------------------------------------- engine memory: the zeroed pad
static char pad[0x700000];
static const uintptr_t AppGlobal = 0x00A1B4A4, CloudFlag = 0x00A7EA88, CloudDir = 0x00A7EABC;
// fake engine objects (inside the pad, away from the three globals)
static const uintptr_t APP = 0x00A20000, REQ = 0x00A20100, REQSTR = 0x00A20200, LOADSTR = 0x00A20400, CLOUDSTR = 0x00A20600;
static void put32(uintptr_t a, uint32_t v) { memcpy((void*)a, &v, 4); }
static void plantExo(uintptr_t at, uintptr_t strAt, const char* s) {   // CExoString {char* p; int len incl. NUL}
    memcpy((void*)strAt, s, strlen(s) + 1); put32(at, (uint32_t)strAt); put32(at + 4, (uint32_t)strlen(s) + 1);
}
static void plantRequest(const char* folder) { put32(AppGlobal, (uint32_t)APP); put32(APP + 0x14, (uint32_t)REQ); plantExo(REQ + 0x18, REQSTR, folder); }
static void clearRequest() { put32(AppGlobal, 0); }
static void plantCloud(bool on, const char* root) {
    *(uint8_t*)CloudFlag = on ? 1 : 0;
    if (on) plantExo(CloudDir, CLOUDSTR, root); else { put32(CloudDir, 0); put32(CloudDir + 4, 0); }
}
static void plantLoad(const char* s) { memcpy((void*)LOADSTR, s, strlen(s) + 1); }
static std::vector<char> g_before;
static void snapshotPad() { g_before.assign(pad, pad + sizeof pad); }
static bool padUnchanged(uintptr_t* at) {
    for (size_t i = 0; i < sizeof pad; ++i) if (pad[i] != g_before[i]) { *at = (uintptr_t)pad + i; return false; }
    return true;
}
static void setupPad() {
    if ((uintptr_t)pad > AppGlobal || (uintptr_t)pad + sizeof pad < CloudDir + 8) {
        printf("saveevents: pad %p..%p does not cover AppGlobal %08x .. cloud root %08x\n", (void*)pad, (void*)(pad + sizeof pad), (unsigned)AppGlobal, (unsigned)CloudDir + 8);
        exit(2);
    }
}

// ---------------------------------------------------------------- the DLL
typedef void (__cdecl *InitFn)(const KPatchApi*);
typedef void (__cdecl *VoidFn)();
typedef void (__cdecl *LoadFn)(const char*);
struct Dll { HMODULE h = nullptr; InitFn init = nullptr; VoidFn saved = nullptr, autosaved = nullptr, autoLoad = nullptr, newGame = nullptr; LoadFn load = nullptr; };
static Dll D;
static FARPROC need(const char* name) {
    FARPROC p = GetProcAddress(D.h, name);
    if (!p) { printf("saveevents: save-events.dll missing export %s\n", name); exit(3); }
    return p;
}
static void loadDll() {
    D.h = LoadLibraryA("save-events.dll");
    if (!D.h) { printf("saveevents: LoadLibrary save-events.dll failed %lu\n", GetLastError()); exit(1); }
    D.init = (InitFn)need("KPatchInit");
    D.saved = (VoidFn)need("SE_OnSaved"); D.autosaved = (VoidFn)need("SE_OnAutosaved"); D.load = (LoadFn)need("SE_OnLoadRequest");
    D.autoLoad = (VoidFn)need("SE_OnAutosaveSlotLoad"); D.newGame = (VoidFn)need("SE_OnNewGame");
}
static bool g_skipProvider = false;
static const K2SaveEventsApi* table() { return (const K2SaveEventsApi*)fakekp::api()->require(K2SE_IFACE, K2SE_VERSION); }
static void freshRegistry() { fakekp::reset(); fakekp::withLog(); }

// ---------------------------------------------------------------- fake kpatch.log inspection ("<channel>:<level>:<text>", level 1 = WARN)
static int logCount(const char* sub) { int n = 0; for (const std::string& l : fakekp::state().logLines) if (l.find(sub) != std::string::npos) ++n; return n; }
static int warnCount() { int n = 0; for (const std::string& l : fakekp::state().logLines) if (l.find(":1:") == 1) ++n; return n; }
static int warnCount(const char* sub) { int n = 0; for (const std::string& l : fakekp::state().logLines) if (l.find(":1:") == 1 && l.find(sub) != std::string::npos) ++n; return n; }

// ---------------------------------------------------------------- consumers
struct Seen { int who, kind, cloud; uint32_t size; std::string folder; bool nullFolder; };
static std::vector<Seen> g_seen;
static void KPATCH_CALL onEvent(const K2SaveEvent* e, void* u) {
    g_seen.push_back({ (int)(intptr_t)u, e->kind, e->cloud, e->struct_size, e->folder ? e->folder : "", e->folder == nullptr });
}
static int countWho(int who) { int n = 0; for (const Seen& s : g_seen) if (s.who == who) ++n; return n; }
static const Seen* lastOf(int who) { for (size_t i = g_seen.size(); i-- > 0;) if (g_seen[i].who == who) return &g_seen[i]; return nullptr; }
static std::string current() { char b[300]; b[0] = 'x'; int n = table()->CurrentSaveFolder(b, sizeof b); return n ? std::string(b) : std::string(); }
static int32_t regAll(int who, int mask, const char* owner) { return table()->Register(mask, onEvent, (void*)(intptr_t)who, owner); }

// ================================================================ wire
static int modeWire() {
    setupPad(); freshRegistry(); loadDll();
    if (!g_skipProvider) D.init(fakekp::api());
    check(fakekp::provided(K2SE_IFACE, K2SE_VERSION), "provider provided k2.saveevents v1 through the registry");
    check(logCount("provided k2.saveevents v1") == 1, "provider logged its 'provided' line once");
    const K2SaveEventsApi* t = table();
    check(t != nullptr, "require(k2.saveevents, 1) returns a table");
    if (!t) { printf("saveevents: no provider table, stopping (%d failed)\n", g_fail); return 1; }
    check(t->struct_size == sizeof(K2SaveEventsApi), "table struct_size == sizeof(K2SaveEventsApi)");
    check(t->Register && t->Unregister && t->CurrentSaveFolder, "table Register / Unregister / CurrentSaveFolder non-null");
    check(k2se::Client::usable(t), "table passes the client helper's usable() gate");
    check(!fakekp::provided("k2.saveevents", 2), "no other major version provided");

    const int32_t tAll = regAll(1, K2SE_ALL, "smoke-all"), tLoad = regAll(2, K2SE_LOAD_REQUEST | K2SE_NEW_GAME, "smoke-load");
    check(tAll > 0 && tLoad > 0 && tAll != tLoad, "two registrations return distinct tokens > 0");
    check(logCount("register mask=15 owner=smoke-all -> ") == 1 && logCount("register mask=12 owner=smoke-load -> ") == 1, "both registrations logged with mask and owner");
    check(current().empty(), "CurrentSaveFolder = none before any event");

    plantCloud(false, ""); plantRequest("SAVES:000012 - Game11"); plantLoad("SAVES:000013 - Quick");
    snapshotPad();                                         // everything planted; the DLL must not write any of it
    g_seen.clear();

    D.saved();
    check(g_seen.size() == 1 && g_seen[0].who == 1 && g_seen[0].kind == K2SE_SAVED, "SE_OnSaved: SAVED reaches the all-mask consumer only");
    check(lastOf(1) && lastOf(1)->folder == "saves/000012 - Game11" && lastOf(1)->cloud == 0, "SAVED folder = saves/000012 - Game11, cloud 0 (alias SAVES: resolved, request chain read)");
    check(lastOf(1) && lastOf(1)->size == sizeof(K2SaveEvent), "event struct_size == sizeof(K2SaveEvent)");
    check(current() == "saves/000012 - Game11", "CurrentSaveFolder follows the save");

    D.autosaved();
    check(countWho(1) == 2 && lastOf(1)->kind == K2SE_AUTOSAVED && lastOf(1)->folder == "saves/000001 - AUTOSAVE" && countWho(2) == 0, "SE_OnAutosaved: AUTOSAVED saves/000001 - AUTOSAVE, not to the load/new-game consumer");

    D.load("SAVES:000013 - Quick");   // the hook passes the engine's char*; here a plain local string (readable memory)
    D.load((const char*)LOADSTR);
    check(countWho(1) == 4 && countWho(2) == 2, "SE_OnLoadRequest x2: both consumers called each time");
    check(lastOf(2) && lastOf(2)->kind == K2SE_LOAD_REQUEST && lastOf(2)->folder == "saves/000013 - Quick" && lastOf(2)->cloud == 0, "LOAD_REQUEST folder = saves/000013 - Quick");
    check(current() == "saves/000013 - Quick", "CurrentSaveFolder follows the load request");

    D.autoLoad();
    check(lastOf(2) && lastOf(2)->kind == K2SE_LOAD_REQUEST && lastOf(2)->folder == "saves/000001 - AUTOSAVE", "SE_OnAutosaveSlotLoad: LOAD_REQUEST saves/000001 - AUTOSAVE");

    plantCloud(true, "cloudsaves/76561198000000001/");
    snapshotPad();
    D.saved();
    check(lastOf(1) && lastOf(1)->kind == K2SE_SAVED && lastOf(1)->cloud == 1 && lastOf(1)->folder == "cloudsaves/76561198000000001/000012 - Game11", "Steam Cloud on: SAVED folder under cloudsaves/<id>/, cloud 1");
    D.autoLoad();
    check(lastOf(2) && lastOf(2)->folder == "cloudsaves/76561198000000001/000001 - AUTOSAVE" && lastOf(2)->cloud == 1, "Steam Cloud on: autosave-slot load folder under cloudsaves/<id>/, cloud 1");
    check(current() == "cloudsaves/76561198000000001/000001 - AUTOSAVE", "CurrentSaveFolder carries the cloud root");

    const size_t before = g_seen.size();
    D.newGame();
    check(g_seen.size() == before + 2, "SE_OnNewGame: both consumers called");
    check(lastOf(1) && lastOf(1)->kind == K2SE_NEW_GAME && lastOf(1)->nullFolder && lastOf(1)->cloud == 0, "NEW_GAME event: folder NULL, cloud 0");
    check(current().empty(), "CurrentSaveFolder cleared by NEW_GAME");

    checkf(logCount("consumers=") == 8, "one evt log line per event (8 events), got %d", logCount("consumers="));
    check(logCount("SAVED folder='saves/000012 - Game11' cloud=0 consumers=1") == 1, "SAVED line: kind, folder, cloud, consumers called");
    check(logCount("LOAD_REQUEST folder='saves/000013 - Quick' cloud=0 consumers=2") == 2, "LOAD_REQUEST lines show 2 consumers");
    check(logCount("NEW_GAME consumers=2") == 1, "NEW_GAME line shows 2 consumers");
    check(logCount("cloud=1 consumers=") == 2, "the two cloud events log cloud=1");
    checkf(warnCount() == 0, "no WARN lines, got %d", warnCount());
    uintptr_t bad = 0;
    checkf(padUnchanged(&bad), "engine pad untouched (the DLL writes nothing), first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ dispatch
static const K2SaveEventsApi* g_t = nullptr;
static int32_t g_selfTok = 0, g_laterTok = 0;
static int g_selfCalls = 0, g_laterCalls = 0;
static void KPATCH_CALL onLater(const K2SaveEvent*, void*) { ++g_laterCalls; }
static void KPATCH_CALL onSelfRemoving(const K2SaveEvent*, void*) {
    ++g_selfCalls;
    g_t->Unregister(g_selfTok);
    g_laterTok = g_t->Register(K2SE_ALL, onLater, nullptr, "smoke-later");
}
static void KPATCH_CALL onSlow(const K2SaveEvent* e, void* u) { onEvent(e, u); Sleep(9); }
static void KPATCH_CALL onNop(const K2SaveEvent*, void*) {}

static int modeDispatch() {
    setupPad(); freshRegistry(); loadDll();
    if (!g_skipProvider) D.init(fakekp::api());
    const K2SaveEventsApi* t = table();
    check(t != nullptr, "provider table available");
    if (!t) { printf("saveevents: no provider table, stopping (%d failed)\n", g_fail); return 1; }
    g_t = t;
    plantCloud(false, ""); plantRequest("SAVES:000012 - Game11"); plantLoad("SAVES:000013 - Quick");

    // kind-mask filtering: one consumer per kind
    int32_t tk[4] = { regAll(11, K2SE_SAVED, "k-saved"), regAll(12, K2SE_AUTOSAVED, "k-auto"), regAll(13, K2SE_LOAD_REQUEST, "k-load"), regAll(14, K2SE_NEW_GAME, "k-new") };
    check(tk[0] > 0 && tk[1] > 0 && tk[2] > 0 && tk[3] > 0, "four single-kind registrations accepted");
    g_seen.clear();
    D.saved(); D.autosaved(); D.load((const char*)LOADSTR); D.newGame();
    check(countWho(11) == 1 && countWho(12) == 1 && countWho(13) == 1 && countWho(14) == 1 && g_seen.size() == 4, "each consumer saw exactly its own kind");
    check(lastOf(11)->kind == K2SE_SAVED && lastOf(12)->kind == K2SE_AUTOSAVED && lastOf(13)->kind == K2SE_LOAD_REQUEST && lastOf(14)->kind == K2SE_NEW_GAME, "event kinds match the masks");

    // unregister accounting
    t->Unregister(tk[0]);
    check(logCount("unregister ") >= 1 && logCount(("unregister " + std::to_string(tk[0]) + " -> 1").c_str()) == 1, "Unregister of a live token logged '-> 1'");
    t->Unregister(tk[0]); t->Unregister(0); t->Unregister(-5);
    check(logCount(("unregister " + std::to_string(tk[0]) + " -> 0").c_str()) == 1, "Unregister of the same token again logged '-> 0' and did not crash");
    g_seen.clear(); D.saved();
    check(countWho(11) == 0, "an unregistered consumer is not called");

    // re-entrancy: a callback unregisters itself and registers another consumer
    g_selfCalls = g_laterCalls = 0;
    g_selfTok = regAll(0, 0, nullptr);
    check(g_selfTok <= 0, "mask 0 is refused");
    g_selfTok = t->Register(K2SE_ALL, onSelfRemoving, nullptr, "smoke-self");
    check(g_selfTok > 0, "self-removing consumer registered");
    D.saved();
    check(g_selfCalls == 1 && g_laterCalls == 0, "first event: self-removing consumer ran once, the consumer it registered waits for the next event");
    D.saved();
    check(g_selfCalls == 1 && g_laterCalls == 1, "second event: self-removed consumer is gone, the registered one runs");

    // refused registrations
    check(t->Register(K2SE_ALL, nullptr, nullptr, "x") <= 0, "null callback refused");
    check(t->Register(0x10, onNop, nullptr, "x") <= 0, "a mask with only unknown bits refused");
    check(warnCount("register mask=") >= 2, "refusals logged as WARN");
    // slow consumer: sleeps 9 ms, logged once per event, later consumers still run.
    g_seen.clear();
    const int32_t slowTok = t->Register(K2SE_ALL, onSlow, (void*)(intptr_t)21, "smoke-slow");
    const int32_t fastTok = regAll(22, K2SE_ALL, "smoke-fast");
    check(slowTok > 0 && fastTok > 0, "slow + fast consumers registered");
    D.saved();
    check(countWho(21) == 1 && countWho(22) == 1, "the consumer after the slow one still ran");
    check(warnCount("slow consumer owner=smoke-slow kind=SAVED") == 1, "one slow-consumer WARN naming owner and kind");
    check(warnCount("smoke-fast") == 0, "the fast consumer was not reported");
    D.newGame();
    check(warnCount("slow consumer owner=smoke-slow kind=NEW_GAME") == 1 && countWho(22) == 2, "slow guard fires per event and dispatch keeps going");
    t->Unregister(slowTok);
    D.saved();
    check(warnCount("slow consumer") == 2, "no slow WARN once the slow consumer is unregistered");

    // CurrentSaveFolder truncation through the provider's table
    char b[64]; memset(b, 'x', sizeof b);
    int n = t->CurrentSaveFolder(b, sizeof b);
    check(n == 21 && !strcmp(b, "saves/000012 - Game11"), "CurrentSaveFolder returns the full length and the string");
    memset(b, 'x', sizeof b); n = t->CurrentSaveFolder(b, 8);
    check(n == 21 && !strcmp(b, "saves/0") && b[7] == 0 && b[8] == 'x', "truncated: NUL-terminated at cap-1, return value = full length");
    memset(b, 'x', sizeof b); n = t->CurrentSaveFolder(b, 1);
    check(n == 21 && b[0] == 0 && b[1] == 'x', "cap 1: just the NUL");
    memset(b, 'x', sizeof b); n = t->CurrentSaveFolder(b, 0);
    check(n == 21 && b[0] == 'x', "cap 0: nothing written, length returned");
    check(t->CurrentSaveFolder(nullptr, 0) == 21, "NULL buffer: length query");

    // table bounded (last: it leaves the registry full)
    int accepted = 0; for (int i = 0; i < 40; ++i) if (t->Register(K2SE_NEW_GAME, onNop, nullptr, "fill") > 0) ++accepted;
    checkf(accepted > 0 && accepted < 40, "the table is bounded: %d of 40 more registrations accepted, then refused", accepted);
    check(t->Register(K2SE_ALL, onNop, nullptr, "over") <= 0, "registration refused when the table is full");
    return g_fail ? 1 : 0;
}

// ================================================================ inert
static int modeInert() {
    setupPad(); freshRegistry(); loadDll();
    clearRequest(); plantCloud(false, ""); put32(APP + 0x14, 0);
    snapshotPad();

    // hooks before KPatchInit: the DLL has no registry yet and no log core; it must neither crash nor provide anything
    D.saved(); D.autosaved(); D.autoLoad(); D.newGame(); D.load(nullptr); D.load((const char*)0x1234);
    check(!fakekp::provided(K2SE_IFACE, K2SE_VERSION), "hooks before KPatchInit: no crash, nothing provided");

    // a KPatchApi too small to hold `require` (a patcher that predates the registry): the provider does not provide
    KPatchApi small = *fakekp::api(); small.struct_size = offsetof(KPatchApi, require);
    D.init(&small);
    check(!fakekp::provided(K2SE_IFACE, K2SE_VERSION), "KPatchApi without `require`: nothing provided");
    D.init(nullptr);
    check(!fakekp::provided(K2SE_IFACE, K2SE_VERSION), "KPatchInit(NULL): nothing provided, no crash");

    if (!g_skipProvider) D.init(fakekp::api());
    const K2SaveEventsApi* t = table();
    check(t != nullptr, "provider table available after a proper KPatchInit");
    if (!t) { printf("saveevents: no provider table, stopping (%d failed)\n", g_fail); return 1; }

    // zero consumers: valid engine data, events are logged with consumers=0, CurrentSaveFolder still tracks
    plantCloud(false, ""); plantRequest("SAVES:000012 - Game11"); plantLoad("SAVES:000013 - Quick");
    snapshotPad();
    D.saved(); D.autosaved(); D.load((const char*)LOADSTR); D.autoLoad();
    check(logCount("consumers=0") == 4, "four events with no consumer: 'consumers=0' logged each");
    check(current() == "saves/000001 - AUTOSAVE", "CurrentSaveFolder tracks events even with no consumer");
    D.newGame();
    check(logCount("NEW_GAME consumers=0") == 1 && current().empty(), "NEW_GAME with no consumer: logged, current cleared");
    check(warnCount() == 0, "no WARN for valid events without consumers");

    // unreadable / implausible engine data: nothing dispatched, WARN, current unchanged
    g_seen.clear();
    check(regAll(1, K2SE_ALL, "smoke-all") > 0, "consumer registered");
    D.saved();   // establishes a current folder
    check(countWho(1) == 1 && current() == "saves/000012 - Game11", "baseline event dispatched");
    g_seen.clear();
    const int warn0 = warnCount();
    clearRequest();                                                   // app global = 0
    D.saved();
    check(g_seen.empty() && warnCount() == warn0 + 1, "SE_OnSaved with a null app global: WARN, nothing dispatched");
    put32(AppGlobal, (uint32_t)APP); put32(APP + 0x14, 0);            // request pointer null
    D.saved();
    check(g_seen.empty() && warnCount() == warn0 + 2, "SE_OnSaved with a null request pointer: WARN, nothing dispatched");
    plantRequest("garbage");                                          // folder is not NNNNNN - Name
    D.saved();
    check(g_seen.empty() && warnCount() == warn0 + 3, "SE_OnSaved with a garbage folder: WARN, nothing dispatched");
    plantRequest("SAVES:00001x - Game11");
    D.saved();
    check(g_seen.empty() && warnCount() == warn0 + 4, "SE_OnSaved with a non-digit folder: WARN, nothing dispatched");
    D.load(nullptr);
    check(g_seen.empty() && warnCount() == warn0 + 5, "SE_OnLoadRequest(NULL): WARN, nothing dispatched");
    D.load((const char*)0x1234);
    check(g_seen.empty() && warnCount() == warn0 + 6, "SE_OnLoadRequest(small pointer): WARN, nothing dispatched");
    plantLoad("not a save");
    D.load((const char*)LOADSTR);
    check(g_seen.empty() && warnCount() == warn0 + 7, "SE_OnLoadRequest(garbage string): WARN, nothing dispatched");
    memset((void*)LOADSTR, 'A', 400); *(char*)(LOADSTR + 400) = 0;    // 400 chars: longer than any plausible folder
    D.load((const char*)LOADSTR);
    check(g_seen.empty() && warnCount() == warn0 + 8, "SE_OnLoadRequest(over-long string): WARN, nothing dispatched");
    check(current() == "saves/000012 - Game11", "CurrentSaveFolder unchanged by every rejected event");
    D.newGame();
    check(countWho(1) == 1 && lastOf(1)->kind == K2SE_NEW_GAME, "NEW_GAME needs no engine read and still dispatches");

    uintptr_t bad = 0;
    // the planted region was rewritten by this mode itself after the last snapshot: re-take it and prove the hooks do not write
    snapshotPad(); D.saved(); D.autosaved(); D.load((const char*)LOADSTR); D.autoLoad(); D.newGame();
    checkf(padUnchanged(&bad), "engine pad untouched by the hooks, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "wire";
    g_skipProvider = (argc > 2 && !strcmp(argv[2], "skip-provider")) || (getenv("SMOKE_SKIP_PROVIDER") && *getenv("SMOKE_SKIP_PROVIDER") == '1');
    printf("saveevents: mode %s%s\n", mode, g_skipProvider ? " (provider init SKIPPED: red-proof run, expected to fail)" : "");
    int rc;
    if (!strcmp(mode, "wire")) rc = modeWire();
    else if (!strcmp(mode, "dispatch")) rc = modeDispatch();
    else if (!strcmp(mode, "inert")) rc = modeInert();
    else die(2, "unknown mode (wire|dispatch|inert)");
    printf("saveevents: %s %d checks, %d failed\n", rc ? "FAIL" : "PASS", g_checks, g_fail);
    return rc;
}
