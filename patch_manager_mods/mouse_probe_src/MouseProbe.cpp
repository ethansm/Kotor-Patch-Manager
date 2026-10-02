// Mouse Probe 0.1.0 -- STRICTLY READ-ONLY diagnostics for the Wine mouse/camera investigation
// (patch_manager_mods/18_mouse_camera_investigation.md). Every hook only reads (through SafeRead) and
// appends to mouseprobe_log.txt; it never writes engine memory. Logging is gated by the flag file
// "mouseprobe.txt" in the game dir (polled every ~30 hook calls; delete the file = kill switch).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include "../_shared/SafeRead.h"

namespace {

constexpr int MaxLogLines = 60000;
constexpr uintptr_t DragFlagAddr = 0x00A7FCBC;   // DAT_00A7FCBC, camera right-drag hold flag (int)
constexpr double StuckSeconds = 5.0;

bool g_on = false;
int g_calls = 0, g_lines = 0;
FILE* g_f = nullptr;
LARGE_INTEGER g_freq, g_t0;
bool g_timeInit = false;

double nowMs() {
    if (!g_timeInit) { QueryPerformanceFrequency(&g_freq); QueryPerformanceCounter(&g_t0); g_timeInit = true; }
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return (double)t.QuadPart * 1000.0 / (double)g_freq.QuadPart;
}

void closeLog() { if (g_f) { fclose(g_f); g_f = nullptr; } }

void logf(const char* fmt, ...) {
    if (!g_on || g_lines >= MaxLogLines) return;
    if (!g_f) { g_f = fopen("mouseprobe_log.txt", "a"); if (!g_f) return; }
    ++g_lines;
    fprintf(g_f, "%.3f ", nowMs());
    va_list ap; va_start(ap, fmt); vfprintf(g_f, fmt, ap); va_end(ap);
    fputc('\n', g_f);
    fflush(g_f);
    if (g_lines == MaxLogLines) { fputs("--- log cap reached ---\n", g_f); fflush(g_f); }
}

// Entry bookkeeping shared by all hooks: scope reset + flag-file poll.
void enter() {
    saferead::beginScope();
    if (g_calls++ % 30 == 0) {
        bool want = GetFileAttributesA("mouseprobe.txt") != INVALID_FILE_ATTRIBUTES;
        if (want && !g_on) { g_on = true; logf("START mouseprobe 0.1.0"); }
        else if (!want && g_on) { logf("STOP (flag file gone)"); g_on = false; closeLog(); }
    }
}

template <typename T> bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(1, base, off, out); }

const char* tag(float v) { return isnan(v) ? " NAN" : isinf(v) ? " INF" : ""; }

// ---- shared state between the raw poll hook and the camera hook ----
bool g_rmbDown = false;           // rgbButtons[1] bit 7 of the last raw poll
unsigned g_lastBtn = 0xFFFFFFFFu;
double g_lastPollMs = 0, g_secStart = 0, g_maxGap = 0;
int g_polls = 0, g_zeroPolls = 0;

int g_drag = -1;                   // last drag flag value seen
double g_dragSince = 0;
bool g_stuckLogged = false;
float g_lastPitch = 0; bool g_havePitch = false;

} // namespace

extern "C" {

// 0x72F14E: raw DIMOUSESTATE, [ebp-0x14]=lX [ebp-0x10]=lY [ebp-0xC]=lZ [ebp-8]=rgbButtons[4]
void __cdecl ProbeMouseRaw(uintptr_t ebp) {
    enter();
    if (!g_on) return;
    int lx = 0, ly = 0, lz = 0; unsigned btn = 0;
    if (!rd(ebp, -0x14, &lx) || !rd(ebp, -0x10, &ly) || !rd(ebp, -0xC, &lz) || !rd(ebp, -8, &btn)) return;
    double t = nowMs();
    if (g_lastPollMs > 0 && t - g_lastPollMs > g_maxGap) g_maxGap = t - g_lastPollMs;
    g_lastPollMs = t;
    ++g_polls;
    bool zero = (lx | ly | lz | (int)btn) == 0;
    if (zero) ++g_zeroPolls;
    g_rmbDown = ((btn >> 8) & 0x80) != 0;      // rgbButtons[1] bit 7
    if (!zero || btn != g_lastBtn) logf("RAW %d %d %d %08x", lx, ly, lz, btn);
    g_lastBtn = btn;
    if (g_secStart == 0) g_secStart = t;
    if (t - g_secStart >= 1000.0) {
        logf("POLLS n=%d zero=%d maxgap=%.2fms", g_polls, g_zeroPolls, g_maxGap);
        g_polls = g_zeroPolls = 0; g_maxGap = 0; g_secStart = t;
    }
}

// 0x72F2EC: this=[ebp-0x28]; this+0x408/+0x40C = normalised dx/dy
void __cdecl ProbeMouseNorm(uintptr_t ebp) {
    enter();
    if (!g_on) return;
    uintptr_t self = 0; float nx = 0, ny = 0;
    if (!rd(ebp, -0x28, &self) || !rd(self, 0x408, &nx) || !rd(self, 0x40C, &ny)) return;
    if (nx == 0.0f && ny == 0.0f) return;
    bool sat = fabsf(nx) >= 1.0f || fabsf(ny) >= 1.0f;
    logf("NORM %.4f%s %.4f%s%s", nx, tag(nx), ny, tag(ny), sat ? " SAT" : "");
}

// 0x78C452: [ebp-0x30]=dx [ebp-0x50]=dy [ebp-0x84]=scale, this=[ebp-0x130], ctl=*(this+0x18)
void __cdecl ProbeCameraFrame(uintptr_t ebp) {
    enter();
    if (!g_on) return;
    float dx = 0, dy = 0, scale = 0;
    if (!rd(ebp, -0x30, &dx) || !rd(ebp, -0x50, &dy) || !rd(ebp, -0x84, &scale)) return;
    int drag = 0; rd(DragFlagAddr, 0, &drag);
    uintptr_t self = 0, ctl = 0; float pitch = 0, yaw = 0; int mode = -1;
    bool haveCtl = rd(ebp, -0x130, &self) && rd(self, 0x18, &ctl) && ctl != 0 &&
                   rd(ctl, 0x94, &pitch) && rd(ctl, 0x98, &yaw) && rd(ctl, 0xC, &mode);
    double t = nowMs();

    if (drag != g_drag) {
        logf("DRAGSTATE %d -> %d rmbDown=%d", g_drag, drag, g_rmbDown ? 1 : 0);
        g_drag = drag; g_dragSince = t; g_stuckLogged = false;
    }
    if (drag && !g_rmbDown && !g_stuckLogged && t - g_dragSince > StuckSeconds * 1000.0) {
        logf("STUCKDRAG? flag set %.1fs with no RMB in raw mask", (t - g_dragSince) / 1000.0);
        g_stuckLogged = true;
    }
    bool pitchMoved = haveCtl && g_havePitch && fabsf(pitch - g_lastPitch) > 0.001f;
    if (drag || dx != 0.0f || dy != 0.0f || pitchMoved || (haveCtl && !g_havePitch)) {
        if (haveCtl) {
            logf("CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d mode=%d yaw=%.4f%s pitch=%.4f%s ctl94=%.4f ctl98=%.4f",
                 dx, tag(dx), dy, tag(dy), scale, tag(scale), drag, mode, yaw, tag(yaw), pitch, tag(pitch), pitch, yaw);
        } else {
            logf("CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d ctl=unreadable", dx, tag(dx), dy, tag(dy), scale, tag(scale), drag);
        }
    }
    if (haveCtl) { g_lastPitch = pitch; g_havePitch = true; }
}

} // extern "C"
