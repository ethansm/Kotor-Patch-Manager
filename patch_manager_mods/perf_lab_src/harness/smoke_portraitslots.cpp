// Smoke test for the split portrait-slots trio (doc 24 batch T6): portrait-slots (provider of "k2.portraitslots" v1) and its two clients,
// buff-duration-hud and shield-bar, loaded TOGETHER under Wine's real PE loader and wired through the registry exactly as KPM would.
// Build: i686-w64-mingw32-g++ -std=c++17 -O1 -I <harness> -I tests/include -I$KPM/Patches/Common -I../../_shared_src smoke_portraitslots.cpp -static
//        (runner flags in smokes.toml; the 7 MB .bss pad must start at or below 0x479AB0 and end above 0xA31000 so the engine addresses the
//        Steam table names -- HudScale 0x479AB0, ClipDepth 0xA309C0 -- are ordinary zeroed memory of this exe; checked at startup, exit 2)
// Run:   smoke_portraitslots.exe <wire|late|inert|order> [skip-provider]   (dir needs portrait-slots.dll, buff-duration-hud.dll, shield-bar.dll).
//        Exit codes: 0 pass, 1 check failed / LoadLibrary failed, 2 setup failure, 3 missing export.
//        Red-proof: argv[2] == "skip-provider" or env SMOKE_SKIP_PROVIDER=1 skips the provider's KPatchInit; wire and late must then FAIL.
//
// What is real: the three DLLs, their KPatchInit / hook exports (called with the KPM hook calling convention: cdecl, int params in the order
// of each kotor2-steam-aspyr.hooks.toml), the k2addr table selection (env KOTOR_VERSION_SHA), the registry semantics (tests/include/
// FakeKPatchApi.h = KPM registry.cpp) and a fake "kpatch.log" sink. What is faked: engine memory = a zeroed pad; the engine HudScale function
// (a counting stub at its Steam address, the only engine call the HUD frame reaches over zeroed memory). No test exports exist or are used:
// everything is observed through the registry, the provider's own interface table, the fake log, the clients' own debug log
// (buffhud_log.txt, enabled by an empty buffhud_debug.txt) and an IAT tap on GetFileAttributesA (dispatch order).
//
//   wire   provider -> buff -> shield. Provider provided + table complete, 3 + 5 registrations with the exact (phase, prio, owner) sets,
//          snapshot -> CreatureForSlot / SlotAt round trip, clients see the provider's creature, leader calls advance Frame() and reach the
//          shield's HUD_DRAW (HudScale stub hit), buff's first-frame note logged, no WARN, pad untouched.
//   late   clients first (provider absent): nothing registered, require() rate-limited to 1 per 256 hook calls; provider init; still nothing
//          at call 512; both clients connect exactly at call 513; then the dispatch works.
//   inert  KOTOR_VERSION_SHA = unknown sha: provider snapshot / dispatch are no-ops (one WARN), clients' hooks AND every registered callback
//          (driven by a fake provider that captures them) do nothing: no HudScale call, pad all zero, "k2addr: icon columns disabled" logged.
//   order  buff registered BEFORE shield; at frame 30 and 60 the clients' kill-switch file checks must run shield (prio 100) before buff (200).
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
#include <algorithm>

#include "FakeKPatchApi.h"
#include "K2Addr.h"
#include "PortraitSlots.h"

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what) {
    ++g_checks;
    printf("portraitslots: %s %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) ++g_fail;
}
static void checkf(bool ok, const char* fmt, int a = 0, int b = 0, int c = 0) {
    char buf[256]; snprintf(buf, sizeof buf, fmt, a, b, c); check(ok, buf);
}
[[noreturn]] static void die(int code, const char* msg) { printf("portraitslots: %s\n", msg); fflush(stdout); exit(code); }

// ---------------------------------------------------------------- engine memory: the zeroed pad (see equipiconfix / robustness smokes)
static char pad[0x700000];
static const uintptr_t HudScaleAddr = kSteam.HudScale;
static const uintptr_t ClipDepthAddr = kSteam.ClipDepth;
static void unprotect(uintptr_t addr, unsigned size) {
    DWORD o;
    if (!VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &o)) { printf("portraitslots: VirtualProtect %08x failed %lu\n", (unsigned)addr, GetLastError()); exit(2); }
}
static volatile long g_hudScaleCalls = 0;
static float __cdecl FakeHudScale() { ++g_hudScaleCalls; return 1.0f; }
static void putJmp(uintptr_t at, void* to) {   // mov eax, imm32 ; jmp eax
    unsigned char b[7] = { 0xB8, 0, 0, 0, 0, 0xFF, 0xE0 }; uintptr_t t = (uintptr_t)to; memcpy(b + 1, &t, 4); memcpy((void*)at, b, 7);
}
// true when every pad byte is zero except the planted stub; *at = first offending address
static bool padClean(uintptr_t* at) {
    const uintptr_t s0 = HudScaleAddr, s1 = HudScaleAddr + 16;
    for (size_t i = 0; i < sizeof pad; ++i) {
        uintptr_t a = (uintptr_t)pad + i;
        if (pad[i] != 0 && !(a >= s0 && a < s1)) { *at = a; return false; }
    }
    return true;
}

// pad addresses used as fake engine objects (all plain zeroed memory)
static int SLOT_A, CREATURE_A, SLOT_B, CREATURE_B, WIDGET, BUTTON;

// ---------------------------------------------------------------- files
static std::string readFile(const char* name) {
    std::string s; FILE* f = fopen(name, "rb"); if (!f) return s;
    char buf[4096]; size_t n; while ((n = fread(buf, 1, sizeof buf, f)) > 0) s.append(buf, n);
    fclose(f); return s;
}
static bool has(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }
static void touch(const char* name) { FILE* f = fopen(name, "wb"); if (f) fclose(f); }

// ---------------------------------------------------------------- fake kpatch.log inspection
static int logCount(const char* sub) {
    int n = 0; for (const std::string& l : fakekp::state().logLines) if (l.find(sub) != std::string::npos) ++n; return n;
}
static int warnCount() {   // fake Submit records "<channel>:<level>:<text>", level 1 = KPLOG_WARN
    int n = 0; for (const std::string& l : fakekp::state().logLines) if (l.find(":1:") == 1) ++n; return n;
}
struct Reg { int phase, prio, token; std::string owner; };
static std::vector<Reg> regsFromLog() {   // provider logs "register phase=%d prio=%d owner=%s -> %d" per apiRegister call
    std::vector<Reg> v;
    for (const std::string& l : fakekp::state().logLines) {
        const char* p = strstr(l.c_str(), "register phase="); if (!p) continue;
        Reg r; char own[64] = {};
        if (sscanf(p, "register phase=%d prio=%d owner=%63s -> %d", &r.phase, &r.prio, own, &r.token) == 4) { r.owner = own; v.push_back(r); }
    }
    return v;
}
static bool regSetIs(const std::vector<Reg>& all, const char* owner, std::vector<std::pair<int, int>> want) {
    std::vector<std::pair<int, int>> got; bool tokens = true;
    for (const Reg& r : all) if (r.owner == owner) { got.push_back({ r.phase, r.prio }); tokens &= r.token > 0; }
    std::sort(got.begin(), got.end()); std::sort(want.begin(), want.end());
    return tokens && got == want;
}
static const std::vector<std::pair<int, int>> kBuffRegs = { { K2PS_PRE_ENGINE_READ, K2PS_PRIO_BUFF }, { K2PS_FRAME_BEGIN, K2PS_PRIO_BUFF }, { K2PS_MENU_OVERLAY, K2PS_PRIO_BUFF } };
static const std::vector<std::pair<int, int>> kShieldRegs = { { K2PS_SNAPSHOT, K2PS_PRIO_SHIELD }, { K2PS_FRAME_BEGIN, K2PS_PRIO_SHIELD }, { K2PS_SLOT, K2PS_PRIO_SHIELD },
                                                              { K2PS_MENU_OVERLAY, K2PS_PRIO_SHIELD }, { K2PS_HUD_DRAW, K2PS_PRIO_SHIELD } };

// ---------------------------------------------------------------- the three DLLs
typedef void (__cdecl *InitFn)(const KPatchApi*);
typedef void (__cdecl *SnapFn)(int, int);
typedef void (__cdecl *SlotDrawFn)(int, int, int);
typedef void (__cdecl *RetargetFn)(int, int, int, int);
typedef void (__cdecl *VoidFn)();
typedef void (__cdecl *ColumnFn)(int, int, int, int, int, int);
typedef void (__cdecl *TipFn)(int);
struct Dlls {
    HMODULE ps = nullptr, buff = nullptr, shield = nullptr;
    InitFn psInit = nullptr, buffInit = nullptr, shieldInit = nullptr;
    SnapFn snapshot = nullptr; SlotDrawFn slotDraw = nullptr;                                  // provider hooks 1a/1b and 6
    RetargetFn retarget = nullptr; VoidFn restore = nullptr; ColumnFn column = nullptr;        // buff hooks 2, 3, 5
    TipFn tip = nullptr;                                                                       // shield tooltip hook
};
static Dlls D;
static HMODULE loadDll(const char* n) {
    HMODULE m = LoadLibraryA(n);
    if (!m) { printf("portraitslots: LoadLibrary %s failed %lu\n", n, GetLastError()); exit(1); }
    return m;
}
static FARPROC need(HMODULE m, const char* dll, const char* name) {
    FARPROC p = GetProcAddress(m, name);
    if (!p) { printf("portraitslots: %s missing export %s\n", dll, name); exit(3); }
    return p;
}
static void loadAll() {
    D.ps = loadDll("portrait-slots.dll"); D.buff = loadDll("buff-duration-hud.dll"); D.shield = loadDll("shield-bar.dll");
    D.psInit = (InitFn)need(D.ps, "portrait-slots.dll", "KPatchInit");
    D.snapshot = (SnapFn)need(D.ps, "portrait-slots.dll", "PS_SnapshotPortraitSlot");
    D.slotDraw = (SlotDrawFn)need(D.ps, "portrait-slots.dll", "PS_PortraitSlotDraw");
    D.buffInit = (InitFn)need(D.buff, "buff-duration-hud.dll", "KPatchInit");
    D.retarget = (RetargetFn)need(D.buff, "buff-duration-hud.dll", "RetargetBuffIcon");
    D.restore = (VoidFn)need(D.buff, "buff-duration-hud.dll", "RestoreBuffIcon");
    D.column = (ColumnFn)need(D.buff, "buff-duration-hud.dll", "DrawBuffColumn");
    D.shieldInit = (InitFn)need(D.shield, "shield-bar.dll", "KPatchInit");
    D.tip = (TipFn)need(D.shield, "shield-bar.dll", "AppendShieldToTooltip");
}

static bool g_skipProvider = false;
static const K2PortraitSlotsApi* providerTable() { return (const K2PortraitSlotsApi*)fakekp::api()->require(K2PS_IFACE, K2PS_VERSION); }

static void setup(bool inert) {
    if ((uintptr_t)pad > HudScaleAddr || (uintptr_t)pad + sizeof pad < ClipDepthAddr + 0x1000) {
        printf("portraitslots: pad %p..%p does not cover HudScale %08x .. ClipDepth %08x\n", (void*)pad, (void*)(pad + sizeof pad), (unsigned)HudScaleAddr, (unsigned)ClipDepthAddr);
        exit(2);
    }
    unprotect(HudScaleAddr, 16);
    putJmp(HudScaleAddr, (void*)&FakeHudScale);
    const int base = (int)(uintptr_t)pad;
    SLOT_A = base + 0x10000; CREATURE_A = base + 0x20000; SLOT_B = base + 0x30000; CREATURE_B = base + 0x40000; WIDGET = base + 0x50000; BUTTON = base + 0x60000;
    // address table selection happens inside each DLL on first use: Steam when unset, none for an unknown sha
    SetEnvironmentVariableA("KOTOR_VERSION_SHA", inert ? "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" : nullptr);
    _putenv(inert ? "KOTOR_VERSION_SHA=0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" : "KOTOR_VERSION_SHA=");
    if (!inert) SetEnvironmentVariableA("KOTOR_VERSION_SHA", nullptr);
    DeleteFileA("buffhud_log.txt"); DeleteFileA("shieldprobe.txt"); DeleteFileA("shieldprobe_log.txt");
    DeleteFileA("buffhud_off.txt"); DeleteFileA("shield_nodraw.txt");
    touch("buffhud_debug.txt");   // turns on the buff HUD's own capped debug log (read below)
    fakekp::reset();
    fakekp::withLog();
}

// leader call = isLarge 1 (advances the frame counter, FRAME_BEGIN, HUD_DRAW); callerEbp is only read in menu frames
static void leaderCalls(int n, int slot) { for (int i = 0; i < n; ++i) D.slotDraw(slot, 1, 0); }

// ================================================================ wire
static int modeWire() {
    setup(false);
    loadAll();
    if (!g_skipProvider) D.psInit(fakekp::api());
    D.buffInit(fakekp::api());
    D.shieldInit(fakekp::api());

    check(fakekp::provided(K2PS_IFACE, K2PS_VERSION), "provider provided k2.portraitslots v1 through the registry");
    check(logCount("provided k2.portraitslots v1") == 1, "provider logged its 'provided' line once");
    const K2PortraitSlotsApi* ps = providerTable();
    check(ps != nullptr, "require(k2.portraitslots, 1) returns a table");
    if (!ps) { printf("portraitslots: no provider table, stopping (%d failed)\n", g_fail); return 1; }
    check(ps->struct_size == sizeof(K2PortraitSlotsApi), "table struct_size == sizeof(K2PortraitSlotsApi)");
    check(ps->Register && ps->Unregister && ps->CreatureForSlot && ps->SlotAt && ps->Frame, "table Register / Unregister / CreatureForSlot / SlotAt / Frame non-null");
    check(k2ps::Client::usable(ps), "table passes the client helper's usable() gate");
    check(!fakekp::provided("k2.portraitslots", 2), "no other major version provided");

    const std::vector<Reg> regs = regsFromLog();
    checkf(regs.size() == 8, "8 registrations in total (3 buff + 5 shield), got %d", (int)regs.size());
    check(regSetIs(regs, "buff-duration-hud", kBuffRegs), "buff-duration-hud registered PRE_ENGINE_READ / FRAME_BEGIN / MENU_OVERLAY at prio 200");
    check(regSetIs(regs, "shield-bar", kShieldRegs), "shield-bar registered SNAPSHOT / FRAME_BEGIN / SLOT / MENU_OVERLAY / HUD_DRAW at prio 100");
    check(ps->Frame() == 0, "frame counter 0 before any hook");

    // before the provider knows the slot, the buff client finds no creature
    D.retarget(SLOT_A, 0, WIDGET, 1);
    char want[160];
    snprintf(want, sizeof want, "arrow slot=%08x idx=0: NO creature in side table", (unsigned)SLOT_A);
    check(has(readFile("buffhud_log.txt"), want), "buff client: no creature for a slot the provider has not snapshotted");

    // UpdatePortraits hook (both addresses use the one export): slot -> creature table
    D.snapshot(SLOT_A, CREATURE_A);
    D.snapshot(SLOT_B, CREATURE_B);
    check(ps->CreatureForSlot(SLOT_A) == CREATURE_A && ps->CreatureForSlot(SLOT_B) == CREATURE_B, "CreatureForSlot returns each snapshotted creature");
    check(ps->CreatureForSlot(SLOT_A + 4) == 0, "CreatureForSlot of an unknown slot is 0");
    int s = 0, c = 0;
    check(ps->SlotAt(0, &s, &c) == 1 && s == SLOT_A && c == CREATURE_A, "SlotAt(0) = first snapshotted slot");
    check(ps->SlotAt(1, &s, &c) == 1 && s == SLOT_B && c == CREATURE_B, "SlotAt(1) = second snapshotted slot");
    check(ps->SlotAt(2, &s, &c) == 0 && ps->SlotAt(-1, &s, &c) == 0 && ps->SlotAt(K2PS_MAX_SLOTS, &s, &c) == 0, "SlotAt of an empty / out-of-range index is 0");

    // the clients read the provider's table through the interface
    D.retarget(SLOT_A, 0, WIDGET, 1);
    snprintf(want, sizeof want, "arrow row=1 slot=%08x idx=0 tracked=0 -> keep", (unsigned)SLOT_A);
    check(has(readFile("buffhud_log.txt"), want), "buff client: RetargetBuffIcon sees the provider's creature (tracked=0 -> keep)");
    D.restore();
    D.column(SLOT_A, WIDGET, 1, 1, 100, 100);
    snprintf(want, sizeof want, "slot=%08x large=1 card=100x100", (unsigned)SLOT_A);
    std::string bl = readFile("buffhud_log.txt");
    check(has(bl, "COLUMN#") && has(bl, want), "buff client: DrawBuffColumn ran its column layout for the provider's creature");
    D.tip(BUTTON);   // zeroed charButton: nothing to append, must return cleanly

    // per-frame hook: leader calls advance the provider's frame counter and dispatch the phases to both clients
    const long hs0 = g_hudScaleCalls;
    const int N = 64;
    leaderCalls(N, SLOT_A);
    for (int i = 0; i < 8; ++i) D.slotDraw(SLOT_B, 0, 0);   // non-leader slots: PRE_ENGINE_READ + SLOT only
    check(ps->Frame() == (uint32_t)N, "frame counter advanced once per leader call");
    check(g_hudScaleCalls - hs0 >= N, "shield-bar HUD_DRAW / SLOT callbacks ran through the provider dispatch (HudScale stub hit)");
    bl = readFile("buffhud_log.txt");
    check(has(bl, "portrait-slots ok: registered"), "buff client logged its first-frame connect note (FRAME_BEGIN callback ran)");
    check(!has(bl, "MISSING") && !has(bl, "k2addr:"), "buff client: no MISSING note, no address-table complaint on the Steam table");
    check(warnCount() == 0, "no WARN lines from the provider on the Steam table");

    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched (all zero but the HudScale stub), first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ late
static int modeLate() {
    setup(false);
    loadAll();
    D.buffInit(fakekp::api());
    D.shieldInit(fakekp::api());
    check(!fakekp::provided(K2PS_IFACE, K2PS_VERSION), "provider absent at client init");
    check(regsFromLog().empty() && logCount("provided") == 0, "nothing registered or provided yet");
    const int requires0 = fakekp::state().requireCalls;   // each client's KPatchInit connect() = 1 require

    // first poll() call tries at once (call 1), then every 256th (call 257): 2 more requires per client in the first 300 calls
    for (int i = 0; i < 300; ++i) { D.retarget(SLOT_A, 0, 0, 1); D.tip(0); }
    checkf(fakekp::state().requireCalls == requires0 + 4, "require rate-limited: 4 retries (2 per client) over 300 hook calls, got %d", fakekp::state().requireCalls - requires0);
    check(regsFromLog().empty(), "still nothing registered after 300 client hook calls without a provider");

    if (!g_skipProvider) D.psInit(fakekp::api());
    check(fakekp::provided(K2PS_IFACE, K2PS_VERSION), "provider initialised after the clients");
    check(regsFromLog().empty(), "clients are not connected right after provider init (they poll)");
    for (int i = 300; i < 512; ++i) { D.retarget(SLOT_A, 0, 0, 1); D.tip(0); }
    check(regsFromLog().empty(), "still not connected after 512 hook calls each (no poll boundary since call 257)");
    D.retarget(SLOT_A, 0, 0, 1);   // call 513: pollCount == 512 -> one require -> connect
    std::vector<Reg> regs = regsFromLog();
    check(regSetIs(regs, "buff-duration-hud", kBuffRegs) && regs.size() == 3, "buff client connected at call 513 and registered its 3 callbacks");
    check(regSetIs(regs, "shield-bar", {}), "shield client has not connected yet (its own call count is still 512)");
    D.tip(0);
    regs = regsFromLog();
    check(regSetIs(regs, "shield-bar", kShieldRegs) && regs.size() == 8, "shield client connected at its call 513 and registered its 5 callbacks");

    const K2PortraitSlotsApi* ps = providerTable();
    if (!ps) { printf("portraitslots: no provider table, stopping (%d failed)\n", g_fail); return 1; }
    D.snapshot(SLOT_A, CREATURE_A);
    const long hs0 = g_hudScaleCalls;
    leaderCalls(40, SLOT_A);
    check(ps->Frame() == 40, "provider frame counter advanced by the leader calls");
    check(g_hudScaleCalls - hs0 >= 40, "late-registered shield callbacks run through the provider dispatch (HudScale stub hit)");
    std::string bl = readFile("buffhud_log.txt");
    check(has(bl, "portrait-slots ok: registered"), "late buff client logged 'portrait-slots ok: registered' on its first frame");
    check(!has(bl, "MISSING"), "late buff client never logged MISSING");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ inert
// A fake provider that captures the clients' callbacks, so the smoke can run every registered callback itself (as the real provider's
// dispatcher would) while the clients' k2addr table is NULL.
struct Cb { int phase; K2PsCallback fn; void* user; };
static std::vector<Cb> g_cbs;
static int32_t KPATCH_CALL fpRegister(int32_t phase, int32_t, K2PsCallback fn, void* user, const char*) {
    if (!fn) return -1; g_cbs.push_back({ phase, fn, user }); return (int32_t)g_cbs.size();
}
static void KPATCH_CALL fpUnregister(int32_t) {}
static int32_t KPATCH_CALL fpCreatureForSlot(int32_t slot) { return slot == SLOT_A ? CREATURE_A : slot == SLOT_B ? CREATURE_B : 0; }
static int32_t KPATCH_CALL fpSlotAt(int32_t i, int32_t* s, int32_t* c) {
    if (i < 0 || i > 1) return 0; if (s) *s = i ? SLOT_B : SLOT_A; if (c) *c = i ? CREATURE_B : CREATURE_A; return 1;
}
static uint32_t KPATCH_CALL fpFrame() { return 0; }
static K2PortraitSlotsApi g_fakePs = { sizeof(K2PortraitSlotsApi), fpRegister, fpUnregister, fpCreatureForSlot, fpSlotAt, fpFrame };
static const void* KPATCH_CALL clientRequire(const char* n, uint32_t v) {
    if (n && !strcmp(n, K2PS_IFACE) && v == K2PS_VERSION) return &g_fakePs;
    return fakekp::requireImpl(n, v);
}
static KPatchApi g_clientApi;

static int modeInert() {
    setup(true);
    g_clientApi = { sizeof(KPatchApi), KPATCH_ABI_VERSION, fakekp::provideImpl, clientRequire };
    loadAll();
    D.psInit(fakekp::api());        // the real provider on the real registry
    D.buffInit(&g_clientApi);       // the clients see the capturing fake provider
    D.shieldInit(&g_clientApi);
    check(fakekp::provided(K2PS_IFACE, K2PS_VERSION), "real provider still provides on an unknown exe (KPatchInit never touches the table)");
    checkf(g_cbs.size() == 8, "clients registered their 8 callbacks with the fake provider, got %d", (int)g_cbs.size());

    const int N = 400;
    for (int i = 0; i < N; ++i) {
        D.snapshot(SLOT_A, CREATURE_A);                       // both UpdatePortraits sites
        D.snapshot(SLOT_B, CREATURE_B);
        D.slotDraw(SLOT_A, 1, 0);                             // leader, HUD frame
        D.slotDraw(SLOT_B, 0, 0);                             // follower
        D.slotDraw(SLOT_A, 1, SLOT_B);                        // leader with a caller frame pointer
        D.retarget(SLOT_A, i & 3, WIDGET, i & 1);
        D.restore();
        D.column(SLOT_A, WIDGET, 1, i & 1, 100, 100);
        D.column(SLOT_B, WIDGET, 0, i & 1, 60, 60);
        D.tip(BUTTON);
        // every phase of the fake dispatcher: menu context with companion controls, HUD context with a live slot
        K2PsContext ctx = {};
        ctx.struct_size = sizeof ctx; ctx.slot = SLOT_A; ctx.creature = CREATURE_A; ctx.isLarge = 1; ctx.frame = (uint32_t)i;
        ctx.panel = WIDGET; ctx.menuCreature = CREATURE_A; ctx.companions[0] = CREATURE_B; ctx.companions[1] = CREATURE_A;
        ctx.companionCtrl[0] = BUTTON; ctx.companionCtrl[1] = BUTTON + 0x1000;
        ctx.panelRect[2] = 640; ctx.panelRect[3] = 480;
        for (const Cb& cb : g_cbs) { ctx.phase = cb.phase; cb.fn(&ctx, cb.user); }
    }

    const K2PortraitSlotsApi* ps = providerTable();
    if (!ps) { printf("portraitslots: no provider table, stopping (%d failed)\n", g_fail); return 1; }
    check(ps->CreatureForSlot(SLOT_A) == 0 && ps->CreatureForSlot(SLOT_B) == 0, "provider snapshot dropped on an unknown exe: CreatureForSlot is 0");
    int s = 0, c = 0;
    check(ps->SlotAt(0, &s, &c) == 0 && ps->SlotAt(1, &s, &c) == 0, "provider snapshot dropped on an unknown exe: SlotAt finds nothing");
    check(ps->Frame() == 0, "provider frame counter never advanced (no dispatch on an unknown exe)");
    checkf(warnCount() == 1, "exactly one provider WARN over %d hook rounds, got %d", N, warnCount());
    check(logCount("k2addr:") == 1, "the one WARN is the k2addr table complaint");
    checkf(g_hudScaleCalls == 0, "no engine HudScale call from any hook or client callback, got %d", (int)g_hudScaleCalls);
    std::string bl = readFile("buffhud_log.txt");
    check(has(bl, "k2addr: icon columns disabled"), "buff client logged 'icon columns disabled' (its own table gate, DrawBuffColumn reached with a creature)");
    check(!has(bl, "COLUMN#"), "buff client drew no column layout");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad all zero after the hammering, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ order
static std::vector<std::string> g_fileChecks;
typedef DWORD (WINAPI *GetFileAttrFn)(LPCSTR);
static GetFileAttrFn g_realGfa = nullptr;
static DWORD WINAPI TapGetFileAttributesA(LPCSTR n) {
    if (n) g_fileChecks.push_back(n);
    return g_realGfa(n);
}
// Replace one named kernel32 import in m's IAT. false when the DLL does not import it by name.
static bool tapImport(HMODULE m, const char* lib, const char* fn, void* repl, void** orig) {
    BYTE* b = (BYTE*)m;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(b + ((IMAGE_DOS_HEADER*)b)->e_lfanew);
    IMAGE_DATA_DIRECTORY dd = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dd.VirtualAddress) return false;
    for (IMAGE_IMPORT_DESCRIPTOR* d = (IMAGE_IMPORT_DESCRIPTOR*)(b + dd.VirtualAddress); d->Name; ++d) {
        if (_stricmp((const char*)(b + d->Name), lib) != 0) continue;
        IMAGE_THUNK_DATA* names = (IMAGE_THUNK_DATA*)(b + (d->OriginalFirstThunk ? d->OriginalFirstThunk : d->FirstThunk));
        IMAGE_THUNK_DATA* addrs = (IMAGE_THUNK_DATA*)(b + d->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++addrs) {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
            if (strcmp((const char*)((IMAGE_IMPORT_BY_NAME*)(b + names->u1.AddressOfData))->Name, fn) != 0) continue;
            DWORD old;
            if (!VirtualProtect(&addrs->u1.Function, sizeof(DWORD_PTR), PAGE_READWRITE, &old)) return false;
            if (*orig == nullptr) *orig = (void*)addrs->u1.Function;
            addrs->u1.Function = (DWORD_PTR)repl;
            VirtualProtect(&addrs->u1.Function, sizeof(DWORD_PTR), old, &old);
            return true;
        }
    }
    return false;
}

static int modeOrder() {
    setup(false);
    loadAll();
    D.psInit(fakekp::api());
    D.buffInit(fakekp::api());      // buff registers BEFORE shield: registration order is the opposite of the priority order
    D.shieldInit(fakekp::api());

    const std::vector<Reg> regs = regsFromLog();
    check(regs.size() == 8, "8 registrations");
    check(regs.size() == 8 && regs[0].owner == "buff-duration-hud" && regs[3].owner == "shield-bar", "registration order is buff first, shield second (opposite of priority order)");
    bool b = tapImport(D.buff, "kernel32.dll", "GetFileAttributesA", (void*)&TapGetFileAttributesA, (void**)&g_realGfa);
    bool s = tapImport(D.shield, "kernel32.dll", "GetFileAttributesA", (void*)&TapGetFileAttributesA, (void**)&g_realGfa);
    check(b && s, "IAT tap on GetFileAttributesA installed in buff-duration-hud and shield-bar");
    if (!(b && s) || !g_realGfa) die(2, "cannot observe dispatch order: GetFileAttributesA not imported by name");
    g_fileChecks.clear();

    leaderCalls(61, SLOT_A);        // frames 1..61: both clients poll their kill-switch file at frame 30 and 60
    std::vector<std::string> seq;
    for (const std::string& n : g_fileChecks) if (n == "shield_nodraw.txt" || n == "buffhud_off.txt") seq.push_back(n);
    std::string joined; for (const std::string& n : seq) { if (!joined.empty()) joined += ","; joined += n; }
    printf("portraitslots: observed kill-switch file checks: %s\n", joined.c_str());
    check(seq.size() == 4, "each client polled its kill-switch file at frames 30 and 60 (4 checks in 61 frames)");
    check(seq.size() == 4 && seq[0] == "shield_nodraw.txt" && seq[1] == "buffhud_off.txt" && seq[2] == "shield_nodraw.txt" && seq[3] == "buffhud_off.txt",
          "FRAME_BEGIN dispatch: shield-bar (prio 100) runs before buff-duration-hud (prio 200) at both frames");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "wire";
    g_skipProvider = (argc > 2 && !strcmp(argv[2], "skip-provider")) || (getenv("SMOKE_SKIP_PROVIDER") && *getenv("SMOKE_SKIP_PROVIDER") == '1');
    printf("portraitslots: mode %s%s\n", mode, g_skipProvider ? " (provider init SKIPPED: red-proof run, expected to fail)" : "");
    int rc;
    if (!strcmp(mode, "wire")) rc = modeWire();
    else if (!strcmp(mode, "late")) rc = modeLate();
    else if (!strcmp(mode, "inert")) rc = modeInert();
    else if (!strcmp(mode, "order")) rc = modeOrder();
    else die(2, "unknown mode (wire|late|inert|order)");
    printf("portraitslots: %s %d checks, %d failed\n", rc ? "FAIL" : "PASS", g_checks, g_fail);
    return rc;
}
