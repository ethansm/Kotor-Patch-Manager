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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include "Inv3dQuat.h"
#include "ItemModelTable.inc"
#include "ItemFitTable.inc"
#include "RigCamera.inc"

#define INV3D_PRESENTATION 1   // 0 = v1 behaviour (no transform handling)
#define INV3D_DRAG 1           // 0 = no mouse polling

namespace {

constexpr int GateGlobal = 0x009f42b4;
constexpr int FnControlCtor = 0x00418990;   // CSWGuiControl ctor, ECX = ctrl
constexpr int ViewVftable = 0x009a3d34;     // CSWGui3DSceneView
constexpr int FnSceneCtor = 0x00417f40;     // CSWGuiScene ctor, ECX = view+0x60
constexpr int FnInitControl = 0x0040f620;   // thiscall(panel, view, CExoString*, addToList, scale) RET 16
constexpr int FnExoCtor = 0x00733570;
constexpr int FnExoDtor = 0x00733780;
constexpr int FnNew = 0x00919723;           // cdecl(size)
constexpr int FnAddModel = 0x00418360;      // thiscall(view+0x60, CExoString*, int)
constexpr int FnRemoveModel = 0x00418430;   // thiscall(view+0x60, index, deleteIt) RET 8
constexpr int FnGetModel = 0x004184b0;      // thiscall(view+0x60, int idx) -> Gob*
constexpr int FnRender = 0x008cd2c0;        // fastcall(fakeThis)
constexpr int SceneOffset = 0x60;
constexpr int SceneObjOffset = 0x74;        // view+0x60+0x14 = Scene*
constexpr int CameraOffset = 0x78;          // view+0x78 = camera object
constexpr int ModelCountOffset = 0x80;      // view+0x80 = scene+0x20
constexpr int FakeCategoryBlockOff = 0x2fc;
constexpr int FakeItemBlockOff = 0x304;
constexpr int FakeThisDelta = 0x3a30;
constexpr int BlockSize = 0x400;
constexpr int ExpectedControlId = 21;
constexpr int GobScaleOff = 0x1e0;          // float, 1.0 = none (render 004b3c90 glScalef)
constexpr int ItemVarOff = 0x2a4;           // item model variation byte
constexpr int ExpectedCamVtable = 0x0098c45c;
constexpr int ExpectedGobVtable = 0x0098b5cc;

struct GobSlotCheck { int off; int addr; const char* name; };
const GobSlotCheck kGobSlots[] = {
    {0x5C, 0x00461490, "SetPosition"}, {0x60, 0x004614e0, "SetOrientation"},
    {0x64, 0x00459710, "GetPosition"}, {0x68, 0x00459740, "GetOrientation"}, {0x108, 0x00461530, "GetParent"}};

using Fast1 = void(__fastcall*)(void*);
using ThisCall2 = void*(__thiscall*)(void*, void*, int);
using GetModelFn = void*(__thiscall*)(void*, int);
using InitCtl = void(__thiscall*)(void*, void*, void*, int, int);
using CExoCtor = void*(__thiscall*)(void*, const char*);
using CExoDtor = void(__thiscall*)(void*);
using Remove = void(__thiscall*)(void*, int, int);
using SceneInit = void(__thiscall*)(void*, const char*, float*, float*);
using NewFn = void*(__cdecl*)(unsigned);
using Render = void(__fastcall*)(void*);
using SetPosFn = void*(__thiscall*)(void*, float*, float, float, float);
using SetOriFn = void*(__thiscall*)(void*, float*, float, float, float, float);
using GetVecFn = void*(__thiscall*)(void*, float*);
using GetParentFn = void*(__thiscall*)(void*, int);

// ---- diagnostics: inv3d_log.txt in the game dir, capped ----
int g_logLines = 0;
constexpr int MaxLogLines = 12000;
bool g_debug = false;  // inv3d_debug.txt present -> per-tick detail
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    FILE* f = fopen("inv3d_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}
int g_dbgLines = 0;
constexpr int MaxDebugLines = 6000;
void dlogf(const char* fmt, ...) {  // debug-only, own cap so it cannot starve the bind/error/drag lines
    if (!g_debug || g_dbgLines >= MaxDebugLines) return;
    FILE* f = fopen("inv3d_log.txt", "a");
    if (!f) return;
    ++g_dbgLines;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

void dumpMem(const char* tag, int addr, int words) {
    if (addr == 0) { logf("  dump %s @0: null", tag); return; }
    for (int i = 0; i < words; i += 8) {
        char line[200]; int p = 0;
        for (int j = i; j < i + 8 && j < words; ++j) {
            if (IsBadReadPtr(reinterpret_cast<const void*>(addr + j * 4), 4)) p += snprintf(line + p, sizeof(line) - p, " ????????");
            else p += snprintf(line + p, sizeof(line) - p, " %08x", *reinterpret_cast<const int*>(addr + j * 4));
        }
        logf("  dump %s +%03x:%s", tag, i * 4, line);
    }
}

template <typename T>
bool readAt(int base, int off, T* out) {
    if (base == 0 || IsBadReadPtr(reinterpret_cast<const void*>(base + off), sizeof(T))) return false;
    *out = *reinterpret_cast<const T*>(base + off);
    return true;
}

// ---- configuration: $GAME/inv3d.ini (missing/garbage = defaults; every key and rejected line logged) ----
struct Ov { bool set; float v[4]; };
struct Cfg {
    int enabled = 1; char rig[64] = "inv3d_light"; int onlyWeaponsArmor = 0;
    float spin = 70, dragDegPx = 0.5f, pitchClamp = 80, idleResume = 1.5f;
    int fitScale = 0; float margin = 1.15f, distMin = 0.25f, distMax = 4.0f;
    float fov = 22.726f; char fovAxis = 'v';
    float camPos[3], camFwd[3], camUp[3];
    float armorBox[6] = {0, 0, 0.95f, 0.6f, 0.4f, 1.9f}; int armorSpin = 1; int hitFlipY = 0;
    float screenW = 3840, screenH = 1600;
    Ov itype[256], row[256];
};
Cfg g_cfg;
unsigned g_cfgGen = 0;
ULONGLONG g_iniStamp = 0; DWORD g_iniLastCheck = 0;

void cfgDefaults(Cfg* c) {
    *c = Cfg();
    for (int i = 0; i < 3; ++i) { c->camPos[i] = kRigCamPos[i]; c->camFwd[i] = kRigCamFwd[i]; c->camUp[i] = kRigCamUp[i]; }
    memset(c->itype, 0, sizeof(c->itype)); memset(c->row, 0, sizeof(c->row));
}
void trim(char* s) {
    char* p = s; while (*p && isspace((unsigned char)*p)) ++p;
    memmove(s, p, strlen(p) + 1);
    for (int n = (int)strlen(s); n > 0 && isspace((unsigned char)s[n - 1]); --n) s[n - 1] = 0;
}
bool parseFloats(const char* v, float* out, int n) {
    const char* p = v;
    for (int i = 0; i < n; ++i) {
        char* e; out[i] = strtof(p, &e);
        if (e == p || !isfinite(out[i])) return false;
        p = e; while (*p && isspace((unsigned char)*p)) ++p;
        if (i < n - 1) { if (*p != ',') return false; ++p; }
    }
    while (*p && isspace((unsigned char)*p)) ++p;
    return *p == 0;
}

void loadIni() {
    Cfg c; cfgDefaults(&c);
    g_debug = GetFileAttributesA("inv3d_debug.txt") != INVALID_FILE_ATTRIBUTES;
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (GetFileAttributesExA("inv3d.ini", GetFileExInfoStandard, &fa))
        g_iniStamp = ((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    else g_iniStamp = 0;
    FILE* f = fopen("inv3d.ini", "r");
    if (!f) { logf("INI: inv3d.ini not found, using defaults (debug=%d)", g_debug); g_cfg = c; ++g_cfgGen; return; }
    char line[256]; int ln = 0, ok = 0, bad = 0;
    while (fgets(line, sizeof(line), f)) {
        ++ln; trim(line);
        if (!line[0] || line[0] == '#' || line[0] == ';') continue;
        char* eq = strchr(line, '=');
        if (!eq) { logf("INI line %d rejected (no '='): %s", ln, line); ++bad; continue; }
        *eq = 0; char* k = line; char* v = eq + 1;
        for (char* cm = v; *cm; ++cm) if (*cm == ';' || *cm == '#') { *cm = 0; break; }  // inline comment
        trim(k); trim(v);
        for (char* p = k; *p; ++p) *p = (char)tolower((unsigned char)*p);
        float a[6]; bool good = true;
        if (!strcmp(k, "enabled")) { good = parseFloats(v, a, 1); if (good) c.enabled = (int)a[0]; }
        else if (!strcmp(k, "rig")) { if (v[0] && strlen(v) < sizeof(c.rig)) strcpy(c.rig, v); else good = false; }
        else if (!strcmp(k, "only_weapons_armor")) { good = parseFloats(v, a, 1); if (good) c.onlyWeaponsArmor = (int)a[0]; }
        else if (!strcmp(k, "spin_deg_s")) { good = parseFloats(v, a, 1); if (good) c.spin = a[0]; }
        else if (!strcmp(k, "drag_deg_per_px")) { good = parseFloats(v, a, 1); if (good) c.dragDegPx = a[0]; }
        else if (!strcmp(k, "pitch_clamp")) { good = parseFloats(v, a, 1); if (good) c.pitchClamp = a[0]; }
        else if (!strcmp(k, "idle_resume_s")) { good = parseFloats(v, a, 1); if (good) c.idleResume = a[0]; }
        else if (!strcmp(k, "fit_mode")) { if (!_stricmp(v, "distance")) c.fitScale = 0; else if (!_stricmp(v, "scale")) c.fitScale = 1; else good = false; }
        else if (!strcmp(k, "fit_margin")) { good = parseFloats(v, a, 1); if (good) c.margin = a[0]; }
        else if (!strcmp(k, "dist_min")) { good = parseFloats(v, a, 1); if (good) c.distMin = a[0]; }
        else if (!strcmp(k, "dist_max")) { good = parseFloats(v, a, 1); if (good) c.distMax = a[0]; }
        else if (!strcmp(k, "fov_deg")) { good = parseFloats(v, a, 1); if (good) c.fov = a[0]; }
        else if (!strcmp(k, "fov_axis")) { if (!_stricmp(v, "v")) c.fovAxis = 'v'; else if (!_stricmp(v, "h")) c.fovAxis = 'h'; else good = false; }
        else if (!strcmp(k, "cam_pos")) { good = parseFloats(v, a, 3); if (good) memcpy(c.camPos, a, 12); }
        else if (!strcmp(k, "cam_fwd")) { good = parseFloats(v, a, 3); if (good) memcpy(c.camFwd, a, 12); }
        else if (!strcmp(k, "cam_up")) { good = parseFloats(v, a, 3); if (good) memcpy(c.camUp, a, 12); }
        else if (!strcmp(k, "armor_box")) { good = parseFloats(v, a, 6); if (good) memcpy(c.armorBox, a, 24); }
        else if (!strcmp(k, "armor_spin")) { good = parseFloats(v, a, 1); if (good) c.armorSpin = (int)a[0]; }
        else if (!strcmp(k, "hit_flip_y")) { good = parseFloats(v, a, 1); if (good) c.hitFlipY = (int)a[0]; }
        else if (!strcmp(k, "screen_w")) { good = parseFloats(v, a, 1); if (good) c.screenW = a[0]; }
        else if (!strcmp(k, "screen_h")) { good = parseFloats(v, a, 1); if (good) c.screenH = a[0]; }
        else if (!strncmp(k, "itype.", 6) || !strncmp(k, "row.", 4)) {
            bool isT = k[0] == 'i'; const char* num = k + (isT ? 6 : 4); char* e; long n = strtol(num, &e, 10);
            good = *num && !*e && n >= 0 && n < 256 && parseFloats(v, a, 4);
            if (good) { Ov& o = isT ? c.itype[n] : c.row[n]; o.set = true; memcpy(o.v, a, 16); }
        } else { logf("INI line %d rejected (unknown key '%s')", ln, k); ++bad; continue; }
        if (!good) { logf("INI line %d rejected (bad value for '%s': '%s')", ln, k, v); ++bad; }
        else { logf("INI %s = %s", k, v); ++ok; }
    }
    fclose(f);
    if (c.margin < 1.0f || c.margin > 3) { logf("INI fit_margin out of range, reset 1.15"); c.margin = 1.15f; }
    if (c.pitchClamp < 0) c.pitchClamp = 0;
    if (c.idleResume < 0) c.idleResume = 0;
    if (c.screenW < 100 || c.screenH < 100) { logf("INI screen_w/h invalid, reset 3840x1600"); c.screenW = 3840; c.screenH = 1600; }
    if (c.distMin < 0.05f) c.distMin = 0.05f;
    if (c.distMax < c.distMin) c.distMax = c.distMin;
    if (c.fov < 5 || c.fov > 120) { logf("INI fov_deg out of range, reset 22.726"); c.fov = 22.726f; }
    g_cfg = c; ++g_cfgGen;
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

// ---- fit table lookup (sorted by row, var) ----
const FitEntry* fitLookup(int row, int var) {
    int lo = 0, hi = kFitCount - 1; unsigned key = ((unsigned)row << 8) | (unsigned)var;
    while (lo <= hi) {
        int m = (lo + hi) / 2; unsigned k = ((unsigned)kFit[m].row << 8) | kFit[m].var;
        if (k == key) return &kFit[m];
        if (k < key) lo = m + 1; else hi = m - 1;
    }
    return nullptr;
}

struct Pres {
    bool valid; int row, var, cat; Q base; float c[3], e[3]; float defPitch;
    float yaw, pitch, lastYaw, lastPitch; float D, s; bool dirty;
    bool dragging; float lastX, lastY; DWORD idleUntil; unsigned cfgGen;
    const FitEntry* fit;
};
struct Slot {
    int panel; unsigned char* block; bool usable; bool rigFallback;
    int lastItem, lastBase, lastVar; unsigned seq;
    DWORD bindTick; DWORD lastBeat; Pres p;
};
Slot g_slots[4];
unsigned g_seq = 0;
bool g_guardLogged = false;

Slot* findSlot(int panel) {
    Slot* best = nullptr;
    for (auto& s : g_slots)
        if (s.seq && s.panel == panel && (!best || s.seq > best->seq)) best = &s;
    return best;
}
Slot* claimSlot(int panel) {
    Slot* pick = nullptr;
    for (auto& s : g_slots) if (s.seq && s.panel == panel) { pick = &s; break; }
    if (!pick) {
        pick = &g_slots[0];
        for (auto& s : g_slots) if (s.seq < pick->seq) pick = &s;
    }
    return pick;
}
int modelCount(unsigned char* view) {
    int n = -1;
    if (!readAt<int>(reinterpret_cast<int>(view), ModelCountOffset, &n)) return -1;
    return n;
}

// ---- Gob access with the vtable guard (Design 8) ----
void* getModel(unsigned char* view, int idx) {
    return reinterpret_cast<GetModelFn>(FnGetModel)(view + SceneOffset, idx);
}
bool gobGuard(void* gob, const char* why) {
    if (!gob || IsBadReadPtr(gob, 0x200)) { logf("GUARD(%s): gob %p unreadable", why, gob); return false; }
    void** vt = *reinterpret_cast<void***>(gob);
    if (IsBadReadPtr(vt, 0x10c)) { logf("GUARD(%s): vtable %p unreadable", why, vt); return false; }
    bool ok = true;
    for (auto& c : kGobSlots)
        if (reinterpret_cast<int>(vt[c.off / 4]) != c.addr) {
            ok = false;
            if (!g_guardLogged) logf("GUARD(%s): slot 0x%x (%s) = %08x, expected %08x", why, c.off, c.name, (int)vt[c.off / 4], c.addr);
        }
    if (!ok && !g_guardLogged) {
        g_guardLogged = true;
        logf("GUARD: gob vtable %p (expected %08x); first 0x44 slots:", vt, ExpectedGobVtable);
        dumpMem("gobvt", reinterpret_cast<int>(vt), 0x44);
    }
    return ok;
}
void gobGetPose(void* gob, float* pos, float* q) {
    void** vt = *reinterpret_cast<void***>(gob);
    pos[0] = pos[1] = pos[2] = q[1] = q[2] = q[3] = 0; q[0] = 1;
    reinterpret_cast<GetVecFn>(vt[0x64 / 4])(gob, pos);
    reinterpret_cast<GetVecFn>(vt[0x68 / 4])(gob, q);
}

// ---- presentation ----
void camBasis(float* right) { vcross(g_cfg.camFwd, g_cfg.camUp, right); }

void computeFit(Pres* p, float aspect) {
    // bbox after qBase: AABB of the 8 rotated corners
    float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
    for (int i = 0; i < 8; ++i) {
        float v[3] = {(i & 1 ? .5f : -.5f) * p->e[0], (i & 2 ? .5f : -.5f) * p->e[1], (i & 4 ? .5f : -.5f) * p->e[2]}, r[3];
        qrotate(p->base, v, r);
        for (int k = 0; k < 3; ++k) { if (r[k] < lo[k]) lo[k] = r[k]; if (r[k] > hi[k]) hi[k] = r[k]; }
    }
    float ex = hi[0] - lo[0], ey = hi[1] - lo[1], ez = hi[2] - lo[2];
    float hs, vs;
    if (p->defPitch == 0) { hs = sqrtf(ex * ex + ey * ey); vs = ez; }
    else { hs = vs = sqrtf(ex * ex + ey * ey + ez * ez); }
    if (aspect < 0.1f) aspect = 1;
    float t = tanf(g_cfg.fov * 3.14159265f / 360.0f);
    float tanV = g_cfg.fovAxis == 'v' ? t : t / aspect;
    float need = g_cfg.margin * fmaxf(vs / (2 * tanV), hs / (2 * tanV * aspect));
    float d0 = -(g_cfg.camPos[0] * g_cfg.camFwd[0] + g_cfg.camPos[1] * g_cfg.camFwd[1] + g_cfg.camPos[2] * g_cfg.camFwd[2]);
    p->s = 1;
    if (g_cfg.fitScale) {
        p->D = d0;
        if (need > 1e-4f) p->s = d0 / need;
        p->s = fminf(fmaxf(p->s, 0.01f), 100.0f);
    } else {
        p->D = need;
        if (p->D < g_cfg.distMin) p->D = g_cfg.distMin;
        if (p->D > g_cfg.distMax) { p->s = g_cfg.distMax / need; p->D = g_cfg.distMax; }
    }
    logf("  fit: ext'=[%.3f %.3f %.3f] h=%.3f v=%.3f aspect=%.3f tanV=%.4f need=%.3f -> D=%.3f s=%.3f (d0=%.3f mode=%s)",
         ex, ey, ez, hs, vs, aspect, tanV, need, p->D, p->s, d0, g_cfg.fitScale ? "scale" : "distance");
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
    void** vt = *reinterpret_cast<void***>(gob);
    float right[3]; camBasis(right);
    Q qy = qaxis(g_cfg.camUp, p.yaw), qp = qaxis(right, p.pitch);
    Q q = qnormalize(qmul(qp, qmul(qy, p.base)));
    float sc[3] = {p.c[0] * p.s, p.c[1] * p.s, p.c[2] * p.s}, rc[3];
    qrotate(q, sc, rc);
    float pos[3];
    for (int k = 0; k < 3; ++k) pos[k] = g_cfg.camPos[k] + g_cfg.camFwd[k] * p.D - rc[k];
    float cur = 0;
    if (readAt<float>(reinterpret_cast<int>(gob), GobScaleOff, &cur) && cur > 0.01f && cur < 100 && fabsf(cur - p.s) > 1e-6f &&
        !IsBadWritePtr(reinterpret_cast<char*>(gob) + GobScaleOff, 4)) {
        *reinterpret_cast<float*>(reinterpret_cast<char*>(gob) + GobScaleOff) = p.s;
        dlogf("apply(%s): scale %.4f -> %.4f", why, cur, p.s);
    }
    float o3[3], o4[4];
    reinterpret_cast<SetPosFn>(vt[0x5C / 4])(gob, o3, pos[0], pos[1], pos[2]);
    reinterpret_cast<SetOriFn>(vt[0x60 / 4])(gob, o4, q.w, q.x, q.y, q.z);
    p.lastYaw = p.yaw; p.lastPitch = p.pitch; p.dirty = false;
    static DWORD lastSpinLog = 0; DWORD nowT = GetTickCount();
    if (strcmp(why, "spin") || nowT - lastSpinLog >= 1000) { if (!strcmp(why, "spin")) lastSpinLog = nowT;
    dlogf("apply(%s): gob=%p yaw=%.1f pitch=%.1f q=(%.3f %.3f %.3f %.3f) pos=(%.3f %.3f %.3f)", why, gob, p.yaw, p.pitch, q.w, q.x, q.y, q.z, pos[0], pos[1], pos[2]); }
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
    void** vt = *reinterpret_cast<void***>(gob);
    logf("  gob vtable=%08x (expect %08x)", (int)vt, ExpectedGobVtable);
    void* par = reinterpret_cast<GetParentFn>(vt[0x108 / 4])(gob, -1);
    float pos[3], q[4]; gobGetPose(gob, pos, q);
    float sc = -1; readAt<float>(reinterpret_cast<int>(gob), GobScaleOff, &sc);
    logf("  parent=%p (expect 0) pos=(%.3f %.3f %.3f) quat=(%.3f %.3f %.3f %.3f) scale=%.3f", par, pos[0], pos[1], pos[2], q[0], q[1], q[2], q[3], sc);
    if (par) { logf("  item has an attached parent (%p): presentation skipped (rig hooks in play)", par); return; }
    void* g0 = getModel(view, 0);
    if (g0 && gobGuard(g0, "rig")) { float p0[3], q0[4]; gobGetPose(g0, p0, q0); logf("  rig gob=%p pos=(%.3f %.3f %.3f) quat=(%.3f %.3f %.3f %.3f)", g0, p0[0], p0[1], p0[2], q0[0], q0[1], q0[2], q0[3]); }
    int camv = 0, cam = 0; readAt<int>(reinterpret_cast<int>(view), CameraOffset, &cam); if (cam) readAt<int>(cam, 0, &camv);
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
    float o[4] = {0, 0, 0, 15}; const char* src = "fallback";
    if (row >= 0 && row < kItemRows) { memcpy(o, kDefaultOrient[row], 16); src = "table"; }
    int it = (row >= 0 && row < kItemRows) ? kItemType[row] : -1;
    if (it >= 0 && it < 256 && g_cfg.itype[it].set) { memcpy(o, g_cfg.itype[it].v, 16); src = "ini itype"; }
    if (row >= 0 && row < 256 && g_cfg.row[row].set) { memcpy(o, g_cfg.row[row].v, 16); src = "ini row"; }
    p.base = qbase(o[0], o[1], o[2]); p.defPitch = o[3];
    logf("  orient rx=%.0f ry=%.0f rz=%.0f pitch=%.0f (%s, itype=%d)", o[0], o[1], o[2], o[3], src, it);
    int ext[4] = {0}; for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(view), 4 + 4 * i, &ext[i]);
    float aspect = ext[3] > 0 ? (float)ext[2] / (float)ext[3] : 1.0f;
    computeFit(&p, aspect);
    p.yaw = 0; p.pitch = p.defPitch; p.valid = true; p.dirty = true;
    static int dumps = 0;
    if (dumps < 10) { ++dumps; dumpMem("gob", reinterpret_cast<int>(gob), 24); }
    applyTransform(s, "setup");
#endif
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
    return g_hwnd;
}
bool cursorEngine(float* x, float* y) {
    HWND h = gameWindow(); if (!h) return false;
    POINT pt; if (!GetCursorPos(&pt) || !ScreenToClient(h, &pt)) return false;
    RECT r; if (!GetClientRect(h, &r) || r.right < 1 || r.bottom < 1) return false;
    *x = pt.x * g_cfg.screenW / r.right; *y = pt.y * g_cfg.screenH / r.bottom;
    return true;
}
#endif

LARGE_INTEGER g_qpcFreq, g_qpcLast;

void tickDrag(Slot* s, float dt) {
#if INV3D_DRAG
    Pres& p = s->p;
    static bool wasDown = false; static int presses = 0;
    bool down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    float cx = 0, cy = 0; bool haveCur = cursorEngine(&cx, &cy);
    int ext[4] = {0}; for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(s->block), 4 + 4 * i, &ext[i]);
    float top = g_cfg.hitFlipY ? g_cfg.screenH - ext[1] - ext[3] : (float)ext[1];
    bool inside = haveCur && cx >= ext[0] && cx < ext[0] + ext[2] && cy >= top && cy < top + ext[3];
    bool fg = gameWindow() && GetForegroundWindow() == g_hwnd;
    if (down && !wasDown) {
        if (presses < 30) { ++presses; logf("DRAG: LMB press #%d cursor=(%.0f,%.0f) rect=[%d,%d,%d,%d] top=%.0f flip=%d inside=%d fg=%d", presses, cx, cy, ext[0], ext[1], ext[2], ext[3], top, g_cfg.hitFlipY, inside, fg); }
        if (inside && fg && p.valid) {
            p.dragging = true; p.lastX = cx; p.lastY = cy;
            logf("DRAG: start at (%.0f,%.0f) yaw=%.1f pitch=%.1f", cx, cy, p.yaw, p.pitch);
        }
    }
    if (p.dragging) {
        if (!down) {
            p.dragging = false; p.idleUntil = GetTickCount() + (DWORD)(g_cfg.idleResume * 1000);
            logf("DRAG: end yaw=%.1f pitch=%.1f, spin resumes in %.1fs", p.yaw, p.pitch, g_cfg.idleResume);
        } else if (haveCur) {
            float dx = cx - p.lastX, dy = cy - p.lastY;
            if (dx != 0 || dy != 0) {
                p.yaw += dx * g_cfg.dragDegPx; p.pitch += dy * g_cfg.dragDegPx;
                if (p.pitch > g_cfg.pitchClamp) p.pitch = g_cfg.pitchClamp;
                if (p.pitch < -g_cfg.pitchClamp) p.pitch = -g_cfg.pitchClamp;
                p.lastX = cx; p.lastY = cy; p.dirty = true;
                dlogf("DRAG: move d=(%.0f,%.0f) yaw=%.1f pitch=%.1f", dx, dy, p.yaw, p.pitch);
            }
        }
    }
    wasDown = down;
#endif
}

} // namespace

extern "C" void __cdecl BindInventoryView(int panel) {
    int ctlCount = -1; readAt<int>(panel, 0x28, &ctlCount);
    int gate = 0; readAt<int>(GateGlobal, 0, &gate);
    int stackLocal = 0;
    logf("H1 BindInventoryView panel=%08x controls=%d gate=%d stack=%p", panel, ctlCount, gate, &stackLocal);
    if (!panel) { logf("H1: null panel, abort"); return; }

    unsigned char* block = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(BlockSize));
    if (!block) { logf("H1: operator new failed"); return; }
    memset(block, 0, BlockSize);
    unsigned char* view = block;

    reinterpret_cast<Fast1>(FnControlCtor)(view);
    *reinterpret_cast<void**>(view) = reinterpret_cast<void*>(ViewVftable);
    reinterpret_cast<Fast1>(FnSceneCtor)(view + SceneOffset);

    char tag[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(tag, "3D_MODEL");
    reinterpret_cast<InitCtl>(FnInitControl)(reinterpret_cast<void*>(panel), view, tag, 1, 1);
    reinterpret_cast<CExoDtor>(FnExoDtor)(tag);

    int id = -999, newCount = -1; int ext[4] = {0};
    readAt<int>(reinterpret_cast<int>(view), 0x54, &id);
    for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(view), 4 + i * 4, &ext[i]);
    readAt<int>(panel, 0x28, &newCount);
    logf("H1: block=%p control id=%d (expect %d) extent=[%d,%d,%d,%d] aspect=%.3f controls now %d (expect %d)",
         block, id, ExpectedControlId, ext[0], ext[1], ext[2], ext[3], ext[3] ? (float)ext[2] / ext[3] : 0.f, newCount, ctlCount + 1);
    dumpMem("view", reinterpret_cast<int>(view), 38);

    Slot* s = claimSlot(panel);
    memset(s, 0, sizeof(*s));
    s->panel = panel; s->block = block; s->seq = ++g_seq; s->bindTick = GetTickCount();

    if (id != ExpectedControlId) { logf("H1: GUI control missing (id=%d), slot unusable", id); return; }
    if (!gate) { logf("H1: gate global 009f42b4 is zero, slot unusable"); return; }

    int scene = 0;
    if (!readAt<int>(reinterpret_cast<int>(view), SceneObjOffset, &scene) || !scene || IsBadReadPtr(reinterpret_cast<void*>(scene), 4)) {
        logf("H1: scene pointer bad (%08x), slot unusable", scene); return;
    }
    void** vt = *reinterpret_cast<void***>(scene);
    if (IsBadReadPtr(vt + 0x70 / 4, 4)) { logf("H1: scene vtable bad, slot unusable"); return; }
    float pos[3] = {0, 0, 0}, quat[4] = {0, 0, 0, 1};
    reinterpret_cast<SceneInit>(vt[0x70 / 4])(reinterpret_cast<void*>(scene), "gui3D_room", pos, quat);
    logf("H1: scene %08x initialised (gui3D_room)", scene);

    const char* rig = g_cfg.enabled ? g_cfg.rig : "upgitem_light";
    void* light = nullptr; int n = -1;
    for (int attempt = 0; attempt < 2; ++attempt) {
        char tag2[16] = {0};
        reinterpret_cast<CExoCtor>(FnExoCtor)(tag2, rig);
        light = reinterpret_cast<ThisCall2>(FnAddModel)(view + SceneOffset, tag2, -1);
        reinterpret_cast<CExoDtor>(FnExoDtor)(tag2);
        n = modelCount(view);
        logf("H1: rig '%s' model=%p, scene model count=%d", rig, light, n);
        if (light && n >= 1) break;
        if (attempt == 0 && strcmp(rig, "upgitem_light")) { logf("H1: rig '%s' failed, falling back to upgitem_light", rig); rig = "upgitem_light"; s->rigFallback = true; }
    }
    if (!light || n < 1) { logf("H1: light model missing, slot unusable"); return; }
    int cam = 0, camv = 0; readAt<int>(reinterpret_cast<int>(view), CameraOffset, &cam); if (cam) readAt<int>(cam, 0, &camv);
    logf("H1: camera=%08x vtable=%08x (expect %08x)", cam, camv, ExpectedCamVtable);
    s->usable = true;
    logf("H1: slot %d ready (panel=%08x seq=%u rigFallback=%d)", static_cast<int>(s - g_slots), panel, s->seq, s->rigFallback);
}

extern "C" void __cdecl ShowItem(int item, int panel) {
    static int repeatLogs = 0, detailLogs = 0;
    Slot* s = findSlot(panel);
    if (!s) { logf("H2: no slot for panel=%08x", panel); return; }
    if (!s->usable) { logf("H2: slot unusable (panel=%08x)", panel); return; }
    if (!item) { logf("H2: null item"); return; }
    unsigned char* view = s->block;

    int id = -1; unsigned char v0 = 0, v1 = 0;
    if (!readAt<int>(item, 0xc, &id) || !readAt<unsigned char>(item, ItemVarOff, &v0) || !readAt<unsigned char>(item, ItemVarOff + 2, &v1)) {
        logf("H2: item %08x unreadable", item); return;
    }
    int var = v0 | (v1 << 8);
    if (id < 0 || id >= kItemRows) { logf("H2: base item id %d out of range", id); return; }
    int cat = kItemCategory[id];
    if ((g_cfg.onlyWeaponsArmor) && !kIsWeaponOrArmor[id]) cat = 0;
    if (item == s->lastItem && id == s->lastBase && var == s->lastVar) {
        if (repeatLogs < 20) { ++repeatLogs; logf("H2: same item %08x base=%d var=%04x, skip", item, id, var); }
        return;
    }
    int n = modelCount(view);
    s->p.valid = false; s->p.dragging = false;
    if (cat == 0) {
        if (n >= 2) reinterpret_cast<Remove>(FnRemoveModel)(view + SceneOffset, 1, 1);
        s->lastItem = item; s->lastBase = id; s->lastVar = var;
        logf("H2: item %08x base=%d var=%04x cat=0 (no preview), models %d -> %d", item, id, var, n, modelCount(view));
        return;
    }
    if (n < 1) { logf("H2: light model gone (count=%d), slot unusable", n); s->usable = false; return; }

    // Design 4: missing/EMPTY variation -> substitute var 1 (item bytes restored on every path)
    int effVar = v0; bool substituted = false;
    int savedVarWord = 0; bool haveSaved = readAt<int>(item, ItemVarOff, &savedVarWord);
    const FitEntry* fe = (cat == 2) ? fitLookup(id, v0) : nullptr;
    auto subst = [&](const char* why) {
        if (!haveSaved || IsBadWritePtr(reinterpret_cast<void*>(item + ItemVarOff), 4)) { logf("H2: cannot substitute (%s): item bytes unwritable", why); return false; }
        *reinterpret_cast<unsigned char*>(item + ItemVarOff) = 1;
        effVar = 1; substituted = true; fe = fitLookup(id, 1);
        logf("H2: substituting var %d -> 1 (%s), item %08x", v0, why, item);
        return true;
    };
    if (cat == 2 && fe && (fe->flags & 1)) subst("EMPTY variation");  // a variation missing from the table is tried as-is; the retry below covers real misses

    auto doRender = [&]() {
        *(view + FakeCategoryBlockOff) = static_cast<unsigned char>(cat);
        *reinterpret_cast<int*>(view + FakeItemBlockOff) = item;
        reinterpret_cast<Render>(FnRender)(view - FakeThisDelta);
        *reinterpret_cast<int*>(view + FakeItemBlockOff) = 0;
    };
    doRender();
    int n2 = modelCount(view);
    if (cat == 2 && n2 < 2 && !substituted) {
        logf("H2: model count did not grow (%d), retrying with var 1", n2);
        if (subst("no model after render")) { doRender(); n2 = modelCount(view); }
    }
    if (haveSaved && substituted && !IsBadWritePtr(reinterpret_cast<void*>(item + ItemVarOff), 4))
        *reinterpret_cast<int*>(item + ItemVarOff) = savedVarWord;  // restore all 4 bytes
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
        logf("H3 beat: slot=%d row=%d yaw=%.1f pitch=%.1f D=%.3f s=%.3f drag=%d model=%p", (int)(s - g_slots), p.row, p.yaw, p.pitch, p.D, p.s, p.dragging, getModel(s->block, 1));
    }
#endif
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) { logf("inv3d v2 loaded"); loadIni(); }
    return TRUE;
}
