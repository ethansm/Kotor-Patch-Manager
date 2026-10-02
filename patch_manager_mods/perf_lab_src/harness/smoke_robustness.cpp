// Smoke test for k2-robustness-fixes 1.1.0 (DLL-only; DllMain patches 6 independent groups, doc 10 "Robustness set"; 1.1.0 adds
// fire counters and cave-based sites for vmid (2nd site 0x669012), sndri, guiid and netovf A, all checked here).
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke_robustness.exe smoke_robustness.cpp
//        (default base 0x400000; the .bss pad covers 0x40F000-0x990000 so every site, the IAT slot 0x98657C and the stub
//        targets are ordinary memory of this exe; check `objdump -h`: .bss must start below 0x40F000)
// Run:   smoke_robustness.exe <run|disabled|refuse|keys>   (dir needs windows_x86.dll from RobustnessFixes.kpatch). Exit 0 = pass.
// Each fix site is the REAL engine byte range (generated from the dump asm) followed/targeted by tiny marker stubs
// (`mov eax,N; ret`). Every scenario runs first with the original bytes (reference = the engine bug) and again after
// LoadLibrary patched the sites, in one process.
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
static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }
static char pad[0x600000];   // must cover 0x40F000 .. 0x990000
static void put(unsigned at, const unsigned char* b, unsigned n) { memcpy((void*)at, b, n); }
static void stubRet(unsigned at, int v) { unsigned char s[6] = { 0xB8, 0, 0, 0, 0, 0xC3 }; memcpy(s + 1, &v, 4); put(at, s, 6); }
static void stubInc(unsigned at, volatile long* ctr, int retN) {   // inc dword [ctr]; ret / ret N
    unsigned char s[9] = { 0xFF, 0x05 }; unsigned a = (unsigned)ctr; memcpy(s + 2, &a, 4);
    if (retN) { s[6] = 0xC2; s[7] = (unsigned char)retN; s[8] = 0; put(at, s, 9); } else { s[6] = 0xC3; put(at, s, 7); }
}
struct Regs { unsigned eax, ecx, edx, ebp, target, out; };
static Regs g_r;
static unsigned run(unsigned target, unsigned eax, unsigned ecx, unsigned edx, unsigned ebp) {
    g_r.eax = eax; g_r.ecx = ecx; g_r.edx = edx; g_r.ebp = ebp; g_r.target = target; g_r.out = 0xDEADBEEF;
    __asm__ __volatile__(
        "pushal\n\t"
        "movl 12(%%esi), %%ebp\n\t"
        "movl 0(%%esi), %%eax\n\t"
        "movl 4(%%esi), %%ecx\n\t"
        "movl 8(%%esi), %%edx\n\t"
        "call *16(%%esi)\n\t"
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
static const char* G[6] = { "kt", "vmid", "sndri", "netovf", "guiid", "abbound" };

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    if ((unsigned)pad > 0x40F960 || (unsigned)pad + sizeof pad < 0x990000) { printf("pad at %p does not cover 0x40F000-0x990000\n", (void*)pad); return 2; }
    DWORD o; if (!VirtualProtect((void*)0x40F000, 0x98F000 - 0x40F000, PAGE_EXECUTE_READWRITE, &o)) { printf("VirtualProtect failed %lu\n", GetLastError()); return 2; }
    g_frame = (unsigned char*)calloc(1, 0x400); g_abThis = (unsigned*)calloc(4, 4);
    // real site bytes + marker stubs
    put(0x668FD9, F_VMID, sizeof F_VMID); stubRet(0x668FE2, 1); { static const unsigned char vmRej[] = { 0xB8, 0x2E, 0xF8, 0xFF, 0xFF, 0xC3 }; put(0x669012, vmRej, sizeof vmRej); }   // real B8 -2002 + ret at 0x669017
    put(0x40F96F, F_GUIID, sizeof F_GUIID); stubRet(0x40F978, 1); stubRet(0x40F9E4, 0);
    put(0x7054CE, F_SNDRI, sizeof F_SNDRI); stubRet(0x7054DA, 2); stubRet(0x70552C, 3); *(void**)0x98657C = (void*)&FakeShutdown;
    put(0x7040EF, F_NETB, sizeof F_NETB); stubRet(0x7040FD, 9); stubRet(0x704415, 5);
    put(0x704406, F_NETA, sizeof F_NETA); stubInc(0x733780, &g_dtors, 0); *(unsigned char*)0x70446D = 0xC3;
    put(0x726527, F_KTA, sizeof F_KTA); *(unsigned char*)0x726531 = 0xC3;
    put(0x726C9C, F_KTB, sizeof F_KTB); *(unsigned char*)0x726CDD = 0xC3;
    put(0x58C60E, F_AB, sizeof F_AB); *(unsigned char*)0x58C63E = 0xC3; stubInc(0x58C700, &g_copies, 4);

    // reference run (original bytes = engine behaviour incl. the bugs)
    check(vmid(5) == 1 && vmid(0x36D) == (unsigned)-2002 && vmid(-1) == 1, "ref vmid: id -1 dispatched (bug)");
    check(guiid(5) == 1 && guiid(-1) == 0 && guiid(5000) == 1, "ref guiid: id 5000 accepted (bug)");
    SndRes s0 = sndri(); check(s0.eax == 3 && s0.d4 == 0 && s0.shut == 0, "ref sndri: driver cleared, startup skipped (bug)");
    check(netb(0x8000, 0x100) == 5 && netb(0x8000, 0x1A000) == 9 && netb(0x8000, 0x20010) == 9, "ref netovf B: lap test");
    NetARes n0 = neta(); check(n0.eax == 5 && n0.dtors == 1, "ref netovf A: overflow branch falls into the memcpy path (bug)");
    KtRes k0 = kt(0x726527, 0x200); check(k0.flags == 0x200 && k0.p14 == 0xF00DF00D && k0.ebpm4 == -1, "ref kt A: CRes+0x14 left dangling (bug)");
    KtRes k1 = kt(0x726C9C, 0x201); check(k1.flags == 0x201 && k1.p14 == 0xF00DF00D, "ref kt B: CRes+0x14 left dangling (bug)");
    check(ab(5, 2) == 5 && ab(2, 5) == 2, "ref ab: shrink copies the OLD count (bug)");

    if (!strcmp(mode, "refuse")) { *(unsigned char*)0x726C9E = 0x0D; *(unsigned char*)0x669014 = 0xF9; }   // kt (2nd site) + vmid (2nd site only; site 1 intact) wrong bytes
    unsigned char snap[9][16];
    const unsigned at[9] = { 0x726527, 0x726C9C, 0x668FE0, 0x7054CE, 0x704406, 0x7040EF, 0x40F972, 0x58C61A, 0x669012 };
    for (int i = 0; i < 9; ++i) memcpy(snap[i], (void*)at[i], 16);
    DeleteFileA("robustnessfixes_log.txt");
    if (!strcmp(mode, "disabled")) { FILE* f = fopen("robustnessfixes.txt", "w"); fprintf(f, "enabled=0\n"); fclose(f); }
    else if (!strcmp(mode, "keys")) { FILE* f = fopen("robustnessfixes.txt", "w"); fprintf(f, "vmid=0\nguiid = 0\nkt=1\n"); fclose(f); }
    else DeleteFileA("robustnessfixes.txt");

    HMODULE m = LoadLibraryA("windows_x86.dll"); if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    StFn st = (StFn)GetProcAddress(m, "RobFixTestState"); CtFn ct = (CtFn)GetProcAddress(m, "RobFixTestCounters"); DmFn dm = (DmFn)GetProcAddress(m, "RobFixTestDummy");
    check(st && ct && dm, "test exports present"); if (!st || !ct || !dm) return 3;
    readLog();
    long c[11], c0[11], c1[11];
    if (!strcmp(mode, "run")) {
        for (int g = 0; g < 6; ++g) { char w[64]; snprintf(w, sizeof w, "state %s = ON", G[g]); check(st(g) == 1, w); }
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
        void* dummy = dm();
        KtRes k2 = kt(0x726527, 0x200); check(k2.flags == 0x200 && k2.p14 == (unsigned)dummy && k2.ebpm4 == -1, "kt A: flag store replayed, CRes+0x14 = dummy, [ebp-4] = -1");
        KtRes k3 = kt(0x726C9C, 0x201); check(k3.flags == 0x201 && k3.p14 == (unsigned)dummy, "kt B: flag store replayed, CRes+0x14 = dummy");
        long cp1 = ab(5, 2), cp2 = ab(2, 5), cp3 = ab(3, 3), cp4 = ab(0, 4);
        check(cp1 == 2 && cp2 == 2 && cp3 == 3 && cp4 == 0, "ab: copy bounded by min(old,new)");
        ct(c); check(c[0] == 1 && c[1] == 1 && c[2] == 1, "counters kt_destroy=1 kt_update=1 ab_clamp=1");
        check(c[3] == 1, "counter sndri_restart == 1");
        check(c[9] == 1 && c[10] == 0x12345, "counters net_drop == 1, net_last_len == 0x12345");
        bool zero = true; for (int i = 0; i < 0x24; ++i) if (((unsigned char*)dummy)[i]) zero = false;
        check(zero, "kt dummy entry still all zero");
        check(strstr(g_log, "6/6 groups patched: kt=ON vmid=ON sndri=ON netovf=ON guiid=ON abbound=ON") != NULL, "log: loaded line");
        Sleep(5600); readLog();   // helper thread polls every 5 s
        check(strstr(g_log, "1.1.0 loaded") != NULL, "log: version 1.1.0");
        check(strstr(g_log, "COUNTERS kt_destroy_orphan=1 kt_update_orphan=1 ab_shrink_clamp=1 sndri_restart=1 vm_reject=") != NULL &&
              strstr(g_log, "net_drop=1 net_last_len=74565") != NULL, "log: COUNTERS line with all 11 values");
        check(strstr(g_log, "FIRED sndri restart=1") && strstr(g_log, "FIRED vmid neg=") && strstr(g_log, "FIRED guiid reject=") &&
              strstr(g_log, "FIRED netovf drop=1 last_len=74565") && strstr(g_log, "FIRED abbound clamp=1") && strstr(g_log, "FIRED kt destroy=1 update=1"), "log: one-shot FIRED lines for every group");
    } else if (!strcmp(mode, "disabled") || !strcmp(mode, "keys")) {
        bool keys = !strcmp(mode, "keys");
        for (int g = 0; g < 6; ++g) {
            bool want = keys && g != 1 && g != 4;
            char w[64]; snprintf(w, sizeof w, "%s: state %s = %s", mode, G[g], want ? "ON" : "off"); check(st(g) == (want ? 1 : 0), w);
        }
        check(memcmp(snap[2], (void*)at[2], 16) == 0 && memcmp(snap[6], (void*)at[6], 16) == 0 && memcmp(snap[8], (void*)at[8], 16) == 0, "vmid (both sites)/guiid sites untouched");
        check(vmid(-1) == 1 && guiid(5000) == 1, "vmid/guiid: original behaviour");
        if (!keys) { bool same = true; for (int i = 0; i < 9; ++i) if (memcmp(snap[i], (void*)at[i], 16)) same = false; check(same, "disabled: every site untouched"); }
        else { check(vmid(-1) == 1 && sndri().eax == 2 && ab(5, 2) == 2, "keys: other groups still patched"); }
        check(strstr(g_log, keys ? "4/6 groups patched" : "0/6 groups patched") != NULL, "log: loaded line");
        DeleteFileA("robustnessfixes.txt");
    } else {
        check(st(0) == -1 && st(1) == -1, "refuse: kt + vmid refused (bytes)");
        check(st(2) == 1 && st(3) == 1 && st(4) == 1 && st(5) == 1, "refuse: other groups still patched");
        check(memcmp(snap[0], (void*)at[0], 16) == 0 && memcmp(snap[1], (void*)at[1], 16) == 0 && memcmp(snap[2], (void*)at[2], 16) == 0, "refuse: kt sites (both) and vmid site 1 untouched");
        check(*(unsigned char*)0x668FE0 == 0x7D && memcmp(snap[8], (void*)at[8], 16) == 0, "refuse: vmid site 1 still 7D, site 2 untouched (group refuses as a whole)");
        check(strstr(g_log, "kt REFUSED: bytes at 0x00726c9c are 89 50 0d eb 3c") != NULL && strstr(g_log, "vmid REFUSED: bytes at 0x00669012 are b8 2e f9 ff ff") != NULL, "log: REFUSED lines with the seen bytes");
        check(strstr(g_log, "4/6 groups patched: kt=REFUSED(bytes) vmid=REFUSED(bytes)") != NULL, "log: loaded line");
    }
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
