// k2-dialogue-letterbox-fix 1.0.0 (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Doc 15 Phase 2 S3, finding L1 (doc 17).
//
// Problem: the dialogue letterbox bar height is bar = trunc((H - trunc(W / c)) / 2) with c a double at .rdata VA
// 0x009A7D30 (vanilla 2.3333330154418945, bytes 00 00 00 80 AA AA 02 40), read by exactly 7 FDIV instructions and
// nothing else. At 3840x1600 W/c = 1645 > H, so bar = -22 and the letterbox disappears.
// Fix: write c = 1.5*W/H (mode h6) so that bar = H/6 at every resolution. W/H are the int16 shorts at GUI-manager
// +0x6C/+0x6E; they are only written by CSWGuiManager::SetScreenSize (0x411A80). Hook 1 sits at 0x411AC1 inside that
// function (after both stores, [ebp-0xC] = manager) and is reached only when W or H changed (startup, options Apply,
// credits 640x480 force + restore). Hooks 2 and 3 (CSWGuiDialogLetterbox::SetTop/SetBottom, ECX = this) only log, and
// give a one-shot late fallback if hook 1 was never seen.
//
// Config: dialogue_letterbox_fix.ini in the game dir (cwd): enabled=1|0 (default 1), mode=h6|clamp|vanilla (default h6),
// log=1|0 (default 1; log=0 still writes the load line and problem lines). Missing file = defaults.
// Log: dialogue_letterbox_fix_log.txt (appended, one line per event, capped at 5000 lines per process).
// Safety: the constant is only ever written when its current 8 bytes are the vanilla bytes or the value this DLL wrote
// earlier; anything else means another mod owns it and the DLL keeps its hands off.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

namespace {

const char* const kVersion = "k2-dialogue-letterbox-fix 1.0.0";
constexpr uintptr_t kConstVA = 0x009A7D30;
const uint8_t kVanilla[8] = { 0x00, 0x00, 0x00, 0x80, 0xAA, 0xAA, 0x02, 0x40 };
constexpr double kNudge = 1.0 - 1e-9;   // keeps trunc(W/c) from landing one below 2H/3 when 2H/3 is an integer (V7 Q5)
constexpr int kTotalLogCap = 5000;
constexpr int kSetLogCap = 400;

enum Mode { MODE_H6 = 0, MODE_CLAMP = 1, MODE_VANILLA = 2 };
const char* const kModeNames[] = { "h6", "clamp", "vanilla" };

bool g_enabled = true;
bool g_log = true;
int g_mode = MODE_H6;
CRITICAL_SECTION g_lock;
bool g_lockInit = false;
int g_logLines = 0;
bool g_logCapNoted = false;
int g_setLines = 0;
bool g_setCapNoted = false;
int g_hits = 0;                 // SetScreenSize hook hits
bool g_foreign = false;         // sticky: the constant was found holding someone else's value
uint8_t g_written[8];
bool g_haveWritten = false;
bool g_lateDone = false;

// force = true lines (load line, problems) are written even with log=0.
void logv(bool force, const char* fmt, va_list ap) {
    if (!g_log && !force) return;
    char line[700];
    vsnprintf(line, sizeof(line), fmt, ap);
    if (g_lockInit) EnterCriticalSection(&g_lock);
    if (g_logLines < kTotalLogCap) {
        ++g_logLines;
        if (FILE* f = fopen("dialogue_letterbox_fix_log.txt", "a")) {
            fprintf(f, "[%lu] %s\n", GetTickCount(), line);
            fclose(f);
        }
    } else if (!g_logCapNoted) {
        g_logCapNoted = true;
        if (FILE* f = fopen("dialogue_letterbox_fix_log.txt", "a")) {
            fprintf(f, "[%lu] log cap (%d lines) reached, further lines dropped\n", GetTickCount(), kTotalLogCap);
            fclose(f);
        }
    }
    if (g_lockInit) LeaveCriticalSection(&g_lock);
}
void logf(const char* fmt, ...) { va_list ap; va_start(ap, fmt); logv(false, fmt, ap); va_end(ap); }
void logForce(const char* fmt, ...) { va_list ap; va_start(ap, fmt); logv(true, fmt, ap); va_end(ap); }

double asDouble(const uint8_t* b) { double d; memcpy(&d, b, 8); return d; }
void toBytes(double d, uint8_t* b) { memcpy(b, &d, 8); }
void hex8(const uint8_t* b, char* out) { sprintf(out, "%02x %02x %02x %02x %02x %02x %02x %02x", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]); }

// bar = trunc((H - trunc(W / c)) / 2), the engine's formula.
int xFor(int w, double c) { return static_cast<int>(static_cast<double>(w) / c); }
int barFor(int w, int h, double c) { return static_cast<int>(static_cast<double>(h - xFor(w, c)) / 2.0); }

bool readConst(uint8_t* out) {
    MEMORY_BASIC_INFORMATION mb;
    if (!VirtualQuery(reinterpret_cast<void*>(kConstVA), &mb, sizeof(mb)) || mb.State != MEM_COMMIT) return false;
    if (mb.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return false;
    memcpy(out, reinterpret_cast<void*>(kConstVA), 8);
    return true;
}

// ---- INI ----
char* trim(char* s) {
    while (*s == ' ' || *s == '\t') ++s;
    char* e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n')) *--e = 0;
    return s;
}
bool flagValue(const char* v) {
    return !(v[0] == '0' || !_stricmp(v, "off") || !_stricmp(v, "false") || !_stricmp(v, "no"));
}
char g_badMode[64];
void readConfig() {
    g_badMode[0] = 0;
    FILE* f = fopen("dialogue_letterbox_fix.ini", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* p = trim(line);
        if (*p == 0 || *p == ';' || *p == '#' || *p == '[') continue;
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = 0;
        char* key = trim(p);
        char* val = trim(eq + 1);
        if (!_stricmp(key, "enabled")) g_enabled = flagValue(val);
        else if (!_stricmp(key, "log")) g_log = flagValue(val);
        else if (!_stricmp(key, "mode")) {
            if (!_stricmp(val, "h6")) g_mode = MODE_H6;
            else if (!_stricmp(val, "clamp")) g_mode = MODE_CLAMP;
            else if (!_stricmp(val, "vanilla")) g_mode = MODE_VANILLA;
            else { g_mode = MODE_H6; strncpy(g_badMode, val, sizeof(g_badMode) - 1); g_badMode[sizeof(g_badMode) - 1] = 0; }
        }
    }
    fclose(f);
}

bool sameBytes(const uint8_t* a, const uint8_t* b) { return memcmp(a, b, 8) == 0; }

void apply(int w, int h, const char* src) {
    ++g_hits;
    if (!g_enabled) { logf("APPLY src=%s W=%d H=%d: DISABLED, no write", src, w, h); return; }
    if (w <= 0 || h <= 0) { logForce("APPLY src=%s W=%d H=%d: INVALID size, no write", src, w, h); return; }

    uint8_t cur[8];
    if (!readConst(cur)) { logForce("APPLY src=%s W=%d H=%d: constant 0x%08x is not readable, no write", src, w, h, (unsigned)kConstVA); return; }
    char hx[40];
    const double vanilla = asDouble(kVanilla);
    const double curD = asDouble(cur);
    const bool isVanilla = sameBytes(cur, kVanilla);
    const bool isOurs = g_haveWritten && sameBytes(cur, g_written);
    if (!isVanilla && !isOurs) {
        hex8(cur, hx);
        g_foreign = true;
        logForce("APPLY src=%s W=%d H=%d: FOREIGN value %s %.17g: another mod owns 0x%08x, not writing", src, w, h, hx, curD, (unsigned)kConstVA);
        return;
    }

    double target;
    switch (g_mode) {
        case MODE_CLAMP:
            target = (barFor(w, h, vanilla) < 0) ? static_cast<double>(w) / static_cast<double>(h) * kNudge : vanilla;
            break;
        case MODE_VANILLA:
            target = vanilla;
            break;
        default:
            target = 1.5 * static_cast<double>(w) / static_cast<double>(h) * kNudge;
            break;
    }
    uint8_t tb[8];
    toBytes(target, tb);

    const int vBar = barFor(w, h, vanilla);
    const int nBar = barFor(w, h, target);
    const int X = xFor(w, target);
    const int stepsOver = (w - 1022 > 0 ? w - 1022 : 0) / 1024;
    const double fs = 1.0 + 0.5 * stepsOver;
    const int panelH = static_cast<int>(100.0 * fs);
    int replyY = (X + h) / 2;
    const int replyMax = static_cast<int>(h - 100.0 * fs);
    if (replyMax < replyY) replyY = replyMax;
    logf("APPLY src=%s W=%d H=%d mode=%s old=%.17g new=%.17g vanillaBar=%d newBar=%d X=%d fs=%.1f panelH=%d replyY=%d",
         src, w, h, kModeNames[g_mode], curD, target, vBar, nBar, X, fs, panelH, replyY);

    if (sameBytes(cur, tb)) { logf("APPLY src=%s: unchanged (already %.17g)", src, curD); return; }

    DWORD old = 0;
    if (!VirtualProtect(reinterpret_cast<void*>(kConstVA), 8, PAGE_READWRITE, &old)) {
        logForce("APPLY src=%s: VirtualProtect(RW) FAILED err=%lu, no write", src, GetLastError());
        return;
    }
    memcpy(reinterpret_cast<void*>(kConstVA), tb, 8);
    DWORD tmp = 0;
    BOOL back = VirtualProtect(reinterpret_cast<void*>(kConstVA), 8, old, &tmp);
    g_haveWritten = true;
    memcpy(g_written, tb, 8);
    uint8_t after[8];
    bool rd = readConst(after);
    const bool ok = rd && sameBytes(after, tb);
    hex8(rd ? after : tb, hx);
    logf("APPLY src=%s: wrote (protect RW ok, old prot=0x%lx; protect restore %s -> 0x%lx); verify %s, now %s = %.17g",
         src, old, back ? "ok" : "FAILED", back ? old : 0UL, ok ? "OK" : "MISMATCH", hx, rd ? asDouble(after) : 0.0);
    if (!back) logForce("APPLY src=%s: VirtualProtect(restore) FAILED err=%lu", src, GetLastError());
    if (!ok) logForce("APPLY src=%s: verify MISMATCH after write", src);
}

bool managerSize(const uint8_t* mgr, int* w, int* h) {
    if (!mgr) return false;
    *w = *reinterpret_cast<const int16_t*>(mgr + 0x6C);
    *h = *reinterpret_cast<const int16_t*>(mgr + 0x6E);
    return true;
}

void logLetterbox(const char* what, const uint8_t* lb) {
    const uint8_t* mgr = *reinterpret_cast<uint8_t* const*>(lb + 0x1C);
    int w = 0, h = 0;
    const bool haveMgr = managerSize(mgr, &w, &h);
    const int32_t* s = reinterpret_cast<const int32_t*>(lb + 0x04);
    const int32_t* t = reinterpret_cast<const int32_t*>(lb + 0x68);
    uint8_t cur[8];
    const bool rc = readConst(cur);
    if (g_setLines < kSetLogCap) {
        ++g_setLines;
        logf("%s lb=%p mode(+0x88, before this store)=%d start(x,y,w,h)=(%d,%d,%d,%d) target(x,y,w,h)=(%d,%d,%d,%d) mgr=%p W=%d H=%d c=%.17g",
             what, lb, *reinterpret_cast<const int32_t*>(lb + 0x88), s[0], s[1], s[2], s[3], t[0], t[1], t[2], t[3],
             mgr, w, h, rc ? asDouble(cur) : 0.0);
    } else if (!g_setCapNoted) {
        g_setCapNoted = true;
        logf("SETTOP/SETBOTTOM log cap (%d lines) reached, further lines dropped", kSetLogCap);
    }
    if (!g_lateDone && g_hits == 0 && haveMgr && w > 0 && h > 0) {
        g_lateDone = true;
        char src[40];
        sprintf(src, "LATE(%s)", what == nullptr ? "?" : (what[3] == 'T' ? "SetTop" : "SetBottom"));
        logf("%s: no SetScreenSize hit seen yet, applying late fallback (the bar just computed used the old constant)", what);
        apply(w, h, src);
    }
}

}  // namespace

// CSWGuiManager::SetScreenSize post-store hook (0x411AC1). ebp = the function frame; [ebp-0xC] = manager.
extern "C" void __cdecl dlbOnScreenSize(void* ebp) {
    if (ebp == nullptr) return;
    uint8_t* mgr = *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(ebp) - 0xC);
    if (mgr == nullptr) { logForce("dlbOnScreenSize: manager pointer [ebp-0xC] is null, skipping"); return; }
    int w, h;
    managerSize(mgr, &w, &h);
    apply(w, h, "SetScreenSize");
}

// SetTop / SetBottom log hooks (0x8BAD7C / 0x8BAF01). lb = CSWGuiDialogLetterbox this (ECX).
extern "C" void __cdecl dlbLogSetTop(void* lb) {
    if (lb == nullptr) return;
    if (!g_log && (g_lateDone || g_hits != 0)) return;
    logLetterbox("SETTOP", static_cast<const uint8_t*>(lb));
}
extern "C" void __cdecl dlbLogSetBottom(void* lb) {
    if (lb == nullptr) return;
    if (!g_log && (g_lateDone || g_hits != 0)) return;
    logLetterbox("SETBOTTOM", static_cast<const uint8_t*>(lb));
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    InitializeCriticalSection(&g_lock);
    g_lockInit = true;
    readConfig();
    uint8_t cur[8] = { 0 };
    const bool rc = readConst(cur);
    char hx[40];
    hex8(cur, hx);
    logForce("%s loaded: enabled=%d mode=%s log=%d const@0x%08x=%s (%.17g%s) pid=%lu", kVersion, g_enabled ? 1 : 0, kModeNames[g_mode],
             g_log ? 1 : 0, (unsigned)kConstVA, hx, asDouble(cur), rc ? "" : ", UNREADABLE", GetCurrentProcessId());
    if (g_badMode[0]) logForce("unknown mode '%s' in dialogue_letterbox_fix.ini, using h6", g_badMode);
    return TRUE;
}
