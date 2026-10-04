// k2-dialogue-letterbox-fix 1.0.2 (Steam Aspyr build 6A522E71...). Doc 15 Phase 2 S3, finding L1 (doc 17).
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
// log=1|0 (default 0), probe=1|0 (probe variant only, default 0). Missing file = defaults. enabled= and mode= are
// behavioural switches and are unchanged. log= is the CONTENT gate: log=0 still emits the load line, the first APPLY pair,
// the first SETTOP/SETBOTTOM pair and problem lines; log=1 emits everything (every APPLY, SETTOP/SETBOTTOM up to 400).
// Safety: the constant is only ever written when its current 8 bytes are the vanilla bytes or the value this DLL wrote
// earlier; anything else means another mod owns it and the DLL keeps its hands off.
//
// Logging (1.0.2): all lines go through the shared KpLog.h (Patches/Common). Tag text and field order are unchanged from
// 1.0.1 (the 1.0.1 "[GetTickCount] " prefix is replaced by the core's "T=<ms> f=<frame> <patch>/<channel> " prefix).
// Channels: main (load line, APPLY header + result lines, LATE notice), set (SETTOP/SETBOTTOM + the SET cap note),
// and in the probe variant probe (PROBE W1/W2/W3/W3b/W5 + the PROBE cap note) and probe_w4 (PROBE W4).
// Problem lines (INVALID size, FOREIGN value, unknown mode, null manager at WARN; not readable, VirtualProtect FAILED,
// verify MISMATCH at ERR) are always visible, with or without kplog.ini. To see the INFO lines on a core with the
// kpatch.log service, add to kplog.ini in the game dir:
//     [k2-dialogue-letterbox-fix]
//     enabled=1
//     level=info
//     channels=main,set            ; probe variant: channels=main,set,probe,probe_w4
// (and [global] enabled=1). On an old core (no service, KPatchInit never runs) the header's fallback sink writes
// dialogue_letterbox_fix_log.txt as before, at INFO, with "<ms> k2-dialogue-letterbox-fix/<channel> " in front of each line.
// The load line is not written from DllMain: DllMain only formats it. KPatchInit emits it once KpLog_Init succeeds, or the
// first hook call does when there is no service. Dropped in 1.0.2: own fopen/fprintf logging, the 5000-line total cap and
// the log lock. Kept: the 400-line SET cap and the probe rate limiter below.
//
// Variants (one source): the shipping build has no dlb_variant.h and exports 3 handlers + KPatchInit. The probe build adds
// probe/ to the include path (probe/dlb_variant.h defines DLB_PROBE), becomes version 1.0.2-probe, and compiles the five
// log-only probe windows W1-W5 (see the block "PROBE variant" below); it needs probe/kotor2-steam-aspyr.hooks.toml and
// probe/exports.def (3 + 5 hooks). Both variants use the patch id k2-dialogue-letterbox-fix, so never install both.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#if __has_include("dlb_variant.h")
#include "dlb_variant.h"
#endif
#include "KpLog.h"

KP_LOG_PATCH("k2-dialogue-letterbox-fix");

namespace {

#ifdef DLB_PROBE
const char* const kVersion = "1.0.2-probe";
#else
const char* const kVersion = "1.0.2";
#endif
constexpr uintptr_t kConstVA = 0x009A7D30;
const uint8_t kVanilla[8] = { 0x00, 0x00, 0x00, 0x80, 0xAA, 0xAA, 0x02, 0x40 };
constexpr double kNudge = 1.0 - 1e-9;   // keeps trunc(W/c) from landing one below 2H/3 when 2H/3 is an integer (V7 Q5)
constexpr int kSetLogCap = 400;

enum Mode { MODE_H6 = 0, MODE_CLAMP = 1, MODE_VANILLA = 2 };
const char* const kModeNames[] = { "h6", "clamp", "vanilla" };

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chSet, "set");
#ifdef DLB_PROBE
KP_CHANNEL(chProbe, "probe");
KP_CHANNEL(chProbeW4, "probe_w4");
#endif

// Only stores the path and the level (no I/O): safe from a static initialiser. Old core: same file as 1.0.1, same lines.
const bool g_fallbackSet = (KpLog_SetFallbackFile("dialogue_letterbox_fix_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

bool g_enabled = true;
bool g_log = false;            // log=0 default: load line, first APPLY, first SETTOP/SETBOTTOM, problem lines
#ifdef DLB_PROBE
bool g_probe = false;          // probe=1: the five PROBE handlers log; default off
#endif
int g_mode = MODE_H6;
int g_setLines = 0;
bool g_setCapNoted = false;
int g_hits = 0;                 // SetScreenSize hook hits
bool g_foreign = false;         // sticky: the constant was found holding someone else's value
uint8_t g_written[8];
bool g_haveWritten = false;
bool g_lateDone = false;
bool g_applyLogged = false;     // the first apply() call of this process is logged even with log=0
bool g_topLogged = false;       // same for the first SETTOP
bool g_bottomLogged = false;    // same for the first SETBOTTOM

// Content gate: a line is written when log=1 or when `force` (load line, problems, first-event lines). KpLog then applies
// its own channel/level filter on top.
#define DLB_LOG(force, ch, ...) \
    do { if (g_log || (force)) KP_LOGV(ch, KPLOG_INFO, __VA_ARGS__); } while (0)

// The load line (and the unknown-mode line) are formatted in DllMain but emitted later, once the logger is known:
// by KPatchInit after KpLog_Init succeeds, else by the first hook call (old core / no log service).
char g_loadLine[400];
char g_badModeLine[160];
volatile LONG g_loadEmitted = 0;
void emitLoadLine() {
    if (g_loadEmitted) return;                         // cheap fast path (hot after the first call)
    if (InterlockedExchange(&g_loadEmitted, 1)) return;
    KP_LOGV(chMain, KPLOG_INFO, "%s", g_loadLine);
    if (g_badModeLine[0]) KP_LOGV(chMain, KPLOG_WARN, "%s", g_badModeLine);
}

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
#ifdef DLB_PROBE
        else if (!_stricmp(key, "probe")) g_probe = flagValue(val);
#endif
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
    const bool first = !g_applyLogged;   // the first non-problem apply() call's lines are written even with log=0
    if (!g_enabled) { g_applyLogged = true; DLB_LOG(first, chMain, "APPLY src=%s W=%d H=%d: DISABLED, no write", src, w, h); return; }
    if (w <= 0 || h <= 0) { KP_LOGV(chMain, KPLOG_WARN, "APPLY src=%s W=%d H=%d: INVALID size, no write", src, w, h); return; }

    uint8_t cur[8];
    if (!readConst(cur)) { KP_LOGV(chMain, KPLOG_ERR, "APPLY src=%s W=%d H=%d: constant 0x%08x is not readable, no write", src, w, h, (unsigned)kConstVA); return; }
    char hx[40];
    const double vanilla = asDouble(kVanilla);
    const double curD = asDouble(cur);
    const bool isVanilla = sameBytes(cur, kVanilla);
    const bool isOurs = g_haveWritten && sameBytes(cur, g_written);
    if (!isVanilla && !isOurs) {
        hex8(cur, hx);
        g_foreign = true;
        KP_LOGV(chMain, KPLOG_WARN, "APPLY src=%s W=%d H=%d: FOREIGN value %s %.17g: another mod owns 0x%08x, not writing", src, w, h, hx, curD, (unsigned)kConstVA);
        return;
    }
    g_applyLogged = true;   // set only after the problem early-outs, so a first INVALID/FOREIGN call does not use up the slot

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
    DLB_LOG(first, chMain, "APPLY src=%s W=%d H=%d mode=%s old=%.17g new=%.17g vanillaBar=%d newBar=%d X=%d fs=%.1f panelH=%d replyY=%d",
            src, w, h, kModeNames[g_mode], curD, target, vBar, nBar, X, fs, panelH, replyY);

    if (sameBytes(cur, tb)) { DLB_LOG(first, chMain, "APPLY src=%s: unchanged (already %.17g)", src, curD); return; }

    DWORD old = 0;
    if (!VirtualProtect(reinterpret_cast<void*>(kConstVA), 8, PAGE_READWRITE, &old)) {
        KP_LOGV(chMain, KPLOG_ERR, "APPLY src=%s: VirtualProtect(RW) FAILED err=%lu, no write", src, GetLastError());
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
    DLB_LOG(first, chMain, "APPLY src=%s: wrote (protect RW ok, old prot=0x%lx; protect restore %s -> 0x%lx); verify %s, now %s = %.17g",
            src, old, back ? "ok" : "FAILED", back ? old : 0UL, ok ? "OK" : "MISMATCH", hx, rd ? asDouble(after) : 0.0);
    if (!back) KP_LOGV(chMain, KPLOG_ERR, "APPLY src=%s: VirtualProtect(restore) FAILED err=%lu", src, GetLastError());
    if (!ok) KP_LOGV(chMain, KPLOG_ERR, "APPLY src=%s: verify MISMATCH after write", src);
}

bool managerSize(const uint8_t* mgr, int* w, int* h) {
    if (!mgr) return false;
    *w = *reinterpret_cast<const int16_t*>(mgr + 0x6C);
    *h = *reinterpret_cast<const int16_t*>(mgr + 0x6E);
    return true;
}

void logLetterbox(const char* what, const uint8_t* lb, bool force) {
    const uint8_t* mgr = *reinterpret_cast<uint8_t* const*>(lb + 0x1C);
    int w = 0, h = 0;
    const bool haveMgr = managerSize(mgr, &w, &h);
    const int32_t* s = reinterpret_cast<const int32_t*>(lb + 0x04);
    const int32_t* t = reinterpret_cast<const int32_t*>(lb + 0x68);
    uint8_t cur[8];
    const bool rc = readConst(cur);
    if (g_setLines < kSetLogCap) {
        ++g_setLines;
        DLB_LOG(force, chSet, "%s lb=%p mode(+0x88, before this store)=%d start(x,y,w,h)=(%d,%d,%d,%d) target(x,y,w,h)=(%d,%d,%d,%d) mgr=%p W=%d H=%d c=%.17g",
                what, lb, *reinterpret_cast<const int32_t*>(lb + 0x88), s[0], s[1], s[2], s[3], t[0], t[1], t[2], t[3],
                mgr, w, h, rc ? asDouble(cur) : 0.0);
    } else if (!g_setCapNoted) {
        g_setCapNoted = true;
        DLB_LOG(false, chSet, "SETTOP/SETBOTTOM log cap (%d lines) reached, further lines dropped", kSetLogCap);
    }
    if (!g_lateDone && g_hits == 0 && haveMgr && w > 0 && h > 0) {
        g_lateDone = true;
        char src[40];
        sprintf(src, "LATE(%s)", what == nullptr ? "?" : (what[3] == 'T' ? "SetTop" : "SetBottom"));
        KP_LOGV(chMain, KPLOG_INFO, "%s: no SetScreenSize hit seen yet, applying late fallback (the bar just computed used the old constant)", what);
        apply(w, h, src);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// PROBE variant (DLB_PROBE, version 1.0.2-probe): log-only windows W1-W5 (engine_research_findings/V8b.md (c)). Every handler
// returns at once unless probe=1 (and log=1, and its KpLog channel is on). All game memory is read through IsBadReadPtr-
// guarded copies; nothing is written to the game; floats are logged as raw bits (no x87 use in the handlers; the KPM wrapper
// additionally FXSAVE/FXRSTORs the x87/SSE state).
// Rate limit (content logic the analyzer relies on: the n= and skip= fields): counters reset at every SETTOP; per window
// the first 120 records are logged every frame, then at most one line per second; global cap 3000 PROBE lines per process.
// Channels: probe (W1, W2, W3 + W3b, W5, and the PROBE cap note), probe_w4 (W4, filtered to the letterbox vtable).
// PROBE W3 is split into two lines so each stays under the core's 232-byte cut: "PROBE W3 n= skip= this= vtbl= flags= bit2=
// bit6= rect= W= H= predicate= dt=" and "PROBE W3b n= skip= alpha= color= mgr= DAT_a309b4= DAT_a309b8= DAT_a309bc=". W3b has
// the same n= and does not take a gate slot.
// ---------------------------------------------------------------------------------------------------------------------
#ifdef DLB_PROBE
constexpr uint32_t kLetterboxVtbl = 0x009A7CBC;   // CSWGuiDialogLetterbox vtable (V8b)
constexpr uintptr_t kDat309B4 = 0x00A309B4;       // int, written at 0x8BB467 (top-bar h)
constexpr uintptr_t kDat309B8 = 0x00A309B8;       // int, written at 0x8BB531 (bottom-bar y)
constexpr uintptr_t kDat309BC = 0x00A309BC;       // byte, written at 0x7CEB9C (arg == 1)
constexpr int kProbeFast = 120;
constexpr DWORD kProbeSlowMs = 1000;
constexpr int kProbeCap = 3000;

enum ProbeWin { PW1 = 0, PW2, PW3, PW4, PW5, PW_COUNT };
struct ProbeState { int logged; int skipped; DWORD last; };
ProbeState g_pw[PW_COUNT];
int g_probeLines = 0;
bool g_probeCapNoted = false;
CRITICAL_SECTION g_probeLock;   // guards g_pw / g_probeLines (the draw hooks can run on more than one thread)
bool g_probeLockInit = false;

// Guarded reader: a failed read sets ok = false and yields 0, so one flag marks the whole line.
struct Rd {
    bool ok = true;
    uint32_t u32(uintptr_t a) {
        uint32_t v = 0;
        if (a == 0 || IsBadReadPtr(reinterpret_cast<const void*>(a), 4)) { ok = false; return 0; }
        memcpy(&v, reinterpret_cast<const void*>(a), 4);
        return v;
    }
    int32_t i32(uintptr_t a) { return static_cast<int32_t>(u32(a)); }
    int16_t i16(uintptr_t a) {
        int16_t v = 0;
        if (a == 0 || IsBadReadPtr(reinterpret_cast<const void*>(a), 2)) { ok = false; return 0; }
        memcpy(&v, reinterpret_cast<const void*>(a), 2);
        return v;
    }
    uint8_t u8(uintptr_t a) {
        uint8_t v = 0;
        if (a == 0 || IsBadReadPtr(reinterpret_cast<const void*>(a), 1)) { ok = false; return 0; }
        v = *reinterpret_cast<const uint8_t*>(a);
        return v;
    }
};

void probeReset() {
    if (g_probeLockInit) EnterCriticalSection(&g_probeLock);
    memset(g_pw, 0, sizeof(g_pw));
    if (g_probeLockInit) LeaveCriticalSection(&g_probeLock);
}

// true = this record should be written. *skipped = records dropped for this window since its previous written line.
bool probeGate(ProbeWin w, int* skipped) {
    bool pass = false;
    bool noteCap = false;
    if (g_probeLockInit) EnterCriticalSection(&g_probeLock);
    if (g_probeLines < kProbeCap) {
        ProbeState& s = g_pw[w];
        const DWORD now = GetTickCount();
        if (s.logged < kProbeFast || now - s.last >= kProbeSlowMs) {
            pass = true;
            ++s.logged;
            s.last = now;
            *skipped = s.skipped;
            s.skipped = 0;
            ++g_probeLines;
        } else {
            ++s.skipped;
        }
    } else if (!g_probeCapNoted) {
        g_probeCapNoted = true;
        noteCap = true;
    }
    if (g_probeLockInit) LeaveCriticalSection(&g_probeLock);
    if (noteCap) DLB_LOG(false, chProbe, "PROBE cap (%d lines) reached, further PROBE lines dropped", kProbeCap);
    return pass;
}

// Base CSWGuiPanel::Draw (0x40EC90) validity predicate from V8b (a): bit 6 of [+0x48], x >= 0, y >= 0, w > 0, h > 0,
// x + w <= W, y + h <= H (W/H = shorts at [[this+0x1C]+0x6C/+0x6E]). Returns "WOULD-DRAW" or the first failing term.
const char* drawPredicate(Rd& rd, uintptr_t self, int* outW, int* outH) {
    const uint32_t flags = rd.u32(self + 0x48);
    const int32_t x = rd.i32(self + 0x04), y = rd.i32(self + 0x08), w = rd.i32(self + 0x0C), h = rd.i32(self + 0x10);
    const uintptr_t mgr = rd.u32(self + 0x1C);
    const int mw = mgr ? rd.i16(mgr + 0x6C) : 0;
    const int mh = mgr ? rd.i16(mgr + 0x6E) : 0;
    *outW = mw;
    *outH = mh;
    if (!rd.ok) return "UNREADABLE";
    if (mgr == 0) return "FAIL:mgr-null";
    if (!((flags >> 6) & 1)) return "FAIL:bit6";
    if (x < 0) return "FAIL:x>=0";
    if (y < 0) return "FAIL:y>=0";
    if (w <= 0) return "FAIL:w>0";
    if (h <= 0) return "FAIL:h>0";
    if (static_cast<int64_t>(x) + w > mw) return "FAIL:x+w<=W";
    if (static_cast<int64_t>(y) + h > mh) return "FAIL:y+h<=H";
    return "WOULD-DRAW";
}

// W1 / W2 common body: this = [ebp-0x34]; rect handed to SetExtent at [ebp+rectOff ..] (x,y,w,h); control rect [this+4..+0x10].
void probeSlide(ProbeWin w, const char* tag, void* ebp, int rectOff) {
    if (!g_probe || !g_log || ebp == nullptr) return;
    emitLoadLine();
    if (!KP_ENABLED(chProbe)) return;
    int skipped = 0;
    if (!probeGate(w, &skipped)) return;
    const uintptr_t fp = reinterpret_cast<uintptr_t>(ebp);
    Rd rd;
    const uintptr_t self = rd.u32(fp - 0x34);
    const int32_t px = rd.i32(fp + rectOff), py = rd.i32(fp + rectOff + 4), pw = rd.i32(fp + rectOff + 8), ph = rd.i32(fp + rectOff + 12);
    int32_t cx = 0, cy = 0, cw = 0, ch = 0, mode = 0, done = 0, th = 0;
    uint32_t alpha = 0;
    if (self) {
        cx = rd.i32(self + 0x04); cy = rd.i32(self + 0x08); cw = rd.i32(self + 0x0C); ch = rd.i32(self + 0x10);
        mode = rd.i32(self + 0x88); done = rd.i32(self + 0x8C); alpha = rd.u32(self + 0x50); th = rd.i32(self + 0x74);
    }
    DLB_LOG(false, chProbe, "PROBE %s n=%d skip=%d this=0x%08x mode=%d done=%d alpha=0x%08x passedRect(x,y,w,h)=(%d,%d,%d,%d) ctrlRect(x,y,w,h)=(%d,%d,%d,%d) targetH(+0x74)=%d%s",
            tag, g_pw[w].logged, skipped, static_cast<unsigned>(self), mode, done, alpha, px, py, pw, ph, cx, cy, cw, ch, th,
            (rd.ok && self) ? "" : " [UNREADABLE]");
}
#endif  // DLB_PROBE

}  // namespace

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def.
extern "C" void __cdecl KPatchInit(const KPatchApi* api) { if (KpLog_Init(api)) emitLoadLine(); }

// CSWGuiManager::SetScreenSize post-store hook (0x411AC1). ebp = the function frame; [ebp-0xC] = manager.
extern "C" void __cdecl dlbOnScreenSize(void* ebp) {
    emitLoadLine();
    if (ebp == nullptr) return;
    uint8_t* mgr = *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(ebp) - 0xC);
    if (mgr == nullptr) { KP_LOGV(chMain, KPLOG_WARN, "dlbOnScreenSize: manager pointer [ebp-0xC] is null, skipping"); return; }
    int w, h;
    managerSize(mgr, &w, &h);
    apply(w, h, "SetScreenSize");
}

// SetTop / SetBottom log hooks (0x8BAD7C / 0x8BAF01). lb = CSWGuiDialogLetterbox this (ECX).
extern "C" void __cdecl dlbLogSetTop(void* lb) {
    emitLoadLine();
    if (lb == nullptr) return;
#ifdef DLB_PROBE
    if (g_probe) probeReset();   // probe: a new dialogue starts, per-window rate limits restart
#endif
    const bool first = !g_topLogged;
    g_topLogged = true;
    if (!g_log && !first && (g_lateDone || g_hits != 0)) return;
    logLetterbox("SETTOP", static_cast<const uint8_t*>(lb), first);
}
extern "C" void __cdecl dlbLogSetBottom(void* lb) {
    emitLoadLine();
    if (lb == nullptr) return;
    const bool first = !g_bottomLogged;
    g_bottomLogged = true;
    if (!g_log && !first && (g_lateDone || g_hits != 0)) return;
    logLetterbox("SETBOTTOM", static_cast<const uint8_t*>(lb), first);
}

#ifdef DLB_PROBE
// ---- 1.0.2-probe handlers (all log-only; see the PROBE variant block above) ----

// W1 0x8BB461 (letterbox Draw, top bar, after SetExtent). ebp = Draw frame: this [ebp-0x34], rect passed [ebp-0x18..-0xC].
extern "C" void __cdecl dlbProbeW1(void* ebp) { probeSlide(PW1, "W1", ebp, -0x18); }

// W2 0x8BB52B (letterbox Draw, bottom bar, after SetExtent). this [ebp-0x34], rect passed [ebp-0x28..-0x1C].
extern "C" void __cdecl dlbProbeW2(void* ebp) { probeSlide(PW2, "W2", ebp, -0x28); }

// W3 0x8BB538 (letterbox Draw, before CALL 0x40EC90). this [ebp-0x34], dt (float bits) [ebp+8]. Steals an x87 FLD/FSTP pair:
// this handler does no floating-point work at all (bits only). Writes two lines (W3 + W3b) with the same n=.
extern "C" void __cdecl dlbProbeW3(void* ebp) {
    if (!g_probe || !g_log || ebp == nullptr) return;
    emitLoadLine();
    if (!KP_ENABLED(chProbe)) return;
    int skipped = 0;
    if (!probeGate(PW3, &skipped)) return;
    const uintptr_t fp = reinterpret_cast<uintptr_t>(ebp);
    Rd rd;
    const uintptr_t self = rd.u32(fp - 0x34);
    const uint32_t dtBits = rd.u32(fp + 8);
    if (!self) { DLB_LOG(false, chProbe, "PROBE W3 n=%d skip=%d this=NULL/UNREADABLE dt=0x%08x", g_pw[PW3].logged, skipped, dtBits); return; }
    const uint32_t vt = rd.u32(self);
    const uint32_t flags = rd.u32(self + 0x48);
    const int32_t x = rd.i32(self + 0x04), y = rd.i32(self + 0x08), w = rd.i32(self + 0x0C), h = rd.i32(self + 0x10);
    const uint32_t alpha = rd.u32(self + 0x50), c0 = rd.u32(self + 0x54), c1 = rd.u32(self + 0x58), c2 = rd.u32(self + 0x5C);
    const uintptr_t mgr = rd.u32(self + 0x1C);
    int mw = 0, mh = 0;
    const char* pred = drawPredicate(rd, self, &mw, &mh);
    const int32_t d4 = rd.i32(kDat309B4), d8 = rd.i32(kDat309B8);
    const int dc = rd.u8(kDat309BC);
    const int n = g_pw[PW3].logged;
    DLB_LOG(false, chProbe, "PROBE W3 n=%d skip=%d this=0x%08x vtbl=0x%08x%s flags(+0x48)=0x%08x bit2=%u bit6=%u rect(x,y,w,h)=(%d,%d,%d,%d) W=%d H=%d predicate=%s dt=0x%08x%s",
            n, skipped, static_cast<unsigned>(self), vt, vt == kLetterboxVtbl ? "(letterbox)" : "(OTHER)", flags,
            (flags >> 2) & 1, (flags >> 6) & 1, x, y, w, h, mw, mh, pred, dtBits,
            rd.ok ? "" : " [SOME-UNREADABLE]");
    DLB_LOG(false, chProbe, "PROBE W3b n=%d skip=%d alpha=0x%08x color(+0x54..)=0x%08x,0x%08x,0x%08x mgr=0x%08x DAT_a309b4=%d DAT_a309b8=%d DAT_a309bc=%d%s",
            n, skipped, alpha, c0, c1, c2, static_cast<unsigned>(mgr), d4, d8, dc,
            rd.ok ? "" : " [SOME-UNREADABLE]");
}

// W4 0x40ED9F (base CSWGuiPanel::Draw, after the fill call; HOT: every panel, every frame). this = [ebp-0x38]; eax = fill result;
// rect pushed to the fill at [ebp-0x10..-0x4] (x,y,w,h). Fast path: probe off -> return; this's vtable != the letterbox's -> return.
extern "C" void __cdecl dlbProbeW4(void* ebp, int eax) {
    if (!g_probe) return;
    if (ebp == nullptr) return;
    if (!g_log) return;
    emitLoadLine();
    if (!KP_ENABLED(chProbeW4)) return;
    const uintptr_t fp = reinterpret_cast<uintptr_t>(ebp);
    Rd rd;
    const uintptr_t self = rd.u32(fp - 0x38);
    if (!rd.ok || self == 0) return;
    const uint32_t vt = rd.u32(self);
    if (!rd.ok || vt != kLetterboxVtbl) return;
    int skipped = 0;
    if (!probeGate(PW4, &skipped)) return;
    const int32_t x = rd.i32(fp - 0x10), y = rd.i32(fp - 0x0C), w = rd.i32(fp - 0x08), h = rd.i32(fp - 0x04);
    const uint32_t alpha = rd.u32(self + 0x50);
    const int32_t mode = rd.i32(self + 0x88);
    DLB_LOG(false, chProbeW4, "PROBE W4 n=%d skip=%d this=0x%08x mode=%d eax=%d fillRect(x,y,w,h)=(%d,%d,%d,%d) alpha=0x%08x%s",
            g_pw[PW4].logged, skipped, static_cast<unsigned>(self), mode, eax, x, y, w, h, alpha, rd.ok ? "" : " [UNREADABLE]");
}

// W5 0x7CEB95 (CGuiInGame::SetGlobalDialogState). arg = [ebp+8] (1 = start, 0 = end), this = [ebp-0x24].
extern "C" void __cdecl dlbProbeW5(void* ebp) {
    if (!g_probe || !g_log || ebp == nullptr) return;
    emitLoadLine();
    if (!KP_ENABLED(chProbe)) return;
    int skipped = 0;
    if (!probeGate(PW5, &skipped)) return;
    const uintptr_t fp = reinterpret_cast<uintptr_t>(ebp);
    Rd rd;
    const int32_t arg = rd.i32(fp + 8);
    const uintptr_t self = rd.u32(fp - 0x24);
    DLB_LOG(false, chProbe, "PROBE W5 n=%d skip=%d arg=%d this=0x%08x%s", g_pw[PW5].logged, skipped, arg, static_cast<unsigned>(self), rd.ok ? "" : " [UNREADABLE]");
}
#endif  // DLB_PROBE

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
#ifdef DLB_PROBE
    InitializeCriticalSection(&g_probeLock);
    g_probeLockInit = true;
#endif
    readConfig();
    uint8_t cur[8] = { 0 };
    const bool rc = readConst(cur);
    char hx[40];
    hex8(cur, hx);
    // Format only; KPatchInit (or the first hook call) emits it, see emitLoadLine().
#ifdef DLB_PROBE
    snprintf(g_loadLine, sizeof(g_loadLine), "k2-dialogue-letterbox-fix %s loaded: enabled=%d mode=%s log=%d probe=%d const@0x%08x=%s (%.17g%s) pid=%lu",
             kVersion, g_enabled ? 1 : 0, kModeNames[g_mode], g_log ? 1 : 0, g_probe ? 1 : 0, (unsigned)kConstVA, hx, asDouble(cur),
             rc ? "" : ", UNREADABLE", GetCurrentProcessId());
#else
    snprintf(g_loadLine, sizeof(g_loadLine), "k2-dialogue-letterbox-fix %s loaded: enabled=%d mode=%s log=%d const@0x%08x=%s (%.17g%s) pid=%lu",
             kVersion, g_enabled ? 1 : 0, kModeNames[g_mode], g_log ? 1 : 0, (unsigned)kConstVA, hx, asDouble(cur),
             rc ? "" : ", UNREADABLE", GetCurrentProcessId());
#endif
    if (g_badMode[0]) snprintf(g_badModeLine, sizeof(g_badModeLine), "unknown mode '%s' in dialogue_letterbox_fix.ini, using h6", g_badMode);
    return TRUE;
}
