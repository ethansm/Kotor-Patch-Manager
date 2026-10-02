// Smoke test for k2-dialogue-letterbox-fix 1.0.0 (DLL-only; exercises the three detour handlers directly, no game).
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke.exe smoke_dialogueletterbox.cpp
//        (default base 0x400000; the 7 MB .bss pad makes the image cover ~0x40D000-0xB0D000 so the .rdata constant VA
//        0x009A7D30 is ordinary memory of this exe; the harness verifies that with VirtualQuery and aborts otherwise)
// Run:   smoke.exe <h6|clamp|vanilla|disabled|foreign|unchanged|late|ini|log0>
//        in a dir containing windows_x86.dll (the DLL from K2DialogueLetterboxFix.kpatch). Exit 0 = all checks passed.
// The harness writes dialogue_letterbox_fix.ini before LoadLibrary and deletes the old log. dlbOnScreenSize is given a
// fake frame (ebp = frame + 0x40, [ebp-0xC] = &fakeManager) and the fake manager's W/H shorts at +0x6C/+0x6E.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }

static char pad[0x700000];   // covers 0x9A7000 .. 0x9A8000 (checked at runtime)
static const unsigned kConst = 0x009A7D30;
static const uint8_t kVanilla[8] = { 0x00, 0x00, 0x00, 0x80, 0xAA, 0xAA, 0x02, 0x40 };

typedef void (__cdecl *Hook)(void*);
static Hook pOnSize, pSetTop, pSetBottom;
static uint8_t* g_mgr;          // heap, 0x100 bytes
static uint8_t* g_frame;        // heap, 0x80 bytes
static uint8_t* g_lb;           // heap, 0x100 bytes
static char* g_log;

static void readLog() {
    if (!g_log) g_log = (char*)malloc(1 << 20);
    g_log[0] = 0;
    FILE* f = fopen("dialogue_letterbox_fix_log.txt", "r");
    if (!f) return;
    size_t k = fread(g_log, 1, (1 << 20) - 1, f);
    g_log[k] = 0;
    fclose(f);
}
static bool logHas(const char* s) { readLog(); return strstr(g_log, s) != nullptr; }
static int logCount(const char* s) { readLog(); int n = 0; for (const char* p = g_log; (p = strstr(p, s)) != nullptr; ++p) ++n; return n; }

static double curC() { return *(volatile double*)kConst; }
static bool isVanilla() { return memcmp((void*)kConst, kVanilla, 8) == 0; }
static int xFor(int w, double c) { return (int)((double)w / c); }
static int barFor(int w, int h, double c) { return (int)((double)(h - xFor(w, c)) / 2.0); }
static DWORD protNow() { MEMORY_BASIC_INFORMATION mb; VirtualQuery((void*)kConst, &mb, sizeof mb); return mb.Protect; }

static void setSize(int w, int h) { *(int16_t*)(g_mgr + 0x6C) = (int16_t)w; *(int16_t*)(g_mgr + 0x6E) = (int16_t)h; }
static void callSize(int w, int h) { setSize(w, h); pOnSize(g_frame + 0x40); }

static void writeIni(const char* text) { FILE* f = fopen("dialogue_letterbox_fix.ini", "wb"); if (f) { fputs(text, f); fclose(f); } }

static void setConst(const uint8_t* b) {
    DWORD o;
    VirtualProtect((void*)0x9A7000, 0x1000, PAGE_READWRITE, &o);
    memcpy((void*)kConst, b, 8);
    VirtualProtect((void*)0x9A7000, 0x1000, PAGE_READONLY, &o);
}

struct Res { int w, h, bar; };
static const Res kRes[] = { { 3840, 1600, 267 }, { 1920, 1080, 180 }, { 3440, 1440, 240 }, { 5120, 1440, 240 }, { 800, 600, 100 }, { 640, 480, 80 } };

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "h6";
    MEMORY_BASIC_INFORMATION mb;
    VirtualQuery((void*)0x9A7000, &mb, sizeof mb);
    printf("pad at %p size %#x; 0x9A7000 region base %p size %#lx state %#lx\n", (void*)pad, (unsigned)sizeof pad, mb.BaseAddress, mb.RegionSize, mb.State);
    if ((unsigned)pad > 0x9A7000 || (unsigned)pad + sizeof pad < 0x9A8000 || mb.State != MEM_COMMIT) { printf("pad does not cover 0x9A7000-0x9A8000, abort\n"); return 2; }

    g_mgr = (uint8_t*)calloc(1, 0x100);
    g_frame = (uint8_t*)calloc(1, 0x80);
    g_lb = (uint8_t*)calloc(1, 0x100);
    *(void**)(g_frame + 0x40 - 0xC) = g_mgr;
    *(void**)(g_lb + 0x1C) = g_mgr;

    // constant: vanilla (or foreign), page read-only like .rdata
    DWORD o;
    VirtualProtect((void*)0x9A7000, 0x1000, PAGE_READWRITE, &o);
    memcpy((void*)kConst, kVanilla, 8);
    if (!strcmp(mode, "foreign")) { double d = 2.0; memcpy((void*)kConst, &d, 8); }
    VirtualProtect((void*)0x9A7000, 0x1000, PAGE_READONLY, &o);
    uint8_t startBytes[8]; memcpy(startBytes, (void*)kConst, 8);

    DeleteFileA("dialogue_letterbox_fix_log.txt");
    DeleteFileA("dialogue_letterbox_fix.ini");
    if (!strcmp(mode, "clamp")) writeIni("mode=clamp\n");
    else if (!strcmp(mode, "vanilla")) writeIni("mode=vanilla\n");
    else if (!strcmp(mode, "disabled")) writeIni("enabled=0\n");
    else if (!strcmp(mode, "ini")) writeIni("; comment line\r\n# another comment\r\n[section]\r\n   enabled  =  1  \r\n  MODE =  bogus \r\nlog=1\r\n");
    else if (!strcmp(mode, "log0")) writeIni("log = 0\r\nmode=h6\r\n");
    // h6 / foreign / unchanged / late: no INI = defaults

    HMODULE m = LoadLibraryA("windows_x86.dll");
    if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    pOnSize = (Hook)GetProcAddress(m, "dlbOnScreenSize");
    pSetTop = (Hook)GetProcAddress(m, "dlbLogSetTop");
    pSetBottom = (Hook)GetProcAddress(m, "dlbLogSetBottom");
    check(pOnSize && pSetTop && pSetBottom, "exports dlbOnScreenSize / dlbLogSetTop / dlbLogSetBottom present");
    if (!pOnSize || !pSetTop || !pSetBottom) return 3;
    if (!strcmp(mode, "foreign"))
        check(logHas("k2-dialogue-letterbox-fix 1.0.0 loaded:") && logHas("00 00 00 00 00 00 00 40"), "log: load line with version and the pre-existing 2.0 bytes");
    else
        check(logHas("k2-dialogue-letterbox-fix 1.0.0 loaded:") && logHas("2.3333330154418945") && logHas("00 00 00 80 aa aa 02 40"), "log: load line with version and vanilla bytes/value");

    if (!strcmp(mode, "h6")) {
        check(logHas("enabled=1 mode=h6 log=1"), "log: load line shows enabled=1 mode=h6 log=1");
        for (const Res& r : kRes) {
            callSize(r.w, r.h);
            double c = curC();
            int bar = barFor(r.w, r.h, c);
            DWORD pr = protNow();
            char what[200]; snprintf(what, sizeof what, "h6 %dx%d: c=%.17g bar=%d (expect %d), protection after call %#lx (expect READONLY %#x)", r.w, r.h, c, bar, r.bar, pr, PAGE_READONLY);
            check(bar == r.bar && pr == PAGE_READONLY, what);
        }
        callSize(3840, 1600);
        double c = curC();
        check(c < 3.6 && c > 3.6 * (1.0 - 1e-8), "h6 3840x1600: c is 1.5*W/H nudged just below (3.6)");
        // invalid size: W = 0 -> unchanged, INVALID logged
        uint8_t before[8]; memcpy(before, (void*)kConst, 8);
        callSize(0, 1600);
        check(memcmp(before, (void*)kConst, 8) == 0, "W=0: constant unchanged");
        callSize(1920, 0);
        check(memcmp(before, (void*)kConst, 8) == 0, "H=0: constant unchanged");
        check(logCount("INVALID size, no write") == 2, "log: two INVALID lines");
        // null ebp: no crash, no change
        pOnSize(nullptr);
        check(memcmp(before, (void*)kConst, 8) == 0, "null ebp: no crash, constant unchanged");
        // null manager: logged and skipped
        void* saved = *(void**)(g_frame + 0x40 - 0xC);
        *(void**)(g_frame + 0x40 - 0xC) = nullptr;
        pOnSize(g_frame + 0x40);
        *(void**)(g_frame + 0x40 - 0xC) = saved;
        check(memcmp(before, (void*)kConst, 8) == 0 && logHas("manager pointer [ebp-0xC] is null"), "null manager: skipped and logged");
        // letterbox log hooks
        setSize(3840, 1600);
        int32_t* start = (int32_t*)(g_lb + 0x04); start[0] = 0; start[1] = 0; start[2] = 3840; start[3] = 0;
        int32_t* targ = (int32_t*)(g_lb + 0x68); targ[0] = 0; targ[1] = 0; targ[2] = 3840; targ[3] = 267;
        *(int32_t*)(g_lb + 0x88) = 1;
        pSetTop(g_lb);
        start[1] = 1600; start[3] = 267; targ[1] = 1333; *(int32_t*)(g_lb + 0x88) = 2;
        pSetBottom(g_lb);
        pSetTop(nullptr); pSetBottom(nullptr);
        check(logHas("SETTOP lb=") && logHas("target(x,y,w,h)=(0,0,3840,267)") && logHas("W=3840 H=1600"), "log: SETTOP line with target rect (0,0,3840,267) and W/H");
        check(logHas("SETBOTTOM lb=") && logHas("target(x,y,w,h)=(0,1333,3840,267)") && logHas("mode(+0x88, before this store)=2"), "log: SETBOTTOM line with target rect (0,1333,3840,267) and mode 2");
        check(logCount("LATE(") == 0, "no LATE fallback after a SetScreenSize hit");
        // restore vanilla size sequence (credits 640x480 then back) still follows
        callSize(640, 480); callSize(3840, 1600);
        check(barFor(3840, 1600, curC()) == 267, "credits-style 640x480 then back to 3840x1600: bar 267 again");
        check(logHas("vanillaBar=-22 newBar=267 X=1066 fs=2.0 panelH=200 replyY=1333"), "log: 3840x1600 numbers (vanillaBar -22, newBar 267, X 1066, fs 2.0, reply y 1333)");
    } else if (!strcmp(mode, "clamp")) {
        callSize(3840, 1600);
        check(barFor(3840, 1600, curC()) == 0, "clamp 3840x1600: bar 0");
        check(protNow() == PAGE_READONLY, "clamp: page protection restored");
        callSize(1920, 1080);
        check(isVanilla() && barFor(1920, 1080, curC()) == 129, "clamp 1920x1080: vanilla bytes restored, bar 129");
        check(protNow() == PAGE_READONLY, "clamp: page protection restored after restoring vanilla");
        callSize(3440, 1440);
        check(barFor(3440, 1440, curC()) == 0, "clamp 3440x1440: bar 0");
    } else if (!strcmp(mode, "vanilla")) {
        callSize(3840, 1600);
        check(isVanilla(), "vanilla 3840x1600: bytes stay vanilla");
        check(barFor(3840, 1600, curC()) == -22, "vanilla 3840x1600: bar -22");
        check(logHas("unchanged"), "log: unchanged");
    } else if (!strcmp(mode, "disabled")) {
        callSize(3840, 1600);
        check(isVanilla() && memcmp(startBytes, (void*)kConst, 8) == 0, "disabled: bytes stay vanilla");
        check(logHas("DISABLED, no write"), "log: DISABLED, no write");
        check(logHas("enabled=0"), "log: load line shows enabled=0");
    } else if (!strcmp(mode, "foreign")) {
        callSize(3840, 1600);
        check(memcmp(startBytes, (void*)kConst, 8) == 0 && curC() == 2.0, "foreign: bytes unchanged (2.0)");
        check(logHas("FOREIGN value") && logHas("another mod owns 0x009a7d30, not writing"), "log: FOREIGN line");
        callSize(1920, 1080);
        check(curC() == 2.0 && logCount("FOREIGN value") == 2, "foreign: still refuses on a later hit");
        // another mod restoring vanilla makes it writable again
        setConst(kVanilla);
        callSize(3840, 1600);
        check(barFor(3840, 1600, curC()) == 267, "foreign: after the other owner restored vanilla, the fix applies (bar 267)");
    } else if (!strcmp(mode, "unchanged")) {
        callSize(3840, 1600);
        double c1 = curC();
        callSize(3840, 1600);
        check(curC() == c1 && barFor(3840, 1600, c1) == 267, "unchanged: value stable across two hits, bar 267");
        check(logCount("wrote (protect RW ok") == 1, "log: exactly one write");
        check(logCount(": unchanged (already") == 1, "log: second hit logs unchanged");
        // our own earlier value is accepted as writable for a new size
        callSize(1920, 1080);
        check(barFor(1920, 1080, curC()) == 180 && logCount("FOREIGN") == 0, "own previous value accepted, new size applied (bar 180), no FOREIGN");
    } else if (!strcmp(mode, "late")) {
        setSize(3840, 1600);
        pSetTop(g_lb);
        check(barFor(3840, 1600, curC()) == 267 && logHas("LATE(SetTop)"), "late: SetTop without a prior SetScreenSize hit applies the fix (LATE(SetTop))");
        pSetBottom(g_lb);
        check(logCount("APPLY src=LATE(SetTop) W=") == 1 && logCount("APPLY src=LATE(SetBottom)") == 0, "late: fallback runs once");
    } else if (!strcmp(mode, "ini")) {
        check(logHas("unknown mode 'bogus'"), "ini (CRLF, comments, spaces, [section]): unknown mode logged");
        check(logHas("enabled=1 mode=h6 log=1"), "ini: unknown mode falls back to h6, enabled/log parsed with spaces");
        callSize(3840, 1600);
        check(barFor(3840, 1600, curC()) == 267, "ini: h6 behaviour, bar 267");
    } else if (!strcmp(mode, "log0")) {
        check(logHas("enabled=1 mode=h6 log=0"), "log0: load line still written (shows log=0)");
        callSize(3840, 1600);
        check(barFor(3840, 1600, curC()) == 267, "log0: fix still applied, bar 267");
        pSetTop(g_lb);
        check(!logHas("APPLY") && !logHas("SETTOP"), "log0: no APPLY/SETTOP lines");
        callSize(0, 0);
        check(logHas("INVALID size"), "log0: problem lines (INVALID) still written");
    } else {
        printf("unknown mode %s\n", mode);
        return 4;
    }
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
