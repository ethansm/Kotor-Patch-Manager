// Load Hang Probe 0.1.11 (Steam Aspyr build 6A522E71...). Diagnostics; the 0.1.9 guard changes engine behaviour; "fix=auto" (default ON, "fix=off" disables) calls AppActivated.
// Version labels 1.2.0 to 1.4.1 in the comments below are internal phase labels (doc 09/10), not patch versions. Patch versions are 0.x.y (manifest.toml).
// manifest 0.1.10: guard-only mode without hangprobe_debug.txt (DR0 breakpoint alone, no hooks or watchdog); fires and a 5-minute hit heartbeat persist to freeze_guard_history.txt. 0.1.9: GUARD for the +aa949 infinite loop (negative arr[i]-arr[j] -> store +inf, resume at +aa980; hangprobe_nofix.txt disables). 0.1.8: DR0 execute breakpoint at the +aa916 division logs each pass; MAIN_STALL writes hangprobe_stall.dmp. 0.1.7: stall report also dumps the float array (struct at [ebp-0xfa8], +0x98, 225 floats, count word +0x3bc) that the +aa974 loop indexes.
// manifest 0.1.6: MAIN_STALL/LOADSCREEN report adds registers, a 0x120-byte frame dump and the float-loop decode (+aa974 spin), and tails cmdq-probe/queue-hud/action-hotbar/companion-ai logs.
// manifest 0.1.5: logging moved to KpLog.h (channel main, file load-hang-probe_log.txt; old core: hangprobe_log.txt); see logging section.
// 1.4.1 (doc 10 Phase 5A): RESTORE-AFTER-MOVIE / PLAYMOVIES-EXIT are logged only when their tuple changes (they run every frame from
// MainLoop ret 0x781DFA while the intro-movie state lingers: 1362 lines in incident 16); `calls=` = cumulative calls of that point.
// 1.4.0 (doc 10 Phase 4, T4/U6 probe-log points; ring slots only, logged as WINEV by the watchdog):
//   0x40B4AD WM_ACTIVATEAPP arrival in WndProc (hwnd, wParam, lParam, foreground pid == ours): lParam==0 with wParam==0 while the
//            foreground window is ours = the movie thread's synthetic deactivate (the H1 discriminator)
//   0x40C6B3 movie window saw a deactivate (movie thread)   0x40D637 movie thread sets [0xA1B76C]=1   0x40D692 synthetic PostMessage(main,0x1C,0,0)
//   0x40BF40 RestoreAfterMovie entry (a1b764/768/76c)        0x798B39 PlayMovies exit ([EBP-0x18], [EBP-0x20])   0x798A77 pump cut-off engaged
//   0x781E0B MainLoop load-screen keep/hide decision: one event per CHANGE of (a1b4e0, foreground-is-game) plus a per-frame count
// 1.3.0 (doc 09 s15): the load screen is dismissed only while the engine's app-active flag DAT_00A1B4E0 is set (WndProc
// 0x40A750 WM_ACTIVATEAPP -> AppActivated 0x40BCF0 / AppDeactivated 0x40BC40). New: WINEV/WIN log lines (window messages,
// activation calls with callers, foreground window vs game window, the activation-gate globals), report section, and an
// optional guarded fix: if the load screen has been up >= fixdelay ms with a1b4e0==0, area ready and the OS foreground window
// IS the game window, call the engine's own AppActivated once (max 5x per load, >= 2 s apart). hangprobe_debug.txt keys:
//   fix=auto (default: ON since 0.1.11; fix=off disables)   fixdelay=6000 (ms)   io=0. Manual causality test: touch hangprobe_activate.txt in the game dir.
// Investigation: patch_manager_mods/09_load_hang_investigation.md sections 9-10 (local-setup/KOTOR-II worktree).
//
// 0.1.11: no SuspendThread sampling while DR0 is armed; fix=auto default on (fix=off disables), gate uses "foreground is ours" and skips an iconic window;
// the 0x40A750 WndProc hook moved to k2-activation-fix 0.3.0 (hook set changed: REINSTALL).
// 0.1.10: without "hangprobe_debug.txt" only the GUARD runs (guardOnly: DR0 breakpoint, no hooks, no watchdog, no kplog writes); its
// fires and a 5-minute hit-count heartbeat go to freeze_guard_history.txt, which kplog rotation never touches.
// With "hangprobe_debug.txt" present, the full probe below runs. Then:
//   * hook 0x00781BE0 (CClientExoAppInternal::MainLoop entry, ECX = client internal): frame heartbeat,
//     remembers the main thread id, the internal pointer and the main thread's stack base;
//   * a watchdog thread polls the load-screen state every 250 ms (plain reads, no suspend) and only when
//     something already looks wrong (load screen up > 5 s while frames run, no frame for 5 s, or a manual
//     trigger) samples the main thread every 250 ms (Suspend / GetThreadContext / stack walk
//     that touches only [ESP, StackBase) / Resume) and reads the load-screen state fields every tick;
//   * load-hang-probe_log.txt (KpLog channel main; hangprobe_log.txt on an old core): load-state CHANGES (one line each) + a heartbeat line every 60 s;
//   * hangprobe_report.txt (rewritten every 5 s while a condition holds): stack histogram with
//     module+offset, load state, failed-probe ring, loaded modules, tails of the other mods' logs.
//     Conditions: LOADSCREEN_STUCK (CSWGuiLoadScreen modal while frames keep running), MAIN_STALL
//     (no frame for 15 s and one stack dominating), MANUAL (touch hangprobe_dump.txt in the game dir).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ProbeLogic.h"
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "../Common/SafeRead.h"
#include "../Common/CodePatch.h"
#include "../Common/GameAddr.h"
#include "../Common/KpLog.h"

KP_LOG_PATCH("load-hang-probe");

namespace {

using saferead::readAt;

// ---- engine layout (Ghidra, 09 section 9d) ------------------------------------------------------------
constexpr uintptr_t AppGlobal = 0x00A1B4A4;          // app; client = *(app+4); internal = *(client+4)
constexpr uintptr_t LoadScreenVtable = 0x009A5C84;   // CSWGuiLoadScreen
constexpr uintptr_t FlagA1B4E0 = gameaddr::AppActiveFlag;   // tested next to HideLoadScreen in MainLoop
constexpr int In_Flag18 = 0x18, In_PlayerId = 0x20, In_Sub48 = 0x48, In_Movie = 0x130, In_GuiMgr = 0x274,
              In_LoadScreen = 0x278, In_AreaReady = 0x288, In_PendingSwap = 0x338, In_PlayerCacheId = 0x3BC,
              In_PlayerCachePtr = 0x3C0;
constexpr int Sub48_Flag = 0x268;
constexpr int App_ModuleState = 0x14;                // *(app+0x14) -> [0]==1, [1] in {1,3} sets +0x288
constexpr int Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;
// activation machinery (Ghidra, doc 09 s15)
constexpr uintptr_t GameHwndG = 0x00A1B484, GateD4 = 0x00A1B4D4, GateBC = 0x00A1B4BC, GateC0 = 0x00A1B4C0, GateTick = 0x00A1B6F4,
                    Gate768 = 0x00A1B768, Gate76C = gameaddr::MovieRestoreSkipFlag, Gate42A0 = 0x009F42A0, InitFlag = 0x00A1B48C;
constexpr uintptr_t FnAppActivated = 0x0040BCF0;   // void __cdecl (void): sets a1b4e0=1 when it was 0

// ---- config -------------------------------------------------------------------------------------------
constexpr DWORD SampleMs = 250;           // poll + (when armed) sample interval
constexpr DWORD ArmLoadMs = 10000;        // arm sampling: load screen up this long ...
constexpr LONG ArmLoadFrames = 120;       // ... with at least this many frames rendered under it
constexpr DWORD ArmStallMs = 5000;        // arm sampling: no MainLoop call this long
constexpr DWORD ManualSampleMs = 2500;    // manual trigger: sample this long, then report
constexpr int Window = 240;                // samples kept (60 s)
constexpr int MaxFrames = 20;              // EBP-chain depth
constexpr int ScanHits = 12;               // extra return-address candidates from a raw stack scan
constexpr DWORD LoadStuckMs = 20000;       // load screen modal this long ...
constexpr LONG LoadStuckFrames = 600;      // ... while at least this many frames ran
constexpr DWORD StallMs = 15000;           // no MainLoop call this long ...
constexpr int StallDominance = 90;         // ... and one stack signature in >= this % of the last 10 s
constexpr DWORD ReportEveryMs = 5000;
constexpr int MaxLogLines = 20000;

// ---- state written by the hook (main thread) ---------------------------------------------------------
volatile LONG g_frames = 0;
volatile DWORD g_mainTid = 0;
volatile uintptr_t g_internal = 0, g_stackBase = 0, g_stackAlloc = 0;   // stack = [g_stackAlloc, g_stackBase)
volatile DWORD g_lastFrameTick = 0;
bool g_on = false;
HMODULE g_self = nullptr;

// ms since the main thread's last MainLoop call; signed so a tick written after `now` was read gives <= 0.
LONG msSinceFrame(DWORD now) { DWORD last = g_lastFrameTick; return last ? static_cast<LONG>(now - last) : 0; }

// ---- logging (watchdog thread + DllMain) -------------------------------------------------------------------
// 0.1.5: lines go through KpLog.h (channel main): new core -> load-hang-probe_log.txt (`T=<s.ms> f=- load-hang-probe/main f<frame> ...`),
// old core -> fallback hangprobe_log.txt at INFO. hangprobe_report.txt stays a dedicated artifact. The core cuts a message at 232 bytes
// ("[...]"), so a long line is split at spaces into pieces of <= 220 bytes (continuations start with `+ `). DllMain logs before KpLog_Init has
// run: those lines are queued (g_early) and emitted by flushEarly() from KPatchInit (new core) or by the watchdog (old core, after <= 1.5 s).
KP_CHANNEL(chMain, "main");
const bool g_fallbackSet = (KpLog_SetFallbackFile("hangprobe_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

constexpr int LogPiece = 220, EarlyMax = 8, EarlyLen = 400;
int g_logLines = 0;
volatile LONG g_logReady = 0, g_initDone = 0, g_flushing = 0, g_earlyN = 0;
char g_early[EarlyMax][EarlyLen];

void emit(const char* s) {
    int n = static_cast<int>(strlen(s));
    if (n <= LogPiece) { KP_LOG(chMain, "%s", s); return; }
    char piece[LogPiece + 1];
    int pos = 0; bool first = true;
    while (pos < n) {
        int len = n - pos < LogPiece - 2 ? n - pos : LogPiece - 2;   // room for the `+ ` marker
        if (pos + len < n) {   // not the last piece: prefer to cut at a space
            int cut = len;
            while (cut > LogPiece / 2 && s[pos + cut] != ' ') --cut;
            if (s[pos + cut] == ' ') len = cut;
        }
        if (first) { memcpy(piece, s + pos, len); piece[len] = 0; }
        else { piece[0] = '+'; piece[1] = ' '; memcpy(piece + 2, s + pos, len); piece[len + 2] = 0; }
        KP_LOG(chMain, "%s", piece);
        first = false; pos += len;
        if (pos < n && s[pos] == ' ') ++pos;
    }
}

// Emits the queued pre-init lines once, then opens normal logging. Safe to call from both KPatchInit and the watchdog.
void flushEarly() {
    if (InterlockedCompareExchange(&g_flushing, 1, 0) != 0) { while (!g_logReady) Sleep(1); return; }   // the loser waits: its next logf must not be queued after the snapshot
    LONG n = g_earlyN; if (n > EarlyMax) n = EarlyMax;
    for (LONG i = 0; i < n; ++i) emit(g_early[i]);
    g_logReady = 1;
}

void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    char buf[1100];
    int k = snprintf(buf, sizeof buf, "f%ld ", g_frames);
    if (k < 0) k = 0;
    va_list ap; va_start(ap, fmt); vsnprintf(buf + k, sizeof buf - k, fmt, ap); va_end(ap);
    ++g_logLines;
    if (!g_logReady) {
        LONG i = InterlockedIncrement(&g_earlyN) - 1;
        if (i < EarlyMax) { strncpy(g_early[i], buf, EarlyLen - 1); g_early[i][EarlyLen - 1] = 0; }
        return;
    }
    emit(buf);
}

// ---- main-thread samples --------------------------------------------------------------------------------
// 0.1.6: reg = eax ecx edx ebx esi edi ebp eflags at the sample; frame[] = the stack words [frameLo, frameLo + 4*nf) around EBP
// (locals below it, the saved EBP / return address above), clamped to [ESP, StackBase). Lets a spin loop's own variables be read.
constexpr int FrameWords = 72;               // 0x120 bytes: EBP-0x100 .. EBP+0x20
struct Sample { DWORD tick; LONG frames; uintptr_t eip, esp; int n; uintptr_t ret[MaxFrames]; int ns; uintptr_t scan[ScanHits];
                uint32_t reg[8]; uintptr_t frameLo; int nf; uint32_t frame[FrameWords]; };
Sample g_s[Window];
int g_sHead = 0, g_sCount = 0;

uintptr_t g_exeLo = 0x00400000, g_exeHi = 0;   // swkotor2.exe image, for the raw scan filter

bool inExe(uintptr_t a) { return a >= g_exeLo + 0x1000 && a < g_exeHi; }

// Suspends the main thread, copies registers, walks the stack with plain reads bounded by
// [ESP, StackBase) (no API call while suspended), resumes. Returns false if anything failed.
bool sampleMain(HANDLE th, Sample* s) {
    uintptr_t base = g_stackBase, alloc = g_stackAlloc;
    if (!base || !alloc) return false;
    if (SuspendThread(th) == (DWORD)-1) return false;
    CONTEXT ctx; memset(&ctx, 0, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
    bool ok = GetThreadContext(th, &ctx) != 0;
    if (ok) {
        uintptr_t esp = ctx.Esp, ebp = ctx.Ebp;
        s->eip = ctx.Eip; s->esp = esp; s->n = 0; s->ns = 0; s->nf = 0; s->frameLo = 0;
        s->reg[0] = ctx.Eax; s->reg[1] = ctx.Ecx; s->reg[2] = ctx.Edx; s->reg[3] = ctx.Ebx;
        s->reg[4] = ctx.Esi; s->reg[5] = ctx.Edi; s->reg[6] = ctx.Ebp; s->reg[7] = ctx.EFlags;
        if (esp >= alloc && esp < base) {           // [esp, base) is committed: the thread has used it
            if (ebp >= esp && ebp < base && !(ebp & 3)) {   // 0.1.6: innermost frame words, plain reads inside [esp, base)
                uintptr_t lo = ebp >= esp + 0x100 ? ebp - 0x100 : esp, hi = ebp + 0x20 < base ? ebp + 0x20 : base;
                if (hi > lo) {
                    int nf = static_cast<int>((hi - lo) / 4); if (nf > FrameWords) nf = FrameWords;
                    for (int i = 0; i < nf; ++i) s->frame[i] = *reinterpret_cast<const uint32_t*>(lo + 4u * i);
                    s->frameLo = lo; s->nf = nf;
                }
            }
            for (int i = 0; i < MaxFrames; ++i) {
                if (ebp < esp || ebp + 8 > base || (ebp & 3)) break;
                uintptr_t next = *reinterpret_cast<const uintptr_t*>(ebp), ret = *reinterpret_cast<const uintptr_t*>(ebp + 4);
                if (!ret) break;
                s->ret[s->n++] = ret;
                if (next <= ebp) break;
                ebp = next;
            }
            uintptr_t end = esp + 0x2000 < base ? esp + 0x2000 : base;
            for (uintptr_t p = esp; p + 4 <= end && s->ns < ScanHits; p += 4) {
                uintptr_t v = *reinterpret_cast<const uintptr_t*>(p);
                if (inExe(v)) s->scan[s->ns++] = v;
            }
        }
    }
    ResumeThread(th);
    return ok;
}

// ---- load state (read without suspending; torn ints are harmless for diagnostics) ---------------------
struct LoadState {
    uintptr_t internal, ls, lsVt, mgr; int lsModal, modalCount;
    int flag18, playerId, areaReady, pendingSwap, cacheId, cachePtr, sub48Flag, movie, a1b4e0, modA, modB, modMode;
    int gstate, lastPlayerSrv, p18, p14, f324;   // 1.2.0: client state machine (+9c), +28c, +18/+14 pointers, +324
};

bool operator!=(const LoadState& a, const LoadState& b) { return memcmp(&a, &b, sizeof(a)) != 0; }

LoadState readLoadState() {
    saferead::beginScope();
    LoadState st; memset(&st, 0, sizeof(st));
    uintptr_t app = 0, client = 0, internal = g_internal;
    readAt(1, AppGlobal, 0, &app);
    if (!internal && readAt(2, app, 4, &client)) readAt(3, client, 4, &internal);
    st.internal = internal;
    if (!internal) return st;
    readAt(10, internal, In_LoadScreen, &st.ls);
    if (st.ls) readAt(11, st.ls, 0, &st.lsVt);
    readAt(12, internal, In_GuiMgr, &st.mgr);
    if (st.mgr) {
        uintptr_t modals = 0; readAt(13, st.mgr, Mgr_Modals, &modals); readAt(14, st.mgr, Mgr_ModalCount, &st.modalCount);
        if (st.ls && modals && st.modalCount > 0 && st.modalCount < 64)
            for (int i = 0; i < st.modalCount; ++i) { uintptr_t p = 0; if (readAt(15, modals, i * 4, &p) && p == st.ls) st.lsModal = 1; }
    }
    readAt(20, internal, In_Flag18, &st.flag18);
    readAt(21, internal, In_PlayerId, &st.playerId);
    readAt(22, internal, In_AreaReady, &st.areaReady);
    readAt(23, internal, In_PendingSwap, &st.pendingSwap);
    readAt(24, internal, In_PlayerCacheId, &st.cacheId);
    readAt(25, internal, In_PlayerCachePtr, &st.cachePtr);
    { uintptr_t sub = 0; if (readAt(26, internal, In_Sub48, &sub) && sub) readAt(27, sub, Sub48_Flag, &st.sub48Flag); else st.sub48Flag = -1; }
    { uintptr_t mv = 0; readAt(28, internal, In_Movie, &mv); st.movie = mv != 0; }
    readAt(29, FlagA1B4E0, 0, &st.a1b4e0);
    readAt(40, internal, 0x9C, &st.gstate); readAt(41, internal, 0x28C, &st.lastPlayerSrv);
    { uintptr_t v = 0; readAt(42, internal, 0x18, &v); st.p18 = (int)v; v = 0; readAt(43, internal, 0x14, &v); st.p14 = (int)v; }
    readAt(44, internal, 0x324, &st.f324);
    { uintptr_t m = 0; if (readAt(30, app, App_ModuleState, &m) && m) { readAt(31, m, 0, &st.modA); readAt(32, m, 4, &st.modB); readAt(33, m, 0x38, &st.modMode); } }
    return st;
}

int fmtState(char* b, int cap, const LoadState& s) {
    return snprintf(b, cap,
        "internal=%08x ls=%08x lsVt=%08x%s lsModal=%d modals=%d | +18=%d playerId=%08x areaReady(+288)=%d pendingSwap(+338)=%08x "
        "cache(+3bc/+3c0)=%08x/%08x sub48flag=%d movie=%d a1b4e0=%d | module state [0]=%d [1]=%d [0x38]=%d | gstate(+9c)=%d +28c=%08x +18=%08x +14=%08x +324=%d",
        (unsigned)s.internal, (unsigned)s.ls, (unsigned)s.lsVt, s.ls && s.lsVt != LoadScreenVtable ? "(!)" : "", s.lsModal,
        s.modalCount, s.flag18, (unsigned)s.playerId, s.areaReady, (unsigned)s.pendingSwap, (unsigned)s.cacheId,
        (unsigned)s.cachePtr, s.sub48Flag, s.movie, s.a1b4e0, s.modA, s.modB, s.modMode,
        s.gstate, (unsigned)s.lastPlayerSrv, (unsigned)s.p18, (unsigned)s.p14, s.f324);
}


// ---- 1.2.0 event rings: SetPlayerId calls + file I/O (hooks write ring slots only; the watchdog flushes) ----
constexpr uintptr_t SetPlayerRets[] = { 0x0078237C, 0x00785453, 0x0079EE47, 0x007B3107, 0x0073F926 };   // expected callers (doc 09 s12)
struct SetPlayerEv { DWORD tick; LONG frames; DWORD tid; uintptr_t ret; unsigned oldId, newId; };
constexpr int SpRing = 64;
SetPlayerEv g_sp[SpRing]; volatile LONG g_spHead = 0; LONG g_spFlushed = 0;

struct IoEv { DWORD tick, tid; char kind; char ok; unsigned short pad; HANDLE h; DWORD a, b, err; char name[40]; };
constexpr int IoRing = 2048;
IoEv g_io[IoRing]; volatile LONG g_ioHead = 0;
struct HSlot { HANDLE volatile h; char name[40]; };
constexpr int HSlots = 1024;
HSlot g_hs[HSlots];
IoEv g_ioFail[64]; volatile LONG g_ioFailHead = 0;
volatile LONG g_opens = 0, g_openFnf = 0, g_openFail = 0, g_reads = 0, g_readFail = 0, g_shortReads = 0, g_seeks = 0;
volatile LONG64 g_readBytes = 0;
volatile DWORD g_lastIoTick = 0; char g_lastIoName[40]; volatile char g_ioOn = 0;

// ---- 1.3.0 window / activation events (hooks write ring slots only; the watchdog flushes them to the log) ----
struct WinEv { DWORD tick; LONG frames; unsigned kind; uintptr_t a, b, c, d, ret; HWND fg; int flag; LONG calls; };   // kind: 0 WNDMSG, 1 ACTIVATED, 2 DEACTIVATED, 3..10 = 1.4.0 points (winKindName)
constexpr int WinRing = 256;
WinEv g_win[WinRing]; volatile LONG g_winHead = 0; LONG g_winFlushed = 0;
char g_fixMsg[32][200]; volatile LONG g_fixHead = 0; LONG g_fixFlushed = 0;   // main-thread notes about the fix, flushed by the watchdog
bool g_fixAuto = true; DWORD g_fixDelayMs = 6000;
volatile LONG g_reqActivate = 0;       // set by the watchdog when hangprobe_activate.txt appears; consumed on the main thread
DWORD g_fixSince = 0, g_fixLastTry = 0; int g_fixTries = 0; bool g_fixNoted = false;   // main thread only
inline int gflag() { return *reinterpret_cast<volatile int*>(FlagA1B4E0); }
void fixNote(const char* fmt, ...) {
    LONG i = InterlockedIncrement(&g_fixHead) - 1;
    va_list ap; va_start(ap, fmt); vsnprintf(g_fixMsg[i & 31], 200, fmt, ap); va_end(ap);
}
const char* msgName(unsigned m) {
    switch (m) { case 0x05: return "WM_SIZE"; case 0x06: return "WM_ACTIVATE"; case 0x07: return "WM_SETFOCUS"; case 0x08: return "WM_KILLFOCUS";
        case 0x18: return "WM_SHOWWINDOW"; case 0x1c: return "WM_ACTIVATEAPP"; case 0x21: return "WM_MOUSEACTIVATE"; case 0x46: return "WM_WINDOWPOSCHANGING";
        case 0x47: return "WM_WINDOWPOSCHANGED"; case 0x7e: return "WM_DISPLAYCHANGE"; case 0x86: return "WM_NCACTIVATE"; case 0x112: return "WM_SYSCOMMAND";
        case 0x10: return "WM_CLOSE"; default: return "WM_?"; }
}
inline WinEv* winSlot() { LONG i = InterlockedIncrement(&g_winHead) - 1; return &g_win[i & (WinRing - 1)]; }
const char* winKindName(unsigned k) {
    switch (k) { case 3: return "ACTIVATEAPP-ARRIVE(0x40B4AD)"; case 4: return "MOVIEWND-DEACTIVATE(0x40C6B3)"; case 5: return "MOVIE-SETS-76C(0x40D637)";
        case 6: return "MOVIE-SYNTH-POST-1C(0x40D692)"; case 7: return "RESTORE-AFTER-MOVIE(0x40BF40)"; case 8: return "PLAYMOVIES-EXIT(0x798B39)";
        case 9: return "PUMP-CUTOFF(0x798A77)"; case 10: return "LS-DECISION-CHANGE(0x781E0B)"; default: return "?"; }
}
inline bool fgIsOurs(HWND fg) { DWORD pid = 0; if (fg) GetWindowThreadProcessId(fg, &pid); return pid == GetCurrentProcessId(); }
volatile LONG g_lsDecisions = 0; int g_lsLastA = -1, g_lsLastFg = -1;   // 0x781E0B (main thread only)
// 1.4.1 change-only state for RestoreAfterMovie (a764, a768, a76c, caller, a1b4e0) and PlayMovies exit (v18, v20, a1b4e0); main thread only.
volatile LONG g_restoreCalls = 0, g_exitCalls = 0; uintptr_t g_restoreLast[5]; uintptr_t g_exitLast[3]; bool g_restoreSeen = false, g_exitSeen = false;
bool tupleChanged(uintptr_t* last, const uintptr_t* now, int n, bool& seen) {
    if (seen && memcmp(last, now, n * sizeof(uintptr_t)) == 0) return false;
    memcpy(last, now, n * sizeof(uintptr_t)); seen = true; return true;
}

struct WinState { uintptr_t hwnd; unsigned fgIsGame, iconic, visible; int a1b4e0, d4, bc, c0, f768, f76c, f42a0, init; DWORD gateTick; DWORD fgPid; };
bool operator!=(const WinState& a, const WinState& b) { return memcmp(&a, &b, sizeof(a)) != 0; }
WinState readWinState(HWND* fgOut) {
    WinState w; memset(&w, 0, sizeof(w));
    HWND h = *reinterpret_cast<HWND volatile*>(GameHwndG); HWND fg = GetForegroundWindow();
    w.hwnd = reinterpret_cast<uintptr_t>(h); w.fgIsGame = h && fg == h; w.iconic = h && IsIconic(h); w.visible = h && IsWindowVisible(h);
    w.a1b4e0 = gflag(); w.d4 = *reinterpret_cast<volatile int*>(GateD4); w.bc = *reinterpret_cast<volatile int*>(GateBC); w.c0 = *reinterpret_cast<volatile int*>(GateC0);
    w.f768 = *reinterpret_cast<volatile int*>(Gate768); w.f76c = *reinterpret_cast<volatile int*>(Gate76C); w.f42a0 = *reinterpret_cast<volatile int*>(Gate42A0);
    w.init = *reinterpret_cast<volatile int*>(InitFlag); w.gateTick = *reinterpret_cast<volatile DWORD*>(GateTick);
    if (fg) GetWindowThreadProcessId(fg, &w.fgPid);
    if (fgOut) *fgOut = fg;
    return w;
}
void fgDesc(HWND fg, char* b, int cap) {
    char t[80] = ""; DWORD pid = 0;
    if (fg) {   // GetWindowText on a window of THIS process sends WM_GETTEXT and would block the watchdog while the main thread is stuck: title only for other processes
        GetWindowThreadProcessId(fg, &pid);
        if (pid != GetCurrentProcessId()) GetWindowTextA(fg, t, sizeof(t)); else strcpy(t, "(this process)");
    }
    snprintf(b, cap, "%p pid=%lu '%s'", fg, pid, t);
}

typedef HANDLE(WINAPI* CreateFileA_t)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI* CreateFileW_t)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL(WINAPI* ReadFile_t)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef DWORD(WINAPI* SetFilePointer_t)(HANDLE, LONG, PLONG, DWORD);
typedef BOOL(WINAPI* CloseHandle_t)(HANDLE);
CreateFileA_t rCreateFileA; CreateFileW_t rCreateFileW; ReadFile_t rReadFile; SetFilePointer_t rSetFilePointer; CloseHandle_t rCloseHandle;

inline unsigned hslot(HANDLE h) { return (reinterpret_cast<uintptr_t>(h) >> 2) & (HSlots - 1); }
HSlot* hfind(HANDLE h) {
    for (unsigned i = 0, k = hslot(h); i < 8; ++i, k = (k + 1) & (HSlots - 1)) if (g_hs[k].h == h) return &g_hs[k];
    return nullptr;
}
void hset(HANDLE h, const char* name) {
    unsigned k = hslot(h);
    for (unsigned i = 0; i < 8; ++i, k = (k + 1) & (HSlots - 1)) if (g_hs[k].h == h || !g_hs[k].h) break;
    HSlot& s = g_hs[k]; s.h = nullptr;
    size_t n = strlen(name); const char* t = n > 39 ? name + n - 39 : name;   // keep the tail (file name)
    strncpy(s.name, t, 39); s.name[39] = 0; s.h = h;
}
inline IoEv* ioSlot() { LONG i = InterlockedIncrement(&g_ioHead) - 1; return &g_io[i & (IoRing - 1)]; }
void ioNote(IoEv* e, const char* name) { g_lastIoTick = e->tick; strncpy(g_lastIoName, name, 39); g_lastIoName[39] = 0; }
void ioFail(const IoEv& e) { LONG i = InterlockedIncrement(&g_ioFailHead) - 1; g_ioFail[i & 63] = e; }

void recordOpen(const char* name, HANDLE h, DWORD access, DWORD disp, DWORD err) {
    InterlockedIncrement(&g_opens);
    bool ok = h != INVALID_HANDLE_VALUE;
    if (!ok && (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)) { InterlockedIncrement(&g_openFnf); return; }   // engine probes many absent paths
    IoEv* e = ioSlot(); e->tick = GetTickCount(); e->tid = GetCurrentThreadId(); e->kind = ok ? 'O' : 'o'; e->ok = ok; e->h = ok ? h : nullptr;
    e->a = access; e->b = disp; e->err = err;
    size_t n = strlen(name); const char* t = n > 39 ? name + n - 39 : name; strncpy(e->name, t, 39); e->name[39] = 0;
    if (ok) hset(h, name); else { InterlockedIncrement(&g_openFail); ioFail(*e); }
    ioNote(e, e->name);
}
HANDLE WINAPI hkCreateFileA(LPCSTR n, DWORD a, DWORD sh, LPSECURITY_ATTRIBUTES sa, DWORD d, DWORD f, HANDLE t) {
    HANDLE h = rCreateFileA(n, a, sh, sa, d, f, t); DWORD err = GetLastError();
    if (n && g_ioOn) recordOpen(n, h, a, d, err);
    SetLastError(err); return h;
}
HANDLE WINAPI hkCreateFileW(LPCWSTR n, DWORD a, DWORD sh, LPSECURITY_ATTRIBUTES sa, DWORD d, DWORD f, HANDLE t) {
    HANDLE h = rCreateFileW(n, a, sh, sa, d, f, t); DWORD err = GetLastError();
    if (n && g_ioOn) {
        char nm[260]; int i = 0; for (; n[i] && i < 259; ++i) nm[i] = n[i] < 128 ? static_cast<char>(n[i]) : '?'; nm[i] = 0;
        recordOpen(nm, h, a, d, err);
    }
    SetLastError(err); return h;
}
BOOL WINAPI hkReadFile(HANDLE h, LPVOID buf, DWORD n, LPDWORD got, LPOVERLAPPED ov) {
    BOOL ok = rReadFile(h, buf, n, got, ov); DWORD err = ok ? 0 : GetLastError();
    if (g_ioOn) {
        InterlockedIncrement(&g_reads);
        HSlot* s = hfind(h);
        if (s) {
            DWORD g = (ok && got && !ov) ? *got : 0;
            InterlockedExchangeAdd64(&g_readBytes, g);
            IoEv* e = ioSlot(); e->tick = GetTickCount(); e->tid = GetCurrentThreadId(); e->kind = 'R'; e->ok = ok != 0; e->h = h; e->a = n; e->b = g; e->err = err;
            memcpy(e->name, s->name, 40);
            if (!ok) { InterlockedIncrement(&g_readFail); ioFail(*e); }
            else if (!ov && g < n && n) { InterlockedIncrement(&g_shortReads); e->kind = 'r'; ioFail(*e); }
            ioNote(e, e->name);
        }
    }
    if (!ok) SetLastError(err);
    return ok;
}
DWORD WINAPI hkSetFilePointer(HANDLE h, LONG lo, PLONG hi, DWORD m) {
    DWORD r = rSetFilePointer(h, lo, hi, m); DWORD err = GetLastError();
    if (g_ioOn) {
        InterlockedIncrement(&g_seeks);
        HSlot* s = hfind(h);
        if (s) { IoEv* e = ioSlot(); e->tick = GetTickCount(); e->tid = GetCurrentThreadId(); e->kind = 'S'; e->ok = r != INVALID_SET_FILE_POINTER; e->h = h; e->a = lo; e->b = m; e->err = err; memcpy(e->name, s->name, 40); ioNote(e, e->name); }
    }
    SetLastError(err); return r;
}
BOOL WINAPI hkCloseHandle(HANDLE h) {
    if (g_ioOn) {
        HSlot* s = hfind(h);
        if (s) { IoEv* e = ioSlot(); e->tick = GetTickCount(); e->tid = GetCurrentThreadId(); e->kind = 'C'; e->ok = 1; e->h = h; e->a = e->b = e->err = 0; memcpy(e->name, s->name, 40); s->h = nullptr; }
    }
    return rCloseHandle(h);
}

const char* funcName(uintptr_t a) { return ""; }   // names are resolved offline (steam_functions.tsv)

void dumpIo(FILE* f, DWORD now, int n) {
    fprintf(f, "\nfile I/O (1.2.0; IAT hooks): opens=%ld (absent-file probes %ld, other open failures %ld) reads=%ld bytes=%lld read-fail=%ld short-reads=%ld seeks=%ld\n",
        g_opens, g_openFnf, g_openFail, g_reads, (long long)g_readBytes, g_readFail, g_shortReads, g_seeks);
    fprintf(f, "  last I/O %ld ms ago on '%s'\n", g_lastIoTick ? static_cast<LONG>(now - g_lastIoTick) : -1, g_lastIoName);
    LONG head = g_ioHead; LONG cnt = head < IoRing ? head : IoRing; LONG from = head - (cnt < n ? cnt : n);
    fprintf(f, "  last %ld events, oldest first (age ms, tid, kind O=open o=open-fail R=read r=short-read S=seek C=close; a,b = size,got | access,disp | pos,method; err):\n", head - from);
    for (LONG i = from; i < head; ++i) {
        const IoEv& e = g_io[i & (IoRing - 1)];
        fprintf(f, "    -%6ld t%-4lu %c%s h=%p a=%lu b=%lu err=%lu %s\n", static_cast<LONG>(now - e.tick), e.tid, e.kind, e.ok ? " " : "!", e.h, e.a, e.b, e.err, e.name);
    }
    LONG fh = g_ioFailHead; LONG fc = fh < 64 ? fh : 64;
    fprintf(f, "  failures / short reads / non-FNF open failures, last %ld:\n", fc);
    for (LONG i = fh - fc; i < fh; ++i) { const IoEv& e = g_ioFail[i & 63]; fprintf(f, "    -%6ld t%-4lu %c a=%lu b=%lu err=%lu %s\n", static_cast<LONG>(now - e.tick), e.tid, e.kind, e.a, e.b, e.err, e.name); }
}

void dumpWin(FILE* f, DWORD now) {
    HWND fg = nullptr; WinState w = readWinState(&fg); char d[160]; fgDesc(fg, d, sizeof(d));
    fprintf(f, "\nwindow / activation (1.3.0): game hwnd=%08x fgIsGame=%u iconic=%u visible=%u a1b4e0(active)=%d gate d4=%d bc=%d c0=%d 768=%d 76c=%d 42a0=%d init=%d lastDeactTick=%lu; foreground=%s; fix=%s delay=%lums tries=%d\n",
        (unsigned)w.hwnd, w.fgIsGame, w.iconic, w.visible, w.a1b4e0, w.d4, w.bc, w.c0, w.f768, w.f76c, w.f42a0, w.init, w.gateTick, d, g_fixAuto ? "auto" : "off", g_fixDelayMs, g_fixTries);
    LONG head = g_winHead; LONG cnt = head < WinRing ? head : WinRing; if (cnt > 80) cnt = 80;
    fprintf(f, "  last %ld window events, oldest first (age ms, frames):\n", cnt);
    for (LONG i = head - cnt; i < head; ++i) { const WinEv& e = g_win[i & (WinRing - 1)];
        if (e.kind == 0) fprintf(f, "    -%6ld f%ld %s(%02x) hwnd=%08x wp=%08x lp=%08x fg=%p a1b4e0=%d\n", static_cast<LONG>(now - e.tick), e.frames, msgName((unsigned)e.b), (unsigned)e.b, (unsigned)e.a, (unsigned)e.c, (unsigned)e.d, e.fg, e.flag);
        else fprintf(f, "    -%6ld f%ld %s ret=%08x arg=%08x before=%d fg=%p\n", static_cast<LONG>(now - e.tick), e.frames, e.kind == 1 ? "APP-ACTIVATED" : "APP-DEACTIVATED", (unsigned)e.ret, (unsigned)e.a, e.flag, e.fg); }
}

void dumpSetPlayer(FILE* f, DWORD now) {
    LONG head = g_spHead; LONG cnt = head < SpRing ? head : SpRing;
    fprintf(f, "\nSetPlayerId (0x00798660) calls, %ld total, last %ld (age ms, frames, tid, caller ret, old -> new; caller is one of 78237C/785453/79EE47/7B3107/73F926 if the hook parameters are right):\n", head, cnt);
    for (LONG i = head - cnt; i < head; ++i) { const SetPlayerEv& e = g_sp[i % SpRing]; fprintf(f, "    -%6ld f%ld t%lu ret=%08x  %08x -> %08x\n", static_cast<LONG>(now - e.tick), e.frames, e.tid, (unsigned)e.ret, e.oldId, e.newId); }
}

// ---- report ---------------------------------------------------------------------------------------------
struct Mod { char name[64]; uintptr_t lo, hi; };
Mod g_mods[256]; int g_modCount = 0;

void snapshotModules() {
    g_modCount = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
    if (snap == INVALID_HANDLE_VALUE) return;
    MODULEENTRY32 me; me.dwSize = sizeof(me);
    for (BOOL ok = Module32First(snap, &me); ok && g_modCount < 256; ok = Module32Next(snap, &me)) {
        Mod& m = g_mods[g_modCount++];
        strncpy(m.name, me.szModule, sizeof(m.name) - 1); m.name[sizeof(m.name) - 1] = 0;
        m.lo = reinterpret_cast<uintptr_t>(me.modBaseAddr); m.hi = m.lo + me.modBaseSize;
    }
    CloseHandle(snap);
}

const char* where(uintptr_t a, char* buf, int cap) {
    for (int i = 0; i < g_modCount; ++i)
        if (a >= g_mods[i].lo && a < g_mods[i].hi) { snprintf(buf, cap, "%s+%x", g_mods[i].name, (unsigned)(a - g_mods[i].lo)); return buf; }
    snprintf(buf, cap, "?"); return buf;
}

struct Count { uintptr_t addr; int n; };
int addCount(Count* c, int n, int cap, uintptr_t a) {
    for (int i = 0; i < n; ++i) if (c[i].addr == a) { ++c[i].n; return n; }
    if (n < cap) c[n++] = { a, 1 };
    return n;
}
void sortCounts(Count* c, int n) {
    for (int i = 1; i < n; ++i) for (int j = i; j > 0 && c[j].n > c[j - 1].n; --j) { Count t = c[j]; c[j] = c[j - 1]; c[j - 1] = t; }
}

void tailFile(FILE* out, const char* path, int lines) {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(out, "  (%s: absent)\n", path); return; }
    fseek(f, 0, SEEK_END); long size = ftell(f);
    long from = size > 16384 ? size - 16384 : 0;
    fseek(f, from, SEEK_SET);
    static char buf[16385]; size_t got = fread(buf, 1, 16384, f); buf[got] = 0; fclose(f);
    int seen = 0; char* p = buf + got;
    while (p > buf && seen <= lines) if (*--p == '\n') ++seen;
    fprintf(out, "  --- tail of %s (%ld bytes) ---\n%s\n", path, size, p == buf ? buf : p + 1);
}

// signature = first 3 ebp-chain return addresses (eip if the chain is empty)
uintptr_t sig(const Sample& s) { return s.n ? (s.ret[0] ^ (s.n > 1 ? s.ret[1] * 3 : 0) ^ (s.n > 2 ? s.ret[2] * 7 : 0)) : s.eip; }

int dominance(DWORD now, DWORD spanMs) {
    Count c[64]; int n = 0, total = 0;
    for (int k = 0; k < g_sCount; ++k) {
        const Sample& s = g_s[(g_sHead - 1 - k + Window) % Window];
        if (now - s.tick > spanMs) break;
        n = addCount(c, n, 64, sig(s)); ++total;
    }
    if (total < 8) return 0;
    sortCounts(c, n);
    return c[0].n * 100 / total;
}

// 0.1.6: stack word of the newest sample at EBP+off (off multiple of 4), false when outside the captured window.
bool frameWord(const Sample& s, int off, uint32_t* out) {
    const intptr_t idx = (static_cast<intptr_t>(s.reg[6]) + off - static_cast<intptr_t>(s.frameLo)) / 4;
    if (s.nf <= 0 || idx < 0 || idx >= s.nf) return false;
    *out = s.frame[idx];
    return true;
}
float bitsToFloat(uint32_t b) { float f; memcpy(&f, &b, 4); return f; }

// Newest sample's innermost frame, plus the float-step loop decode: the main thread spun at +aa974..aa9b5 in all four 11-11 freezes
// (`[ebp-0xdc] += *0x9f6524` vs double 1.0 @0x986b50; `[ebp-0x14] += *0x9f6524` vs float @0xa04a10). Never ends if a local is huge/NaN or the step is 0.
void dumpFrame(FILE* f) {
    if (g_sCount <= 0) return;
    const Sample& s = g_s[(g_sHead - 1 + Window) % Window];
    if (s.nf <= 0) { fprintf(f, "\nframe dump: none (ebp %08x outside [esp %08x, stack base))\n", (unsigned)s.reg[6], (unsigned)s.esp); return; }
    fprintf(f, "\nframe dump of the newest sample (ebp=%08x, offsets from ebp; locals below, saved ebp/ret at +0/+4):\n", (unsigned)s.reg[6]);
    for (int i = 0; i < s.nf; i += 4) {
        const int off = static_cast<int>(s.frameLo + 4u * i) - static_cast<int>(s.reg[6]);
        fprintf(f, "  ebp%c%03x:", off < 0 ? '-' : '+', off < 0 ? -off : off);
        for (int j = i; j < i + 4 && j < s.nf; ++j) fprintf(f, " %08x", (unsigned)s.frame[j]);
        fputc('\n', f);
    }
    if (s.eip >= 0x004AA900 && s.eip < 0x004AB000) {
        uint32_t a = 0, b = 0;
        const bool ha = frameWord(s, -0xdc, &a), hb = frameWord(s, -0x14, &b);
        fprintf(f, "float loop locals (eip %08x): [ebp-0xdc]=%s%g (%08x)  [ebp-0x14]=%s%g (%08x)\n",
                (unsigned)s.eip, ha ? "" : "n/a ", ha ? (double)bitsToFloat(a) : 0.0, (unsigned)a,
                hb ? "" : "n/a ", hb ? (double)bitsToFloat(b) : 0.0, (unsigned)b);
        saferead::beginScope();
        uint32_t step = 0, lim = 0, dlo = 0, dhi = 0;
        const bool g1 = readAt(50, 0x009F6524, 0, &step), g2 = readAt(51, 0x00A04A10, 0, &lim);
        const bool g3 = readAt(52, 0x00986B50, 0, &dlo) && readAt(53, 0x00986B50, 4, &dhi);
        double dbound = 0.0; if (g3) { uint64_t u = (static_cast<uint64_t>(dhi) << 32) | dlo; memcpy(&dbound, &u, 8); }
        fprintf(f, "float loop globals: step@9f6524=%s%g (%08x, initial 0.001)  limit@a04a10=%s%g (%08x)  bound@986b50=%s%g\n",
                g1 ? "" : "n/a ", g1 ? (double)bitsToFloat(step) : 0.0, (unsigned)step,
                g2 ? "" : "n/a ", g2 ? (double)bitsToFloat(lim) : 0.0, (unsigned)lim, g3 ? "" : "n/a ", dbound);
        // 0.1.6b: the float array the loop indexes. Frame layout of the spinning function: struct pointer at [ebp-0xfa8] (outside the frame dump),
        // key indices i=[ebp-0x40] j=[ebp-0xd4] (words), loop counter [ebp-0x38], float array at struct+0x98 (225 floats), count word at struct+0x3bc.
        uint32_t p = 0, wi = 0, wj = 0, wk = 0;
        const bool hp = readAt(54, s.reg[6], -0xfa8, &p);
        const bool hi = frameWord(s, -0x40, &wi), hj = frameWord(s, -0xd4, &wj), hk = frameWord(s, -0x38, &wk);
        fprintf(f, "float array owner: ptr@[ebp-0xfa8]=%s%08x  i=[ebp-0x40]=%s%d j=[ebp-0xd4]=%s%d counter=[ebp-0x38]=%s%d\n",
                hp ? "" : "n/a ", (unsigned)p, hi ? "" : "n/a ", (int)(int16_t)wi, hj ? "" : "n/a ", (int)(int16_t)wj, hk ? "" : "n/a ", (int)wk);
        if (hp && p) {
            uint32_t vt = 0; uint16_t cnt = 0; char b[160];
            const bool hv = readAt(55, p, 0, &vt), hc = readAt(56, p, 0x3bc, &cnt);
            fprintf(f, "  struct %08x: vtable=%s%08x (%s) count@+3bc=%s%u\n", (unsigned)p, hv ? "" : "n/a ", (unsigned)vt, hv ? where(vt, b, sizeof(b)) : "-", hc ? "" : "n/a ", (unsigned)cnt);
            fprintf(f, "  floats at +0x98 (index: value/hex), 8 per row:\n");
            for (int i = 0; i < 225; i += 8) {
                fprintf(f, "   [%3d]", i);
                for (int j = i; j < i + 8 && j < 225; ++j) {
                    uint32_t v = 0; const bool ok = readAt(57, p, 0x98 + 4 * j, &v);
                    if (ok) fprintf(f, " %g/%08x", (double)bitsToFloat(v), (unsigned)v); else fprintf(f, " n/a");
                }
                fputc('\n', f);
            }
        }
    }
}

// ---- 0.1.8: division-site capture (hardware execute breakpoint, no code patch) -----------------------------
// The 0.1.7 dump showed the stored step [ebp-0xdc] = -2097152 is NOT 0.001/(arr[i]-arr[j]) for the i/j alive at the stall, so the value was
// written on an earlier pass. DR0 is set (execute, length 1) on the instruction that starts that division (+aa916). A vectored handler on the
// main thread records ebp, the struct pointer, i, j, arr[i], arr[j] and the count for each pass into a ring; the stall report prints the last
// DivRing passes with the computed quotient, so the pass that produced -2097152 shows its inputs. Disabled by hangprobe_nodr.txt; the handler
// switches itself off after DivMaxHits passes. Whether Wine delivers the debug-register trap is unverified: the log line "DR arm" shows the
// read-back, the report shows the hit count (0 hits at a stall = not delivered, or the site is not on that path).
constexpr uintptr_t DivSite = 0x004AA916;
constexpr int DivRing = 64;
constexpr LONG DivMaxHits = 2000000000;   // 0.1.9: effectively never; the guard below needs the breakpoint for the whole session
// 0.1.9 GUARD (on unless hangprobe_nofix.txt exists): the +aa949 loop adds d = 0.001/(arr[i]-arr[j]) to an accumulator until it exceeds 1.0, so a
// negative difference (arr[i] < arr[j]; freezes 5-7: 0 vs 0.01) never exits (stall values -2097152 / 32768 are float32 fixed points). When that
// pass starts, store +inf in the accumulator [ebp-0xdc] and resume at the loop check +aa980: the engine then takes the exit it already takes for
// every arr[i] == arr[j] pass (d = +inf). Pure decision in divNeedsGuard (unit-testable); FPU stack is empty at both +aa916 and +aa980.
constexpr uintptr_t DivExit = 0x004AA980;
inline bool divNeedsGuard(float fi, float fj) { return fi < fj; }   // false for NaN
volatile LONG g_divFixed = 0;
volatile uint32_t g_divLastP = 0, g_divLastIJ = 0, g_divLastFi = 0, g_divLastFj = 0;   // last guarded pass, for the watchdog's log line
bool g_divFix = true;
struct DivHit { DWORD tick; LONG n; uint32_t ebp, p, fi, fj; int16_t i, j; uint16_t cnt; uint8_t okp, oki, fixed; };
DivHit g_div[DivRing];
volatile LONG g_divHits = 0;
bool g_drArmed = false;
DWORD g_guardTid = 0;   // 0.1.10: main thread id captured in DllMain (guard-only mode has no MainLoop hook)

// 0.1.10: persisted guard history, next to the exe. Its name is not *_log.txt, so the kplog rotation (current + one previous session) never
// clears it. Written by the guard thread only, never by the exception handler.
void histLine(const char* fmt, ...) {
    FILE* f = fopen("freeze_guard_history.txt", "a");
    if (!f) return;
    SYSTEMTIME st; GetLocalTime(&st);
    fprintf(f, "%04u-%02u-%02u %02u:%02u:%02u pid=%lu ", (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay, (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond, (unsigned long)GetCurrentProcessId());
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f);
    fclose(f);
}

// Stack bounds, as HangProbeMainLoop records them. The handler runs on the main thread, so the TIB it reads is the right one.
void initStackBounds() {
    NT_TIB* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
    g_stackBase = reinterpret_cast<uintptr_t>(tib->StackBase);
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(&mbi, &mbi, sizeof(mbi)) == sizeof(mbi)) g_stackAlloc = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
}

LONG CALLBACK divVeh(EXCEPTION_POINTERS* ep) {
    if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP) return EXCEPTION_CONTINUE_SEARCH;
    CONTEXT* c = ep->ContextRecord;
    if (c->Eip != DivSite) return EXCEPTION_CONTINUE_SEARCH;
    const LONG n = InterlockedIncrement(&g_divHits);
    DivHit& h = g_div[(n - 1) % DivRing];
    memset(&h, 0, sizeof(h));
    h.tick = GetTickCount(); h.n = n; h.ebp = c->Ebp;
    if (!g_stackBase) initStackBounds();   // guard-only mode: no MainLoop hook has set them
    const uintptr_t ebp = c->Ebp, lo = g_stackAlloc, hi = g_stackBase;
    if (lo && hi && ebp >= lo + 0xfb0 && ebp + 4 <= hi && !(ebp & 3)) {
        h.p = *reinterpret_cast<const uint32_t*>(ebp - 0xfa8);
        h.i = static_cast<int16_t>(*reinterpret_cast<const uint32_t*>(ebp - 0x40));
        h.j = static_cast<int16_t>(*reinterpret_cast<const uint32_t*>(ebp - 0xd4));
        h.okp = h.p && !IsBadReadPtr(reinterpret_cast<const void*>(static_cast<uintptr_t>(h.p)), 0x3c0);
        if (h.okp) {
            h.cnt = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(h.p) + 0x3bc);
            if (h.i >= 0 && h.i < 225 && h.j >= 0 && h.j < 225) {
                h.fi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(h.p) + 0x98 + 4u * h.i);
                h.fj = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(h.p) + 0x98 + 4u * h.j);
                h.oki = 1;
                if (g_divFix && divNeedsGuard(bitsToFloat(h.fi), bitsToFloat(h.fj))) {
                    *reinterpret_cast<uint32_t*>(ebp - 0xdc) = 0x7F800000u;   // +inf
                    c->Eip = DivExit;
                    h.fixed = 1; g_divLastP = h.p; g_divLastIJ = (static_cast<uint32_t>(static_cast<uint16_t>(h.i)) << 16) | static_cast<uint16_t>(h.j); g_divLastFi = h.fi; g_divLastFj = h.fj; InterlockedIncrement(&g_divFixed);
                }
            }
        }
    }
    c->EFlags |= 0x10000;   // RF: do not re-trigger on the same instruction
    c->Dr6 = 0;
    if (n >= DivMaxHits) c->Dr7 = 0;
    return EXCEPTION_CONTINUE_EXECUTION;
}

void armDivBreakpoint(HANDLE th) {
    if (g_drArmed || !th) return;
    g_drArmed = true;
    g_divFix = GetFileAttributesA("hangprobe_nofix.txt") == INVALID_FILE_ATTRIBUTES;
    if (GetFileAttributesA("hangprobe_nodr.txt") != INVALID_FILE_ATTRIBUTES) { logf("DR arm skipped (hangprobe_nodr.txt)"); return; }
    if (!AddVectoredExceptionHandler(1, divVeh)) { logf("DR arm FAILED: AddVectoredExceptionHandler"); return; }
    if (SuspendThread(th) == (DWORD)-1) { logf("DR arm FAILED: SuspendThread err=%lu", GetLastError()); return; }
    CONTEXT c; memset(&c, 0, sizeof(c)); c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    bool got = GetThreadContext(th, &c) != 0, set = false;
    if (got) {
        c.Dr0 = DivSite; c.Dr7 = (c.Dr7 & ~0x000F0003u) | 1u;   // L0 on, RW0=00 (execute), LEN0=00
        set = SetThreadContext(th, &c) != 0;
    }
    DWORD err = GetLastError();
    CONTEXT r; memset(&r, 0, sizeof(r)); r.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    bool back = GetThreadContext(th, &r) != 0;
    ResumeThread(th);
    logf("DR arm guard=%d site=%08x get=%d set=%d err=%lu readback=%d dr0=%08x dr7=%08x", g_divFix ? 1 : 0, (unsigned)DivSite, got, set, err, back, back ? (unsigned)r.Dr0 : 0u, back ? (unsigned)r.Dr7 : 0u);
}

// 0.1.10 guard-only mode (hangprobe_debug.txt absent): one thread, no hooks, no per-frame work. Arms DR0 on the main thread after 3 s,
// then polls every 250 ms: each guard fire and a 5-minute heartbeat (total hits, guarded passes) go to freeze_guard_history.txt.
// hangprobe_nofix.txt turns the guard off and the breakpoint is not armed at all.
DWORD WINAPI guardOnly(LPVOID) {
    Sleep(3000);
    if (GetFileAttributesA("hangprobe_nofix.txt") != INVALID_FILE_ATTRIBUTES) { histLine("guard off (hangprobe_nofix.txt): no breakpoint"); return 0; }
    HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, g_guardTid);
    histLine("guard-only mode 0.1.10: main thread %lu %s", (unsigned long)g_guardTid, th ? "opened" : "OPEN FAILED");
    if (th) armDivBreakpoint(th);
    histLine("DR0 armed=%d guard=%d site=%08x (kplog load-hang-probe_log.txt has the readback)", g_drArmed ? 1 : 0, g_divFix ? 1 : 0, (unsigned)DivSite);
    LONG seen = 0;
    DWORD beat = GetTickCount();
    for (;;) {
        Sleep(250);
        const LONG nf = g_divFixed;
        if (nf != seen) {
            histLine("GUARD fired: %ld total (+%ld) struct=%08x i=%d j=%d arr[i]=%g arr[j]=%g hits=%ld", (long)nf, (long)(nf - seen), (unsigned)g_divLastP,
                     (int)(int16_t)(g_divLastIJ >> 16), (int)(int16_t)(g_divLastIJ & 0xFFFF), (double)bitsToFloat(g_divLastFi), (double)bitsToFloat(g_divLastFj), (long)g_divHits);
            seen = nf;
        }
        if (GetTickCount() - beat >= 300000) { histLine("heartbeat: hits=%ld guarded=%ld", (long)g_divHits, (long)nf); beat = GetTickCount(); }
    }
    return 0;
}

void dumpDivRing(FILE* f) {
    const LONG total = g_divHits;
    fprintf(f, "\ndivision-site breakpoint (+aa916, 0.1.9): armed=%d guard=%d guarded=%ld hits=%ld (ring = last %d passes, oldest first; quotient = float32 0.001/(fi-fj))\n", g_drArmed ? 1 : 0, g_divFix ? 1 : 0, (long)g_divFixed, total, DivRing);
    const LONG first = total > DivRing ? total - DivRing + 1 : 1;
    for (LONG n = first; n <= total; ++n) {
        const DivHit h = g_div[(n - 1) % DivRing];
        if (h.n != n) continue;   // slot being rewritten
        const float fi = bitsToFloat(h.fi), fj = bitsToFloat(h.fj), d = fi - fj;
        const float q = h.oki ? 0.001f / d : 0.0f;
        fprintf(f, "  #%ld t=%lu ebp=%08x struct=%08x%s i=%d j=%d cnt=%u arr[i]=%g/%08x arr[j]=%g/%08x diff=%g quotient=%g%s\n", n, (unsigned long)h.tick, (unsigned)h.ebp, (unsigned)h.p,
                h.okp ? "" : "(unreadable)", (int)h.i, (int)h.j, (unsigned)h.cnt, (double)fi, (unsigned)h.fi, (double)fj, (unsigned)h.fj, (double)d, (double)q,
                h.fixed ? "  <== GUARDED (skipped the infinite loop)" : (h.oki && (q < -1e5f || q != q || q > 1e5f)) ? "  <== HUGE" : "");
    }
}

// Full process minidump of the stalled game (dbghelp loaded at run time; no link dependency). Once per launch.
// type = MiniDumpWithDataSegs | WithIndirectlyReferencedMemory | WithProcessThreadData (stacks + heap the stacks point at).
void writeMiniDump() {
    static bool done = false;
    if (done) return;
    done = true;
    if (GetFileAttributesA("hangprobe_nodump.txt") != INVALID_FILE_ATTRIBUTES) { logf("MINIDUMP skipped (hangprobe_nodump.txt)"); return; }
    typedef BOOL (WINAPI *MiniDumpFn)(HANDLE, DWORD, HANDLE, DWORD, void*, void*, void*);
    HMODULE dh = LoadLibraryA("dbghelp.dll");
    MiniDumpFn fn = dh ? reinterpret_cast<MiniDumpFn>(GetProcAddress(dh, "MiniDumpWriteDump")) : nullptr;
    if (!fn) { logf("MINIDUMP unavailable (dbghelp=%p)", (void*)dh); return; }
    HANDLE hf = CreateFileA("hangprobe_stall.dmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hf == INVALID_HANDLE_VALUE) { logf("MINIDUMP create failed err=%lu", GetLastError()); return; }
    const DWORD t0 = GetTickCount();
    BOOL ok = fn(GetCurrentProcess(), GetCurrentProcessId(), hf, 0x1 | 0x40 | 0x100, nullptr, nullptr, nullptr);
    DWORD err = GetLastError();
    LARGE_INTEGER sz; sz.QuadPart = 0; GetFileSizeEx(hf, &sz);
    CloseHandle(hf);
    logf("MINIDUMP hangprobe_stall.dmp ok=%d err=%lu size=%lld bytes in %lu ms", ok, err, (long long)sz.QuadPart, GetTickCount() - t0);
}

void writeReport(const char* reason, DWORD now, const LoadState& st) {
    snapshotModules();
    FILE* f = fopen("hangprobe_report.txt", "w");
    if (!f) return;
    char b[160], line[1024];
    fprintf(f, "hangprobe report: %s at tick %lu, frames=%ld (last MainLoop %ld ms ago), mainTid=%lu\n",
            reason, now, g_frames, msSinceFrame(now), g_mainTid);
    fmtState(line, sizeof(line), st); fprintf(f, "load state: %s\n\n", line);

    Count eips[128], rets[256]; int ne = 0, nr = 0, total = 0;
    for (int k = 0; k < g_sCount; ++k) {
        const Sample& s = g_s[(g_sHead - 1 - k + Window) % Window];
        if (now - s.tick > 30000) break;
        ++total;
        ne = addCount(eips, ne, 128, s.eip & ~0xFu);
        uintptr_t seen[MaxFrames + ScanHits]; int nseen = 0;
        for (int i = 0; i < s.n; ++i) { bool dup = false; for (int j = 0; j < nseen; ++j) dup |= seen[j] == s.ret[i]; if (!dup) { seen[nseen++] = s.ret[i]; nr = addCount(rets, nr, 256, s.ret[i]); } }
        for (int i = 0; i < s.ns; ++i) { bool dup = false; for (int j = 0; j < nseen; ++j) dup |= seen[j] == s.scan[i]; if (!dup) { seen[nseen++] = s.scan[i]; nr = addCount(rets, nr, 256, s.scan[i]); } }
    }
    sortCounts(eips, ne); sortCounts(rets, nr);
    fprintf(f, "main thread, last 30 s: %d samples\n  top EIPs (16-byte buckets):\n", total);
    for (int i = 0; i < ne && i < 12; ++i) fprintf(f, "    %3d%%  %08x  %s\n", eips[i].n * 100 / (total ? total : 1), (unsigned)eips[i].addr, where(eips[i].addr, b, sizeof(b)));
    fprintf(f, "  return addresses on the stack (share of samples containing it; ebp chain + raw scan):\n");
    for (int i = 0; i < nr && i < 40; ++i) fprintf(f, "    %3d%%  %08x  %s\n", rets[i].n * 100 / (total ? total : 1), (unsigned)rets[i].addr, where(rets[i].addr, b, sizeof(b)));
    fprintf(f, "  last 3 samples:\n");
    for (int k = 0; k < g_sCount && k < 3; ++k) {
        const Sample& s = g_s[(g_sHead - 1 - k + Window) % Window];
        fprintf(f, "    t=%lu f=%ld eip=%08x (%s) esp=%08x\n      chain:", s.tick, s.frames, (unsigned)s.eip, where(s.eip, b, sizeof(b)), (unsigned)s.esp);
        for (int i = 0; i < s.n; ++i) fprintf(f, " %08x", (unsigned)s.ret[i]);
        fprintf(f, "\n      scan: ");
        for (int i = 0; i < s.ns; ++i) fprintf(f, " %08x", (unsigned)s.scan[i]);
        fprintf(f, "\n      regs:  eax=%08x ecx=%08x edx=%08x ebx=%08x esi=%08x edi=%08x ebp=%08x efl=%08x\n",
                (unsigned)s.reg[0], (unsigned)s.reg[1], (unsigned)s.reg[2], (unsigned)s.reg[3],
                (unsigned)s.reg[4], (unsigned)s.reg[5], (unsigned)s.reg[6], (unsigned)s.reg[7]);
    }
    dumpFrame(f);
    dumpDivRing(f);
    static char ring[8192]; saferead::dump(ring, sizeof(ring));
    fprintf(f, "\nprobe's own %s", ring);
    dumpWin(f, now);
    dumpSetPlayer(f, now);
    dumpIo(f, now, 150);
    fprintf(f, "\nmodules:\n");
    for (int i = 0; i < g_modCount; ++i) fprintf(f, "  %08x-%08x %s\n", (unsigned)g_mods[i].lo, (unsigned)g_mods[i].hi, g_mods[i].name);
    fprintf(f, "\nother mods' logs:\n");
    // 0.1.6: the old names (force_power_hotbar, buffhud, inv3d) are stale; tail the logs that show the events before a combat/queue freeze.
    tailFile(f, "cmdq-probe_log.txt", 30);
    tailFile(f, "queue-hud_log.txt", 30);
    tailFile(f, "action-hotbar_log.txt", 20);
    tailFile(f, "companion-ai_log.txt", 20);
    tailFile(f, "k2-frame-pacing_log.txt", 8);
    tailFile(f, "load-hang-probe_log.txt", 60);
    tailFile(f, "hangprobe_log.txt", 60);
    fclose(f);
}

DWORD WINAPI watchdog(LPVOID) {
    for (int w = 0; w < 30 && !g_initDone; ++w) Sleep(50);   // old core: no KPatchInit, give up after 1.5 s
    flushEarly();
    HANDLE th = nullptr;
    LoadState last; memset(&last, 0, sizeof(last));
    DWORD lsSince = 0, lastReport = 0, lastBeat = GetTickCount();
    LONG lsFrames = 0;
    struct { LONG opens, reads; LONG64 bytes; LONG fails; } lsIoMark = { 0, 0, 0, 0 };
    DWORD nextTl = 0;
    WinState lastWin; memset(&lastWin, 0, sizeof(lastWin)); bool firstWin = true;
    bool firstState = true, armed = false, samplingSkippedNoted = false;
    DWORD manualUntil = 0, armedSince = 0;
    int armedSamples = 0;
    for (;;) {
        Sleep(SampleMs);
        DWORD now = GetTickCount();
        if (!th && g_mainTid) {
            th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, g_mainTid);
            logf("main thread %lu %s, internal=%08x stackBase=%08x stackAlloc=%08x", g_mainTid, th ? "opened" : "OPEN FAILED", (unsigned)g_internal, (unsigned)g_stackBase, (unsigned)g_stackAlloc);
            if (th) { CloseHandle(th); th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, g_mainTid); armDivBreakpoint(th); }
        }
        LoadState st = readLoadState();
        if (firstState || st != last) {
            char line[1024]; fmtState(line, sizeof(line), st);
            logf("STATE %s", line);
            last = st; firstState = false;
        }
        bool lsUp = st.ls && st.lsVt == LoadScreenVtable && st.lsModal;
        {   // 1.3.0: window / activation state and events
            HWND fg = nullptr; WinState ws = readWinState(&fg);
            if (firstWin || ws != lastWin) {
                char d[160]; fgDesc(fg, d, sizeof(d));
                logf("WIN game=%08x fgIsGame=%u iconic=%u visible=%u | a1b4e0(active)=%d | gate d4=%d bc=%d c0=%d 768=%d 76c=%d 42a0=%d init=%d lastDeact=%lu | fg=%s",
                     (unsigned)ws.hwnd, ws.fgIsGame, ws.iconic, ws.visible, ws.a1b4e0, ws.d4, ws.bc, ws.c0, ws.f768, ws.f76c, ws.f42a0, ws.init, ws.gateTick, d);
                lastWin = ws; firstWin = false;
            }
            while (g_winFlushed < g_winHead) {
                LONG i = g_winFlushed++;
                if (g_winHead - i > WinRing) continue;
                const WinEv& e = g_win[i & (WinRing - 1)];
                if (e.kind == 0) logf("WINEV f%ld %s(%02x) hwnd=%08x wp=%08x lp=%08x fg=%p a1b4e0=%d", e.frames, msgName((unsigned)e.b), (unsigned)e.b, (unsigned)e.a, (unsigned)e.c, (unsigned)e.d, e.fg, e.flag);
                else if (e.kind <= 2) logf("WINEV f%ld %s ret=%08x arg=%08x a1b4e0(before)=%d fg=%p", e.frames, e.kind == 1 ? "APP-ACTIVATED(0x40BCF0)" : "APP-DEACTIVATED(0x40BC40)", (unsigned)e.ret, (unsigned)e.a, e.flag, e.fg);
                else if (e.kind == 3) logf("WINEV f%ld tid=%lu %s hwnd=%08x wp=%08x lp=%08x fgOurs=%u a1b4e0=%d fg=%p%s", e.frames, (unsigned long)e.d, winKindName(e.kind),
                          (unsigned)e.a, (unsigned)e.b, (unsigned)e.ret, (unsigned)e.c, e.flag, e.fg, (e.b == 0 && e.ret == 0 && e.c) ? " <- SYNTHETIC DEACTIVATE while foreground is ours" : "");
                else if (e.calls) logf("WINEV f%ld tid=%lu %s a=%08x b=%08x c=%08x ret=%08x a1b4e0=%d fg=%p calls=%ld", e.frames, (unsigned long)e.d, winKindName(e.kind),
                          (unsigned)e.a, (unsigned)e.b, (unsigned)e.c, (unsigned)e.ret, e.flag, e.fg, e.calls);
                else logf("WINEV f%ld tid=%lu %s a=%08x b=%08x c=%08x ret=%08x a1b4e0=%d fg=%p", e.frames, (unsigned long)e.d, winKindName(e.kind),
                          (unsigned)e.a, (unsigned)e.b, (unsigned)e.c, (unsigned)e.ret, e.flag, e.fg);
            }
            while (g_fixFlushed < g_fixHead) { LONG i = g_fixFlushed++; if (g_fixHead - i > 32) continue; logf("FIX %s", g_fixMsg[i & 31]); }
            if (GetFileAttributesA("hangprobe_activate.txt") != INVALID_FILE_ATTRIBUTES) { DeleteFileA("hangprobe_activate.txt"); logf("manual activate requested (a1b4e0=%d, fgIsGame=%u)", ws.a1b4e0, ws.fgIsGame); InterlockedExchange(&g_reqActivate, 1); }
        }
        {   // 0.1.9: report guarded passes (the handler cannot log)
            static LONG fixLogged = 0;
            const LONG nf = g_divFixed;
            if (nf != fixLogged) {
                logf("DIVGUARD skipped an infinite loop: %ld total (+%ld); last struct=%08x i=%d j=%d arr[i]=%g arr[j]=%g", (long)nf, (long)(nf - fixLogged), (unsigned)g_divLastP,
                     (int)(int16_t)(g_divLastIJ >> 16), (int)(int16_t)(g_divLastIJ & 0xFFFF), (double)bitsToFloat(g_divLastFi), (double)bitsToFloat(g_divLastFj));
                fixLogged = nf;
            }
        }
        while (g_spFlushed < g_spHead) {   // SetPlayerId events written by the hook since the last tick
            LONG i = g_spFlushed++;
            if (g_spHead - i > SpRing) continue;
            const SetPlayerEv& e = g_sp[i % SpRing];
            bool known = false; for (uintptr_t r : SetPlayerRets) known |= r == e.ret;
            logf("SETPLAYER f%ld t%lu ret=%08x%s %08x -> %08x", e.frames, e.tid, (unsigned)e.ret, known ? "" : "(?)", e.oldId, e.newId);
        }
        if (lsUp && !lsSince) { lsSince = now; lsFrames = g_frames; lsIoMark = { g_opens, g_reads, g_readBytes, g_readFail + g_shortReads }; nextTl = now + 2000;
            logf("load screen UP (io so far: opens=%ld reads=%ld last='%s')", g_opens, g_reads, g_lastIoName); }
        if (lsUp && lsSince && static_cast<LONG>(now - nextTl) >= 0) {   // load timeline every 2 s while the load screen is up
            nextTl = now + 2000;
            logf("LOADTL +%lums f+%ld io: opens+%ld reads+%ld bytes+%lld fails+%ld lastIO=%ldms ago '%s' | gstate=%d playerId=%08x +28c=%08x areaReady=%d active(a1b4e0)=%d fgIsGame=%d",
                now - lsSince, g_frames - lsFrames, g_opens - lsIoMark.opens, g_reads - lsIoMark.reads, (long long)(g_readBytes - lsIoMark.bytes),
                (g_readFail + g_shortReads) - lsIoMark.fails, g_lastIoTick ? static_cast<LONG>(now - g_lastIoTick) : -1, g_lastIoName,
                st.gstate, (unsigned)st.playerId, (unsigned)st.lastPlayerSrv, st.areaReady, st.a1b4e0, lastWin.fgIsGame);
        }
        if (!lsUp && lsSince) { logf("load screen DOWN after %lu ms, %ld frames (io: opens+%ld reads+%ld bytes+%lld fails+%ld)", now - lsSince, g_frames - lsFrames,
            g_opens - lsIoMark.opens, g_reads - lsIoMark.reads, (long long)(g_readBytes - lsIoMark.bytes), (g_readFail + g_shortReads) - lsIoMark.fails); lsSince = 0; }

        if (GetFileAttributesA("hangprobe_dump.txt") != INVALID_FILE_ATTRIBUTES) {
            DeleteFileA("hangprobe_dump.txt"); manualUntil = now + ManualSampleMs; logf("manual dump requested");
        }
        // Sampling (the only part that pauses the game thread) runs only while armed.
        const char* armWhy = manualUntil ? "manual"
            : (lsSince && now - lsSince > ArmLoadMs && g_frames - lsFrames > ArmLoadFrames) ? "load screen up while frames run"
            : msSinceFrame(now) > static_cast<LONG>(ArmStallMs) ? "no MainLoop frame" : nullptr;
        if (armWhy && !armed) { armed = true; armedSince = now; armedSamples = 0; logf("sampling ARMED (%s)", armWhy); }
        if (!armWhy && armed) { armed = false; logf("sampling disarmed after %lu ms, %d samples", now - armedSince, armedSamples); }
        if (armed && th && !probe::shouldSample(armed, g_drArmed)) {
            if (!samplingSkippedNoted) { samplingSkippedNoted = true; logf("sampling skipped: DR0 armed on the main thread (SuspendThread could deadlock under the debug-register trap); report uses counters only"); }
        } else if (armed && th) {
            Sample& s = g_s[g_sHead];
            if (sampleMain(th, &s)) {
                s.tick = now; s.frames = g_frames; ++armedSamples;
                g_sHead = (g_sHead + 1) % Window; if (g_sCount < Window) ++g_sCount;
            }
        }

        const char* reason = nullptr;
        if (manualUntil && static_cast<LONG>(now - manualUntil) >= 0) { reason = "MANUAL"; lastReport = 0; manualUntil = 0; }
        else if (lsSince && now - lsSince > LoadStuckMs && g_frames - lsFrames > LoadStuckFrames) reason = "LOADSCREEN_STUCK";
        else if (msSinceFrame(now) > static_cast<LONG>(StallMs) && dominance(now, 10000) >= StallDominance) reason = "MAIN_STALL";
        if (reason && (!lastReport || now - lastReport >= ReportEveryMs)) {
            writeReport(reason, now, st);
            logf("REPORT %s written (dominance %d%%)", reason, dominance(now, 10000));
            lastReport = now;
            if (!strcmp(reason, "MAIN_STALL")) writeMiniDump();
        }
        if (!reason) lastReport = 0;

        if (now - lastBeat >= 60000) {
            lastBeat = now;
            logf("heartbeat: frames=%ld armed=%d samples=%d lsUp=%d dominance10s=%d%% failedProbes=%ld", g_frames, armed ? 1 : 0, g_sCount, lsUp ? 1 : 0, dominance(now, 10000), saferead::g_ringCount);
        }
    }
}

} // namespace

extern "C" {

// Hook: CClientExoAppInternal::MainLoop entry (0x00781BE0), ECX = internal. Main thread, every frame.
void __cdecl HangProbeMainLoop(void* internal) {
    if (!g_on) return;
    if (!g_mainTid) {
        NT_TIB* tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
        g_stackBase = reinterpret_cast<uintptr_t>(tib->StackBase);
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(&mbi, &mbi, sizeof(mbi)) == sizeof(mbi)) g_stackAlloc = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
        g_internal = reinterpret_cast<uintptr_t>(internal);
        g_mainTid = GetCurrentThreadId();
    }
    g_lastFrameTick = GetTickCount();
    InterlockedIncrement(&g_frames);
    // ---- 1.3.0 guarded activation fix (main thread, frame boundary; only when the engine thinks the window is inactive) ----
    if (gflag() != 0) { g_fixSince = 0; g_fixTries = 0; g_fixNoted = false; if (!g_reqActivate) return; }
    uintptr_t in = reinterpret_cast<uintptr_t>(internal), ls = *reinterpret_cast<uintptr_t*>(in + In_LoadScreen);
    bool lsUp = ls && *reinterpret_cast<uintptr_t*>(ls) == LoadScreenVtable;
    DWORD now = g_lastFrameTick;
    if (g_reqActivate) {   // manual test (touch hangprobe_activate.txt): one direct call, logs the effect
        InterlockedExchange(&g_reqActivate, 0);
        if (gflag() == 0) { fixNote("MANUAL: calling AppActivated 0x40BCF0 (lsUp=%d areaReady=%d)", lsUp, *reinterpret_cast<int*>(in + In_AreaReady)); reinterpret_cast<void(__cdecl*)()>(FnAppActivated)(); fixNote("MANUAL: a1b4e0 now %d", gflag()); }
        else fixNote("MANUAL: a1b4e0 already 1, nothing to do");
        return;
    }
    if (!lsUp) { g_fixSince = 0; return; }
    if (!g_fixSince) g_fixSince = now;
    if (!g_fixAuto || now - g_fixSince < g_fixDelayMs) return;
    HWND game = *reinterpret_cast<HWND volatile*>(GameHwndG); bool fgOurs = fgIsOurs(GetForegroundWindow()); bool iconic = game && IsIconic(game);
    int ready = *reinterpret_cast<int*>(in + In_AreaReady);
    probe::FixGate gate = probe::fixGate(ready != 0, fgOurs, iconic, g_fixTries, g_fixLastTry, now);
    if (gate != probe::FixGate::Act) {
        if (!g_fixNoted && gate != probe::FixGate::Cooldown) { g_fixNoted = true; fixNote("AUTO: load screen up %lu ms, active=0 but NOT acting: gate=%d areaReady=%d fgOurs=%d iconic=%d tries=%d", now - g_fixSince, (int)gate, ready, fgOurs, iconic, g_fixTries); }
        return;
    }
    ++g_fixTries; g_fixLastTry = now;
    fixNote("AUTO: load screen up %lu ms with active=0, foreground is ours, area ready -> calling AppActivated (try %d)", now - g_fixSince, g_fixTries);
    reinterpret_cast<void(__cdecl*)()>(FnAppActivated)();
    fixNote("AUTO: a1b4e0 now %d", gflag());
}

// Hook: AppActivated 0x0040BCF0 / AppDeactivated 0x0040BC40 entry. [esp+0] = caller, [esp+4] = first stack arg (deactivate: a value from the message).
void __cdecl HangProbeActivated(void* ret, int arg) {
    if (!g_on) return;
    WinEv* e = winSlot(); e->tick = GetTickCount(); e->frames = g_frames; e->kind = 1; e->a = static_cast<uintptr_t>(arg); e->b = e->c = e->d = 0; e->ret = reinterpret_cast<uintptr_t>(ret); e->fg = GetForegroundWindow(); e->flag = gflag();
}
void __cdecl HangProbeDeactivated(void* ret, int arg) {
    if (!g_on) return;
    WinEv* e = winSlot(); e->tick = GetTickCount(); e->frames = g_frames; e->kind = 2; e->a = static_cast<uintptr_t>(arg); e->b = e->c = e->d = 0; e->ret = reinterpret_cast<uintptr_t>(ret); e->fg = GetForegroundWindow(); e->flag = gflag();
}

// ---- 1.4.0 probe-log points (ring slots only). d = thread id unless noted. ----
static WinEv* ev14(unsigned kind) { WinEv* e = winSlot(); e->tick = GetTickCount(); e->frames = g_frames; e->kind = kind; e->a = e->b = e->c = e->ret = 0;
    e->d = GetCurrentThreadId(); e->fg = GetForegroundWindow(); e->flag = gflag(); e->calls = 0; return e; }
// 0x40B4AD: WndProc case WM_ACTIVATEAPP ([EBP+8] hwnd, [EBP+0x10] wParam, [EBP+0x14] lParam). c bit0 = foreground window belongs to us, a = hwnd, b = wParam, ret = lParam.
void __cdecl HangProbeActAppArrive(void* hwnd, int wp, int lp) {
    if (!g_on) return;
    WinEv* e = ev14(3); e->a = reinterpret_cast<uintptr_t>(hwnd); e->b = static_cast<uintptr_t>(wp); e->ret = static_cast<uintptr_t>(lp); e->c = fgIsOurs(e->fg) ? 1 : 0;
}
void __cdecl HangProbeMovieWndDeact() { if (g_on) ev14(4); }
void __cdecl HangProbeMovie76c() { if (g_on) ev14(5); }
void __cdecl HangProbeSynthPost() { if (g_on) { WinEv* e = ev14(6); e->a = *reinterpret_cast<volatile uintptr_t*>(GameHwndG); } }
// 0x40BF40 RestoreAfterMovie entry ([esp+0] = caller): a = [a1b764], b = [a1b768], c = [a1b76c].
// 1.4.1: one slot per tuple change only.
void __cdecl HangProbeRestoreAfterMovie(void* ret) {
    if (!g_on) return;
    LONG n = InterlockedIncrement(&g_restoreCalls);
    uintptr_t t[5] = { *reinterpret_cast<volatile unsigned*>(0x00A1B764), *reinterpret_cast<volatile unsigned*>(Gate768), *reinterpret_cast<volatile unsigned*>(Gate76C),
                       reinterpret_cast<uintptr_t>(ret), static_cast<uintptr_t>(gflag()) };
    if (!tupleChanged(g_restoreLast, t, 5, g_restoreSeen)) return;
    WinEv* e = ev14(7); e->a = t[0]; e->b = t[1]; e->c = t[2]; e->ret = t[3]; e->calls = n;
}
// 0x798B39 PlayMovies exit: a = [EBP-0x18], b = [EBP-0x20] (cut-off latch). 1.4.1: one slot per tuple change only.
void __cdecl HangProbeMovieExit(int v18, int v20) {
    if (!g_on) return;
    LONG n = InterlockedIncrement(&g_exitCalls);
    uintptr_t t[3] = { static_cast<uintptr_t>(v18), static_cast<uintptr_t>(v20), static_cast<uintptr_t>(gflag()) };
    if (!tupleChanged(g_exitLast, t, 3, g_exitSeen)) return;
    WinEv* e = ev14(8); e->a = t[0]; e->b = t[1]; e->calls = n;
}
void __cdecl HangProbePumpCutoff() { if (g_on) ev14(9); }
// 0x781E0B MainLoop: CMP [a1b4e0],0 deciding whether the load screen may be hidden (main thread, every frame). Logs only changes.
void __cdecl HangProbeLsDecision() {
    if (!g_on) return;
    InterlockedIncrement(&g_lsDecisions);
    int a = gflag(); HWND fg = GetForegroundWindow(); int f = fgIsOurs(fg) ? 1 : 0;
    if (a == g_lsLastA && f == g_lsLastFg) return;
    g_lsLastA = a; g_lsLastFg = f;
    WinEv* e = ev14(10); e->a = static_cast<uintptr_t>(a); e->b = static_cast<uintptr_t>(f); e->c = static_cast<uintptr_t>(g_lsDecisions);
}

// Hook: CClientExoAppInternal::SetPlayerCreatureId (0x00798660) entry, thiscall: ECX = internal, [esp+0] = caller,
// [esp+4] = new id. Only writes one ring slot (no I/O); the watchdog logs it as SETPLAYER.
void __cdecl HangProbeSetPlayerId(void* internal, void* ret, int newId) {
    if (!g_on || !internal) return;
    LONG i = InterlockedIncrement(&g_spHead) - 1;
    SetPlayerEv& e = g_sp[i % SpRing];
    e.tick = GetTickCount(); e.frames = g_frames; e.tid = GetCurrentThreadId(); e.ret = reinterpret_cast<uintptr_t>(ret);
    e.oldId = *reinterpret_cast<unsigned*>(reinterpret_cast<uintptr_t>(internal) + 0x20); e.newId = static_cast<unsigned>(newId);
}

// For other diagnostics: the probe's failed-read ring as text.
int __cdecl HangProbeDumpDiag(char* buf, int cap) { return buf && cap > 0 ? saferead::dump(buf, cap) : 0; }

} // extern "C"

extern "C" void __cdecl KPatchInit(const KPatchApi* api) { if (KpLog_Init(api)) { g_initDone = 1; flushEarly(); } }

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = inst;
        g_on = GetFileAttributesA("hangprobe_debug.txt") != INVALID_FILE_ATTRIBUTES;
        if (!g_on) {   // 0.1.10 guard-only mode: DllMain runs on the game's main thread (implicit load of the binkw32 proxy)
            g_guardTid = GetCurrentThreadId();
            HANDLE gt = CreateThread(nullptr, 0, guardOnly, nullptr, 0, nullptr);
            if (gt) CloseHandle(gt);
            return TRUE;
        }
        saferead::enableRing(true);
        HMODULE exe = GetModuleHandleA(nullptr);
        g_exeLo = reinterpret_cast<uintptr_t>(exe);
        auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(exe);
        auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(g_exeLo + dos->e_lfanew);
        g_exeHi = g_exeLo + nt->OptionalHeader.SizeOfImage;
        if (FILE* cf = fopen("hangprobe_debug.txt", "r")) { char l[64]; while (fgets(l, sizeof l, cf)) { if (!strncmp(l, "fix=auto", 8)) g_fixAuto = true; if (probe::parseFixOff(l)) g_fixAuto = false; if (!strncmp(l, "fixdelay=", 9)) { long v = atol(l + 9); if (v >= 1000 && v <= 60000) g_fixDelayMs = (DWORD)v; } } fclose(cf); }
        logf("load-hang-probe 0.1.11 loaded: attach tid=%lu exe=%08x-%08x fix=%s fixdelay=%lums", GetCurrentThreadId(), (unsigned)g_exeLo, (unsigned)g_exeHi, g_fixAuto ? "auto" : "off (log only)", g_fixDelayMs);
        {   // 1.2.0 file-I/O capture; "io=0" in hangprobe_debug.txt disables it
            bool ioWanted = true;
            if (FILE* cf = fopen("hangprobe_debug.txt", "r")) { char l[64]; while (fgets(l, sizeof l, cf)) if (!strncmp(l, "io=0", 4)) ioWanted = false; fclose(cf); }
            if (ioWanted) {
                void *o1 = 0, *o2 = 0, *o3 = 0, *o4 = 0, *o5 = 0;
                bool a = codepatch::patchIat(exe, "KERNEL32.dll", "CreateFileA", reinterpret_cast<void*>(hkCreateFileA), &o1); if (a) rCreateFileA = reinterpret_cast<CreateFileA_t>(o1);
                bool w = codepatch::patchIat(exe, "KERNEL32.dll", "CreateFileW", reinterpret_cast<void*>(hkCreateFileW), &o2); if (w) rCreateFileW = reinterpret_cast<CreateFileW_t>(o2);
                bool r = codepatch::patchIat(exe, "KERNEL32.dll", "ReadFile", reinterpret_cast<void*>(hkReadFile), &o3); if (r) rReadFile = reinterpret_cast<ReadFile_t>(o3);
                bool s = codepatch::patchIat(exe, "KERNEL32.dll", "SetFilePointer", reinterpret_cast<void*>(hkSetFilePointer), &o4); if (s) rSetFilePointer = reinterpret_cast<SetFilePointer_t>(o4);
                bool c = codepatch::patchIat(exe, "KERNEL32.dll", "CloseHandle", reinterpret_cast<void*>(hkCloseHandle), &o5); if (c) rCloseHandle = reinterpret_cast<CloseHandle_t>(o5);
                g_ioOn = (a || w) && r && s && c;
                logf("IAT file I/O hooks: CreateFileA=%d CreateFileW=%d ReadFile=%d SetFilePointer=%d CloseHandle=%d -> capture %s", a, w, r, s, c, g_ioOn ? "ON" : "OFF (ReadFile/SetFilePointer/CloseHandle or both CreateFile variants missing)");
            } else logf("file I/O capture disabled (io=0)");
        }
        HANDLE t = CreateThread(nullptr, 0, watchdog, nullptr, 0, nullptr);
        if (t) CloseHandle(t); else logf("watchdog thread creation FAILED (%lu)", GetLastError());
    }
    return TRUE;
}
