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
#include "EquipWeaponTable.inc"   // baseitems row -> itemclass + weaponwield (scripts/gen_equip3d_weapon_table.py)

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
// toolbar button labels (ported from Mod 3): border fill swap for hover, text set for state
constexpr int FnLoadImage = 0x0047EB60, FnTextSet = 0x00416E30, BorderVtable = 0x009875BC, TextVtable = 0x009876B4;
constexpr int LabelBorderOff = 0x60, BorderFillOff = 0x74, LabelTextOff = 0xD8, TextRendererOff = 0x14, TextStringSubOff = 0x18;
// weapons (research: 0086a430 attaches model instances with Gob vt[0x50](parentGob, "rhand"/"lhand", 0); detach = vt[0x50](0,0,0)+vt[0x4c](0)+delete)
constexpr int FnGetItemObj = 0x0073f530, ItemMgrGlobal = 0x00a1b4a4, FnCreateModel = 0x00462320;   // thiscall(mgr=[[a1b4a4]+4], id) -> client item*; cdecl(name, inst, 0, 0) -> Gob*
constexpr int CrRightItemOff = 0x238, CrLeftItemOff = 0x23c, ItemBaseIdxOff = 0xc, ItemVariationOff = 0x11c, NoItemId = 0x7f000000;
constexpr int ItemModelResOff = 0x128;   // client item ctor 00801fa0 stores the model ResRef it chose (name formula 006d5000 + defaultmodel fallback), char[16]
constexpr int GobAttachSlot = 0x50, GobDetachSlot = 0x4c;
constexpr int NumButtons = 5;   // LBL_E3D_ANIM / SPIN / WEAP / RESET / EMOTE (ids ExpectedControlId+3..+7)
constexpr int MaxMenu = 20;     // LBL_E3D_M00..M19 (ids +8..+27): Emotes pop-up items; then LBL_E3D_TIPBG / LBL_E3D_TIP (+28,+29)
constexpr int MenuRows = 8;     // items per pop-up column
constexpr int CtlFlagsOff = 0x48;           // bit1 visible, bit5 0x20 click-through
constexpr int CtlIdOff = 0x54;
constexpr int SceneOffset = 0x60;
constexpr int SceneObjOffset = 0x74;        // view+0x74 = Scene*
constexpr int CameraOffset = 0x78;
constexpr int ModelCountOffset = 0x80;
constexpr int BlockSize = 0x400;
constexpr int ExpectedControlId = 49;       // equip_p.gui has 49 controls (ids 0..48); 3D_MODEL=49, LBL_BEVEL=50, LBL_BEVEL2=51
constexpr int TipBgId = ExpectedControlId + 3 + NumButtons + MaxMenu, TipId = TipBgId + 1;
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
    int hitFlipY = 0; float screenW = 3840, screenH = 1600; bool screenExplicit = false;
    int flourish = 1; float flMin = 12, flMax = 25, flFirst = 6, flQuiet = 3; int flOnEquip = 1;
    char flStance[512] = "";   // optional flourish list used while a weapon stance is on (empty = same list)
    char flList[512] = "shrug,nodyes,salute,bow,greeting,touchheart,point,rolleyes,scanning,victory,pausesh";
    char equipAnim[64] = "itemequip";
    int pollMs = 100, rebuildMinMs = 250, keepObj10 = 0, helmet = 1, buttons = 1; float spinToggleDegS = 20;
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
        else if (!strcmp(k, "screen_w")) { good = parseFloats(v, a, 1); if (good) { c.screenW = a[0]; c.screenExplicit = true; } }
        else if (!strcmp(k, "screen_h")) { good = parseFloats(v, a, 1); if (good) { c.screenH = a[0]; c.screenExplicit = true; } }
        else if (!strcmp(k, "flourish")) { good = parseFloats(v, a, 1); if (good) c.flourish = (int)a[0]; }
        else if (!strcmp(k, "flourish_min_s")) { good = parseFloats(v, a, 1); if (good) c.flMin = a[0]; }
        else if (!strcmp(k, "flourish_max_s")) { good = parseFloats(v, a, 1); if (good) c.flMax = a[0]; }
        else if (!strcmp(k, "flourish_first_s")) { good = parseFloats(v, a, 1); if (good) c.flFirst = a[0]; }
        else if (!strcmp(k, "flourish_quiet_s")) { good = parseFloats(v, a, 1); if (good) c.flQuiet = a[0]; }
        else if (!strcmp(k, "flourish_on_equip")) { good = parseFloats(v, a, 1); if (good) c.flOnEquip = (int)a[0]; }
        else if (!strcmp(k, "flourish_stance_list")) { if (strlen(v) < sizeof(c.flStance)) strcpy(c.flStance, v); else good = false; }
        else if (!strcmp(k, "flourish_list")) { if (strlen(v) < sizeof(c.flList)) strcpy(c.flList, v); else good = false; }
        else if (!strcmp(k, "equip_anim")) { if (v[0] && strlen(v) < sizeof(c.equipAnim)) strcpy(c.equipAnim, v); else good = false; }
        else if (!strcmp(k, "poll_ms")) { good = parseFloats(v, a, 1); if (good) c.pollMs = (int)a[0]; }
        else if (!strcmp(k, "buttons")) { good = parseFloats(v, a, 1); if (good) c.buttons = (int)a[0]; }
        else if (!strcmp(k, "spin_toggle_deg_s")) { good = parseFloats(v, a, 1); if (good) c.spinToggleDegS = a[0]; }
        else if (!strcmp(k, "keep_obj10")) { good = parseFloats(v, a, 1); if (good) c.keepObj10 = (int)a[0]; if (good && c.keepObj10) logf("WARNING: keep_obj10=1 is DEPRECATED and IGNORED (it reparented the REAL helmet Gob onto the preview: 0085a9e0, doc 10); use helmet=1"); }
        else if (!strcmp(k, "helmet")) { good = parseFloats(v, a, 1); if (good) c.helmet = (int)a[0]; }
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
    int builds = 0; int baseModels = -1;   // scene model count after the first build (growth check)
    bool built = false;
    // input
    float yaw = 0, zoom = 1;
    bool dragging = false; float lastX = 0, lastY = 0;
    // flourish scheduler
    DWORD nextFlourish = 0, flourishEnd = 0, lastEquipMs = 0;
    bool flourishActive = false;
    char idleName[16] = "";      // the CURRENT looping idle (stance while stanceOn, else the alignment idle)
    char baseIdle[16] = "";      // alignment idle
    char activeName[32] = "";
    // weapons
    void* wpn[2] = {}; int wpnIds[2] = {0, 0}; bool wpnApplied = false, stanceOn = false; char stanceName[12] = "", wpnNames[2][28] = {};
    // own-model helmet (never the real item's Gob: DB lesson 149, 0085a9e0)
    void* helm = nullptr; int helmId = 0; bool helmApplied = false; char helmName[20] = "";
    // toolbar
    unsigned char* btn[NumButtons] = {}; char btnText[NumButtons][20] = {}; int btnHover[NumButtons] = {-1, -1, -1, -1, -1}; bool btnWasL = false;
    // Emotes pop-up (items are engine labels; itemState = fill state + 1, 0 = not yet set)
    unsigned char* item[MaxMenu] = {}; char itemText[MaxMenu][20] = {}; int itemState[MaxMenu] = {};
    float rowRect[4] = {0, 0, 0, 0}; int spinLaid = -1;
    int menuPhase = 0; DWORD menuPhaseAt = 0; char pendAnim[12] = ""; bool pendAtk = false;   // phase: 0 shown, 1 hidden (pre-buffer), 2 playing, 3 hidden (post-buffer)
    bool menuOpen = false; int menuPage = 0; int menuCount = 0, menuLaid = -1, menuReal = 0; char menuAnim[MaxMenu][12] = {}; char menuLabel[MaxMenu][16] = {}; bool menuAttack[MaxMenu] = {};
    unsigned char* tipBg = nullptr; unsigned char* tip = nullptr; char tipText[200] = ""; int tipFor = -1, overBtn = -1; DWORD overSince = 0;
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
// Engine framebuffer size (Aspyr globals 0x009F42A4/0x009F42A8, same ones BuffDurationHUD reads): used unless the INI sets screen_w/screen_h explicitly.
void syncScreen() {
    if (g_cfg.screenExplicit) return;
    int w = 0, h = 0; readAt<int>(0x009F42A4, 0, &w); readAt<int>(0x009F42A8, 0, &h);
    if (w >= 320 && w <= 16384 && h >= 200 && h <= 16384) { g_cfg.screenW = (float)w; g_cfg.screenH = (float)h; }
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
            syncScreen(); float x = pt.x * g_cfg.screenW / r.right, y = pt.y * g_cfg.screenH / r.bottom;
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
    syncScreen(); *x = pt.x * g_cfg.screenW / r.right; *y = pt.y * g_cfg.screenH / r.bottom;
    return true;
}
LARGE_INTEGER g_qpcFreq, g_qpcLast;

// ---- cover labels (character-page arcs); built here because only constructor-bound controls get created ----
unsigned char* bindCover(int panel, const char* tagName, int expectId) {
    using LabelCtorFn = void(__fastcall*)(void*);
    unsigned char* ctl = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(LabelSize));
    if (!ctl) { logf("H1 cover %s: allocation failed", tagName); return nullptr; }
    memset(ctl, 0, LabelSize);
    reinterpret_cast<LabelCtorFn>(FnLabelCtor)(ctl);
    int vt = 0; readAt<int>(reinterpret_cast<int>(ctl), 0, &vt);
    if (vt != LabelVftable) { logf("H1 cover %s: ctor left vtable %08x (expected %08x), not binding", tagName, vt, LabelVftable); return nullptr; }
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
    return id == expectId ? ctl : nullptr;
}

// ---- toolbar (Anim / Spin / Reset): labels bound by tag; state text + hover fill set at runtime, clicks polled (labels are click-through) ----
bool g_animOn = true, g_spinOn = false, g_weaponsOn = false, g_spinUi = false;   // g_spinUi: the spin row shows play/pause + Reset instead of the single Spin button
   // session toggles (initialised from the INI at load / first bind)
struct ImgPoolEntry { char resref[17]; void* image; };
ImgPoolEntry g_imgPool[8]; int g_imgCount = 0;
void* btnImage(const char* resref) {
    for (int i = 0; i < g_imgCount; ++i) if (!strcmp(g_imgPool[i].resref, resref)) return g_imgPool[i].image;
    if (g_imgCount >= 8) return nullptr;
    char buf[20] = {}; strncpy(buf, resref, 16);
    void* im = reinterpret_cast<void*(__cdecl*)(const char*)>(FnLoadImage)(buf);
    ImgPoolEntry& e = g_imgPool[g_imgCount++]; memset(&e, 0, sizeof e); strncpy(e.resref, resref, 16); e.image = im;
    logf("btn: image '%s' -> %p", resref, im);
    return im;
}
void setLabelText(unsigned char* c, char* cache, size_t cacheN, const char* text, const char* nm) {
    if (!c || !strcmp(cache, text)) return;
    uintptr_t tx = reinterpret_cast<uintptr_t>(c) + LabelTextOff;
    int tvt = 0, rend = 0; readAt<int>(static_cast<int>(tx), 0, &tvt); readAt<int>(static_cast<int>(tx), TextRendererOff, &rend);
    if (tvt != TextVtable || !rend) { logf("%s: text object bad (vt=%08x renderer=%08x), cannot set \"%s\"", nm, tvt, rend, text); return; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(exo, text);
    reinterpret_cast<void(__thiscall*)(void*, void*)>(FnTextSet)(reinterpret_cast<void*>(tx + TextStringSubOff), exo);
    reinterpret_cast<CExoDtor>(FnExoDtor)(exo);
    dlogf("%s: text \"%s\" -> \"%s\"", nm, cache, text);
    strncpy(cache, text, cacheN - 1); cache[cacheN - 1] = 0;
}
void btnSetText(Slot* s, int i, const char* text) { char nm[12]; snprintf(nm, sizeof nm, "btn[%d]", i); setLabelText(s->btn[i], s->btnText[i], sizeof(s->btnText[i]), text, nm); }
void btnSetFill(Slot* s, int i, const char* resref) {
    unsigned char* c = s->btn[i]; if (!c) return;
    uintptr_t border = reinterpret_cast<uintptr_t>(c) + LabelBorderOff;
    int bvt = 0; readAt<int>(static_cast<int>(border), 0, &bvt);
    if (bvt != BorderVtable) { logf("btn[%d]: border vtable %08x != %08x, no fill change", i, bvt, BorderVtable); return; }
    void* im = btnImage(resref); if (!im) return;
    *reinterpret_cast<void**>(border + BorderFillOff) = im;
}
bool ctlRect(unsigned char* c, float* r) {   // engine-space rect from a label's extent (+4..+0x10, top-left origin)
    if (!c) return false;
    int e[4]; for (int k = 0; k < 4; ++k) if (!readAt<int>(reinterpret_cast<int>(c), 4 + 4 * k, &e[k])) return false;
    for (int k = 0; k < 4; ++k) r[k] = (float)e[k];
    return e[2] > 0 && e[3] > 0;
}
bool btnRect(Slot* s, int i, float* r) { return ctlRect(s->btn[i], r); }
bool overAnyButton(Slot* s, float cx, float cy) {
    for (int i = 0; i < NumButtons; ++i) { float r[4]; if (btnRect(s, i, r) && cx >= r[0] && cx < r[0] + r[2] && cy >= r[1] && cy < r[1] + r[3]) return true; }
    if (s->menuOpen && s->menuPhase == 0) for (int i = 0; i < s->menuCount; ++i) { float r[4]; if (ctlRect(s->item[i], r) && cx >= r[0] && cx < r[0] + r[2] && cy >= r[1] && cy < r[1] + r[3]) return true; }
    return false;
}
void applyView(Slot* s);
void endFlourish(Slot* s);
void** vtOfCtl(unsigned char* c) { void** vt = nullptr; readAt<void**>(reinterpret_cast<int>(c), 0, &vt); return vt; }
void setCtlVisible(unsigned char* c, bool vis) {
    int fl = 0; if (!c || !readAt<int>(reinterpret_cast<int>(c), CtlFlagsOff, &fl)) return;
    int nf = vis ? (fl | 2) : (fl & ~2);
    if (nf != fl) *reinterpret_cast<int*>(c + CtlFlagsOff) = nf;
}
void setCtlRect(unsigned char* c, int x, int y, int w, int h) {
    void** vt = vtOfCtl(c); if (!vt || IsBadReadPtr(vt + 1, 4)) return;
    int r[4] = {x, y, w, h}; reinterpret_cast<void(__thiscall*)(void*, int*)>(vt[1])(c, r);
}
bool playFlourish(Slot* s, const char* name, const char* why, float forceLen = 0.0f);
void menuBuild(Slot* s);
const char* const kTips[NumButtons + 1] = {
    "Animate\nPlays a gesture every few\nseconds. Click to turn on/off.",
    "Play / Pause\nSlowly turns the character;\npauses in place. Drag to rotate,\nwheel to zoom.",
    "Weapons\nShow equipped weapons in a\ncombat-ready stance. Click to toggle.",
    "Reset\nCentres the view again and goes\nback to the Spin button.",
    "Emotes\nPick a gesture (or, with\nWeapons on, an attack) to play.",
    "Spin\nSlowly turns the character.\nClick for play/pause and reset."};
constexpr int TipCharW = 21, TipLineH = 32, TipPad = 22, TipDelayMs = 350;   // px estimates (same font/metrics as Mod 3's tooltip)
void hideTip(Slot* s) { if (s->tipFor < 0) return; setCtlVisible(s->tipBg, false); setCtlVisible(s->tip, false); s->tipFor = -1; }
void showTip(Slot* s, int i, const float* br) {
    if (!s->tip || !s->tipBg) return;
    const char* text = kTips[(i == 1 && !g_spinUi) ? 5 : i];
    setLabelText(s->tip, s->tipText, sizeof s->tipText, text, "tip");
    int lines = 1, maxLen = 0, cur = 0;
    for (const char* q = text; *q; ++q) { if (*q == '\n') { ++lines; cur = 0; } else if (++cur > maxLen) maxLen = cur; }
    int w = maxLen * TipCharW + 2 * TipPad, h = lines * TipLineH + 2 * TipPad;
    int x = (int)br[0] - w - 12, y = (int)br[1]; if (x < 8) x = 8;
    if (y + h > (int)g_cfg.screenH - 8) y = (int)g_cfg.screenH - 8 - h;
    setCtlRect(s->tipBg, x, y, w, h); setCtlRect(s->tip, x + TipPad / 2, y + TipPad / 2, w - TipPad, h - TipPad);
    setCtlVisible(s->tipBg, true); setCtlVisible(s->tip, true);
    if (s->tipFor != i) logf("tip: show %d rect=[%d,%d,%d,%d] lines=%d maxLen=%d", i, x, y, w, h, lines, maxLen);
    s->tipFor = i;
}
constexpr DWORD MenuPreMs = 180, MenuPostMs = 300;   // buffers around an emote: menu hidden -> (pre) -> play -> finished -> (post) -> menu shown
void spinLayout(Slot* s) {   // single Spin button, or (after the first click) play/pause 30 % + Reset on the same row
    if (!s->rowRect[2] || !s->btn[1]) return;
    int x = (int)s->rowRect[0], y = (int)s->rowRect[1], W = (int)s->rowRect[2], H = (int)s->rowRect[3];
    if (!g_spinUi) { setCtlRect(s->btn[1], x, y, W, H); setCtlVisible(s->btn[3], false); }
    else { int w1 = (int)(W * 0.30f), gp = (int)(W * 0.04f); setCtlRect(s->btn[1], x, y, w1, H); setCtlRect(s->btn[3], x + w1 + gp, y, W - w1 - gp, H); setCtlVisible(s->btn[3], true); }
    s->btnHover[1] = s->btnHover[3] = -1; s->spinLaid = g_spinUi ? 1 : 0;
    logf("spin row: %s", g_spinUi ? "play/pause + Reset" : "Spin");
}
void menuClose(Slot* s, const char* why) {
    if (!s->menuOpen) return;
    s->menuOpen = false; s->menuLaid = -1; s->menuPhase = 0;
    for (int i = 0; i < MaxMenu; ++i) setCtlVisible(s->item[i], false);
    logf("menu: closed (%s)", why);
}
void menuHideItems(Slot* s) { for (int i = 0; i < MaxMenu; ++i) setCtlVisible(s->item[i], false); s->menuLaid = -1; }
void menuLayout(Slot* s) {   // anchored at the view's LEFT edge, below the top arc; column 0 ends with the page toggle at the bottom row
    float e[4], r0[4], r1[4];
    if (!btnRect(s, 4, e)) return;
    float w = e[2], h = e[3], pitch = (btnRect(s, 0, r0) && btnRect(s, 2, r1) && r1[1] - r0[1] > 0) ? (r1[1] - r0[1]) / 2 : h * 1.2f, gap = w * 0.08f;
    int vx = 0, vy = 0, vh = 0; readAt<int>(reinterpret_cast<int>(s->block), 4, &vx); readAt<int>(reinterpret_cast<int>(s->block), 8, &vy); readAt<int>(reinterpret_cast<int>(s->block), 16, &vh);
    float left = vx + w * 0.10f, top = vy + vh * 0.12f, bottom = vy + vh - vh * 0.13f;   // margins keep clear of the curved cut-outs
    if (vh > 0 && top + MenuRows * pitch > bottom) top = fmaxf((float)vy, bottom - MenuRows * pitch);
    int j = 0;   // running index over the non-toggle items
    for (int i = 0; i < MaxMenu; ++i) {
        if (i >= s->menuCount) { setCtlVisible(s->item[i], false); continue; }
        int col, row;
        if (!strcmp(s->menuAnim[i], "@page")) { col = 0; row = MenuRows - 1; }
        else { if (j < MenuRows - 1) { col = 0; row = j; } else { col = 1 + (j - (MenuRows - 1)) / MenuRows; row = (j - (MenuRows - 1)) % MenuRows; } ++j; }
        int x = (int)(left + col * (w + gap)), y = (int)(top + row * pitch);
        setCtlRect(s->item[i], x, y, (int)w, (int)h);
        setCtlVisible(s->item[i], true);
        char nm[12]; snprintf(nm, sizeof nm, "item[%d]", i); setLabelText(s->item[i], s->itemText[i], sizeof s->itemText[i], s->menuLabel[i], nm);
        s->itemState[i] = 0;
    }
    s->menuLaid = s->menuCount;
    logf("menu: layout %d items left=%.0f top=%.0f w=%.0f pitch=%.0f", s->menuCount, left, top, w, pitch);
}
void tickButtons(Slot* s) {
    bool any = false; for (int i = 0; i < NumButtons; ++i) any = any || s->btn[i];
    if (!any || !g_cfg.buttons) return;
    if (s->spinLaid != (g_spinUi ? 1 : 0)) spinLayout(s);
    btnSetText(s, 0, "Animate"); btnSetText(s, 1, !g_spinUi ? "Spin" : (g_spinOn ? "II" : ">")); btnSetText(s, 2, "Weapons"); btnSetText(s, 3, "Reset"); btnSetText(s, 4, "Emotes");   // state = fill colour (teal on, grey off, beige hover)
    float cx = 0, cy = 0; bool haveCur = cursorEngine(&cx, &cy);
    bool l = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0, fg = gameWindow() && GetForegroundWindow() == g_hwnd;
    bool edge = l && !s->btnWasL;
    DWORD t = GetTickCount();
    if (s->menuOpen && s->menuReal != s->real) menuClose(s, "character changed");
    menuBuild(s);
    if (s->menuOpen && s->menuPhase == 0 && s->menuCount != s->menuLaid) { if (s->menuCount) menuLayout(s); else menuClose(s, "no items"); }
    if (s->menuOpen && s->menuPhase) {   // hide -> pre buffer -> play -> (finished) -> post buffer -> show again
        if (s->menuPhase == 1 && t >= s->menuPhaseAt) {
            if (playFlourish(s, s->pendAnim, "menu", s->pendAtk ? 1.5f : 0.0f)) { s->menuPhase = 2; s->menuPhaseAt = t; }
            else { logf("menu: '%s' could not be played", s->pendAnim); s->menuPhase = 3; s->menuPhaseAt = t + MenuPostMs; }
        } else if (s->menuPhase == 2 && (!s->flourishActive || t - s->menuPhaseAt > 15000)) { s->menuPhase = 3; s->menuPhaseAt = t + MenuPostMs; }
        else if (s->menuPhase == 3 && t >= s->menuPhaseAt) { s->menuPhase = 0; menuLayout(s); logf("menu: shown again"); }
    }
    bool menuAvail = s->menuCount > 0;
    int overIdx = -1; float overRect[4] = {0, 0, 0, 0}; bool overEmote = false;
    for (int i = 0; i < NumButtons; ++i) {
        if (i == 3 && !g_spinUi) continue;   // Reset exists only after Spin was clicked
        float r[4]; int over = haveCur && fg && btnRect(s, i, r) && cx >= r[0] && cx < r[0] + r[2] && cy >= r[1] && cy < r[1] + r[3];
        bool on = i == 0 ? g_animOn : (i == 1 ? g_spinUi : (i == 2 ? g_weaponsOn : (i == 3 ? true : menuAvail)));   // play/pause and Reset never go grey
        bool inert = i == 4 && !on;   // greyed: no hover, no click
        if (over) { overIdx = i; memcpy(overRect, r, sizeof r); }
        if (i == 4 && over) overEmote = true;
        int st = (over && !inert) ? 2 : (on ? 1 : 0);   // hover / on / off
        if (st != s->btnHover[i]) { s->btnHover[i] = st; btnSetFill(s, i, st == 2 ? "e3d_btn_hi" : (st ? "e3d_btn" : "e3d_btn_off")); dlogf("btn[%d]: state=%d", i, st); }
        if (over && edge) {
            logf("btn[%d]: click at (%.0f,%.0f)%s", i, cx, cy, inert ? " (inert, ignored)" : "");
            if (inert) continue;
            s->btnHover[i] = -1;
            if (i == 0) { g_animOn = !g_animOn; if (!g_animOn && s->flourishActive) endFlourish(s); logf("btn: animation flourishes %s", g_animOn ? "ON" : "OFF"); }
            else if (i == 1) { if (!g_spinUi) { g_spinUi = true; g_spinOn = true; logf("btn: spin started (row split)"); } else { g_spinOn = !g_spinOn; logf("btn: auto-spin %s at yaw=%.1f", g_spinOn ? "ON" : "paused", s->yaw); } }
            else if (i == 3) { logf("btn: view reset (yaw=%.1f zoom=%.2f spin=%d)", s->yaw, s->zoom, g_spinOn); s->yaw = 0; s->zoom = 1; g_spinOn = false; g_spinUi = false; applyView(s); }
            else if (i == 2) { g_weaponsOn = !g_weaponsOn; logf("btn: weapons %s", g_weaponsOn ? "ON" : "OFF"); }
            else { if (s->menuOpen) menuClose(s, "Emotes again"); else { s->menuOpen = true; s->menuPage = 0; s->menuPhase = 0; s->menuReal = s->real; menuBuild(s); menuLayout(s); logf("menu: opened (%d items)", s->menuCount); } }
        }
    }
    bool overItem = false;
    if (s->menuOpen && s->menuPhase == 0) {
        for (int i = 0; i < s->menuCount; ++i) {
            float r[4]; int over = haveCur && fg && ctlRect(s->item[i], r) && cx >= r[0] && cx < r[0] + r[2] && cy >= r[1] && cy < r[1] + r[3];
            int st = over ? 2 : 1; overItem = overItem || over;
            if (s->itemState[i] != st + 1) { s->itemState[i] = st + 1; unsigned char* c = s->item[i]; uintptr_t border = reinterpret_cast<uintptr_t>(c) + LabelBorderOff; int bvt = 0; readAt<int>(static_cast<int>(border), 0, &bvt);
                if (bvt == BorderVtable) if (void* im = btnImage(st == 2 ? "e3d_btn_hi" : "e3d_btn")) *reinterpret_cast<void**>(border + BorderFillOff) = im; }
            if (over && edge) {
                logf("menu: item[%d] '%s' (%s) click", i, s->menuAnim[i], s->menuAttack[i] ? "attack" : "emote");
                if (!strcmp(s->menuAnim[i], "@page")) { s->menuPage ^= 1; menuBuild(s); menuLayout(s); logf("menu: page %d", s->menuPage); break; }
                strncpy(s->pendAnim, s->menuAnim[i], sizeof s->pendAnim - 1); s->pendAnim[sizeof s->pendAnim - 1] = 0; s->pendAtk = s->menuAttack[i];
                menuHideItems(s); s->menuPhase = 1; s->menuPhaseAt = t + MenuPreMs; logf("menu: hidden, '%s' plays in %u ms", s->pendAnim, (unsigned)MenuPreMs);
                break;
            }
        }
        if (edge && !overItem && !overEmote) { bool onBtn = overIdx >= 0; menuClose(s, onBtn ? "other button" : "click outside"); }
    }
    if (overItem || (s->menuOpen && overEmote)) { overIdx = -1; }
    if (overIdx != s->overBtn) { s->overBtn = overIdx; s->overSince = t; hideTip(s); }
    else if (overIdx >= 0 && t - s->overSince >= (DWORD)TipDelayMs) showTip(s, overIdx, overRect);
    s->btnWasL = l;
}

// ---- preview creature ----
void** vtOf(void* obj) { void** vt = nullptr; readAt<void**>(reinterpret_cast<int>(obj), 0, &vt); return vt; }

// ---- weapons ----
using AttachFn = void(__thiscall*)(void*, void*, const char*, int);
void detachWeapons(Slot* s, const char* why) {
    for (int i = 0; i < 2; ++i) {
        void* g = s->wpn[i]; if (!g) continue;
        s->wpn[i] = nullptr;
        void** vt = vtOf(g); int vt0 = 0; readAt<int>(reinterpret_cast<int>(g), 0, &vt0);
        if (!vt || vt0 != ExpectedGobVtable || IsBadReadPtr(vt + 0x50 / 4, 4)) { logf("weapon[%d]: detach %p (%s): vtable %08x unexpected, NOT freeing (leak instead of crash)", i, g, why, vt0); continue; }
        reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, nullptr, nullptr, 0);
        reinterpret_cast<void(__thiscall*)(void*, int)>(vt[GobDetachSlot / 4])(g, 0);
        reinterpret_cast<void(__thiscall*)(void*, int)>(vt[0])(g, 1);
        logf("weapon[%d]: detached+freed %p '%s' (%s)", i, g, s->wpnNames[i], why);
    }
    s->wpnNames[0][0] = s->wpnNames[1][0] = 0;
}
int getItemObj(int id) {
    int mgr = 0, tbl = 0; if (!readAt<int>(ItemMgrGlobal, 0, &mgr) || !mgr || !readAt<int>(mgr, 4, &tbl) || !tbl) return 0;
    return reinterpret_cast<int(__thiscall*)(int, int)>(FnGetItemObj)(tbl, id);
}
void detachHelmet(Slot* s, const char* why) {
    void* g = s->helm; if (!g) return;
    s->helm = nullptr;
    void** vt = vtOf(g); int vt0 = 0; readAt<int>(reinterpret_cast<int>(g), 0, &vt0);
    if (!vt || vt0 != ExpectedGobVtable || IsBadReadPtr(vt + GobAttachSlot / 4, 4)) { logf("helmet: WARNING detach %p (%s): vtable %08x unexpected, NOT freeing (leak instead of crash)", g, why, vt0); s->helmName[0] = 0; return; }
    reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, nullptr, nullptr, 0);
    reinterpret_cast<void(__thiscall*)(void*, int)>(vt[GobDetachSlot / 4])(g, 0);
    reinterpret_cast<void(__thiscall*)(void*, int)>(vt[0])(g, 1);
    logf("helmet: detached+freed %p '%s' (%s)", g, s->helmName, why);
    s->helmName[0] = 0;
}
void* partGob(Slot* s, int id);
// Head item on the preview from OUR OWN model instance. The engine path (0085a9e0, R9) reparents the REAL item's Gob onto the
// preview head ("GoggleHook") and the preview dtor then frees it with the helmet attached: the real helmet vanishes / dangles.
void attachHelmet(Slot* s, int id, const char* why) {
    s->helmApplied = true; s->helmId = id;
    if (id == NoItemId || id == 0) { dlogf("helmet: none (id=%08x, %s)", id, why); return; }
    int item = getItemObj(id);
    if (!item) { logf("helmet: WARNING id=%08x -> no client item object (%s), skipped", id, why); return; }
    char name[17] = {};
    if (IsBadReadPtr(reinterpret_cast<void*>(item + ItemModelResOff), 16)) { logf("helmet: WARNING item %08x model ResRef unreadable, skipped", item); return; }
    memcpy(name, reinterpret_cast<void*>(item + ItemModelResOff), 16); name[16] = 0;
    bool okName = name[0] != 0;
    for (char* p = name; *p; ++p) if (!isalnum((unsigned char)*p) && *p != '_') okName = false;
    int base = -1; readAt<int>(item, ItemBaseIdxOff, &base); unsigned char var = 0; readAt<unsigned char>(item, ItemVariationOff, &var);
    if (!okName || !_stricmp(name, "i_null")) { logf("helmet: id=%08x item=%08x base=%d var=%d model='%s': no visible model, skipped", id, item, base, var, okName ? name : "?"); return; }
    void* head = partGob(s, 0xfe);
    if (!head) { logf("helmet: WARNING no head part on the preview (%s), skipped", why); return; }
    void* g = reinterpret_cast<void*(__cdecl*)(const char*, const char*, int, int)>(FnCreateModel)(name, reinterpret_cast<const char*>(0x009a4ab0), 0, 0);
    int gvt = 0; if (g) readAt<int>(reinterpret_cast<int>(g), 0, &gvt);
    if (!g) { logf("helmet: WARNING model '%s' failed to load (id=%08x), skipped", name, id); return; }
    if (gvt != ExpectedGobVtable) { logf("helmet: WARNING model '%s' gob=%p vtable %08x (expect %08x), not attaching (leak instead of crash)", name, g, gvt, ExpectedGobVtable); return; }
    void** vt = vtOf(g);
    reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, head, "GoggleHook", 0);   // same node + call as 0085a9e0; missing node = silently unattached (00461670)
    reinterpret_cast<PartPlayFn>(vt[PartPlaySlot / 4])(g, "default", 1.0f, 0, 0.0f);   // as 0085a9e0 does (animated visors)
    s->helm = g; strncpy(s->helmName, name, sizeof(s->helmName) - 1);
    logf("helmet: item id=%08x obj=%08x base=%d var=%d model='%s' gob=%p head=%p attach ok (%s)", id, item, base, var, name, g, head, why);
}
void destroyCreature(Slot* s, const char* why) {
    detachWeapons(s, why);
    detachHelmet(s, why);
    if (!s->creature) return;
    void** vt = vtOf(s->creature);
    int vt0 = 0; readAt<int>(reinterpret_cast<int>(s->creature), 0, &vt0);
    logf("creature: destroy %p (%s) vtable=%08x", s->creature, why, vt0);
    if (vt0 == CreatureVftable && vt && !IsBadReadPtr(vt, 4)) reinterpret_cast<CrDtorFn>(vt[0])(s->creature, 1);
    else logf("creature: vtable mismatch, NOT calling dtor (leak instead of crash)");
    s->creature = nullptr; s->built = false; s->real = 0; s->flourishActive = false; s->baseModels = -1;
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
// Static animation knowledge (R5 inventory): the vt[0x1c] length probe cannot see supermodel-inherited anims, so it is NOT used.
struct FlLen { const char* name; float len; };
const FlLen kHumanoid[] = {{"pausesh",2.0f},{"scanning",5.1f},{"salute",1.7f},{"victory",1.3f},{"bow",2.5f},{"greeting",1.5f},
    {"taunt",1.4f},{"touchheart",2.8f},{"rolleyes",2.3f},{"shrug",1.6f},{"nodyes",1.5f},{"point",2.0f},{"itemequip",1.3f},{"equip",1.0f}};
const FlLen kDroidT3[] = {{"victory",1.0f},{"greeting",1.0f},{"taunt",1.0f},{"equip",1.3f}};
const FlLen kDroidG0[] = {{"victory",2.0f},{"greeting",1.3f},{"taunt",1.3f},{"equip",1.3f}};
int appRow(Slot* s) { return (int)*reinterpret_cast<unsigned short*>(s->app + 0x18); }
bool isDroidRow(int r) { return r == 2 || r == 57 || r == 231 || r == 232 || r == 453 || r == 3 || r == 538 || r == 539; }
const FlLen* animTable(int row, int* n) {
    if (row == 2 || row == 57 || row == 231 || row == 232) { *n = 4; return kDroidT3; }
    if (row == 453) { *n = 4; return kDroidG0; }
    if (row == 3 || row == 538 || row == 539) { *n = 0; return nullptr; }
    *n = (int)(sizeof(kHumanoid) / sizeof(kHumanoid[0])); return kHumanoid;
}
bool animLen(Slot* s, const char* name, float* len) {
    int n; const FlLen* t = animTable(appRow(s), &n);
    for (int i = 0; i < n; ++i) if (!strcmp(t[i].name, name)) { *len = t[i].len; return true; }
    return false;
}
const char* labelFor(const char* n) {
    static const char* const m[][2] = {{"pausesh","Pause"},{"scanning","Scan"},{"salute","Salute"},{"victory","Victory"},{"bow","Bow"},{"greeting","Greet"},
        {"taunt","Taunt"},{"touchheart","Touch heart"},{"rolleyes","Roll eyes"},{"shrug","Shrug"},{"nodyes","Nod"},{"point","Point"},{"itemequip","Equip item"},{"equip","Equip"}};
    for (auto& e : m) if (!strcmp(n, e[0])) return e[1];
    return n;
}
// Attack clips per wield style (R5 chains: melee g<N>a<k>, ranged b<N>a<k>; all ~1.5 s). Shown only while Weapons is on and a stance exists.
void menuBuild(Slot* s) {   // page 0 = gestures (+ "Attacks >" when a weapon stance exists), page 1 = attacks (+ "< Emotes")
    int n = 0, en = 0; const FlLen* t = animTable(appRow(s), &en);
    auto add = [&](const char* a, const char* label, bool atk) { if (n < MaxMenu) { strncpy(s->menuAnim[n], a, 11); s->menuAnim[n][11] = 0; strncpy(s->menuLabel[n], label, 15); s->menuLabel[n][15] = 0; s->menuAttack[n] = atk; ++n; } };
    int N = (g_weaponsOn && s->stanceOn && s->stanceName[0] == 'g' && s->stanceName[1] >= '1' && s->stanceName[1] <= '8') ? s->stanceName[1] - '0' : 0;
    bool droid = isDroidRow(appRow(s));
    int cnt = 0;
    if (N && !(droid && !(N == 5 || N == 6 || N == 8))) cnt = N == 1 || N == 8 ? 2 : (N <= 4 ? 5 : 4);
    if (!cnt) s->menuPage = 0;
    if (s->menuPage == 0) {
        for (int i = 0; i < en; ++i) add(t[i].name, labelFor(t[i].name), false);
        if (cnt) add("@page", "Attacks >", false);
    } else {
        char nm[12], lb[16];
        for (int k = 1; k <= cnt; ++k) { snprintf(nm, sizeof nm, "%c%da%d", (N >= 5 && N <= 7) ? 'b' : 'g', N, k); snprintf(lb, sizeof lb, "Attack %d", k); add(nm, lb, true); }
        add("@page", "< Emotes", false);
    }
    s->menuCount = n;
}
void chooseIdle(Slot* s) {
    void* body = partGob(s, 0xff);
    short al = s->align; int row = appRow(s);
    const char* want = isDroidRow(row) ? "pause1" : (al < 40 ? "evil" : (al < 60 ? "neutral" : "good"));
    strcpy(s->baseIdle, want); strcpy(s->idleName, want);
    logf("anim: idle for align=%d row=%d -> '%s' body=%p", al, row, s->idleName, body);
}
// Engine wield-style rule (R7f 0076d590 / R7h 00863650). Only the weapons we actually attached count.
const char* const kStyleAnim[10] = {"", "g1r1", "g2r1", "g3r1", "g4r1", "g5r1", "g6r1", "g7r1", "g8r1", "g7r1"};
int styleFor(int rWield, int lWield, bool rHas, bool lHas) {
    if (rHas && lHas) return lWield == 2 ? 4 : (lWield == 4 ? 6 : 0);
    if (rHas) { static const int map[7] = {0, 1, 2, 3, 5, 7, 9}; return (rWield >= 1 && rWield <= 6) ? map[rWield] : 0; }
    return 0;   // left only, or nothing (unarmed style 8 deliberately not applied: keep the normal idle)
}
void attachWeapons(Slot* s, int real, const char* why) {
    s->stanceName[0] = 0;
    void* body = partGob(s, 0xff);
    if (!body) { logf("weapons: no body part, cannot attach (%s)", why); return; }
    int wield[2] = {0, 0}, baseOf[2] = {-1, -1}; bool has[2] = {false, false};
    for (int i = 0; i < 2; ++i) {
        int id = NoItemId; readAt<int>(real, i ? CrLeftItemOff : CrRightItemOff, &id); s->wpnIds[i] = id;
        const char* hook = i ? "lhand" : "rhand";
        if (id == NoItemId || id == 0) { logf("weapons: %s empty (id=%08x)", hook, id); continue; }
        int item = getItemObj(id);
        if (!item) { logf("weapons: %s id=%08x -> no client item object", hook, id); continue; }
        int base = -1, var = 0; readAt<int>(item, ItemBaseIdxOff, &base); { unsigned char vb = 0; readAt<unsigned char>(item, ItemVariationOff, &vb); var = vb; }
        if (base < 0 || base >= kWeaponRows || !kWeapon[base].wield) { logf("weapons: %s id=%08x item=%08x base=%d var=%d: not a wieldable weapon (rows=%d), skipped", hook, id, item, base, var, kWeaponRows); continue; }
        char name[28]; snprintf(name, sizeof name, "%s_%03d", kWeapon[base].cls, var);
        void* g = reinterpret_cast<void*(__cdecl*)(const char*, const char*, int, int)>(FnCreateModel)(name, reinterpret_cast<const char*>(0x009a4ab0), 0, 0);
        int gvt = 0; if (g) readAt<int>(reinterpret_cast<int>(g), 0, &gvt);
        logf("weapons: %s id=%08x item=%08x base=%d var=%d wield=%d model='%s' -> gob=%p vtable=%08x (expect %08x)", hook, id, item, base, var, kWeapon[base].wield, name, g, gvt, ExpectedGobVtable);
        if (!g) continue;
        if (gvt != ExpectedGobVtable) { logf("weapons: model gob vtable mismatch, not attaching (leak instead of crash)"); continue; }
        void** vt = vtOf(g);
        reinterpret_cast<AttachFn>(vt[GobAttachSlot / 4])(g, body, hook, 0);
        s->wpn[i] = g; strncpy(s->wpnNames[i], name, sizeof(s->wpnNames[i]) - 1);
        has[i] = true; wield[i] = kWeapon[base].wield; baseOf[i] = base;
        logf("weapons: attached %p to body %p node %s", g, body, hook);
    }
    int style = styleFor(wield[0], wield[1], has[0], has[1]);
    if (style > 0) snprintf(s->stanceName, sizeof s->stanceName, "%s", kStyleAnim[style]);
    logf("weapons: style=%d anim=%s R(w=%d,base=%d) L(w=%d,base=%d)", style, s->stanceName[0] ? s->stanceName : "-", wield[0], baseOf[0], wield[1], baseOf[1]);
    logf("weapons: (%s) stance='%s' L/R ids %08x/%08x", why, s->stanceName, s->wpnIds[1], s->wpnIds[0]);
}
void refreshWeapons(Slot* s, int real, const char* why) {
    void* body = partGob(s, 0xff); void* head = partGob(s, 0xfe);
    bool wasStance = s->stanceOn;
    detachWeapons(s, why);
    s->stanceName[0] = 0;
    if (g_weaponsOn) attachWeapons(s, real, why);
    s->wpnApplied = g_weaponsOn;
    readAt<int>(real, CrRightItemOff, &s->wpnIds[0]); readAt<int>(real, CrLeftItemOff, &s->wpnIds[1]);
    s->stanceOn = g_weaponsOn && s->stanceName[0];
    strcpy(s->idleName, s->stanceOn ? s->stanceName : s->baseIdle);
    const char* anim = (s->stanceOn || wasStance) ? s->idleName : nullptr;
    if (anim && anim[0]) {
        s->flourishActive = false;
        if (body) playPart(body, anim, 1.0f, 0);   // flags 0 = cross-fade in over the anim header transition (flags&2 would snap: R8a/R8b)
        if (head) playPart(head, anim, 1.0f, 0);
        logf("weapons: idle -> '%s' (stance=%d) body=%p head=%p", anim, s->stanceOn, body, head);
    }
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
    detachWeapons(s, "before rebuild");   // our weapon models hang off the old body Gob: free them first
    detachHelmet(s, "before rebuild");    // likewise the helmet on the old head Gob
    s->helmApplied = false;               // a failed rebuild below leaves the old head: the tick toggle block re-attaches to it
    s->stanceOn = false; s->wpnApplied = false;
    if (!allocCreature(s)) return false;
    unsigned char copy[AppCopyBytes]; short al = 0;
    if (!readApp(real, copy, &al)) { logf("build: real %08x appearance unreadable", real); return false; }
    unsigned char raw[AppCopyBytes]; memcpy(raw, copy, AppCopyBytes);  // snapshot BEFORE the +0x10 override (else compare never matches)
    logf("build: raw +0x10=%08x", *reinterpret_cast<int*>(copy + 0x10));
    // ALWAYS blank the head item, as SetStats does: with a real id 0085a9e0 reparents the REAL item's Gob onto our head and our dtor
    // then takes it down (headgear-loss bug, doc 10). keep_obj10 is ignored; attachHelmet shows the helmet from our own model.
    int headId = *reinterpret_cast<int*>(copy + 0x10);
    *reinterpret_cast<int*>(copy + 0x10) = NoItemId;
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
    { int mc = modelCount(s->block); if (s->baseModels < 0) s->baseModels = mc;
      logf("build: scene models=%d (baseline %d)%s body=%p head=%p", mc, s->baseModels, mc > s->baseModels ? " WARNING: scene models GREW across rebuilds" : "", partGob(s, 0xff), partGob(s, 0xfe)); }
    memcpy(s->app, raw, AppCopyBytes); s->align = al; s->real = real; s->built = true; ++s->builds; s->lastBuild = GetTickCount();
    s->flourishActive = false;
    chooseIdle(s);
    applyView(s);
    if (g_weaponsOn) refreshWeapons(s, real, "after build");
    s->helmApplied = false;
    if (g_cfg.helmet) attachHelmet(s, headId, "after build");
    return true;
}

// ---- flourish scheduler ----
unsigned g_rng = 0x1234567;
unsigned rnd() { g_rng = g_rng * 1664525u + 1013904223u; return g_rng >> 8; }
float rndRange(float a, float b) { return a + (b - a) * ((rnd() & 0xffff) / 65535.0f); }

bool playFlourish(Slot* s, const char* name, const char* why, float forceLen) {
    void* body = partGob(s, 0xff); void* head = partGob(s, 0xfe);
    float len = 0;
    if (!body) { logf("flourish: no body part"); return false; }
    if (forceLen > 0) len = forceLen;
    else if (!animLen(s, name, &len)) { dlogf("flourish: '%s' not in table for row %d", name, appRow(s)); return false; }
    if (len > 12.0f) { logf("flourish: '%s' length %.2f too long, skipped", name, len); return false; }
    logf("flourish: play '%s' (%s) len=%.2fs body=%p head=%p idle='%s'", name, why, len, body, head, s->idleName);
    // R8a/R8b: speed<0 starts at the END and plays BACKWARDS (Gob update: time += dt*speed); the engine's "once" is flag bit 1, and a
    // one-shot fades to nothing. So play forward with flags 0 (weight ramps 0->1 over the header transition, blending from the current pose)
    // and hand back to the idle ourselves just before the loop would wrap.
    playPart(body, name, 1.0f, 0);
    if (head) playPart(head, name, 1.0f, 0);
    s->flourishActive = true; s->flourishEnd = GetTickCount() + (DWORD)((len > 0.4f ? len - 0.06f : len) * 1000.0f);
    strncpy(s->activeName, name, sizeof(s->activeName) - 1);
    return true;
}
void endFlourish(Slot* s) {
    void* body = partGob(s, 0xff); void* head = partGob(s, 0xfe);
    if (s->idleName[0]) {
        if (body) playPart(body, s->idleName, 1.0f, 0);   // blend back from the flourish pose
        if (head) playPart(head, s->idleName, 1.0f, 0);
    }
    logf("flourish: '%s' ended, idle '%s' re-issued (body=%p head=%p)", s->activeName, s->idleName, body, head);
    s->flourishActive = false;
    s->nextFlourish = GetTickCount() + (DWORD)(rndRange(g_cfg.flMin, g_cfg.flMax) * 1000.0f);
}
void pickFlourish(Slot* s) {
    char list[512]; strncpy(list, (s->stanceOn && g_cfg.flStance[0]) ? g_cfg.flStance : g_cfg.flList, sizeof(list) - 1); list[sizeof(list) - 1] = 0;
    char* names[32]; int n = 0;
    for (char* p = strtok(list, ","); p && n < 32; p = strtok(nullptr, ",")) { trim(p); if (p[0]) names[n++] = p; }
    if (!n) { s->nextFlourish = GetTickCount() + 10000; return; }
    int start = rnd() % n;
    for (int i = 0; i < n; ++i) if (playFlourish(s, names[(start + i) % n], "scheduled")) return;
    logf("flourish: none of %d listed names apply to this appearance, retry in 30 s", n);
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
    if (inside && fg && l && !wasL && !overAnyButton(s, cx, cy)) { s->dragging = true; s->lastX = cx; logf("INPUT: drag start at (%.0f,%.0f) yaw=%.1f", cx, cy, s->yaw); }
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
    if (g_cfg.buttons) {
        static const char* const tags[NumButtons] = {"LBL_E3D_ANIM", "LBL_E3D_SPIN", "LBL_E3D_WEAP", "LBL_E3D_RESET", "LBL_E3D_EMOTE"};
        for (int i = 0; i < NumButtons; ++i) { s->btn[i] = bindCover(panel, tags[i], ExpectedControlId + 3 + i); if (!s->btn[i]) logf("H1: toolbar button %s not bound (GUI not patched?)", tags[i]); }
        for (int i = 0; i < MaxMenu; ++i) {
            char tg[16]; snprintf(tg, sizeof tg, "LBL_E3D_M%02d", i);
            s->item[i] = bindCover(panel, tg, ExpectedControlId + 3 + NumButtons + i); if (!s->item[i]) logf("H1: menu label %s not bound (GUI not patched?)", tg);
            setCtlVisible(s->item[i], false);
        }
        { float r3[4]; if (ctlRect(s->btn[3], r3) && s->btn[4]) setCtlRect(s->btn[4], (int)r3[0], (int)r3[1], (int)r3[2], (int)r3[3]); }   // Emotes takes the Reset row
        if (ctlRect(s->btn[1], s->rowRect)) logf("H1: spin row rect [%.0f,%.0f,%.0f,%.0f]", s->rowRect[0], s->rowRect[1], s->rowRect[2], s->rowRect[3]);
        setCtlVisible(s->btn[3], false);   // Reset appears only once Spin was clicked (spinLayout)
        s->tipBg = bindCover(panel, "LBL_E3D_TIPBG", TipBgId); s->tip = bindCover(panel, "LBL_E3D_TIP", TipId);
        setCtlVisible(s->tipBg, false); setCtlVisible(s->tip, false);   // shown only while a button is hovered
    }
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
            if (g_cfg.stage >= 4 && g_cfg.flourish && g_animOn && g_cfg.flOnEquip) {  // short "item equipped" gesture shortly after the swap
                if (!playFlourish(s, g_cfg.equipAnim, "after equip")) dlogf("flourish: equip anim '%s' unavailable", g_cfg.equipAnim);
            }
        }
        if (!s->nextFlourish) s->nextFlourish = t + (DWORD)(g_cfg.flFirst * 1000.0f);
    }
    if (!s->built) return;

    if (g_cfg.stage >= 4) tickButtons(s);
    {   // weapons: toggle or the real creature's hand items changed
        int ids[2] = {NoItemId, NoItemId}; readAt<int>(real, CrRightItemOff, &ids[0]); readAt<int>(real, CrLeftItemOff, &ids[1]);
        if (s->wpnApplied != g_weaponsOn || (g_weaponsOn && (ids[0] != s->wpnIds[0] || ids[1] != s->wpnIds[1]))) {
            logf("weapons: refresh (toggle=%d applied=%d ids %08x/%08x -> %08x/%08x)", g_weaponsOn, s->wpnApplied, s->wpnIds[1], s->wpnIds[0], ids[1], ids[0]);
            refreshWeapons(s, real, "toggle/items");
        }
    }
    if (s->helmApplied != (g_cfg.helmet != 0)) {   // helmet= toggled by INI hot reload (an item change rebuilds via the appearance compare)
        detachHelmet(s, "ini toggle");
        int hid = NoItemId; readAt<int>(reinterpret_cast<int>(s->app), 0x10, &hid);
        if (g_cfg.helmet) attachHelmet(s, hid, "ini toggle"); else s->helmApplied = false;
    }
    if (g_cfg.stage >= 4 && s->flourishActive && t >= s->flourishEnd) endFlourish(s);   // also for menu-triggered gestures while Animate is off
    if (g_cfg.stage >= 4 && g_cfg.flourish && g_animOn) {
        if (!s->nextFlourish) s->nextFlourish = t + (DWORD)(g_cfg.flFirst * 1000.0f);
        if (s->flourishActive && t >= s->flourishEnd) endFlourish(s);
        else if (!s->flourishActive && t >= s->nextFlourish) {
            if (s->dragging || s->menuOpen || t - s->lastEquipMs < (DWORD)(g_cfg.flQuiet * 1000.0f)) s->nextFlourish = t + 1000;
            else pickFlourish(s);
        }
    }
    if (g_cfg.stage >= 5) tickInput(s);
    float spin = g_spinOn ? (g_cfg.spinDegS != 0 ? g_cfg.spinDegS : g_cfg.spinToggleDegS) : g_cfg.spinDegS;
    if (spin != 0 && !s->dragging) { s->yaw += spin * (dt > 0.1f ? 0.1f : dt); applyView(s); }
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
    if (reason == DLL_PROCESS_DETACH && g_oldProc && g_hwnd && IsWindow(g_hwnd) &&
        reinterpret_cast<WNDPROC>(GetWindowLongPtrA(g_hwnd, GWLP_WNDPROC)) == wheelProc) {
        SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_oldProc)); g_oldProc = nullptr;   // never leave a dangling window proc
    }
    if (reason == DLL_PROCESS_ATTACH) { logf("equip3d 0.1.0-probe loaded"); loadIni(); g_rng ^= GetTickCount(); }
    return TRUE;
}
