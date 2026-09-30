// PerfLab 0.1.1 (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Diagnostics only, no behaviour change.
// Research program: patch_manager_mods/10_engine_optimization_research.md.
//
// Inert unless "perflab_debug.txt" exists in the game dir (key=value lines, all optional):
//   alloc=1          1a allocation census (default 1 when the file exists)
//   heapfilter=1     only track the CRT heap (handle in the exe global 0x00A806C8); 0 = any heap (harness)
//   flush_ms=30000   perflab_alloc.txt rewrite interval
//   scan=48          stack dwords scanned per allocation for call-site attribution
//   snap_min=1024    per-site live bytes needed to appear in a load-screen snapshot
//
// 1a: IAT hooks (installed from DllMain) on the exe's KERNEL32 HeapAlloc/HeapReAlloc/HeapFree. The exe's CRT
// (VS2008, static) reaches these for every malloc/calloc/realloc/free (doc 10 finding 1), so tracking the
// CRT heap tracks the engine. Each live pointer maps to (site, size) in a lock-free open-addressing table
// placed in VirtualAlloc memory (never in the tracked heap). Site = first two call-preceded return
// addresses found by scanning the stack, skipping the CRT range (frame pointers are not reliable in the
// optimised build; attribution is approximate). Hooks never do I/O; a helper thread writes:
//   perflab_log.txt          attach info, table occupancy, hook self cost
//   perflab_alloc.txt        rewritten every flush_ms: totals + top sites (by live bytes, total bytes, count)
//   perflab_alloc_snaps.txt  appended at every load-screen UP/DOWN edge: per-site live bytes (leak deltas)
// Resolve site addresses to function names offline: perf_lab_src/resolve_sites.py.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "../_shared/SafeRead.h"

namespace {

// ---- engine constants (Ghidra) -------------------------------------------------------------------------
constexpr uintptr_t CrtHeapGlobal = 0x00A806C8;      // _crtheap (HANDLE)
constexpr uintptr_t CrtLo = 0x00919000, CrtHi = 0x0093A000;   // CRT text: allocation wrappers, skipped in attribution
constexpr uintptr_t AppGlobal = 0x00A1B4A4;          // app; client = *(app+4); internal = *(client+4)
constexpr uintptr_t LoadScreenVtable = 0x009A5C84;
constexpr int In_GuiMgr = 0x274, In_LoadScreen = 0x278, Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;

// ---- config --------------------------------------------------------------------------------------------
struct Cfg { bool alloc = true, heapFilter = true; DWORD flushMs = 30000; int scan = 48; unsigned snapMin = 1024; } g_cfg;
bool g_on = false;
uintptr_t g_exeLo = 0, g_exeHi = 0;
volatile LONG g_ticks = 0;                            // hook 0x00478A20 calls (frame heartbeat)

// ---- tables ---------------------------------------------------------------------------------------------
constexpr int SiteCap = 1 << 14;
constexpr int PtrBits = 22;
constexpr uint32_t PtrCap = 1u << PtrBits;
constexpr uint32_t LargeMark = 0x3FFFF;               // size field: 18 bits; >= mark means "query HeapSize"
constexpr int Hist = 16;

struct Site {
    volatile LONG64 key;                              // a1 | a2<<32; 0 = empty
    volatile LONG64 allocs, bytes, frees, freeBytes, reallocs;
    volatile LONG hist[Hist];
};
Site* g_sites = nullptr;
volatile LONG g_siteCount = 0;
uint32_t* g_pk = nullptr;                             // pointer keys: 0 empty, 1 tomb
uint32_t* g_pv = nullptr;                             // site(14) | size(18)<<14
volatile LONG64 g_allocN = 0, g_freeN = 0, g_reallocN = 0, g_liveBytes = 0, g_liveN = 0, g_totalBytes = 0;
volatile LONG g_dropped = 0, g_siteOverflow = 0, g_ptrProbeMax = 0, g_untracked = 0;
volatile LONG64 g_hookCycles = 0, g_hookSamples = 0;

typedef LPVOID(WINAPI* HeapAlloc_t)(HANDLE, DWORD, SIZE_T);
typedef LPVOID(WINAPI* HeapReAlloc_t)(HANDLE, DWORD, LPVOID, SIZE_T);
typedef BOOL(WINAPI* HeapFree_t)(HANDLE, DWORD, LPVOID);
typedef SIZE_T(WINAPI* HeapSize_t)(HANDLE, DWORD, LPCVOID);
HeapAlloc_t rHeapAlloc; HeapReAlloc_t rHeapReAlloc; HeapFree_t rHeapFree; HeapSize_t rHeapSize;

inline bool mine(HANDLE h) {
    if (!g_cfg.heapFilter) return true;
    HANDLE crt = *reinterpret_cast<HANDLE volatile*>(CrtHeapGlobal);
    return crt == nullptr || h == crt;
}

inline bool callPreceded(uintptr_t v) {
    const uint8_t* b = reinterpret_cast<const uint8_t*>(v);
    return b[-5] == 0xE8 || (b[-6] == 0xFF && (b[-5] == 0x15 || (b[-5] & 0xF8) == 0x90)) ||
           (b[-2] == 0xFF && (b[-1] & 0x38) == 0x10) || (b[-3] == 0xFF && (b[-2] & 0x38) == 0x10);
}

// Called from the hook wrappers; `sp` is the wrapper's frame address.
inline uint64_t siteKey(const uintptr_t* sp) {
    uintptr_t base = __readfsdword(4);                // NT_TIB.StackBase
    uintptr_t found[2] = { 0, 0 }; int n = 0;
    int scan = g_cfg.scan;
    for (int i = 0; i < scan && reinterpret_cast<uintptr_t>(sp + i + 1) <= base && n < 2; ++i) {
        uintptr_t v = sp[i];
        if (v < g_exeLo + 0x1000 || v >= g_exeHi || (v >= CrtLo && v < CrtHi)) continue;
        if (callPreceded(v)) found[n++] = v;
    }
    if (!found[0]) found[0] = 1;
    return static_cast<uint64_t>(found[0]) | (static_cast<uint64_t>(found[1]) << 32);
}

int siteIndex(uint64_t key) {
    uint32_t h = static_cast<uint32_t>((key ^ (key >> 29)) * 0x9E3779B97F4A7C15ull >> 40) & (SiteCap - 1);
    for (int i = 0; i < SiteCap; ++i, h = (h + 1) & (SiteCap - 1)) {
        LONG64 k = g_sites[h].key;
        if (k == static_cast<LONG64>(key)) return static_cast<int>(h);
        if (k == 0) {
            LONG64 prev = InterlockedCompareExchange64(&g_sites[h].key, static_cast<LONG64>(key), 0);
            if (prev == 0) { InterlockedIncrement(&g_siteCount); return static_cast<int>(h); }
            if (prev == static_cast<LONG64>(key)) return static_cast<int>(h);
        }
    }
    return -1;
}

inline int histIdx(SIZE_T n) { int b = 0; SIZE_T v = n >> 4; while (v && b < Hist - 1) { ++b; v >>= 1; } return b; }

inline uint32_t ptrHash(uint32_t p) { return (p >> 3) * 2654435761u >> (32 - PtrBits); }

bool ptrInsert(uint32_t p, uint32_t val) {
    uint32_t h = ptrHash(p);
    for (uint32_t i = 0; i < 4096; ++i, h = (h + 1) & (PtrCap - 1)) {
        uint32_t k = g_pk[h];
        if (k == 0 || k == 1) {
            if (static_cast<uint32_t>(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&g_pk[h]), static_cast<LONG>(p), static_cast<LONG>(k))) == k) {
                g_pv[h] = val;
                if (static_cast<LONG>(i) > g_ptrProbeMax) g_ptrProbeMax = i;
                return true;
            }
            --i; h = (h - 1) & (PtrCap - 1);           // lost the race for this slot: retry it
        }
    }
    return false;
}

// Removes p; returns false if untracked. Value is read before the slot is released.
bool ptrRemove(uint32_t p, uint32_t* val) {
    uint32_t h = ptrHash(p);
    for (uint32_t i = 0; i < 4096; ++i, h = (h + 1) & (PtrCap - 1)) {
        uint32_t k = g_pk[h];
        if (k == 0) return false;
        if (k == p) { *val = g_pv[h]; g_pk[h] = 1; return true; }
    }
    return false;
}

inline void trackAlloc(HANDLE h, void* r, SIZE_T n, const uintptr_t* sp, bool isRealloc) {
    uint64_t key = siteKey(sp);
    int si = siteIndex(key);
    if (si < 0) { InterlockedIncrement(&g_siteOverflow); si = 0; }
    SIZE_T acct = n;
    uint32_t sz = n >= LargeMark ? LargeMark : static_cast<uint32_t>(n);
    if (sz == LargeMark) acct = rHeapSize(h, 0, r);
    if (!ptrInsert(reinterpret_cast<uintptr_t>(r), static_cast<uint32_t>(si) | (sz << 14))) {
        InterlockedIncrement(&g_dropped); return;
    }
    Site& s = g_sites[si];
    InterlockedIncrement64(&s.allocs); InterlockedExchangeAdd64(&s.bytes, acct);
    if (isRealloc) InterlockedIncrement64(&s.reallocs);
    InterlockedIncrement(&s.hist[histIdx(n)]);
    InterlockedIncrement64(&g_allocN); InterlockedExchangeAdd64(&g_liveBytes, acct);
    InterlockedIncrement64(&g_liveN); InterlockedExchangeAdd64(&g_totalBytes, acct);
}

// Must run BEFORE the real free/realloc so a recycled pointer cannot collide with its own tombstone race.
inline bool trackFree(HANDLE h, void* p) {
    uint32_t val;
    if (!ptrRemove(reinterpret_cast<uintptr_t>(p), &val)) { InterlockedIncrement(&g_untracked); return false; }
    uint32_t sz = val >> 14, si = val & 0x3FFF;
    SIZE_T acct = sz == LargeMark ? rHeapSize(h, 0, p) : sz;
    Site& s = g_sites[si];
    InterlockedIncrement64(&s.frees); InterlockedExchangeAdd64(&s.freeBytes, acct);
    InterlockedIncrement64(&g_freeN); InterlockedExchangeAdd64(&g_liveBytes, -static_cast<LONG64>(acct));
    InterlockedDecrement64(&g_liveN);
    return true;
}

struct Timer {   // samples 1 in 64 calls
    uint64_t t0 = 0; bool on;
    Timer() { static volatile LONG c; on = (InterlockedIncrement(&c) & 63) == 0; if (on) t0 = __rdtsc(); }
    ~Timer() { if (on) { InterlockedExchangeAdd64(&g_hookCycles, static_cast<LONG64>(__rdtsc() - t0)); InterlockedIncrement64(&g_hookSamples); } }
};

LPVOID WINAPI hkHeapAlloc(HANDLE h, DWORD f, SIZE_T n) {
    LPVOID r = rHeapAlloc(h, f, n);
    if (r && mine(h)) { Timer t; trackAlloc(h, r, n, static_cast<const uintptr_t*>(__builtin_frame_address(0)), false); }
    return r;
}
LPVOID WINAPI hkHeapReAlloc(HANDLE h, DWORD f, LPVOID p, SIZE_T n) {
    if (!p || !mine(h)) return rHeapReAlloc(h, f, p, n);
    Timer t;
    uint32_t val = 0; bool had = ptrRemove(reinterpret_cast<uintptr_t>(p), &val);
    uint32_t sz = val >> 14; SIZE_T oldAcct = had ? (sz == LargeMark ? rHeapSize(h, 0, p) : sz) : 0;
    LPVOID r = rHeapReAlloc(h, f, p, n);
    if (had) {
        Site& os = g_sites[val & 0x3FFF];
        if (r) {   // old block gone
            InterlockedIncrement64(&os.frees); InterlockedExchangeAdd64(&os.freeBytes, oldAcct);
            InterlockedIncrement64(&g_freeN); InterlockedExchangeAdd64(&g_liveBytes, -static_cast<LONG64>(oldAcct)); InterlockedDecrement64(&g_liveN);
        } else ptrInsert(reinterpret_cast<uintptr_t>(p), val);   // failed: old block still live
    } else InterlockedIncrement(&g_untracked);
    if (r) { InterlockedIncrement64(&g_reallocN); trackAlloc(h, r, n, static_cast<const uintptr_t*>(__builtin_frame_address(0)), true); }
    return r;
}
BOOL WINAPI hkHeapFree(HANDLE h, DWORD f, LPVOID p) {
    if (p && mine(h)) { Timer t; trackFree(h, p); }
    return rHeapFree(h, f, p);
}

// ---- IAT patching -----------------------------------------------------------------------------------------
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

// ---- logging / reports (helper thread only) ---------------------------------------------------------------
void logf(const char* fmt, ...) {
    FILE* f = fopen("perflab_log.txt", "a");
    if (!f) return;
    fprintf(f, "[%lu t%ld] ", GetTickCount(), g_ticks);
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

int cmpLive(const void* a, const void* b) {
    auto& x = g_sites[*static_cast<const int*>(a)]; auto& y = g_sites[*static_cast<const int*>(b)];
    LONG64 lx = x.bytes - x.freeBytes, ly = y.bytes - y.freeBytes; return lx < ly ? 1 : lx > ly ? -1 : 0;
}
int cmpTotal(const void* a, const void* b) {
    auto& x = g_sites[*static_cast<const int*>(a)]; auto& y = g_sites[*static_cast<const int*>(b)];
    return x.bytes < y.bytes ? 1 : x.bytes > y.bytes ? -1 : 0;
}
int cmpCount(const void* a, const void* b) {
    auto& x = g_sites[*static_cast<const int*>(a)]; auto& y = g_sites[*static_cast<const int*>(b)];
    return x.allocs < y.allocs ? 1 : x.allocs > y.allocs ? -1 : 0;
}

void writeTotals(FILE* f) {
    fprintf(f, "tick=%lu frames(hook)=%ld allocs=%lld frees=%lld reallocs=%lld live_bytes=%lld live_blocks=%lld total_bytes=%lld\n",
        GetTickCount(), g_ticks, (long long)g_allocN, (long long)g_freeN, (long long)g_reallocN, (long long)g_liveBytes, (long long)g_liveN, (long long)g_totalBytes);
    fprintf(f, "sites=%ld site_overflow=%ld dropped_ptr_inserts=%ld untracked_frees=%ld ptr_probe_max=%ld\n",
        g_siteCount, g_siteOverflow, g_dropped, g_untracked, g_ptrProbeMax);
    double cyc = g_hookSamples ? double(g_hookCycles) / double(g_hookSamples) : 0;
    fprintf(f, "hook self cost: %.0f cycles/call (sampled 1/64, %lld samples)\n", cyc, (long long)g_hookSamples);
}

void writeTop(FILE* f, const char* title, int (*cmp)(const void*, const void*), int n) {
    static int idx[SiteCap]; int m = 0;
    for (int i = 0; i < SiteCap; ++i) if (g_sites[i].key) idx[m++] = i;
    qsort(idx, m, sizeof(int), cmp);
    fprintf(f, "\n== top %d sites by %s (a1 a2 | allocs total_bytes live_bytes live_blocks reallocs | hist<=16,32,..)\n", n, title);
    for (int r = 0; r < m && r < n; ++r) {
        Site& s = g_sites[idx[r]];
        uint64_t k = static_cast<uint64_t>(s.key);
        fprintf(f, "%08x %08x | %lld %lld %lld %lld %lld |", (unsigned)k, (unsigned)(k >> 32), (long long)s.allocs, (long long)s.bytes,
            (long long)(s.bytes - s.freeBytes), (long long)(s.allocs - s.frees), (long long)s.reallocs);
        for (int b = 0; b < Hist; ++b) fprintf(f, " %ld", s.hist[b]);
        fputc('\n', f);
    }
}

void writeReport() {
    FILE* f = fopen("perflab_alloc.tmp", "w");
    if (!f) return;
    writeTotals(f);
    writeTop(f, "live bytes", cmpLive, 60);
    writeTop(f, "total bytes", cmpTotal, 40);
    writeTop(f, "alloc count", cmpCount, 40);
    fclose(f);
    remove("perflab_alloc.txt");
    rename("perflab_alloc.tmp", "perflab_alloc.txt");
}

int g_snapN = 0;
void writeSnapshot(const char* ev) {
    FILE* f = fopen("perflab_alloc_snaps.txt", "a");
    if (!f) return;
    fprintf(f, "#SNAP %d %s ", ++g_snapN, ev);
    writeTotals(f);
    for (int i = 0; i < SiteCap; ++i) {
        Site& s = g_sites[i];
        if (!s.key) continue;
        LONG64 live = s.bytes - s.freeBytes;
        if (live < static_cast<LONG64>(g_cfg.snapMin)) continue;
        uint64_t k = static_cast<uint64_t>(s.key);
        fprintf(f, "S %08x %08x %lld %lld\n", (unsigned)k, (unsigned)(k >> 32), (long long)live, (long long)(s.allocs - s.frees));
    }
    fclose(f);
}

bool loadScreenUp() {
    using saferead::readAt;
    saferead::beginScope();
    uintptr_t app = 0, client = 0, in = 0, ls = 0, vt = 0, mgr = 0, modals = 0; int cnt = 0;
    if (!readAt(1, AppGlobal, 0, &app) || !app || !readAt(2, app, 4, &client) || !client || !readAt(3, client, 4, &in) || !in) return false;
    if (!readAt(4, in, In_LoadScreen, &ls) || !ls || !readAt(5, ls, 0, &vt) || vt != LoadScreenVtable) return false;
    if (!readAt(6, in, In_GuiMgr, &mgr) || !mgr || !readAt(7, mgr, Mgr_Modals, &modals) || !readAt(8, mgr, Mgr_ModalCount, &cnt)) return false;
    if (!modals || cnt <= 0 || cnt >= 64) return false;
    for (int i = 0; i < cnt; ++i) { uintptr_t p = 0; if (readAt(9, modals, i * 4, &p) && p == ls) return true; }
    return false;
}

volatile LONG g_stop = 0;
volatile LONG g_snapNow = 0;
bool g_gameProcess = false;

DWORD WINAPI helper(LPVOID) {
    DWORD lastFlush = GetTickCount(); bool up = false; bool haveState = false;
    while (!g_stop) {
        Sleep(100);
        if (g_gameProcess) {
            bool nowUp = loadScreenUp();
            if (haveState && nowUp != up) writeSnapshot(nowUp ? "LOAD_UP" : "LOAD_DOWN");
            up = nowUp; haveState = true;
        }
        if (InterlockedExchange(&g_snapNow, 0)) writeSnapshot("MANUAL");
        if (GetFileAttributesA("perflab_dump.txt") != INVALID_FILE_ATTRIBUTES) { DeleteFileA("perflab_dump.txt"); writeReport(); writeSnapshot("MANUAL_FILE"); }
        if (GetTickCount() - lastFlush >= g_cfg.flushMs) { lastFlush = GetTickCount(); writeReport(); logf("live=%lldB blocks=%lld sites=%ld dropped=%ld", (long long)g_liveBytes, (long long)g_liveN, g_siteCount, g_dropped); }
    }
    writeReport();
    return 0;
}

void readCfg() {
    FILE* f = fopen("perflab_debug.txt", "r");
    if (!f) return;
    char line[128];
    while (fgets(line, sizeof line, f)) {
        char* eq = strchr(line, '='); if (!eq) continue; *eq = 0; long v = atol(eq + 1);
        if (!strcmp(line, "alloc")) g_cfg.alloc = v != 0;
        else if (!strcmp(line, "heapfilter")) g_cfg.heapFilter = v != 0;
        else if (!strcmp(line, "flush_ms") && v > 500) g_cfg.flushMs = v;
        else if (!strcmp(line, "scan") && v >= 4 && v <= 256) g_cfg.scan = v;
        else if (!strcmp(line, "snap_min") && v >= 0) g_cfg.snapMin = v;
    }
    fclose(f);
}

} // namespace

extern "C" {
// Detour on 0x00478A20 (per-frame function called from MainLoop): heartbeat only.
void __cdecl PerfLabTick() { InterlockedIncrement(&g_ticks); }
// For the smoke harness / other diagnostics.
void __cdecl PerfLabSnapshot() { InterlockedExchange(&g_snapNow, 1); }
int __cdecl PerfLabStats(long long* out, int n) {   // live_bytes, live_blocks, allocs, frees, untracked, dropped
    long long v[6] = { g_liveBytes, g_liveN, g_allocN, g_freeN, g_untracked, g_dropped };
    for (int i = 0; i < n && i < 6; ++i) out[i] = v[i];
    return 6;
}
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        if (GetFileAttributesA("perflab_debug.txt") == INVALID_FILE_ATTRIBUTES) return TRUE;
        readCfg();
        HMODULE exe = GetModuleHandleA(nullptr);
        g_exeLo = reinterpret_cast<uintptr_t>(exe);
        auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(exe);
        auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(g_exeLo + dos->e_lfanew);
        g_exeHi = g_exeLo + nt->OptionalHeader.SizeOfImage;
        g_gameProcess = g_exeLo == 0x00400000 && nt->OptionalHeader.SizeOfImage > 0x600000;
        logf("perf-lab 0.1.1 attach exe=%08x-%08x game=%d alloc=%d heapfilter=%d scan=%d", (unsigned)g_exeLo, (unsigned)g_exeHi, g_gameProcess, g_cfg.alloc, g_cfg.heapFilter, g_cfg.scan);
        if (g_cfg.alloc) {
            g_sites = static_cast<Site*>(VirtualAlloc(nullptr, sizeof(Site) * SiteCap, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            g_pk = static_cast<uint32_t*>(VirtualAlloc(nullptr, PtrCap * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            g_pv = static_cast<uint32_t*>(VirtualAlloc(nullptr, PtrCap * 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            if (!g_sites || !g_pk || !g_pv) { logf("table allocation FAILED (%lu); alloc census off", GetLastError()); g_cfg.alloc = false; }
        }
        if (g_cfg.alloc) {
            HMODULE k32 = GetModuleHandleA("kernel32.dll");
            rHeapSize = reinterpret_cast<HeapSize_t>(GetProcAddress(k32, "HeapSize"));
            void* o1 = nullptr; void* o2 = nullptr; void* o3 = nullptr;
            // resolve the originals first (through the IAT slot values), then flip all three
            bool a = patchIat(exe, "KERNEL32.dll", "HeapAlloc", reinterpret_cast<void*>(hkHeapAlloc), &o1);
            if (a) rHeapAlloc = reinterpret_cast<HeapAlloc_t>(o1);
            bool r = patchIat(exe, "KERNEL32.dll", "HeapReAlloc", reinterpret_cast<void*>(hkHeapReAlloc), &o2);
            if (r) rHeapReAlloc = reinterpret_cast<HeapReAlloc_t>(o2);
            bool fr = patchIat(exe, "KERNEL32.dll", "HeapFree", reinterpret_cast<void*>(hkHeapFree), &o3);
            if (fr) rHeapFree = reinterpret_cast<HeapFree_t>(o3);
            logf("IAT hooks: HeapAlloc=%d HeapReAlloc=%d HeapFree=%d (orig %p %p %p) tables: sites=%uKB ptrs=%uMB", a, r, fr, o1, o2, o3,
                 (unsigned)(sizeof(Site) * SiteCap >> 10), (unsigned)(PtrCap * 8 >> 20));
        }
        g_on = true;
        HANDLE t = CreateThread(nullptr, 0, helper, nullptr, 0, nullptr);
        if (t) CloseHandle(t); else logf("helper thread creation FAILED (%lu)", GetLastError());
    }
    return TRUE;
}
