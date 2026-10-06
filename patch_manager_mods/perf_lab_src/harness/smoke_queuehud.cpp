// Smoke test for queue-hud 0.4.0-dev (0.1.0 base commit 66b3702): the out-of-combat copy of the vanilla action-queue panel. Loaded under Wine's real PE
// loader TOGETHER with portrait-slots (the provider of "k2.portraitslots" v1), wired through the registry exactly as KPM would.
// Build: i686-w64-mingw32-g++ -std=c++17 -O1 -I <harness> -I tests/include -I$KPM/Patches/Common -I../../_shared_src smoke_queuehud.cpp -static
//        (runner flags in smokes.toml, incl. --image-base=0x120000; the 10 MB .bss pad must start at or below the lowest engine address stubbed below, TexBasicCtor 0x423000,
//        and end above AppGlobal / ClipDepth, so every engine address the DLLs touch is ordinary zeroed memory of this exe; checked at
//        startup, exit 2)
// Run:   smoke_queuehud.exe <wire|combat|nocreature|badhud|badchain|strips|cancel|inert>   (dir needs queue-hud.dll + portrait-slots.dll).
//        Exit codes: 0 pass, 1 check failed / LoadLibrary failed, 2 setup failure, 3 missing export.
//
// What is real: both DLLs, QueueHud's KPatchInit / QueueHudCapture (called with the KPM hook calling convention: cdecl, one int param =
// the HUD pointer, per kotor2-steam-aspyr.hooks.toml), the portrait-slots dispatcher (PS_PortraitSlotDraw leader calls drive the real
// HUD_DRAW phase), the k2addr Steam table, saferead, the registry semantics (tests/include/FakeKPatchApi.h) and a fake "kpatch.log" sink.
// What is faked: engine memory = a zeroed pad; a fake HUD object (a heap buffer, flags dword at +0xA904); and TRIPWIRES planted at every
// engine function a draw can reach (Begin/EndGui2D, Push/PopClipRect, FlushGuiBatch, HudScale, the icon resolver 0x74E0B0, GuiLoadImage,
// ServerClearAllActions): each only counts its calls, and every mode other than a real draw requires all counts to stay 0 ("draws nothing").
// 0.3.0 also plants STUBS (not tripwires) for the embtex engine calls the FRAME_BEGIN callback makes on its first frame (engine new / new[],
// CAurTextureBasic ctor, pending-list add, Reset; the pad is zeroed code, so executing them unstubbed is an access violation): they hand out
// heap memory and count calls; the first leader call must create exactly the 5 HudKit textures once, never Reset. No test exports are used: the
// callback is observed through its own log lines ("hidden (<reason>)"), the registry and the tripwires.
//
//   wire        provider -> queue-hud. KPatchInit OK, provided + exports present, exactly TWO registrations, both owner "queue-hud":
//               K2PS_HUD_DRAW (5) prio 400 and K2PS_FRAME_BEGIN (2) prio 300; the client connected; the one expected WARN is hud-host's
//               'NOT connected (no Cancel controls)' (hud-host is not loaded in this harness).
//   combat      QueueHudCapture(hud) with flags bit1 set: logs combat=1 once; leader calls dispatch HUD_DRAW: "hidden (combat)", no
//               tripwire hit, no crash. Bit 1 clear (other bits set) flips it to combat=0; set again flips back.
//   nocreature  combat=0, zeroed game memory (party chain reads 0): the callback returns "hidden (no creature)" cleanly; then a capture
//               older than 250 ms: "hidden (stale capture)". No tripwire hit.
//   badhud      QueueHudCapture(unreadable pointer): treated as combat (hidden), no crash; HUD_DRAW logs "hidden (hud unreadable)".
//   badchain    combat=0 with the AppGlobal party chain broken at each link / pointing at unreadable memory: saferead fails cleanly, the
//               callback returns, no tripwire hit, engine memory untouched.
//   strips      0.2.0 companion strips: 3 fake HUD slots (snapshotted into the real provider) while the party chain is unreadable, then
//               readable-but-empty, then served by fake PartyGetAt / MemberServerCreature stubs (members 1-2 matched by CLIENT and by SERVER
//               pointer, the leader never): bad / zero rects, empty queues, an unresolvable use-item entry. No crash, no draw, the
//               resolver is only called for the real queue entry, 'strip k=%d hidden' logged once per member (edge log).
//   cancel      0.3.0: hud-host absent -> no Cancel controls exist: 40 leader frames (hover/click paths run) never call ServerClearAllActions
//               (tripwire), never log CANCEL, never load an image.
//   inert       KOTOR_VERSION_SHA = unknown sha: the real provider's dispatch is a no-op (neither phase reaches queue-hud), capture still
//               safe, nothing drawn, exactly two WARNs (the provider's k2addr table complaint + hud-host NOT connected).
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
    printf("queuehud: %s %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) ++g_fail;
}
static void checkf(bool ok, const char* fmt, int a = 0, int b = 0, int c = 0) {
    char buf[256]; snprintf(buf, sizeof buf, fmt, a, b, c); check(ok, buf);
}
[[noreturn]] static void die(int code, const char* msg) { printf("queuehud: %s\n", msg); fflush(stdout); exit(code); }

// ---------------------------------------------------------------- engine memory: the zeroed pad + tripwires
static char pad[0xA00000];   // 0.3.0: 10 MB; the exe is linked at --image-base=0x120000 so this .bss starts below 0x423000 (embtex stubs)

static const uintptr_t AddrResolver = 0x0074E0B0;   // the engine icon resolver queue-hud calls (thiscall, 6 stack args, RET 0x18)
enum Wire { W_BEGIN, W_END, W_PUSH, W_POP, W_FLUSH, W_HUDSCALE, W_RESOLVER, W_LOADIMG, W_CLEARALL, W_COUNT };
static const char* const kWireName[W_COUNT] = { "BeginGui2D", "EndGui2D", "PushClipRect", "PopClipRect", "FlushGuiBatch", "HudScale", "icon resolver 0x74E0B0", "GuiLoadImage", "ServerClearAllActions" };
static uintptr_t wireAddr(int w) {
    switch (w) {
        case W_BEGIN: return kSteam.BeginGui2D; case W_END: return kSteam.EndGui2D; case W_PUSH: return kSteam.PushClipRect;
        case W_POP: return kSteam.PopClipRect; case W_FLUSH: return kSteam.FlushGuiBatch; case W_HUDSCALE: return kSteam.HudScale;
        case W_LOADIMG: return gameaddr::GuiLoadImage; case W_CLEARALL: return gameaddr::ServerClearAllActions;
        default: return AddrResolver;
    }
}
static volatile long g_hits[W_COUNT];
static void  __cdecl   FakeBegin()                            { ++g_hits[W_BEGIN]; }
static void  __cdecl   FakeEnd()                              { ++g_hits[W_END]; }
static int   __cdecl   FakePush(int, int, int, int, const void*, int, float) { ++g_hits[W_PUSH]; return 0; }
static void  __cdecl   FakePop()                              { ++g_hits[W_POP]; }
static void  __cdecl   FakeFlush(float)                       { ++g_hits[W_FLUSH]; }
static float __cdecl   FakeHudScale()                         { ++g_hits[W_HUDSCALE]; return 1.0f; }
static void  __stdcall FakeResolver(int, int, int, int, int, int) { ++g_hits[W_RESOLVER]; }   // thiscall = ecx + 6 stack args, callee pops 0x18
static void* __cdecl   FakeLoadImage(const char*)              { ++g_hits[W_LOADIMG]; return nullptr; }
static void  __fastcall FakeClearAll(void*, void*, unsigned)  { ++g_hits[W_CLEARALL]; }   // thiscall(server app, object id) RET 4
static void* const kFake[W_COUNT] = { (void*)&FakeBegin, (void*)&FakeEnd, (void*)&FakePush, (void*)&FakePop, (void*)&FakeFlush, (void*)&FakeHudScale, (void*)&FakeResolver, (void*)&FakeLoadImage, (void*)&FakeClearAll };

// embtex engine STUBS (0.3.0; EmbedTex.h's engine table): heap-backed so the real create / fill path runs, calls counted, not tripwires.
enum Emb { E_NEW, E_NEWARR, E_CTOR, E_LISTADD, E_RESET, E_COUNT };
static volatile long g_eh[E_COUNT];
static uintptr_t embAddr(int e) {
    switch (e) { case E_NEW: return gameaddr::New; case E_NEWARR: return gameaddr::EngineNewArr; case E_CTOR: return gameaddr::TexBasicCtor;
                 case E_LISTADD: return gameaddr::TexListAdd; default: return gameaddr::TexReset; }
}
static void* __cdecl     EmbNew(unsigned n)                         { ++g_eh[E_NEW]; return calloc(1, n); }
static void* __cdecl     EmbNewArr(unsigned n)                      { ++g_eh[E_NEWARR]; return calloc(1, n); }
static void* __fastcall  EmbCtor(void* self, void*, const char*, const char*) { ++g_eh[E_CTOR]; return self; }   // thiscall RET 8
static void  __fastcall  EmbListAdd(void*, void*, void*)            { ++g_eh[E_LISTADD]; }                        // thiscall(list, basic) RET 4
static void  __fastcall  EmbReset(void*)                            { ++g_eh[E_RESET]; }                          // fastcall(basic)
static void* const kEmb[E_COUNT] = { (void*)&EmbNew, (void*)&EmbNewArr, (void*)&EmbCtor, (void*)&EmbListAdd, (void*)&EmbReset };

static void unprotect(uintptr_t addr, unsigned size) {
    DWORD o;
    if (!VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &o)) { printf("queuehud: VirtualProtect %08x failed %lu\n", (unsigned)addr, GetLastError()); exit(2); }
}
static void putJmp(uintptr_t at, void* to) {   // mov eax, imm32 ; jmp eax
    unsigned char b[7] = { 0xB8, 0, 0, 0, 0, 0xFF, 0xE0 }; uintptr_t t = (uintptr_t)to; memcpy(b + 1, &t, 4); memcpy((void*)at, b, 7);
}
static long totalHits() { long n = 0; for (int i = 0; i < W_COUNT; ++i) n += g_hits[i]; return n; }
static void checkNoEngineCalls(const char* where) {
    char buf[200];
    snprintf(buf, sizeof buf, "%s: no engine draw / resolver call (tripwires all 0)", where);
    check(totalHits() == 0, buf);
    for (int i = 0; i < W_COUNT; ++i)
        if (g_hits[i]) printf("queuehud:   tripwire %s hit %ld time(s)\n", kWireName[i], (long)g_hits[i]);
}
// true when every pad byte is zero except the planted stubs (16-byte windows); *at = first offending address
static uintptr_t g_extraStub[2];   // strips mode: fake PartyGetAt / MemberServerCreature windows (not tripwires)
static bool padClean(uintptr_t* at) {
    for (size_t i = 0; i < sizeof pad; ++i) {
        uintptr_t a = (uintptr_t)pad + i;
        if (pad[i] == 0) continue;
        bool stub = false;
        for (int w = 0; w < W_COUNT; ++w) if (a >= wireAddr(w) && a < wireAddr(w) + 16) stub = true;
        for (int e = 0; e < E_COUNT; ++e) if (a >= embAddr(e) && a < embAddr(e) + 16) stub = true;
        for (uintptr_t x : g_extraStub) if (x && a >= x && a < x + 16) stub = true;
        if (!stub) { *at = a; return false; }
    }
    return true;
}
static bool unreadable(uintptr_t a) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery((void*)a, &mbi, sizeof mbi)) return true;
    return mbi.State != MEM_COMMIT;
}

// ---------------------------------------------------------------- fake objects
static const int HudSize = 0xB000;               // >= 0xA910: flags dword at +0xA904, slot controls at +0x5F2C
static const int HudFlagsOff = 0xA904;
static char* g_hud = nullptr;
static int SLOT_A;
static const uintptr_t kBadPtr = 0xDEAD0000;     // an address Wine must not have committed (checked at setup)
static void setFlags(uint32_t f) { memcpy(g_hud + HudFlagsOff, &f, 4); }

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

// ---------------------------------------------------------------- the two DLLs
typedef void (__cdecl *InitFn)(const KPatchApi*);
typedef void (__cdecl *CaptureFn)(int);
typedef void (__cdecl *SlotDrawFn)(int, int, int);
typedef void (__cdecl *SnapFn)(int, int);
struct Dlls {
    HMODULE ps = nullptr, qh = nullptr;
    InitFn psInit = nullptr, qhInit = nullptr;
    CaptureFn capture = nullptr;      // QueueHud hook H8
    SlotDrawFn slotDraw = nullptr;    // portrait-slots hook 6 (drives the HUD_DRAW dispatch)
    SnapFn snapshot = nullptr;        // portrait-slots hooks 1a/1b (slot -> creature table, what slotAt() serves)
};
static Dlls D;
static HMODULE loadDll(const char* n) {
    HMODULE m = LoadLibraryA(n);
    if (!m) { printf("queuehud: LoadLibrary %s failed %lu\n", n, GetLastError()); exit(1); }
    return m;
}
static FARPROC need(HMODULE m, const char* dll, const char* name) {
    FARPROC p = GetProcAddress(m, name);
    if (!p) { printf("queuehud: %s missing export %s\n", dll, name); exit(3); }
    return p;
}
static void loadAll() {
    D.ps = loadDll("portrait-slots.dll"); D.qh = loadDll("queue-hud.dll");
    D.psInit = (InitFn)need(D.ps, "portrait-slots.dll", "KPatchInit");
    D.slotDraw = (SlotDrawFn)need(D.ps, "portrait-slots.dll", "PS_PortraitSlotDraw");
    D.snapshot = (SnapFn)need(D.ps, "portrait-slots.dll", "PS_SnapshotPortraitSlot");
    D.qhInit = (InitFn)need(D.qh, "queue-hud.dll", "KPatchInit");
    D.capture = (CaptureFn)need(D.qh, "queue-hud.dll", "QueueHudCapture");
}

static void setup(bool inert) {
    uintptr_t lo = ~(uintptr_t)0, hi = 0;
    for (int w = 0; w < W_COUNT; ++w) { lo = std::min(lo, wireAddr(w)); hi = std::max(hi, wireAddr(w) + 16); }
    for (int e = 0; e < E_COUNT; ++e) { lo = std::min(lo, embAddr(e)); hi = std::max(hi, embAddr(e) + 16); }
    hi = std::max(hi, (uintptr_t)gameaddr::TexPendingList + 0x1000);   // embtex reads the pending-list count at +4 (zeroed pad)
    hi = std::max(hi, (uintptr_t)kSteam.ClipDepth + 0x1000);
    hi = std::max(hi, (uintptr_t)kSteam.AppGlobal + 0x1000);
    if ((uintptr_t)pad > lo || (uintptr_t)pad + sizeof pad < hi) {
        printf("queuehud: pad %p..%p does not cover engine addresses %08x .. %08x\n", (void*)pad, (void*)(pad + sizeof pad), (unsigned)lo, (unsigned)hi);
        exit(2);
    }
    if (!unreadable(kBadPtr)) die(2, "bad-pointer sentinel 0xDEAD0000 is committed memory in this process");
    for (int w = 0; w < W_COUNT; ++w) { unprotect(wireAddr(w), 16); putJmp(wireAddr(w), kFake[w]); }
    for (int e = 0; e < E_COUNT; ++e) { unprotect(embAddr(e), 16); putJmp(embAddr(e), kEmb[e]); }
    g_hud = (char*)VirtualAlloc(nullptr, HudSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);   // zeroed
    if (!g_hud) die(2, "cannot allocate the fake HUD buffer");
    SLOT_A = (int)(uintptr_t)pad + 0x10000;
    // address table selection happens inside each DLL on first use: Steam when unset, none for an unknown sha
    SetEnvironmentVariableA("KOTOR_VERSION_SHA", inert ? "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" : nullptr);
    _putenv(inert ? "KOTOR_VERSION_SHA=0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" : "KOTOR_VERSION_SHA=");
    if (!inert) SetEnvironmentVariableA("KOTOR_VERSION_SHA", nullptr);
    DeleteFileA("queue_hud.ini");   // defaults: enabled, log on
    fakekp::reset();
    fakekp::withLog();
    loadAll();
    D.psInit(fakekp::api());
    D.qhInit(fakekp::api());
}

// leader call = isLarge 1 (advances the frame counter, FRAME_BEGIN, HUD_DRAW); callerEbp is only read in menu frames
static void leaderCalls(int n) { for (int i = 0; i < n; ++i) D.slotDraw(SLOT_A, 1, 0); }
static const uint32_t FlagCombat = 2;   // bit 1 of [hud+0xA904]

// ================================================================ wire
static int modeWire() {
    setup(false);
    check(fakekp::provided(K2PS_IFACE, K2PS_VERSION), "portrait-slots provided k2.portraitslots v1 (KPatchInit of queue-hud ran after it without crashing)");
    const std::vector<Reg> regs = regsFromLog();
    checkf(regs.size() == 2, "exactly 2 registrations in total (queue-hud's HUD_DRAW + FRAME_BEGIN), got %d", (int)regs.size());
    check(regs.size() == 2 && regs[0].phase == K2PS_HUD_DRAW && K2PS_HUD_DRAW == 5, "first registered phase is K2PS_HUD_DRAW (5)");
    check(regs.size() == 2 && regs[0].prio == 400, "HUD_DRAW priority is 400");
    check(regs.size() == 2 && regs[1].phase == K2PS_FRAME_BEGIN && K2PS_FRAME_BEGIN == 2, "second registered phase is K2PS_FRAME_BEGIN (2)");
    check(regs.size() == 2 && regs[1].prio == 300, "FRAME_BEGIN priority is 300");
    check(regs.size() == 2 && regs[0].owner == "queue-hud" && regs[1].owner == "queue-hud", "both registrations are owned by \"queue-hud\"");
    check(regs.size() == 2 && regs[0].token > 0 && regs[1].token > 0, "registration tokens > 0 (accepted)");
    check(logCount("portraitslots: connected") == 1, "queue-hud connected to the provider (onConnect logged once)");
    check(logCount("0.4.0-dev: kit=1 ") == 1 && logCount(" enabled=1 ") == 1, "queue-hud logged its version / config line once (default config: kit on, enabled)");
    check(logCount("portraitslots: NOT connected") == 0, "the portrait-slots client is connected");
    check(logCount("hud-host: NOT connected (no Cancel controls)") == 1 && warnCount() == 1, "exactly one WARN: hud-host absent -> 'NOT connected (no Cancel controls)'");

    // QueueHudCapture returns cleanly and logs the hud pointer once
    setFlags(0);
    D.capture((int)(uintptr_t)g_hud);
    D.capture((int)(uintptr_t)g_hud);
    checkf(logCount("hud=") == 1, "capture of a fake HUD logged the pointer once (same pointer twice), got %d", logCount("hud="));
    checkNoEngineCalls("wire");
    check(g_eh[E_NEW] == 0 && g_eh[E_CTOR] == 0, "KPatchInit / capture create no texture (that waits for the first FRAME_BEGIN)");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched (all zero but the stubs), first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ combat
static int modeCombat() {
    setup(false);
    setFlags(FlagCombat);
    D.capture((int)(uintptr_t)g_hud);
    char want[96];
    snprintf(want, sizeof want, "hud=%08x flags=%08x combat=1", (unsigned)(uintptr_t)g_hud, (unsigned)FlagCombat);
    check(logCount(want) == 1, "capture with flags bit1 set logged combat=1");

    leaderCalls(20);
    checkf(g_eh[E_NEW] == 5 && g_eh[E_CTOR] == 5 && g_eh[E_NEWARR] == 5 && g_eh[E_LISTADD] == 5, "FRAME_BEGIN created the 5 HudKit textures exactly once over 20 frames (new/ctor/new[]/list add = %d/%d/%d)", (int)g_eh[E_NEW], (int)g_eh[E_CTOR], (int)g_eh[E_NEWARR]);
    check(g_eh[E_RESET] == 0, "heal() never Reset a healthy (queued, pixel-bearing) texture");
    check(logCount("hidden (combat)") == 1, "20 HUD_DRAW dispatches with combat=1: callback ran and logged 'hidden (combat)' once (edge log)");
    check(logCount("hidden (no creature)") == 0 && logCount("shown ") == 0, "combat short-circuits before the party chain and before any draw");
    checkNoEngineCalls("combat=1");

    // other bits set, bit 1 clear: not combat -> the callback now gets past the combat test (zeroed game memory: no creature)
    setFlags(0xFFFFFFFDu);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(3);
    check(logCount("hud=") == 1 && logCount("combat=0") == 0, "the same HUD pointer is not re-logged (log is on pointer change only)");
    check(logCount("hidden (no creature)") == 1, "flags 0xFFFFFFFD (bit 1 clear): combat=0, callback reaches the party read and hides (no creature)");
    setFlags(FlagCombat | 0x4);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(3);
    check(logCount("hidden (combat)") == 2, "bit 1 set again: hidden (combat) logged again on the transition");
    checkNoEngineCalls("combat toggling");
    check(warnCount() == 1 && logCount("hud-host: NOT connected") == 1, "only the expected hud-host WARN");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ nocreature
static int modeNoCreature() {
    setup(false);
    setFlags(0);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(50);   // zeroed engine memory: AppGlobal reads 0 -> party::table fails -> gather() false
    check(logCount("hidden (no creature)") == 1, "combat=0 + zeroed game memory: HUD_DRAW returns cleanly with 'hidden (no creature)' (once)");
    check(logCount("shown ") == 0 && logCount("changed ") == 0, "no 'shown' / 'changed' draw line");
    checkNoEngineCalls("combat=0, no game memory");

    // a capture older than 250 ms is stale: hidden before the party read
    Sleep(320);
    leaderCalls(5);
    check(logCount("hidden (stale capture)") == 1, "capture older than 250 ms: 'hidden (stale capture)' (HUD hook stopped firing)");
    D.capture((int)(uintptr_t)g_hud);   // fresh again
    leaderCalls(5);
    check(logCount("hidden (no creature)") == 2, "fresh capture again: back to 'hidden (no creature)'");
    checkNoEngineCalls("stale / fresh");
    check(warnCount() == 1 && logCount("hud-host: NOT connected") == 1, "only the expected hud-host WARN");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ badhud
static int modeBadHud() {
    setup(false);
    D.capture((int)kBadPtr);   // unreadable [hud+0xA904]: treated as combat, never dereferenced raw
    check(logCount("combat=1") == 1, "capture of an unreadable HUD pointer logged combat=1 (treat as combat), no crash");
    leaderCalls(10);
    check(logCount("hidden (hud unreadable)") == 1, "HUD_DRAW with an unreadable captured HUD: 'hidden (hud unreadable)' (once), no crash");
    D.capture(0);   // null hud: nothing to show, silent
    leaderCalls(10);
    checkNoEngineCalls("bad hud");
    check(warnCount() == 1 && logCount("hud-host: NOT connected") == 1, "only the expected hud-host WARN");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ badchain
static void putU32(uintptr_t addr, uint32_t v) { memcpy((void*)addr, &v, 4); }
static int modeBadChain() {
    setup(false);
    setFlags(0);
    const uintptr_t app = (uintptr_t)kSteam.AppGlobal;
    const uintptr_t base = (uintptr_t)pad;
    const uintptr_t A = base + 0x20000, B = base + 0x21000, C = base + 0x22000;   // fake chain objects inside the pad (plain zeroed memory)
    struct Case { const char* what; };
    auto run = [&](const char* what) {
        D.capture((int)(uintptr_t)g_hud);
        const int before = logCount("hidden (no creature)");
        leaderCalls(10);
        char buf[160];
        snprintf(buf, sizeof buf, "%s: HUD_DRAW returned cleanly with 'hidden (no creature)'", what);
        // noteHidden is an edge log: the very first case logs, later identical reasons stay silent. Either way the callback must return.
        check(logCount("hidden (no creature)") >= before && logCount("hidden (no creature)") >= 1, buf);
        snprintf(buf, sizeof buf, "%s: no engine draw / resolver call", what);
        check(totalHits() == 0, buf);
    };
    putU32(app, (uint32_t)kBadPtr);                                    run("AppGlobal -> unreadable pointer");
    putU32(app, (uint32_t)A); putU32(A + 4, (uint32_t)kBadPtr);        run("app+4 -> unreadable pointer");
    putU32(A + 4, (uint32_t)B); putU32(B + 4, (uint32_t)kBadPtr);      run("a+4 -> unreadable pointer");
    putU32(B + 4, (uint32_t)C); putU32(C + 0x270, (uint32_t)kBadPtr);  run("party table pointer unreadable");
    putU32(C + 0x270, (uint32_t)(base + 0x23000));                      // table readable, count 0, member 0 = null
    run("party table readable but empty");
    check(logCount("shown ") == 0 && logCount("changed ") == 0, "no draw line in any broken-chain case");
    check(warnCount() == 1 && logCount("hud-host: NOT connected") == 1, "only the expected hud-host WARN");
    checkNoEngineCalls("broken party chains");
    // restore the zeroed pad before the cleanliness check
    putU32(app, 0); putU32(A + 4, 0); putU32(B + 4, 0); putU32(C + 0x270, 0);
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad back to all zero after the cells are restored, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ strips
static void putI32(uintptr_t addr, int v) { memcpy((void*)addr, &v, 4); }
static uintptr_t g_fakeMember[3], g_fakeServer[3];   // party positions 0..2: client / server creatures
static int g_memberCalls = 0, g_serverCalls = 0;
static uintptr_t __fastcall FakeGetAt(void*, void*, int idx) { ++g_memberCalls; return idx >= 0 && idx < 3 ? g_fakeMember[idx] : 0; }   // thiscall(table, idx) RET 4
static uintptr_t __fastcall FakeServerOf(void* member, void*) {                                                                      // thiscall(member) RET 0
    ++g_serverCalls;
    for (int i = 0; i < 3; ++i) if ((uintptr_t)member == g_fakeMember[i]) return g_fakeServer[i];
    return 0;
}
static int modeStrips() {
    setup(false);
    setFlags(0);
    const uintptr_t app = (uintptr_t)kSteam.AppGlobal, base = (uintptr_t)pad;
    const uintptr_t A = base + 0x20000, B = base + 0x21000, C = base + 0x22000, TBL = base + 0x23000;
    const uintptr_t S1 = base + 0x24000, S2 = base + 0x25000, S3 = base + 0x26000;                        // fake portrait slots (zeroed)
    const uintptr_t CL0 = base + 0x27000, CL1 = base + 0x27400, CL2 = base + 0x27800;                    // client creatures
    const uintptr_t SV0 = base + 0x28000, SV1 = base + 0x29000, SV2 = base + 0x2A000;                    // server creatures
    const uintptr_t QI = base + 0x2B000, QN = base + 0x2B100, QD = base + 0x2B200;                       // queue: internal, node, action
    const uintptr_t getAt = (uintptr_t)kSteam.PartyGetAt, serverOf = (uintptr_t)kSteam.MemberServerCreature;
    if (getAt < (uintptr_t)pad || getAt + 16 > (uintptr_t)pad + sizeof pad || serverOf < (uintptr_t)pad || serverOf + 16 > (uintptr_t)pad + sizeof pad) die(2, "PartyGetAt / MemberServerCreature outside the pad");
    const int vitOff = kSteam.Slot_VitRect, cardOff = kSteam.Slot_CardRect;
    g_fakeMember[0] = CL0; g_fakeMember[1] = CL1; g_fakeMember[2] = CL2; g_fakeServer[0] = SV0; g_fakeServer[1] = SV1; g_fakeServer[2] = SV2;
    D.capture((int)(uintptr_t)g_hud);
    auto strip = [&](int k, const char* what) { char t[32]; snprintf(t, sizeof t, "strip k=%d %s", k, what); return logCount(t); };

    // fake slots: S1 shows member 1 by its CLIENT pointer, S2 member 2 by its SERVER pointer, S3 the leader (client)
    D.snapshot((int)S1, (int)CL1); D.snapshot((int)S2, (int)SV2); D.snapshot((int)S3, (int)CL0);

    // 1. party chain unreadable
    putU32(app, (uint32_t)kBadPtr);
    leaderCalls(10);
    check(strip(1, "hidden") == 1 && strip(2, "hidden") == 1, "unreadable party chain: 'strip k=1 hidden' and 'strip k=2 hidden' logged once each (edge)");
    check(strip(1, "shown") == 0 && strip(2, "shown") == 0, "unreadable party chain: no strip shown");
    checkNoEngineCalls("strips, unreadable chain");

    // 2. readable but empty table (count 0): member() returns 0 without calling the engine
    putU32(app, (uint32_t)A); putU32(A + 4, (uint32_t)B); putU32(B + 4, (uint32_t)C); putU32(C + 0x270, (uint32_t)TBL);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(10);
    check(g_memberCalls == 0, "empty party table: PartyGetAt never called");
    check(strip(1, "hidden") == 1 && strip(2, "hidden") == 1, "empty party table: still hidden, no repeated edge line");
    checkNoEngineCalls("strips, empty table");

    // 3. members served by fake engine functions; slot rects still zero -> skipped
    unprotect(getAt, 16); putJmp(getAt, (void*)&FakeGetAt); g_extraStub[0] = getAt;
    unprotect(serverOf, 16); putJmp(serverOf, (void*)&FakeServerOf); g_extraStub[1] = serverOf;
    putI32(TBL, 3);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(10);
    check(g_memberCalls >= 2 && g_serverCalls >= 2, "party of 3: members 1-2 resolved through the (fake) engine accessors");
    check(strip(1, "hidden") == 1 && strip(2, "hidden") == 1 && strip(1, "shown") == 0, "zero slot rects: slots skipped, nothing shown");
    checkNoEngineCalls("strips, zero rects");

    // 4. valid rects (3840x1600 companion cards), empty queues -> nothing to show, still no engine call
    const int vit[4] = { 3671, 1285, 47, 96 }, card[4] = { 3711, 1285, 85, 96 };
    for (uintptr_t s : { S1, S2, S3 }) for (int j = 0; j < 4; ++j) { putI32(s + vitOff + 4 * j, vit[j]); putI32(s + cardOff + 4 * j, card[j]); }
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(10);
    check(strip(1, "shown") == 0 && strip(2, "shown") == 0, "valid rects + empty queues: nothing shown");
    checkNoEngineCalls("strips, empty queues");

    // 5. a use-item entry in member 1's queue (and the leader's): the resolver is the ONLY engine call; the fake leaves the icon blank -> no strip
    putU32(SV1 + 0x100, (uint32_t)QI); putU32(QI, (uint32_t)QN); putU32(QN + 4, 0); putU32(QN + 8, (uint32_t)QD);
    putU32(QD, 0x2E); putU32(QD + 0x38, 0x7F000001u);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(10);
    check(g_hits[W_RESOLVER] >= 1 && g_hits[W_RESOLVER] <= 2, "member 1 use-item entry: icon resolver called (blank result is cached, not re-called every frame)");
    long others = totalHits() - g_hits[W_RESOLVER];
    check(others == 0, "no draw call: a blank icon takes no slot");
    check(strip(1, "shown") == 0 && strip(2, "shown") == 0, "blank icon: no strip shown");
    const long hitsBefore = g_hits[W_RESOLVER];
    putU32(SV0 + 0x100, (uint32_t)QI);   // the LEADER's server creature gets the same queue: it is never matched as a companion strip
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(10);
    check(g_hits[W_RESOLVER] - hitsBefore <= 1, "leader slot never drives a strip (only the cached key / one leader-panel resolve)");

    // 6. garbage everywhere on a member: unreadable queue pointer / unreadable action
    putU32(SV1 + 0x100, (uint32_t)kBadPtr); putU32(SV2 + 0x100, (uint32_t)QI); putU32(QN + 8, (uint32_t)kBadPtr);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(10);
    check(strip(1, "shown") == 0 && strip(2, "shown") == 0, "unreadable queue / action pointers: no crash, nothing shown");
    // a member whose pointer no slot carries: no match
    D.snapshot((int)S1, (int)kBadPtr);
    leaderCalls(5);
    check(warnCount() <= 3, "only the expected hud-host WARN + blank-icon WARN lines (<= 3)");
    // restore the zeroed pad
    for (uintptr_t x = base + 0x20000; x < base + 0x2C000; x += 4) putU32(x, 0);
    putU32(app, 0);
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad back to all zero (stubs aside), first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ cancel
static int modeCancel() {
    setup(false);
    setFlags(0);
    D.capture((int)(uintptr_t)g_hud);
    leaderCalls(40);   // FRAME_BEGIN runs the cancel / hover / click path every frame (menu flag 0 from the fake context, combat=0)
    check(logCount("hud-host: NOT connected (no Cancel controls)") == 1, "hud-host absent: 'NOT connected (no Cancel controls)' logged once");
    check(g_hits[W_CLEARALL] == 0 && logCount("CANCEL") == 0, "no Cancel control exists: ServerClearAllActions never called, no CANCEL log line");
    check(g_hits[W_LOADIMG] == 0, "no Cancel art requested without a control (GuiLoadImage never called)");
    checkNoEngineCalls("cancel (hud-host absent)");
    check(warnCount() == 1, "only the expected hud-host WARN");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ inert
static int modeInert() {
    setup(true);
    check(fakekp::provided(K2PS_IFACE, K2PS_VERSION), "provider still provides on an unknown exe");
    check(regsFromLog().size() == 2, "queue-hud still registers its two callbacks (KPatchInit never touches the table)");
    setFlags(0);
    for (int i = 0; i < 100; ++i) { D.capture((int)(uintptr_t)g_hud); leaderCalls(2); }
    setFlags(FlagCombat);
    for (int i = 0; i < 100; ++i) { D.capture((int)(uintptr_t)g_hud); leaderCalls(2); }
    check(logCount("hidden (") == 0 && logCount("shown ") == 0, "unknown exe: HUD_DRAW never dispatched to queue-hud (no 'hidden' / 'shown' line)");
    checkf(warnCount() == 2 && logCount("hud-host: NOT connected") == 1, "exactly two WARNs (provider's k2addr table complaint + hud-host NOT connected), got %d", warnCount());
    check(g_eh[E_NEW] == 0 && g_eh[E_CTOR] == 0, "unknown exe: FRAME_BEGIN never dispatched, no texture created");
    checkNoEngineCalls("unknown exe");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "wire";
    printf("queuehud: mode %s\n", mode);
    int rc;
    if (!strcmp(mode, "wire")) rc = modeWire();
    else if (!strcmp(mode, "combat")) rc = modeCombat();
    else if (!strcmp(mode, "nocreature")) rc = modeNoCreature();
    else if (!strcmp(mode, "badhud")) rc = modeBadHud();
    else if (!strcmp(mode, "badchain")) rc = modeBadChain();
    else if (!strcmp(mode, "strips")) rc = modeStrips();
    else if (!strcmp(mode, "cancel")) rc = modeCancel();
    else if (!strcmp(mode, "inert")) rc = modeInert();
    else die(2, "unknown mode (wire|combat|nocreature|badhud|badchain|strips|cancel|inert)");
    printf("queuehud: %s %d checks, %d failed\n", rc ? "FAIL" : "PASS", g_checks, g_fail);
    return rc;
}
