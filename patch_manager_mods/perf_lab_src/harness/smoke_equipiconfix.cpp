// Smoke test for k2-equip-icon-fix 1.1.0 (DLL-only; DllMain re-targets 8 CALL rel32 sites in/around 0x8AD930).
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke_equipiconfix.exe smoke_equipiconfix.cpp
//        (default base 0x400000; the 7 MB .bss pad covers 0x414000-0xB00000, so 0x414840, 0x8AD930 and every site are
//        ordinary memory of this exe; `objdump -h`: .text must end below 0x414000, pad must start below 0x414840)
// Run:   smoke_equipiconfix.exe <run|disabled|refuse>   (dir needs windows_x86.dll from EquipIconFix.kpatch). Exit 0 = pass.
// Each site holds its real 5-byte CALL followed by RET. A fake setter at 0x414840 (thiscall, RET 8) records per-control
// names; a fake 0x8AD930 replays the engine's call order for a slot scenario. `run` plays the scenario with the original
// bytes (reference), loads the DLL, replays it and requires identical per-frame final names, no more than one name
// change per control per frame, and reloads only on real changes.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }
static char pad[0x700000];
static void unprotect(unsigned addr, unsigned size) { DWORD o; if (!VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &o)) { printf("VirtualProtect %08x failed %lu\n", addr, GetLastError()); exit(2); } }

// ---- fake GUI controls + setter ----
struct Ctl { char name[17]; int changes, calls, changesThisFrame; };
static Ctl g_ctl[11];
static int __thiscall FakeSetImage(Ctl* c, const char* rr, int) {
    ++c->calls;
    if (memcmp(c->name, rr, 16) != 0) { memcpy(c->name, rr, 16); c->name[16] = 0; ++c->changes; ++c->changesThisFrame; }
    return 0;
}
// ---- calling a site: push our return, the args, ECX = this, then JMP to the site (its CALL sees the engine's stack view;
//      the target's RET 8 lands on the site's RET, which returns here) ----
static void viaSiteThis(uintptr_t site, void* ctl, const void* rr, int force) {
    __asm__ __volatile__(
        "pushl $1f\n\t"
        "pushl %%esi\n\t"
        "pushl %%edx\n\t"
        "jmp *%%eax\n\t"
        "1:\n\t"
        : "+a"(site), "+c"(ctl), "+d"(rr), "+S"(force) : : "memory");
}
static void viaSiteFast(uintptr_t site, void* ecx) {
    __asm__ __volatile__(
        "pushl $1f\n\t"
        "jmp *%%eax\n\t"
        "1:\n\t"
        : "+a"(site), "+c"(ecx) : : "edx", "memory");
}
static void rr(char out[16], const char* s) { memset(out, 0, 16); strncpy(out, s, 16); }

// ---- scenario: slot states per frame (empty = "") ----
struct Frame { const char* s0; const char* s1; bool twoHand1; const char* s9; const char* s10; bool twoHand10; };
static const Frame g_frames[] = {
    { "iw_vbroshort_001", "", false, "", "", false },
    { "iw_vbroshort_001", "", false, "", "", false },
    { "iw_vbroshort_001", "", false, "", "", false },
    { "", "", false, "", "", false },                              // unequip slot 0
    { "", "", false, "", "", false },
    { "", "iw_dblsaber_001", true, "", "", false },                // two-handed in slot 1: control 0 shows it greyed
    { "", "iw_dblsaber_001", true, "", "", false },
    { "", "", false, "", "", false },                              // removed: control 0 back to placeholder
    { "", "", false, "", "", false },
    { "", "", false, "", "iw_dblsaber_002", true },                // two-handed in slot 10: control 9 greyed
    { "", "", false, "", "iw_dblsaber_002", true },
    { "", "", false, "", "", false },                              // removed while slot 1 empty: only the top reset restores ctl 9
    { "", "", false, "", "", false },
    { "iw_blaster_001", "", false, "iw_vbroshort_001", "", false }, // both weapon slots equipped
    { "iw_blaster_001", "", false, "iw_vbroshort_001", "", false },
};
constexpr int NF = sizeof(g_frames) / sizeof(g_frames[0]);
static int g_frame = 0;
// Fake 0x8AD930 (fastcall, ECX = panel): the engine's call order for the slots that matter (0, 1, 9, 10).
static void __fastcall FakePanelUpdate(void*) {
    const Frame& f = g_frames[g_frame];
    char p0[16], p9[16], x[16]; rr(p0, "iweap_l"); rr(p9, "iweap_l2");
    viaSiteThis(0x8ADA08, &g_ctl[0], p0, 0);          // top: placeholder reset control 0
    viaSiteThis(0x8ADA81, &g_ctl[9], p9, 0);          // top: placeholder reset control 9
    // k = 0
    if (*f.s0) { rr(x, f.s0); viaSiteThis(0x8AE19A, &g_ctl[0], x, 0); viaSiteThis(0x8AE1CD, &g_ctl[0], x, 0); }
    else viaSiteThis(0x8AE793, &g_ctl[0], p0, 0);    // empty k=0: change path runs every frame (local_24 == local_18 == 0)
    // k = 1
    bool slot1Empty = !*f.s1;
    if (*f.s1) {
        rr(x, f.s1);
        if (f.twoHand1 && !*f.s0) viaSiteThis(0x8ADFA2, &g_ctl[0], x, 0);   // greyed copy in control 0
        viaSiteThis(0x8AE19A, &g_ctl[1], x, 0); viaSiteThis(0x8AE1CD, &g_ctl[1], x, 0);
    }
    // k = 9
    if (*f.s9) { rr(x, f.s9); viaSiteThis(0x8AE19A, &g_ctl[9], x, 0); viaSiteThis(0x8AE1CD, &g_ctl[9], x, 0); }
    else if (!slot1Empty && !f.twoHand1) viaSiteThis(0x8AE793, &g_ctl[9], p9, 0);   // change-path condition at k=9: local_18 == 0 (slot 1 not empty) && local_24 == 0 (no two-handed decrement)
    // k = 10
    if (*f.s10) {
        rr(x, f.s10);
        if (f.twoHand10 && !*f.s9) viaSiteThis(0x8AE02D, &g_ctl[9], x, 0);
        viaSiteThis(0x8AE19A, &g_ctl[10], x, 0); viaSiteThis(0x8AE1CD, &g_ctl[10], x, 0);
    }
}
struct Result { char names[NF][11][17]; int changes[NF][11]; int maxPerFrame; };
static void play(Result& r) {
    memset(g_ctl, 0, sizeof g_ctl); r.maxPerFrame = 0;
    for (g_frame = 0; g_frame < NF; ++g_frame) {
        for (Ctl& c : g_ctl) c.changesThisFrame = 0;
        viaSiteFast(0x8ABA8C, (void*)0x12345678);   // the panel update call in 0x8ABA50
        for (int k = 0; k < 11; ++k) { memcpy(r.names[g_frame][k], g_ctl[k].name, 17); r.changes[g_frame][k] = g_ctl[k].changesThisFrame; if (g_ctl[k].changesThisFrame > r.maxPerFrame) r.maxPerFrame = g_ctl[k].changesThisFrame; }
    }
}
static void putJmp(uintptr_t at, void* to) { unsigned char b[7] = { 0xB8, 0, 0, 0, 0, 0xFF, 0xE0 }; uintptr_t t = (uintptr_t)to; memcpy(b + 1, &t, 4); memcpy((void*)at, b, 7); }
static void putCall(uintptr_t at, uintptr_t target) { unsigned char b[6] = { 0xE8, 0, 0, 0, 0, 0xC3 }; int32_t d = (int32_t)(target - (at + 5)); memcpy(b + 1, &d, 4); memcpy((void*)at, b, 6); }
static char* g_log;   // heap: static buffers in .bss sit in front of pad
static void readLog() { if (!g_log) g_log = (char*)malloc(16384); g_log[0] = 0; FILE* f = fopen("equipiconfix_log.txt", "r"); if (!f) return; size_t k = fread(g_log, 1, 16383, f); g_log[k] = 0; fclose(f); }
static void checkLoadOnce() { int n = 0; if (g_log) for (const char* p = g_log; (p = strstr(p, " 1.1.0 loaded: ")) != nullptr; ++p) ++n; check(n == 1, "log: load line appears exactly once"); }
typedef int (__cdecl *GetSt)(); typedef void (__cdecl *GetCnt)(long*);
int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "run";
    if ((unsigned)pad > 0x414000 || (unsigned)pad + sizeof pad < 0x8AF000) { printf("pad at %p does not cover 0x414000-0x8AF000\n", (void*)pad); return 2; }
    unprotect(0x414000, 0x1000); unprotect(0x8AB000, 0x4000);
    putJmp(0x414840, (void*)&FakeSetImage);
    putJmp(0x8AD930, (void*)&FakePanelUpdate);
    putCall(0x8ABA8C, 0x8AD930);
    const uintptr_t setSites[7] = { 0x8ADA08, 0x8ADA81, 0x8ADFA2, 0x8AE02D, 0x8AE19A, 0x8AE1CD, 0x8AE793 };
    for (uintptr_t s : setSites) putCall(s, 0x414840);
    Result& ref = *(Result*)calloc(1, sizeof(Result)); Result& pat = *(Result*)calloc(1, sizeof(Result));   // heap, see g_log
    play(ref);   // original bytes = engine reference
    int refReloads = 0; for (int f = 0; f < NF; ++f) for (int k = 0; k < 11; ++k) refReloads += ref.changes[f][k];
    printf("reference: %d name changes over %d frames, max %d per control per frame\n", refReloads, NF, ref.maxPerFrame);
    if (!strcmp(mode, "refuse")) *(unsigned char*)(0x8AE1CD + 1) ^= 0x01;   // one site's rel32 wrong
    DeleteFileA("equipiconfix_log.txt");
    if (!strcmp(mode, "disabled")) { FILE* cf = fopen("equipiconfix.txt", "w"); fprintf(cf, "enabled=0\n"); fclose(cf); } else DeleteFileA("equipiconfix.txt");
    unsigned char before[8][5]; const uintptr_t all[8] = { 0x8ABA8C, 0x8ADA08, 0x8ADA81, 0x8ADFA2, 0x8AE02D, 0x8AE19A, 0x8AE1CD, 0x8AE793 };
    for (int i = 0; i < 8; ++i) memcpy(before[i], (void*)all[i], 5);
    HMODULE m = LoadLibraryA("windows_x86.dll"); if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    { typedef void (__cdecl *InitFn)(const void*); InitFn ini = (InitFn)GetProcAddress(m, "KPatchInit"); check(ini != nullptr, "KPatchInit export present"); if (ini) ini(nullptr); }
    GetSt state = (GetSt)GetProcAddress(m, "EquipFixTestState"); GetCnt cnt = (GetCnt)GetProcAddress(m, "EquipFixTestCounters");
    check(state && cnt, "test exports present"); if (!state || !cnt) return 3;
    bool untouched = true; for (int i = 0; i < 8; ++i) untouched &= memcmp(before[i], (void*)all[i], 5) == 0;
    if (!strcmp(mode, "run")) {
        check(state() == 1, "state = patched");
        bool rewired = true; for (int i = 0; i < 8; ++i) { const unsigned char* p = (const unsigned char*)all[i]; int32_t d; memcpy(&d, p + 1, 4);
            MEMORY_BASIC_INFORMATION mb; VirtualQuery((void*)(all[i] + 5 + d), &mb, sizeof mb); rewired &= p[0] == 0xE8 && mb.AllocationBase == (void*)m && p[5] == 0xC3; }
        check(rewired, "all 8 sites re-targeted into the patch DLL, next bytes untouched");
        play(pat);
        bool same = true; for (int f = 0; f < NF; ++f) for (int k = 0; k < 11; ++k) if (strcmp(ref.names[f][k], pat.names[f][k])) { same = false; printf("  frame %d ctl %d: ref '%s' patched '%s'\n", f, k, ref.names[f][k], pat.names[f][k]); }
        check(same, "per-frame final names identical to the engine (15 frames x 11 controls)");
        check(pat.maxPerFrame <= 1, "at most one name change per control per frame");
        int patReloads = 0, transitions = 0;
        for (int f = 0; f < NF; ++f) for (int k = 0; k < 11; ++k) { patReloads += pat.changes[f][k]; if (strcmp(ref.names[f][k], f ? ref.names[f - 1][k] : "")) ++transitions; }
        printf("patched: %d name changes (real changes of the final state: %d), engine: %d\n", patReloads, transitions, refReloads);
        check(patReloads == transitions && patReloads < refReloads, "patched reloads only on real changes");
        // steady state with a weapon equipped: frames 1-2 (and 14) cause no change at all
        check(pat.changes[1][0] == 0 && pat.changes[2][0] == 0 && pat.changes[14][0] == 0 && pat.changes[14][9] == 0, "steady state with weapons equipped: zero reloads");
        long c[7]; cnt(c);
        printf("counters passes=%ld deferred=%ld cancelled=%ld applied=%ld sets=%ld outside=%ld replaced=%ld\n", c[0], c[1], c[2], c[3], c[4], c[5], c[6]);
        check(c[0] == NF && c[1] >= 2 * NF && c[2] + c[3] + c[6] == c[1] && c[5] == 0, "counters: one pass per frame, >= 2 deferred per pass, each cancelled, replaced or applied");
        Sleep(10500); readLog(); checkLoadOnce();
        check(strstr(g_log, "k2-equip-icon-fix 1.1.0 loaded: 8 sites patched") != nullptr, "log: patched line");
        check(strstr(g_log, "CANCEL ctl=") != nullptr && strstr(g_log, "deferred 'iweap_l' superseded by 'iw_vbroshort_001'") != nullptr, "log: CANCEL event names the alternation");
        check(strstr(g_log, "STATS passes=15") != nullptr, "log: STATS line");
    } else if (!strcmp(mode, "disabled")) {
        check(state() == 0 && untouched, "disabled: all sites untouched");
        play(pat); int r = 0; for (int f = 0; f < NF; ++f) for (int k = 0; k < 11; ++k) r += pat.changes[f][k];
        check(r == refReloads, "disabled: engine behaviour (same reload count)");
        readLog(); checkLoadOnce(); check(strstr(g_log, "DISABLED by equipiconfix.txt") != nullptr, "log: disabled line");
        DeleteFileA("equipiconfix.txt");
    } else {
        check(state() == -1 && untouched, "refuse: one wrong site -> refused, all 8 sites untouched");
        readLog(); checkLoadOnce(); check(strstr(g_log, "REFUSED(bytes") != nullptr && strstr(g_log, "at 0x008ae1cd") != nullptr, "log: REFUSED(bytes ...) names the site");
    }
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
