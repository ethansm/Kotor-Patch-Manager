// Smoke test for k2-dialogue-letterbox-fix 1.0.2 and 1.0.2-probe (DLL-only; exercises the detour handlers directly, no game).
// ONE harness for both variants: it detects the probe build by GetProcAddress("dlbProbeW1"). Probe-only modes print SKIP and
// exit 0 on the shipping DLL.
// Build: i686-w64-mingw32-g++ -O1 -static-libgcc -static-libstdc++ -I<kpm>/Patches/Common -o smoke.exe smoke_dialogueletterbox.cpp
//        (-I is only for kpatch_api.h, used by the core mode's stub KPatchApi; default image base 0x400000; the 7 MB .bss pad
//        makes the image cover ~0x40D000-0xB0D000 so the .rdata constant VA 0x009A7D30 is ordinary memory of this exe; the
//        harness verifies that with VirtualQuery and aborts otherwise)
// Run:   smoke.exe <mode>   in a dir containing windows_x86.dll (the DLL from the .kpatch). Exit 0 = all checks passed.
//   shipping modes (both DLLs): h6 clamp vanilla disabled foreign unchanged late ini log0 default core
//   probe-only modes:           probe probe0 probelog0          (SKIP on the shipping DLL)
// 1.0.2 logging: KPatchInit is never called except in mode `core`, so every other mode runs the KpLog.h FALLBACK sink: the file
// dialogue_letterbox_fix_log.txt with lines "<ms> k2-dialogue-letterbox-fix/<channel> [WARN |ERR ]text" (CRLF on Windows). The load
// line is no longer written from DllMain: the first hook call emits it (the harness triggers it with pSetTop(nullptr), which has
// no other effect). The harness writes dialogue_letterbox_fix.ini before LoadLibrary and deletes the old log; every mode that
// checks full logging writes "log=1" (the INI default is log=0); log0 writes log=0 and `default` writes no INI at all (both must
// produce only the load line, the first APPLY pair, the first SETTOP/SETBOTTOM and problem lines).
// Mode `core`: calls the DLL's KPatchInit with a stub KPatchApi whose "kpatch.log" service appends "<channel> <level> <text>" to
// core_lines.txt (level 0 ERR, 1 WARN, 2 INFO); checks the load line (once, main), the WARN FOREIGN line, nothing in the fallback
// file, no FrameTick calls and every Submit length. In every mode each log message must be <= 220 bytes (core cut: 232).
// dlbOnScreenSize is given a fake frame (ebp = frame + 0x40, [ebp-0xC] = &fakeManager) and the fake manager's W/H shorts at
// +0x6C/+0x6E.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "kpatch_api.h"

static int g_fail = 0;
static void check(bool ok, const char* what) { printf("%s %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++g_fail; }

static char pad[0x700000];   // covers 0x9A7000 .. 0x9A8000 and 0xA20000 .. 0xA31000 (checked at runtime)
static const unsigned kConst = 0x009A7D30;
static const uint8_t kVanilla[8] = { 0x00, 0x00, 0x00, 0x80, 0xAA, 0xAA, 0x02, 0x40 };
static const char* const kId = "k2-dialogue-letterbox-fix";
static const size_t kMaxMsg = 220;
enum { L_ERR = 0, L_WARN = 1, L_INFO = 2, L_ANY = -1 };

typedef void (__cdecl *Hook)(void*);
typedef void (__cdecl *Hook2)(void*, int);
static Hook pOnSize, pSetTop, pSetBottom, pW1, pW2, pW3, pW5;
static Hook2 pW4;
static KPatchInitFn pInit;
static bool g_isProbe;
static const char* g_ver;       // "1.0.2" or "1.0.2-probe"
static uint8_t* g_mgr;          // heap, 0x100 bytes
static uint8_t* g_frame;        // heap, 0x80 bytes
static uint8_t* g_lb;           // heap, 0x100 bytes

// ---- the fallback log: "<ms> <id>/<ch> [WARN |ERR ]text" ----
struct Line { unsigned ms; std::string ch; int lvl; std::string text; };
static std::vector<Line> g_lines;
static int g_badFormat;         // lines that do not match the prefix shape (counted by every read)
static std::string g_raw;

static bool fileSize(const char* path, long* size) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fclose(f);
    return true;
}

static void readLog() {
    g_lines.clear();
    g_raw.clear();
    g_badFormat = 0;
    FILE* f = fopen("dialogue_letterbox_fix_log.txt", "rb");
    if (!f) return;
    char buf[65536];
    size_t k;
    while ((k = fread(buf, 1, sizeof buf, f)) > 0) g_raw.append(buf, k);
    fclose(f);
    size_t pos = 0;
    const std::string idSlash = std::string(kId) + "/";
    while (pos < g_raw.size()) {
        size_t e = g_raw.find('\n', pos);
        if (e == std::string::npos) e = g_raw.size();
        std::string ln = g_raw.substr(pos, e - pos);
        pos = e + 1;
        if (!ln.empty() && ln.back() == '\r') ln.pop_back();   // fopen "a" is text mode: CRLF
        if (ln.empty()) continue;
        char* end = nullptr;
        unsigned long ms = strtoul(ln.c_str(), &end, 10);
        Line L; L.ms = (unsigned)ms; L.lvl = L_INFO;
        if (end == ln.c_str() || *end != ' ' || ln.compare(end + 1 - ln.c_str(), idSlash.size(), idSlash) != 0) { ++g_badFormat; L.ch = "?"; L.text = ln; g_lines.push_back(L); continue; }
        const char* p = end + 1 + idSlash.size();
        const char* sp = strchr(p, ' ');
        if (!sp) { ++g_badFormat; L.ch = "?"; L.text = ln; g_lines.push_back(L); continue; }
        L.ch.assign(p, sp - p);
        const char* t = sp + 1;
        if (!strncmp(t, "WARN ", 5)) { L.lvl = L_WARN; t += 5; }
        else if (!strncmp(t, "ERR ", 4)) { L.lvl = L_ERR; t += 4; }
        L.text = t;
        g_lines.push_back(L);
    }
}
// text contains `s` (any channel/level)
static bool logHas(const char* s) { readLog(); for (const Line& l : g_lines) if (l.text.find(s) != std::string::npos) return true; return false; }
static int logCount(const char* s) { readLog(); int n = 0; for (const Line& l : g_lines) if (l.text.find(s) != std::string::npos) ++n; return n; }
// text contains `s` on channel `ch` at level `lvl` (L_ANY = any)
static int countOn(const char* ch, int lvl, const char* s) {
    readLog();
    int n = 0;
    for (const Line& l : g_lines) if (l.ch == ch && (lvl == L_ANY || l.lvl == lvl) && l.text.find(s) != std::string::npos) ++n;
    return n;
}
static bool hasOn(const char* ch, int lvl, const char* s) { return countOn(ch, lvl, s) > 0; }
// number of PROBE gate lines (W1..W5; W3b rides on its W3 and takes no gate slot, the cap note is not a record)
static int pcount() {
    readLog();
    int n = 0;
    for (const Line& l : g_lines) if (l.text.compare(0, 7, "PROBE W") == 0 && l.text.compare(0, 8, "PROBE W3") != 0) ++n;
    for (const Line& l : g_lines) if (l.text.compare(0, 9, "PROBE W3 ") == 0) ++n;
    return n;
}
static int countStart(const char* prefix) {
    readLog();
    int n = 0;
    const size_t k = strlen(prefix);
    for (const Line& l : g_lines) if (l.text.compare(0, k, prefix) == 0) ++n;
    return n;
}

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

// Generic checks run at the end of every fallback-sink mode.
static void finishFallbackChecks() {
    readLog();
    check(g_badFormat == 0, "every log line has the '<ms> k2-dialogue-letterbox-fix/<channel> ' prefix");
    size_t mx = 0;
    for (const Line& l : g_lines) if (l.text.size() > mx) mx = l.text.size();
    char w[160]; snprintf(w, sizeof w, "every log message <= %u bytes (longest %u)", (unsigned)kMaxMsg, (unsigned)mx);
    check(mx <= kMaxMsg, w);
    snprintf(w, sizeof w, "load line '%s %s loaded:' written exactly once, on /main at INFO", kId, g_ver);
    char want[120]; snprintf(want, sizeof want, "%s %s loaded:", kId, g_ver);
    check(countOn("main", L_INFO, want) == 1 && logCount("loaded:") == 1, w);
}

// ---- probe fakes: fake objects live at game VAs inside the .bss pad (0xA20000.. / DAT_00a309b4..bc at 0xA309B4..) ----
static uint8_t* const kLb = (uint8_t*)0x00A20000;      // fake CSWGuiDialogLetterbox
static uint8_t* const kMgr = (uint8_t*)0x00A20400;     // fake GUI manager
static uint8_t* const kOther = (uint8_t*)0x00A20800;   // fake non-letterbox panel (other vtable)
static uint8_t* const kFrameA = (uint8_t*)0x00A20C00 + 0x100;   // ebp for W1/W3/W4/W5
static uint8_t* const kFrameB = (uint8_t*)0x00A21000 + 0x100;   // ebp for W2
static const uint32_t kVtbl = 0x009A7CBC;

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
static bool probeSetup() {
    if ((uintptr_t)pad > 0xA20000 || (uintptr_t)pad + sizeof pad < 0xA31000) { check(false, "pad covers 0xA20000-0xA31000"); return false; }
    *(int16_t*)(kMgr + 0x6C) = 3840; *(int16_t*)(kMgr + 0x6E) = 1600;
    *(uint32_t*)(kOther + 0) = 0x009A7000;
    *(int32_t*)0xA309B4 = 267; *(int32_t*)0xA309B8 = 1333; *(uint8_t*)0xA309BC = 1;
    return true;
}
// One record per window (the same call sequence as the 1.0.1-probe harness).
static void probeOneEach() {
    setLb(1, 0, 0, 0, 3840, 267, 0x44);                    // mode 1 (top), bits 2 and 6 set
    setFrameA(0, 0, 3840, 8); pW1(kFrameA);
    setRect(kFrameB - 0x28, 0, 1333, 3840, 267); *(void**)(kFrameB - 0x34) = kLb; *(int32_t*)(kLb + 0x88) = 2; pW2(kFrameB);
    *(int32_t*)(kLb + 0x88) = 1;
    pW3(kFrameA);
    setFrameW4(kLb, 0, 0, 3840, 267); pW4(kFrameA, 1);
    *(int32_t*)(kFrameA + 8) = 1; pW5(kFrameA);
    *(int32_t*)(kFrameA + 8) = 0; pW5(kFrameA);
    setFrameA(0, 0, 3840, 8);                               // restore the dt slot
}

static void runProbe(const char* mode) {
    const bool on = !strcmp(mode, "probe");
    const bool log0 = !strcmp(mode, "probelog0");
    if (!probeSetup()) return;
    check(logHas(on || log0 ? "probe=1 const@" : "probe=0 const@"), on || log0 ? "load line shows probe=1" : "load line shows probe=0 (explicit INI)");
    // bare "enabled=1 mode=h6 log=1" prefix of the 1.0.0 load line is unchanged
    check(logHas(log0 ? "enabled=1 mode=h6 log=0 probe=" : "enabled=1 mode=h6 log=1 probe="), "load line keeps the 1.0.0 prefix, probe= appended");

    // ---- one record per window, every field ----
    probeOneEach();
    if (!on) {
        // probe=0 / log=0: every handler returned at once and wrote nothing
        check(pcount() == 0 && logCount("PROBE") == 0, log0 ? "probe=1 log=0: no PROBE line written" : "probe=0: no PROBE line written by W1..W5");
        pW4(nullptr, 0); pW1(nullptr); pW2(nullptr); pW3(nullptr); pW5(nullptr);
        pSetTop(kLb);                                       // the SETTOP hook still works with probe=0
        if (!log0) check(hasOn("set", L_INFO, "SETTOP lb="), "probe=0: the SETTOP line is still written (on /set)");
        else check(hasOn("set", L_INFO, "SETTOP lb=") && countOn("set", L_ANY, "SETTOP lb=") == 1, "probe=1 log=0: the first SETTOP is still written (log=0 first-event rule)");
        // garbage frame with probe off: returns before reading anything
        pW4((void*)0x10, 1); pW1((void*)0x10);
        check(pcount() == 0, "probe off: garbage ebp is never dereferenced, still nothing logged");
        if (!log0) check(!logHas("PROBE") && !logHas("W3b"), "probe=0: log has no PROBE text anywhere");
        return;
    }
    check(hasOn("probe", L_INFO, "PROBE W1 n=1 skip=0 this=0x00a20000 mode=1 done=0 alpha=0x3f000000 passedRect(x,y,w,h)=(0,0,3840,8) ctrlRect(x,y,w,h)=(0,0,3840,267) targetH(+0x74)=267"), "W1 line on /probe: mode/done/alpha/passed rect/control rect/target h");
    check(hasOn("probe", L_INFO, "PROBE W2 n=1 skip=0 this=0x00a20000 mode=2 done=0 alpha=0x3f000000 passedRect(x,y,w,h)=(0,1333,3840,267)"), "W2 line on /probe: mode 2, passed rect (0,1333,3840,267)");
    check(hasOn("probe", L_INFO, "PROBE W3 n=1 skip=0 this=0x00a20000 vtbl=0x009a7cbc(letterbox) flags(+0x48)=0x00000044 bit2=1 bit6=1 rect(x,y,w,h)=(0,0,3840,267) W=3840 H=1600 predicate=WOULD-DRAW dt=0x3c888889"),
          "W3 line on /probe: vtbl, flag bits, rect, W/H, WOULD-DRAW, dt bits");
    check(hasOn("probe", L_INFO, "PROBE W3b n=1 skip=0 alpha=0x3f000000 color(+0x54..)=0x00000000,0x00000000,0x00000000 mgr=0x00a20400 DAT_a309b4=267 DAT_a309b8=1333 DAT_a309bc=1"),
          "W3b line on /probe: alpha, colour, mgr, DAT_a309b4/b8/bc");
    check(hasOn("probe_w4", L_INFO, "PROBE W4 n=1 skip=0 this=0x00a20000 mode=1 eax=1 fillRect(x,y,w,h)=(0,0,3840,267) alpha=0x3f000000") && countOn("probe", L_ANY, "PROBE W4") == 0, "W4 line on /probe_w4 (not on /probe): eax=1 and the fill rect");
    check(hasOn("probe", L_INFO, "PROBE W5 n=1 skip=0 arg=1 this=0x00a20000") && hasOn("probe", L_INFO, "PROBE W5 n=2 skip=0 arg=0 this=0x00a20000"), "W5 lines on /probe: arg 1 then arg 0");
    check(logCount("UNREADABLE") == 0, "no UNREADABLE marker on valid objects");
    {   // W3 and W3b: same n= each time, one W3b per W3
        readLog();
        std::vector<int> n3, n3b;
        for (const Line& l : g_lines) {
            int n = 0;
            if (l.text.compare(0, 9, "PROBE W3 ") == 0 && sscanf(l.text.c_str() + 9, "n=%d", &n) == 1) n3.push_back(n);
            else if (l.text.compare(0, 10, "PROBE W3b ") == 0 && sscanf(l.text.c_str() + 10, "n=%d", &n) == 1) n3b.push_back(n);
        }
        check(!n3.empty() && n3 == n3b, "every PROBE W3 line has a PROBE W3b line with the same n=");
    }

    // ---- W3 predicate: each failing term in V8b order ----
    struct P { const char* name; int x, y, w, h; uint32_t flags; };
    const P cases[] = { { "FAIL:bit6", 0, 0, 3840, 267, 0x04 }, { "FAIL:x>=0", -1, 0, 3840, 267, 0x44 }, { "FAIL:y>=0", 0, -22, 3840, 267, 0x44 },
                        { "FAIL:w>0", 0, 0, 0, 267, 0x44 }, { "FAIL:h>0", 0, 0, 3840, 0, 0x44 }, { "FAIL:x+w<=W", 10, 0, 3840, 267, 0x44 },
                        { "FAIL:y+h<=H", 0, 1400, 3840, 267, 0x44 } };
    for (const P& c : cases) {
        setLb(1, 0, c.x, c.y, c.w, c.h, c.flags);
        pW3(kFrameA);
        char want[80]; snprintf(want, sizeof want, "predicate=%s dt=", c.name);
        check(hasOn("probe", L_INFO, want), want);
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
    check(hasOn("probe", L_INFO, "this=0x00001000") && hasOn("probe", L_INFO, "[UNREADABLE]"), "W2 with an unreadable this: line carries [UNREADABLE], no crash");
    *(void**)(kFrameB - 0x34) = kLb;
    pW1(nullptr); pW2(nullptr); pW3(nullptr); pW5(nullptr);

    if (log0) return;

    // ---- rate limit: first 120 per window every frame, then 1/s; SETTOP resets ----
    int w2Before = countStart("PROBE W2 ");
    DWORD t0 = GetTickCount();
    for (int i = 0; i < 300; ++i) pW2(kFrameB);
    DWORD dt = GetTickCount() - t0;
    int w2After = countStart("PROBE W2 ");
    printf("300 W2 calls took %lu ms, W2 lines %d -> %d\n", dt, w2Before, w2After);
    if (dt < 900) check(w2After == 120, "W2: exactly 120 lines for 300 rapid calls");
    else check(w2After >= 120, "W2: >= 120 lines (slow machine, 1/s tail may have added some)");
    Sleep(1100);
    pW2(kFrameB);
    check(countStart("PROBE W2 ") == w2After + 1 && logHas("PROBE W2 n=121 skip="), "W2: after 1.1 s one more line (n=121) with skip= count");
    pSetTop(kLb);                                            // new dialogue: counters restart
    pW2(kFrameB);
    check(logHas("PROBE W2 n=1 skip=0 this=") && logCount("PROBE W2 n=1 ") >= 2, "SETTOP resets the per-window counters (W2 n=1 logged again)");

    // ---- global cap 3000 ----
    for (int round = 0; round < 12; ++round) {
        pSetTop(kLb);
        for (int i = 0; i < 125; ++i) { pW1(kFrameA); pW2(kFrameB); pW3(kFrameA); pW4(kFrameA, 1); pW5(kFrameA); }
    }
    int total = pcount();
    printf("PROBE lines after cap test: %d\n", total);
    check(total == 3000, "global cap: exactly 3000 PROBE gate lines written (W3b rides on W3)");
    check(countOn("probe", L_INFO, "PROBE cap (3000 lines) reached") == 1, "global cap: one cap notice line, on /probe");
    check(hasOn("set", L_INFO, "SETTOP lb=") && logHas("loaded:"), "load/SETTOP lines coexist with the PROBE lines");
    {   // every PROBE line is on /probe or (W4 only) /probe_w4
        readLog();
        bool ok = true;
        for (const Line& l : g_lines) {
            if (l.text.compare(0, 6, "PROBE ") != 0) continue;
            const bool w4 = l.text.compare(0, 8, "PROBE W4") == 0;
            if (l.ch != (w4 ? "probe_w4" : "probe") || l.lvl != L_INFO) ok = false;
        }
        check(ok, "all PROBE lines are INFO on /probe (W4 on /probe_w4)");
    }
}

// ---- core mode: stub "kpatch.log" service ----
static const int kMaxCh = 16;
static char* g_chName[kMaxCh];
static int g_nCh;
static int g_ticks, g_badSubmit, g_submits;
static char g_regId[80];
static FILE* g_coreOut;
static int32_t __cdecl stRegister(const char* id) { snprintf(g_regId, sizeof g_regId, "%s", id ? id : "(null)"); return 1; }
static int32_t __cdecl stChannel(int32_t, const char* name) {
    if (!name) return -1;
    for (int i = 0; i < g_nCh; ++i) if (!strcmp(g_chName[i], name)) return i;
    if (g_nCh >= kMaxCh) return -1;
    g_chName[g_nCh] = _strdup(name);
    return g_nCh++;
}
static int32_t __cdecl stEnabled(int32_t, int32_t) { return 1; }
static void __cdecl stSubmit(int32_t ch, int32_t lvl, const char* line, uint32_t len) {
    ++g_submits;
    if (ch < 0 || ch >= g_nCh || !line || len == 0 || len != strlen(line) || len > kMaxMsg || strchr(line, '\n')) { ++g_badSubmit; printf("bad Submit ch=%d len=%u\n", (int)ch, (unsigned)len); }
    if (g_coreOut && ch >= 0 && ch < g_nCh) { fprintf(g_coreOut, "%s %d %s\n", g_chName[ch], (int)lvl, line); fflush(g_coreOut); }
}
static void __cdecl stMark(const char*) {}
static void __cdecl stFrameTick(int32_t) { ++g_ticks; }
static uint32_t __cdecl stFrame(void) { return 0; }
static uint64_t __cdecl stNowUs(void) { return 0; }
static KPatchLogApi g_logApi;
static const void* __cdecl stRequire(const char* name, uint32_t ver) {
    if (name && !strcmp(name, KPATCH_LOG_IFACE) && ver == KPATCH_LOG_VERSION) return &g_logApi;
    return nullptr;
}
static int32_t __cdecl stProvide(const char*, uint32_t, const void*) { return 1; }
static KPatchApi g_api;

// Lines of core_lines.txt as (channel, level, text).
struct CoreLine { std::string ch; int lvl; std::string text; };
static std::vector<CoreLine> readCore() {
    std::vector<CoreLine> v;
    FILE* f = fopen("core_lines.txt", "rb");
    if (!f) return v;
    char buf[2048];
    while (fgets(buf, sizeof buf, f)) {
        size_t n = strlen(buf);
        while (n && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) buf[--n] = 0;
        char ch[40]; int lvl = 0, off = 0;
        if (sscanf(buf, "%39s %d %n", ch, &lvl, &off) >= 2 && off > 0) v.push_back({ ch, lvl, buf + off });
    }
    fclose(f);
    return v;
}
static int coreCount(const std::vector<CoreLine>& v, const char* ch, int lvl, const char* s) {
    int n = 0;
    for (const CoreLine& l : v) if (l.ch == ch && (lvl == L_ANY || l.lvl == lvl) && l.text.find(s) != std::string::npos) ++n;
    return n;
}

static int runCore() {
    memset(&g_logApi, 0, sizeof g_logApi);
    g_logApi.struct_size = sizeof(KPatchLogApi);
    g_logApi.Register = stRegister; g_logApi.Channel = stChannel; g_logApi.Enabled = stEnabled; g_logApi.Submit = stSubmit;
    g_logApi.Mark = stMark; g_logApi.FrameTick = stFrameTick; g_logApi.Frame = stFrame; g_logApi.NowUs = stNowUs;
    memset(&g_api, 0, sizeof g_api);
    g_api.struct_size = sizeof(KPatchApi); g_api.abi_version = KPATCH_ABI_VERSION; g_api.provide = stProvide; g_api.require = stRequire;
    DeleteFileA("core_lines.txt");
    g_coreOut = fopen("core_lines.txt", "wb");
    if (!g_coreOut) { check(false, "core_lines.txt can be created"); return 1; }

    pInit(&g_api);   // KPatchInit(api): KpLog_Init found the service, so the load line goes to Submit right here
    {
        std::vector<CoreLine> v = readCore();
        char want[120]; snprintf(want, sizeof want, "%s %s loaded:", kId, g_ver);
        check(v.size() == 1 && coreCount(v, "main", L_INFO, want) == 1, "core: KPatchInit emitted exactly the load line, on /main at INFO");
        check(!strcmp(g_regId, kId) && g_nCh >= 1 && !strcmp(g_chName[0], "main"), "core: Register(\"k2-dialogue-letterbox-fix\") and the main channel resolved");
    }
    // FOREIGN first (constant pre-set to 2.0 by main), then the owner "restores vanilla" and the normal flow runs.
    callSize(3840, 1600);
    check(curC() == 2.0, "core: constant left at 2.0 on FOREIGN");
    setConst(kVanilla);
    callSize(3840, 1600);
    check(barFor(3840, 1600, curC()) == 267, "core: after vanilla is restored the fix applies (bar 267)");
    setSize(3840, 1600);
    int32_t* start = (int32_t*)(g_lb + 0x04); start[0] = 0; start[1] = 0; start[2] = 3840; start[3] = 0;
    int32_t* targ = (int32_t*)(g_lb + 0x68); targ[0] = 0; targ[1] = 0; targ[2] = 3840; targ[3] = 267;
    *(int32_t*)(g_lb + 0x88) = 1;
    pSetTop(g_lb);
    start[1] = 1600; start[3] = 267; targ[1] = 1333; *(int32_t*)(g_lb + 0x88) = 2;
    pSetBottom(g_lb);
    callSize(0, 1600);
    if (g_isProbe && probeSetup()) { probeOneEach(); pSetTop(kLb); pW2(kFrameB); }

    std::vector<CoreLine> v = readCore();
    check(coreCount(v, "main", L_INFO, "loaded:") == 1 && coreCount(v, "set", L_ANY, "loaded:") + coreCount(v, "probe", L_ANY, "loaded:") == 0, "core: the load line appears exactly once in the whole run");
    check(coreCount(v, "main", L_WARN, "APPLY src=SetScreenSize W=3840 H=1600: FOREIGN value 00 00 00 00 00 00 00 40") == 1, "core: FOREIGN is a WARN-level line on /main");
    check(coreCount(v, "main", L_INFO, "APPLY src=SetScreenSize: wrote (protect RW ok") == 1 && coreCount(v, "main", L_INFO, "verify OK") == 1, "core: APPLY header + wrote lines on /main at INFO");
    check(coreCount(v, "set", L_INFO, "SETTOP lb=") == (g_isProbe ? 2 : 1) && coreCount(v, "set", L_INFO, "SETBOTTOM lb=") == 1, "core: SETTOP/SETBOTTOM on /set at INFO (probe variant: a second SETTOP from the probe sequence)");
    check(coreCount(v, "main", L_WARN, "INVALID size, no write") == 1, "core: INVALID size is a WARN-level line on /main");
    if (g_isProbe) {
        check(coreCount(v, "probe", L_INFO, "PROBE W1 n=1") == 1 && coreCount(v, "probe", L_INFO, "PROBE W2 n=1") == 2 && coreCount(v, "probe", L_INFO, "PROBE W5 n=1") == 1
              && coreCount(v, "probe", L_INFO, "PROBE W3 n=1") == 1 && coreCount(v, "probe", L_INFO, "PROBE W3b n=1") == 1, "core: PROBE W1/W2/W3/W3b/W5 on /probe");
        check(coreCount(v, "probe_w4", L_INFO, "PROBE W4 n=1") == 1 && coreCount(v, "probe", L_ANY, "PROBE W4") == 0, "core: PROBE W4 on /probe_w4 only");
    }
    check(g_ticks == 0, "core: no FrameTick calls (the fix is not a frame-global hook)");
    check(g_badSubmit == 0 && g_submits == (int)v.size() && g_submits > 0, "core: every Submit has len == strlen, no newline, <= 220 bytes");
    long sz = 0;
    check(!fileSize("dialogue_letterbox_fix_log.txt", &sz) || sz == 0, "core: nothing reached the fallback file (dialogue_letterbox_fix_log.txt absent or empty)");
    fclose(g_coreOut);
    return 0;
}

struct Res { int w, h, bar; };
static const Res kRes[] = { { 3840, 1600, 267 }, { 1920, 1080, 180 }, { 3440, 1440, 240 }, { 5120, 1440, 240 }, { 800, 600, 100 }, { 640, 480, 80 } };

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "h6";
    const bool probeMode = !strcmp(mode, "probe") || !strcmp(mode, "probe0") || !strcmp(mode, "probelog0");
    const bool coreMode = !strcmp(mode, "core");
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
    if (!strcmp(mode, "foreign") || coreMode) { double d = 2.0; memcpy((void*)kConst, &d, 8); }
    VirtualProtect((void*)0x9A7000, 0x1000, PAGE_READONLY, &o);
    uint8_t startBytes[8]; memcpy(startBytes, (void*)kConst, 8);

    DeleteFileA("dialogue_letterbox_fix_log.txt");
    DeleteFileA("dialogue_letterbox_fix.ini");
    if (!strcmp(mode, "clamp")) writeIni("mode=clamp\nlog=1\n");
    else if (!strcmp(mode, "vanilla")) writeIni("mode=vanilla\nlog=1\n");
    else if (!strcmp(mode, "disabled")) writeIni("enabled=0\nlog=1\n");
    else if (!strcmp(mode, "ini")) writeIni("; comment line\r\n# another comment\r\n[section]\r\n   enabled  =  1  \r\n  MODE =  bogus \r\nlog=1\r\n");
    else if (!strcmp(mode, "log0")) writeIni("log = 0\r\nmode=h6\r\n");
    else if (!strcmp(mode, "default")) { /* no INI file at all = defaults (log=0) */ }
    else if (!strcmp(mode, "probe")) writeIni("probe=1\r\nlog=1\r\n");
    else if (!strcmp(mode, "probe0")) writeIni("probe = 0\r\nlog=1\r\n");
    else if (!strcmp(mode, "probelog0")) writeIni("probe=1\nlog=0\n");
    else if (coreMode) writeIni("log=1\nprobe=1\n");   // probe= is ignored by the shipping DLL
    else if (!strcmp(mode, "h6") || !strcmp(mode, "foreign") || !strcmp(mode, "unchanged") || !strcmp(mode, "late")) writeIni("log=1\n");   // full logging (the default is log=0)
    else { printf("unknown mode %s\n", mode); return 4; }

    HMODULE m = LoadLibraryA("windows_x86.dll");
    if (!m) { printf("LoadLibrary failed %lu\n", GetLastError()); return 1; }
    pOnSize = (Hook)GetProcAddress(m, "dlbOnScreenSize");
    pSetTop = (Hook)GetProcAddress(m, "dlbLogSetTop");
    pSetBottom = (Hook)GetProcAddress(m, "dlbLogSetBottom");
    pInit = (KPatchInitFn)GetProcAddress(m, "KPatchInit");
    check(pOnSize && pSetTop && pSetBottom && pInit, "exports KPatchInit / dlbOnScreenSize / dlbLogSetTop / dlbLogSetBottom present");
    if (!pOnSize || !pSetTop || !pSetBottom || !pInit) return 3;
    pW1 = (Hook)GetProcAddress(m, "dlbProbeW1"); pW2 = (Hook)GetProcAddress(m, "dlbProbeW2"); pW3 = (Hook)GetProcAddress(m, "dlbProbeW3");
    pW4 = (Hook2)GetProcAddress(m, "dlbProbeW4"); pW5 = (Hook)GetProcAddress(m, "dlbProbeW5");
    g_isProbe = pW1 != nullptr;
    g_ver = g_isProbe ? "1.0.2-probe" : "1.0.2";
    printf("variant: %s (version %s)\n", g_isProbe ? "probe" : "shipping", g_ver);
    if (probeMode && !g_isProbe) { printf("SKIP %s: shipping DLL (no dlbProbeW1 export)\n", mode); return 0; }
    if (g_isProbe) {
        check(pW1 && pW2 && pW3 && pW4 && pW5, "exports dlbProbeW1..W5 present");
        if (!(pW1 && pW2 && pW3 && pW4 && pW5)) return 3;
    }

    if (coreMode) {
        int rc = runCore();
        printf("%s: %d failures\n", mode, g_fail);
        return rc ? rc : (g_fail ? 1 : 0);
    }

    // DllMain only formats the load line; nothing is written until the first hook call (fallback sink opens lazily).
    long sz = 0;
    check(!fileSize("dialogue_letterbox_fix_log.txt", &sz), "no log file right after LoadLibrary (the load line is not written from DllMain)");
    pSetTop(nullptr);   // first hook call: emits the load line (nothing else: null lb returns)
    {
        char want[120]; snprintf(want, sizeof want, "%s %s loaded:", kId, g_ver);
        check(hasOn("main", L_INFO, want), "log: first hook call wrote the load line with version, on /main at INFO");
    }
    if (!strcmp(mode, "foreign"))
        check(logHas("00 00 00 00 00 00 00 40"), "log: load line shows the pre-existing 2.0 bytes");
    else
        check(logHas("2.3333330154418945") && logHas("00 00 00 80 aa aa 02 40"), "log: load line shows vanilla bytes/value");

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
        check(logCount("INVALID size, no write") == 2 && countOn("main", L_WARN, "INVALID size, no write") == 2, "log: two INVALID lines, both /main WARN");
        // null ebp: no crash, no change
        pOnSize(nullptr);
        check(memcmp(before, (void*)kConst, 8) == 0, "null ebp: no crash, constant unchanged");
        // null manager: logged and skipped
        void* saved = *(void**)(g_frame + 0x40 - 0xC);
        *(void**)(g_frame + 0x40 - 0xC) = nullptr;
        pOnSize(g_frame + 0x40);
        *(void**)(g_frame + 0x40 - 0xC) = saved;
        check(memcmp(before, (void*)kConst, 8) == 0 && hasOn("main", L_WARN, "manager pointer [ebp-0xC] is null"), "null manager: skipped and logged (/main WARN)");
        // letterbox log hooks
        setSize(3840, 1600);
        int32_t* start = (int32_t*)(g_lb + 0x04); start[0] = 0; start[1] = 0; start[2] = 3840; start[3] = 0;
        int32_t* targ = (int32_t*)(g_lb + 0x68); targ[0] = 0; targ[1] = 0; targ[2] = 3840; targ[3] = 267;
        *(int32_t*)(g_lb + 0x88) = 1;
        pSetTop(g_lb);
        start[1] = 1600; start[3] = 267; targ[1] = 1333; *(int32_t*)(g_lb + 0x88) = 2;
        pSetBottom(g_lb);
        pSetTop(nullptr); pSetBottom(nullptr);
        check(hasOn("set", L_INFO, "SETTOP lb=") && logHas("target(x,y,w,h)=(0,0,3840,267)") && logHas("W=3840 H=1600"), "log: SETTOP line on /set with target rect (0,0,3840,267) and W/H");
        check(hasOn("set", L_INFO, "SETBOTTOM lb=") && logHas("target(x,y,w,h)=(0,1333,3840,267)") && logHas("mode(+0x88, before this store)=2"), "log: SETBOTTOM line on /set with target rect (0,1333,3840,267) and mode 2");
        check(countOn("main", L_ANY, "SETTOP lb=") == 0 && countOn("set", L_ANY, "APPLY") == 0, "channels: SETTOP/SETBOTTOM only on /set, APPLY only on /main");
        check(logCount("LATE(") == 0, "no LATE fallback after a SetScreenSize hit");
        // restore vanilla size sequence (credits 640x480 then back) still follows
        callSize(640, 480); callSize(3840, 1600);
        check(barFor(3840, 1600, curC()) == 267, "credits-style 640x480 then back to 3840x1600: bar 267 again");
        check(logHas("vanillaBar=-22 newBar=267 X=1066 fs=2.0 panelH=200 replyY=1333"), "log: 3840x1600 numbers (vanillaBar -22, newBar 267, X 1066, fs 2.0, reply y 1333)");
        check(hasOn("main", L_INFO, "APPLY src=SetScreenSize W=3840 H=1600 mode=h6") && hasOn("main", L_INFO, "verify OK"), "log: APPLY header and verify OK are /main INFO");
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
        check(hasOn("main", L_INFO, "unchanged"), "log: unchanged (/main INFO)");
    } else if (!strcmp(mode, "disabled")) {
        callSize(3840, 1600);
        check(isVanilla() && memcmp(startBytes, (void*)kConst, 8) == 0, "disabled: bytes stay vanilla");
        check(hasOn("main", L_INFO, "DISABLED, no write"), "log: DISABLED, no write (/main INFO)");
        check(logHas("enabled=0"), "log: load line shows enabled=0");
    } else if (!strcmp(mode, "foreign")) {
        callSize(3840, 1600);
        check(memcmp(startBytes, (void*)kConst, 8) == 0 && curC() == 2.0, "foreign: bytes unchanged (2.0)");
        check(hasOn("main", L_WARN, "FOREIGN value") && logHas("another mod owns 0x009a7d30, not writing"), "log: FOREIGN line is /main WARN");
        callSize(1920, 1080);
        check(curC() == 2.0 && logCount("FOREIGN value") == 2 && countOn("main", L_WARN, "FOREIGN value") == 2, "foreign: still refuses on a later hit (two /main WARN lines)");
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
        check(hasOn("main", L_INFO, "no SetScreenSize hit seen yet, applying late fallback"), "late: the LATE notice is a /main INFO line");
        pSetBottom(g_lb);
        check(logCount("APPLY src=LATE(SetTop) W=") == 1 && logCount("APPLY src=LATE(SetBottom)") == 0, "late: fallback runs once");
    } else if (!strcmp(mode, "ini")) {
        check(hasOn("main", L_WARN, "unknown mode 'bogus'"), "ini (CRLF, comments, spaces, [section]): unknown mode logged as /main WARN");
        check(logHas("enabled=1 mode=h6 log=1"), "ini: unknown mode falls back to h6, enabled/log parsed with spaces");
        callSize(3840, 1600);
        check(barFor(3840, 1600, curC()) == 267, "ini: h6 behaviour, bar 267");
    } else if (!strcmp(mode, "log0") || !strcmp(mode, "default")) {
        // log=0 semantics (log0 = INI log=0, default = no INI): load line, first APPLY pair, first SETTOP and
        // first SETBOTTOM, problem lines; nothing else.
        char w[200];
#define LCHK(ok, text) do { snprintf(w, sizeof w, "%s: %s", mode, text); check(ok, w); } while (0)
        LCHK(logHas("enabled=1 mode=h6 log=0"), "load line written and shows log=0");
        callSize(3840, 1600);
        LCHK(barFor(3840, 1600, curC()) == 267, "fix still applied, bar 267");
        LCHK(logCount("APPLY src=SetScreenSize W=3840 H=1600 mode=h6") == 1 && logCount("verify OK") == 1, "first APPLY pair logged (header + verify OK)");
        callSize(1920, 1080);
        LCHK(barFor(1920, 1080, curC()) == 180, "second SetScreenSize (1920x1080) applied, bar 180");
        LCHK(logCount("APPLY src=") == 2 && !logHas("W=1920 H=1080") && logCount("wrote (protect") == 1 && logCount("verify OK") == 1,
             "second apply NOT logged (still exactly the first header + wrote lines)");
        setSize(3840, 1600);
        int32_t* start = (int32_t*)(g_lb + 0x04); start[0] = 0; start[1] = 0; start[2] = 3840; start[3] = 0;
        int32_t* targ = (int32_t*)(g_lb + 0x68); targ[0] = 0; targ[1] = 0; targ[2] = 3840; targ[3] = 267;
        *(int32_t*)(g_lb + 0x88) = 1;
        pSetTop(g_lb);
        start[1] = 1600; start[3] = 267; targ[1] = 1333; *(int32_t*)(g_lb + 0x88) = 2;
        pSetBottom(g_lb);
        LCHK(countOn("set", L_INFO, "SETTOP lb=") == 1 && countOn("set", L_INFO, "SETBOTTOM lb=") == 1 && logCount("SETTOP lb=") == 1 && logCount("SETBOTTOM lb=") == 1, "first SETTOP and first SETBOTTOM logged, on /set");
        LCHK(logHas("target(x,y,w,h)=(0,0,3840,267)") && logHas("target(x,y,w,h)=(0,1333,3840,267)"), "first SETTOP/SETBOTTOM lines carry the target rects");
        // second dialogue
        *(int32_t*)(g_lb + 0x88) = 0; pSetTop(g_lb); *(int32_t*)(g_lb + 0x88) = 1; pSetBottom(g_lb);
        pSetTop(g_lb); pSetBottom(g_lb);
        LCHK(logCount("SETTOP lb=") == 1 && logCount("SETBOTTOM lb=") == 1, "second and later SetTop/SetBottom NOT logged");
        LCHK(logCount("LATE(") == 0, "no LATE fallback after a SetScreenSize hit");
        callSize(0, 0);
        LCHK(hasOn("main", L_WARN, "INVALID size"), "problem line (INVALID) still written, /main WARN");
        LCHK(countOn("main", L_INFO, "INVALID") == 0, "the problem line carries WARN (not INFO)");
#undef LCHK
    } else if (probeMode) {
        runProbe(mode);
    } else {
        printf("unknown mode %s\n", mode);
        return 4;
    }
    finishFallbackChecks();
    printf("%s: %d failures\n", mode, g_fail);
    return g_fail ? 1 : 0;
}
