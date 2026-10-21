// Mouse Probe 0.4.1 -- Wine mouse/camera fix + diagnostics
// (patch_manager_mods/18_mouse_camera_investigation.md). Hooks read engine memory through SafeRead and
// log through KpLog.h. There are two engine writes, both gated by [mouse_fix] enabled in mouse_probe.ini and
// on by default: (1) sanitizeNorm (sanitize_norm=1) in ProbeMouseNorm, which overwrites the normalised
// delta at this+0x408/+0x40C; (2) SYNTHDBL (synth_dblclk=1) at OnMouseMessage, which rewrites the message id
// at msg+4 to WM_LBUTTONDBLCLK when Wine drops the double click. Both stores are probeWrite-guarded (skipped + logged when not writable). Every other hook only reads through SafeRead.
//
// Logging (0.2.0): channels main (START), raw (RAW), polls (POLLS), norm (NORM), drag (DRAGSTATE,
// STUCKDRAG? at WARN), cam (CAM). On a core with the kpatch.log service, kplog.ini [mouse-probe]
// switches them on and the lines land in mouse-probe_log.txt with the core's T=/f= prefix. On the old
// core (no service, KPatchInit never runs) the header's fallback sink writes mouseprobe_log.txt as
// before and the legacy flag file "mouseprobe.txt" (polled every ~30 hook calls; delete = kill switch)
// toggles the fallback level. With [mouse_fix] enabled=0 the probe behaves as 0.1.0 (read-only) apart from the line prefix.
//
// 0.3.0 adds the "click" channel (doc 18 "Double-click chain (traced)"): CLICK (every WM_*BUTTON* message at
// OnMouseMessage 0x72E039, with the SetCursorPos guard this+0x3EC and time/distance since the last 0x201), DBLACT
// (PerformLButtonDownAction entry), WORLDCLICK (HandleMouseClickInWorld entry + derived branch A/B), DEFACT (the default-action
// call in branch B), MOVEPT (AddMoveToPointAction entry, only within 3 s of an LMB message, else counted in movept_bg= on the
// next CLICK) and WALKWP (HandlePlayerToServerInputWalkToWaypoint, expected never). WARN heuristics MISSEDDBL (two 0x201 within
// 500 ms / 4 px with no 0x203 between) and DBLGUARD (0x203 while the guard int is set) live in ClickLogic.h (pure, tested).
// Hooks at function entries read ECX / [ESP+4] (KPM passes the game's stack: [esp+0] = return address).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "KpLog.h"
#include "ClickLogic.h"
#include "MouseFixLogic.h"
#include "../_shared/SafeRead.h"

KP_LOG_PATCH("mouse-probe");

extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }

namespace {

constexpr uintptr_t DragFlagAddr = 0x00A7FCBC;   // DAT_00A7FCBC, camera right-drag hold flag (int)
constexpr double StuckSeconds = 5.0;
constexpr unsigned SiteMsgWrite = 2;   // saferead failure-ring ids: 1 = reads (rd), 2 = SYNTHDBL msg+4 store, 3 = sanitize norm store
constexpr unsigned SiteNormWrite = 3;

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chRaw, "raw");
KP_CHANNEL(chPolls, "polls");
KP_CHANNEL(chNorm, "norm");
KP_CHANNEL(chDrag, "drag");
KP_CHANNEL(chCam, "cam");
KP_CHANNEL(chClick, "click");

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
        if (!g_started) { g_started = true; KP_LOG(chMain, "START mouseprobe 0.3.0"); }
        return;                                    // new core: kplog.ini is the switch, no STOP line
    }
    if (g_calls++ % 30 == 0) {
        // Legacy flag file: only meaningful without the service. It drives the fallback level so
        // KP_ENABLED/KP_LOG follow it (INFO = everything, WARN = just STUCKDRAG?).
        bool want = GetFileAttributesA("mouseprobe.txt") != INVALID_FILE_ATTRIBUTES;
        if (want && !g_flagOn) {
            g_flagOn = true; KpLog_SetFallbackLevel(KPLOG_INFO);
            KP_LOG(chMain, "START mouseprobe 0.3.0");
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
int g_lastMode = -999; uintptr_t g_lastCtl = 0;   // 0.4.1 CAMMODE change tracking

// ---- click channel state (0.3.0); always tracked so the WARN heuristics work with the channel off ----
clicklogic::ClickState g_click;
int g_moveptBg = 0;                // AddMoveToPointAction calls outside the 3 s click window since the last CLICK line

// ---- mouse fix state (0.4.0) ----
mousefix::FixConfig g_fixCfg;
bool g_fixCfgLoaded = false;
mousefix::SanitizeState g_sanState;

typedef BOOL (WINAPI *pfn_GetCursorPos)(LPPOINT);
typedef BOOL (WINAPI *pfn_SetCursorPos)(int, int);
typedef HWND (WINAPI *pfn_GetForegroundWindow)(void);
typedef BOOL (WINAPI *pfn_GetWindowRect)(HWND, LPRECT);

pfn_GetCursorPos dyn_GetCursorPos = nullptr;
pfn_SetCursorPos dyn_SetCursorPos = nullptr;
pfn_GetForegroundWindow dyn_GetForegroundWindow = nullptr;
pfn_GetWindowRect dyn_GetWindowRect = nullptr;
bool g_user32Resolved = false;

void resolveUser32() {
    if (g_user32Resolved) return;
    g_user32Resolved = true;
    HMODULE u32 = GetModuleHandleA("user32.dll");
    if (!u32) u32 = LoadLibraryA("user32.dll");
    if (u32) {
        dyn_GetCursorPos = reinterpret_cast<pfn_GetCursorPos>(reinterpret_cast<void*>(GetProcAddress(u32, "GetCursorPos")));
        dyn_SetCursorPos = reinterpret_cast<pfn_SetCursorPos>(reinterpret_cast<void*>(GetProcAddress(u32, "SetCursorPos")));
        dyn_GetForegroundWindow = reinterpret_cast<pfn_GetForegroundWindow>(reinterpret_cast<void*>(GetProcAddress(u32, "GetForegroundWindow")));
        dyn_GetWindowRect = reinterpret_cast<pfn_GetWindowRect>(reinterpret_cast<void*>(GetProcAddress(u32, "GetWindowRect")));
    }
}

void loadFixConfig() {
    if (g_fixCfgLoaded) return;
    g_fixCfgLoaded = true;
    const char* ini = ".\\mouse_probe.ini";
    g_fixCfg.enabled = GetPrivateProfileIntA("mouse_fix", "enabled", 1, ini) != 0;
    g_fixCfg.sanitizeNorm = GetPrivateProfileIntA("mouse_fix", "sanitize_norm", 1, ini) != 0;
    g_fixCfg.synthDblClk = GetPrivateProfileIntA("mouse_fix", "synth_dblclk", 1, ini) != 0;
    g_fixCfg.edgeWarp = GetPrivateProfileIntA("mouse_fix", "edge_warp", 1, ini) != 0;
    int pct = GetPrivateProfileIntA("mouse_fix", "sens_scale_pct", 5, ini);
    if (pct > 0) g_fixCfg.sensScale = static_cast<float>(pct) / 100.0f;
    int margin = GetPrivateProfileIntA("mouse_fix", "edge_margin", 50, ini);
    if (margin > 0) g_fixCfg.edgeMargin = margin;
}

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
    if (!zero || btn != g_lastBtn) {
        if (btn != g_lastBtn) KP_LOG(chRaw, "RAW %d %d %d %08x", lx, ly, lz, btn);
        else KP_LOG_EVERY(chRaw, 100, "RAW %d %d %d %08x", lx, ly, lz, btn);
    }
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
    loadFixConfig();
    uintptr_t self = 0;
    if (!rd(ebp, -0x28, &self) || self == 0) return;

    if (g_fixCfg.enabled && g_fixCfg.sanitizeNorm) {
        resolveUser32();
        int drag = 0;
        if (!rd(DragFlagAddr, 0, &drag))
            KP_LOG_EVERY(chNorm, 100, "DRAGFLAG unreadable; g_rmbDown decides");
        bool dragging = (drag != 0) || g_rmbDown;

        POINT cur = {0, 0};
        bool curOk = dyn_GetCursorPos && dyn_GetCursorPos(&cur);

        float fixedNx = 0.0f, fixedNy = 0.0f;
        if (mousefix::sanitizeFrame(g_sanState, dragging, curOk, cur.x, cur.y,
                                    g_fixCfg.sensScale, fixedNx, fixedNy) == mousefix::SanitizeResult::Write) {
            if (dragging && g_fixCfg.edgeWarp && dyn_GetForegroundWindow && dyn_GetWindowRect && dyn_SetCursorPos) {
                HWND hwnd = dyn_GetForegroundWindow();
                RECT rc;
                if (hwnd && dyn_GetWindowRect(hwnd, &rc)) {
                    int wx = 0, wy = 0;
                    if (mousefix::checkEdgeWarp(cur.x, cur.y, rc.left, rc.top, rc.right, rc.bottom, g_fixCfg.edgeMargin, wx, wy)) {
                        dyn_SetCursorPos(wx, wy);
                        g_sanState.lastX = wx;
                        g_sanState.lastY = wy;
                    }
                }
            }
            if (saferead::probeWrite(SiteNormWrite, reinterpret_cast<void*>(self + 0x408), 8)) {
                *reinterpret_cast<float*>(self + 0x408) = fixedNx;
                *reinterpret_cast<float*>(self + 0x40C) = fixedNy;
            } else {
                KP_LOG_EVERY(chNorm, 100, "NORMWRITE self+0x408 not writable; sanitize skipped");
            }
        } else {
            KP_LOG_EVERY(chNorm, 100, "CURSORFAIL GetCursorPos unavailable/failed; sanitize skipped (engine delta kept)");
        }
    }

    if (!KP_ENABLED(chNorm)) return;
    float nx = 0, ny = 0;
    if (!rd(self, 0x408, &nx) || !rd(self, 0x40C, &ny)) return;
    if (nx == 0.0f && ny == 0.0f) return;
    bool sat = fabsf(nx) >= 1.0f || fabsf(ny) >= 1.0f;
    KP_LOG_EVERY(chNorm, 100, "NORM %.4f%s %.4f%s%s", nx, tag(nx), ny, tag(ny), sat ? " SAT" : "");
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
    // 0.4.1: log every camera controller mode/instance change (not only during a drag), so a stuck
    // special camera after a load can be dated against the other mods' logs.
    if (haveCtl && (mode != g_lastMode || ctl != g_lastCtl)) {
        KP_LOG(chCam, "CAMMODE %d -> %d ctl=%08x (was %08x) yaw=%.4f pitch=%.4f drag=%d",
               g_lastMode, mode, (unsigned)ctl, (unsigned)g_lastCtl, yaw, pitch, drag);
        g_lastMode = mode; g_lastCtl = ctl;
    }
    bool pitchMoved = haveCtl && g_havePitch && fabsf(pitch - g_lastPitch) > 0.001f;
    if (drag || dx != 0.0f || dy != 0.0f || pitchMoved || (haveCtl && !g_havePitch)) {
        if (drag != g_drag || (haveCtl && !g_havePitch)) {
            if (haveCtl) {
                KP_LOG(chCam, "CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d mode=%d yaw=%.4f%s pitch=%.4f%s ctl94=%.4f ctl98=%.4f",
                       dx, tag(dx), dy, tag(dy), scale, tag(scale), drag, mode, yaw, tag(yaw), pitch, tag(pitch), pitch, yaw);
            } else {
                KP_LOG(chCam, "CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d ctl=unreadable", dx, tag(dx), dy, tag(dy), scale, tag(scale), drag);
            }
        } else {
            if (haveCtl) {
                KP_LOG_EVERY(chCam, 100, "CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d mode=%d yaw=%.4f%s pitch=%.4f%s ctl94=%.4f ctl98=%.4f",
                             dx, tag(dx), dy, tag(dy), scale, tag(scale), drag, mode, yaw, tag(yaw), pitch, tag(pitch), pitch, yaw);
            } else {
                KP_LOG_EVERY(chCam, 100, "CAM dx=%.4f%s dy=%.4f%s scale=%.3f%s drag=%d ctl=unreadable", dx, tag(dx), dy, tag(dy), scale, tag(scale), drag);
            }
        }
    }
    if (haveCtl) { g_lastPitch = pitch; g_havePitch = true; }
}

// ---------------- 0.3.0 click channel ----------------

// 0x72E039 inside OnMouseMessage: this=[ebp-0x1C], msg struct=[ebp+8]; msg id at msg+4, lParam at msg+0xC
// (x = LOWORD, y = HIWORD, y already flipped). this+0x3EC = SetCursorPos guard int.
void __cdecl ProbeMouseMsg(uintptr_t ebp) {
    enter();
    uintptr_t msgp = 0;
    if (!rd(ebp, 8, &msgp)) return;
    int id = 0;
    if (!rd(msgp, 4, &id) || !clicklogic::isMouseButtonMsg(id)) return;
    uint32_t lp = 0;
    if (!rd(msgp, 0xC, &lp)) return;
    uintptr_t self = 0; int guard = 0;
    if (rd(ebp, -0x1C, &self)) rd(self, 0x3EC, &guard);   // unreadable guard -> 0, the CLICK line still goes out
    int x = clicklogic::unpackX(lp), y = clicklogic::unpackY(lp);
    double t = nowMs();
    clicklogic::Verdict v = g_click.step(id, x, y, guard, t);
    if (KP_ENABLED(chClick)) {
        if (g_click.haveDelta)
            KP_LOG(chClick, "CLICK msg=0x%03x x=%d y=%d guard3EC=%d dtLmbDown=%.0f dpos=%.1f movept_bg=%d",
                   id, x, y, guard, g_click.dtMs, g_click.dpos, g_moveptBg);
        else
            KP_LOG(chClick, "CLICK msg=0x%03x x=%d y=%d guard3EC=%d dtLmbDown=none dpos=none movept_bg=%d", id, x, y, guard, g_moveptBg);
        g_moveptBg = 0;
    }
    loadFixConfig();
    bool synth = mousefix::shouldSynthesizeDblClick(id, v == clicklogic::Verdict::MissedDbl, g_fixCfg.enabled && g_fixCfg.synthDblClk);
    if (synth && saferead::probeWrite(SiteMsgWrite, reinterpret_cast<void*>(msgp + 4), 4)) {
        *reinterpret_cast<int*>(msgp + 4) = clicklogic::MsgLDbl;
        KP_LOG(chClick, "SYNTHDBL 0x203 synthesized from dropped 0x201 (dt=%.0f dpos=%.1f)", g_click.dtMs, g_click.dpos);
    } else if (v == clicklogic::Verdict::MissedDbl) {
        if (synth) KP_LOGV(chClick, KPLOG_WARN, "SYNTHDBL skipped: msg+4 not writable");
        KP_LOGV(chClick, KPLOG_WARN, "MISSEDDBL dt=%.0f dpos=%.1f", g_click.dtMs, g_click.dpos);
    }
    if (v == clicklogic::Verdict::DblGuard)
        KP_LOGV(chClick, KPLOG_WARN, "DBLGUARD 0x203 with guard3EC=%d", guard);
}

// 0x7B3560 PerformLButtonDownAction entry: ECX=this, [ESP+4]=isDbl (1 if the LMB axis value was -1)
void __cdecl ProbeLButtonDown(uintptr_t self, int isDbl) {
    enter();
    if (!KP_ENABLED(chClick)) return;
    uint32_t hov = 0, sel = 0, last = 0; float vx = 0, vy = 0, vz = 0;
    if (!rd(self, 0x4B0, &hov) || !rd(self, 0x2B4, &sel) || !rd(self, 0x2B8, &last)) return;
    rd(self, 0x4B4, &vx); rd(self, 0x4B8, &vy); rd(self, 0x4BC, &vz);
    KP_LOG(chClick, "DBLACT isDbl=%d hov=0x%08x sel=0x%08x last=0x%08x vec=(%.3f%s,%.3f%s,%.3f%s)",
           isDbl, hov, sel, last, vx, tag(vx), vy, tag(vy), vz, tag(vz));
}

// 0x7B3CA0 HandleMouseClickInWorld entry (fastcall): ECX=this
void __cdecl ProbeClickInWorld(uintptr_t self) {
    enter();
    if (!KP_ENABLED(chClick)) return;
    uint32_t hov = 0, sel = 0, last = 0; int nact = -1;
    if (!rd(self, 0x4B0, &hov) || !rd(self, 0x2B4, &sel) || !rd(self, 0x2B8, &last)) return;
    rd(self, 0x4D0, &nact);
    KP_LOG(chClick, "WORLDCLICK hov=0x%08x sel=0x%08x last=0x%08x nact=%d branch=%c",
           hov, sel, last, nact, clicklogic::isBranchB(hov, sel, last) ? 'B' : 'A');
}

// 0x7B3E3C branch B, just before the default-action call: this=[ebp-0x70], entry=[ebp-4] (+8 arg, +0xC fn, +0x1C type), actor=[ebp-8]
void __cdecl ProbeDefaultAction(uintptr_t ebp) {
    enter();
    if (!KP_ENABLED(chClick)) return;
    uintptr_t self = 0, entry = 0; uint32_t actor = 0, fn = 0, arg = 0, type = 0, hov = 0;
    if (!rd(ebp, -0x70, &self) || !rd(ebp, -4, &entry) || !rd(entry, 0xC, &fn)) return;
    rd(entry, 8, &arg); rd(entry, 0x1C, &type); rd(ebp, -8, &actor); rd(self, 0x4B0, &hov);
    KP_LOG(chClick, "DEFACT fn=0x%08x arg=0x%08x type=0x%08x actor=0x%08x hov=0x%08x", fn, arg, type, actor, hov);
}

// 0x564770 AddMoveToPointAction entry: ECX=creature, [ESP+0]=return address. Called often by AI/scripts: only
// logged within 3 s of an LMB message; otherwise counted and reported as movept_bg= on the next CLICK.
void __cdecl ProbeMoveToPoint(uintptr_t creature, uintptr_t caller) {
    enter();
    if (!g_click.recentLmb(nowMs())) { ++g_moveptBg; return; }
    KP_LOG(chClick, "MOVEPT caller=0x%08x obj=0x%08x", (unsigned)caller, (unsigned)creature);
}

// 0x65B4F0 HandlePlayerToServerInputWalkToWaypoint entry: [ESP+0]=return address. Expected never to fire.
void __cdecl ProbeWalkToWaypoint(uintptr_t caller) {
    enter();
    KP_LOG(chClick, "WALKWP caller=0x%08x", (unsigned)caller);
}

} // extern "C"
