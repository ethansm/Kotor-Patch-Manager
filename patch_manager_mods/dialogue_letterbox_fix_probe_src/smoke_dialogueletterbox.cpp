// Smoke test for k2-dialogue-letterbox-fix 1.0.1-probe (DLL-only; exercises the three detour handlers directly, no game).
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -o smoke.exe smoke_dialogueletterbox.cpp
//        (default base 0x400000; the 7 MB .bss pad makes the image cover ~0x40D000-0xB0D000 so the .rdata constant VA
//        0x009A7D30 is ordinary memory of this exe; the harness verifies that with VirtualQuery and aborts otherwise)
// Run:   smoke.exe <h6|clamp|vanilla|disabled|foreign|unchanged|late|ini|log0|probe|probe0|probelog0>
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
typedef void (__cdecl *Hook2)(void*, int);
static Hook pOnSize, pSetTop, pSetBottom, pW1, pW2, pW3, pW5;
static Hook2 pW4;
static uint8_t* g_mgr;          // heap, 0x100 bytes
static uint8_t* g_frame;        // heap, 0x80 bytes
static uint8_t* g_lb;           // heap, 0x100 bytes
static char* g_log;

static void readLog() {
    if (!g_log) g_log = (char*)malloc(1 << 23);
    g_log[0] = 0;
    FILE* f = fopen("dialogue_letterbox_fix_log.txt", "r");
    if (!f) return;
    size_t k = fread(g_log, 1, (1 << 23) - 1, f);
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


// ---- probe modes: fake objects live at game VAs inside the .bss pad (0xA20000.. / DAT_00a309b4..bc at 0xA309B4..) ----
static uint8_t* const kLb = (uint8_t*)0x00A20000;      // fake CSWGuiDialogLetterbox
static uint8_t* const kMgr = (uint8_t*)0x00A20400;     // fake GUI manager
static uint8_t* const kOther = (uint8_t*)0x00A20800;   // fake non-letterbox panel (other vtable)
static uint8_t* const kFrameA = (uint8_t*)0x00A20C00 + 0x100;   // ebp for W1/W3/W4/W5
static uint8_t* const kFrameB = (uint8_t*)0x00A21000 + 0x100;   // ebp for W2
static const uint32_t kVtbl = 0x009A7CBC;

static int pcount() { return logCount("] PROBE W"); }
static void setRect(uint8_t* base, int a, int b, int c, int d) { int32_t* r = (int32_t*)base; r[0] = a; r[1] = b; r[2] = c; r[3] = d; }
static void setLb(int mode, int done, int x, int y, int w, int h, uint32_t flags) {
    *(uint32_t*)(kLb + 0) = kVtbl; *(void**)(kLb + 0x1C) = kMgr;
    setRect(kLb + 0x04, x, y, w, h); *(uint32_t*)(kLb + 0x48) = flags; *(uint32_t*)(kLb + 0x50) = 0x3F000000;
    *(int32_t*)(kLb + 0x88) = mode; *(int32_t*)(kLb + 0x8C) = done; *(int32_t*)(kLb + 0x74) = 267;
}
static void setFrameA(int px, int py, int pw, int ph) {      // W1 rect at ebp-0x18, W4 rect at ebp-0x10 (we reuse by re-setting)
    *(void**)(kFrameA - 0x34) = kLb; *(void**)(kFrameA - 0x38) = kLb; *(void**)(kFrameA - 0x24) = kLb;
    setRect(kFrameA - 0x18, px, py, pw, ph); *(uint32_t*)(kFrameA + 8) = 0x3C888889;
}
static void setFrameW4(uint8_t* thisp, int x, int y, int w, int h) {
    *(void**)(kFrameA - 0x38) = thisp; setRect(kFrameA - 0x10, x, y, w, h);
}

static void runProbe(const char* mode) {
    const bool on = !strcmp(mode, "probe");
    const bool log0 = !strcmp(mode, "probelog0");
    if ((uintptr_t)pad > 0xA20000 || (uintptr_t)pad + sizeof pad < 0xA31000) { check(false, "pad covers 0xA20000-0xA31000"); return; }
    *(int16_t*)(kMgr + 0x6C) = 3840; *(int16_t*)(kMgr + 0x6E) = 1600;
    *(uint32_t*)(kOther + 0) = 0x009A7000;
    *(int32_t*)0xA309B4 = 267; *(int32_t*)0xA309B8 = 1333; *(uint8_t*)0xA309BC = 1;
    check(logHas(on || log0 ? "probe=1 const@" : "probe=0 const@"), on || log0 ? "load line shows probe=1" : "load line shows probe=0 (explicit INI)");
    // bare "enabled=1 mode=h6 log=1" prefix of the 1.0.0 load line is unchanged
    check(logHas(log0 ? "enabled=1 mode=h6 log=0 probe=" : "enabled=1 mode=h6 log=1 probe="), "load line keeps the 1.0.0 prefix, probe= appended");

    // ---- one record per window, every field ----
    setLb(1, 0, 0, 0, 3840, 267, 0x44);                    // mode 1 (top), bits 2 and 6 set
    setFrameA(0, 0, 3840, 8); pW1(kFrameA);
    setRect(kFrameB - 0x28, 0, 1333, 3840, 267); *(void**)(kFrameB - 0x34) = kLb; *(int32_t*)(kLb + 0x88) = 2; pW2(kFrameB);
    *(int32_t*)(kLb + 0x88) = 1;
    pW3(kFrameA);
    setFrameW4(kLb, 0, 0, 3840, 267); pW4(kFrameA, 1);
    *(int32_t*)(kFrameA + 8) = 1; pW5(kFrameA);
    *(int32_t*)(kFrameA + 8) = 0; pW5(kFrameA);
    setFrameA(0, 0, 3840, 8);                               // restore the dt slot
    if (!on) {
        // probe=0 / log=0: every handler returned at once and wrote nothing
        check(pcount() == 0 && logCount("PROBE") == 0, on ? "" : (log0 ? "probe=1 log=0: no PROBE line written" : "probe=0: no PROBE line written by W1..W5"));
        pW4(nullptr, 0); pW1(nullptr); pW2(nullptr); pW3(nullptr); pW5(nullptr);
        pSetTop(kLb);                                       // the 1.0.0 SETTOP hook still works with probe=0
        if (!log0) check(logHas("SETTOP lb="), "probe=0: the 1.0.0 SETTOP line is still written");
        // garbage frame with probe off: returns before reading anything
        pW4((void*)0x10, 1); pW1((void*)0x10);
        check(pcount() == 0, "probe off: garbage ebp is never dereferenced, still nothing logged");
        if (!log0) {
            readLog();
            check(strstr(g_log, "PROBE") == nullptr, "probe=0: log is 1.0.0-identical (no PROBE text anywhere)");
        }
        printf("%s: %d failures\n", mode, g_fail);
        return;
    }
    check(logHas("PROBE W1 n=1 skip=0 this=0x00a20000 mode=1 done=0 alpha=0x3f000000 passedRect(x,y,w,h)=(0,0,3840,8) ctrlRect(x,y,w,h)=(0,0,3840,267) targetH(+0x74)=267"), "W1 line: mode/done/alpha/passed rect/control rect/target h");
    check(logHas("PROBE W2 n=1 skip=0 this=0x00a20000 mode=2 done=0 alpha=0x3f000000 passedRect(x,y,w,h)=(0,1333,3840,267)"), "W2 line: mode 2, passed rect (0,1333,3840,267)");
    check(logHas("PROBE W3 n=1 skip=0 this=0x00a20000 vtbl=0x009a7cbc(letterbox) flags(+0x48)=0x00000044 bit2=1 bit6=1 rect(x,y,w,h)=(0,0,3840,267) alpha=0x3f000000 color(+0x54..)=0x00000000,0x00000000,0x00000000")
          && logHas("W=3840 H=1600 predicate=WOULD-DRAW dt=0x3c888889 DAT_a309b4=267 DAT_a309b8=1333 DAT_a309bc=1"), "W3 line: vtbl, flag bits, rect, alpha, colour, W/H, WOULD-DRAW, dt bits, DAT_a309b4/b8/bc");
    check(logHas("PROBE W4 n=1 skip=0 this=0x00a20000 mode=1 eax=1 fillRect(x,y,w,h)=(0,0,3840,267) alpha=0x3f000000"), "W4 line: eax=1 and the fill rect");
    check(logHas("PROBE W5 n=1 skip=0 arg=1 this=0x00a20000") && logHas("PROBE W5 n=2 skip=0 arg=0 this=0x00a20000"), "W5 lines: arg 1 then arg 0");
    check(logCount("UNREADABLE") == 0, "no UNREADABLE marker on valid objects");

    // ---- W3 predicate: each failing term in V8b order ----
    struct P { const char* name; int x, y, w, h; uint32_t flags; };
    const P cases[] = { { "FAIL:bit6", 0, 0, 3840, 267, 0x04 }, { "FAIL:x>=0", -1, 0, 3840, 267, 0x44 }, { "FAIL:y>=0", 0, -22, 3840, 267, 0x44 },
                        { "FAIL:w>0", 0, 0, 0, 267, 0x44 }, { "FAIL:h>0", 0, 0, 3840, 0, 0x44 }, { "FAIL:x+w<=W", 10, 0, 3840, 267, 0x44 },
                        { "FAIL:y+h<=H", 0, 1400, 3840, 267, 0x44 } };
    for (const P& c : cases) {
        setLb(1, 0, c.x, c.y, c.w, c.h, c.flags);
        pW3(kFrameA);
        char want[80]; snprintf(want, sizeof want, "predicate=%s dt=", c.name);
        check(logHas(want), want);
    }
    setLb(1, 0, 0, 0, 3840, 267, 0x44);

    // ---- W4 vtable filter / guarded reads ----
    int before = pcount();
    setFrameW4(kOther, 0, 0, 100, 100); pW4(kFrameA, 1);
    check(pcount() == before, "W4 with another vtable logs nothing");
    setFrameW4((uint8_t*)0x1000, 0, 0, 100, 100); pW4(kFrameA, 1);       // unmapped this: no crash, no line
    setFrameW4(nullptr, 0, 0, 100, 100); pW4(kFrameA, 1); pW4(nullptr, 1);
    check(pcount() == before, "W4 with unreadable / null this or null ebp: no crash, no line");
    setFrameW4(kLb, 0, 0, 3840, 267);
    *(void**)(kFrameB - 0x34) = (void*)0x1000; pW2(kFrameB);              // W2 with an unreadable this: logged with the marker
    check(logHas("this=0x00001000") && logHas("[UNREADABLE]"), "W2 with an unreadable this: line carries [UNREADABLE], no crash");
    *(void**)(kFrameB - 0x34) = kLb;
    pW1(nullptr); pW2(nullptr); pW3(nullptr); pW5(nullptr);

    if (log0) {
        printf("%s: %d failures\n", mode, g_fail);
        return;
    }

    // ---- rate limit: first 120 per window every frame, then 1/s; SETTOP resets ----
    int w2Before = logCount("] PROBE W2 ");
    DWORD t0 = GetTickCount();
    for (int i = 0; i < 300; ++i) pW2(kFrameB);
    DWORD dt = GetTickCount() - t0;
    int w2After = logCount("] PROBE W2 ");
    printf("300 W2 calls took %lu ms, W2 lines %d -> %d\n", dt, w2Before, w2After);
    if (dt < 900) check(w2After == 120, "W2: exactly 120 lines for 300 rapid calls");
    else check(w2After >= 120, "W2: >= 120 lines (slow machine, 1/s tail may have added some)");
    Sleep(1100);
    pW2(kFrameB);
    check(logCount("] PROBE W2 ") == w2After + 1 && logHas("PROBE W2 n=121 skip=") , "W2: after 1.1 s one more line (n=121) with skip= count");
    pSetTop(kLb);                                            // new dialogue: counters restart
    pW2(kFrameB);
    check(logHas("PROBE W2 n=1 skip=0 this=") && logCount("] PROBE W2 n=1 ") >= 2, "SETTOP resets the per-window counters (W2 n=1 logged again)");

    // ---- global cap 3000 ----
    for (int round = 0; round < 12; ++round) {
        pSetTop(kLb);
        for (int i = 0; i < 125; ++i) { pW1(kFrameA); pW2(kFrameB); pW3(kFrameA); pW4(kFrameA, 1); pW5(kFrameA); }
    }
    int total = pcount();
    printf("PROBE lines after cap test: %d\n", total);
    check(total == 3000, "global cap: exactly 3000 PROBE lines written");
    check(logCount("PROBE cap (3000 lines) reached") == 1, "global cap: one cap notice line");
    check(logHas("SETTOP lb=") && logHas("loaded:"), "1.0.0 lines coexist with the PROBE lines");
    printf("%s: %d failures\n", mode, g_fail);
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
    else if (!strcmp(mode, "probe")) writeIni("probe=1\r\n");
    else if (!strcmp(mode, "probe0")) writeIni("probe = 0\r\n");
    else if (!strcmp(mode, "probelog0")) writeIni("probe=1\nlog=0\n");
    // h6 / foreign / unchanged / late: no INI = defaults

    HMODULE m = LoadLibraryA("windows_x86.dll");
    if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    pOnSize = (Hook)GetProcAddress(m, "dlbOnScreenSize");
    pSetTop = (Hook)GetProcAddress(m, "dlbLogSetTop");
    pSetBottom = (Hook)GetProcAddress(m, "dlbLogSetBottom");
    check(pOnSize && pSetTop && pSetBottom, "exports dlbOnScreenSize / dlbLogSetTop / dlbLogSetBottom present");
    if (!pOnSize || !pSetTop || !pSetBottom) return 3;
    pW1 = (Hook)GetProcAddress(m, "dlbProbeW1"); pW2 = (Hook)GetProcAddress(m, "dlbProbeW2"); pW3 = (Hook)GetProcAddress(m, "dlbProbeW3");
    pW4 = (Hook2)GetProcAddress(m, "dlbProbeW4"); pW5 = (Hook)GetProcAddress(m, "dlbProbeW5");
    check(pW1 && pW2 && pW3 && pW4 && pW5, "exports dlbProbeW1..W5 present");
    if (!(pW1 && pW2 && pW3 && pW4 && pW5)) return 3;
    if (!strcmp(mode, "foreign"))
        check(logHas("k2-dialogue-letterbox-fix 1.0.1-probe loaded:") && logHas("00 00 00 00 00 00 00 40"), "log: load line with version and the pre-existing 2.0 bytes");
    else
        check(logHas("k2-dialogue-letterbox-fix 1.0.1-probe loaded:") && logHas("2.3333330154418945") && logHas("00 00 00 80 aa aa 02 40"), "log: load line with version and vanilla bytes/value");

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
    } else if (!strcmp(mode, "probe") || !strcmp(mode, "probe0") || !strcmp(mode, "probelog0")) {
        runProbe(mode);
        return g_fail ? 1 : 0;
    } else {
        printf("unknown mode %s\n", mode);
        return 4;
    }
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
