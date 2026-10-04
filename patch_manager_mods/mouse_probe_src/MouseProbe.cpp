// Mouse Probe 0.2.0 -- STRICTLY READ-ONLY diagnostics for the Wine mouse/camera investigation
// (patch_manager_mods/18_mouse_camera_investigation.md). Every hook only reads (through SafeRead) and
// logs through KpLog.h; it never writes engine memory.
//
// Logging (0.2.0): channels main (START), raw (RAW), polls (POLLS), norm (NORM), drag (DRAGSTATE,
// STUCKDRAG? at WARN), cam (CAM). On a core with the kpatch.log service, kplog.ini [mouse-probe]
// switches them on and the lines land in mouse-probe_log.txt with the core's T=/f= prefix. On the old
// core (no service, KPatchInit never runs) the header's fallback sink writes mouseprobe_log.txt as
// before and the legacy flag file "mouseprobe.txt" (polled every ~30 hook calls; delete = kill switch)
// toggles the fallback level, so behaviour matches 0.1.0 apart from the line prefix.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "KpLog.h"
#include "../_shared/SafeRead.h"

KP_LOG_PATCH("mouse-probe");

extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }

namespace {

constexpr uintptr_t DragFlagAddr = 0x00A7FCBC;   // DAT_00A7FCBC, camera right-drag hold flag (int)
constexpr double StuckSeconds = 5.0;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chRaw, "raw");
KP_CHANNEL(chPolls, "polls");
KP_CHANNEL(chNorm, "norm");
KP_CHANNEL(chDrag, "drag");
KP_CHANNEL(chCam, "cam");

// Only stores the path (no I/O), so it is safe from a static initialiser. Old core: same file as 0.1.0.
// The header's default cap (60000 lines) matches the old MaxLogLines.
const bool g_fallbackSet = (KpLog_SetFallbackFile("mouseprobe_log.txt"), true);

int g_calls = 0;
bool g_started = false;           // START line written (new core: once; old core: per flag-file session)
bool g_flagOn = false;            // old core only: flag file seen
LARGE_INTEGER g_freq;
bool g_timeInit = false;

// Monotonic ms for gap / stuck-drag arithmetic only; it is no longer written to the log (the core's
// T= prefix replaces it).
double nowMs() {
    if (!g_timeInit) { QueryPerformanceFrequency(&g_freq); g_timeInit = true; }
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return (double)t.QuadPart * 1000.0 / (double)g_freq.QuadPart;
}

// Entry bookkeeping shared by all hooks: scope reset + (old core) flag-file poll / (new core) START.
void enter() {
    saferead::beginScope();
    if (KpLog_HasCore()) {
        if (!g_started) { g_started = true; KP_LOG(chMain, "START mouseprobe 0.2.0"); }
        return;                                    // new core: kplog.ini is the switch, no STOP line
    }
    if (g_calls++ % 30 == 0) {
        // Legacy flag file: only meaningful without the service. It drives the fallback level so
        // KP_ENABLED/KP_LOG follow it (INFO = everything, WARN = just STUCKDRAG?).
        bool want = GetFileAttributesA("mouseprobe.txt") != INVALID_FILE_ATTRIBUTES;
        if (want && !g_flagOn) {
            g_flagOn = true; KpLog_SetFallbackLevel(KPLOG_INFO);
            KP_LOG(chMain, "START mouseprobe 0.2.0");
        } else if (!want && g_flagOn) {
            KP_LOG(chMain, "STOP (flag file gone)");
            g_flagOn = false; KpLog_SetFallbackLevel(KPLOG_WARN);
        }
    }
}

template <typename T> bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(1, base, off, out); }

const char* tag(float v) { return isnan(v) ? " NAN" : isinf(v) ? " INF" : ""; }

// ---- shared state between the raw poll hook and the camera hook ----
bool g_rmbDown = false;           // rgbButtons[1] bit 7 of the last raw poll (always tracked: STUCKDRAG? needs it)
unsigned g_lastBtn = 0xFFFFFFFFu;
double g_lastPollMs = 0, g_secStart = 0, g_maxGap = 0;
int g_polls = 0, g_zeroPolls = 0;

int g_drag = -1;                   // last drag flag value seen (always tracked, even with the drag channel off)
double g_dragSince = 0;
bool g_stuckLogged = false;
float g_lastPitch = 0; bool g_havePitch = false;

} // namespace

extern "C" {

// 0x72F14E: raw DIMOUSESTATE, [ebp-0x14]=lX [ebp-0x10]=lY [ebp-0xC]=lZ [ebp-8]=rgbButtons[4]
void __cdecl ProbeMouseRaw(uintptr_t ebp) {
    enter();
    unsigned btn = 0;
    if (!rd(ebp, -8, &btn)) return;
    g_rmbDown = ((btn >> 8) & 0x80) != 0;      // rgbButtons[1] bit 7; tracked even with raw/polls off
    if (!(KP_ENABLED(chRaw) || KP_ENABLED(chPolls))) { g_lastBtn = btn; return; }
    int lx = 0, ly = 0, lz = 0;
    if (!rd(ebp, -0x14, &lx) || !rd(ebp, -0x10, &ly) || !rd(ebp, -0xC, &lz)) return;
    double t = nowMs();
    if (g_lastPollMs > 0 && t - g_lastPollMs > g_maxGap) g_maxGap = t - g_lastPollMs;
    g_lastPollMs = t;
    ++g_polls;
    bool zero = (lx | ly | lz | (int)btn) == 0;
    if (zero) ++g_zeroPolls;
    if (!zero || btn != g_lastBtn) KP_LOG(chRaw, "RAW %d %d %d %08x", lx, ly, lz, btn);
    g_lastBtn = btn;
    if (g_secStart == 0) g_secStart = t;
    if (t - g_secStart >= 1000.0) {
        KP_LOG(chPolls, "POLLS n=%d zero=%d maxgap=%.2fms", g_polls, g_zeroPolls, g_maxGap);
        g_polls = g_zeroPolls = 0; g_maxGap = 0; g_secStart = t;
    }
}

// 0x72F2EC: this=[ebp-0x28]; this+0x408/+0x40C = normalised dx/dy
void __cdecl ProbeMouseNorm(uintptr_t ebp) {
    enter();
    if (!KP_ENABLED(chNorm)) return;
    uintptr_t self = 0; float nx = 0, ny = 0;
    if (!rd(ebp, -0x28, &self) || !rd(self, 0x408, &nx) || !rd(self, 0x40C, &ny)) return;
    if (nx == 0.0f && ny == 0.0f) return;
    bool sat = fabsf(nx) >= 1.0f || fabsf(ny) >= 1.0f;
    KP_LOG(chNorm, "NORM %.4f%s %.4f%s%s", nx, tag(nx), ny, tag(ny), sat ? " SAT" : "");
}

// 0x78C452: [ebp-0x30]=dx [ebp-0x50]=dy [ebp-0x84]=scale, this=[ebp-0x130], ctl=*(this+0x18)
// No KP_FRAME_TICK here on purpose: this hook only runs in-world, so it would take the shared frame
// counter away from a better (frame-global) ticker. Opt in with -DMOUSEPROBE_FRAMETICK for a test.
void __cdecl ProbeCameraFrame(uintptr_t ebp) {
    enter();
#ifdef MOUSEPROBE_FRAMETICK
    KP_FRAME_TICK();
#endif
    // Drag state is always tracked so STUCKDRAG? (WARN) works with the drag/cam channels off.
    int drag = 0; rd(DragFlagAddr, 0, &drag);
    double t = nowMs();
    if (drag != g_drag) {
        KP_LOG(chDrag, "DRAGSTATE %d -> %d rmbDown=%d", g_drag, drag, g_rmbDown ? 1 : 0);
        g_drag = drag; g_dragSince = t; g_stuckLogged = false;
    }
    if (drag && !g_rmbDown && !g_stuckLogged && t - g_dragSince > StuckSeconds * 1000.0) {
        KP_LOGV(chDrag, KPLOG_WARN, "STUCKDRAG? flag set %.1fs with no RMB in raw mask", (t - g_dragSince) / 1000.0);
        g_stuckLogged = true;
    }

    if (!KP_ENABLED(chCam)) return;
    float dx = 0, dy = 0, scale = 0;
    if (!rd(ebp, -0x30, &dx) || !rd(ebp, -0x50, &dy) || !rd(ebp, -0x84, &scale)) return;
    uintptr_t self = 0, ctl = 0; float pitch = 0, yaw = 0; int mode = -1;
    bool haveCtl = rd(ebp, -0x130, &self) && rd(self, 0x18, &ctl) && ctl != 0 &&
                   rd(ctl, 0x94, &pitch) && rd(ctl, 0x98, &yaw) && rd(ctl, 0xC, &mode);
    bool pitchMoved = haveCtl && g_havePitch && fabsf(pitch - g_lastPitch) > 0.001f;
    if (drag || dx != 0.0f || dy != 0.0f || pitchMoved || (haveCtl && !g_havePitch)) {
        if (haveCtl) {
            KP_LOG(chCam, "CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d mode=%d yaw=%.4f%s pitch=%.4f%s ctl94=%.4f ctl98=%.4f",
                   dx, tag(dx), dy, tag(dy), scale, tag(scale), drag, mode, yaw, tag(yaw), pitch, tag(pitch), pitch, yaw);
        } else {
            KP_LOG(chCam, "CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d ctl=unreadable", dx, tag(dx), dy, tag(dy), scale, tag(scale), drag);
        }
    }
    if (haveCtl) { g_lastPitch = pitch; g_havePitch = true; }
}

} // extern "C"
