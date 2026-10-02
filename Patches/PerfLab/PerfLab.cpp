// PerfLab 0.3.0 (Steam Aspyr build 6A522E71...). Diagnostics; the only behaviour changes are the
// flag-gated experiments tx=1 / g1skip=1 (off by default). Research program: patch_manager_mods/10_engine_optimization_research.md.
// 0.3.0 (Phase 4) adds, all counter-only, all output from the helper thread (keys below, default 1 when metrics=1):
//   live=1   LIVE line: engine live counters read from globals (textures, GL bytes, models, mesh bytes, gobs, nodes, scenes,
//            focus dummies, particle emitters); no hooks
//   br=1     BR: per-name tables (TGA resman loads, BuildAndStoreTexture, GUI image setters 0x414870/0x4160BD, texture-record
//            miss 0x427609) + a ring of the last 64 events, to name the in-game TGA reload churn (run A)
//   kt=1     U5/T2: async request starts 0x71447A, DestroyTable 0x726350 + still-in-demand branch 0x72650B, KT use-after-free
//            write 0x71306C (CRes flag 0x200), TXI-via-TPC 0x93BEA6, texture requeue 0x474000 by caller, FreeImage 0x425160
//            (C3: TPC/TGA, buffer present), ~CAurTextureBasic 0x423480, server UnloadModule 0x5315E0 (+ metrics snapshot UNLOAD)
//   vm=1     U1: RunScript 0x7022A0 (count, per-script names), ExecuteCommand 0x668FD0 (count, out-of-range ids), instruction
//            limit trip 0x6FFA76, DelayCommand 0x670370, ActionDoCommand entry 0x6699D0 / handoff 0x669A32 (leaks = diff)
//   ai=1     U3: AIMaster UpdateState 0x51D790 (ticks, list sizes per level), per-object update pre 0x51DC70 / post 0x51DC86 (time,
//            slow objects, O(n^2) pre-pass iterations derived as sum of the level list size per update), pathfind 0x55AB30 /
//            result 0x55AC26 (calls, pending/ok/fail, time), ApplyEffect 0x544210 / RemoveEffect 0x544640
//   misc=1   U6: net ring send 0x704060 (count, max len), overflow test site 0x7040E0, focus-dummy creation 0x464789, scene dtor 0x464D70
//   HOOKCOST is reported per handler.
//
// Inert unless "perflab_debug.txt" exists in the game dir (key=value lines, all optional):
//   alloc=1          1a allocation census (default 1 when the file exists)
//   heapfilter=1     only track the CRT heap (handle in the exe global 0x00A806C8); 0 = any heap (harness)
//   flush_ms=30000   perflab_alloc.txt rewrite interval (also the perflab_metrics.txt interval)
//   scan=48          stack dwords scanned per allocation for call-site attribution
//   snap_min=1024    per-site live bytes needed to appear in a load-screen snapshot
//   metrics=0        0.2.0 measurement pack M1-M7 (detour counters + SwapBuffers/gluBuild2DMipmaps IAT hooks):
//                    snapshots appended to perflab_metrics.txt every flush_ms, at load-screen UP/DOWN and on perflab_dump.txt
//   vq=0             M8 VirtualQuery address-space census (same trigger points), perflab_metrics.txt
//   tx=0             experiment TX (guarded, batch T1): at the first tick, BuildAndStoreTexture 0x4261CF `0F B6 C8 85 C9 75 10`
//                    -> JMP to a cave that also frees the CPU image of TPC textures after GL upload, except textures with a
//                    procedural controller ([tex+0x34] != 0, the only post-upload GetImage readers); TGA unchanged
//   g1skip=0         experiment G1: at the first tick, 0x4265F3 74 06 -> EB 06 (skip glFinish per texture upload)
//                    Both verify the original bytes first and refuse on mismatch; the arm is logged in perflab_log.txt.
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
struct Cfg {
    bool alloc = true, heapFilter = true; DWORD flushMs = 30000; int scan = 48; unsigned snapMin = 1024;
    bool metrics = false, vq = false, tx = false, g1skip = false;
    int live = -1, br = -1, kt = -1, vm = -1, misc = -1, ai = -1;   // 0.3.0 groups; -1 = follow metrics
} g_cfg;
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

// ---- 0.2.0 measurement pack (doc 10 "Runtime measurement pack"; sites verified with perf_lab_src/hookbytes.py) -------
// Hook bodies only add to counters in one VirtualAlloc'd block or write fixed slots; the helper thread formats them.
// CRes fields (R1): +4 data refcount (u16), +8 source (>>30), +0xC flags, +0x10 data, +0x14 key entry (restype = u16 at
// +0x1A), +0x18 size. The CRes pointer is read only after the same NULL test the engine makes and only where the engine
// itself dereferences it next; the key entry (not dereferenced by those functions) goes through SafeRead.
constexpr int RtUnknown = 10000, RtSlots = 10001;     // restype ids 0..9999; slot 10000 = no key entry / out of range
constexpr int EvSlots = 8192, CallerSlots = 64, TopRegions = 10;
constexpr uintptr_t FreeChunkEvictRet = 0x00712245;   // FreeResourceData call inside FreeChunk (eviction)
constexpr uintptr_t PatchTx = 0x004261CF, TxFree = 0x004261D6, TxSkip = 0x004261E6, PatchG1 = 0x004265F3;
// GetImage caller classes (T1): upload routines (only called from glImage, before the free) and the TextureController
// family (reads [ctrl+4]'s image after upload). [lo, hi) from the dump's function sizes.
struct Range { uintptr_t lo, hi; };
constexpr Range UploadRa[] = { { 0x432FA0, 0x432FA0 + 883 }, { 0x433320, 0x433320 + 1167 }, { 0x433CD0, 0x433CD0 + 1493 }, { 0x434350, 0x434350 + 1557 } };
constexpr Range CtrlRa[] = { { 0x48FF30, 0x48FF30 + 148 }, { 0x490710, 0x490710 + 219 }, { 0x4907F0, 0x4907F0 + 543 }, { 0x491C40, 0x491C40 + 431 },
                             { 0x492310, 0x492310 + 329 }, { 0x492460, 0x492460 + 871 } };
template <int N> bool inRanges(const Range (&r)[N], uintptr_t a) { for (auto& x : r) if (a >= x.lo && a < x.hi) return true; return false; }
constexpr uintptr_t G_FrameLimit = 0x00A1B710, G_SleepGate = 0x00A1B704, G_FrameLimitSrc = 0x00A7EB9C, G_GlFinishFlag = 0x009F6100;
constexpr uintptr_t G_ResMan = 0x00A1B490;

// 0.3.0 per-handler cost ids (HOOKCOST lines) and per-name tables (NAME lines).
enum Hk { HkMalloc, HkEvict, HkFreeData, HkRelease, HkDemand, HkGetImage, HkBuild, HkBucket, HkActionBar,
          HkBr1, HkBr2, HkBr3, HkAsync, HkDestroyTable, HkStillDemand, HkUafWrite, HkTxiTpc, HkRequeue, HkFreeImage, HkTexDtor,
          HkUnload, HkRunScript, HkExecCmd, HkTmi, HkDelay, HkAdcEnter, HkAdcHandoff, HkNetSend, HkNetOvf, HkFocus, HkSceneDtor,
          HkAiTick, HkAiPre, HkAiPost, HkPathEnter, HkPathResult, HkEffApply, HkEffRemove, HkCount };
const char* const HkName[HkCount] = { "ResMalloc", "ResEvict", "ResFreeData", "ResRelease", "ResDemand", "GetImage", "BuildTex", "Bucket",
          "ActionBar", "BrSetter1", "BrSetter2", "BrTexMiss", "AsyncStart", "DestroyTable", "StillInDemand", "KtUafWrite", "TxiViaTpc",
          "TexRequeue", "FreeImage", "TexDtor", "UnloadModule", "RunScript", "ExecCommand", "InstrLimit", "DelayCommand", "AdcEnter",
          "AdcHandoff", "NetSend", "NetOvfTest", "FocusDummy", "SceneDtor", "AiTick", "AiObjPre", "AiObjPost", "PathEnter", "PathResult",
          "EffectApply", "EffectRemove" };
enum Nt { NtTgaLoad, NtBuild, NtBr1, NtBr2, NtBr3, NtScript, NtTxiTpc, NtCount };
const char* const NtName[NtCount] = { "tga_load", "build", "gui_setter1", "gui_setter2", "tex_miss", "script", "txi_via_tpc" };
constexpr int NameSlots = 512, NameLen = 24, RingN = 64, ExecIds = 0x36D;
struct NameSlot { volatile LONG key, n; volatile LONG ra; char name[NameLen]; };
struct RingEv { volatile LONG tick, table, ra; char name[NameLen]; };
struct HookCost { volatile LONG64 cyc, n, calls; };

struct RtStat { volatile LONG64 allocN, allocB, evictN, evictB, freeN, freeB, relN, relZero, demN, redemN; };
struct CallerStat { volatile LONG key; volatile LONG n; };
struct EvSlot { volatile LONG cres, tick; };
struct Metrics {
    RtStat rt[RtSlots];
    volatile LONG64 mallocN, mallocOverBudget; volatile LONG budgetMin;              // M1 / J1
    EvSlot ev[EvSlots]; volatile LONG64 redemAll;
    CallerStat freeCallers[CallerSlots];
    volatile LONG64 waitBegin, waitEnd, waitQpc, waitMaxQpc, sleeps; volatile LONG waitHist[16];   // M2 (hist: sleeps per wait, log2)
    LONG64 waitT0; LONG waitSleeps0;                                                   // main-thread scratch
    volatile LONG64 getImgN, getImgTpc, getImgTpcRetained, getImgUpload, getImgCtrl, getImgCtrlTpcRetained, getImgOther, getImgOtherTpcRetained;  // M3
    CallerStat getImgCallers[CallerSlots];                                             // non-upload callers only
    volatile LONG64 buildN, buildRequeueTpcRetained, buildTpcCtrl;                     // C1/C2 (T1): BuildAndStoreTexture entries
    volatile LONG64 glImageN, glFinishN, glFinishQpc, glFinishMaxQpc, mipN, mipQpc, mipPixels; LONG64 glT0;  // M4
    volatile LONG64 frames, bucketCalls, actionBarCalls, frameMaxQpc; volatile LONG frameHist[10], allocHist[16];  // M5
    LONG64 lastSwap, lastAllocN, lastBucket, lastActionBar; volatile LONG maxBucketPerFrame, maxActionBarPerFrame;
    volatile LONG64 movieCalls, movieSpins; volatile LONG spinHist[24], cutoffSeen, lastThis254; LONG64 spins0;  // M6
    volatile LONG64 hookCycles, hookSamples;
    // 0.3.0
    HookCost hk[HkCount];
    NameSlot nt[NtCount][NameSlots]; volatile LONG ntOverflow[NtCount];
    RingEv ring[RingN]; volatile LONG ringN;
    volatile LONG64 asyncStart, asyncBytes, destroyTable, stillDemand, uafWrite, releaseOk, txiTpc;
    CallerStat requeueCallers[CallerSlots]; volatile LONG64 requeueN, requeueBuilt;
    volatile LONG64 freeImgN, freeImgTpc, freeImgTpcBuf, freeImgTgaBuf, texDtor, unloadN;
    volatile LONG64 runScript, execCmd, execBad, tmi, delayCmd, adcEnter, adcHandoff; volatile LONG execHist[ExecIds];
    volatile LONG64 netSend, netBytes, netOvfTest, netOvfHit, focusDummy, sceneDtor; volatile LONG netMaxLen;
    // U3 (server thread = main thread in single player; scratch fields are plain)
    volatile LONG64 aiTicks, aiObj, aiObjQpc, aiObjMaxQpc, aiObjSlow, aiPrepass, aiTickQpc; volatile LONG aiMaxN, aiMaxNLevel, aiTickHist[8];
    uintptr_t aiMaster; LONG64 aiTickT0, aiLastPost, aiObjT0;
    volatile LONG64 pathN, pathPending, pathOk, pathFail, pathOther, pathQpc, pathMaxQpc; LONG64 pathT0;
    volatile LONG64 effApply, effRemove;
};
Metrics* g_m = nullptr;
LONG64 g_qpf = 1;
volatile LONG g_firstTickDone = 0;
volatile LONG g_txState = 0, g_g1State = 0;          // 0 not requested, 1 applied, -1 byte mismatch, -2 VirtualProtect failed, -3 cave alloc failed

inline LONG64 qpc() { LARGE_INTEGER v; QueryPerformanceCounter(&v); return v.QuadPart; }
// Histogram bucket for v with n buckets: 0 = zero, k = [2^(k-1), 2^k), last bucket open-ended.
inline int lbucket(uint64_t v, int n) { if (!v) return 0; int b = 1; while (v > 1 && b < n - 1) { v >>= 1; ++b; } return b; }
inline void atomicMax(volatile LONG64* p, LONG64 v) { LONG64 o = *p; while (v > o) { LONG64 r = InterlockedCompareExchange64(p, v, o); if (r == o) break; o = r; } }
inline void atomicMin(volatile LONG* p, LONG v) { LONG o = *p; while (v < o) { LONG r = InterlockedCompareExchange(p, v, o); if (r == o) break; o = r; } }

struct MTimer {   // counts calls per handler, samples 1 in 64 of them (handler body only; the KPM wrapper cost is not included)
    uint64_t t0 = 0; bool on; int id;
    explicit MTimer(int i) : id(i) { on = (InterlockedIncrement64(&g_m->hk[i].calls) & 63) == 0; if (on) t0 = __rdtsc(); }
    ~MTimer() {
        if (!on) return;
        LONG64 d = static_cast<LONG64>(__rdtsc() - t0);
        InterlockedExchangeAdd64(&g_m->hookCycles, d); InterlockedIncrement64(&g_m->hookSamples);
        InterlockedExchangeAdd64(&g_m->hk[id].cyc, d); InterlockedIncrement64(&g_m->hk[id].n);
    }
};

void countCaller(CallerStat* t, uintptr_t ret) {
    LONG key = static_cast<LONG>(ret ? ret : 1);
    uint32_t h = (static_cast<uint32_t>(key) * 2654435761u) >> 26;
    for (int i = 0; i < CallerSlots; ++i, h = (h + 1) & (CallerSlots - 1)) {
        LONG k = t[h].key;
        if (k == 0) { LONG prev = InterlockedCompareExchange(&t[h].key, key, 0); k = prev ? prev : key; }
        if (k == key) { InterlockedIncrement(&t[h].n); return; }
    }
    InterlockedIncrement(&t[0].n);                    // table full: lumped into slot 0
}

inline int restypeOf(uintptr_t cres) {
    uintptr_t ke = *reinterpret_cast<const uintptr_t*>(cres + 0x14);
    uint16_t t = 0;
    saferead::beginScope();
    if (!ke || !saferead::readAt(20, ke, 0x1A, &t)) return RtUnknown;
    return t < RtUnknown ? t : RtUnknown;
}
inline uint32_t evHash(uintptr_t p) { return ((static_cast<uint32_t>(p) >> 3) * 2654435761u) >> 19; }   // 13 bits

// 0.3.0 per-name counting. Copies up to `max` chars (stops at NUL; 16-byte CResRefs are not NUL-terminated) through
// SafeRead, lower-cases, and bumps the slot keyed by FNV-1a of the name. The first writer of a slot copies the name;
// a racing reader may see a partially copied name in that one snapshot (cosmetic).
bool readName(uintptr_t src, int max, char* out) {
    if (!src) return false;
    uint32_t w[NameLen / 4] = {};
    int words = (max + 3) / 4; if (words > NameLen / 4) words = NameLen / 4;
    saferead::beginScope();
    for (int i = 0; i < words; ++i) if (!saferead::readAt(50, src, i * 4, &w[i])) { if (!i) return false; break; }
    const char* c = reinterpret_cast<const char*>(w);
    int n = 0;
    for (; n < max && n < NameLen - 1 && c[n]; ++n) { char ch = c[n]; out[n] = (ch >= 'A' && ch <= 'Z') ? ch + 32 : (ch >= 32 && ch < 127 ? ch : '?'); }
    out[n] = 0;
    return n > 0;
}
void countName(int table, uintptr_t src, int max, uintptr_t ra) {
    Metrics& m = *g_m;
    char nm[NameLen];
    if (!readName(src, max, nm)) { strcpy(nm, "?"); }
    uint32_t h = 2166136261u; for (const char* p = nm; *p; ++p) h = (h ^ static_cast<uint8_t>(*p)) * 16777619u;
    LONG key = static_cast<LONG>(h | 1);
    NameSlot* t = m.nt[table];
    for (int i = 0, s = h & (NameSlots - 1); i < NameSlots; ++i, s = (s + 1) & (NameSlots - 1)) {
        LONG k = t[s].key;
        if (k == 0) {
            LONG prev = InterlockedCompareExchange(&t[s].key, key, 0);
            if (prev == 0) { memcpy(t[s].name, nm, NameLen); k = key; } else k = prev;
        }
        if (k == key) { InterlockedIncrement(&t[s].n); t[s].ra = static_cast<LONG>(ra); goto ring; }
    }
    InterlockedIncrement(&m.ntOverflow[table]);
ring:
    if (table == NtScript || table == NtTxiTpc) return;   // high-rate tables stay out of the event ring
    LONG i = InterlockedIncrement(&m.ringN) - 1;
    RingEv& e = m.ring[i % RingN];
    e.tick = static_cast<LONG>(GetTickCount()); e.table = table; e.ra = static_cast<LONG>(ra); memcpy(e.name, nm, NameLen);
}
inline bool grp(int v) { return v < 0 ? g_cfg.metrics : v != 0; }

// IAT hooks (M4 mipmaps, M5 frame time)
typedef int(WINAPI* GluMip_t)(unsigned, int, int, int, unsigned, unsigned, const void*);
typedef BOOL(WINAPI* Swap_t)(HDC);
GluMip_t rGluMip; Swap_t rSwap;

int WINAPI hkGluBuild2DMipmaps(unsigned target, int comps, int w, int h, unsigned fmt, unsigned type, const void* data) {
    LONG64 t0 = qpc();
    int r = rGluMip(target, comps, w, h, fmt, type, data);
    if (g_m) {
        InterlockedIncrement64(&g_m->mipN); InterlockedExchangeAdd64(&g_m->mipQpc, qpc() - t0);
        if (w > 0 && h > 0) InterlockedExchangeAdd64(&g_m->mipPixels, static_cast<LONG64>(w) * h);
    }
    return r;
}
BOOL WINAPI hkSwapBuffers(HDC dc) {
    if (g_m) {   // called on the render (main) thread only: plain scratch fields
        Metrics& m = *g_m;
        LONG64 now = qpc();
        if (m.lastSwap) {
            LONG64 d = now - m.lastSwap;
            InterlockedIncrement(&m.frameHist[lbucket(static_cast<uint64_t>(d * 1000 / g_qpf), 10)]);   // 0 = <1 ms, 1 = 1 ms, 2 = 2-3, ..., 5 = 16-31, 9 = >=256 ms
            atomicMax(&m.frameMaxQpc, d);
            LONG64 a = g_allocN, bc = m.bucketCalls, ab = m.actionBarCalls;
            InterlockedIncrement(&m.allocHist[lbucket(static_cast<uint64_t>(a - m.lastAllocN), 16)]);
            if (bc - m.lastBucket > m.maxBucketPerFrame) m.maxBucketPerFrame = static_cast<LONG>(bc - m.lastBucket);
            if (ab - m.lastActionBar > m.maxActionBarPerFrame) m.maxActionBarPerFrame = static_cast<LONG>(ab - m.lastActionBar);
            m.lastAllocN = a; m.lastBucket = bc; m.lastActionBar = ab;
        } else { m.lastAllocN = g_allocN; m.lastBucket = m.bucketCalls; m.lastActionBar = m.actionBarCalls; }
        m.lastSwap = now;
        InterlockedIncrement64(&m.frames);
    }
    return rSwap(dc);
}

// Experiments, applied from the first tick (main thread: the engine cannot be executing the patched bytes then).
uint8_t g_txSeen[7], g_g1Seen[2];
uint8_t* g_txCave = nullptr;
int writeCode(uintptr_t at, const uint8_t* expect, const uint8_t* repl, int n, uint8_t* seen) {
    auto p = reinterpret_cast<uint8_t*>(at);
    memcpy(seen, p, n);
    if (memcmp(p, expect, n) != 0) return -1;
    DWORD old;
    if (!VirtualProtect(p, n, PAGE_EXECUTE_READWRITE, &old)) return -2;
    memcpy(p, repl, n);
    VirtualProtect(p, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, n);
    return 1;
}
inline void putRel32(uint8_t* at, uintptr_t target) { int32_t d = static_cast<int32_t>(target - (reinterpret_cast<uintptr_t>(at) + 4)); memcpy(at, &d, 4); }
// TX cave (EAX = IsTPCLoaded() result, [EBP-4] = texture; EDX is reloaded at 0x4261D6 and dead at the epilogue 0x4261E6):
//   movzx ecx,al / test ecx,ecx / jz free        ; not TPC: free as the engine always did
//   mov edx,[ebp-4] / cmp dword [edx+0x34],0 / jnz skip   ; TPC with a procedural controller: keep the image (engine behaviour)
//   free: jmp 0x4261D6                           ; TPC without controller: FreeImage (the experiment)
//   skip: jmp 0x4261E6
int applyTx() {
    static const uint8_t orig[7] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10 };
    g_txCave = static_cast<uint8_t*>(VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!g_txCave) return -3;
    static const uint8_t body[16] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x74, 0x09, 0x8B, 0x55, 0xFC, 0x83, 0x7A, 0x34, 0x00, 0x75, 0x05 };
    memcpy(g_txCave, body, 16);
    g_txCave[16] = 0xE9; putRel32(g_txCave + 17, TxFree);
    g_txCave[21] = 0xE9; putRel32(g_txCave + 22, TxSkip);
    FlushInstructionCache(GetCurrentProcess(), g_txCave, 26);
    uint8_t jmp[7] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90 };
    int32_t d = static_cast<int32_t>(reinterpret_cast<uintptr_t>(g_txCave) - (PatchTx + 5)); memcpy(jmp + 1, &d, 4);
    return writeCode(PatchTx, orig, jmp, 7, g_txSeen);
}
void applyExperiments() {
    static const uint8_t g1Orig[2] = { 0x74, 0x06 }, g1New[2] = { 0xEB, 0x06 };
    memcpy(g_txSeen, reinterpret_cast<const void*>(PatchTx), 7);   // logged even when the experiment is off (0.3.0)
    memcpy(g_g1Seen, reinterpret_cast<const void*>(PatchG1), 2);
    if (g_cfg.tx) g_txState = applyTx();
    if (g_cfg.g1skip) g_g1State = writeCode(PatchG1, g1Orig, g1New, 2, g_g1Seen);
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

// M7: one-shot engine globals (read on every metrics snapshot; cheap).
void writeGlobals(FILE* f) {
    using saferead::readAt;
    saferead::beginScope();
    float fl = 0; int sleepGate = 0, flSrc = 0, glf = 0; uintptr_t rm = 0; int b4 = 0, b8 = 0;
    bool ok1 = readAt(30, G_FrameLimit, 0, &fl), ok2 = readAt(31, G_SleepGate, 0, &sleepGate), ok3 = readAt(32, G_FrameLimitSrc, 0, &flSrc);
    bool ok4 = readAt(33, G_GlFinishFlag, 0, &glf), ok5 = readAt(34, G_ResMan, 0, &rm) && rm && readAt(35, rm, 4, &b4) && readAt(36, rm, 8, &b8);
    fprintf(f, "G a1b710=%g(%d) a1b704=%d(%d) a7eb9c=%d(%d) 9f6100=%d(%d) resman=%08x budget4=%d budget8=%d(%d)\n",
        fl, ok1, sleepGate, ok2, flSrc, ok3, glf, ok4, (unsigned)rm, b4, b8, ok5);
}

// M8: whole 4 GB address space by state/type, largest free block, top regions.
void writeVq(FILE* f) {
    struct R { uintptr_t base; SIZE_T size; DWORD state, type, prot; } top[TopRegions] = {};
    uint64_t by[3][4] = {};   // [commit, reserve, free][image, mapped, private, none]
    unsigned cnt[3] = {}; SIZE_T largestFree = 0; uintptr_t largestFreeAt = 0;
    MEMORY_BASIC_INFORMATION mbi;
    for (uintptr_t a = 0; VirtualQuery(reinterpret_cast<LPCVOID>(a), &mbi, sizeof mbi) == sizeof mbi; ) {
        int st = mbi.State == MEM_COMMIT ? 0 : mbi.State == MEM_RESERVE ? 1 : 2;
        int ty = mbi.Type == MEM_IMAGE ? 0 : mbi.Type == MEM_MAPPED ? 1 : mbi.Type == MEM_PRIVATE ? 2 : 3;
        by[st][ty] += mbi.RegionSize; ++cnt[st];
        if (st == 2 && mbi.RegionSize > largestFree) { largestFree = mbi.RegionSize; largestFreeAt = reinterpret_cast<uintptr_t>(mbi.BaseAddress); }
        if (st != 2) {   // keep the TopRegions largest non-free regions
            int k = TopRegions - 1;
            if (mbi.RegionSize > top[k].size) {
                while (k > 0 && top[k - 1].size < mbi.RegionSize) { top[k] = top[k - 1]; --k; }
                top[k] = { reinterpret_cast<uintptr_t>(mbi.BaseAddress), mbi.RegionSize, mbi.State, mbi.Type, mbi.Protect };
            }
        }
        uintptr_t next = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
        if (next <= a) break;
        a = next;
    }
    const char* sn[3] = { "commit", "reserve", "free" };
    fprintf(f, "VQ regions commit=%u reserve=%u free=%u largest_free=%uMB@%08x\n", cnt[0], cnt[1], cnt[2], (unsigned)(largestFree >> 20), (unsigned)largestFreeAt);
    for (int s = 0; s < 3; ++s)
        fprintf(f, "VQ %s MB image=%u mapped=%u private=%u none=%u\n", sn[s], (unsigned)(by[s][0] >> 20), (unsigned)(by[s][1] >> 20), (unsigned)(by[s][2] >> 20), (unsigned)(by[s][3] >> 20));
    for (int i = 0; i < TopRegions && top[i].size; ++i)
        fprintf(f, "VQTOP %08x %uKB state=%lx type=%lx prot=%lx\n", (unsigned)top[i].base, (unsigned)(top[i].size >> 10), top[i].state, top[i].type, top[i].prot);
}

void writeCallers(FILE* f, const char* tag, const CallerStat* t) {
    int idx[CallerSlots], m = 0;
    for (int i = 0; i < CallerSlots; ++i) if (t[i].key) idx[m++] = i;
    for (int i = 1; i < m; ++i) for (int j = i; j > 0 && t[idx[j]].n > t[idx[j - 1]].n; --j) { int x = idx[j]; idx[j] = idx[j - 1]; idx[j - 1] = x; }
    for (int i = 0; i < m && i < 16; ++i) fprintf(f, "%s %08x %ld\n", tag, (unsigned)t[idx[i]].key, t[idx[i]].n);
}

// 0.3.0 output (formats documented in perf_lab_src/README.md "perf-lab 0.3.0").
void writeLive(FILE* f) {
    using saferead::readAt;
    saferead::beginScope();
    struct G { const char* n; uintptr_t a; } gs[] = { { "textures", 0x00A1BBE0 }, { "gl_bytes_est", 0x00A1BBA0 }, { "models", 0x00A1FFAC },
        { "mesh_node_bytes", 0x00A1DEE0 }, { "gob_instances", 0x00A1FF68 }, { "node_objects", 0x00A1FF60 }, { "scenes", 0x00A200F4 },
        { "focus_dummies", 0x00A20030 }, { "emitters", 0x00A7BD28 } };
    fprintf(f, "LIVE");
    for (auto& g : gs) { int v = 0; if (readAt(60, g.a, 0, &v)) fprintf(f, " %s=%d", g.n, v); else fprintf(f, " %s=?", g.n); }
    fputc('\n', f);
}
void writeNames(FILE* f, int table) {
    NameSlot* t = g_m->nt[table];
    static int idx[NameSlots]; int m = 0;
    for (int i = 0; i < NameSlots; ++i) if (t[i].key) idx[m++] = i;
    for (int i = 1; i < m; ++i) for (int j = i; j > 0 && t[idx[j]].n > t[idx[j - 1]].n; --j) { int x = idx[j]; idx[j] = idx[j - 1]; idx[j - 1] = x; }
    fprintf(f, "NAMES %s distinct=%d overflow=%ld\n", NtName[table], m, g_m->ntOverflow[table]);
    for (int i = 0; i < m && i < 32; ++i) { char nm[NameLen]; memcpy(nm, t[idx[i]].name, NameLen); nm[NameLen - 1] = 0; fprintf(f, "NAME %s %ld %s ra=%08x\n", NtName[table], t[idx[i]].n, nm, (unsigned)t[idx[i]].ra); }
}
void writeMetrics3(FILE* f, Metrics& m) {
    for (int i = 0; i < HkCount; ++i)
        if (m.hk[i].calls) fprintf(f, "HOOKCOST_BY %s calls=%lld cycles=%.0f samples=%lld\n", HkName[i], (long long)m.hk[i].calls,
                                    m.hk[i].n ? double(m.hk[i].cyc) / double(m.hk[i].n) : 0.0, (long long)m.hk[i].n);
    if (grp(g_cfg.live)) writeLive(f);
    if (grp(g_cfg.br)) {
        for (int t = NtTgaLoad; t <= NtBr3; ++t) writeNames(f, t);
        LONG n = m.ringN; int k = n < RingN ? n : RingN;
        for (int i = 0; i < k; ++i) {
            RingEv& e = m.ring[(n - k + i) % RingN]; char nm[NameLen]; memcpy(nm, e.name, NameLen); nm[NameLen - 1] = 0;
            fprintf(f, "BRRING %lu %s %s ra=%08x\n", (unsigned long)e.tick, NtName[e.table], nm, (unsigned)e.ra);
        }
    }
    if (grp(g_cfg.kt)) {
        fprintf(f, "KT async_start=%lld async_bytes=%lld destroy_table=%lld still_in_demand=%lld uaf_write=%lld release_ok=%lld txi_via_tpc=%lld unload=%lld\n",
            (long long)m.asyncStart, (long long)m.asyncBytes, (long long)m.destroyTable, (long long)m.stillDemand, (long long)m.uafWrite,
            (long long)m.releaseOk, (long long)m.txiTpc, (long long)m.unloadN);
        fprintf(f, "TEXLIFE requeue=%lld requeue_built=%lld freeimage=%lld freeimage_tpc=%lld freeimage_tpc_buf=%lld freeimage_tga_buf=%lld tex_dtor=%lld\n",
            (long long)m.requeueN, (long long)m.requeueBuilt, (long long)m.freeImgN, (long long)m.freeImgTpc, (long long)m.freeImgTpcBuf,
            (long long)m.freeImgTgaBuf, (long long)m.texDtor);
        writeCallers(f, "REQUEUECALLER", m.requeueCallers);
        writeNames(f, NtTxiTpc);
    }
    if (grp(g_cfg.vm)) {
        fprintf(f, "VM run_script=%lld exec_cmd=%lld exec_bad_id=%lld instr_limit=%lld delay_cmd=%lld adc_enter=%lld adc_handoff=%lld adc_leak_upper=%lld\n",
            (long long)m.runScript, (long long)m.execCmd, (long long)m.execBad, (long long)m.tmi, (long long)m.delayCmd, (long long)m.adcEnter,
            (long long)m.adcHandoff, (long long)(m.adcEnter - m.adcHandoff));
        int top[16], tn = 0;   // 16 most used command ids, descending
        for (int i = 0; i < ExecIds; ++i) {
            LONG v = m.execHist[i];
            if (!v || (tn == 16 && v <= m.execHist[top[15]])) continue;
            int k = tn < 16 ? tn++ : 15;
            while (k > 0 && v > m.execHist[top[k - 1]]) { top[k] = top[k - 1]; --k; }
            top[k] = i;
        }
        fprintf(f, "VMCMD"); for (int i = 0; i < tn; ++i) fprintf(f, " %d:%ld", top[i], m.execHist[top[i]]); fputc('\n', f);
        writeNames(f, NtScript);
    }
    if (grp(g_cfg.ai)) {
        auto ms = [](LONG64 q) { return static_cast<double>(q) * 1000.0 / static_cast<double>(g_qpf); };
        fprintf(f, "AI ticks=%lld max_n=%ld max_n_level=%ld obj_updates=%lld obj_ms=%.1f obj_max_ms=%.2f obj_slow10ms=%lld prepass_iter=%lld tick_ms_est=%.1f tick_hist",
            (long long)m.aiTicks, m.aiMaxN, m.aiMaxNLevel, (long long)m.aiObj, ms(m.aiObjQpc), ms(m.aiObjMaxQpc), (long long)m.aiObjSlow, (long long)m.aiPrepass, ms(m.aiTickQpc));
        for (int i = 0; i < 8; ++i) fprintf(f, " %ld", m.aiTickHist[i]);
        fprintf(f, "\nPATH calls=%lld pending=%lld ok=%lld fail=%lld other=%lld ms=%.1f max_ms=%.2f | EFFECT apply=%lld remove=%lld\n", (long long)m.pathN,
            (long long)m.pathPending, (long long)m.pathOk, (long long)m.pathFail, (long long)m.pathOther, ms(m.pathQpc), ms(m.pathMaxQpc), (long long)m.effApply, (long long)m.effRemove);
    }
    if (grp(g_cfg.misc))
        fprintf(f, "NET send=%lld bytes=%lld max_len=%ld ovf_test=%lld ovf_hit=%lld | SCENE focus_dummy=%lld scene_dtor=%lld\n", (long long)m.netSend,
            (long long)m.netBytes, m.netMaxLen, (long long)m.netOvfTest, (long long)m.netOvfHit, (long long)m.focusDummy, (long long)m.sceneDtor);
}

int g_metricsN = 0;
void writeMetrics(const char* ev) {
    if (!g_m && !g_cfg.vq) return;
    FILE* f = fopen("perflab_metrics.txt", "a");
    if (!f) return;
    auto ms = [](LONG64 q) { return static_cast<double>(q) * 1000.0 / static_cast<double>(g_qpf); };
    fprintf(f, "#METRICS %d %s tick=%lu frames(hook)=%ld live_bytes=%lld allocs=%lld tx=%ld g1skip=%ld\n", ++g_metricsN, ev, GetTickCount(), g_ticks,
        (long long)g_liveBytes, (long long)g_allocN, g_txState, g_g1State);
    writeGlobals(f);
    if (g_m) {
        Metrics& m = *g_m;
        fprintf(f, "RES malloc=%lld over_budget=%lld budget_min=%ld redemand_60s=%lld\n", (long long)m.mallocN, (long long)m.mallocOverBudget, m.budgetMin, (long long)m.redemAll);
        fprintf(f, "# RT restype allocN allocB evictN evictB freeN freeB relN relZero demN redem60 resident\n");
        for (int i = 0; i < RtSlots; ++i) {
            RtStat& r = m.rt[i];
            if (!r.allocN && !r.freeN && !r.demN && !r.relN) continue;
            fprintf(f, "RT %d %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\n", i == RtUnknown ? -1 : i,
                (long long)r.allocN, (long long)r.allocB, (long long)r.evictN, (long long)r.evictB, (long long)r.freeN, (long long)r.freeB,
                (long long)r.relN, (long long)r.relZero, (long long)r.demN, (long long)r.redemN, (long long)(r.allocB - r.freeB));
        }
        writeCallers(f, "FREECALLER", m.freeCallers);
        fprintf(f, "WAIT begin=%lld end=%lld total_ms=%.1f max_ms=%.2f sleeps=%lld hist", (long long)m.waitBegin, (long long)m.waitEnd, ms(m.waitQpc), ms(m.waitMaxQpc), (long long)m.sleeps);
        for (int i = 0; i < 16; ++i) fprintf(f, " %ld", m.waitHist[i]);
        fprintf(f, "\nGETIMG calls=%lld tpc=%lld tpc_retained=%lld upload=%lld ctrl=%lld ctrl_tpc_retained=%lld other=%lld other_tpc_retained=%lld\n",
            (long long)m.getImgN, (long long)m.getImgTpc, (long long)m.getImgTpcRetained, (long long)m.getImgUpload, (long long)m.getImgCtrl,
            (long long)m.getImgCtrlTpcRetained, (long long)m.getImgOther, (long long)m.getImgOtherTpcRetained);
        fprintf(f, "BUILD calls=%lld requeue_tpc_retained=%lld tpc_with_ctrl=%lld\n", (long long)m.buildN, (long long)m.buildRequeueTpcRetained, (long long)m.buildTpcCtrl);
        writeCallers(f, "GETIMGCALLER", m.getImgCallers);
        fprintf(f, "GL glimage=%lld glfinish=%lld glfinish_ms=%.1f glfinish_max_ms=%.2f mipmaps=%lld mip_ms=%.1f mip_mpix=%.2f\n", (long long)m.glImageN,
            (long long)m.glFinishN, ms(m.glFinishQpc), ms(m.glFinishMaxQpc), (long long)m.mipN, ms(m.mipQpc), m.mipPixels / 1e6);
        fprintf(f, "FRAME frames=%lld max_ms=%.1f bucket_calls=%lld max_bucket_per_frame=%ld actionbar_calls=%lld max_actionbar_per_frame=%ld hist_ms",
            (long long)m.frames, ms(m.frameMaxQpc), (long long)m.bucketCalls, m.maxBucketPerFrame, (long long)m.actionBarCalls, m.maxActionBarPerFrame);
        for (int i = 0; i < 10; ++i) fprintf(f, " %ld", m.frameHist[i]);
        fprintf(f, " allocs_per_frame_hist");
        for (int i = 0; i < 16; ++i) fprintf(f, " %ld", m.allocHist[i]);
        fprintf(f, "\nMOVIE calls=%lld spins=%lld pump_cutoff_seen=%ld last_this254=%ld spin_hist", (long long)m.movieCalls, (long long)m.movieSpins, m.cutoffSeen, m.lastThis254);
        for (int i = 0; i < 24; ++i) fprintf(f, " %ld", m.spinHist[i]);
        double cyc = m.hookSamples ? double(m.hookCycles) / double(m.hookSamples) : 0;
        fprintf(f, "\nHOOKCOST %.0f cycles/call (sampled 1/64, %lld samples)\n", cyc, (long long)m.hookSamples);
        writeMetrics3(f, m);
    }
    if (g_cfg.vq) writeVq(f);
    fprintf(f, "#END %d\n", g_metricsN);
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
volatile LONG g_snapNow = 0, g_unloadNow = 0;
bool g_gameProcess = false;

void logExperiments() {
    auto st = [](LONG v) { return v == 1 ? "ON" : v == 0 ? "OFF" : v == -1 ? "REFUSED(bytes)" : v == -2 ? "REFUSED(protect)" : "REFUSED(cave alloc)"; };
    logf("experiments at first tick: tx=%s (0x4261CF saw %02x %02x %02x %02x %02x %02x %02x, cave %p) g1skip=%s (0x4265F3 saw %02x %02x)", st(g_txState),
         g_txSeen[0], g_txSeen[1], g_txSeen[2], g_txSeen[3], g_txSeen[4], g_txSeen[5], g_txSeen[6], (void*)g_txCave, st(g_g1State), g_g1Seen[0], g_g1Seen[1]);
}

DWORD WINAPI helper(LPVOID) {
    DWORD lastFlush = GetTickCount(); bool up = false; bool haveState = false, loggedExp = false;
    while (!g_stop) {
        Sleep(100);
        if (!loggedExp && g_firstTickDone) { loggedExp = true; logExperiments(); }
        if (g_gameProcess) {
            bool nowUp = loadScreenUp();
            if (haveState && nowUp != up) { writeSnapshot(nowUp ? "LOAD_UP" : "LOAD_DOWN"); writeMetrics(nowUp ? "LOAD_UP" : "LOAD_DOWN"); }
            up = nowUp; haveState = true;
        }
        if (InterlockedExchange(&g_snapNow, 0)) { writeSnapshot("MANUAL"); writeMetrics("MANUAL"); }
        if (InterlockedExchange(&g_unloadNow, 0)) { writeSnapshot("UNLOAD"); writeMetrics("UNLOAD"); }
        if (GetFileAttributesA("perflab_dump.txt") != INVALID_FILE_ATTRIBUTES) {
            DeleteFileA("perflab_dump.txt"); writeReport(); writeSnapshot("MANUAL_FILE"); writeMetrics("MANUAL_FILE");
        }
        if (GetTickCount() - lastFlush >= g_cfg.flushMs) {
            lastFlush = GetTickCount(); writeReport(); writeMetrics("FLUSH");
            logf("live=%lldB blocks=%lld sites=%ld dropped=%ld", (long long)g_liveBytes, (long long)g_liveN, g_siteCount, g_dropped);
        }
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
        else if (!strcmp(line, "metrics")) g_cfg.metrics = v != 0;
        else if (!strcmp(line, "vq")) g_cfg.vq = v != 0;
        else if (!strcmp(line, "tx")) g_cfg.tx = v != 0;
        else if (!strcmp(line, "g1skip")) g_cfg.g1skip = v != 0;
        else if (!strcmp(line, "live")) g_cfg.live = v != 0;
        else if (!strcmp(line, "br")) g_cfg.br = v != 0;
        else if (!strcmp(line, "kt")) g_cfg.kt = v != 0;
        else if (!strcmp(line, "vm")) g_cfg.vm = v != 0;
        else if (!strcmp(line, "misc")) g_cfg.misc = v != 0;
        else if (!strcmp(line, "ai")) g_cfg.ai = v != 0;
    }
    fclose(f);
}

} // namespace

extern "C" {
// Detour on 0x00478A20 (per-frame function called from MainLoop): heartbeat; the first call applies the experiments.
void __cdecl PerfLabTick() {
    if (InterlockedIncrement(&g_ticks) == 1 && g_on) { applyExperiments(); g_firstTickDone = 1; }
}
// For the smoke harness / other diagnostics.
void __cdecl PerfLabSnapshot() { InterlockedExchange(&g_snapNow, 1); }
int __cdecl PerfLabStats(long long* out, int n) {   // live_bytes, live_blocks, allocs, frees, untracked, dropped
    long long v[6] = { g_liveBytes, g_liveN, g_allocN, g_freeN, g_untracked, g_dropped };
    for (int i = 0; i < n && i < 6; ++i) out[i] = v[i];
    return 6;
}

// ---- M1: CExoResMan (all thiscall/stdcall entries; the hook sees the stack at entry) ----
// 0x712F30 Malloc(this=ECX, CRes* [esp+4]): allocation of [CRes+0x18] bytes; J1: size > budget [this+8] -> FreeChunk passes.
void __cdecl PerfLabResMalloc(void* resman, void* cres) {
    if (!g_m || !cres) return;
    MTimer t(HkMalloc);
    auto c = reinterpret_cast<uintptr_t>(cres);
    int size = *reinterpret_cast<const int*>(c + 0x18);
    int rt = restypeOf(c);
    RtStat& r = g_m->rt[rt];
    InterlockedIncrement64(&r.allocN); InterlockedExchangeAdd64(&r.allocB, size);
    // BR: TGA (restype 3) loads by resref; key entry +0 = 16-byte CResRef (DestroyTable 0x726350 prints it from there)
    if (rt == 3 && grp(g_cfg.br)) countName(NtTgaLoad, *reinterpret_cast<const uintptr_t*>(c + 0x14), 16, 0);
    InterlockedIncrement64(&g_m->mallocN);
    if (resman) {
        int budget = *reinterpret_cast<const int*>(reinterpret_cast<uintptr_t>(resman) + 8);
        if (size > budget) InterlockedIncrement64(&g_m->mallocOverBudget);
        atomicMin(&g_m->budgetMin, budget);
    }
}
// 0x71222D inside FreeChunk, after the evicted CRes's vtbl[3] call and before FreeResourceData: CRes = [EBP-4].
void __cdecl PerfLabResEvict(void* cres) {
    if (!g_m || !cres) return;
    MTimer t(HkEvict);
    auto c = reinterpret_cast<uintptr_t>(cres);
    RtStat& r = g_m->rt[restypeOf(c)];
    InterlockedIncrement64(&r.evictN); InterlockedExchangeAdd64(&r.evictB, *reinterpret_cast<const int*>(c + 0x18));
    EvSlot& e = g_m->ev[evHash(c)];
    e.cres = static_cast<LONG>(c); e.tick = static_cast<LONG>(GetTickCount());
}
// 0x7120C0 FreeResourceData(this=ECX, CRes* [esp+4]), caller = [esp+0]. Mirrors the engine's own test for a real free.
void __cdecl PerfLabResFreeData(void* cres, void* ret) {
    if (!g_m || !cres) return;
    MTimer t(HkFreeData);
    auto c = reinterpret_cast<uintptr_t>(cres);
    if (!*reinterpret_cast<const uintptr_t*>(c + 0x10) || (*reinterpret_cast<const uint32_t*>(c + 8) >> 30) == 1) return;
    RtStat& r = g_m->rt[restypeOf(c)];
    InterlockedIncrement64(&r.freeN); InterlockedExchangeAdd64(&r.freeB, *reinterpret_cast<const int*>(c + 0x18));
    countCaller(g_m->freeCallers, reinterpret_cast<uintptr_t>(ret));
}
// 0x7130B0 Release(CRes* [esp+4]): refcount 1 -> 0 puts the CRes on the evict list.
void __cdecl PerfLabResRelease(void* cres) {
    if (!g_m || !cres) return;
    MTimer t(HkRelease);
    auto c = reinterpret_cast<uintptr_t>(cres);
    RtStat& r = g_m->rt[restypeOf(c)];
    InterlockedIncrement64(&r.relN);
    if (*reinterpret_cast<const uint16_t*>(c + 4) == 1) InterlockedIncrement64(&r.relZero);
}
// 0x711C20 Demand(this=ECX, CRes* [esp+4]): count by restype; J1 thrash = Demand of a CRes evicted < 60 s ago.
void __cdecl PerfLabResDemand(void* cres) {
    if (!g_m || !cres) return;
    MTimer t(HkDemand);
    auto c = reinterpret_cast<uintptr_t>(cres);
    RtStat& r = g_m->rt[restypeOf(c)];
    InterlockedIncrement64(&r.demN);
    EvSlot& e = g_m->ev[evHash(c)];
    if (e.cres == static_cast<LONG>(c)) {
        if (static_cast<DWORD>(GetTickCount() - static_cast<DWORD>(e.tick)) < 60000) { InterlockedIncrement64(&r.redemN); InterlockedIncrement64(&g_m->redemAll); }
        e.cres = 0;
    }
}

// ---- M2: Demand's wait on an in-flight async CRes (main thread) ----
// 0x711D04: flag 0x10 branch entered (wait begins if resman+0x2C != 0); 0x711D27: one Sleep(1) poll; 0x711D44: wait ended OK.
void __cdecl PerfLabWaitBegin() {
    if (!g_m) return;
    InterlockedIncrement64(&g_m->waitBegin);
    g_m->waitT0 = qpc(); g_m->waitSleeps0 = static_cast<LONG>(g_m->sleeps);
}
void __cdecl PerfLabWaitSleep() { if (g_m) InterlockedIncrement64(&g_m->sleeps); }
void __cdecl PerfLabWaitEnd() {
    if (!g_m || !g_m->waitT0) return;
    LONG64 d = qpc() - g_m->waitT0; g_m->waitT0 = 0;
    InterlockedIncrement64(&g_m->waitEnd); InterlockedExchangeAdd64(&g_m->waitQpc, d); atomicMax(&g_m->waitMaxQpc, d);
    InterlockedIncrement(&g_m->waitHist[lbucket(static_cast<uint64_t>(static_cast<LONG>(g_m->sleeps) - g_m->waitSleeps0), 16)]);
}

// ---- M3: GetImage 0x422640 at 0x42266F, after it resolved this->vtbl[2]()->vtbl[1]() to the texture ([EBP-4], non-NULL;
// the engine reads its +0x44 next, so the fields below are read directly). Caller = [EBP+4].
// "TPC with a retained buffer after upload" (T1 F4): +0xE9 == 1 (TPC) && +0xE4 == 1 (built) && +0x58 != 0 (GL name) && +0x44 != 0.
// TX gate: other_tpc_retained == 0 (no reader outside the upload routines and the controller family, which the guard exempts).
void __cdecl PerfLabGetImage(void* tex, void* ret) {
    if (!g_m || !tex) return;
    MTimer t(HkGetImage);
    Metrics& m = *g_m;
    auto x = reinterpret_cast<uintptr_t>(tex); auto ra = reinterpret_cast<uintptr_t>(ret);
    bool tpc = *reinterpret_cast<const uint8_t*>(x + 0xE9) == 1;
    bool retained = tpc && *reinterpret_cast<const uint8_t*>(x + 0xE4) == 1 && *reinterpret_cast<const uint32_t*>(x + 0x58) != 0 &&
                    *reinterpret_cast<const uint32_t*>(x + 0x44) != 0;
    InterlockedIncrement64(&m.getImgN);
    if (tpc) InterlockedIncrement64(&m.getImgTpc);
    if (retained) InterlockedIncrement64(&m.getImgTpcRetained);
    if (inRanges(UploadRa, ra)) { InterlockedIncrement64(&m.getImgUpload); return; }
    countCaller(m.getImgCallers, ra);
    if (inRanges(CtrlRa, ra)) { InterlockedIncrement64(&m.getImgCtrl); if (retained) InterlockedIncrement64(&m.getImgCtrlTpcRetained); }
    else { InterlockedIncrement64(&m.getImgOther); if (retained) InterlockedIncrement64(&m.getImgOtherTpcRetained); }
}

// ---- C1/C2 (T1): BuildAndStoreTexture 0x4260E0 entry (ECX = texture). A built TPC texture that still has its buffer and is
// queued again is re-uploaded today; with tx=1 its buffer is gone and the re-upload is skipped (TX's only behaviour delta).
void __cdecl PerfLabBuildTex(void* tex) {
    if (!g_m || !tex) return;
    MTimer t(HkBuild);
    Metrics& m = *g_m;
    auto x = reinterpret_cast<uintptr_t>(tex);
    InterlockedIncrement64(&m.buildN);
    if (grp(g_cfg.br)) countName(NtBuild, x + 0x7C, NameLen - 1, 0);   // texture name, inline at +0x7C (FindTextureByName 0x4269F0)
    if (*reinterpret_cast<const uint8_t*>(x + 0xE9) == 1) {
        if (*reinterpret_cast<const uint8_t*>(x + 0xE4) == 1 && *reinterpret_cast<const uint32_t*>(x + 0x44) != 0) InterlockedIncrement64(&m.buildRequeueTpcRetained);
        if (*reinterpret_cast<const uint32_t*>(x + 0x34) != 0) InterlockedIncrement64(&m.buildTpcCtrl);
    }
}

// ---- M4: glFinish per texture upload (0x4265F5 call / 0x4265FB after; 0x4265FB also = every glImage call) ----
void __cdecl PerfLabGlFinishPre() { if (g_m) g_m->glT0 = qpc(); }
void __cdecl PerfLabGlImagePost() {
    if (!g_m) return;
    InterlockedIncrement64(&g_m->glImageN);
    if (g_m->glT0) {
        LONG64 d = qpc() - g_m->glT0; g_m->glT0 = 0;
        InterlockedIncrement64(&g_m->glFinishN); InterlockedExchangeAdd64(&g_m->glFinishQpc, d); atomicMax(&g_m->glFinishMaxQpc, d);
    }
}

// ---- M5: per-frame callers (counted; per-frame maxima taken at SwapBuffers) ----
void __cdecl PerfLabBucketInsert() { if (g_m) { MTimer t(HkBucket); InterlockedIncrement64(&g_m->bucketCalls); } }   // 0x472630 map find-or-insert
void __cdecl PerfLabActionBar() { if (g_m) { MTimer t(HkActionBar); InterlockedIncrement64(&g_m->actionBarCalls); } }   // 0x74CFE0 action bar rebuild

// ---- M6: PlayMovies 0x798820 entry / spin 0x798A87 ([EBP-0x20] = pump cut-off latch, [EBP-0x7C] = this) ----
void __cdecl PerfLabMovieEnter() {
    if (!g_m) return;
    Metrics& m = *g_m;
    if (m.movieCalls) InterlockedIncrement(&m.spinHist[lbucket(static_cast<uint64_t>(m.movieSpins - m.spins0), 24)]);   // previous call's spins
    m.spins0 = m.movieSpins;
    InterlockedIncrement64(&m.movieCalls);
}
void __cdecl PerfLabMovieSpin(int cutoff, void* self) {
    if (!g_m) return;
    InterlockedIncrement64(&g_m->movieSpins);
    if (cutoff) {
        g_m->cutoffSeen = 1;
        saferead::beginScope();
        int v = 0;
        if (saferead::readAt(40, reinterpret_cast<uintptr_t>(self), 0x254, &v)) g_m->lastThis254 = v;
    }
}

// Harness / manual: force a metrics snapshot on the helper thread.
void __cdecl PerfLabMetricsNow() { InterlockedExchange(&g_snapNow, 1); }

// ---- 0.3.0 BR (U4): GUI image setters, changed-name path (0x414870 / 0x4160BD: [EBP+8] = CResRef*, [EBP+4] = caller);
// texture-record miss in 0x427520 at 0x427609 ([EBP+8] = char* name, [EBP+4] = caller).
void __cdecl PerfLabBrSetter1(void* resref, void* ra) { if (g_m && grp(g_cfg.br)) { MTimer t(HkBr1); countName(NtBr1, reinterpret_cast<uintptr_t>(resref), 16, reinterpret_cast<uintptr_t>(ra)); } }
void __cdecl PerfLabBrSetter2(void* resref, void* ra) { if (g_m && grp(g_cfg.br)) { MTimer t(HkBr2); countName(NtBr2, reinterpret_cast<uintptr_t>(resref), 16, reinterpret_cast<uintptr_t>(ra)); } }
void __cdecl PerfLabBrTexMiss(void* name, void* ra) { if (g_m && grp(g_cfg.br)) { MTimer t(HkBr3); countName(NtBr3, reinterpret_cast<uintptr_t>(name), NameLen - 1, reinterpret_cast<uintptr_t>(ra)); } }

// ---- 0.3.0 KT / resman / textures (U5, T2) ----
// 0x71447A in CExoResMan::Update: OR ECX,0x10 = an async request starts; resman = [EBP-8], CRes = [resman+0x28].
void __cdecl PerfLabAsyncStart(void* resman) {
    if (!g_m || !grp(g_cfg.kt)) return;
    MTimer t(HkAsync);
    InterlockedIncrement64(&g_m->asyncStart);
    uintptr_t cres = 0; int size = 0;
    saferead::beginScope();
    if (saferead::readAt(51, reinterpret_cast<uintptr_t>(resman), 0x28, &cres) && saferead::readAt(52, cres, 0x18, &size)) InterlockedExchangeAdd64(&g_m->asyncBytes, size);
}
void __cdecl PerfLabDestroyTable() { if (g_m && grp(g_cfg.kt)) { MTimer t(HkDestroyTable); InterlockedIncrement64(&g_m->destroyTable); } }
// 0x72650B: DestroyTable's still-in-demand branch (ECX = CRes): CRes+0x14 keeps pointing into the array freed next (KT).
void __cdecl PerfLabStillInDemand(void* cres) { if (g_m && grp(g_cfg.kt) && cres) { MTimer t(HkStillDemand); InterlockedIncrement64(&g_m->stillDemand); } }
// 0x71306C in ReleaseResObject: about to write 0 to [[CRes+0x14]+0x10] (EAX = CRes). Flag 0x200 = the table died (KT UAF write).
void __cdecl PerfLabUafWrite(void* cres) {
    if (!g_m || !grp(g_cfg.kt) || !cres) return;
    MTimer t(HkUafWrite);
    if (*reinterpret_cast<const uint32_t*>(reinterpret_cast<uintptr_t>(cres) + 0xC) & 0x200) InterlockedIncrement64(&g_m->uafWrite);
    else InterlockedIncrement64(&g_m->releaseOk);   // normal path: key entry still valid
}
// 0x93BEA6: TXI provider falls back to Demanding the TPC to read its embedded TXI ([EBP+8] = char* name) (U5, TPC2X).
void __cdecl PerfLabTxiTpc(void* name) { if (g_m && grp(g_cfg.kt)) { MTimer t(HkTxiTpc); InterlockedIncrement64(&g_m->txiTpc); countName(NtTxiTpc, reinterpret_cast<uintptr_t>(name), NameLen - 1, 0); } }
// 0x474000 UniquePush entry (ECX = list, [esp+4] = texture, [esp+0] = caller); only the pending-texture list 0xA1BC2C.
void __cdecl PerfLabRequeue(void* list, void* tex, void* ra) {
    if (!g_m || !grp(g_cfg.kt) || reinterpret_cast<uintptr_t>(list) != 0x00A1BC2C || !tex) return;
    MTimer t(HkRequeue);
    InterlockedIncrement64(&g_m->requeueN);
    uint8_t e4 = 0; saferead::beginScope();
    if (saferead::readAt(53, reinterpret_cast<uintptr_t>(tex), 0xE4, &e4) && e4 == 1) InterlockedIncrement64(&g_m->requeueBuilt);
    countCaller(g_m->requeueCallers, reinterpret_cast<uintptr_t>(ra));
}
// C3: FreeImage 0x425160 entry (ECX = texture).
void __cdecl PerfLabFreeImage(void* tex) {
    if (!g_m || !grp(g_cfg.kt) || !tex) return;
    MTimer t(HkFreeImage);
    auto x = reinterpret_cast<uintptr_t>(tex);
    InterlockedIncrement64(&g_m->freeImgN);
    bool buf = *reinterpret_cast<const uint32_t*>(x + 0x44) != 0;   // FreeImage itself tests [this+0x44] first (0x42516C)
    if (*reinterpret_cast<const uint8_t*>(x + 0xE9) == 1) { InterlockedIncrement64(&g_m->freeImgTpc); if (buf) InterlockedIncrement64(&g_m->freeImgTpcBuf); }
    else if (buf) InterlockedIncrement64(&g_m->freeImgTgaBuf);
}
void __cdecl PerfLabTexDtor() { if (g_m && grp(g_cfg.kt)) { MTimer t(HkTexDtor); InterlockedIncrement64(&g_m->texDtor); } }
// Server UnloadModule 0x5315E0 entry: count + ask the helper for an UNLOAD metrics snapshot.
void __cdecl PerfLabUnload() {
    if (!g_m || !grp(g_cfg.kt)) return;
    MTimer t(HkUnload);
    InterlockedIncrement64(&g_m->unloadN); InterlockedExchange(&g_unloadNow, 1);
}

// ---- 0.3.0 NWScript VM (U1) ----
// RunScript 0x7022A0 (ECX = VMInternal, [esp+4] = CExoString* script; CExoString = { char* text; int len }).
void __cdecl PerfLabRunScript(void* str) {
    if (!g_m || !grp(g_cfg.vm)) return;
    MTimer t(HkRunScript);
    InterlockedIncrement64(&g_m->runScript);
    uintptr_t text = 0; saferead::beginScope();
    if (saferead::readAt(54, reinterpret_cast<uintptr_t>(str), 0, &text)) countName(NtScript, text, 16, 0);
}
// ExecuteCommand 0x668FD0 ([esp+4] = command id, signed; table has 0x36D entries; ids >= 0x8000 arrive negative: VMID).
void __cdecl PerfLabExecCmd(int id) {
    if (!g_m || !grp(g_cfg.vm)) return;
    MTimer t(HkExecCmd);
    InterlockedIncrement64(&g_m->execCmd);
    if (id < 0 || id >= ExecIds) InterlockedIncrement64(&g_m->execBad); else InterlockedIncrement(&g_m->execHist[id]);
}
void __cdecl PerfLabInstrLimit() { if (g_m && grp(g_cfg.vm)) { MTimer t(HkTmi); InterlockedIncrement64(&g_m->tmi); } }
void __cdecl PerfLabDelayCommand() { if (g_m && grp(g_cfg.vm)) { MTimer t(HkDelay); InterlockedIncrement64(&g_m->delayCmd); } }
void __cdecl PerfLabAdcEnter() { if (g_m && grp(g_cfg.vm)) { MTimer t(HkAdcEnter); InterlockedIncrement64(&g_m->adcEnter); } }
void __cdecl PerfLabAdcHandoff() { if (g_m && grp(g_cfg.vm)) { MTimer t(HkAdcHandoff); InterlockedIncrement64(&g_m->adcHandoff); } }

// ---- 0.3.0 net rings / scenes (U6) ----
// SendMessageToPlayer 0x704060 entry ([esp+0xC] = length).
void __cdecl PerfLabNetSend(int len) {
    if (!g_m || !grp(g_cfg.misc)) return;
    MTimer t(HkNetSend);
    InterlockedIncrement64(&g_m->netSend); if (len > 0) InterlockedExchangeAdd64(&g_m->netBytes, len);
    LONG o = g_m->netMaxLen; while (len > o) { LONG r = InterlockedCompareExchange(&g_m->netMaxLen, len, o); if (r == o) break; o = r; }
}
// 0x7040E0: [EBP-0x20] = ring object, [EBP-0x14] = new write index; the engine next tests new > [[obj+0x2000C]+0x20000] + 0x10000
// (unsigned) and on true logs "Message Buffer Overflow" and copies anyway (NETOVF). Same test here, read-only.
void __cdecl PerfLabNetOvfTest(void* obj, unsigned newW) {
    if (!g_m || !grp(g_cfg.misc)) return;
    MTimer t(HkNetOvf);
    InterlockedIncrement64(&g_m->netOvfTest);
    uintptr_t peer = 0; unsigned rd = 0; saferead::beginScope();
    if (saferead::readAt(55, reinterpret_cast<uintptr_t>(obj), 0x2000C, &peer) && saferead::readAt(56, peer, 0x20000, &rd) && newW > rd + 0x10000u)
        InterlockedIncrement64(&g_m->netOvfHit);
}
void __cdecl PerfLabFocusDummy() { if (g_m && grp(g_cfg.misc)) { MTimer t(HkFocus); InterlockedIncrement64(&g_m->focusDummy); } }
// ---- 0.3.0 AI / pathfinding / effects (U3) ----
// CServerAIMaster::UpdateState 0x51D790 entry (ECX = AI master; 5 priority lists, count of level L at [ecx+8+16*L]). The function's
// epilogue (0x51DFCD, 4 bytes) cannot be hooked, so the previous tick's time is estimated at the next entry as entry -> last object post.
void __cdecl PerfLabAiTick(void* aim) {
    if (!g_m || !grp(g_cfg.ai) || !aim) return;
    MTimer t(HkAiTick);
    Metrics& m = *g_m;
    LONG64 now = qpc();
    if (m.aiTickT0 && m.aiLastPost > m.aiTickT0) {
        LONG64 d = m.aiLastPost - m.aiTickT0; m.aiTickQpc += d;
        LONG us = static_cast<LONG>(d * 1000000 / g_qpf);   // buckets: <1, 1-2, 2-5, 5-10, 10-20, 20-75, 75-200, >=200 ms
        int b = us < 1000 ? 0 : us < 2000 ? 1 : us < 5000 ? 2 : us < 10000 ? 3 : us < 20000 ? 4 : us < 75000 ? 5 : us < 200000 ? 6 : 7;
        InterlockedIncrement(&m.aiTickHist[b]);
    }
    m.aiTickT0 = now; m.aiLastPost = 0; m.aiMaster = reinterpret_cast<uintptr_t>(aim);
    InterlockedIncrement64(&m.aiTicks);
    LONG total = 0; saferead::beginScope();
    for (int L = 0; L < 5; ++L) { int n = 0; if (saferead::readAt(57, m.aiMaster, 8 + 16 * L, &n) && n > 0 && n < 100000) { total += n; if (n > m.aiMaxNLevel) m.aiMaxNLevel = n; } }
    if (total > m.aiMaxN) m.aiMaxN = total;
}
// 0x51DC70: about to dispatch obj->vtbl[0x70] ([EBP-0x28] = object, [EBP-0x34] = level). Pre-pass work of this iteration = size of the level list.
void __cdecl PerfLabAiObjPre(void* obj, int level) {
    if (!g_m || !grp(g_cfg.ai) || !obj) return;
    MTimer t(HkAiPre);
    Metrics& m = *g_m;
    InterlockedIncrement64(&m.aiObj);
    if (m.aiMaster && level >= 0 && level < 5) { int n = 0; saferead::beginScope(); if (saferead::readAt(58, m.aiMaster, 8 + 16 * level, &n) && n > 0 && n < 100000) m.aiPrepass += n; }
    m.aiObjT0 = qpc();
}
// 0x51DC86: after the dispatch (also reached on the skip path with no object: only a pending pre stamp counts).
void __cdecl PerfLabAiObjPost() {
    if (!g_m || !grp(g_cfg.ai)) return;
    Metrics& m = *g_m;
    if (!m.aiObjT0) return;
    MTimer t(HkAiPost);
    LONG64 now = qpc(), d = now - m.aiObjT0; m.aiObjT0 = 0; m.aiLastPost = now;
    m.aiObjQpc += d; atomicMax(&m.aiObjMaxQpc, d);
    if (d * 100 > g_qpf) InterlockedIncrement64(&m.aiObjSlow);   // > 10 ms
}
// Pathfind 0x55AB30 entry / result store 0x55AC26 (EAX = result: 1 pending, 2 success, 3 fail).
void __cdecl PerfLabPathEnter() { if (g_m && grp(g_cfg.ai)) { MTimer t(HkPathEnter); InterlockedIncrement64(&g_m->pathN); g_m->pathT0 = qpc(); } }
void __cdecl PerfLabPathResult(int r) {
    if (!g_m || !grp(g_cfg.ai)) return;
    MTimer t(HkPathResult);
    Metrics& m = *g_m;
    if (r == 1) InterlockedIncrement64(&m.pathPending); else if (r == 2) InterlockedIncrement64(&m.pathOk); else if (r == 3) InterlockedIncrement64(&m.pathFail); else InterlockedIncrement64(&m.pathOther);
    if (m.pathT0) { LONG64 d = qpc() - m.pathT0; m.pathT0 = 0; m.pathQpc += d; atomicMax(&m.pathMaxQpc, d); }
}
void __cdecl PerfLabEffectApply() { if (g_m && grp(g_cfg.ai)) { MTimer t(HkEffApply); InterlockedIncrement64(&g_m->effApply); } }
void __cdecl PerfLabEffectRemove() { if (g_m && grp(g_cfg.ai)) { MTimer t(HkEffRemove); InterlockedIncrement64(&g_m->effRemove); } }

void __cdecl PerfLabSceneDtor() { if (g_m && grp(g_cfg.misc)) { MTimer t(HkSceneDtor); InterlockedIncrement64(&g_m->sceneDtor); } }
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
        { LARGE_INTEGER f; QueryPerformanceFrequency(&f); g_qpf = f.QuadPart ? f.QuadPart : 1; }
        logf("perf-lab 0.3.0 attach exe=%08x-%08x game=%d alloc=%d heapfilter=%d scan=%d metrics=%d vq=%d tx=%s g1skip=%s", (unsigned)g_exeLo, (unsigned)g_exeHi,
             g_gameProcess, g_cfg.alloc, g_cfg.heapFilter, g_cfg.scan, g_cfg.metrics, g_cfg.vq, g_cfg.tx ? "ON" : "OFF", g_cfg.g1skip ? "ON" : "OFF");
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
        if (g_cfg.metrics) {
            auto m = static_cast<Metrics*>(VirtualAlloc(nullptr, sizeof(Metrics), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            if (!m) logf("metrics block allocation FAILED (%lu); metrics off", GetLastError());
            else {
                m->budgetMin = 0x7FFFFFFF;
                void* o1 = nullptr; void* o2 = nullptr;
                bool s = patchIat(exe, "GDI32.dll", "SwapBuffers", reinterpret_cast<void*>(hkSwapBuffers), &o1);
                if (s) rSwap = reinterpret_cast<Swap_t>(o1);
                bool g = patchIat(exe, "GLU32.dll", "gluBuild2DMipmaps", reinterpret_cast<void*>(hkGluBuild2DMipmaps), &o2);
                if (g) rGluMip = reinterpret_cast<GluMip_t>(o2);
                g_m = m;   // publish last: detours see NULL until everything is set
                logf("metrics on: block=%uKB IAT SwapBuffers=%d gluBuild2DMipmaps=%d (orig %p %p) qpf=%lld", (unsigned)(sizeof(Metrics) >> 10), s, g, o1, o2, (long long)g_qpf);
            }
        }
        g_on = true;
        HANDLE t = CreateThread(nullptr, 0, helper, nullptr, 0, nullptr);
        if (t) CloseHandle(t); else logf("helper thread creation FAILED (%lu)", GetLastError());
    }
    return TRUE;
}
