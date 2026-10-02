// k2-equip-icon-fix 1.0.0 (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Fixes candidate BR (doc 10, Phase 5C):
// the equipment screen reloads the equipped weapon's icon texture every frame.
//
// Mechanism (static, decomp/8a/008ad930.*; run A NAME tables incidents_15_1824): the equipment panel per-frame update
// 0x8ABA50 calls 0x8AD930, which first resets slot-image controls 0 (this+0x14D0) and 9 (this+0x2058) to their
// placeholders ("iweap_l"... built by 0x8AB790) at 0x8ADA08 / 0x8ADA81, then, in the slot loop, sets the same controls to
// the item icon (0x8AE19A / 0x8AE1CD, control this+0x14D0+k*0x148, k=[EBP-0x28]) or to a greyed two-handed icon
// (0x8ADFA2 / 0x8AE02D) or the placeholder again (change path 0x8AE793). The GUI image setter 0x414840 (thiscall
// (CResRef* name, int force), RET 8; current name at ctl+0x44) only skips work when the name is unchanged, so the
// alternation destroys and re-creates the icon texture every frame (loose override TGA: disk read + decode + upload +
// glFinish ~2.6 ms per frame).
//
// Fix (defer-and-cancel, final state identical to the engine's): DllMain re-targets 8 existing CALL rel32 sites:
//   0x8ABA8C CALL 0x8AD930          -> EquipFixPass   (opens a pass, calls 0x8AD930, then applies any deferred set)
//   0x8ADA08 / 0x8ADA81 / 0x8AE793 CALL 0x414840 (placeholder sets: top resets of controls 0/9 and the empty-slot
//                                    change path of control k) -> EquipFixDefer (records control + 16-byte resref)
//   0x8ADFA2 0x8AE02D 0x8AE19A 0x8AE1CD CALL 0x414840 (item / greyed two-handed icon) -> EquipFixSet (cancels a deferred
//                                    set on that control, then calls 0x414840 unchanged)
// 0x8AE793 is deferred too: with a two-handed weapon in slot 1 and slot 0 empty the change path resets control 0 every
// frame (its condition holds at k=0 because local_24/local_18 are still 0) before 0x8ADFA2 sets the greyed icon.
// A deferred placeholder is applied at the end of the pass only if nothing else set that control in the same pass, so
// every control ends each frame with exactly the name the engine would have left, with one setter call per control.
// Outside a pass (should not happen: 0x8AD930 has one caller) EquipFixDefer sets immediately.
//
// Config: $G/equipiconfix.txt `enabled=0` disables (sites untouched). Log: $G/equipiconfix_log.txt (DllMain line; a
// helper thread appends a STATS line every 10 s while counters change and the first 16 cancel events).
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

namespace {

constexpr uintptr_t SetImage = 0x00414840;      // CSWGuiControl image setter, thiscall(CResRef*, int), RET 8
constexpr uintptr_t PanelUpdate = 0x008AD930;   // equipment panel slot refresh, fastcall(ECX = panel)
struct Site { uintptr_t at, target; int kind; };   // kind 0 pass wrapper, 1 defer, 2 set
const Site Sites[8] = {
    { 0x008ABA8C, PanelUpdate, 0 },
    { 0x008ADA08, SetImage, 1 }, { 0x008ADA81, SetImage, 1 },
    { 0x008AE793, SetImage, 1 },
    { 0x008ADFA2, SetImage, 2 }, { 0x008AE02D, SetImage, 2 }, { 0x008AE19A, SetImage, 2 }, { 0x008AE1CD, SetImage, 2 },
};

typedef int (__thiscall* SetImageFn)(void* ctl, const void* resref, int force);
typedef void (__fastcall* PanelUpdateFn)(void* panel);

struct Pending { void* ctl; uint8_t resref[16]; int force; bool live; };
constexpr int MaxPending = 16;
Pending g_pending[MaxPending];   // main thread only; one per control (11 slot controls)
bool g_inPass = false;

volatile LONG g_passes = 0, g_deferred = 0, g_cancelled = 0, g_applied = 0, g_sets = 0, g_outside = 0, g_replaced = 0;
struct CancelEv { void* ctl; char from[17], to[17]; uintptr_t ra; };
CancelEv g_cancelEv[16]; volatile LONG g_cancelHead = 0; LONG g_cancelFlushed = 0;

CRITICAL_SECTION g_logLock;
int g_state = 0;            // 1 patched, 0 disabled, -1 refused (bytes), -2 refused (protect), -3 refused (unmapped)
int g_badSite = -1; uint8_t g_seen[5];

void logf(const char* fmt, ...) {
    char line[400];
    va_list ap; va_start(ap, fmt); vsnprintf(line, sizeof(line), fmt, ap); va_end(ap);
    EnterCriticalSection(&g_logLock);
    if (FILE* f = fopen("equipiconfix_log.txt", "a")) {
        SYSTEMTIME t; GetLocalTime(&t);
        fprintf(f, "[%02u:%02u:%02u.%03u %lu] %s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, GetTickCount(), line);
        fclose(f);
    }
    LeaveCriticalSection(&g_logLock);
}

bool configEnabled() {
    FILE* f = fopen("equipiconfix.txt", "r");
    if (!f) return true;
    bool on = true; char line[128];
    while (fgets(line, sizeof(line), f)) {
        char* p = line; while (*p == ' ' || *p == '\t') ++p;
        if (!strncmp(p, "enabled", 7)) { p += 7; while (*p == ' ' || *p == '=') ++p; on = *p != '0'; }
    }
    fclose(f);
    return on;
}

void resrefName(const void* r, char out[17]) { memcpy(out, r, 16); out[16] = 0; for (int i = 0; i < 16; ++i) if (out[i] && (out[i] < 0x20 || out[i] > 0x7e)) out[i] = '?'; }

bool committed(uintptr_t at, size_t n) {
    MEMORY_BASIC_INFORMATION mb;
    return VirtualQuery(reinterpret_cast<void*>(at), &mb, sizeof(mb)) && mb.State == MEM_COMMIT &&
           at + n <= reinterpret_cast<uintptr_t>(mb.BaseAddress) + mb.RegionSize;
}
uint8_t* callBytes(uintptr_t at, uintptr_t target, uint8_t out[5]) {
    out[0] = 0xE8; int32_t d = static_cast<int32_t>(target - (at + 5)); memcpy(out + 1, &d, 4); return out;
}
int writeCode(uintptr_t at, const uint8_t* repl, int n) {
    auto p = reinterpret_cast<uint8_t*>(at);
    DWORD old;
    if (!VirtualProtect(p, n, PAGE_EXECUTE_READWRITE, &old)) return -2;
    memcpy(p, repl, n);
    VirtualProtect(p, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, n);
    return 1;
}

DWORD WINAPI statsThread(LPVOID) {
    LONG last = -1;
    for (;;) {
        Sleep(10000);
        while (g_cancelFlushed < g_cancelHead && g_cancelFlushed < 16) {
            const CancelEv& e = g_cancelEv[g_cancelFlushed++];
            logf("CANCEL ctl=%p deferred '%s' superseded by '%s' (ra=%08x) in the same pass", e.ctl, e.from, e.to, (unsigned)e.ra);
        }
        LONG p = g_passes;
        if (p != last) {
            logf("STATS passes=%ld deferred=%ld cancelled=%ld replaced=%ld applied=%ld sets=%ld outside_pass=%ld", p, (long)g_deferred, (long)g_cancelled, (long)g_replaced, (long)g_applied, (long)g_sets, (long)g_outside);
            last = p;
        }
    }
}

}  // namespace

// 0x8ABA8C: replaces CALL 0x8AD930 (fastcall, ECX = panel). Main thread.
extern "C" void __fastcall EquipFixPass(void* panel) {
    InterlockedIncrement(&g_passes);
    for (Pending& d : g_pending) d.live = false;
    g_inPass = true;
    reinterpret_cast<PanelUpdateFn>(PanelUpdate)(panel);
    g_inPass = false;
    for (Pending& d : g_pending) {
        if (!d.live) continue;
        d.live = false;
        InterlockedIncrement(&g_applied);
        reinterpret_cast<SetImageFn>(SetImage)(d.ctl, d.resref, d.force);
    }
}

// 0x8ADA08 / 0x8ADA81: replaces the placeholder CALL 0x414840 (thiscall, RET 8). The resref is a stack local of
// 0x8AD930 that dies right after the call, so its 16 bytes are copied.
extern "C" int __thiscall EquipFixDefer(void* ctl, const void* resref, int force) {
    if (!g_inPass) { InterlockedIncrement(&g_outside); return reinterpret_cast<SetImageFn>(SetImage)(ctl, resref, force); }
    Pending* d = nullptr;   // the control's own slot (a later placeholder set of the same control replaces the earlier one), else a free one
    for (Pending& p : g_pending) if (p.live && p.ctl == ctl) { d = &p; break; }
    if (!d) for (Pending& p : g_pending) if (!p.live) { d = &p; break; }
    if (!d) { InterlockedIncrement(&g_outside); return reinterpret_cast<SetImageFn>(SetImage)(ctl, resref, force); }
    if (d->live) InterlockedIncrement(&g_replaced);   // same control deferred twice in one pass: the later set wins, as in the engine
    d->ctl = ctl; memcpy(d->resref, resref, 16); d->force = force; d->live = true;
    InterlockedIncrement(&g_deferred);
    return 0;
}

// The other 0x414840 calls of 0x8AD930: cancel a deferred set on the same control, then set as the engine does.
extern "C" int __thiscall EquipFixSet(void* ctl, const void* resref, int force) {
    InterlockedIncrement(&g_sets);
    for (Pending& d : g_pending) {
        if (!d.live || d.ctl != ctl) continue;
        d.live = false;
        InterlockedIncrement(&g_cancelled);
        LONG i = g_cancelHead;   // keep the first 16 cancels for the log (runtime proof of the same-control alternation)
        if (i < 16) {
            CancelEv& e = g_cancelEv[i]; e.ctl = ctl; resrefName(d.resref, e.from); resrefName(resref, e.to);
            e.ra = reinterpret_cast<uintptr_t>(__builtin_return_address(0)); g_cancelHead = i + 1;
        }
    }
    return reinterpret_cast<SetImageFn>(SetImage)(ctl, resref, force);
}

// Harness-only exports (never called by the game).
extern "C" int __cdecl EquipFixTestState() { return g_state; }
extern "C" void __cdecl EquipFixTestCounters(long* out7) {
    out7[0] = g_passes; out7[1] = g_deferred; out7[2] = g_cancelled; out7[3] = g_applied; out7[4] = g_sets; out7[5] = g_outside; out7[6] = g_replaced;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    InitializeCriticalSection(&g_logLock);
    if (!configEnabled()) {
        logf("k2-equip-icon-fix 1.0.0 loaded: DISABLED by equipiconfix.txt (8 sites untouched) pid=%lu", GetCurrentProcessId());
        return TRUE;
    }
    // Verify all 8 sites before writing any of them.
    for (int i = 0; i < 8; ++i) {
        uint8_t want[5];
        callBytes(Sites[i].at, Sites[i].target, want);
        if (!committed(Sites[i].at, 5)) { g_state = -3; g_badSite = i; break; }
        memcpy(g_seen, reinterpret_cast<void*>(Sites[i].at), 5);
        if (memcmp(g_seen, want, 5) != 0) { g_state = -1; g_badSite = i; break; }
    }
    if (g_state == 0) {
        g_state = 1;
        for (int i = 0; i < 8 && g_state == 1; ++i) {
            uintptr_t to = Sites[i].kind == 0 ? reinterpret_cast<uintptr_t>(&EquipFixPass)
                         : Sites[i].kind == 1 ? reinterpret_cast<uintptr_t>(&EquipFixDefer) : reinterpret_cast<uintptr_t>(&EquipFixSet);
            uint8_t repl[5];
            int r = writeCode(Sites[i].at, callBytes(Sites[i].at, to, repl), 5);
            if (r != 1) { g_state = r; g_badSite = i; }
        }
    }
    if (g_state == 1)
        logf("k2-equip-icon-fix 1.0.0 loaded: 8 sites patched (pass 0x8ABA8C, defer 0x8ADA08/0x8ADA81/0x8AE793, set x4) pid=%lu", GetCurrentProcessId());
    else if (g_state == -1)
        logf("k2-equip-icon-fix 1.0.0 loaded: REFUSED(bytes %02x %02x %02x %02x %02x at 0x%08x) all sites untouched pid=%lu",
             g_seen[0], g_seen[1], g_seen[2], g_seen[3], g_seen[4], (unsigned)Sites[g_badSite].at, GetCurrentProcessId());
    else
        logf("k2-equip-icon-fix 1.0.0 loaded: REFUSED(%s at 0x%08x)%s pid=%lu", g_state == -2 ? "protect" : "unmapped", (unsigned)Sites[g_badSite].at,
             g_state == -2 && g_badSite > 0 ? " PARTIAL WRITE" : " all sites untouched", GetCurrentProcessId());
    if (g_state == 1) CloseHandle(CreateThread(nullptr, 0, statsThread, nullptr, 0, nullptr));
    return TRUE;
}
