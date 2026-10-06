// Smoke test for companion-selfheal 0.7.0-dev (CS12): WIRING only, not heal behaviour. The real selfheal DLL is loaded under Wine's PE loader
// TOGETHER with the real portrait-slots provider (k2.portraitslots v1) and the real hud-host provider (k2.hud v1), wired through the registry
// exactly as KPM would (tests/include/FakeKPatchApi.h). Engine memory = a zeroed pad at image base 0x120000 (see smoke_queuehud.cpp); the heal
// engine calls are TRIPWIRES (UseItemRouteA 0x57D8B0, ActListRemoveHead 0x7F2A90, ActListAddTail 0x54AA30, RoundHasPending 0x590110 + the draw /
// clear-actions set). The embtex engine calls and the label ctor are harmless stubs (the BUILD phase runs the real ensureTextures / createLabel).
// Run:   smoke_selfheal.exe <wire|inert|badchain|kick-off>   (dir needs companion-selfheal.dll + portrait-slots.dll + hud-host.dll).
//        Exit codes: 0 pass, 1 check failed / LoadLibrary failed, 2 setup failure, 3 missing export.
//
//   wire       provider + hud-host -> selfheal. KPatchInit OK; registrations owner "companion-selfheal": K2PS_FRAME_BEGIN (2) prio 300 and
//              K2HUD_BUILD (3) prio 200; both clients connected; the version line (0.7.0-dev ... kick=1) once; zero WARNs. Then the BUILD phase on a
//              fake HUD (vtable only; the label ctor stub leaves no vtable): exactly one WARN, 'BUILD n=0/16', no engine calls.
//   inert      unknown KOTOR_VERSION_SHA: the provider dispatch is a no-op, nothing drawn / used, no tick evidence.
//   badchain   HUD built, party chain broken at each link: the FRAME_BEGIN tick returns cleanly (evidence: the P3 slot-map log), kickTick /
//              pendingTick make NO engine call (tripwires 0). NOTE: both ticks return early with no pending use armed (arming needs useNow()
//              = route A behind the foreground / HUD gate), so this proves the chain read is crash-free and silent, not the rotate itself.
//   kick-off   companion_selfheal.ini [queue] kick=0 -> the version line logs kick=0.
//   kick-rotate NOT implemented: a pending member can only be armed through useNow() (route A + foreground gate), which a Wine smoke cannot drive.
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
#include "HudHost.h"

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what) {
    ++g_checks;
    printf("selfheal: %s %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) ++g_fail;
}
static void checkf(bool ok, const char* fmt, int a = 0, int b = 0, int c = 0) {
    char buf[256]; snprintf(buf, sizeof buf, fmt, a, b, c); check(ok, buf);
}
[[noreturn]] static void die(int code, const char* msg) { printf("selfheal: %s\n", msg); fflush(stdout); exit(code); }

static char pad[0xA00000];   // 10 MB .bss; exe linked at --image-base=0x120000 so this covers every engine address the DLLs touch

// ---------------------------------------------------------------- tripwires (counted, never expected to fire)
enum Wire { W_BEGIN, W_END, W_PUSH, W_POP, W_FLUSH, W_HUDSCALE, W_CLEARALL, W_USEITEM, W_REMOVEHEAD, W_ADDTAIL, W_ROUNDPEND, W_COUNT };
static const char* const kWireName[W_COUNT] = { "BeginGui2D", "EndGui2D", "PushClipRect", "PopClipRect", "FlushGuiBatch", "HudScale", "ServerClearAllActions",
                                                 "UseItemRouteA 0x57D8B0", "ActListRemoveHead 0x7F2A90", "ActListAddTail 0x54AA30", "RoundHasPending 0x590110" };
static uintptr_t wireAddr(int w) {
    switch (w) {
        case W_BEGIN: return kSteam.BeginGui2D; case W_END: return kSteam.EndGui2D; case W_PUSH: return kSteam.PushClipRect;
        case W_POP: return kSteam.PopClipRect; case W_FLUSH: return kSteam.FlushGuiBatch; case W_HUDSCALE: return kSteam.HudScale;
        case W_CLEARALL: return gameaddr::ServerClearAllActions; case W_USEITEM: return gameaddr::UseItemRouteA;
        case W_REMOVEHEAD: return gameaddr::ActListRemoveHead; case W_ADDTAIL: return gameaddr::ActListAddTail;
        default: return gameaddr::RoundHasPending;
    }
}
static volatile long g_hits[W_COUNT];
static void  __cdecl    FakeBegin()                            { ++g_hits[W_BEGIN]; }
static void  __cdecl    FakeEnd()                              { ++g_hits[W_END]; }
static int   __cdecl    FakePush(int, int, int, int, const void*, int, float) { ++g_hits[W_PUSH]; return 0; }
static void  __cdecl    FakePop()                              { ++g_hits[W_POP]; }
static void  __cdecl    FakeFlush(float)                       { ++g_hits[W_FLUSH]; }
static float __cdecl    FakeHudScale()                         { ++g_hits[W_HUDSCALE]; return 1.0f; }
static void  __fastcall FakeClearAll(void*, void*, unsigned)   { ++g_hits[W_CLEARALL]; }
static void  __fastcall FakeUseItem(void*, void*, int, int, int, int, int, int, int, int) { ++g_hits[W_USEITEM]; }   // thiscall, 8 stack args, RET 0x20
static unsigned __fastcall FakeRemoveHead(void*, void*)        { ++g_hits[W_REMOVEHEAD]; return 0; }                // thiscall(list) RET 0
static void  __fastcall FakeAddTail(void*, void*, unsigned)    { ++g_hits[W_ADDTAIL]; }                             // thiscall(list, data) RET 4
static unsigned __fastcall FakeRoundPend(void*, void*)         { ++g_hits[W_ROUNDPEND]; return 0; }                 // thiscall(round) RET 0
static void* const kFake[W_COUNT] = { (void*)&FakeBegin, (void*)&FakeEnd, (void*)&FakePush, (void*)&FakePop, (void*)&FakeFlush, (void*)&FakeHudScale,
                                      (void*)&FakeClearAll, (void*)&FakeUseItem, (void*)&FakeRemoveHead, (void*)&FakeAddTail, (void*)&FakeRoundPend };

// ---------------------------------------------------------------- stubs (counted, not tripwires)
enum Stub { S_NEW, S_NEWARR, S_CTOR, S_LISTADD, S_RESET, S_LABELCTOR, S_LOADIMG, S_COUNT };
static volatile long g_sh[S_COUNT];
static uintptr_t stubAddr(int e) {
    switch (e) { case S_NEW: return gameaddr::New; case S_NEWARR: return gameaddr::EngineNewArr; case S_CTOR: return gameaddr::TexBasicCtor;
                 case S_LISTADD: return gameaddr::TexListAdd; case S_RESET: return gameaddr::TexReset; case S_LABELCTOR: return gameaddr::LabelCtor;
                 default: return gameaddr::GuiLoadImage; }
}
static void* __cdecl     StNew(unsigned n)                          { ++g_sh[S_NEW]; return calloc(1, n); }
static void* __cdecl     StNewArr(unsigned n)                       { ++g_sh[S_NEWARR]; return calloc(1, n); }
static void* __fastcall  StCtor(void* self, void*, const char*, const char*) { ++g_sh[S_CTOR]; return self; }   // thiscall RET 8
static void  __fastcall  StListAdd(void*, void*, void*)             { ++g_sh[S_LISTADD]; }                        // thiscall(list, basic) RET 4
static void  __fastcall  StReset(void*)                             { ++g_sh[S_RESET]; }                          // fastcall(basic)
static void  __fastcall  StLabelCtor(void*)                         { ++g_sh[S_LABELCTOR]; }                      // thiscall ECX only: leaves the label vtable 0 -> CF_Vtable
static void* __cdecl     StLoadImage(const char*)                   { ++g_sh[S_LOADIMG]; return nullptr; }
static void* const kStub[S_COUNT] = { (void*)&StNew, (void*)&StNewArr, (void*)&StCtor, (void*)&StListAdd, (void*)&StReset, (void*)&StLabelCtor, (void*)&StLoadImage };

static void unprotect(uintptr_t addr, unsigned size) {
    DWORD o;
    if (!VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &o)) { printf("selfheal: VirtualProtect %08x failed %lu\n", (unsigned)addr, GetLastError()); exit(2); }
}
static void putJmp(uintptr_t at, void* to) {   // mov eax, imm32 ; jmp eax
    unsigned char b[7] = { 0xB8, 0, 0, 0, 0, 0xFF, 0xE0 }; uintptr_t t = (uintptr_t)to; memcpy(b + 1, &t, 4); memcpy((void*)at, b, 7);
}
static long totalHits() { long n = 0; for (int i = 0; i < W_COUNT; ++i) n += g_hits[i]; return n; }
static void checkNoEngineCalls(const char* where) {
    char buf[200];
    snprintf(buf, sizeof buf, "%s: no heal / draw engine call (tripwires all 0)", where);
    check(totalHits() == 0, buf);
    for (int i = 0; i < W_COUNT; ++i)
        if (g_hits[i]) printf("selfheal:   tripwire %s hit %ld time(s)\n", kWireName[i], (long)g_hits[i]);
}
static bool padClean(uintptr_t* at) {   // all zero except the planted 16-byte windows
    for (size_t i = 0; i < sizeof pad; ++i) {
        uintptr_t a = (uintptr_t)pad + i;
        if (pad[i] == 0) continue;
        bool planted = false;
        for (int w = 0; w < W_COUNT; ++w) if (a >= wireAddr(w) && a < wireAddr(w) + 16) planted = true;
        for (int e = 0; e < S_COUNT; ++e) if (a >= stubAddr(e) && a < stubAddr(e) + 16) planted = true;
        if (!planted) { *at = a; return false; }
    }
    return true;
}
static bool unreadable(uintptr_t a) {
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery((void*)a, &mbi, sizeof mbi)) return true;
    return mbi.State != MEM_COMMIT;
}

// ---------------------------------------------------------------- fake HUD + log inspection
static const int HudSize = 0xB000;
static char* g_hud = nullptr;
static const uintptr_t kBadPtr = 0xDEAD0000;
static int SLOT_A;
static int logCount(const char* sub) {
    int n = 0; for (const std::string& l : fakekp::state().logLines) if (l.find(sub) != std::string::npos) ++n; return n;
}
static int warnCount() {   // fake Submit records "<channel>:<level>:<text>", level 1 = KPLOG_WARN
    int n = 0; for (const std::string& l : fakekp::state().logLines) if (l.find(":1:") == 1) ++n; return n;
}
static void dumpWarns() { for (const std::string& l : fakekp::state().logLines) if (l.find(":1:") == 1) printf("selfheal:   WARN %s\n", l.c_str()); }
struct Reg { int phase, prio, token; std::string owner; };
static std::vector<Reg> regsFromLog() {   // both providers log "register phase=%d prio=%d owner=%s -> %d" per apiRegister call
    std::vector<Reg> v;
    for (const std::string& l : fakekp::state().logLines) {
        const char* p = strstr(l.c_str(), "register phase="); if (!p) continue;
        Reg r; char own[64] = {};
        if (sscanf(p, "register phase=%d prio=%d owner=%63s -> %d", &r.phase, &r.prio, own, &r.token) == 4) { r.owner = own; v.push_back(r); }
    }
    return v;
}

// ---------------------------------------------------------------- the three DLLs
typedef void (__cdecl *InitFn)(const KPatchApi*);
typedef void (__cdecl *SlotDrawFn)(int, int, int);
typedef void (__cdecl *SnapFn)(int, int);
typedef void (__cdecl *BuildFn)(void*);
struct Dlls {
    HMODULE ps = nullptr, hh = nullptr, sh = nullptr;
    InitFn psInit = nullptr, hhInit = nullptr, shInit = nullptr;
    SlotDrawFn slotDraw = nullptr; SnapFn snapshot = nullptr; BuildFn layoutClosing = nullptr;
};
static Dlls D;
static HMODULE loadDll(const char* n) {
    HMODULE m = LoadLibraryA(n);
    if (!m) { printf("selfheal: LoadLibrary %s failed %lu\n", n, GetLastError()); exit(1); }
    return m;
}
static FARPROC need(HMODULE m, const char* dll, const char* name) {
    FARPROC p = GetProcAddress(m, name);
    if (!p) { printf("selfheal: %s missing export %s\n", dll, name); exit(3); }
    return p;
}

static void setup(bool inert, bool kickOff) {
    uintptr_t lo = ~(uintptr_t)0, hi = 0;
    for (int w = 0; w < W_COUNT; ++w) { lo = std::min(lo, wireAddr(w)); hi = std::max(hi, wireAddr(w) + 16); }
    for (int e = 0; e < S_COUNT; ++e) { lo = std::min(lo, stubAddr(e)); hi = std::max(hi, stubAddr(e) + 16); }
    hi = std::max(hi, (uintptr_t)gameaddr::TexPendingList + 0x1000);
    hi = std::max(hi, (uintptr_t)kSteam.ClipDepth + 0x1000);
    hi = std::max(hi, (uintptr_t)kSteam.AppGlobal + 0x1000);
    if ((uintptr_t)pad > lo || (uintptr_t)pad + sizeof pad < hi) {
        printf("selfheal: pad %p..%p does not cover engine addresses %08x .. %08x\n", (void*)pad, (void*)(pad + sizeof pad), (unsigned)lo, (unsigned)hi);
        exit(2);
    }
    if (!unreadable(kBadPtr)) die(2, "bad-pointer sentinel 0xDEAD0000 is committed memory in this process");
    for (int w = 0; w < W_COUNT; ++w) { unprotect(wireAddr(w), 16); putJmp(wireAddr(w), kFake[w]); }
    for (int e = 0; e < S_COUNT; ++e) { unprotect(stubAddr(e), 16); putJmp(stubAddr(e), kStub[e]); }
    g_hud = (char*)VirtualAlloc(nullptr, HudSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);   // zeroed; only the vtable is planted
    if (!g_hud) die(2, "cannot allocate the fake HUD buffer");
    const uint32_t vt = (uint32_t)gameaddr::HudVtable; memcpy(g_hud, &vt, 4);   // hud-host dispatches BUILD only for the HUD vtable
    SLOT_A = (int)(uintptr_t)pad + 0x10000;
    SetEnvironmentVariableA("KOTOR_VERSION_SHA", inert ? "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" : nullptr);
    _putenv(inert ? "KOTOR_VERSION_SHA=0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" : "KOTOR_VERSION_SHA=");
    if (!inert) SetEnvironmentVariableA("KOTOR_VERSION_SHA", nullptr);
    DeleteFileA("companion_selfheal.ini");
    if (kickOff) {
        FILE* f = fopen("companion_selfheal.ini", "w");
        if (!f) die(2, "cannot write companion_selfheal.ini");
        fputs("[queue]\r\nkick=0\r\n", f); fclose(f);
    }
    fakekp::reset();
    fakekp::withLog();
    D.ps = loadDll("portrait-slots.dll"); D.hh = loadDll("hud-host.dll"); D.sh = loadDll("companion-selfheal.dll");
    D.psInit = (InitFn)need(D.ps, "portrait-slots.dll", "KPatchInit");
    D.slotDraw = (SlotDrawFn)need(D.ps, "portrait-slots.dll", "PS_PortraitSlotDraw");
    D.snapshot = (SnapFn)need(D.ps, "portrait-slots.dll", "PS_SnapshotPortraitSlot");
    D.hhInit = (InitFn)need(D.hh, "hud-host.dll", "KPatchInit");
    D.layoutClosing = (BuildFn)need(D.hh, "hud-host.dll", "HH_LayoutClosing");
    D.shInit = (InitFn)need(D.sh, "companion-selfheal.dll", "KPatchInit");
    D.psInit(fakekp::api());
    D.hhInit(fakekp::api());
    D.shInit(fakekp::api());
}
static void leaderCalls(int n) { for (int i = 0; i < n; ++i) D.slotDraw(SLOT_A, 1, 0); }   // isLarge 1: frame counter + FRAME_BEGIN + HUD_DRAW

static void putU32(uintptr_t addr, uint32_t v) { memcpy((void*)addr, &v, 4); }

// ================================================================ wire
static int modeWire() {
    setup(false, false);
    check(fakekp::provided(K2PS_IFACE, K2PS_VERSION), "portrait-slots provided k2.portraitslots v1 (selfheal KPatchInit ran after it without crashing)");
    check(fakekp::provided(K2HUD_IFACE, K2HUD_VERSION), "hud-host provided k2.hud v1");
    const std::vector<Reg> regs = regsFromLog();
    checkf(regs.size() == 2, "exactly 2 registrations in total (FRAME_BEGIN + hud BUILD), got %d", (int)regs.size());
    check(regs.size() == 2 && regs[0].phase == K2PS_FRAME_BEGIN && K2PS_FRAME_BEGIN == 2, "first registered phase is K2PS_FRAME_BEGIN (2)");
    check(regs.size() == 2 && regs[0].prio == 300, "FRAME_BEGIN priority is 300");
    check(regs.size() == 2 && regs[1].phase == K2HUD_BUILD && K2HUD_BUILD == 3, "second registered phase is K2HUD_BUILD (3)");
    check(regs.size() == 2 && regs[1].prio == 200, "BUILD priority is 200");
    check(regs.size() == 2 && regs[0].owner == "companion-selfheal" && regs[1].owner == "companion-selfheal", "both registrations are owned by \"companion-selfheal\"");
    check(regs.size() == 2 && regs[0].token > 0 && regs[1].token > 0, "registration tokens > 0 (accepted)");
    check(logCount("portraitslots: connected") == 1, "selfheal connected to portrait-slots (onConnect logged once)");
    check(logCount("k2.hud: connected") == 1, "selfheal connected to hud-host (onHudConnect logged once)");
    check(logCount("0.7.0-dev: probe call=0") == 1 && logCount(" kick=1 ") == 1, "version line '0.7.0-dev ... kick=1' logged once");
    check(logCount("NOT connected") == 0 && warnCount() == 0, "no WARN after init (both providers present)");
    if (warnCount()) dumpWarns();

    leaderCalls(10);   // no HUD panel seen yet: FRAME_BEGIN returns at once
    check(logCount("P3 slot") == 0, "before the BUILD phase the FRAME_BEGIN tick does nothing (no HUD panel)");

    // BUILD phase: the vtable-only fake HUD; the label ctor stub leaves no vtable, so every widget fails cleanly
    D.layoutClosing(g_hud);
    check(logCount("first BUILD dispatch") == 1, "hud-host dispatched BUILD for the HUD vtable once");
    check(logCount("BUILD n=0/16") == 1, "selfheal BUILD built 0/16 widgets (label ctor stub) and said so");
    check(warnCount() == 1, "exactly one WARN: the BUILD n=0/16 summary");
    if (warnCount() != 1) dumpWarns();
    check(g_sh[S_LABELCTOR] >= 1, "createLabel reached the (stubbed) label ctor");
    checkf(g_sh[S_CTOR] == 14 && g_sh[S_RESET] == 0, "BUILD created the 14 embedded textures once (9 own + 5 HudKit), never Reset (tex ctor / reset = %d / %d)", (int)g_sh[S_CTOR], (int)g_sh[S_RESET]);
    leaderCalls(5);
    checkNoEngineCalls("wire");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched (all zero but the planted stubs), first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ inert
static int modeInert() {
    setup(true, false);
    check(fakekp::provided(K2PS_IFACE, K2PS_VERSION), "provider still provides on an unknown exe");
    check(regsFromLog().size() == 2, "selfheal still registers its two callbacks (KPatchInit never touches the table)");
    D.snapshot(SLOT_A + 0x1000, SLOT_A + 0x2000);
    D.layoutClosing(g_hud);   // hud-host + guikit with a null address table: no label can be built, nothing is called
    for (int i = 0; i < 100; ++i) leaderCalls(2);
    check(logCount("P3 slot") == 0, "unknown exe: FRAME_BEGIN never dispatched to selfheal (no slot-map tick)");
    check(logCount("BUILD n=0/16") == 1, "BUILD still reports 0/16 (guikit refuses without a table)");
    check(g_sh[S_LABELCTOR] == 0 && g_sh[S_CTOR] == 0, "unknown exe: no label ctor / texture ctor call");
    check(warnCount() >= 1, "the provider's k2addr table complaint is logged");
    checkNoEngineCalls("unknown exe");
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad untouched, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ badchain
static int modeBadChain() {
    setup(false, false);
    D.layoutClosing(g_hud);   // g_hud set inside selfheal (BUILD), so the tick runs past its first guard
    D.snapshot(SLOT_A + 0x1000, SLOT_A + 0x2000);   // one slot -> the 2 s slot-map poll logs on the first tick
    const uintptr_t app = (uintptr_t)kSteam.AppGlobal, base = (uintptr_t)pad;
    const uintptr_t A = base + 0x20000, B = base + 0x21000, C = base + 0x22000;
    auto run = [&](const char* what) {
        leaderCalls(10);
        char buf[160];
        snprintf(buf, sizeof buf, "%s: FRAME_BEGIN tick returned cleanly, no engine call", what);
        check(totalHits() == 0, buf);
    };
    leaderCalls(3);
    check(logCount("P3 slot") >= 1, "the tick is live after BUILD (slot-map log seen), so the chain cases below really run it");
    putU32(app, (uint32_t)kBadPtr);                                    run("AppGlobal -> unreadable pointer");
    putU32(app, (uint32_t)A); putU32(A + 4, (uint32_t)kBadPtr);        run("app+4 -> unreadable pointer");
    putU32(A + 4, (uint32_t)B); putU32(B + 4, (uint32_t)kBadPtr);      run("a+4 -> unreadable pointer");
    putU32(B + 4, (uint32_t)C); putU32(C + 0x270, (uint32_t)kBadPtr);  run("party table pointer unreadable");
    putU32(C + 0x270, (uint32_t)(base + 0x23000));                      // table readable, count 0
    run("party table readable but empty");
    check(logCount("KICK") == 0 && logCount("DONE") == 0 && logCount("AUTO") == 0, "no KICK / DONE / AUTO line in any broken-chain case");
    check(warnCount() == 1 && logCount("BUILD n=0/16") == 1, "only the expected BUILD n=0/16 WARN");
    checkNoEngineCalls("broken party chains");
    putU32(app, 0); putU32(A + 4, 0); putU32(B + 4, 0); putU32(C + 0x270, 0);
    uintptr_t bad = 0;
    checkf(padClean(&bad), "engine pad back to all zero after the cells are restored, first bad byte %08x", (int)bad);
    return g_fail ? 1 : 0;
}

// ================================================================ kick-off
static int modeKickOff() {
    setup(false, true);
    check(logCount("0.7.0-dev: probe call=0") == 1 && logCount(" kick=0 ") == 1 && logCount(" kick=1 ") == 0, "[queue] kick=0 in companion_selfheal.ini -> version line logs kick=0");
    D.layoutClosing(g_hud);
    leaderCalls(10);
    checkNoEngineCalls("kick=0");
    DeleteFileA("companion_selfheal.ini");
    return g_fail ? 1 : 0;
}

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "wire";
    printf("selfheal: mode %s\n", mode);
    int rc;
    if (!strcmp(mode, "wire")) rc = modeWire();
    else if (!strcmp(mode, "inert")) rc = modeInert();
    else if (!strcmp(mode, "badchain")) rc = modeBadChain();
    else if (!strcmp(mode, "kick-off")) rc = modeKickOff();
    else die(2, "unknown mode (wire|inert|badchain|kick-off)");
    printf("selfheal: %s %d checks, %d failed\n", rc ? "FAIL" : "PASS", g_checks, g_fail);
    return rc;
}
