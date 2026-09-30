// Load Hang Probe (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Diagnostics only, no behaviour change.
// Investigation: patch_manager_mods/09_load_hang_investigation.md sections 9-10 (local-setup/KOTOR-II worktree).
//
// Does nothing unless "hangprobe_debug.txt" exists in the game dir. Then:
//   * hook 0x00781BE0 (CClientExoAppInternal::MainLoop entry, ECX = client internal): frame heartbeat,
//     remembers the main thread id, the internal pointer and the main thread's stack base;
//   * a watchdog thread samples the main thread every 250 ms (Suspend / GetThreadContext / stack walk
//     that touches only [ESP, StackBase) / Resume) and reads the load-screen state fields every tick;
//   * hangprobe_log.txt: load-state CHANGES (one line each) + a heartbeat line every 60 s;
//   * hangprobe_report.txt (rewritten every 5 s while a condition holds): stack histogram with
//     module+offset, load state, failed-probe ring, loaded modules, tails of the other mods' logs.
//     Conditions: LOADSCREEN_STUCK (CSWGuiLoadScreen modal while frames keep running), MAIN_STALL
//     (no frame for 15 s and one stack dominating), MANUAL (touch hangprobe_dump.txt in the game dir).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../_shared/SafeRead.h"

namespace {

using saferead::readAt;

// ---- engine layout (Ghidra, 09 section 9d) ------------------------------------------------------------
constexpr uintptr_t AppGlobal = 0x00A1B4A4;          // app; client = *(app+4); internal = *(client+4)
constexpr uintptr_t LoadScreenVtable = 0x009A5C84;   // CSWGuiLoadScreen
constexpr uintptr_t FlagA1B4E0 = 0x00A1B4E0;         // tested next to HideLoadScreen in MainLoop
constexpr int In_Flag18 = 0x18, In_PlayerId = 0x20, In_Sub48 = 0x48, In_Movie = 0x130, In_GuiMgr = 0x274,
              In_LoadScreen = 0x278, In_AreaReady = 0x288, In_PendingSwap = 0x338, In_PlayerCacheId = 0x3BC,
              In_PlayerCachePtr = 0x3C0;
constexpr int Sub48_Flag = 0x268;
constexpr int App_ModuleState = 0x14;                // *(app+0x14) -> [0]==1, [1] in {1,3} sets +0x288
constexpr int Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;

// ---- config -------------------------------------------------------------------------------------------
constexpr DWORD SampleMs = 250;
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

// ---- logging (watchdog thread only) --------------------------------------------------------------------
int g_logLines = 0;
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    FILE* f = fopen("hangprobe_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    fprintf(f, "[%lu f%ld] ", GetTickCount(), g_frames);
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

// ---- main-thread samples --------------------------------------------------------------------------------
struct Sample { DWORD tick; LONG frames; uintptr_t eip, esp; int n; uintptr_t ret[MaxFrames]; int ns; uintptr_t scan[ScanHits]; };
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
        s->eip = ctx.Eip; s->esp = esp; s->n = 0; s->ns = 0;
        if (esp >= alloc && esp < base) {           // [esp, base) is committed: the thread has used it
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
    { uintptr_t m = 0; if (readAt(30, app, App_ModuleState, &m) && m) { readAt(31, m, 0, &st.modA); readAt(32, m, 4, &st.modB); readAt(33, m, 0x38, &st.modMode); } }
    return st;
}

int fmtState(char* b, int cap, const LoadState& s) {
    return snprintf(b, cap,
        "internal=%08x ls=%08x lsVt=%08x%s lsModal=%d modals=%d | +18=%d playerId=%08x areaReady(+288)=%d pendingSwap(+338)=%08x "
        "cache(+3bc/+3c0)=%08x/%08x sub48flag=%d movie=%d a1b4e0=%d | module state [0]=%d [1]=%d [0x38]=%d",
        (unsigned)s.internal, (unsigned)s.ls, (unsigned)s.lsVt, s.ls && s.lsVt != LoadScreenVtable ? "(!)" : "", s.lsModal,
        s.modalCount, s.flag18, (unsigned)s.playerId, s.areaReady, (unsigned)s.pendingSwap, (unsigned)s.cacheId,
        (unsigned)s.cachePtr, s.sub48Flag, s.movie, s.a1b4e0, s.modA, s.modB, s.modMode);
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
        fputc('\n', f);
    }
    static char ring[8192]; saferead::dump(ring, sizeof(ring));
    fprintf(f, "\nprobe's own %s", ring);
    fprintf(f, "\nmodules:\n");
    for (int i = 0; i < g_modCount; ++i) fprintf(f, "  %08x-%08x %s\n", (unsigned)g_mods[i].lo, (unsigned)g_mods[i].hi, g_mods[i].name);
    fprintf(f, "\nother mods' logs:\n");
    tailFile(f, "force_power_hotbar_log.txt", 40);
    tailFile(f, "buffhud_log.txt", 40);
    tailFile(f, "inv3d_log.txt", 40);
    tailFile(f, "hangprobe_log.txt", 60);
    fclose(f);
}

DWORD WINAPI watchdog(LPVOID) {
    HANDLE th = nullptr;
    LoadState last; memset(&last, 0, sizeof(last));
    DWORD lsSince = 0, lastReport = 0, lastBeat = GetTickCount();
    LONG lsFrames = 0;
    bool firstState = true;
    for (;;) {
        Sleep(SampleMs);
        DWORD now = GetTickCount();
        if (!th && g_mainTid) {
            th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, g_mainTid);
            logf("main thread %lu %s, internal=%08x stackBase=%08x stackAlloc=%08x", g_mainTid, th ? "opened" : "OPEN FAILED", (unsigned)g_internal, (unsigned)g_stackBase, (unsigned)g_stackAlloc);
        }
        if (th) {
            Sample& s = g_s[g_sHead];
            if (sampleMain(th, &s)) {
                s.tick = now; s.frames = g_frames;
                g_sHead = (g_sHead + 1) % Window; if (g_sCount < Window) ++g_sCount;
            }
        }

        LoadState st = readLoadState();
        if (firstState || st != last) {
            char line[1024]; fmtState(line, sizeof(line), st);
            logf("STATE %s", line);
            last = st; firstState = false;
        }
        bool lsUp = st.ls && st.lsVt == LoadScreenVtable && st.lsModal;
        if (lsUp && !lsSince) { lsSince = now; lsFrames = g_frames; logf("load screen UP"); }
        if (!lsUp && lsSince) { logf("load screen DOWN after %lu ms, %ld frames", now - lsSince, g_frames - lsFrames); lsSince = 0; }

        const char* reason = nullptr;
        if (GetFileAttributesA("hangprobe_dump.txt") != INVALID_FILE_ATTRIBUTES) { DeleteFileA("hangprobe_dump.txt"); reason = "MANUAL"; lastReport = 0; }
        else if (lsSince && now - lsSince > LoadStuckMs && g_frames - lsFrames > LoadStuckFrames) reason = "LOADSCREEN_STUCK";
        else if (msSinceFrame(now) > static_cast<LONG>(StallMs) && dominance(now, 10000) >= StallDominance) reason = "MAIN_STALL";
        if (reason && (!lastReport || now - lastReport >= ReportEveryMs)) {
            writeReport(reason, now, st);
            logf("REPORT %s written (dominance %d%%)", reason, dominance(now, 10000));
            lastReport = now;
        }
        if (!reason) lastReport = 0;

        if (now - lastBeat >= 60000) {
            lastBeat = now;
            logf("heartbeat: frames=%ld samples=%d lsUp=%d dominance10s=%d%% failedProbes=%ld", g_frames, g_sCount, lsUp ? 1 : 0, dominance(now, 10000), saferead::g_ringCount);
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
}

// For other diagnostics: the probe's failed-read ring as text.
int __cdecl HangProbeDumpDiag(char* buf, int cap) { return buf && cap > 0 ? saferead::dump(buf, cap) : 0; }

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = inst;
        g_on = GetFileAttributesA("hangprobe_debug.txt") != INVALID_FILE_ATTRIBUTES;
        if (!g_on) return TRUE;
        saferead::enableRing(true);
        HMODULE exe = GetModuleHandleA(nullptr);
        g_exeLo = reinterpret_cast<uintptr_t>(exe);
        auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(exe);
        auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(g_exeLo + dos->e_lfanew);
        g_exeHi = g_exeLo + nt->OptionalHeader.SizeOfImage;
        logf("load-hang-probe 1.0.0 loaded: attach tid=%lu exe=%08x-%08x", GetCurrentThreadId(), (unsigned)g_exeLo, (unsigned)g_exeHi);
        HANDLE t = CreateThread(nullptr, 0, watchdog, nullptr, 0, nullptr);
        if (t) CloseHandle(t); else logf("watchdog thread creation FAILED (%lu)", GetLastError());
    }
    return TRUE;
}
