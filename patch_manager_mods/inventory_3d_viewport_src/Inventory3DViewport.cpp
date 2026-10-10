// Inventory 3D Viewport v2 (Steam Aspyr build 6A522E71...). Shows the item under the cursor as a live 3D
// model in the ultrawide margin of the inventory screen, by reusing the Workbench's render function 008cd2c0
// with a private CSWGui3DSceneView block ("fake this" = block-0x3a30).
// v2: our own neutral light rig (inv3d_light), and the DLL owns the item transform (orientation per item
// class, centring, fit by distance/scale, auto-spin and mouse-drag rotation).
// Research trail: patch_manager_mods/01_inventory_3d_viewport.md (local-setup/KOTOR-II worktree).
//
// Hooks (cdecl detours, original bytes still run afterwards):
//   H1 0x008a69ad BindInventoryView  panel ctor 008a6170, before StopLoadFromLayout
//   H2 0x008a8172 ShowItem           entry event-0 handler 008a8100, after GetItemByObjectId
//   H3 0x008a7350 InventoryTick      inventory Update prologue: spin, drag, INI reload
// 0.3.0-dev (doc 32 MG3): KpLog (core file inventory-3d-viewport_log.txt, old core fallback inv3d_log.txt), IniKit primitives; lazy init from KPatchInit/first hook.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include "KpLog.h"
#include "IniKit.h"
#include "SafeRead.h"   // VirtualQuery probing: the Win32 bad-pointer probes fault-storm under Wine (DB lesson 234)
#include "SceneView.h"  // CSWGui3DSceneView block, scene models, Gob vtable guard (doc 23)
#include "Inv3dQuat.h"
#include "GuiInput.h"
#include "GuiKit.h"      // Common: screen sync, float cursor, wheel subclass (kp::WindowHook inside), bindByTag
#include "PanelSlots.h"  // Common: panel -> slot table
#include "Inv3dLogic.h"   // pure logic (INI parse/clamp, fit, cursor math): host-tested by inv3d_logic_test.cpp (doc 20 T1)
#include "ItemModelTable.inc"

KP_LOG_PATCH("inventory-3d-viewport");

#define INV3D_PRESENTATION 1   // 0 = v1 behaviour (no transform handling)
#define INV3D_DRAG 1           // 0 = no mouse polling

namespace {

// Scene view (block ctor, scene room, AddModel/GetModel/RemoveModel, camera, Gob guard/pose/scale): SceneView.h.
constexpr int FnRender = 0x008cd2c0;        // fastcall(fakeThis)
constexpr int SceneOffset = 0x60;           // view+0x60 = CSWGuiScene (dumped for diagnostics only)
constexpr int FakeCategoryBlockOff = 0x2fc;
constexpr int FakeItemBlockOff = 0x304;
constexpr int FakeThisDelta = 0x3a30;
constexpr int ExpectedControlId = 21;
constexpr int ItemVarOff = 0x2a4;           // item model variation byte
constexpr unsigned ExpectedCamVtable = gameaddr::SceneCameraVtable;   // logged only (SceneView.h reads the K2Addr table)
constexpr unsigned ExpectedGobVtable = gameaddr::GobVtable;

using Render = void(__fastcall*)(void*);
using sceneview::modelCount;
using sceneview::getModel;

// ---- diagnostics: KpLog (core file inventory-3d-viewport_log.txt; old core: fallback inv3d_log.txt at INFO), capped ----
KP_CHANNEL(chMain, "main");
KP_CHANNEL(chDbg, "debug");
// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("inv3d_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);
int g_logLines = 0;
constexpr int MaxLogLines = 12000;
bool g_debug = false;  // inv3d_debug.txt present -> per-tick detail
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    ++g_logLines;
    char buf[600]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    KP_LOGV(chMain, KPLOG_INFO, "%s", buf);
}
int g_warnLines = 0;
constexpr int MaxWarnLines = 4000;  // warnings can fire per ShowItem call while a slot stays unusable
void logw(const char* fmt, ...) {
    if (g_warnLines >= MaxWarnLines) return;
    ++g_warnLines;
    char buf[600]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    KP_LOGV(chMain, KPLOG_WARN, "%s", buf);
}
void loge(const char* fmt, ...) {
    char buf[600]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    KP_LOGV(chMain, KPLOG_ERR, "%s", buf);
}
int g_dbgLines = 0;
constexpr int MaxDebugLines = 6000;
void dlogf(const char* fmt, ...) {  // debug-only, own cap so it cannot starve the bind/error/drag lines
    if (!g_debug || g_dbgLines >= MaxDebugLines) return;
    ++g_dbgLines;
    char buf[600]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    KP_LOGV(chDbg, KPLOG_INFO, "%s", buf);
}
char g_loadLine[160];   // formatted by DllMain, emitted by ensureInit (the logger is only known after KPatchInit)

constexpr unsigned SiteInv3d = 0x0100;   // saferead site id for this DLL's own probes
void dumpMem(const char* tag, int addr, int words) {
    if (addr == 0) { logf("  dump %s @0: null", tag); return; }
    for (int i = 0; i < words; i += 8) {
        char line[200]; int p = 0;
        for (int j = i; j < i + 8 && j < words; ++j) {
            if (!saferead::probeRead(SiteInv3d, reinterpret_cast<const void*>(addr + j * 4), 4)) p += snprintf(line + p, sizeof(line) - p, " ????????");
            else p += snprintf(line + p, sizeof(line) - p, " %08x", *reinterpret_cast<const int*>(addr + j * 4));
        }
        logf("  dump %s +%03x:%s", tag, i * 4, line);
    }
}

template <typename T>
bool readAt(int base, int off, T* out) { return saferead::readAt<T>(SiteInv3d, static_cast<uintptr_t>(static_cast<unsigned>(base)), off, out); }

// ---- configuration: $GAME/inv3d.ini (missing/garbage = defaults; every key and rejected line logged) ----
using inv3d::Ov; using inv3d::Cfg; using inv3d::FitEntry;
Cfg g_cfg;
unsigned g_cfgGen = 0;
ULONGLONG g_iniStamp = 0; DWORD g_iniLastCheck = 0;
void syncScreen();

void loadIni() {
    Cfg c; inv3d::cfgDefaults(&c);
    g_debug = GetFileAttributesA("inv3d_debug.txt") != INVALID_FILE_ATTRIBUTES;
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (GetFileAttributesExA("inv3d.ini", GetFileExInfoStandard, &fa))
        g_iniStamp = ((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    else g_iniStamp = 0;
    FILE* f = fopen("inv3d.ini", "r");
    if (!f) { logf("INI: inv3d.ini not found, using defaults (debug=%d)", g_debug); g_cfg = c; ++g_cfgGen; syncScreen(); return; }
    char line[256]; int ln = 0, ok = 0, bad = 0;
    while (fgets(line, sizeof(line), f)) {
        ++ln;
        if (inv3d::drainLongLine(line, sizeof(line), f)) { logf("INI line %d rejected (longer than 255 chars)", ln); ++bad; continue; }
        inv3d::trim(line);
        char* k = nullptr; char* v = nullptr;
        inv3d::IniLine sp = inv3d::splitIniLine(line, &k, &v);
        if (sp == inv3d::IniLineSkip) continue;
        if (sp == inv3d::IniLineNoEquals) { logf("INI line %d rejected (no '='): %s", ln, line); ++bad; continue; }
        inv3d::IniResult r = inv3d::parseIniLine(c, k, v);
        if (r == inv3d::IniUnknown) { logf("INI line %d rejected (unknown key '%s')", ln, k); ++bad; continue; }
        if (r == inv3d::IniBad) { logf("INI line %d rejected (bad value for '%s': '%s')", ln, k, v); ++bad; }
        else { logf("INI %s = %s", k, v); ++ok; }
    }
    fclose(f);
    int clamped = inv3d::clampCfg(c);
    if (clamped & inv3d::ClampMargin) logf("INI fit_margin out of range, reset 1.15");
    if (clamped & inv3d::ClampWheelStep) logf("INI wheel_step out of range, reset 0.9");
    if (clamped & inv3d::ClampScreen) logf("INI screen_w/h invalid, reset 3840x1600");
    if (clamped & inv3d::ClampFov) logf("INI fov_deg out of range, reset 22.726");
    g_cfg = c; ++g_cfgGen;
    syncScreen();
    logf("INI loaded: %d keys ok, %d rejected, debug=%d gen=%u enabled=%d rig=%s", ok, bad, g_debug, g_cfgGen, c.enabled, c.rig);
}
void maybeReloadIni() {
    DWORD now = GetTickCount();
    if (now - g_iniLastCheck < 1000) return;
    g_iniLastCheck = now;
    WIN32_FILE_ATTRIBUTE_DATA fa; ULONGLONG st = 0;
    if (GetFileAttributesExA("inv3d.ini", GetFileExInfoStandard, &fa))
        st = ((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    bool dbg = GetFileAttributesA("inv3d_debug.txt") != INVALID_FILE_ATTRIBUTES;
    if (st != g_iniStamp || dbg != g_debug) { logf("INI changed on disk, reloading"); loadIni(); }
}

using inv3d::fitLookup;   // fit table lookup (sorted by row, var): Inv3dLogic.h

struct Pres {
    bool valid; int row, var, cat; Q base; float c[3], e[3]; float defPitch;
    float yaw, pitch, roll, zoom, zoom0, lastYaw, lastPitch; float D, s, fill; bool dirty;
    int dragMode; bool dragging; float lastX, lastY; DWORD idleUntil; unsigned cfgGen;
    const FitEntry* fit;
};
struct Slot {
    int panel; unsigned char* block; bool usable; bool rigFallback;
    int lastItem, lastBase, lastVar; unsigned seq;
    DWORD bindTick; DWORD lastBeat; Pres p;
};
panelslots::PanelSlots<Slot, 4> g_slots;
unsigned g_seq = 0;
bool g_guardLogged = false;

uintptr_t slotKey(int panel) { return static_cast<uintptr_t>(static_cast<unsigned>(panel)); }
Slot* findSlot(int panel) { return g_slots.find(slotKey(panel)); }
// Array index of a slot (slots are never released, so the used entries are the prefix and forEach order == array order).
int slotIndex(const Slot* s) {
    int idx = -1, i = 0;
    g_slots.forEach([&](Slot& e) { if (&e == s) idx = i; ++i; });
    return idx;
}
// ---- Gob access with the vtable guard (Design 8; checks in SceneView.h gobCheck, log lines kept here) ----
bool gobGuard(void* gob, const char* why) {
    uint32_t vtv = 0; sceneview::GobSlotCheck ck[sceneview::GobSlotCount]; int bad = 0;
    int r = sceneview::gobCheck(gob, &vtv, ck, &bad);
    void* vt = reinterpret_cast<void*>(static_cast<uintptr_t>(vtv));
    if (r == sceneview::GF_Unknown) { logf("GUARD(%s): engine address table unknown for this exe", why); return false; }
    if (r == sceneview::GF_Unreadable) { logf("GUARD(%s): gob %p unreadable", why, gob); return false; }
    if (r == sceneview::GF_VtUnreadable) { logf("GUARD(%s): vtable %p unreadable", why, vt); return false; }
    if (r == sceneview::GF_None) return true;
    if (!g_guardLogged) {
        for (auto& c : ck)
            if (c.got != c.expect) logf("GUARD(%s): slot 0x%x (%s) = %08x, expected %08x", why, c.off, c.name, c.got, c.expect);
        g_guardLogged = true;
        logf("GUARD: gob vtable %p (expected %08x); first 0x44 slots:", vt, ExpectedGobVtable);
        dumpMem("gobvt", static_cast<int>(vtv), 0x44);
    }
    return false;
}

// ---- presentation ----
void computeFit(Pres* p, float aspect) {
    inv3d::FitResult fr;
    inv3d::computeFit(g_cfg, p->base, p->e, p->defPitch, p->cat, aspect, &fr);
    p->fill = fr.fill; p->D = fr.D; p->s = fr.s;
    logf("  fit: ext'=[%.3f %.3f %.3f] h=%.3f v=%.3f aspect=%.3f tanV=%.4f fill=%.2f need=%.3f -> D=%.3f s=%.3f (d0=%.3f mode=%s)",
         fr.ex, fr.ey, fr.ez, fr.hs, fr.vs, fr.aspect, fr.tanV, fr.fill, fr.need, fr.D, fr.s, fr.d0, g_cfg.fitScale ? "scale" : "distance");
}

void applyTransform(Slot* s, const char* why) {
#if INV3D_PRESENTATION
    Pres& p = s->p;
    if (!p.valid) return;
    unsigned char* view = s->block;
    int n = modelCount(view);
    if (n < 2) { dlogf("apply(%s): model count %d < 2, skip", why, n); return; }
    void* gob = getModel(view, 1);
    if (!gobGuard(gob, why)) { p.valid = false; logf("apply(%s): guard failed, presentation off for this item", why); return; }
    float right[3]; inv3d::camBasis(g_cfg, right);
    Q qy = qaxis(g_cfg.camUp, p.yaw), qp = qaxis(right, p.pitch);
    Q qr = qaxis(g_cfg.camFwd, p.roll);
    Q q = qnormalize(qmul(qr, qmul(qp, qmul(qy, p.base))));
    float Deff = fmaxf(0.1f, p.D * p.zoom);
    float sc[3] = {p.c[0] * p.s, p.c[1] * p.s, p.c[2] * p.s}, rc[3];
    qrotate(q, sc, rc);
    float pos[3];
    for (int k = 0; k < 3; ++k) pos[k] = g_cfg.camPos[k] + g_cfg.camFwd[k] * Deff - rc[k];
    float cur = 0;
    if (sceneview::gobGetScale(gob, &cur) && cur > 0.01f && cur < 100 && fabsf(cur - p.s) > 1e-6f && sceneview::gobSetScale(gob, p.s)) {
        dlogf("apply(%s): scale %.4f -> %.4f", why, cur, p.s);
    }
    sceneview::gobSetPosition(gob, pos[0], pos[1], pos[2]);
    sceneview::gobSetOrientation(gob, q.w, q.x, q.y, q.z);
    p.lastYaw = p.yaw; p.lastPitch = p.pitch; p.dirty = false;
    static DWORD lastSpinLog = 0; DWORD nowT = GetTickCount();
    if (strcmp(why, "spin") || nowT - lastSpinLog >= 1000) { if (!strcmp(why, "spin")) lastSpinLog = nowT;
    dlogf("apply(%s): gob=%p yaw=%.1f pitch=%.1f roll=%.1f zoom=%.2f D=%.3f q=(%.3f %.3f %.3f %.3f) pos=(%.3f %.3f %.3f)", why, gob, p.yaw, p.pitch, p.roll, p.zoom, Deff, q.w, q.x, q.y, q.z, pos[0], pos[1], pos[2]); }
#endif
}

void setupPresentation(Slot* s, int row, int var, int cat, const FitEntry* fe) {
#if INV3D_PRESENTATION
    Pres& p = s->p;
    memset(&p, 0, sizeof(p));
    p.row = row; p.var = var; p.cat = cat; p.fit = fe; p.cfgGen = g_cfgGen; p.idleUntil = 0;
    unsigned char* view = s->block;
    void* gob = getModel(view, 1);
    logf("PRES row=%d var=%d cat=%d gob=%p", row, var, cat, gob);
    if (!g_cfg.enabled) { logf("  presentation disabled (enabled=0)"); return; }
    if (!gob) { logf("  no item gob, skip"); return; }
    if (!gobGuard(gob, "setup")) return;
    int vt = 0; readAt<int>(reinterpret_cast<int>(gob), 0, &vt);
    logf("  gob vtable=%08x (expect %08x)", vt, ExpectedGobVtable);
    void* par = nullptr; sceneview::gobGetParent(gob, &par, -1);
    float pos[3], q[4]; sceneview::gobGetPose(gob, pos, q);
    float sc = -1; sceneview::gobGetScale(gob, &sc);
    logf("  parent=%p (expect 0) pos=(%.3f %.3f %.3f) quat=(%.3f %.3f %.3f %.3f) scale=%.3f", par, pos[0], pos[1], pos[2], q[0], q[1], q[2], q[3], sc);
    if (par) { logf("  item has an attached parent (%p): presentation skipped (rig hooks in play)", par); return; }
    void* g0 = getModel(view, 0);
    if (g0 && gobGuard(g0, "rig")) { float p0[3], q0[4]; sceneview::gobGetPose(g0, p0, q0); logf("  rig gob=%p pos=(%.3f %.3f %.3f) quat=(%.3f %.3f %.3f %.3f)", g0, p0[0], p0[1], p0[2], q0[0], q0[1], q0[2], q0[3]); }
    uint32_t camv = 0, cam = 0; sceneview::camera(view, &cam, &camv);
    logf("  camera=%08x vtable=%08x (expect %08x) rigFallback=%d", cam, camv, ExpectedCamVtable, s->rigFallback);

    if (cat == 4) {
        for (int k = 0; k < 3; ++k) { p.c[k] = g_cfg.armorBox[k]; p.e[k] = g_cfg.armorBox[3 + k]; }
        logf("  armour box c=(%.2f %.2f %.2f) e=(%.2f %.2f %.2f)", p.c[0], p.c[1], p.c[2], p.e[0], p.e[1], p.e[2]);
    } else if (fe && !(fe->flags & 1)) {
        for (int k = 0; k < 3; ++k) { p.c[k] = fe->c[k]; p.e[k] = fe->e[k]; }
        logf("  fit entry c=(%.3f %.3f %.3f) e=(%.3f %.3f %.3f)", p.c[0], p.c[1], p.c[2], p.e[0], p.e[1], p.e[2]);
    } else {
        p.c[0] = p.c[1] = p.c[2] = 0; p.e[0] = p.e[1] = p.e[2] = 0.3f;
        logf("  no usable fit entry (%s), default box 0.3", fe ? "EMPTY" : "missing");
    }
    float o[5] = {0, 0, 0, 15, 1}; const char* src = "fallback";
    if (row >= 0 && row < kItemRows) { memcpy(o, kDefaultOrient[row], 16); o[4] = 1; src = "table"; }
    int it = (row >= 0 && row < kItemRows) ? kItemType[row] : -1;
    if (it >= 0 && it < 256 && g_cfg.itype[it].set) { memcpy(o, g_cfg.itype[it].v, 20); src = "ini itype"; }
    if (row >= 0 && row < 256 && g_cfg.row[row].set) { memcpy(o, g_cfg.row[row].v, 20); src = "ini row"; }
    p.base = qbase(o[0], o[1], o[2]); p.defPitch = o[3];
    logf("  orient rx=%.0f ry=%.0f rz=%.0f pitch=%.0f zoom=%.2f (%s, itype=%d)", o[0], o[1], o[2], o[3], o[4], src, it);
    int ext[4] = {0}; for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(view), 4 + 4 * i, &ext[i]);
    float aspect = ext[3] > 0 ? (float)ext[2] / (float)ext[3] : 1.0f;
    computeFit(&p, aspect);
    p.yaw = 0; p.pitch = p.defPitch; p.roll = 0; p.zoom0 = fminf(fmaxf(o[4], g_cfg.zoomMin), g_cfg.zoomMax); p.zoom = p.zoom0; p.valid = true; p.dirty = true;
    static int dumps = 0;
    if (dumps < 10) { ++dumps; dumpMem("gob", reinterpret_cast<int>(gob), 24); }
    applyTransform(s, "setup");
#endif
}

// Engine framebuffer size (GuiKit reads the K2Addr ScreenW/ScreenH globals): used unless the INI sets screen_w/screen_h explicitly.
// Kept outside INV3D_DRAG: loadIni() calls it in every build.
void syncScreen() {
    int w = (int)g_cfg.screenW, h = (int)g_cfg.screenH;
    if (guikit::syncScreen(g_cfg.screenExplicit ? w : 0, g_cfg.screenExplicit ? h : 0, &w, &h)) { g_cfg.screenW = (float)w; g_cfg.screenH = (float)h; }
}

// ---- mouse drag (Design 6) ----
#if INV3D_DRAG
HWND g_hwnd = nullptr;
struct EnumCtx { DWORD pid; HWND best; long area; };
BOOL CALLBACK enumProc(HWND h, LPARAM lp) {
    EnumCtx* c = reinterpret_cast<EnumCtx*>(lp);
    DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
    if (pid != c->pid || !IsWindowVisible(h)) return TRUE;
    RECT r; if (!GetClientRect(h, &r)) return TRUE;
    long a = (r.right - r.left) * (r.bottom - r.top);
    if (a > c->area) { c->area = a; c->best = h; }
    return TRUE;
}
// ---- mouse wheel: GuiKit.h subclasses the game window (only consumes wheel events over the 3D view while the inventory ticks) ----
void wheelEvent(int delta, int px, int py, bool inside) { logf("WHEEL: delta=%d cursor=(%d,%d) inside=%d", delta, px, py, inside); }
bool g_wheelCfgSet = false;

HWND gameWindow() {
    if (g_hwnd && IsWindow(g_hwnd)) return g_hwnd;
    static DWORD lastTry = 0; DWORD nowT = GetTickCount();
    if (lastTry && nowT - lastTry < 2000) return nullptr;
    lastTry = nowT;
    EnumCtx c = {GetCurrentProcessId(), nullptr, 0};
    EnumWindows(enumProc, reinterpret_cast<LPARAM>(&c));
    g_hwnd = c.best;
    RECT r = {0, 0, 0, 0}; if (g_hwnd) GetClientRect(g_hwnd, &r);
    logf("DRAG: hwnd=%p client=%ldx%ld (engine %.0fx%.0f)", g_hwnd, r.right, r.bottom, g_cfg.screenW, g_cfg.screenH);
    if (g_hwnd && g_cfg.wheelZoom && !guikit::g_wheelHwnd) {
        if (!g_wheelCfgSet) { g_wheelCfgSet = true; guikit::g_wheel.scoped = true; guikit::g_wheel.onEvent = wheelEvent; }
        bool ok = guikit::wheelInstall();
        logf("WHEEL: subclassed window %p, old proc %p (err=%lu)", (void*)guikit::g_wheelHwnd, (void*)guikit::detail::WheelWin::old, ok ? 0UL : GetLastError());
    }
    return g_hwnd;
}
bool cursorEngine(float* x, float* y) {
    if (!gameWindow()) return false;
    syncScreen();
    return guikit::cursorEngineF(x, y, (int)g_cfg.screenW, (int)g_cfg.screenH);
}
#endif

LARGE_INTEGER g_qpcFreq, g_qpcLast;

void tickDrag(Slot* s, float dt) {
    (void)dt;
#if INV3D_DRAG
    Pres& p = s->p;
    static bool wasL = false, wasR = false, wasM = false; static int presses = 0;
    bool l = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0, r = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0, mb = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    float cx = 0, cy = 0; bool haveCur = cursorEngine(&cx, &cy);
    int ext[4] = {0}; for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(s->block), 4 + 4 * i, &ext[i]);
    float top = inv3d::flipTop(g_cfg.hitFlipY, g_cfg.screenH, ext[1], ext[3]);
    guikit::g_wheel.setRect(0, ext[0], (int)top, ext[2], ext[3]);
    bool inside = haveCur && inv3d::rectContainsTop(cx, cy, ext[0], top, ext[2], ext[3]);
    bool fg = gameWindow() && GetForegroundWindow() == g_hwnd;
    bool anyEdge = (l && !wasL) || (r && !wasR) || (mb && !wasM);
    if (anyEdge && presses < 30) { ++presses; logf("DRAG: press #%d L=%d R=%d M=%d shift=%d cursor=(%.0f,%.0f) rect=[%d,%d,%d,%d] top=%.0f flip=%d inside=%d fg=%d", presses, l, r, mb, shift, cx, cy, ext[0], ext[1], ext[2], ext[3], top, g_cfg.hitFlipY, inside, fg); }
    if (inside && fg && p.valid && !p.dragMode) {
        if (mb && !wasM) {  // reset view
            p.yaw = 0; p.pitch = p.defPitch; p.roll = 0; p.zoom = p.zoom0; p.dirty = true; p.idleUntil = 0;
            logf("DRAG: view reset (MMB) pitch=%.0f zoom=%.2f", p.pitch, p.zoom);
        } else if (l && !wasL) p.dragMode = shift ? 2 : 1;
        else if (r && !wasR) p.dragMode = 3;
        if (p.dragMode) { p.dragging = true; p.lastX = cx; p.lastY = cy; logf("DRAG: start mode=%d (1=rotate 2=roll 3=zoom) at (%.0f,%.0f) yaw=%.1f pitch=%.1f roll=%.1f zoom=%.2f", p.dragMode, cx, cy, p.yaw, p.pitch, p.roll, p.zoom); }
    }
    if (p.dragMode) {
        bool held = p.dragMode == 3 ? r : l;
        if (!held) {
            logf("DRAG: end mode=%d yaw=%.1f pitch=%.1f roll=%.1f zoom=%.2f", p.dragMode, p.yaw, p.pitch, p.roll, p.zoom);
            if (p.dragMode != 3) p.idleUntil = GetTickCount() + (DWORD)(g_cfg.idleResume * 1000);
            p.dragMode = 0; p.dragging = false;
        } else if (haveCur) {
            float dx = cx - p.lastX, dy = cy - p.lastY;
            if (dx != 0 || dy != 0) {
                if (p.dragMode == 1) {
                    p.yaw += dx * g_cfg.dragDegPx; p.pitch += dy * g_cfg.dragDegPx;
                    if (p.pitch > g_cfg.pitchClamp) p.pitch = g_cfg.pitchClamp;
                    if (p.pitch < -g_cfg.pitchClamp) p.pitch = -g_cfg.pitchClamp;
                } else if (p.dragMode == 2) p.roll += dx * g_cfg.rollDegPx;
                else { p.zoom *= expf(dy * g_cfg.zoomDrag); p.zoom = fminf(fmaxf(p.zoom, g_cfg.zoomMin), g_cfg.zoomMax); }
                p.lastX = cx; p.lastY = cy; p.dirty = true;
                dlogf("DRAG: move d=(%.0f,%.0f) yaw=%.1f pitch=%.1f roll=%.1f zoom=%.2f", dx, dy, p.yaw, p.pitch, p.roll, p.zoom);
            }
        }
    }
    LONG wheel = guikit::g_wheel.take();
    if (wheel && p.valid) {
        p.zoom *= powf(g_cfg.wheelStep, wheel / 120.0f); p.zoom = fminf(fmaxf(p.zoom, g_cfg.zoomMin), g_cfg.zoomMax); p.dirty = true;
        logf("WHEEL: applied %ld -> zoom=%.2f", wheel, p.zoom);
    }
    wasL = l; wasR = r; wasM = mb;
#endif
}

// Cover labels (character-page style arcs) that frame the 3D view. Only controls the constructor binds by tag get created,
// so they are built here right after the view (control order = draw order, so they draw on top of it).
void bindCover(int panel, const char* tagName, int expectId) {
    guikit::BindInfo bi;
    guikit::bindByTag(static_cast<uintptr_t>(static_cast<unsigned>(panel)), tagName, expectId, 0, &bi);
    if (bi.why == guikit::CF_Alloc) { logw("H1 cover %s: allocation failed", tagName); return; }
    if (bi.why == guikit::CF_Vtable) { logw("H1 cover %s: ctor left vtable %08x (expected %08x), not binding", tagName, bi.badVtable, (unsigned)gameaddr::LabelVtable); return; }
    logf("H1 cover %s: ctl=%p id=%d (expect %d) extent=[%d,%d,%d,%d] flags=%08x -> %08x", tagName, (void*)bi.ctl, bi.id, expectId, bi.ext[0], bi.ext[1], bi.ext[2], bi.ext[3], bi.flagsBefore, bi.flagsAfter);
}

// Lazy init: load line + first loadIni(). Runs from KPatchInit, else from the first hook (old core without KPatchInit).
volatile LONG g_inited = 0;
void ensureInit() {
    if (g_inited) return;                                 // cheap fast path
    if (InterlockedExchange(&g_inited, 1)) return;
    logf("%s", g_loadLine);
    loadIni();
}

} // namespace

extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    guikit::initGuiInput(api, "inventory-3d-viewport");
    ensureInit();
}

extern "C" void __cdecl BindInventoryView(int panel) {
    ensureInit();
    saferead::beginScope();   // engine code ran since the last hook: drop the readable-region cache
    int ctlCount = -1; readAt<int>(panel, 0x28, &ctlCount);
    int gate = 0; sceneview::gateOpen(&gate);
    int stackLocal = 0;
    logf("H1 BindInventoryView panel=%08x controls=%d gate=%d stack=%p", panel, ctlCount, gate, &stackLocal);
    if (!panel) { logf("H1: null panel, abort"); return; }

    // alloc 0x400 + CSWGuiControl ctor + scene-view vtable + scene ctor + InitControl("3D_MODEL", addToList=1, scale=1)
    unsigned char* block = sceneview::createView(static_cast<uintptr_t>(static_cast<unsigned>(panel)), "3D_MODEL");
    if (!block) { loge("H1: operator new failed"); return; }
    unsigned char* view = block;

    int id = -999, newCount = -1; int ext[4] = {0};
    readAt<int>(reinterpret_cast<int>(view), 0x54, &id);
    for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(view), 4 + i * 4, &ext[i]);
    readAt<int>(panel, 0x28, &newCount);
    logf("H1: block=%p control id=%d (expect %d) extent=[%d,%d,%d,%d] aspect=%.3f controls now %d (expect %d)",
         block, id, ExpectedControlId, ext[0], ext[1], ext[2], ext[3], ext[3] ? (float)ext[2] / ext[3] : 0.f, newCount, ctlCount + 1);
    dumpMem("view", reinterpret_cast<int>(view), 38);

    Slot* s = g_slots.claim(slotKey(panel), ++g_seq);
    memset(s, 0, sizeof(*s));   // PanelSlots keeps its key/seq/used bookkeeping outside Slot
    s->panel = panel; s->block = block; s->seq = g_seq; s->bindTick = GetTickCount();

    if (id != ExpectedControlId) { logw("H1: GUI control missing (id=%d), slot unusable", id); return; }
    if (!gate) { logw("H1: gate global 009f42b4 is zero, slot unusable"); return; }

    uintptr_t scene = 0;
    int sf = sceneview::initScene(view, "gui3D_room", &scene);
    if (sf == sceneview::SF_VtableBad) { logw("H1: scene vtable bad, slot unusable"); return; }
    if (sf != sceneview::SF_None) { logw("H1: scene pointer bad (%08x), slot unusable", (unsigned)scene); return; }
    logf("H1: scene %08x initialised (gui3D_room)", (unsigned)scene);

    const char* rig = g_cfg.enabled ? g_cfg.rig : "upgitem_light";
    void* light = nullptr; int n = -1;
    for (int attempt = 0; attempt < 2; ++attempt) {
        light = sceneview::addModel(view, rig, -1);
        n = modelCount(view);
        logf("H1: rig '%s' model=%p, scene model count=%d", rig, light, n);
        if (light && n >= 1) break;
        if (attempt == 0 && strcmp(rig, "upgitem_light")) { logw("H1: rig '%s' failed, falling back to upgitem_light", rig); rig = "upgitem_light"; s->rigFallback = true; }
    }
    if (!light || n < 1) { logw("H1: light model missing, slot unusable"); return; }
    uint32_t cam = 0, camv = 0; sceneview::camera(view, &cam, &camv);
    logf("H1: camera=%08x vtable=%08x (expect %08x)", cam, camv, ExpectedCamVtable);
    if (g_cfg.bevels) { bindCover(panel, "LBL_BEVEL", ExpectedControlId + 1); bindCover(panel, "LBL_BEVEL2", ExpectedControlId + 2); }
    s->usable = true;
    logf("H1: slot %d ready (panel=%08x seq=%u rigFallback=%d)", slotIndex(s), panel, s->seq, s->rigFallback);
}

extern "C" void __cdecl ShowItem(int item, int panel) {
    ensureInit();
    saferead::beginScope();
    static int repeatLogs = 0, detailLogs = 0;
    Slot* s = findSlot(panel);
    if (!s) { logf("H2: no slot for panel=%08x", panel); return; }
    if (!s->usable) { logw("H2: slot unusable (panel=%08x)", panel); return; }
    if (!item) { logf("H2: null item"); return; }
    unsigned char* view = s->block;

    int id = -1; unsigned char v0 = 0, v1 = 0;
    if (!readAt<int>(item, 0xc, &id) || !readAt<unsigned char>(item, ItemVarOff, &v0) || !readAt<unsigned char>(item, ItemVarOff + 2, &v1)) {
        logw("H2: item %08x unreadable", item); return;
    }
    int var = v0 | (v1 << 8);
    if (id < 0 || id >= kItemRows) { logw("H2: base item id %d out of range", id); return; }
    int cat = kItemCategory[id];
    if ((g_cfg.onlyWeaponsArmor) && !kIsWeaponOrArmor[id]) cat = 0;
    if (item == s->lastItem && id == s->lastBase && var == s->lastVar) {
        if (repeatLogs < 20) { ++repeatLogs; logf("H2: same item %08x base=%d var=%04x, skip", item, id, var); }
        return;
    }
    int n = modelCount(view);
    s->p.valid = false; s->p.dragging = false;
    if (cat == 0) {
        if (n >= 2) { sceneview::removeModel(view, 1, true); saferead::beginScope(); }
        s->lastItem = item; s->lastBase = id; s->lastVar = var;
        logf("H2: item %08x base=%d var=%04x cat=0 (no preview), models %d -> %d", item, id, var, n, modelCount(view));
        return;
    }
    if (n < 1) { logw("H2: light model gone (count=%d), slot unusable", n); s->usable = false; return; }

    // Design 4: missing/EMPTY variation -> substitute var 1 (item bytes restored on every path)
    int effVar = v0; bool substituted = false;
    int savedVarWord = 0; bool haveSaved = readAt<int>(item, ItemVarOff, &savedVarWord);
    struct VarGuard {   // restores the real item's variation word on every exit path (never leave the live CItem mutated)
        int item; int saved; bool armed;
        ~VarGuard() { if (armed && saferead::probeWrite(SiteInv3d, reinterpret_cast<void*>(item + ItemVarOff), 4)) *reinterpret_cast<int*>(item + ItemVarOff) = saved; }
    } guard = {item, savedVarWord, false};
    const FitEntry* fe = (cat == 2) ? fitLookup(id, v0) : nullptr;
    auto subst = [&](const char* why) {
        if (!haveSaved || !saferead::probeWrite(SiteInv3d, reinterpret_cast<void*>(item + ItemVarOff), 4)) { logf("H2: cannot substitute (%s): item bytes unwritable", why); return false; }
        *reinterpret_cast<unsigned char*>(item + ItemVarOff) = 1;
        guard.armed = true; effVar = 1; substituted = true; fe = fitLookup(id, 1);
        logf("H2: substituting var %d -> 1 (%s), item %08x", v0, why, item);
        return true;
    };
    if (cat == 2 && fe && (fe->flags & 1)) subst("EMPTY variation");  // a variation missing from the table is tried as-is; the retry below covers real misses

    auto doRender = [&]() {
        *(view + FakeCategoryBlockOff) = static_cast<unsigned char>(cat);
        *reinterpret_cast<int*>(view + FakeItemBlockOff) = item;
        reinterpret_cast<Render>(FnRender)(view - FakeThisDelta);
        *reinterpret_cast<int*>(view + FakeItemBlockOff) = 0;
        saferead::beginScope();   // the render created/freed scene models: drop the readable-region cache
    };
    doRender();
    int n2 = modelCount(view);
    if (cat == 2 && n2 < 2 && !substituted) {
        logf("H2: model count did not grow (%d), retrying with var 1", n2);
        if (subst("no model after render")) { doRender(); n2 = modelCount(view); }
    }
    if (guard.armed) { guard.armed = false; if (saferead::probeWrite(SiteInv3d, reinterpret_cast<void*>(item + ItemVarOff), 4)) *reinterpret_cast<int*>(item + ItemVarOff) = savedVarWord; }  // restore all 4 bytes (guard covers early exits)
    if (cat == 2 && n2 < 2) {
        logf("H2: item %08x base=%d var=%04x: still no model after render (count=%d), no preview", item, id, var, n2);
        s->lastItem = item; s->lastBase = id; s->lastVar = var;
        return;
    }
    s->lastItem = item; s->lastBase = id; s->lastVar = var;
    logf("H2: rendered item %08x base=%d var=%04x cat=%d effVar=%d subst=%d, models %d -> %d", item, id, var, cat, effVar, substituted, n, n2);
    if (detailLogs < 10) { ++detailLogs; dumpMem("scene", reinterpret_cast<int>(view + SceneOffset), 16); }
    setupPresentation(s, id, effVar, cat, fe);
}

extern "C" void __cdecl InventoryTick(int panel) {
    ensureInit();
    saferead::beginScope();
    static int calls = 0;
    if (!g_qpcFreq.QuadPart) { QueryPerformanceFrequency(&g_qpcFreq); QueryPerformanceCounter(&g_qpcLast); }
    LARGE_INTEGER now; QueryPerformanceCounter(&now);
    float dt = (float)((double)(now.QuadPart - g_qpcLast.QuadPart) / (double)g_qpcFreq.QuadPart);
    g_qpcLast = now;
    if (dt > 0.5f) logf("H3: tick gap %.2fs (inventory reopened or stalled)", dt);
    if (dt > 0.1f) dt = 0.1f;
    if (calls < 5) { ++calls; Slot* s0 = findSlot(panel); logf("H3 tick #%d panel=%08x slot=%s", calls, panel, s0 ? (s0->usable ? "usable" : "unusable") : "none"); }
    Slot* s = findSlot(panel);
    if (!s || !s->usable || modelCount(s->block) < 2) return;
    maybeReloadIni();
    Pres& p = s->p;
    if (p.valid && p.cfgGen != g_cfgGen) {  // INI changed: recompute this item's presentation
        logf("H3: config changed (gen %u -> %u), re-running presentation for row %d", p.cfgGen, g_cfgGen, p.row);
        setupPresentation(s, p.row, p.var, p.cat, p.fit);
    }
    if (!p.valid) return;
#if INV3D_PRESENTATION
    syncScreen();
    guikit::g_wheel.enabled = g_cfg.wheelZoom; guikit::g_wheel.iniW = (int)g_cfg.screenW; guikit::g_wheel.iniH = (int)g_cfg.screenH;
    guikit::g_wheel.touch(GetTickCount());
    tickDrag(s, dt);
    DWORD t = GetTickCount();
    bool spinning = false;
    if (!p.dragging && t >= p.idleUntil && g_cfg.spin != 0 && (p.cat != 4 || g_cfg.armorSpin)) {
        if (p.idleUntil) { logf("H3: spin resumed"); p.idleUntil = 0; }
        p.yaw += -g_cfg.spin * dt; if (p.yaw < -720) p.yaw += 720;
        p.dirty = true; spinning = true;
    }
    if (p.dirty) applyTransform(s, spinning ? "spin" : "drag");
    if (t - s->bindTick < 60000 && t - s->lastBeat >= 1000) {
        s->lastBeat = t;
        logf("H3 beat: slot=%d row=%d yaw=%.1f pitch=%.1f D=%.3f s=%.3f drag=%d model=%p", slotIndex(s), p.row, p.yaw, p.pitch, p.D, p.s, p.dragging, getModel(s->block, 1));
    }
#endif
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_DETACH) guikit::wheelUninstall();   // restore if still on top, else stay chained but inert
    if (reason == DLL_PROCESS_ATTACH) { snprintf(g_loadLine, sizeof g_loadLine, "inv3d v2 0.3.0-dev loaded (Common SafeRead + SceneView + GuiKit + PanelSlots + KpLog) tick=%lu", (unsigned long)GetTickCount()); }
    return TRUE;
}
