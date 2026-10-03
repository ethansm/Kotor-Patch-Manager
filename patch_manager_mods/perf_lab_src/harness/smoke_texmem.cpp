// Smoke test for k2-texture-memory-release 1.0.0 (DLL-only; DllMain rewrites BuildAndStoreTexture's tail 0x4261CF).
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke_texmem.exe smoke_texmem.cpp
//        (.bss pad covers 0x420000-0x430000; check `objdump -h`: .bss must start below 0x4261C0)
// Run:   smoke_texmem.exe <run|disabled|refuse|foreign>   (dir needs windows_x86.dll from TextureMemoryRelease.kpatch). Exit 0 = pass.
// The fragment is the real 0x4261C2..0x4261E5 (vtable+0x30 IsTPCLoaded, site, vtable+0xB4 FreeImage) + RET at the
// epilogue 0x4261E6; fake textures carry a vtable with fastcall stubs (ECX = this) and [tex+0x34] = controller.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static const unsigned char F_TAIL[] = { 0x8B, 0x4D, 0xFC, 0x8B, 0x11, 0x8B, 0x4D, 0xFC, 0x8B, 0x42, 0x30, 0xFF, 0xD0, 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10, 0x8B, 0x55, 0xFC, 0x8B, 0x02, 0x8B, 0x4D, 0xFC, 0x8B, 0x90, 0xB4, 0x00, 0x00, 0x00, 0xFF, 0xD2 };   // 0x4261C2..0x4261E5 from the dump
static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }
static char pad[0x30000];   // must cover 0x4261C0 .. 0x426200
struct Tex { void** vt; int isTpc; int pad1[11]; void* ctrl; int frees; };   // ctrl at +0x34
static int __attribute__((fastcall)) IsTpc(Tex* t) { return t->isTpc ? 0x12345601 : 0x12345600; }   // only AL counts
static void __attribute__((fastcall)) FreeImage(Tex* t) { t->frees++; }
static void* g_vt[0x40];
static unsigned char* g_frame; static unsigned g_ebpv;
static int runTail(Tex* t) {
    *(Tex**)(g_frame + 0x200 - 4) = t; t->frees = 0;
    g_ebpv = (unsigned)g_frame + 0x200;   // global via ESI: an "m" operand on the stack would be read after pushal moved ESP
    __asm__ __volatile__("pushal\n\tmovl (%%esi), %%ebp\n\tmovl $0x004261C2, %%eax\n\tcall *%%eax\n\tpopal\n\t" : : "S"(&g_ebpv) : "memory", "cc");
    return t->frees;
}
static int g_ctrlObj;
static int scenario(int tpc, int ctrl) { static Tex t; memset(&t, 0, sizeof t); t.vt = g_vt; t.isTpc = tpc; t.ctrl = ctrl ? &g_ctrlObj : 0; return runTail(&t); }
static char* g_log;
static void readLog() { if (!g_log) g_log = (char*)malloc(8192); g_log[0] = 0; FILE* f = fopen("texmem_log.txt", "r"); if (!f) return; size_t k = fread(g_log, 1, 8191, f); g_log[k] = 0; fclose(f); }
typedef int (__cdecl *StFn)(); typedef void (__cdecl *CtFn)(long*);
int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    if ((unsigned)pad > 0x4261C0 || (unsigned)pad + sizeof pad < 0x426200) { printf("pad at %p does not cover 0x4261C0\n", (void*)pad); return 2; }
    DWORD o; if (!VirtualProtect((void*)0x426000, 0x1000, PAGE_EXECUTE_READWRITE, &o)) { printf("VirtualProtect failed\n"); return 2; }
    g_frame = (unsigned char*)calloc(1, 0x400);
    g_vt[0x30 / 4] = (void*)&IsTpc; g_vt[0xB4 / 4] = (void*)&FreeImage;
    memcpy((void*)0x4261C2, F_TAIL, sizeof F_TAIL); *(unsigned char*)0x4261E6 = 0xC3;
    check(scenario(0, 0) == 1 && scenario(0, 1) == 1 && scenario(1, 0) == 0 && scenario(1, 1) == 0, "ref: TGA freed, TPC kept (engine)");
    if (!strcmp(mode, "refuse")) *(unsigned char*)0x4261D5 = 0x11;   // JNZ +0x11: wrong bytes
    if (!strcmp(mode, "foreign")) {   // PerfLab tx=1 got there first: its 7 bytes E9 rel32 90 90 (PerfLab.cpp applyTx: rel32 = cave - (site + 5))
        unsigned char* cave = (unsigned char*)VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!cave) { printf("VirtualAlloc failed\n"); return 2; }
        unsigned char jmp[7] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90 };
        int d = (int)((unsigned)cave - (0x4261CF + 5)); memcpy(jmp + 1, &d, 4);
        memcpy((void*)0x4261CF, jmp, 7);
    }
    unsigned char before[7]; memcpy(before, (void*)0x4261CF, 7);
    DeleteFileA("texmem_log.txt");
    if (!strcmp(mode, "disabled")) { FILE* f = fopen("texmem.txt", "w"); fprintf(f, "enabled=0\n"); fclose(f); } else DeleteFileA("texmem.txt");
    HMODULE m = LoadLibraryA("windows_x86.dll"); if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    StFn st = (StFn)GetProcAddress(m, "TexMemTestState"); CtFn ct = (CtFn)GetProcAddress(m, "TexMemTestCounters");
    check(st && ct, "test exports present"); if (!st || !ct) return 3;
    readLog(); long c[2];
    const unsigned char* s = (const unsigned char*)0x4261CF;
    if (!strcmp(mode, "run")) {
        check(st() == 1, "state = patched");
        check(s[0] == 0xE9 && s[5] == 0x90 && s[6] == 0x90, "site = E9 rel32 90 90");
        check(scenario(0, 0) == 1, "TGA without controller: freed (unchanged)");
        check(scenario(0, 1) == 1, "TGA with controller: freed (unchanged)");
        check(scenario(1, 0) == 1, "TPC without controller: FREED (the fix)");
        check(scenario(1, 1) == 0, "TPC with controller: kept (guard)");
        ct(c); check(c[0] == 1 && c[1] == 1, "counters tpc_freed=1 tpc_kept_controller=1");
        check(strstr(g_log, "k2-texture-memory-release 1.0.0 loaded: site 0x004261cf patched") != NULL, "log: patched line");
    } else if (!strcmp(mode, "disabled")) {
        check(st() == 0 && memcmp(s, before, 7) == 0, "disabled: site untouched");
        check(scenario(1, 0) == 0, "disabled: TPC kept (engine)");
        check(strstr(g_log, "DISABLED by texmem.txt") != NULL, "log: disabled line");
        DeleteFileA("texmem.txt");
    } else if (!strcmp(mode, "foreign")) {
        check(before[0] == 0xE9 && before[5] == 0x90 && before[6] == 0x90, "foreign: PerfLab's E9 rel32 90 90 is in place before init");
        check(st() == -1 && memcmp(s, before, 7) == 0, "foreign: PerfLab's jmp present -> refused, site untouched");
        check(strstr(g_log, "REFUSED(bytes e9") != NULL, "log: REFUSED(bytes e9 ...) line");
    } else {
        check(st() == -1 && memcmp(s, before, 7) == 0, "refuse: wrong bytes -> refused, site untouched");
        check(strstr(g_log, "REFUSED(bytes 0f b6 c8 85 c9 75 11") != NULL, "log: REFUSED line");
    }
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
