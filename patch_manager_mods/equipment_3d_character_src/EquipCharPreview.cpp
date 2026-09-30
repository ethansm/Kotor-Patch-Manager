// Equipment 3D Character 0.1.0-probe (Steam Aspyr build 6A522E71...). Shows a live 3D render of the character whose gear
// is being edited on the equipment screen (CSWGuiInGameEquip), with an occasional flourish animation.
// Research trail: patch_manager_mods/10_equipment_3d_character.md and equipment_3d_character_src/research/R* (worktree
// local-setup/KOTOR-II). Technique: Mod 1 (inventory_3d_viewport_src) for the view; the preview creature is built the way
// the character page does it (SetStats 0084e6f0, raw disasm 0084f2c6..0084f482).
//
// Hooks (cdecl detours, original bytes still run afterwards):
//   H-E1 0x008aa16a BindEquipView  panel ctor 008a92d0, before StopLoadFromLayout ([ebp-0x1d8] = panel)
//   H-E3 0x008aba50 EquipTick      panel Update (vtable slot 0x34) prologue, ECX = panel
//   H-E4 0x008aa6f0 EquipDestroy   panel scalar deleting dtor prologue, ECX = panel: frees the preview creature
//
// Stages (equip3d.ini `stage=N`, each includes the ones below; lets a crash be bisected without a rebuild):
//   1 view + rig + covers only          2 preview creature built when the shown character changes (no refresh)
//   3 polled refresh when the real creature's appearance changes    4 idle + flourish scheduler    5 drag / wheel / reset
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

namespace {

// ---- engine addresses (Steam Aspyr) ----
constexpr int GateGlobal = 0x009f42b4;
constexpr int FnControlCtor = 0x00418990;   // CSWGuiControl ctor, fastcall ECX = ctrl
constexpr int ViewVftable = 0x009a3d34;     // CSWGui3DSceneView
constexpr int FnSceneCtor = 0x00417f40;     // CSWGuiScene ctor, ECX = view+0x60
constexpr int FnInitControl = 0x0040f620;   // thiscall(panel, ctl, CExoString*, addToList, scale) RET 16
constexpr int FnExoCtor = 0x00733570;
constexpr int FnExoDtor = 0x00733780;
constexpr int FnNew = 0x00919723;           // cdecl(size)
constexpr int FnAddModel = 0x00418360;      // thiscall(view+0x60, CExoString*, int)
constexpr int FnGetModel = 0x004184b0;      // thiscall(view+0x60, int idx) -> Gob*
constexpr int FnLabelCtor = 0x00419740;     // CSWGuiLabel ctor, fastcall
constexpr int LabelSize = 0x148;
constexpr int LabelVftable = 0x009878BC;
constexpr int CtlFlagsOff = 0x48;           // bit1 visible, bit5 0x20 click-through
constexpr int CtlIdOff = 0x54;
constexpr int SceneOffset = 0x60;
constexpr int SceneObjOffset = 0x74;        // view+0x74 = Scene*
constexpr int CameraOffset = 0x78;
constexpr int ModelCountOffset = 0x80;
constexpr int BlockSize = 0x400;
constexpr int ExpectedControlId = 49;       // equip_p.gui has 49 controls (ids 0..48); 3D_MODEL=49, LBL_BEVEL=50, LBL_BEVEL2=51
constexpr int ExpectedCamVtable = 0x0098c45c;
constexpr int ExpectedGobVtable = 0x0098b5cc;

constexpr int PanelCharOff = 0x68;          // panel+0x68 = client creature POINTER of the shown character (SetCharacter arg)
constexpr int FnCreatureCtor = 0x007645f0;  // CSWCCreature ctor, fastcall ECX = block (0x470 bytes)
constexpr int CreatureSize = 0x470;
constexpr int CreatureVftable = 0x0099ee14;
constexpr int CreatureAppOff = 0x224;       // creature[0x89] = appearance object pointer (0x6c bytes, first 0x3c copied)
constexpr int CreatureStatsOff = 0x310;     // creature[0xc4] = stats; +0x80 short = alignment
constexpr int CreatureHolderOff = 0x68;     // creature[0x1a] = part holder: holder->vt[8](0xff) body Gob, (0xfe) head Gob
constexpr int FnBuildApp = 0x0076d200;      // thiscall(preview, copy60, flags, 1, 0) RET 0x10
constexpr int FnSetAlign = 0x0085d430;      // thiscall(preview[0x89], int align) RET 4
constexpr int FnSetupScene = 0x0077ec20;    // thiscall(preview, view, real, 0, 1) RET 0x10
constexpr int CamAttachSlot = 0x74, CamFovSlot = 0x44;
constexpr int CrSetScene = 0x94, CrSetPos = 0x8c, CrSetFacing = 0x88;
constexpr int PartPlaySlot = 0x18, PartLenSlot = 0x1c;
constexpr int AppCopyBytes = 0x3c;

using Fast1 = void(__fastcall*)(void*);
using ThisCall2 = void*(__thiscall*)(void*, void*, int);
using GetModelFn = void*(__thiscall*)(void*, int);
using InitCtl = void(__thiscall*)(void*, void*, void*, int, int);
using CExoCtor = void*(__thiscall*)(void*, const char*);
using CExoDtor = void(__thiscall*)(void*);
using SceneInit = void(__thiscall*)(void*, const char*, float*, float*);
using NewFn = void*(__cdecl*)(unsigned);
using CamAttach = void(__thiscall*)(void*, void*, const char*, int);
using CamFov = void(__thiscall*)(void*, float);
using BuildAppFn = int(__thiscall*)(void*, void*, int, int, int);
using SetAlignFn = void(__thiscall*)(void*, int);
using SetupSceneFn = void(__thiscall*)(void*, void*, void*, int, int);
using CrPtrFn = void(__thiscall*)(void*, void*);
using CrDtorFn = void(__thiscall*)(void*, int);
using PartGetFn = void*(__thiscall*)(void*, int);
using PartPlayFn = void(__thiscall*)(void*, const char*, float, unsigned, float);
using PartLenFn = void*(__thiscall*)(void*, const char*, int, float*);

// ---- diagnostics: equip3d_log.txt in the game dir (capped), equip3d_debug.txt enables per-tick detail ----
int g_logLines = 0;
constexpr int MaxLogLines = 15000;
bool g_debug = false;
int g_dbgLines = 0;
constexpr int MaxDebugLines = 8000;
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    FILE* f = fopen("equip3d_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    SYSTEMTIME st; GetLocalTime(&st);
    fprintf(f, "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}
void dlogf(const char* fmt, ...) {
    if (!g_debug || g_dbgLines >= MaxDebugLines) return;
    FILE* f = fopen("equip3d_log.txt", "a");
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

// ---- configuration: $GAME/equip3d.ini (missing/garbage = defaults; every key and rejected line logged) ----
struct Cfg {
    int enabled = 1, stage = 5;
    char rig[64] = "charrec_light";
    float fov = 22.726f;
    int bevels = 1;
    float dragDegPx = 0.5f; int wheelZoom = 1; float wheelStep = 0.9f, zoomMin = 0.6f, zoomMax = 2.5f;
    int hitFlipY = 0; float screenW = 3840, screenH = 1600;
    int flourish = 1; float flMin = 12, flMax = 25, flFirst = 6, flQuiet = 3; int flOnEquip = 1;
    char flList[512] = "shrug,nodyes,salute,bow,greeting,touchheart,point,rolleyes,scanning,victory,pausesh";
    char equipAnim[64] = "itemequip";
    int pollMs = 100, rebuildMinMs = 250;
    float spinDegS = 0;
};
Cfg g_cfg;
unsigned g_cfgGen = 0;
ULONGLONG g_iniStamp = 0; DWORD g_iniLastCheck = 0;

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
    Cfg c;
    g_debug = GetFileAttributesA("equip3d_debug.txt") != INVALID_FILE_ATTRIBUTES;
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (GetFileAttributesExA("equip3d.ini", GetFileExInfoStandard, &fa))
        g_iniStamp = ((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    else g_iniStamp = 0;
    FILE* f = fopen("equip3d.ini", "r");
    if (!f) { logf("INI: equip3d.ini not found, using defaults (stage=%d debug=%d)", c.stage, g_debug); g_cfg = c; ++g_cfgGen; return; }
    char line[600]; int ln = 0, ok = 0, bad = 0;
    while (fgets(line, sizeof(line), f)) {
        ++ln; trim(line);
        if (!line[0] || line[0] == '#' || line[0] == ';') continue;
        char* eq = strchr(line, '=');
        if (!eq) { logf("INI line %d rejected (no '='): %s", ln, line); ++bad; continue; }
        *eq = 0; char* k = line; char* v = eq + 1;
        for (char* cm = v; *cm; ++cm) if (*cm == ';' || *cm == '#') { *cm = 0; break; }
        trim(k); trim(v);
        for (char* p = k; *p; ++p) *p = (char)tolower((unsigned char)*p);
        float a[2]; bool good = true;
        if (!strcmp(k, "enabled")) { good = parseFloats(v, a, 1); if (good) c.enabled = (int)a[0]; }
        else if (!strcmp(k, "stage")) { good = parseFloats(v, a, 1); if (good) c.stage = (int)a[0]; }
        else if (!strcmp(k, "rig")) { if (v[0] && strlen(v) < sizeof(c.rig)) strcpy(c.rig, v); else good = false; }
        else if (!strcmp(k, "fov_deg")) { good = parseFloats(v, a, 1); if (good) c.fov = a[0]; }
        else if (!strcmp(k, "bevels")) { good = parseFloats(v, a, 1); if (good) c.bevels = (int)a[0]; }
        else if (!strcmp(k, "drag_deg_per_px")) { good = parseFloats(v, a, 1); if (good) c.dragDegPx = a[0]; }
        else if (!strcmp(k, "wheel_zoom")) { good = parseFloats(v, a, 1); if (good) c.wheelZoom = (int)a[0]; }
        else if (!strcmp(k, "wheel_step")) { good = parseFloats(v, a, 1); if (good) c.wheelStep = a[0]; }
        else if (!strcmp(k, "zoom_min")) { good = parseFloats(v, a, 1); if (good) c.zoomMin = a[0]; }
        else if (!strcmp(k, "zoom_max")) { good = parseFloats(v, a, 1); if (good) c.zoomMax = a[0]; }
        else if (!strcmp(k, "hit_flip_y")) { good = parseFloats(v, a, 1); if (good) c.hitFlipY = (int)a[0]; }
        else if (!strcmp(k, "screen_w")) { good = parseFloats(v, a, 1); if (good) c.screenW = a[0]; }
        else if (!strcmp(k, "screen_h")) { good = parseFloats(v, a, 1); if (good) c.screenH = a[0]; }
        else if (!strcmp(k, "flourish")) { good = parseFloats(v, a, 1); if (good) c.flourish = (int)a[0]; }
        else if (!strcmp(k, "flourish_min_s")) { good = parseFloats(v, a, 1); if (good) c.flMin = a[0]; }
        else if (!strcmp(k, "flourish_max_s")) { good = parseFloats(v, a, 1); if (good) c.flMax = a[0]; }
        else if (!strcmp(k, "flourish_first_s")) { good = parseFloats(v, a, 1); if (good) c.flFirst = a[0]; }
        else if (!strcmp(k, "flourish_quiet_s")) { good = parseFloats(v, a, 1); if (good) c.flQuiet = a[0]; }
        else if (!strcmp(k, "flourish_on_equip")) { good = parseFloats(v, a, 1); if (good) c.flOnEquip = (int)a[0]; }
        else if (!strcmp(k, "flourish_list")) { if (strlen(v) < sizeof(c.flList)) strcpy(c.flList, v); else good = false; }
        else if (!strcmp(k, "equip_anim")) { if (v[0] && strlen(v) < sizeof(c.equipAnim)) strcpy(c.equipAnim, v); else good = false; }
        else if (!strcmp(k, "poll_ms")) { good = parseFloats(v, a, 1); if (good) c.pollMs = (int)a[0]; }
        else if (!strcmp(k, "rebuild_min_ms")) { good = parseFloats(v, a, 1); if (good) c.rebuildMinMs = (int)a[0]; }
        else if (!strcmp(k, "spin_deg_s")) { good = parseFloats(v, a, 1); if (good) c.spinDegS = a[0]; }
        else { logf("INI line %d rejected (unknown key '%s')", ln, k); ++bad; continue; }
        if (!good) { logf("INI line %d rejected (bad value for '%s': '%s')", ln, k, v); ++bad; }
        else { logf("INI %s = %s", k, v); ++ok; }
    }
    fclose(f);
    if (c.stage < 0) c.stage = 0;
    if (c.stage > 5) c.stage = 5;
    if (c.fov < 5 || c.fov > 90) { logf("INI fov_deg out of range, reset 22.726"); c.fov = 22.726f; }
    if (c.wheelStep < 0.5f || c.wheelStep > 0.99f) { logf("INI wheel_step out of range, reset 0.9"); c.wheelStep = 0.9f; }
    if (c.zoomMin < 0.2f) c.zoomMin = 0.2f;
    if (c.zoomMax < c.zoomMin) c.zoomMax = c.zoomMin;
    if (c.flMin < 2) c.flMin = 2;
    if (c.flMax < c.flMin) c.flMax = c.flMin;
    if (c.pollMs < 16) c.pollMs = 16;
    if (c.screenW < 100 || c.screenH < 100) { c.screenW = 3840; c.screenH = 1600; }
    g_cfg = c; ++g_cfgGen;
    logf("INI loaded: %d keys ok, %d rejected, debug=%d gen=%u enabled=%d stage=%d rig=%s", ok, bad, g_debug, g_cfgGen, c.enabled, c.stage, c.rig);
}
void maybeReloadIni() {
    DWORD now = GetTickCount();
    if (now - g_iniLastCheck < 1000) return;
    g_iniLastCheck = now;
    WIN32_FILE_ATTRIBUTE_DATA fa; ULONGLONG st = 0;
    if (GetFileAttributesExA("equip3d.ini", GetFileExInfoStandard, &fa))
        st = ((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    bool dbg = GetFileAttributesA("equip3d_debug.txt") != INVALID_FILE_ATTRIBUTES;
    if (st != g_iniStamp || dbg != g_debug) { logf("INI changed on disk, reloading"); loadIni(); }
}

// ---- per-panel slot ----
struct Slot {
    int panel = 0;
    unsigned char* block = nullptr;   // the CSWGui3DSceneView block (= view)
    bool usable = false;
    unsigned seq = 0;
    DWORD bindTick = 0, lastBeat = 0;
    // preview creature
    unsigned char* creature = nullptr;
    bool creatureBad = false;         // construction failed: never retry this panel
    int real = 0;                     // the client creature the preview was built from
    unsigned char app[AppCopyBytes] = {};
    short align = -1;
    DWORD lastPoll = 0, lastBuild = 0;
    int builds = 0;
    bool built = false;
    // input
    float yaw = 0, zoom = 1;
    bool dragging = false; float lastX = 0, lastY = 0;
    // flourish scheduler
    DWORD nextFlourish = 0, flourishEnd = 0, lastEquipMs = 0;
    bool flourishActive = false;
    char idleName[16] = "";
    char activeName[32] = "";
};
constexpr int MaxSlots = 4;
Slot g_slots[MaxSlots];
unsigned g_seq = 0;

Slot* findSlot(int panel) {
    for (auto& s : g_slots) if (s.panel == panel && s.block) return &s;
    return nullptr;
}
void destroyCreature(Slot* s, const char* why);
Slot* claimSlot(int panel) {
    if (Slot* e = findSlot(panel)) { destroyCreature(e, "re-bind"); return e; }
    Slot* oldest = &g_slots[0];
    for (auto& s : g_slots) { if (!s.block) { oldest = &s; break; } if (s.seq < oldest->seq) oldest = &s; }
    if (oldest->block) { logf("slot: evicting panel=%08x seq=%u", oldest->panel, oldest->seq); destroyCreature(oldest, "evict"); }
    return oldest;
}

int modelCount(unsigned char* view) { int n = -1; readAt<int>(reinterpret_cast<int>(view), ModelCountOffset, &n); return n; }
void* getModel(unsigned char* view, int idx) {
    if (!view) return nullptr;
    return reinterpret_cast<GetModelFn>(FnGetModel)(view + SceneOffset, idx);
}

// ---- game window / wheel (copied from Mod 1) ----
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
WNDPROC g_oldProc = nullptr;
volatile LONG g_wheelAccum = 0;
volatile DWORD g_lastTickMs = 0;
volatile LONG g_viewRect[4] = {0, 0, 0, 0};
int g_wheelLogs = 0;
LRESULT CALLBACK wheelProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_MOUSEWHEEL && g_cfg.wheelZoom && GetTickCount() - g_lastTickMs < 300 && g_viewRect[2] > 0) {
        POINT pt = {(short)LOWORD(l), (short)HIWORD(l)}; RECT r;
        bool inside = false;
        if (ScreenToClient(h, &pt) && GetClientRect(h, &r) && r.right > 0 && r.bottom > 0) {
            float x = pt.x * g_cfg.screenW / r.right, y = pt.y * g_cfg.screenH / r.bottom;
            inside = x >= g_viewRect[0] && x < g_viewRect[0] + g_viewRect[2] && y >= g_viewRect[1] && y < g_viewRect[1] + g_viewRect[3];
        }
        if (g_wheelLogs < 20) { ++g_wheelLogs; logf("WHEEL: delta=%d cursor=(%ld,%ld) inside=%d", (int)(short)HIWORD(w), pt.x, pt.y, inside); }
        if (inside) { InterlockedExchangeAdd(&g_wheelAccum, (short)HIWORD(w)); return 0; }
    }
    return CallWindowProcA(g_oldProc, h, m, w, l);
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
    logf("INPUT: hwnd=%p client=%ldx%ld (engine %.0fx%.0f)", g_hwnd, r.right, r.bottom, g_cfg.screenW, g_cfg.screenH);
    if (g_hwnd && g_cfg.wheelZoom && !g_oldProc) {
        g_oldProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wheelProc)));
        logf("WHEEL: subclassed window %p, old proc %p (err=%lu)", g_hwnd, g_oldProc, g_oldProc ? 0UL : GetLastError());
    }
    return g_hwnd;
}
bool cursorEngine(float* x, float* y) {
    HWND h = gameWindow(); if (!h) return false;
    POINT pt; if (!GetCursorPos(&pt) || !ScreenToClient(h, &pt)) return false;
    RECT r; if (!GetClientRect(h, &r) || r.right < 1 || r.bottom < 1) return false;
    *x = pt.x * g_cfg.screenW / r.right; *y = pt.y * g_cfg.screenH / r.bottom;
    return true;
}
LARGE_INTEGER g_qpcFreq, g_qpcLast;

// ---- cover labels (character-page arcs); built here because only constructor-bound controls get created ----
void bindCover(int panel, const char* tagName, int expectId) {
    using LabelCtorFn = void(__fastcall*)(void*);
    unsigned char* ctl = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(LabelSize));
    if (!ctl) { logf("H1 cover %s: allocation failed", tagName); return; }
    memset(ctl, 0, LabelSize);
    reinterpret_cast<LabelCtorFn>(FnLabelCtor)(ctl);
    int vt = 0; readAt<int>(reinterpret_cast<int>(ctl), 0, &vt);
    if (vt != LabelVftable) { logf("H1 cover %s: ctor left vtable %08x (expected %08x), not binding", tagName, vt, LabelVftable); return; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(exo, tagName);
    reinterpret_cast<InitCtl>(FnInitControl)(reinterpret_cast<void*>(panel), ctl, exo, 1, 1);
    reinterpret_cast<CExoDtor>(FnExoDtor)(exo);
    int id = -999, fl = 0, ext[4] = {0};
    readAt<int>(reinterpret_cast<int>(ctl), CtlIdOff, &id); readAt<int>(reinterpret_cast<int>(ctl), CtlFlagsOff, &fl);
    for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(ctl), 4 + 4 * i, &ext[i]);
    if (id != expectId) fl &= ~2;
    *reinterpret_cast<int*>(ctl + CtlFlagsOff) = fl | 0x20;
    logf("H1 cover %s: ctl=%p id=%d (expect %d) extent=[%d,%d,%d,%d] flags=%08x -> %08x", tagName, ctl, id, expectId, ext[0], ext[1], ext[2], ext[3], fl, fl | 0x20);
}

// ---- preview creature ----
void** vtOf(void* obj) { void** vt = nullptr; readAt<void**>(reinterpret_cast<int>(obj), 0, &vt); return vt; }

void destroyCreature(Slot* s, const char* why) {
    if (!s->creature) return;
    void** vt = vtOf(s->creature);
    int vt0 = 0; readAt<int>(reinterpret_cast<int>(s->creature), 0, &vt0);
    logf("creature: destroy %p (%s) vtable=%08x", s->creature, why, vt0);
    if (vt0 == CreatureVftable && vt && !IsBadReadPtr(vt, 4)) reinterpret_cast<CrDtorFn>(vt[0])(s->creature, 1);
    else logf("creature: vtable mismatch, NOT calling dtor (leak instead of crash)");
    s->creature = nullptr; s->built = false; s->real = 0; s->flourishActive = false;
}

bool allocCreature(Slot* s) {
    if (s->creature) return true;
    if (s->creatureBad) return false;
    unsigned char* c = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(CreatureSize));
    if (!c) { logf("creature: allocation failed"); s->creatureBad = true; return false; }
    memset(c, 0, CreatureSize);
    reinterpret_cast<Fast1>(FnCreatureCtor)(c);
    int vt = 0; readAt<int>(reinterpret_cast<int>(c), 0, &vt);
    logf("creature: allocated %p vtable=%08x (expect %08x)", c, vt, CreatureVftable);
    if (vt != CreatureVftable) { logf("creature: ctor left wrong vtable, abandoning this panel"); dumpMem("creature", reinterpret_cast<int>(c), 24); s->creatureBad = true; return false; }
    int app = 0, holder = 0, stats = 0;
    readAt<int>(reinterpret_cast<int>(c), CreatureAppOff, &app); readAt<int>(reinterpret_cast<int>(c), CreatureHolderOff, &holder); readAt<int>(reinterpret_cast<int>(c), CreatureStatsOff, &stats);
    logf("creature: app=%08x holder=%08x stats=%08x", app, holder, stats);
    s->creature = c;
    return true;
}

int realValid(int real) {
    int vt = 0;
    if (!readAt<int>(real, 0, &vt) || vt != CreatureVftable) return 0;
    return 1;
}
bool readApp(int real, unsigned char* out, short* align) {
    int app = 0, stats = 0;
    if (!readAt<int>(real, CreatureAppOff, &app) || !app || IsBadReadPtr(reinterpret_cast<void*>(app), AppCopyBytes)) return false;
    memcpy(out, reinterpret_cast<void*>(app), AppCopyBytes);
    *align = 0;
    if (readAt<int>(real, CreatureStatsOff, &stats) && stats) readAt<short>(stats, 0x80, align);
    return true;
}

void* partGob(Slot* s, int id) {
    int holder = 0;
    if (!s->creature || !readAt<int>(reinterpret_cast<int>(s->creature), CreatureHolderOff, &holder) || !holder) return nullptr;
    void** hv = vtOf(reinterpret_cast<void*>(holder));
    if (!hv || IsBadReadPtr(hv + 2, 4)) return nullptr;
    return reinterpret_cast<PartGetFn>(hv[2])(reinterpret_cast<void*>(holder), id);
}
// Idle name as 0077ec20 picks it (evil <40, neutral <60, else good; droids: pause1) - verified by the anim length probe.
bool partLen(void* part, const char* name, float* len) {
    void** pv = vtOf(part);
    if (!pv || IsBadReadPtr(pv + PartLenSlot / 4, 4)) return false;
    float l = 0; reinterpret_cast<PartLenFn>(pv[PartLenSlot / 4])(part, name, 0, &l);
    *len = l;
    return l > 0.0f;   // -1.0 = missing animation
}
void playPart(void* part, const char* name, float speed, unsigned flags) {
    void** pv = vtOf(part);
    if (!pv || IsBadReadPtr(pv + PartPlaySlot / 4, 4)) return;
    reinterpret_cast<PartPlayFn>(pv[PartPlaySlot / 4])(part, name, speed, flags, 0.0f);
}
void chooseIdle(Slot* s) {
    void* body = partGob(s, 0xff);
    short al = s->align;
    const char* want = al < 40 ? "evil" : (al < 60 ? "neutral" : "good");
    float len = 0;
    if (body && partLen(body, want, &len)) strcpy(s->idleName, want);
    else if (body && partLen(body, "pause1", &len)) strcpy(s->idleName, "pause1");
    else strcpy(s->idleName, "");
    logf("anim: idle for align=%d -> '%s' (wanted '%s') body=%p", al, s->idleName, want, body);
}
void applyView(Slot* s) {
    // zoom = FOV scale on the scene camera; the facing yaw is re-applied after every rebuild
    int cam = 0; readAt<int>(reinterpret_cast<int>(s->block), CameraOffset, &cam);
    if (cam) {
        void** cv = vtOf(reinterpret_cast<void*>(cam));
        if (cv && !IsBadReadPtr(cv + CamFovSlot / 4, 4)) reinterpret_cast<CamFov>(cv[CamFovSlot / 4])(reinterpret_cast<void*>(cam), g_cfg.fov / s->zoom);
    }
    if (s->creature) {
        float rad = s->yaw * 0.0174532925f, v[3] = {cosf(rad), sinf(rad), 0.f};
        void** cv = vtOf(s->creature);
        if (cv) reinterpret_cast<CrPtrFn>(cv[CrSetFacing / 4])(s->creature, v);
    }
}

// Builds/rebuilds the preview from `real` exactly like SetStats (0084f2c6..0084f482).
bool buildPreview(Slot* s, int real, const char* why) {
    if (!allocCreature(s)) return false;
    unsigned char copy[AppCopyBytes]; short al = 0;
    if (!readApp(real, copy, &al)) { logf("build: real %08x appearance unreadable", real); return false; }
    *reinterpret_cast<int*>(copy + 0x10) = 0x7f000000;
    int gate = 0; readAt<int>(GateGlobal, 0, &gate);
    if (!gate) { logf("build: gate 009f42b4 is zero, skipping"); return false; }
    void* pv = s->creature; void** vt = vtOf(pv);
    int scene = 0; readAt<int>(reinterpret_cast<int>(s->block), SceneObjOffset, &scene);
    if (!vt || !scene) { logf("build: bad vtable %p / scene %08x", vt, scene); return false; }
    logf("build #%d (%s): real=%08x align=%d creature=%p scene=%08x", s->builds + 1, why, real, al, pv, scene);
    dumpMem("appcopy", reinterpret_cast<int>(copy), 15);
    int ok = reinterpret_cast<BuildAppFn>(FnBuildApp)(pv, copy, 3, 1, 0);
    logf("build: 0076d200 returned %d", ok);
    int pApp = 0; readAt<int>(reinterpret_cast<int>(pv), CreatureAppOff, &pApp);
    if (pApp) reinterpret_cast<SetAlignFn>(FnSetAlign)(reinterpret_cast<void*>(pApp), (int)(unsigned short)al);
    reinterpret_cast<CrPtrFn>(vt[CrSetScene / 4])(pv, reinterpret_cast<void*>(scene));
    float pos[3] = {0, 0, 0}, face[3] = {1, 0, 0};
    reinterpret_cast<CrPtrFn>(vt[CrSetPos / 4])(pv, pos);
    reinterpret_cast<CrPtrFn>(vt[CrSetFacing / 4])(pv, face);
    reinterpret_cast<SetupSceneFn>(FnSetupScene)(pv, s->block, reinterpret_cast<void*>(real), 0, 1);
    logf("build: scene models=%d body=%p head=%p", modelCount(s->block), partGob(s, 0xff), partGob(s, 0xfe));
    memcpy(s->app, copy, AppCopyBytes); s->align = al; s->real = real; s->built = true; ++s->builds; s->lastBuild = GetTickCount();
    s->flourishActive = false;
    chooseIdle(s);
    applyView(s);
    return true;
}

// ---- flourish scheduler ----
unsigned g_rng = 0x1234567;
unsigned rnd() { g_rng = g_rng * 1664525u + 1013904223u; return g_rng >> 8; }
float rndRange(float a, float b) { return a + (b - a) * ((rnd() & 0xffff) / 65535.0f); }

bool playFlourish(Slot* s, const char* name, const char* why) {
    void* body = partGob(s, 0xff); void* head = partGob(s, 0xfe);
    float len = 0;
    if (!body) { logf("flourish: no body part"); return false; }
    if (!partLen(body, name, &len)) { dlogf("flourish: '%s' missing on body (len=%.2f)", name, len); return false; }
    if (len > 12.0f) { logf("flourish: '%s' length %.2f too long, skipped", name, len); return false; }
    logf("flourish: play '%s' (%s) len=%.2fs body=%p head=%p idle='%s'", name, why, len, body, head, s->idleName);
    playPart(body, name, -1.0f, 0);
    if (head) playPart(head, name, -1.0f, 0);
    s->flourishActive = true; s->flourishEnd = GetTickCount() + (DWORD)(len * 1000.0f) + 40;
    strncpy(s->activeName, name, sizeof(s->activeName) - 1);
    return true;
}
void endFlourish(Slot* s) {
    void* body = partGob(s, 0xff); void* head = partGob(s, 0xfe);
    if (s->idleName[0]) {
        if (body) playPart(body, s->idleName, 1.0f, 2);
        if (head) playPart(head, s->idleName, 1.0f, 2);
    }
    logf("flourish: '%s' ended, idle '%s' re-issued (body=%p head=%p)", s->activeName, s->idleName, body, head);
    s->flourishActive = false;
    s->nextFlourish = GetTickCount() + (DWORD)(rndRange(g_cfg.flMin, g_cfg.flMax) * 1000.0f);
}
void pickFlourish(Slot* s) {
    char list[512]; strncpy(list, g_cfg.flList, sizeof(list) - 1); list[sizeof(list) - 1] = 0;
    char* names[32]; int n = 0;
    for (char* p = strtok(list, ","); p && n < 32; p = strtok(nullptr, ",")) { trim(p); if (p[0]) names[n++] = p; }
    if (!n) { s->nextFlourish = GetTickCount() + 10000; return; }
    int start = rnd() % n;
    for (int i = 0; i < n; ++i) if (playFlourish(s, names[(start + i) % n], "scheduled")) return;
    logf("flourish: none of %d listed names exist on this body, retry in 30 s", n);
    s->nextFlourish = GetTickCount() + 30000;
}

void tickInput(Slot* s) {
    static bool wasL = false, wasM = false;
    bool l = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0, mb = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    float cx = 0, cy = 0; bool haveCur = cursorEngine(&cx, &cy);
    int ext[4] = {0}; for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(s->block), 4 + 4 * i, &ext[i]);
    float top = g_cfg.hitFlipY ? g_cfg.screenH - ext[1] - ext[3] : (float)ext[1];
    g_viewRect[0] = ext[0]; g_viewRect[1] = (LONG)top; g_viewRect[2] = ext[2]; g_viewRect[3] = ext[3];
    bool inside = haveCur && cx >= ext[0] && cx < ext[0] + ext[2] && cy >= top && cy < top + ext[3];
    bool fg = gameWindow() && GetForegroundWindow() == g_hwnd;
    static int presses = 0;
    if (((l && !wasL) || (mb && !wasM)) && presses < 30) { ++presses; logf("INPUT: press #%d L=%d M=%d cursor=(%.0f,%.0f) rect=[%d,%d,%d,%d] top=%.0f inside=%d fg=%d", presses, l, mb, cx, cy, ext[0], ext[1], ext[2], ext[3], top, inside, fg); }
    bool changed = false;
    if (inside && fg && mb && !wasM) { s->yaw = 0; s->zoom = 1; changed = true; logf("INPUT: view reset (MMB)"); }
    if (inside && fg && l && !wasL) { s->dragging = true; s->lastX = cx; logf("INPUT: drag start at (%.0f,%.0f) yaw=%.1f", cx, cy, s->yaw); }
    if (s->dragging) {
        if (!l) { s->dragging = false; logf("INPUT: drag end yaw=%.1f", s->yaw); }
        else if (haveCur && cx != s->lastX) { s->yaw += (cx - s->lastX) * g_cfg.dragDegPx; s->lastX = cx; changed = true; dlogf("INPUT: yaw=%.1f", s->yaw); }
    }
    LONG wheel = InterlockedExchange(&g_wheelAccum, 0);
    if (wheel) {
        s->zoom *= powf(g_cfg.wheelStep, -wheel / 120.0f);   // wheel up (+120) = zoom in (fov smaller)
        s->zoom = fminf(fmaxf(s->zoom, g_cfg.zoomMin), g_cfg.zoomMax); changed = true;
        logf("WHEEL: applied %ld -> zoom=%.2f", wheel, s->zoom);
    }
    if (changed && s->built) applyView(s);
    wasL = l; wasM = mb;
}

} // namespace

extern "C" void __cdecl BindEquipView(int panel) {
    int ctlCount = -1; readAt<int>(panel, 0x28, &ctlCount);
    int gate = 0; readAt<int>(GateGlobal, 0, &gate);
    logf("H1 BindEquipView panel=%08x controls=%d gate=%d enabled=%d stage=%d", panel, ctlCount, gate, g_cfg.enabled, g_cfg.stage);
    if (!panel) { logf("H1: null panel, abort"); return; }
    if (!g_cfg.enabled || g_cfg.stage < 1) { logf("H1: disabled by INI (enabled=%d stage=%d)", g_cfg.enabled, g_cfg.stage); return; }

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
    readAt<int>(reinterpret_cast<int>(view), CtlIdOff, &id);
    for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(view), 4 + i * 4, &ext[i]);
    readAt<int>(panel, 0x28, &newCount);
    logf("H1: block=%p control id=%d (expect %d) extent=[%d,%d,%d,%d] aspect=%.3f controls now %d (expect %d)",
         block, id, ExpectedControlId, ext[0], ext[1], ext[2], ext[3], ext[3] ? (float)ext[2] / ext[3] : 0.f, newCount, ctlCount + 1);
    dumpMem("view", reinterpret_cast<int>(view), 38);

    Slot* s = claimSlot(panel);
    *s = Slot();
    s->panel = panel; s->block = block; s->seq = ++g_seq; s->bindTick = GetTickCount(); s->zoom = 1;

    if (id != ExpectedControlId) { logf("H1: GUI control missing/mismatched (id=%d), slot unusable (is equip_p.gui patched? scripts/add_equip_3dview_control.py)", id); return; }
    if (!gate) { logf("H1: gate global 009f42b4 is zero, slot unusable"); return; }
    int scene = 0;
    if (!readAt<int>(reinterpret_cast<int>(view), SceneObjOffset, &scene) || !scene || IsBadReadPtr(reinterpret_cast<void*>(scene), 4)) { logf("H1: scene pointer bad (%08x), slot unusable", scene); return; }
    void** svt = *reinterpret_cast<void***>(scene);
    if (IsBadReadPtr(svt + 0x70 / 4, 4)) { logf("H1: scene vtable bad, slot unusable"); return; }
    float pos[3] = {0, 0, 0}, quat[4] = {0, 0, 0, 1};
    reinterpret_cast<SceneInit>(svt[0x70 / 4])(reinterpret_cast<void*>(scene), "gui3D_room", pos, quat);
    logf("H1: scene %08x initialised (gui3D_room)", scene);

    char tag2[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(tag2, g_cfg.rig);
    void* rig = reinterpret_cast<ThisCall2>(FnAddModel)(view + SceneOffset, tag2, -1);
    reinterpret_cast<CExoDtor>(FnExoDtor)(tag2);
    int n = modelCount(view);
    logf("H1: rig '%s' model=%p, scene model count=%d", g_cfg.rig, rig, n);
    if (!rig || n < 1) { logf("H1: rig missing, slot unusable"); return; }
    int cam = 0, camv = 0; readAt<int>(reinterpret_cast<int>(view), CameraOffset, &cam); if (cam) readAt<int>(cam, 0, &camv);
    logf("H1: camera=%08x vtable=%08x (expect %08x)", cam, camv, ExpectedCamVtable);
    if (cam && camv == ExpectedCamVtable) {
        void** cv = *reinterpret_cast<void***>(cam);
        reinterpret_cast<CamAttach>(cv[CamAttachSlot / 4])(reinterpret_cast<void*>(cam), rig, "camerahook", 1);
        reinterpret_cast<CamFov>(cv[CamFovSlot / 4])(reinterpret_cast<void*>(cam), g_cfg.fov);
        logf("H1: camera attached to 'camerahook', fov=%.3f", g_cfg.fov);
    } else logf("H1: camera vtable mismatch, camera left as is");
    if (g_cfg.bevels) { bindCover(panel, "LBL_BEVEL", ExpectedControlId + 1); bindCover(panel, "LBL_BEVEL2", ExpectedControlId + 2); }
    s->usable = true;
    logf("H1: slot %d ready (panel=%08x seq=%u)", static_cast<int>(s - g_slots), panel, s->seq);
}

extern "C" void __cdecl EquipTick(int panel) {
    static int calls = 0;
    if (!g_qpcFreq.QuadPart) { QueryPerformanceFrequency(&g_qpcFreq); QueryPerformanceCounter(&g_qpcLast); }
    LARGE_INTEGER now; QueryPerformanceCounter(&now);
    float dt = (float)((double)(now.QuadPart - g_qpcLast.QuadPart) / (double)g_qpcFreq.QuadPart);
    g_qpcLast = now;
    g_lastTickMs = GetTickCount();
    if (dt > 0.5f) logf("H3: tick gap %.2fs (panel reopened or stalled)", dt);
    Slot* s = findSlot(panel);
    if (calls < 5) { ++calls; logf("H3 tick #%d panel=%08x slot=%s", calls, panel, s ? (s->usable ? "usable" : "unusable") : "none"); }
    if (!s || !s->usable) return;
    maybeReloadIni();
    if (!g_cfg.enabled || g_cfg.stage < 2) return;
    DWORD t = GetTickCount();
    if (modelCount(s->block) < 1) { static int w = 0; if (w++ < 5) logf("H3: rig gone (model count %d)", modelCount(s->block)); return; }

    int real = 0; readAt<int>(panel, PanelCharOff, &real);
    if (!real) { dlogf("H3: panel+0x68 is null"); return; }
    if (!realValid(real)) { static int w = 0; if (w++ < 10) { int vt = 0; readAt<int>(real, 0, &vt); logf("H3: real %08x has vtable %08x, expected %08x: skipping", real, vt, CreatureVftable); } return; }

    bool needBuild = false; const char* why = "";
    if (!s->built || real != s->real) { needBuild = true; why = s->built ? "character changed" : "first build"; }
    else if (g_cfg.stage >= 3 && t - s->lastPoll >= (DWORD)g_cfg.pollMs) {
        s->lastPoll = t;
        unsigned char cur[AppCopyBytes]; short al = 0;
        if (readApp(real, cur, &al) && (memcmp(cur, s->app, AppCopyBytes) != 0 || al != s->align)) {
            if (t - s->lastBuild >= (DWORD)g_cfg.rebuildMinMs) { needBuild = true; why = "appearance changed"; }
        }
    }
    if (needBuild && !s->creatureBad) {
        bool wasBuilt = s->built;
        if (buildPreview(s, real, why) && wasBuilt && !strcmp(why, "appearance changed")) {
            s->lastEquipMs = t;
            if (g_cfg.stage >= 4 && g_cfg.flourish && g_cfg.flOnEquip) {  // short "item equipped" gesture shortly after the swap
                if (!playFlourish(s, g_cfg.equipAnim, "after equip")) dlogf("flourish: equip anim '%s' unavailable", g_cfg.equipAnim);
            }
        }
        if (!s->nextFlourish) s->nextFlourish = t + (DWORD)(g_cfg.flFirst * 1000.0f);
    }
    if (!s->built) return;

    if (g_cfg.stage >= 4 && g_cfg.flourish) {
        if (!s->nextFlourish) s->nextFlourish = t + (DWORD)(g_cfg.flFirst * 1000.0f);
        if (s->flourishActive && t >= s->flourishEnd) endFlourish(s);
        else if (!s->flourishActive && t >= s->nextFlourish) {
            if (s->dragging || t - s->lastEquipMs < (DWORD)(g_cfg.flQuiet * 1000.0f)) s->nextFlourish = t + 1000;
            else pickFlourish(s);
        }
    }
    if (g_cfg.stage >= 5) tickInput(s);
    if (g_cfg.spinDegS != 0 && !s->dragging) { s->yaw += g_cfg.spinDegS * (dt > 0.1f ? 0.1f : dt); applyView(s); }
    if (t - s->bindTick < 90000 && t - s->lastBeat >= 2000) {
        s->lastBeat = t;
        logf("H3 beat: builds=%d built=%d real=%08x yaw=%.1f zoom=%.2f flourish=%d idle='%s' models=%d", s->builds, s->built, s->real, s->yaw, s->zoom, s->flourishActive, s->idleName, modelCount(s->block));
    }
}

extern "C" void __cdecl EquipDestroy(int panel) {
    Slot* s = findSlot(panel);
    logf("H4 EquipDestroy panel=%08x slot=%s creature=%p", panel, s ? "found" : "none", s ? s->creature : nullptr);
    if (!s) return;
    destroyCreature(s, "panel destroyed");
    s->usable = false; s->block = nullptr; s->panel = 0;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) { logf("equip3d 0.1.0-probe loaded"); loadIni(); g_rng ^= GetTickCount(); }
    return TRUE;
}
