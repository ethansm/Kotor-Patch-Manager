// Area Hook Probe 0.2.1 (Steam Aspyr build 6A522E71...). Diagnostics only: no gameplay change, spawns nothing.
// Question: when a SAVED game loads into an already-visited area, which engine paths run, and can the area resref/id be read then?
//
// Hooks (detour, 5-byte prologue 55 8B EC 6A FF, handler reads ECX only):
//   H1 0x00533830 CServerExoAppInternal::MainLoop (every tick). this+0x14 = area-ready latch (INFERRED, not a ready marker).
//      Logged on change only. On each 0 -> nonzero latch transition: one block dumping 0x40 dwords of `this` (capped at 8).
//      POP watch (armed by H2/H3): 30 consecutive ticks with no AddObjectToArea call (after at least one add) -> "H1 POP-STABLE" + one dump of `this`.
//      No stable window within 1800 ticks -> "H1 POP-TIMEOUT". Each armed watch resolves once.
//   H2 0x0055A460 CSWSModule::LoadModuleInProgress (FRESH area load: creates and loads the area).
//   H3 0x005582F0 CSWSModule::LoadModuleStart (SAVED-game module load; does NOT call LoadArea).
//   H4 0x00522BF0 CSWSArea::AddObjectToArea (ECX = area). Logged when the area pointer changes. Not an end-of-population
//      marker: it fires on the first add of each new area.
//      area+0x124 = object id (CONFIRMED), area+0x110 = 16-byte resref (INFERRED), prev_area_calls = AddObjectToArea calls
//      made for the previous area after its own log line, total_adds = all calls since attach.
//   H2 and H3 forget the last logged area pointer and arm the POP watch, so a reload that reuses a freed address is still logged.
//
// Output: area_hook_probe_log.txt via KpLog.h since 0.2.1 (channel main; core prefix `T=<s.ms> f=- area-hook-probe/main`, lines start `t<ticks> `;
//   old core: same file name as fallback at INFO, appended, the attached line marks each process start). Stack arguments
// of the hooked functions are never read. Every memory read goes through SafeRead (no faults, no unvalidated derefs).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "../_shared/SafeRead.h"
#include "KpLog.h"

KP_LOG_PATCH("area-hook-probe");

namespace {

using saferead::readAt;

constexpr const char* kVersion = "0.2.1";
constexpr const char* kLogName = "area_hook_probe_log.txt";
constexpr int kMaxLogLines = 2000;
constexpr int kMaxDumps = 8;
constexpr int kLatchOff = 0x14;        // CServerExoAppInternal area-ready latch (INFERRED)
constexpr int kDumpDwords = 0x40;      // dwords of `this` dumped per 0 -> nonzero latch transition
constexpr int kAreaIdOff = 0x124;      // CSWSArea object id (CONFIRMED)
constexpr int kAreaResrefOff = 0x110;  // CSWSArea resref, 16 bytes (INFERRED)
constexpr int kResrefLen = 16;
constexpr int kStableTicks = 30;       // POP watch: consecutive H1 ticks with no new AddObjectToArea call
constexpr long kPopTimeoutTicks = 1800; // POP watch: H1 ticks after arming before POP-TIMEOUT

// SafeRead site ids (the failure ring stays disabled; the ids only label the call site).
constexpr unsigned kSiteLatch = 1, kSiteDump = 2, kSiteAreaId = 3, kSiteAreaResref = 4;

// ---- state (all hooks run on the main thread) ---------------------------------------------------------
int g_logLines = 0;
long g_mainLoopTicks = 0;               // H1 calls since attach
DWORD g_attachTick = 0;
int g_dumps = 0;
const char* g_lastLoadHook = "none";    // last module-load hook that fired: H2 or H3

bool g_latchKnown = false;              // H1: a latch read has been made
bool g_latchOk = false;                 // H1: last read succeeded
int g_latchVal = 0;                     // H1: last readable latch value

uintptr_t g_lastArea = 0;               // H4: area pointer of the last logged distinct area (0 = none; reset by H2/H3)
long g_addsSincePrev = 0;               // H4: calls for that area after its log line
long g_addsTotal = 0;                   // H4: all calls since attach
char g_lastAreaId[16] = "none";         // H4: id of the last logged area ("none" until H4 after a load hook)
char g_lastAreaRef[128] = "none";       // H4: resref of the last logged area, fmtResref form

// POP watch: armed by H2/H3 at each module load; H1 polls it every tick
bool g_pendingArea = false;             // watch armed and not yet resolved
long g_ticksSinceLoad = 0;              // H1 ticks since the arming hook
long g_addsAtLastTick = 0;              // g_addsTotal as of the previous H1 tick
int g_stableTicks = 0;                  // consecutive H1 ticks with no new AddObjectToArea call

// ---- logging: state changes only ------------------------------------------------------------------------
// 0.2.1: lines go through KpLog.h (channel main): new core -> area_hook_probe_log.txt, old core -> fallback file of the same name at INFO.
// The core cuts a message at 232 bytes ("[...]"), so a long line is split at spaces into pieces of <= 220 bytes (continuations start with
// `+ `). Nothing is logged before KpLog_Init (KPatchInit logs the attached line first; hooks fire later).
KP_CHANNEL(chMain, "main");
const bool g_fallbackSet = (KpLog_SetFallbackFile(kLogName), KpLog_SetFallbackLevel(KPLOG_INFO), true);

constexpr int LogPiece = 220;

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

void logf(const char* fmt, ...) {
    if (g_logLines >= kMaxLogLines) return;
    ++g_logLines;
    char buf[640];
    int n = snprintf(buf, sizeof buf, "t%ld ", g_mainLoopTicks);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf + n, sizeof buf - n, fmt, ap);
    va_end(ap);
    emit(buf);
}

// "none" (never read), "unreadable", or the decimal value.
void fmtLatch(char* out, size_t n, bool known, bool ok, int v) {
    if (!known) snprintf(out, n, "none");
    else if (!ok) snprintf(out, n, "unreadable");
    else snprintf(out, n, "%d", v);
}

// Hex + ASCII when the resref is not a clean NUL-terminated printable string.
void fmtResref(char* out, size_t n, bool ok, const unsigned char* rf) {
    if (!ok) { snprintf(out, n, "unreadable"); return; }
    int len = 0;
    while (len < kResrefLen && rf[len]) ++len;
    if (len == 0) { snprintf(out, n, "(empty)"); return; }
    bool printable = true;
    for (int i = 0; i < len; ++i) if (rf[i] < 0x20 || rf[i] > 0x7e) { printable = false; break; }
    if (printable) { snprintf(out, n, "\"%.*s\"", len, reinterpret_cast<const char*>(rf)); return; }
    int p = snprintf(out, n, "hex[");
    for (int i = 0; i < kResrefLen; ++i) p += snprintf(out + p, n - p, "%02x%s", rf[i], i + 1 < kResrefLen ? " " : "");
    p += snprintf(out + p, n - p, "] ascii\"");
    for (int i = 0; i < kResrefLen && static_cast<size_t>(p) + 2 < n; ++i) {
        out[p++] = (rf[i] >= 0x20 && rf[i] < 0x7f) ? static_cast<char>(rf[i]) : '.';
    }
    out[p++] = '"';
    out[p] = '\0';
}

// One block: header line, then kDumpDwords dwords as hex, 8 per row. ???????? marks an unreadable dword.
void dumpThis(uintptr_t obj, int latch) {
    if (g_dumps >= kMaxDumps || g_logLines + 1 + kDumpDwords / 8 > kMaxLogLines) return;
    ++g_dumps;
    g_logLines += 1 + kDumpDwords / 8;
    char line[160];
    snprintf(line, sizeof line, "t%ld H1 DUMP #%d this=%08lx latch=%d (%d dwords, hex, offsets from this)",
             g_mainLoopTicks, g_dumps, static_cast<unsigned long>(obj), latch, kDumpDwords);
    emit(line);
    for (int row = 0; row < kDumpDwords; row += 8) {
        int n = snprintf(line, sizeof line, "  +0x%03x:", row * 4);
        for (int i = row; i < row + 8; ++i) {
            unsigned v = 0;
            if (readAt(kSiteDump, obj, i * 4, &v)) n += snprintf(line + n, sizeof line - n, " %08x", v);
            else n += snprintf(line + n, sizeof line - n, " ????????");
        }
        emit(line);
    }
}

}  // namespace

extern "C" {

// H1: CServerExoAppInternal::MainLoop (thiscall, ECX = this). Every tick: no output unless the latch changes.
void __cdecl AreaHookProbeMainLoop(void* self) {
    saferead::beginScope();
    uintptr_t obj = reinterpret_cast<uintptr_t>(self);
    ++g_mainLoopTicks;
    if (g_mainLoopTicks == 1) logf("H1 first MainLoop call: this=%08lx", static_cast<unsigned long>(obj));
    int latch = 0;
    bool ok = readAt(kSiteLatch, obj, kLatchOff, &latch);
    bool changed = !g_latchKnown || ok != g_latchOk || (ok && latch != g_latchVal);
    if (changed) {
        char was[16], now[16];
        fmtLatch(was, sizeof was, g_latchKnown, g_latchOk, g_latchVal);
        fmtLatch(now, sizeof now, true, ok, latch);
        logf("H1 latch this+0x%02x: %s -> %s (last load hook: %s, ms since attach %lu)",
             static_cast<unsigned>(kLatchOff), was, now, g_lastLoadHook,
             static_cast<unsigned long>(GetTickCount() - g_attachTick));
        bool rose = g_latchKnown && g_latchOk && g_latchVal == 0 && ok && latch != 0;
        if (rose) dumpThis(obj, latch);
    }
    g_latchKnown = true;
    g_latchOk = ok;
    g_latchVal = ok ? latch : 0;

    if (g_pendingArea) {
        ++g_ticksSinceLoad;
        if (g_addsTotal == g_addsAtLastTick) {
            ++g_stableTicks;
        } else {
            g_stableTicks = 0;
            g_addsAtLastTick = g_addsTotal;
        }
        // g_lastArea != 0: H4 has seen at least one add since the load hook, so a quiet window means "done", not "not started".
        if (g_stableTicks >= kStableTicks && g_lastArea != 0) {
            g_pendingArea = false;
            logf("H1 POP-STABLE area=%s id=%s adds_total=%ld ticks_since_load=%ld last_hook=%s",
                 g_lastAreaRef, g_lastAreaId, g_addsTotal, g_ticksSinceLoad, g_lastLoadHook);
            dumpThis(obj, latch);
        } else if (g_ticksSinceLoad >= kPopTimeoutTicks) {
            g_pendingArea = false;
            logf("H1 POP-TIMEOUT area=%s id=%s adds_total=%ld ticks_since_load=%ld last_hook=%s",
                 g_lastAreaRef, g_lastAreaId, g_addsTotal, g_ticksSinceLoad, g_lastLoadHook);
        }
    }
}

// H2: CSWSModule::LoadModuleInProgress (thiscall, ECX = module). Fresh area load. Rare.
void __cdecl AreaHookProbeFreshLoad(void* module) {
    g_lastLoadHook = "H2 fresh";
    g_lastArea = 0;   // a new area may reuse a freed pointer: let the next AddObjectToArea log
    g_pendingArea = true;
    g_ticksSinceLoad = 0;
    g_addsAtLastTick = g_addsTotal;
    g_stableTicks = 0;
    snprintf(g_lastAreaId, sizeof g_lastAreaId, "none");
    snprintf(g_lastAreaRef, sizeof g_lastAreaRef, "none");
    logf("H2 fresh area load: CSWSModule::LoadModuleInProgress module=%08lx", static_cast<unsigned long>(reinterpret_cast<uintptr_t>(module)));
}

// H3: CSWSModule::LoadModuleStart (thiscall, ECX = module). Saved-game module load.
void __cdecl AreaHookProbeSavedLoad(void* module) {
    g_lastLoadHook = "H3 saved";
    g_lastArea = 0;   // same reason as H2
    g_pendingArea = true;
    g_ticksSinceLoad = 0;
    g_addsAtLastTick = g_addsTotal;
    g_stableTicks = 0;
    snprintf(g_lastAreaId, sizeof g_lastAreaId, "none");
    snprintf(g_lastAreaRef, sizeof g_lastAreaRef, "none");
    logf("H3 saved module load: CSWSModule::LoadModuleStart module=%08lx", static_cast<unsigned long>(reinterpret_cast<uintptr_t>(module)));
}

// H4: CSWSArea::AddObjectToArea (thiscall, ECX = area). Stack args not read. Logs only when the area pointer changes.
void __cdecl AreaHookProbeAddObject(void* area) {
    saferead::beginScope();
    uintptr_t a = reinterpret_cast<uintptr_t>(area);
    ++g_addsTotal;
    if (a != 0 && a == g_lastArea) { ++g_addsSincePrev; return; }
    unsigned id = 0;
    bool idOk = readAt(kSiteAreaId, a, kAreaIdOff, &id);
    unsigned char rf[kResrefLen];
    memset(rf, 0, sizeof rf);
    bool rfOk = saferead::probeRead(kSiteAreaResref, reinterpret_cast<const void*>(a + kAreaResrefOff), kResrefLen);
    if (rfOk) memcpy(rf, reinterpret_cast<const void*>(a + kAreaResrefOff), kResrefLen);
    char idStr[16], refStr[128];
    if (idOk) snprintf(idStr, sizeof idStr, "%08x", id);
    else snprintf(idStr, sizeof idStr, "unreadable");
    fmtResref(refStr, sizeof refStr, rfOk, rf);
    snprintf(g_lastAreaId, sizeof g_lastAreaId, "%s", idStr);
    snprintf(g_lastAreaRef, sizeof g_lastAreaRef, "%s", refStr);
    logf("H4 area changed: area=%08lx id=%s resref=%s prev_area_calls=%ld total_adds=%ld (last load hook: %s)",
         static_cast<unsigned long>(a), idStr, refStr, g_addsSincePrev, g_addsTotal, g_lastLoadHook);
    g_lastArea = a;
    g_addsSincePrev = 0;
}

}  // extern "C"

// The attached line is logged here, not in DllMain: nothing before KpLog_Init reaches the core file, and KpLog_Init must not run under the
// loader lock. No file I/O here.
extern "C" __declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    logf("area-hook-probe %s attached: H1 0x00533830 CServerExoAppInternal::MainLoop | H2 0x0055A460 CSWSModule::LoadModuleInProgress | H3 0x005582F0 CSWSModule::LoadModuleStart | H4 0x00522BF0 CSWSArea::AddObjectToArea | log %s",
         kVersion, kLogName);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) g_attachTick = GetTickCount();
    return TRUE;
}
