// Smoke test for k2-robustness-fixes 1.2.0 (DLL-only; DllMain patches 12 independent groups, doc 10 "Robustness set"; 1.1.0 added
// fire counters and cave-based sites for vmid (2nd site 0x669012), sndri, guiid and netovf A; 1.2.0 adds fs / ssb / ssc / td / vmlk / movspin
// from W8, all checked here against real engine byte ranges).
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke_robustness.exe smoke_robustness.cpp
//        (default base 0x400000; the 7 MB .bss pad covers 0x428A00-0x99A000 so every site, the IAT slots 0x98657C / 0x9860D4, the byte at
//        0x99447F and the stub targets are ordinary memory of this exe; the guiid site 0x40F96F sits in the zero padding after .rdata,
//        checked at startup: `objdump -h` must show .rdata ending below 0x40F960 and .bss below 0x428A00)
// Run:   smoke_robustness.exe <run|fs2|movon|disabled|refuse|refuse2|keys|keys2>   (dir needs windows_x86.dll from RobustnessFixes.kpatch).
//        Exit 0 = pass.  run = default config (fs=1 count only, movspin OFF), fs2 = fs=2 (count + free), movon = movspin=1,
//        refuse = kt + vmid corrupted, refuse2 = one byte corrupted in each of fs/ssb/ssc/td/vmlk/movspin (movspin=1),
//        keys / keys2 = two complementary sets of key=0 (every group's key is 0 in one of them), disabled = enabled=0.
// Each fix site is the REAL engine byte range (generated from the dump asm / vanilla exe) followed/targeted by tiny marker stubs
// (`mov eax,N; ret`); TD runs the whole real C2DA::NextToken 0x71C180. Every scenario runs first with the original bytes (reference =
// the engine bug) and again after LoadLibrary patched the sites, in one process. Mode run/fs2/movon also dump every cave (cave_*.bin +
// caves_<mode>.txt) for the objdump check against W8.
#define __USE_MINGW_ANSI_STDIO 0   // use msvcrt printf: the mingw ANSI stdio adds ~25 KB .text and the image must end below 0x40F000
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static const unsigned char F_VMID[] = { 0x81, 0x7D, 0x08, 0x6D, 0x03, 0x00, 0x00, 0x7D, 0x30 }; // 0x668FD9..0x668FE1
static const unsigned char F_GUIID[] = { 0x89, 0x4D, 0xB0, 0x83, 0x7D, 0xB0, 0x00, 0x7C, 0x6C }; // 0x40F96F..0x40F977
static const unsigned char F_SNDRI[] = { 0xC7, 0x82, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x52 }; // 0x7054CE..0x7054D9
static const unsigned char F_NETB[] = { 0x05, 0x00, 0x00, 0x01, 0x00, 0x39, 0x45, 0xEC, 0x0F, 0x86, 0x18, 0x03, 0x00, 0x00 }; // 0x7040EF..0x7040FC
static const unsigned char F_NETA[] = { 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x4D, 0xE4, 0xE8, 0x6B, 0xF3, 0x02, 0x00 }; // 0x704406..0x704414
static const unsigned char F_KTA[] = { 0x89, 0x50, 0x0C, 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF }; // 0x726527..0x726530
static const unsigned char F_KTB[] = { 0x89, 0x50, 0x0C, 0xEB, 0x3C }; // 0x726C9C..0x726CA0
static const unsigned char F_AB[] = { 0x8B, 0x45, 0xEC, 0x83, 0xC0, 0x01, 0x89, 0x45, 0xEC, 0x8B, 0x4D, 0xC4, 0x8B, 0x55, 0xEC, 0x3B, 0x51, 0x04, 0x7D, 0x1C, 0x8B, 0x45, 0xEC, 0x6B, 0xC0, 0x3C, 0x03, 0x45, 0xF0, 0x50, 0x8B, 0x4D, 0xEC, 0x6B, 0xC9, 0x3C, 0x8B, 0x55, 0xC4, 0x03, 0x0A, 0xE8, 0xC4, 0x00, 0x00, 0x00, 0xEB, 0xD0 }; // 0x58C60E..0x58C63D
static const unsigned char F_FS[] = { 0x8B, 0x45, 0xF0, 0x8B, 0x4D, 0x08 }; // 0x712F55..0x712F5A
static const unsigned char F_SSB[] = { 0x83, 0xBD, 0x5C, 0xFF, 0xFF, 0xFF, 0x00, 0x74, 0x11 }; // 0x428B2A..0x428B32
static const unsigned char F_SSC[] = { 0x3B, 0x55, 0xF0, 0x7E, 0x2D }; // 0x428AE5..0x428AE9
static const unsigned char F_TDFN[] = { 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x10, 0x89, 0x4D, 0xF0, 0x8B, 0x45, 0x0C, 0x83, 0x38, 0x00, 0x75, 0x0D, 0x8B, 0x4D, 0x10, 0xC6, 0x01, 0x00, 0x33, 0xC0, 0xE9, 0x76, 0x01, 0x00, 0x00, 0xC7, 0x45, 0xF8, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x45, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x55, 0x08, 0x8B, 0x02, 0x89, 0x45, 0xFC, 0x8B, 0x4D, 0xFC, 0x0F, 0xBE, 0x11, 0x85, 0xD2, 0x75, 0x09, 0x8B, 0x45, 0xFC, 0x83, 0xC0, 0x01, 0x89, 0x45, 0xFC, 0x8B, 0x4D, 0x0C, 0x8B, 0x55, 0xF4, 0x3B, 0x11, 0x73, 0x0B, 0x8B, 0x45, 0xFC, 0x0F, 0xBE, 0x08, 0x83, 0xF9, 0x20, 0x74, 0x0B, 0x8B, 0x55, 0xFC, 0x0F, 0xBE, 0x02, 0x83, 0xF8, 0x09, 0x75, 0x14, 0x8B, 0x4D, 0xFC, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xFC, 0x8B, 0x55, 0xF4, 0x83, 0xC2, 0x01, 0x89, 0x55, 0xF4, 0xEB, 0xCC, 0x8B, 0x45, 0x14, 0x8B, 0x4D, 0xF4, 0x89, 0x08, 0x8B, 0x55, 0x0C, 0x8B, 0x02, 0x2B, 0x45, 0xF4, 0x8B, 0x4D, 0x0C, 0x89, 0x01, 0x8B, 0x55, 0xFC, 0x0F, 0xBE, 0x02, 0x83, 0xF8, 0x22, 0x75, 0x6E, 0x8B, 0x4D, 0xFC, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xFC, 0x8B, 0x55, 0x0C, 0x8B, 0x45, 0xF8, 0x3B, 0x02, 0x73, 0x3C, 0x8B, 0x4D, 0xFC, 0x0F, 0xBE, 0x11, 0x85, 0xD2, 0x74, 0x32, 0x8B, 0x45, 0xFC, 0x0F, 0xBE, 0x08, 0x83, 0xF9, 0x22, 0x74, 0x27, 0x8B, 0x55, 0x10, 0x8B, 0x45, 0xFC, 0x8A, 0x08, 0x88, 0x0A, 0x8B, 0x55, 0x10, 0x83, 0xC2, 0x01, 0x89, 0x55, 0x10, 0x8B, 0x45, 0xFC, 0x83, 0xC0, 0x01, 0x89, 0x45, 0xFC, 0x8B, 0x4D, 0xF8, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xF8, 0xEB, 0xBA, 0x8B, 0x55, 0xFC, 0x0F, 0xBE, 0x02, 0x83, 0xF8, 0x22, 0x75, 0x12, 0x8B, 0x4D, 0xFC, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xFC, 0x8B, 0x55, 0xF8, 0x83, 0xC2, 0x01, 0x89, 0x55, 0xF8, 0xEB, 0x67, 0x8B, 0x45, 0x0C, 0x8B, 0x4D, 0xF8, 0x3B, 0x08, 0x73, 0x5D, 0x8B, 0x55, 0xFC, 0x0F, 0xBE, 0x02, 0x85, 0xC0, 0x74, 0x53, 0x8B, 0x4D, 0xFC, 0x0F, 0xBE, 0x11, 0x83, 0xFA, 0x20, 0x74, 0x48, 0x8B, 0x45, 0xFC, 0x0F, 0xBE, 0x08, 0x83, 0xF9, 0x09, 0x74, 0x3D, 0x8B, 0x55, 0xFC, 0x0F, 0xBE, 0x02, 0x83, 0xF8, 0x0A, 0x74, 0x32, 0x8B, 0x4D, 0xFC, 0x0F, 0xBE, 0x11, 0x83, 0xFA, 0x0D, 0x74, 0x27, 0x8B, 0x45, 0x10, 0x8B, 0x4D, 0xFC, 0x8A, 0x11, 0x88, 0x10, 0x8B, 0x45, 0x10, 0x83, 0xC0, 0x01, 0x89, 0x45, 0x10, 0x8B, 0x4D, 0xFC, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xFC, 0x8B, 0x55, 0xF8, 0x83, 0xC2, 0x01, 0x89, 0x55, 0xF8, 0xEB, 0x99, 0x8B, 0x45, 0x08, 0x8B, 0x4D, 0xFC, 0x89, 0x08, 0x8B, 0x55, 0x0C, 0x8B, 0x02, 0x2B, 0x45, 0xF8, 0x8B, 0x4D, 0x0C, 0x89, 0x01, 0x8B, 0x55, 0x10, 0xC6, 0x02, 0x00, 0x33, 0xC0, 0x3B, 0x45, 0xF8, 0x1B, 0xC0, 0xF7, 0xD8, 0x8B, 0xE5, 0x5D, 0xC2, 0x10, 0x00 }; // whole C2DA::NextToken 0x71C180..0x71C319
static const unsigned char F_VMLK[] = { 0x0F, 0xB6, 0x0D, 0x7F, 0x44, 0x99, 0x00, 0x3B, 0xC1, 0x75, 0x0C, 0x8B, 0x55, 0xF4, 0x52, 0x8B, 0x4D, 0xF8, 0xE8, 0x02, 0x79, 0xF7, 0xFF, 0x33, 0xC0 }; // 0x669A27..0x669A3F
static const unsigned char F_MOV[] = { 0x8B, 0x45, 0x84, 0x8B, 0x88, 0x54, 0x02, 0x00, 0x00 }; // 0x798A47..0x798A4F
static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }
static char pad[0x700000];   // must cover 0x40F000 .. 0x99A000
static void put(unsigned at, const unsigned char* b, unsigned n) { memcpy((void*)at, b, n); }
static void stubRet(unsigned at, int v) { unsigned char s[6] = { 0xB8, 0, 0, 0, 0, 0xC3 }; memcpy(s + 1, &v, 4); put(at, s, 6); }
static void stubInc(unsigned at, volatile long* ctr, int retN) {   // inc dword [ctr]; ret / ret N
    unsigned char s[9] = { 0xFF, 0x05 }; unsigned a = (unsigned)ctr; memcpy(s + 2, &a, 4);
    if (retN) { s[6] = 0xC2; s[7] = (unsigned char)retN; s[8] = 0; put(at, s, 9); } else { s[6] = 0xC3; put(at, s, 7); }
}
struct Regs { unsigned eax, ecx, edx, ebp, target, out, esp0, esp1; };
static Regs g_r;
static unsigned run(unsigned target, unsigned eax, unsigned ecx, unsigned edx, unsigned ebp) {
    g_r.eax = eax; g_r.ecx = ecx; g_r.edx = edx; g_r.ebp = ebp; g_r.target = target; g_r.out = 0xDEADBEEF;
    __asm__ __volatile__(
        "pushal\n\t"
        "movl 12(%%esi), %%ebp\n\t"
        "movl 0(%%esi), %%eax\n\t"
        "movl 4(%%esi), %%ecx\n\t"
        "movl 8(%%esi), %%edx\n\t"
        "movl %%esp, 24(%%esi)\n\t"
        "call *16(%%esi)\n\t"
        "movl %%esp, 28(%%esi)\n\t"
        "movl %%eax, 20(%%esi)\n\t"
        "popal\n\t"
        : : "S"(&g_r) : "memory", "cc");
    return g_r.out;
}
static volatile long g_shutdowns = 0, g_dtors = 0, g_copies = 0;
static void __stdcall FakeShutdown() { ++g_shutdowns; }
static unsigned char* g_frame;   // heap (a static buffer would push pad up)
static unsigned ebpAt() { return (unsigned)g_frame + 0x200; }
static int& fr(int off) { return *(int*)(ebpAt() + off); }
static char* g_log;
static void readLog() { if (!g_log) g_log = (char*)malloc(16384); g_log[0] = 0; FILE* f = fopen("robustnessfixes_log.txt", "r"); if (!f) return; size_t k = fread(g_log, 1, 16383, f); g_log[k] = 0; fclose(f); }

// --- scenarios: each returns a small result struct the reference/patched checks compare -------------------------
static unsigned vmid(int id) { fr(8) = id; return run(0x668FD9, 0, 0, 0, ebpAt()); }
static unsigned guiid(int id) { return run(0x40F96F, 0, (unsigned)id, 0, ebpAt()); }
struct SndRes { unsigned eax; int d4; long shut; };
static SndRes sndri() { static int obj[0x40]; obj[0xD4 / 4] = 0x1234; g_shutdowns = 0; SndRes r; r.eax = run(0x7054CE, 0, 0, (unsigned)obj, ebpAt()); r.d4 = obj[0xD4 / 4]; r.shut = g_shutdowns; return r; }
static unsigned netb(unsigned readIdx, unsigned end) { fr(-0x14) = (int)end; return run(0x7040EF, readIdx, 0, 0, ebpAt()); }
struct NetARes { unsigned eax; long dtors; };
static NetARes neta() { g_dtors = 0; fr(0x10) = 0x12345; NetARes r; r.eax = run(0x704406, 0x77, 0, 0, ebpAt()); r.dtors = g_dtors; return r; }
struct KtRes { unsigned flags, p14; int ebpm4; };
static KtRes kt(unsigned site, unsigned flags) {
    static unsigned cres[0x10]; cres[0xC / 4] = 0; cres[0x14 / 4] = 0xF00DF00D; fr(-4) = 0x55;
    run(site, (unsigned)cres, 0, flags, ebpAt()); KtRes r = { cres[0xC / 4], cres[0x14 / 4], fr(-4) }; return r;
}
static unsigned* g_abThis;
static long ab(int oldCount, int newCap) {   // SetAllocated copy loop entered at 0x58C617 with j=0
    g_abThis[0] = 0x10000; g_abThis[1] = oldCount; g_abThis[2] = newCap;
    fr(-0x14) = 0; fr(-0x3C) = (int)(unsigned)g_abThis; fr(-0x10) = 0x20000; g_copies = 0;
    run(0x58C617, 0, 0, 0, ebpAt()); return g_copies;
}

typedef int (__cdecl *StFn)(int); typedef void (__cdecl *CtFn)(long*); typedef void* (__cdecl *DmFn)();
static const char* G[12] = { "kt", "vmid", "sndri", "netovf", "guiid", "abbound", "fs", "ssb", "ssc", "td", "vmlk", "movspin" };
static CtFn g_ct = nullptr;
enum { C_FS = 11, C_FSB = 12, C_SSB = 13, C_SSC = 14, C_TD = 15, C_VMLK = 16, C_F1 = 17 };
static long ctr(int i) { if (!g_ct) return 0; long c[18]; g_ct(c); return c[i]; }
static char tag[64];
static void checkf(bool ok, const char* fmt, const char* what) { char w[160]; snprintf(w, sizeof w, fmt, tag, what); check(ok, w); }

// --- 1.2.0 stub helpers ------------------------------------------------------------------------------------------
static void put32(unsigned char* p, unsigned v) { memcpy(p, &v, 4); }
static void stubJmpInd(unsigned at, void** ptrVar) { unsigned char s[6] = { 0xFF, 0x25 }; put32(s + 2, (unsigned)ptrVar); put(at, s, 6); }   // jmp [ptrVar]
static void stubCapEaxJmp(unsigned at, volatile unsigned* eaxVar, void** ptrVar) {   // mov [eaxVar],eax ; jmp [ptrVar]
    unsigned char s[11] = { 0xA3 }; put32(s + 1, (unsigned)eaxVar); s[5] = 0xFF; s[6] = 0x25; put32(s + 7, (unsigned)ptrVar); put(at, s, 11);
}
static void stubCapEaxEcxRet(unsigned at, volatile unsigned* e, volatile unsigned* c) {   // mov [e],eax ; mov [c],ecx ; ret
    unsigned char s[12] = { 0xA3 }; put32(s + 1, (unsigned)e); s[5] = 0x89; s[6] = 0x0D; put32(s + 7, (unsigned)c); s[11] = 0xC3; put(at, s, 12);
}

// --- FS: CExoResMan::Malloc join 0x712F55 (fake resman / CRes / vtable; fake OnResourceFreed and FreeResourceData) ----------------
static unsigned g_resman[8], g_cres[0x10], g_vt[8];
// volatile: written by engine-side stubs / fake callbacks the compiler cannot see being called from the asm trampoline
static volatile unsigned g_fsEax, g_fsResumeEcx, g_fsSizeAtResume;
static volatile long g_fsLeaked, g_onFreed, g_frees;
static volatile bool g_selfFreeMode;
static void* p_fsResume; static void* p_fakeFree;
static void __attribute__((fastcall)) fsResume(void* cres) {   // reached at 0x712F5B: Malloc would now overwrite +0x10
    unsigned* c = (unsigned*)cres; g_fsResumeEcx = (unsigned)cres; g_fsSizeAtResume = c[0x18 / 4];
    if (c[0x10 / 4] && !(c[0xC / 4] & 0x14)) ++g_fsLeaked;   // a stale buffer that Malloc is about to forget
}
static void __attribute__((fastcall)) fakeOnFreed(void* cres) { ++g_onFreed; if (g_selfFreeMode) ((unsigned*)cres)[0x10 / 4] = 0; }   // vtbl+0xC
static void __attribute__((thiscall)) fakeFreeRes(void* resman, void* cres) {   // 0x7120C0: refund, free, then zero +0x18/+0x10 (the W7 bug)
    unsigned* r = (unsigned*)resman; unsigned* c = (unsigned*)cres;
    if (c[0x10 / 4]) { ++g_frees; r[2] += c[0x18 / 4]; }
    c[0x18 / 4] = 0; c[0x10 / 4] = 0;
}
struct FsR { long frees, onFreed, leaked; unsigned size, ptr, eax, ecx, sizeAtResume, budget; bool balanced; };
static FsR fsRun(unsigned flags, unsigned ptr, unsigned size, bool selfFree) {
    memset(g_cres, 0, sizeof g_cres); g_vt[3] = (unsigned)fakeOnFreed; g_cres[0] = (unsigned)g_vt; g_cres[3] = flags; g_cres[4] = ptr; g_cres[6] = size;
    memset(g_resman, 0, sizeof g_resman); g_resman[2] = 0x100000;
    g_frees = g_onFreed = g_fsLeaked = 0; g_selfFreeMode = selfFree; g_fsEax = 0; g_fsResumeEcx = 0; g_fsSizeAtResume = 0xFFFF;
    fr(-0x10) = (int)(unsigned)g_resman; fr(8) = (int)(unsigned)g_cres;
    run(0x712F55, 0x11111111, 0x22222222, 0x33333333, ebpAt());
    FsR r = { g_frees, g_onFreed, g_fsLeaked, g_cres[6], g_cres[4], g_fsEax, g_fsResumeEcx, g_fsSizeAtResume, g_resman[2], g_r.esp0 == g_r.esp1 };
    return r;
}
static bool fsReplayOk(const FsR& r) { return r.eax == (unsigned)g_resman && r.ecx == (unsigned)g_cres && r.balanced; }
// level: 0 = group not patched (engine behaviour), 1 = count only, 2 = count + free
static void checkFs(int level) {
    long a0 = ctr(C_FS), b0 = ctr(C_FSB);
    FsR r = fsRun(0, 0x5000, 0x1000, false);
    long da = ctr(C_FS) - a0, db = ctr(C_FSB) - b0;
    if (level == 0)      checkf(r.frees == 0 && r.onFreed == 0 && r.leaked == 1 && r.ptr == 0x5000 && da == 0 && db == 0 && fsReplayOk(r), "%s fs%s: stale buffer kept and then forgotten by Malloc (engine behaviour, no counter)", "off");
    else if (level == 1) checkf(r.frees == 0 && r.onFreed == 0 && r.leaked == 1 && r.ptr == 0x5000 && r.size == 0x1000 && da == 1 && db == 0x1000 && fsReplayOk(r), "%s fs%s: fs=1 counts fs_stale +1 / bytes +0x1000, keeps the buffer, replay + stack ok", "=1");
    else                 checkf(r.frees == 1 && r.onFreed == 1 && r.leaked == 0 && r.ptr == 0 && r.sizeAtResume == 0x1000 && r.size == 0x1000 && r.budget == 0x101000 && da == 1 && db == 0x1000 && fsReplayOk(r),
                                "%s fs%s: fs=2 frees once (vtbl+0xC + 0x7120C0), +0x18 restored, budget refunded once, counted", "=2");
    // cases that must never count / free at any level
    struct { unsigned flags, ptr; const char* what; } nz[] = { { 4, 0x5000, "flags 4 (loaded)" }, { 0x10, 0x5000, "flags 0x10 (async)" }, { 0x14, 0x5000, "flags 0x14" }, { 0, 0, "+0x10 == 0" } };
    for (auto& c : nz) {
        long a1 = ctr(C_FS); FsR q = fsRun(c.flags, c.ptr, 0x1000, false); long d1 = ctr(C_FS) - a1;
        checkf(q.frees == 0 && q.onFreed == 0 && d1 == 0 && q.size == 0x1000 && q.ptr == c.ptr && fsReplayOk(q), "%s fs: no count / no free for %s", c.what);
    }
    if (level >= 1) {
        long a1 = ctr(C_FS), b1 = ctr(C_FSB); FsR q = fsRun(2, 0x5000, 0x20, false); long d1 = ctr(C_FS) - a1, e1 = ctr(C_FSB) - b1;
        checkf(d1 == 1 && e1 == 0x20 && fsReplayOk(q) && q.size == 0x20, "%s fs: flags 2 (not 4/0x10) still stale -> counted (%s)", level == 2 ? "fs=2" : "fs=1");
    }
    if (level == 2) {
        long a1 = ctr(C_FS); FsR q = fsRun(0, 0x5000, 0x1000, true); long d1 = ctr(C_FS) - a1;
        checkf(q.onFreed == 1 && q.frees == 0 && q.ptr == 0 && q.size == 0x1000 && q.sizeAtResume == 0x1000 && d1 == 1 && fsReplayOk(q), "%s fs=2: vtbl+0xC frees +0x10 itself -> no second free, +0x18 restored (%s)", "self-free");
    }
}

// --- SS-b / SS-c: CaptureScreenToTGA -----------------------------------------------------------------------------------------
static unsigned ssbRun(int cam, bool* bal) { fr(-0xA4) = cam; unsigned r = run(0x428B2A, 0, 0, 0, ebpAt()); *bal = g_r.esp0 == g_r.esp1; return r; }
static void checkSsb(bool on) {
    bool b0, b1; long c0 = ctr(C_SSB);
    unsigned rn = ssbRun(0, &b0), rp = ssbRun(0x1234, &b1); long dc = ctr(C_SSB) - c0;
    if (on) checkf(rn == 2 && rp == 1 && dc == 1 && b0 && b1, "%s ssb: camera NULL skips the render block (0x428B88) + counted once, present -> normal path (%s)", "patched");
    else    checkf(rn == 3 && rp == 1 && dc == 0 && b0 && b1, "%s ssb: camera NULL takes the engine NULL arm 0x428B44 (bug), present -> normal path (%s)", "not patched");
}
static unsigned sscRun(int w, int h, bool* bal) { fr(-0x10) = h; unsigned r = run(0x428AE5, 0, 0, (unsigned)w, ebpAt()); *bal = g_r.esp0 == g_r.esp1; return r; }
static void checkSsc(bool on) {
    bool b0, b1, b2; long c0 = ctr(C_SSC);
    unsigned wide = sscRun(0x400, 0x300, &b0), tall = sscRun(0x300, 0x400, &b1), sq = sscRun(0x300, 0x300, &b2); long dc = ctr(C_SSC) - c0;
    if (on) checkf(wide == 1 && tall == 1 && sq == 1 && dc == 2 && b0 && b1 && b2, "%s ssc: dims writers always reached (0x428AEA); counts w<h and w==h only (%s)", "patched");
    else    checkf(wide == 1 && tall == 2 && sq == 2 && dc == 0 && b0 && b1 && b2, "%s ssc: w<=h jumps over the dims writers (bug) (%s)", "not patched");
}

// --- TD: the whole real C2DA::NextToken (0x71C180) ---------------------------------------------------------------------------
typedef unsigned char (__attribute__((thiscall)) *NextTokFn)(void*, char**, unsigned*, char*, unsigned*);
struct TdR { unsigned ret, remaining, skipped, cursorOff; size_t len; bool canary; unsigned char dst[0x400]; };
static TdR tdRun(int len, bool quoted) {
    static char src[1024]; TdR r; memset(&r, 0, sizeof r);
    int n = 0; if (quoted) src[n++] = '"'; for (int i = 0; i < len; ++i) src[n++] = (char)('a' + i % 26); if (quoted) src[n++] = '"';
    memcpy(src + n, " tail", 6);
    char* cur = src; unsigned remaining = (unsigned)strlen(src), skipped = 0xFFFF;
    memset(r.dst, 0xCC, sizeof r.dst); r.dst[0x3FF] = 0;
    r.ret = ((NextTokFn)0x71C180)(nullptr, &cur, &remaining, (char*)r.dst, &skipped);
    r.remaining = remaining; r.skipped = skipped; r.cursorOff = (unsigned)(cur - src); r.len = strlen((char*)r.dst);
    r.canary = true; for (int i = 256; i < 0x3FF; ++i) if (r.dst[i] != 0xCC) { r.canary = false; break; }
    return r;
}
static const int TD_LENS[4] = { 10, 255, 256, 300 };
static TdR g_tdRef[8];
static bool tdSame(const TdR& a, const TdR& b) { return a.ret == b.ret && a.remaining == b.remaining && a.skipped == b.skipped && a.cursorOff == b.cursorOff && !memcmp(a.dst, b.dst, sizeof a.dst); }
static void tdReference() {   // reference run (original bytes)
    for (int q = 0; q < 2; ++q) for (int i = 0; i < 4; ++i) g_tdRef[q * 4 + i] = tdRun(TD_LENS[i], q != 0);
    check(g_tdRef[0].canary && g_tdRef[1].canary && g_tdRef[1].len == 255 && !g_tdRef[2].canary && !g_tdRef[3].canary && g_tdRef[3].len == 300, "ref td: bare token 256+ chars writes past the 256-byte buffer (bug), 255 fits");
    check(g_tdRef[4].canary && g_tdRef[5].canary && !g_tdRef[6].canary && !g_tdRef[7].canary && g_tdRef[7].len == 300, "ref td: quoted token 256+ chars writes past the buffer (bug)");
}
static void checkTd(bool on) {
    for (int q = 0; q < 2; ++q) for (int i = 0; i < 4; ++i) {
        int len = TD_LENS[i]; const TdR& ref = g_tdRef[q * 4 + i];
        long c0 = ctr(C_TD); TdR r = tdRun(len, q != 0); long dc = ctr(C_TD) - c0;
        char what[64]; snprintf(what, sizeof what, "%s token len %d", q ? "quoted" : "bare", len);
        // counted when the length accounting ([ebp-8]) exceeds 255: bare len >= 256; quoted counts the closing quote, so len >= 255
        long expCount = on ? ((q ? len >= 255 : len >= 256) ? 1 : 0) : 0;
        bool fits = (q ? len <= 254 : len <= 255);
        if (!on) checkf(tdSame(r, ref) && dc == 0, "%s td not patched: identical to the reference run (%s)", what);
        else if (fits || len == 255) checkf(tdSame(r, ref) && dc == expCount, "%s td patched: tokens that fit behave exactly as before, counter ok (%s)", what);
        else checkf(r.canary && r.len == 255 && r.dst[255] == 0 && r.ret == 1 && !memcmp(r.dst, ref.dst, 255) && r.remaining == ref.remaining && r.cursorOff == ref.cursorOff && r.skipped == ref.skipped && dc == 1,
                    "%s td patched: clamped to 255 chars, cookie intact, whole token consumed, counted (%s)", what);
    }
}

// --- VMLK: ActionDoCommand failure path -----------------------------------------------------------------------------------------
static volatile long g_handoff, g_delsit, g_wrongDel; static volatile unsigned g_hoOwner, g_hoSit, g_delArg;
static void* p_handoff; static void* p_delsit; static void* p_wrongDel;
static void __attribute__((thiscall)) fakeHandoff(void* owner, void* sit) { ++g_handoff; g_hoOwner = (unsigned)owner; g_hoSit = (unsigned)sit; }   // 0x5E1340
static void __stdcall fakeDelSit(void* sit) { ++g_delsit; g_delArg = (unsigned)sit; }   // 0x7037C0 (ignores ECX)
static void __stdcall fakeWrongDel(void* sit) { ++g_wrongDel; (void)sit; }   // 0x6FDBB0: thiscall needing ECX=owner (1.2.0 bug); must never be called
static unsigned vmlkRun(unsigned lookup, bool* bal) {
    fr(-0xC) = 0x5150; fr(-8) = 0x7770; g_handoff = g_delsit = g_wrongDel = 0; g_hoOwner = g_hoSit = g_delArg = 0;
    unsigned r = run(0x669A27, lookup, 0, 0, ebpAt()); *bal = g_r.esp0 == g_r.esp1; return r;
}
static void checkVmlk(bool on) {
    bool b0, b1; long c0 = ctr(C_VMLK);
    unsigned rf = vmlkRun(0, &b0); bool foundOk = rf == 0 && b0 && g_handoff == 1 && g_hoOwner == 0x7770 && g_hoSit == 0x5150 && g_delsit == 0;
    long dFound = ctr(C_VMLK) - c0;
    unsigned rm = vmlkRun(1, &b1); long dMiss = ctr(C_VMLK) - c0;
    checkf(foundOk && dFound == 0, "%s vmlk: object found -> hand-over 0x5E1340(owner, situation), nothing freed, not counted (%s)", on ? "patched" : "not patched");
    if (on) checkf(rm == 0 && b1 && g_handoff == 0 && g_delsit == 1 && g_wrongDel == 0 && g_delArg == 0x5150 && dMiss == 1, "%s vmlk: object missing -> situation freed via 0x7037C0 once (never 0x6FDBB0) + counted, returns 0 (%s)", "patched");
    else    checkf(rm == 0 && b1 && g_handoff == 0 && g_delsit == 0 && dMiss == 0, "%s vmlk: object missing -> situation dropped, never freed (leak) (%s)", "not patched");
}

// --- F1 / movspin: PlayMovies pump branch --------------------------------------------------------------------------------------
static volatile unsigned g_movEax, g_movEcx, g_sleepArg; static unsigned g_thisObj[0x100]; static volatile long g_sleeps;
static void __stdcall FakeSleep(unsigned ms) { ++g_sleeps; g_sleepArg = ms; }
static void checkMov(bool on) {
    g_thisObj[0x254 / 4] = 0x77; fr(-0x7C) = (int)(unsigned)g_thisObj; g_sleeps = 0; g_sleepArg = 0; g_movEax = g_movEcx = 0;
    long c0 = ctr(C_F1);
    run(0x798A47, 1, 2, 3, ebpAt()); long dc = ctr(C_F1) - c0;
    bool ok = g_movEax == (unsigned)g_thisObj && g_movEcx == 0x77 && g_r.esp0 == g_r.esp1;
    if (on) checkf(ok && g_sleeps == 1 && g_sleepArg == 1 && dc == 1, "%s movspin: one Sleep(1) via the IAT per pump, counted, replay + stack ok (%s)", "ON");
    else    checkf(ok && g_sleeps == 0 && dc == 0, "%s movspin: no Sleep, original path (%s)", "off");
}

// --- generic site table ----------------------------------------------------------------------------------------------------------
struct SiteRow { int g; unsigned at, n; };
static const SiteRow SITES[] = { {0,0x726527,10}, {0,0x726C9C,5}, {1,0x668FE0,2}, {1,0x669012,5}, {2,0x7054CE,12}, {3,0x704406,15}, {3,0x7040EF,5}, {4,0x40F972,6}, {5,0x58C61A,8},
                                 {6,0x712F55,6}, {7,0x428B2A,9}, {8,0x428AE5,5}, {9,0x71C243,37}, {9,0x71C2C9,37}, {9,0x71C2F0,6}, {10,0x669A27,11}, {11,0x798A47,9} };
static const int NSITES = sizeof SITES / sizeof SITES[0];
static unsigned char g_snap[NSITES][40];
static void checkStates(StFn st, const int want[12]) {
    for (int g = 0; g < 12; ++g) {
        bool changed = false, unchanged = true;
        for (int i = 0; i < NSITES; ++i) if (SITES[i].g == g) { bool c = memcmp(g_snap[i], (void*)SITES[i].at, SITES[i].n) != 0; changed |= c; if (c) unchanged = false; else changed = changed; }
        bool allChanged = true; for (int i = 0; i < NSITES; ++i) if (SITES[i].g == g && !memcmp(g_snap[i], (void*)SITES[i].at, SITES[i].n)) allChanged = false;
        const char* ws = want[g] == 1 ? "ON" : want[g] == 0 ? "off" : "REFUSED(bytes)";
        char w[128]; snprintf(w, sizeof w, "%s: state %s = %s, %s", tag, G[g], ws, want[g] == 1 ? "every site rewritten" : "every site untouched");
        check(st(g) == want[g] && (want[g] == 1 ? allChanged : unchanged), w);
    }
}
static void dumpCaves(const char* mode, StFn st) {
    char name[96]; snprintf(name, sizeof name, "caves_%s.txt", mode); FILE* idx = fopen(name, "w"); if (!idx) return;
    for (int i = 0; i < NSITES; ++i) if (st(SITES[i].g) == 1) {
        snprintf(name, sizeof name, "site_%s_%08x.bin", mode, SITES[i].at); if (FILE* f = fopen(name, "wb")) { fwrite((void*)SITES[i].at, 1, SITES[i].n, f); fclose(f); }
        if (*(unsigned char*)SITES[i].at == 0xE9) {
            int rel; memcpy(&rel, (void*)(SITES[i].at + 1), 4); unsigned cave = SITES[i].at + 5 + rel;
            fprintf(idx, "%s %08x %08x %u\n", G[SITES[i].g], SITES[i].at, cave, SITES[i].n);
            snprintf(name, sizeof name, "cave_%s_%08x.bin", mode, SITES[i].at); if (FILE* f = fopen(name, "wb")) { fwrite((void*)cave, 1, 0x60, f); fclose(f); }
        } else fprintf(idx, "%s %08x inplace %u\n", G[SITES[i].g], SITES[i].at, SITES[i].n);
    }
    fclose(idx);
}
struct Corr { unsigned at; unsigned char orig, bad; };

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    // The only site below the pad is guiid 0x40F96F..0x40F9E5: it lies in the zero page-padding after this exe's own .rdata (checked below);
    // everything from 0x428A00 up (all other sites, IAT slots, stubs) is the .bss pad.
    if ((unsigned)pad > 0x428A00 || (unsigned)pad + sizeof pad < 0x99A000) { printf("pad at %p does not cover 0x428A00-0x99A000\n", (void*)pad); return 2; }
    DWORD o; if (!VirtualProtect((void*)0x40F000, 0x99A000 - 0x40F000, PAGE_EXECUTE_READWRITE, &o)) { printf("VirtualProtect failed %lu\n", GetLastError()); return 2; }
    for (unsigned a = 0x40F960; a < 0x40FA00; ++a) if (*(unsigned char*)a) { printf("0x40F960-0x40FA00 is not free image padding (byte at %x)\n", a); return 2; }
    g_frame = (unsigned char*)calloc(1, 0x400); g_abThis = (unsigned*)calloc(4, 4);
    snprintf(tag, sizeof tag, "%s", mode);
    // real site bytes + marker stubs
    put(0x668FD9, F_VMID, sizeof F_VMID); stubRet(0x668FE2, 1); { static const unsigned char vmRej[] = { 0xB8, 0x2E, 0xF8, 0xFF, 0xFF, 0xC3 }; put(0x669012, vmRej, sizeof vmRej); }   // real B8 -2002 + ret at 0x669017
    put(0x40F96F, F_GUIID, sizeof F_GUIID); stubRet(0x40F978, 1); stubRet(0x40F9E4, 0);
    put(0x7054CE, F_SNDRI, sizeof F_SNDRI); stubRet(0x7054DA, 2); stubRet(0x70552C, 3); *(void**)0x98657C = (void*)&FakeShutdown;
    put(0x7040EF, F_NETB, sizeof F_NETB); stubRet(0x7040FD, 9); stubRet(0x704415, 5);
    put(0x704406, F_NETA, sizeof F_NETA); stubInc(0x733780, &g_dtors, 0); *(unsigned char*)0x70446D = 0xC3;
    put(0x726527, F_KTA, sizeof F_KTA); *(unsigned char*)0x726531 = 0xC3;
    put(0x726C9C, F_KTB, sizeof F_KTB); *(unsigned char*)0x726CDD = 0xC3;
    put(0x58C60E, F_AB, sizeof F_AB); *(unsigned char*)0x58C63E = 0xC3; stubInc(0x58C700, &g_copies, 4);
    // 1.2.0 sites
    p_fsResume = (void*)&fsResume; p_fakeFree = (void*)&fakeFreeRes; p_handoff = (void*)&fakeHandoff; p_delsit = (void*)&fakeDelSit; p_wrongDel = (void*)&fakeWrongDel;
    put(0x712F55, F_FS, sizeof F_FS); stubCapEaxJmp(0x712F5B, &g_fsEax, &p_fsResume); stubJmpInd(0x7120C0, &p_fakeFree);
    put(0x428B2A, F_SSB, sizeof F_SSB); stubRet(0x428B33, 1); stubRet(0x428B44, 3); stubRet(0x428B88, 2);
    put(0x428AE5, F_SSC, sizeof F_SSC); stubRet(0x428AEA, 1); stubRet(0x428B17, 2);
    put(0x71C180, F_TDFN, sizeof F_TDFN);
    *(unsigned char*)0x99447F = 0; put(0x669A27, F_VMLK, sizeof F_VMLK); *(unsigned char*)0x669A40 = 0xC3; stubJmpInd(0x5E1340, &p_handoff); stubJmpInd(0x7037C0, &p_delsit); stubJmpInd(0x6FDBB0, &p_wrongDel);
    put(0x798A47, F_MOV, sizeof F_MOV); stubCapEaxEcxRet(0x798A50, &g_movEax, &g_movEcx); *(void**)0x9860D4 = (void*)&FakeSleep;

    // reference run (original bytes = engine behaviour incl. the bugs)
    check(vmid(5) == 1 && vmid(0x36D) == (unsigned)-2002 && vmid(-1) == 1, "ref vmid: id -1 dispatched (bug)");
    check(guiid(5) == 1 && guiid(-1) == 0 && guiid(5000) == 1, "ref guiid: id 5000 accepted (bug)");
    SndRes s0 = sndri(); check(s0.eax == 3 && s0.d4 == 0 && s0.shut == 0, "ref sndri: driver cleared, startup skipped (bug)");
    check(netb(0x8000, 0x100) == 5 && netb(0x8000, 0x1A000) == 9 && netb(0x8000, 0x20010) == 9, "ref netovf B: lap test");
    NetARes n0 = neta(); check(n0.eax == 5 && n0.dtors == 1, "ref netovf A: overflow branch falls into the memcpy path (bug)");
    KtRes k0 = kt(0x726527, 0x200); check(k0.flags == 0x200 && k0.p14 == 0xF00DF00D && k0.ebpm4 == -1, "ref kt A: CRes+0x14 left dangling (bug)");
    KtRes k1 = kt(0x726C9C, 0x201); check(k1.flags == 0x201 && k1.p14 == 0xF00DF00D, "ref kt B: CRes+0x14 left dangling (bug)");
    check(ab(5, 2) == 5 && ab(2, 5) == 2, "ref ab: shrink copies the OLD count (bug)");
    strcpy(tag, "ref"); checkFs(0); checkSsb(false); checkSsc(false); tdReference(); checkTd(false); checkVmlk(false); checkMov(false); snprintf(tag, sizeof tag, "%s", mode);

    // per-mode setup: corruptions, config, expected states
    static const int WANT_RUN[12]    = { 1,1,1,1,1,1, 1,1,1,1,1,0 };
    static const int WANT_MOVON[12]  = { 1,1,1,1,1,1, 1,1,1,1,1,1 };
    static const int WANT_OFF[12]    = { 0,0,0,0,0,0, 0,0,0,0,0,0 };
    static const int WANT_KEYS[12]   = { 1,0,1,1,0,1, 0,1,0,0,1,0 };
    static const int WANT_KEYS2[12]  = { 0,1,0,0,1,0, 1,0,1,1,0,1 };
    static const int WANT_REF1[12]   = { -1,-1,1,1,1,1, 1,1,1,1,1,0 };
    static const int WANT_REF2[12]   = { 1,1,1,1,1,1, -1,-1,-1,-1,-1,-1 };
    const int* want = WANT_RUN; int fsLvl = 1; const char* cfg = nullptr;
    Corr corr[8]; int ncorr = 0;
    bool isRun = !strcmp(mode, "run"), isFs2 = !strcmp(mode, "fs2"), isMovon = !strcmp(mode, "movon"), isDis = !strcmp(mode, "disabled");
    bool isKeys = !strcmp(mode, "keys"), isKeys2 = !strcmp(mode, "keys2"), isRef = !strcmp(mode, "refuse"), isRef2 = !strcmp(mode, "refuse2");
    if (!(isRun || isFs2 || isMovon || isDis || isKeys || isKeys2 || isRef || isRef2)) { printf("unknown mode %s\n", mode); return 2; }
    if (isFs2) { cfg = "fs=2\n"; fsLvl = 2; }
    if (isMovon) { cfg = "movspin=1\n"; want = WANT_MOVON; }
    if (isDis) { cfg = "enabled=0\n"; want = WANT_OFF; fsLvl = 0; }
    if (isKeys) { cfg = "vmid=0\nguiid = 0\nkt=1\nfs=0\nssc=0\ntd=0\nmovspin=0\n"; want = WANT_KEYS; fsLvl = 0; }
    if (isKeys2) { cfg = "ssb=0\nvmlk=0\nsndri=0\nnetovf=0\nkt=0\nabbound=0\nfs=1\nmovspin=1\n"; want = WANT_KEYS2; }
    if (isRef) { want = WANT_REF1; corr[ncorr++] = { 0x726C9E, 0x0C, 0x0D }; corr[ncorr++] = { 0x669014, 0xF8, 0xF9 }; }   // kt (2nd site) + vmid (2nd site only; site 1 intact)
    if (isRef2) {
        want = WANT_REF2; fsLvl = 0; cfg = "movspin=1\n";
        corr[ncorr++] = { 0x712F57, 0xF0, 0xF1 }; corr[ncorr++] = { 0x428B2E, 0xFF, 0xFE }; corr[ncorr++] = { 0x428AE7, 0xF0, 0xF1 };
        corr[ncorr++] = { 0x71C2F2, 0x08, 0x09 };   // td: the COUNTER site only; the two in-place blocks must stay untouched (all or none)
        corr[ncorr++] = { 0x669A2A, 0x7F, 0x7E }; corr[ncorr++] = { 0x798A49, 0x84, 0x85 };
    }
    for (int i = 0; i < ncorr; ++i) { if (*(unsigned char*)corr[i].at != corr[i].orig) { printf("corruption setup: byte at %x is not %02x\n", corr[i].at, corr[i].orig); return 2; } *(unsigned char*)corr[i].at = corr[i].bad; }
    for (int i = 0; i < NSITES; ++i) memcpy(g_snap[i], (void*)SITES[i].at, SITES[i].n);
    DeleteFileA("robustnessfixes_log.txt");
    if (cfg) { FILE* f = fopen("robustnessfixes.txt", "w"); fputs(cfg, f); fclose(f); } else DeleteFileA("robustnessfixes.txt");

    HMODULE m = LoadLibraryA("windows_x86.dll"); if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    StFn st = (StFn)GetProcAddress(m, "RobFixTestState"); CtFn ct = (CtFn)GetProcAddress(m, "RobFixTestCounters"); DmFn dm = (DmFn)GetProcAddress(m, "RobFixTestDummy");
    check(st && ct && dm, "test exports present"); if (!st || !ct || !dm) return 3;
    g_ct = ct;
    readLog();
    long c[18], c0[18], c1[18];
    checkStates(st, want);
    if (isRun || isFs2 || isMovon) dumpCaves(mode, st);
    int npatched = 0; for (int g = 0; g < 12; ++g) if (want[g] == 1) ++npatched;
    char loaded[400]; snprintf(loaded, sizeof loaded, "%d/12 groups patched:", npatched); check(strstr(g_log, loaded) != NULL, "log: loaded line count");
    check(strstr(g_log, "k2-robustness-fixes 1.2.1 loaded") != NULL, "log: version 1.2.1");
    if (isRun || isFs2 || isMovon) {
        const char* names[12] = { "kt", "vmid", "sndri", "netovf", "guiid", "abbound", "fs", "ssb", "ssc", "td", "vmlk", "movspin" };
        char exp[400]; int k = 0; for (int g = 0; g < 12; ++g) {
            if (g == 6) k += snprintf(exp + k, sizeof exp - k, " fs=ON(%s)", fsLvl == 2 ? "free" : "count");
            else k += snprintf(exp + k, sizeof exp - k, " %s=%s", names[g], want[g] == 1 ? "ON" : "off");
        }
        char w[500]; snprintf(w, sizeof w, "%d/12 groups patched:%s", npatched, exp); check(strstr(g_log, w) != NULL, "log: loaded line with per-group state (fs mode, movspin)");
    }
    if (isRef) {
        check(strstr(g_log, "kt REFUSED: bytes at 0x00726c9c are 89 50 0d eb 3c") != NULL && strstr(g_log, "vmid REFUSED: bytes at 0x00669012 are b8 2e f9 ff ff") != NULL, "log: REFUSED lines with the seen bytes");
        check(strstr(g_log, "9/12 groups patched: kt=REFUSED(bytes) vmid=REFUSED(bytes)") != NULL, "log: loaded line");
        check(*(unsigned char*)0x668FE0 == 0x7D, "refuse: vmid site 1 still 7D (group refuses as a whole)");
    }
    if (isRef2) {
        check(strstr(g_log, "fs REFUSED: bytes at 0x00712f55 are 8b 45 f1 8b 4d 08 (group untouched)") != NULL, "log: fs REFUSED line");
        check(strstr(g_log, "ssb REFUSED: bytes at 0x00428b2a are 83 bd 5c ff fe ff 00 74 11 (group untouched)") != NULL, "log: ssb REFUSED line");
        check(strstr(g_log, "ssc REFUSED: bytes at 0x00428ae5 are 3b 55 f1 7e 2d (group untouched)") != NULL, "log: ssc REFUSED line");
        check(strstr(g_log, "td REFUSED: bytes at 0x0071c2f0 are 8b 45 09 8b 4d fc (group untouched)") != NULL, "log: td REFUSED line (counter site seen, in-place blocks untouched)");
        check(strstr(g_log, "vmlk REFUSED: bytes at 0x00669a27 are 0f b6 0d 7e 44 99 00 3b c1 75 0c (group untouched)") != NULL, "log: vmlk REFUSED line");
        check(strstr(g_log, "movspin REFUSED: bytes at 0x00798a47 are 8b 45 85 8b 88 54 02 00 00 (group untouched)") != NULL, "log: movspin REFUSED line");
        check(strstr(g_log, "6/12 groups patched: kt=ON vmid=ON sndri=ON netovf=ON guiid=ON abbound=ON fs=REFUSED(bytes) ssb=REFUSED(bytes) ssc=REFUSED(bytes) td=REFUSED(bytes) vmlk=REFUSED(bytes) movspin=REFUSED(bytes)") != NULL, "log: loaded line");
    }
    for (int i = 0; i < ncorr; ++i) *(unsigned char*)corr[i].at = corr[i].orig;   // refused groups must now behave like the reference run

    if (isRun) {
        void* dummy = dm();
        check(vmid(5) == 1 && vmid(0x36C) == 1 && vmid(0x36D) == (unsigned)-2002 && vmid(-1) == (unsigned)-2002 && vmid(0x8000 - 0x10000) == (unsigned)-2002, "vmid: valid ids dispatched, negative ids rejected (-2002)");
        ct(c0);
        check(vmid(0x36D) == (unsigned)-2002 && vmid(-1) == (unsigned)-2002, "vmid counters: both rejects return -2002 through the cave");
        ct(c1); check(c1[4] - c0[4] == 2 && c1[5] - c0[5] == 1 && c1[6] == -1, "vmid counters: vm_reject +2, vm_neg +1, vm_last_id == -1");
        check(vmid(5) == 1, "vmid counters: valid id 5 takes the accept path");
        ct(c0); check(c0[4] == c1[4] && c0[5] == c1[5], "vmid counters: accepted id not counted");
        check(guiid(0) == 1 && guiid(143) == 1 && guiid(4095) == 1 && guiid(4096) == 0 && guiid(5000) == 0 && guiid(-1) == 0 && guiid(0x7FFFFFFF) == 0, "guiid: 0..4095 accepted, >=4096 and negative rejected");
        ct(c0);
        check(guiid(5000) == 0, "guiid counters: id 5000 -> rejected path (0)");
        ct(c1); check(c1[7] - c0[7] == 1 && c1[8] == 5000, "guiid counters: gui_reject +1, gui_last_id == 5000");
        check(guiid(-1) == 0 && guiid(10) == 1, "guiid counters: -1 rejected, 10 accepted");
        ct(c0); check(c0[7] == c1[7] && c0[8] == c1[8], "guiid counters: negative and valid ids not counted");
        SndRes s1 = sndri(); check(s1.eax == 2 && s1.shut == 1 && s1.d4 == 0x1234, "sndri: shutdown called once, then the startup block runs");
        check(netb(0x8000, 0x100) == 5 && netb(0x8000, 0x1A000) == 5 && netb(0x8000, 0x20000) == 5 && netb(0x8000, 0x20001) == 9, "netovf B: only end > 0x20000 takes the overflow branch (lap case unchanged)");
        NetARes n1 = neta(); check(n1.eax == 0 && n1.dtors == 1, "netovf A: overflow drops the message (string destroyed once, returns 0)");
        KtRes k2 = kt(0x726527, 0x200); check(k2.flags == 0x200 && k2.p14 == (unsigned)dummy && k2.ebpm4 == -1, "kt A: flag store replayed, CRes+0x14 = dummy, [ebp-4] = -1");
        KtRes k3 = kt(0x726C9C, 0x201); check(k3.flags == 0x201 && k3.p14 == (unsigned)dummy, "kt B: flag store replayed, CRes+0x14 = dummy");
        long cp1 = ab(5, 2), cp2 = ab(2, 5), cp3 = ab(3, 3), cp4 = ab(0, 4);
        check(cp1 == 2 && cp2 == 2 && cp3 == 3 && cp4 == 0, "ab: copy bounded by min(old,new)");
        ct(c); check(c[0] == 1 && c[1] == 1 && c[2] == 1, "counters kt_destroy=1 kt_update=1 ab_clamp=1");
        check(c[3] == 1, "counter sndri_restart == 1");
        check(c[9] == 1 && c[10] == 0x12345, "counters net_drop == 1, net_last_len == 0x12345");
        bool zero = true; for (int i = 0; i < 0x24; ++i) if (((unsigned char*)dummy)[i]) zero = false;
        check(zero, "kt dummy entry still all zero");
    } else if (!isRef && !isRef2) {
        // light checks for the 1.1.0 groups in the remaining modes: patched iff want[g] == 1
        void* dummy = dm();
        checkf(vmid(-1) == (want[1] == 1 ? (unsigned)-2002 : 1u), "%s: vmid id -1 %s", want[1] == 1 ? "rejected" : "dispatched (original)");
        checkf(guiid(5000) == (want[4] == 1 ? 0u : 1u), "%s: guiid id 5000 %s", want[4] == 1 ? "rejected" : "accepted (original)");
        checkf(sndri().eax == (want[2] == 1 ? 2u : 3u), "%s: sndri %s", want[2] == 1 ? "patched" : "original");
        checkf(neta().eax == (want[3] == 1 ? 0u : 5u), "%s: netovf A %s", want[3] == 1 ? "patched" : "original");
        checkf(kt(0x726527, 0x200).p14 == (want[0] == 1 ? (unsigned)dummy : 0xF00DF00Du), "%s: kt A %s", want[0] == 1 ? "patched" : "original");
        checkf(ab(5, 2) == (want[5] == 1 ? 2 : 5), "%s: abbound %s", want[5] == 1 ? "patched" : "original");
    } else {
        // refuse modes: the still-patched 1.1.0 groups work
        void* dummy = dm();
        if (isRef2) check(vmid(-1) == (unsigned)-2002 && guiid(5000) == 0 && sndri().eax == 2 && neta().eax == 0 && kt(0x726527, 0x200).p14 == (unsigned)dummy && ab(5, 2) == 2, "refuse2: the six 1.1.0 groups are patched and work");
        else check(sndri().eax == 2 && guiid(5000) == 0 && neta().eax == 0 && ab(5, 2) == 2 && vmid(-1) == 1 && kt(0x726527, 0x200).p14 == 0xF00DF00Du, "refuse: sndri/guiid/netovf/abbound patched, vmid/kt refused (original behaviour)");
    }
    // 1.2.0 groups: behaviour must match the expected state in every mode
    checkFs(want[6] == 1 ? fsLvl : 0); checkSsb(want[7] == 1); checkSsc(want[8] == 1); checkTd(want[9] == 1); checkVmlk(want[10] == 1); checkMov(want[11] == 1);

    if (isRun || isFs2 || isMovon) {
        ct(c);
        Sleep(5600); readLog();   // helper thread polls every 5 s
        char exp[900]; snprintf(exp, sizeof exp, "COUNTERS kt_destroy_orphan=%ld kt_update_orphan=%ld ab_shrink_clamp=%ld sndri_restart=%ld vm_reject=%ld vm_neg=%ld vm_last_id=%ld "
            "gui_reject=%ld gui_last_id=%ld net_drop=%ld net_last_len=%ld fs_stale=%ld fs_stale_bytes=%ld ssb_camnull=%ld ssc_dims=%ld td_long=%ld vmlk_free=%ld f1_sleeps=%ld",
            c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13], c[14], c[15], c[16], c[17]);
        check(strstr(g_log, exp) != NULL, "log: COUNTERS line with all 18 values");
        char f[200];
        if (isRun) {
            check(strstr(g_log, "FIRED sndri restart=1") && strstr(g_log, "FIRED vmid neg=") && strstr(g_log, "FIRED guiid reject=") &&
                  strstr(g_log, "FIRED netovf drop=1 last_len=74565") && strstr(g_log, "FIRED abbound clamp=1") && strstr(g_log, "FIRED kt destroy=1 update=1"), "log: one-shot FIRED lines for the 1.1.0 groups");
            check(c[11] == 2 && c[12] == 0x1020 && c[16] == 1 && c[13] == 1 && c[14] == 2 && c[17] == 0, "counters fs_stale=2 fs_stale_bytes=0x1020 ssb=1 ssc=2 vmlk=1 f1_sleeps=0 (default config)");
            check(strstr(g_log, "FIRED movspin") == NULL, "log: no FIRED movspin (default off)");
        }
        snprintf(f, sizeof f, "FIRED fs stale=%ld bytes=%ld mode=%s", c[11], c[12], fsLvl == 2 ? "free" : "count"); check(strstr(g_log, f) != NULL, "log: FIRED fs (mode shown)");
        snprintf(f, sizeof f, "FIRED ssb camnull=%ld", c[13]); check(strstr(g_log, f) != NULL, "log: FIRED ssb");
        snprintf(f, sizeof f, "FIRED ssc dims=%ld", c[14]); check(strstr(g_log, f) != NULL, "log: FIRED ssc");
        snprintf(f, sizeof f, "FIRED td long=%ld", c[15]); check(strstr(g_log, f) != NULL, "log: FIRED td");
        snprintf(f, sizeof f, "FIRED vmlk free=%ld", c[16]); check(strstr(g_log, f) != NULL, "log: FIRED vmlk");
        if (isMovon) { snprintf(f, sizeof f, "FIRED movspin sleeps=%ld", c[17]); check(c[17] == 1 && strstr(g_log, f) != NULL, "log: FIRED movspin sleeps=1 (movspin=1)"); }
        if (isFs2) check(c[11] == 3 && c[12] == 0x2020, "counters fs=2: fs_stale=3 (stale, flags 2, self-free) bytes=0x2020");
    }
    if (cfg) DeleteFileA("robustnessfixes.txt");
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
