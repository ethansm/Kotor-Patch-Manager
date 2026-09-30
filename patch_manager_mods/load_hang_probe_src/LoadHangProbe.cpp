// Load Hang Probe (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Diagnostics only, no behaviour change.
// Investigation: patch_manager_mods/09_load_hang_investigation.md sections 9-10 (local-setup/KOTOR-II worktree).
//
// Does nothing unless "hangprobe_debug.txt" exists in the game dir. Then:
//   * hook 0x00781BE0 (CClientExoAppInternal::MainLoop entry, ECX = client internal): frame heartbeat,
//     remembers the main thread id, the internal pointer and the main thread's stack base;
//   * a watchdog thread polls the load-screen state every 250 ms (plain reads, no suspend) and only when
//     something already looks wrong (load screen up > 5 s while frames run, no frame for 5 s, or a manual
//     trigger) samples the main thread every 250 ms (Suspend / GetThreadContext / stack walk
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

bool patchIat(HMODULE mod, const char* dll, const char* fn, void* hook, void** orig) {
    auto base = reinterpret_cast<uint8_t*>(mod);
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dir.VirtualAddress) return false;
    for (auto d = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress); d->Name; ++d) {
        if (_stricmp(reinterpret_cast<const char*>(base + d->Name), dll) != 0) continue;
        auto oft = reinterpret_cast<IMAGE_THUNK_DATA32*>(base + (d->OriginalFirstThunk ? d->OriginalFirstThunk : d->FirstThunk));
        auto ft = reinterpret_cast<IMAGE_THUNK_DATA32*>(base + d->FirstThunk);
        for (; oft->u1.AddressOfData; ++oft, ++ft) {
            if (oft->u1.Ordinal & IMAGE_ORDINAL_FLAG32) continue;
            auto ibn = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + oft->u1.AddressOfData);
            if (strcmp(reinterpret_cast<const char*>(ibn->Name), fn) != 0) continue;
            DWORD old;
            if (!VirtualProtect(&ft->u1.Function, 4, PAGE_READWRITE, &old)) return false;
            *orig = reinterpret_cast<void*>(ft->u1.Function);
            ft->u1.Function = reinterpret_cast<DWORD>(hook);
            VirtualProtect(&ft->u1.Function, 4, old, &old);
            return true;
        }
    }
    return false;
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
    dumpSetPlayer(f, now);
    dumpIo(f, now, 150);
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
    struct { LONG opens, reads; LONG64 bytes; LONG fails; } lsIoMark = { 0, 0, 0, 0 };
    DWORD nextTl = 0;
    bool firstState = true, armed = false;
    DWORD manualUntil = 0, armedSince = 0;
    int armedSamples = 0;
    for (;;) {
        Sleep(SampleMs);
        DWORD now = GetTickCount();
        if (!th && g_mainTid) {
            th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, g_mainTid);
            logf("main thread %lu %s, internal=%08x stackBase=%08x stackAlloc=%08x", g_mainTid, th ? "opened" : "OPEN FAILED", (unsigned)g_internal, (unsigned)g_stackBase, (unsigned)g_stackAlloc);
        }
        LoadState st = readLoadState();
        if (firstState || st != last) {
            char line[1024]; fmtState(line, sizeof(line), st);
            logf("STATE %s", line);
            last = st; firstState = false;
        }
        bool lsUp = st.ls && st.lsVt == LoadScreenVtable && st.lsModal;
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
            logf("LOADTL +%lums f+%ld io: opens+%ld reads+%ld bytes+%lld fails+%ld lastIO=%ldms ago '%s' | gstate=%d playerId=%08x +28c=%08x areaReady=%d",
                now - lsSince, g_frames - lsFrames, g_opens - lsIoMark.opens, g_reads - lsIoMark.reads, (long long)(g_readBytes - lsIoMark.bytes),
                (g_readFail + g_shortReads) - lsIoMark.fails, g_lastIoTick ? static_cast<LONG>(now - g_lastIoTick) : -1, g_lastIoName,
                st.gstate, (unsigned)st.playerId, (unsigned)st.lastPlayerSrv, st.areaReady);
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
        if (armed && th) {
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
        logf("load-hang-probe 1.2.0 loaded: attach tid=%lu exe=%08x-%08x", GetCurrentThreadId(), (unsigned)g_exeLo, (unsigned)g_exeHi);
        {   // 1.2.0 file-I/O capture; "io=0" in hangprobe_debug.txt disables it
            bool ioWanted = true;
            if (FILE* cf = fopen("hangprobe_debug.txt", "r")) { char l[64]; while (fgets(l, sizeof l, cf)) if (!strncmp(l, "io=0", 4)) ioWanted = false; fclose(cf); }
            if (ioWanted) {
                void *o1 = 0, *o2 = 0, *o3 = 0, *o4 = 0, *o5 = 0;
                bool a = patchIat(exe, "KERNEL32.dll", "CreateFileA", reinterpret_cast<void*>(hkCreateFileA), &o1); if (a) rCreateFileA = reinterpret_cast<CreateFileA_t>(o1);
                bool w = patchIat(exe, "KERNEL32.dll", "CreateFileW", reinterpret_cast<void*>(hkCreateFileW), &o2); if (w) rCreateFileW = reinterpret_cast<CreateFileW_t>(o2);
                bool r = patchIat(exe, "KERNEL32.dll", "ReadFile", reinterpret_cast<void*>(hkReadFile), &o3); if (r) rReadFile = reinterpret_cast<ReadFile_t>(o3);
                bool s = patchIat(exe, "KERNEL32.dll", "SetFilePointer", reinterpret_cast<void*>(hkSetFilePointer), &o4); if (s) rSetFilePointer = reinterpret_cast<SetFilePointer_t>(o4);
                bool c = patchIat(exe, "KERNEL32.dll", "CloseHandle", reinterpret_cast<void*>(hkCloseHandle), &o5); if (c) rCloseHandle = reinterpret_cast<CloseHandle_t>(o5);
                g_ioOn = (a || w) && r && s && c;
                logf("IAT file I/O hooks: CreateFileA=%d CreateFileW=%d ReadFile=%d SetFilePointer=%d CloseHandle=%d -> capture %s", a, w, r, s, c, g_ioOn ? "ON" : "OFF (ReadFile/SetFilePointer/CloseHandle or both CreateFile variants missing)");
            } else logf("file I/O capture disabled (io=0)");
        }
        HANDLE t = CreateThread(nullptr, 0, watchdog, nullptr, 0, nullptr);
        if (t) CloseHandle(t); else logf("watchdog thread creation FAILED (%lu)", GetLastError());
    }
    return TRUE;
}
