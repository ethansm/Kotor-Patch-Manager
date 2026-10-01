// Character Active Effects Panel 0.1.0-probe (Steam Aspyr build 6A522E71...). Fills the right third of the character page
// (CSWGuiInGameCharacter) with the shown character's active effects: icon, name, remaining time; Harmful section first,
// then Beneficial; expiring first, permanent last; whole-entry scrolling with a scrollbar only when the list overflows.
// Research trail: patch_manager_mods/14_character_active_effects_panel.md and active_effects_panel_src/research/R* (worktree
// local-setup/KOTOR-II). Layout numbers = scripts/character_page_mockup.py; controls = scripts/mirror_character_page.py.
//
// Hooks (cdecl detours, original bytes still run afterwards):
//   H-C1 0x0084d871 BindCharEffects    panel ctor 0084c3a0, before StopLoadFromLayout ([ebp-0x254] = panel)
//   H-C3 0x0084fbb0 CharEffectsTick    panel Update (vtable 0x9a3e7c slot 0x34) prologue, ECX = panel
//   H-C4 0x0084def0 CharEffectsDestroy panel scalar deleting dtor prologue, ECX = panel
//
// Data (R2b/R2c): leader = 0x7e5da0(party [[client+4]+0x270], 0) (what SetStats shows), server creature = 0x77d800(client),
// effects = server +0x148/+0x14c. EffectIcon effects (type 0x43, ints[0] = effecticon.2da row) carry the engine's own
// good/bad flag, name and icon; other effects fall back to the Buff HUD spell tables. chareffects.ini `stage`:
//   0 frame + header + summary text, log-only effect dump      1 full rows, sections, scrolling, scrollbar
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include "EffectIconTable.inc"   // Buff HUD: beneficial spells.2da row -> icon
#include "DebuffIconTable.inc"   // Buff HUD: harmful spells.2da row -> icon (+ item flag)
#include "EffectNameTable.inc"   // scripts/gen_effect_name_table.py: spells/effecticon names + icons, font widths
#include "../_shared/SafeRead.h"  // VirtualQuery-based probing: IsBadReadPtr faults are a storm under Wine (DB lesson 234)

namespace {

// ---- engine addresses (Steam Aspyr) ----
constexpr int FnInitControl = 0x0040f620;   // thiscall(panel, ctl, CExoString*, addToList, scale) RET 16
constexpr int FnExoCtor = 0x00733570, FnExoDtor = 0x00733780;
constexpr int FnNew = 0x00919723;           // cdecl(size)
constexpr int FnLabelCtor = 0x00419740, LabelSize = 0x148, LabelVftable = 0x009878BC;
constexpr int FnLoadImage = 0x0047EB60;     // cdecl(resref buf) -> image
constexpr int FnTextSet = 0x00416E30;       // thiscall(text+0x18, CExoString*)
constexpr int FnTextColor = 0x00417140;     // thiscall(text+0x18, float rgb[3]) RET 4 (R2d)
constexpr int BorderVtable = 0x009875BC, TextVtable = 0x009876B4;
constexpr int LabelBorderOff = 0x60, BorderColorOff = 0x28, BorderFillOff = 0x74;
constexpr int LabelTextOff = 0xD8, TextRendererOff = 0x14, TextStringSubOff = 0x18, TextColorOff = 0x2c;   // colour at text+0x18+0x2c
constexpr int CtlFlagsOff = 0x48, CtlIdOff = 0x54;   // flags bit1 visible, 0x20 click-through
constexpr int AppGlobal = 0x00a1b4a4;       // CAppManager*: +4 client
constexpr int FnPartyMember = 0x007e5da0;   // thiscall(party, i) RET 4 -> client creature
constexpr int FnClientToServer = 0x0077d800;   // ECX = client creature -> server creature (R2c)
constexpr int ClientCreatureVftable = 0x0099ee14;
constexpr int PanelShownOff = 0x4b70;       // SetStats' cache of the shown client creature (logged for comparison only)
constexpr int EffArrOff = 0x148, EffCntOff = 0x14c;
constexpr int EffTypeOff = 0x8, EffSubOff = 0xA, EffDurOff = 0xC, EffDayOff = 0x10, EffMsOff = 0x14, EffSpellOff = 0x1c;
constexpr int EffNIntsOff = 0x30, EffIntsOff = 0x34;
constexpr int EffTypeIcon = 0x43;
constexpr int WorldTimerGetCurrentTime = 0x0051ad20;   // thiscall(timer, int* day, int* ms)
constexpr int MsPerDay = 86400000, MaxEffects = 512;

// ---- controls (character_p.gui IDs 69.., order = NEW_TAGS in scripts/mirror_character_page.py) ----
constexpr int FirstId = 69, MaxRows = 14;
enum { T_BACK, T_HEADER, T_SECG, T_SECB, T_D0, T_I0 = T_D0 + MaxRows, T_N0 = T_I0 + MaxRows, T_T0 = T_N0 + MaxRows,
       T_MORE = T_T0 + MaxRows, T_SBTRK, T_SBTHM, T_SBUP, T_SBDN, T_TIPBG, T_TIPT, T_TIP, NumTags };
static_assert(NumTags == 68, "tag count must match mirror_character_page.NEW_TAGS");

// ---- layout (GUI units, 800x600; = scripts/character_page_mockup.py) ----
constexpr float PanelX = 536, PanelY = 134, PanelW = 216, PanelH = 336;
constexpr float PadX = 8, PadTop = 6, RowH = 22, SecH = 20, SecGap = 6, IconW = 10, IconH = 18, TimeW = 54;
constexpr float SbW = 10, SbArrowH = 18, SbGap = 3, SbThumbMin = 16;
constexpr float X0 = PanelX + PadX, InnerW = PanelW - 2 * PadX, Top = PanelY + PadTop, Win = PanelH - 2 * PadTop;

const float Cream[3] = {0.8f, 0.8f, 0.69804f}, Orange[3] = {0.94902f, 0.65882f, 0.30196f};
const float TealText[3] = {0.10196f, 0.69804f, 0.54902f}, TealFrame[3] = {0.05098f, 0.34902f, 0.27059f};
const float DiscTeal[3] = {0.035f, 0.415f, 0.34f}, White[3] = {1, 1, 1};   // disc = the Buff HUD backing colour
const char* const DiscResref = "buffhud_circle";

using CExoCtor = void*(__thiscall*)(void*, const char*);
using CExoDtor = void(__thiscall*)(void*);
using InitCtl = void(__thiscall*)(void*, void*, void*, int, int);
using NewFn = void*(__cdecl*)(unsigned);

// ---- diagnostics: chareffects_log.txt (capped); chareffects_debug.txt enables detail ----
int g_logLines = 0; constexpr int MaxLogLines = 20000;
bool g_debug = false; int g_dbgLines = 0; constexpr int MaxDebugLines = 10000;
void vlog(const char* fmt, va_list ap, bool stamp) {
    FILE* f = fopen("chareffects_log.txt", "a");
    if (!f) return;
    if (stamp) { SYSTEMTIME st; GetLocalTime(&st); fprintf(f, "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds); }
    vfprintf(f, fmt, ap); fputc('\n', f); fclose(f);
}
void logf(const char* fmt, ...) { if (g_logLines >= MaxLogLines) return; ++g_logLines; va_list ap; va_start(ap, fmt); vlog(fmt, ap, true); va_end(ap); }
void dlogf(const char* fmt, ...) { if (!g_debug || g_dbgLines >= MaxDebugLines) return; ++g_dbgLines; va_list ap; va_start(ap, fmt); vlog(fmt, ap, true); va_end(ap); }
template <typename T>
bool readAt(int base, int off, T* out) { return saferead::readAt<T>(0, static_cast<uintptr_t>(static_cast<unsigned>(base)), off, out); }
// early-return reasons: logged when the reason changes, else at most every 10 s
int g_lastWhy = -1; DWORD g_lastWhyAt = 0;
void whyLog(int code, const char* fmt, ...) {
    DWORD t = GetTickCount();
    if (code == g_lastWhy && t - g_lastWhyAt < 10000) return;
    g_lastWhy = code; g_lastWhyAt = t;
    if (g_logLines >= MaxLogLines) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vlog(fmt, ap, true); va_end(ap);
}

// ---- configuration: $GAME/chareffects.ini (missing/garbage = defaults; every key and rejected line logged) ----
struct Cfg {
    int enabled = 1, stage = 1, pollMs = 250, showPermanent = 1, wheelScroll = 1, tooltips = 0;
    int skipEquipped = 1, iconSource = 1, colorTimes = 1, discs = 1, soonS = 30;
    float textScale = 1.0f, screenW = 3840, screenH = 1600; bool screenExplicit = false;
    char fallbackIcon[17] = "ip_mind";
};
Cfg g_cfg; unsigned g_cfgGen = 0;
ULONGLONG g_iniStamp = 0; DWORD g_iniLastCheck = 0;
void trim(char* s) {
    char* p = s; while (*p && isspace((unsigned char)*p)) ++p;
    memmove(s, p, strlen(p) + 1);
    for (int n = (int)strlen(s); n > 0 && isspace((unsigned char)s[n - 1]); --n) s[n - 1] = 0;
}
bool parseNum(const char* v, float* out) { char* e; *out = strtof(v, &e); return e != v && !*e && isfinite(*out); }
void loadIni() {
    Cfg c;
    g_debug = GetFileAttributesA("chareffects_debug.txt") != INVALID_FILE_ATTRIBUTES;
    WIN32_FILE_ATTRIBUTE_DATA fa;
    g_iniStamp = GetFileAttributesExA("chareffects.ini", GetFileExInfoStandard, &fa)
        ? (((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime) : 0;
    FILE* f = fopen("chareffects.ini", "r");
    if (!f) { logf("INI: chareffects.ini not found, using defaults (stage=%d debug=%d)", c.stage, g_debug); g_cfg = c; ++g_cfgGen; return; }
    char line[300]; int ln = 0, ok = 0, bad = 0;
    struct { const char* k; int* i; float* fl; } keys[] = {
        {"enabled", &c.enabled, nullptr}, {"stage", &c.stage, nullptr}, {"poll_ms", &c.pollMs, nullptr},
        {"show_permanent", &c.showPermanent, nullptr}, {"wheel_scroll", &c.wheelScroll, nullptr}, {"tooltips", &c.tooltips, nullptr},
        {"skip_equipped", &c.skipEquipped, nullptr}, {"icon_source", &c.iconSource, nullptr}, {"color_times", &c.colorTimes, nullptr},
        {"discs", &c.discs, nullptr}, {"soon_s", &c.soonS, nullptr}, {"text_scale", nullptr, &c.textScale},
        {"screen_w", nullptr, &c.screenW}, {"screen_h", nullptr, &c.screenH}};
    while (fgets(line, sizeof(line), f)) {
        ++ln; trim(line);
        if (!line[0] || line[0] == '#' || line[0] == ';') continue;
        char* eq = strchr(line, '=');
        if (!eq) { logf("INI line %d rejected (no '='): %s", ln, line); ++bad; continue; }
        *eq = 0; char* k = line; char* v = eq + 1;
        for (char* cm = v; *cm; ++cm) if (*cm == ';' || *cm == '#') { *cm = 0; break; }
        trim(k); trim(v);
        for (char* p = k; *p; ++p) *p = (char)tolower((unsigned char)*p);
        bool known = false, good = false;
        if (!strcmp(k, "fallback_icon")) { known = true; good = v[0] && strlen(v) < sizeof(c.fallbackIcon); if (good) strcpy(c.fallbackIcon, v); }
        for (auto& e : keys) if (!strcmp(k, e.k)) {
            known = true; float a; good = parseNum(v, &a);
            if (good) { if (e.i) *e.i = (int)a; else *e.fl = a; }
            if (good && (!strcmp(k, "screen_w") || !strcmp(k, "screen_h"))) c.screenExplicit = true;
        }
        if (!known) { logf("INI line %d rejected (unknown key '%s')", ln, k); ++bad; }
        else if (!good) { logf("INI line %d rejected (bad value for '%s': '%s')", ln, k, v); ++bad; }
        else { logf("INI %s = %s", k, v); ++ok; }
    }
    fclose(f);
    if (c.stage < 0) c.stage = 0;
    if (c.stage > 1) c.stage = 1;
    if (c.pollMs < 16) c.pollMs = 16;
    if (c.textScale < 0.5f || c.textScale > 2.0f) c.textScale = 1.0f;
    if (c.screenW < 100 || c.screenH < 100) { c.screenW = 3840; c.screenH = 1600; c.screenExplicit = false; }
    g_cfg = c; ++g_cfgGen;
    logf("INI loaded: %d keys ok, %d rejected, debug=%d gen=%u enabled=%d stage=%d poll_ms=%d", ok, bad, g_debug, g_cfgGen, c.enabled, c.stage, c.pollMs);
}
void maybeReloadIni() {
    DWORD now = GetTickCount();
    if (now - g_iniLastCheck < 1000) return;
    g_iniLastCheck = now;
    WIN32_FILE_ATTRIBUTE_DATA fa; ULONGLONG st = 0;
    if (GetFileAttributesExA("chareffects.ini", GetFileExInfoStandard, &fa))
        st = ((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    bool dbg = GetFileAttributesA("chareffects_debug.txt") != INVALID_FILE_ATTRIBUTES;
    if (st != g_iniStamp || dbg != g_debug) { logf("INI changed on disk, reloading"); loadIni(); }
}

// ---- game window / wheel (copied from Mod 5 EquipCharPreview.cpp; chains to the previous proc) ----
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
void syncScreen() {   // engine framebuffer size (Aspyr globals, same ones BuffDurationHUD reads) unless the INI sets it
    if (g_cfg.screenExplicit) return;
    int w = 0, h = 0; readAt<int>(0x009F42A4, 0, &w); readAt<int>(0x009F42A8, 0, &h);
    if (w >= 320 && w <= 16384 && h >= 200 && h <= 16384) { g_cfg.screenW = (float)w; g_cfg.screenH = (float)h; }
}
WNDPROC g_oldProc = nullptr;
volatile LONG g_wheelAccum = 0;
volatile DWORD g_lastTickMs = 0;
volatile LONG g_panelRect[4] = {0, 0, 0, 0};   // engine px; [2] = 0 while the list does not overflow (wheel passes through)
int g_wheelLogs = 0;
LRESULT CALLBACK wheelProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_MOUSEWHEEL && g_cfg.wheelScroll && GetTickCount() - g_lastTickMs < 300 && g_panelRect[2] > 0) {
        POINT pt = {(short)LOWORD(l), (short)HIWORD(l)}; RECT r;
        bool inside = false;
        if (ScreenToClient(h, &pt) && GetClientRect(h, &r) && r.right > 0 && r.bottom > 0) {
            syncScreen(); float x = pt.x * g_cfg.screenW / r.right, y = pt.y * g_cfg.screenH / r.bottom;
            inside = x >= g_panelRect[0] && x < g_panelRect[0] + g_panelRect[2] && y >= g_panelRect[1] && y < g_panelRect[1] + g_panelRect[3];
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
    if (g_hwnd && g_cfg.wheelScroll && !g_oldProc) {
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

// ---- label plumbing (Mod 5) ----
unsigned char* bindLabel(int panel, const char* tagName, int expectId) {
    using LabelCtorFn = void(__fastcall*)(void*);
    unsigned char* ctl = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(LabelSize));
    if (!ctl) { logf("H1 %s: allocation failed", tagName); return nullptr; }
    memset(ctl, 0, LabelSize);
    reinterpret_cast<LabelCtorFn>(FnLabelCtor)(ctl);
    int vt = 0; readAt<int>(reinterpret_cast<int>(ctl), 0, &vt);
    if (vt != LabelVftable) { logf("H1 %s: ctor left vtable %08x (expected %08x), not binding", tagName, vt, LabelVftable); return nullptr; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(exo, tagName);
    reinterpret_cast<InitCtl>(FnInitControl)(reinterpret_cast<void*>(panel), ctl, exo, 1, 1);
    reinterpret_cast<CExoDtor>(FnExoDtor)(exo);
    int id = -999, fl = 0, ext[4] = {0};
    readAt<int>(reinterpret_cast<int>(ctl), CtlIdOff, &id); readAt<int>(reinterpret_cast<int>(ctl), CtlFlagsOff, &fl);
    for (int i = 0; i < 4; ++i) readAt<int>(reinterpret_cast<int>(ctl), 4 + 4 * i, &ext[i]);
    int nf = (id == expectId ? fl : (fl & ~2)) | 0x20;   // click-through; a mismatched control is hidden and not used
    *reinterpret_cast<int*>(ctl + CtlFlagsOff) = nf;
    logf("H1 %s: ctl=%p id=%d (expect %d)%s extent=[%d,%d,%d,%d] flags=%08x -> %08x", tagName, ctl, id, expectId,
         id == expectId ? "" : " MISMATCH", ext[0], ext[1], ext[2], ext[3], fl, nf);
    return id == expectId ? ctl : nullptr;
}
struct ImgPoolEntry { char resref[17]; void* image; };
constexpr int MaxImages = 128;
ImgPoolEntry g_imgPool[MaxImages]; int g_imgCount = 0;
void* imageFor(const char* resref) {   // one engine image per resref, never released (Buff HUD / Mod 5 pattern)
    for (int i = 0; i < g_imgCount; ++i) if (!strcmp(g_imgPool[i].resref, resref)) return g_imgPool[i].image;
    if (g_imgCount >= MaxImages) return nullptr;
    char buf[20] = {}; strncpy(buf, resref, 16);
    for (char* p = buf; *p; ++p) *p = (char)tolower((unsigned char)*p);
    void* im = reinterpret_cast<void*(__cdecl*)(const char*)>(FnLoadImage)(buf);
    ImgPoolEntry& e = g_imgPool[g_imgCount++]; memset(&e, 0, sizeof e); strncpy(e.resref, resref, 16); e.image = im;
    logf("image '%s' -> %p", buf, im);
    return im;
}
void setCtlVisible(unsigned char* c, bool vis) {
    int fl = 0; if (!c || !readAt<int>(reinterpret_cast<int>(c), CtlFlagsOff, &fl)) return;
    int nf = vis ? (fl | 2) : (fl & ~2);
    if (nf != fl) *reinterpret_cast<int*>(c + CtlFlagsOff) = nf;
}
bool ctlRect(unsigned char* c, float* r) {   // engine-space rect from a label's extent (+4..+0x10, top-left origin)
    if (!c) return false;
    int e[4]; for (int k = 0; k < 4; ++k) if (!readAt<int>(reinterpret_cast<int>(c), 4 + 4 * k, &e[k])) return false;
    for (int k = 0; k < 4; ++k) r[k] = (float)e[k];
    return e[2] > 0 && e[3] > 0;
}
void setCtlRect(unsigned char* c, int x, int y, int w, int h) {
    void** vt = nullptr; void* fn = nullptr; if (!c || !readAt<void**>(reinterpret_cast<int>(c), 0, &vt) || !vt || !readAt<void*>(reinterpret_cast<int>(vt), 4, &fn) || !fn) return;
    int r[4] = {x, y, w, h}; reinterpret_cast<void(__thiscall*)(void*, int*)>(vt[1])(c, r);
}
bool textObjOk(unsigned char* c) {
    int tx = reinterpret_cast<int>(c) + LabelTextOff, tvt = 0, rend = 0;
    return readAt<int>(tx, 0, &tvt) && tvt == TextVtable && readAt<int>(tx, TextRendererOff, &rend) && rend;
}
bool readVec3(int addr, float* v) { for (int k = 0; k < 3; ++k) if (!readAt<float>(addr, 4 * k, &v[k])) return false; return true; }
bool near3(const float* a, const float* b) { for (int k = 0; k < 3; ++k) if (fabsf(a[k] - b[k]) > 0.01f) return false; return true; }

// ---- per-panel slot ----
struct Entry {
    bool section, harmful, gap, item, perm;
    int key;              // dedup key: 0x10000 + effecticon row, or spells.2da row
    double remMs;         // < 0 = unknown
    char name[64]; char icon[17];
};
constexpr int MaxEnt = 96;
struct Slot {
    int panel = 0; bool bound = false; unsigned seq = 0; DWORD bindTick = 0;
    unsigned char* ctl[NumTags] = {};
    char text[NumTags][64] = {}; void* fill[NumTags] = {}; int col[NumTags] = {}; int rect[NumTags][4] = {};
    float bx = 0, by = 0, sx = 0, sy = 0;     // unit -> px from the bound LBL_EFX_BACK extent
    bool canTextColor = false, canBorderColor = false;
    int client = 0, server = 0; unsigned sig = 0; DWORD lastPoll = 0, lastBeat = 0;
    Entry ent[MaxEnt]; int nEnt = 0; int first = 0;
    // input
    bool wasL = false, dragging = false; float dragOff = 0; int heldPart = 0; DWORD heldNext = 0;
    // last layout (engine px) for hit tests
    bool over = false; int shownN = 0, lastFirst = 0; float rUp[4] = {}, rDn[4] = {}, rTrk[4] = {}, rThm[4] = {};
};
constexpr int MaxSlots = 4;
Slot g_slots[MaxSlots];
unsigned g_seq = 0;
Slot* findSlot(int panel) { for (auto& s : g_slots) if (s.panel == panel && s.bound) return &s; return nullptr; }
Slot* claimSlot(int panel) {
    if (Slot* e = findSlot(panel)) return e;
    Slot* oldest = &g_slots[0];
    for (auto& s : g_slots) { if (!s.bound) { oldest = &s; break; } if (s.seq < oldest->seq) oldest = &s; }
    if (oldest->bound) logf("slot: evicting panel=%08x seq=%u", oldest->panel, oldest->seq);
    return oldest;
}

int ux(Slot* s, float u) { return (int)lroundf(s->bx + (u - PanelX) * s->sx); }
int uy(Slot* s, float u) { return (int)lroundf(s->by + (u - PanelY) * s->sy); }
void place(Slot* s, int t, float x, float y, float w, float h) {
    unsigned char* c = s->ctl[t]; if (!c) return;
    int r[4] = {ux(s, x), uy(s, y), ux(s, x + w) - ux(s, x), uy(s, y + h) - uy(s, y)};
    if (memcmp(r, s->rect[t], sizeof r)) { setCtlRect(c, r[0], r[1], r[2], r[3]); memcpy(s->rect[t], r, sizeof r); }
    setCtlVisible(c, true);
}
void hide(Slot* s, int t) { setCtlVisible(s->ctl[t], false); }
void setText(Slot* s, int t, const char* txt) {
    unsigned char* c = s->ctl[t]; if (!c || !strcmp(s->text[t], txt)) return;
    if (!textObjOk(c)) { whyLog(100 + t, "label %d: text object bad, cannot set \"%s\"", t, txt); return; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(exo, txt);
    reinterpret_cast<void(__thiscall*)(void*, void*)>(FnTextSet)(c + LabelTextOff + TextStringSubOff, exo);
    reinterpret_cast<CExoDtor>(FnExoDtor)(exo);
    dlogf("label %d: text \"%s\" -> \"%s\"", t, s->text[t], txt);
    strncpy(s->text[t], txt, sizeof s->text[t] - 1); s->text[t][sizeof s->text[t] - 1] = 0;
}
void setTextColor(Slot* s, int t, int which, const float* rgb) {   // which: cache key (1 cream, 2 orange)
    unsigned char* c = s->ctl[t]; if (!c || !s->canTextColor || s->col[t] == which || !textObjOk(c)) return;
    float v[3] = {rgb[0], rgb[1], rgb[2]};
    reinterpret_cast<void(__thiscall*)(void*, float*)>(FnTextColor)(c + LabelTextOff + TextStringSubOff, v);
    s->col[t] = which;
}
void setFill(Slot* s, int t, const char* resref, const float* tint) {
    unsigned char* c = s->ctl[t]; if (!c) return;
    int border = reinterpret_cast<int>(c) + LabelBorderOff, bvt = 0;
    if (!readAt<int>(border, 0, &bvt) || bvt != BorderVtable) { whyLog(200 + t, "label %d: border vtable %08x != %08x, no fill", t, bvt, BorderVtable); return; }
    void* im = resref ? imageFor(resref) : nullptr;
    if (resref && !im) im = imageFor(g_cfg.fallbackIcon);
    if (s->fill[t] != im) { *reinterpret_cast<void**>(border + BorderFillOff) = im; s->fill[t] = im; }
    if (tint && s->canBorderColor) { float* col = reinterpret_cast<float*>(border + BorderColorOff); if (!near3(col, tint)) for (int k = 0; k < 3; ++k) col[k] = tint[k]; }
}

// ---- effect data ----
bool currentTime(int* day, int* ms) {   // BuffDurationHUD.cpp
    int g, a, b;
    if (!readAt(AppGlobal, 0, &g) || !readAt(g, 8, &a) || !readAt(a, 4, &b)) return false;
    int timer;
    if (!readAt(b, 0x10048, &timer) || timer == 0) return false;
    reinterpret_cast<void(__thiscall*)(void*, int*, int*)>(WorldTimerGetCurrentTime)(reinterpret_cast<void*>(timer), day, ms);
    return true;
}
bool isItemAbility(int spell) {   // BuffDurationHUD.cpp: stims, shots, shields, cloak
    return (spell >= 69 && spell <= 77) || (spell >= 99 && spell <= 108) || (spell >= 110 && spell <= 115) || spell == 129 || spell == 132 || spell == 257;
}
int leaderClient() {   // what SetStats 0x84e6f0 shows: 0x7e5da0(0x73fb90(client), 0)
    int app = 0, client = 0, c4 = 0, party = 0;
    if (!readAt<int>(AppGlobal, 0, &app) || !readAt<int>(app, 4, &client) || !readAt<int>(client, 4, &c4) || !readAt<int>(c4, 0x270, &party) || !party) return 0;
    int n = 0; if (!readAt<int>(party, 0, &n) || n <= 0 || n > 64) return 0;
    return reinterpret_cast<int(__thiscall*)(int, int)>(FnPartyMember)(party, 0);
}
bool clientValid(int c) { int vt = 0; return readAt<int>(c, 0, &vt) && vt == ClientCreatureVftable; }

struct Raw { int eff, type, sub, spell, iconRow; float dur; double rem; bool perm; };
void fmtTime(const Entry& e, char* out, int n) {
    if (e.perm) { snprintf(out, n, "Permanent"); return; }
    if (e.remMs < 0) { snprintf(out, n, "--"); return; }
    long secs = (long)ceil(e.remMs / 1000.0);
    if (secs >= 3600) snprintf(out, n, "%ld:%02ld:%02ld", secs / 3600, secs / 60 % 60, secs % 60);
    else snprintf(out, n, "%ld:%02ld", secs / 60, secs % 60);
}
// Collect + classify + dedup + sort; returns a signature of the structure (keys/order) for change logging.
int collect(Slot* s, int server, Entry* out, bool dump) {
    int arr = 0, count = 0;
    if (!readAt<int>(server, EffArrOff, &arr) || !readAt<int>(server, EffCntOff, &count) || count < 0 || count > MaxEffects) return -1;
    if (!arr || !count) return 0;
    int day = 0, ms = 0; bool haveTime = currentTime(&day, &ms);
    static Raw raw[MaxEffects]; int nr = 0;
    for (int i = 0; i < count; ++i) {
        int eff = 0; if (!readAt<int>(arr, 4 * i, &eff) || !eff) continue;
        Raw r = {}; r.eff = eff; unsigned short ty = 0, sb = 0;
        if (!readAt<unsigned short>(eff, EffTypeOff, &ty) || !readAt<unsigned short>(eff, EffSubOff, &sb) || !readAt<int>(eff, EffSpellOff, &r.spell)) continue;
        r.type = ty; r.sub = sb; r.iconRow = -1; readAt<float>(eff, EffDurOff, &r.dur);
        int dt = sb & 7;   // 0 instant, 1 temporary, 2 permanent, 3 equipped, 4 innate
        if (r.type == EffTypeIcon) { int ni = 0, ip = 0; if (readAt<int>(eff, EffNIntsOff, &ni) && ni > 0 && readAt<int>(eff, EffIntsOff, &ip) && ip) readAt<int>(ip, 0, &r.iconRow); }
        r.perm = dt != 1 || !(r.dur > 0); r.rem = -1;
        int eDay = 0, eMs = 0;
        if (!r.perm && haveTime && readAt<int>(eff, EffDayOff, &eDay) && readAt<int>(eff, EffMsOff, &eMs)) { r.rem = double(eDay - day) * MsPerDay + double(eMs - ms); if (r.rem < 0) r.rem = 0; }
        raw[nr++] = r;
    }
    // spells that carry an EffectIcon: the icon row is the authoritative entry for them
    static int iconSpells[MaxEffects]; int nis = 0;
    for (int i = 0; i < nr; ++i) if (raw[i].type == EffTypeIcon && raw[i].iconRow > 0 && raw[i].iconRow < kEffIconRowCount && kEffIconRows[raw[i].iconRow].name) iconSpells[nis++] = raw[i].spell;
    Entry tmp[MaxEnt]; int nt = 0;
    for (int i = 0; i < nr; ++i) {
        const Raw& r = raw[i];
        int dt = r.sub & 7;
        Entry e = {}; bool use = false; const char* why = "";
        if (g_cfg.skipEquipped && dt == 3) why = "equipped";
        else if (r.type == EffTypeIcon) {
            if (r.iconRow <= 0 || r.iconRow >= kEffIconRowCount) why = "icon row out of range";
            else if (!kEffIconRows[r.iconRow].name) why = "icon row has no name";
            else {
                const EffIconRow& row = kEffIconRows[r.iconRow];
                bool harm = row.good == 0;
                if (row.good < 0) harm = r.spell >= 0 && r.spell < kDebuffIconTableSize && kDebuffIcons[r.spell].resref;
                e.key = 0x10000 + r.iconRow; e.harmful = harm; strncpy(e.name, row.name, sizeof e.name - 1);
                if (row.icon) strncpy(e.icon, row.icon, 16);
                e.item = row.icon && (!strncmp(row.icon, "ii_", 3) || !strncmp(row.icon, "iw_", 3));
                use = g_cfg.iconSource != 0; if (!use) why = "icon_source=0";
            }
        } else if (r.spell < 0) why = "no spell";
        else if (r.spell >= kSpellRowCount || r.spell >= kEffectIconTableSize) why = "spell out of range";
        else {
            bool iconCovered = false; for (int k = 0; k < nis; ++k) if (iconSpells[k] == r.spell) { iconCovered = true; break; }
            const char* bad = kDebuffIcons[r.spell].resref; const char* good = kEffectIcons[r.spell];
            if (g_cfg.iconSource && iconCovered) why = "covered by EffectIcon";
            else if (!bad && !good) why = "untracked spell";
            else {
                e.key = r.spell; e.harmful = bad != nullptr;
                const char* nm = kSpellRows[r.spell].name;
                if (nm) strncpy(e.name, nm, sizeof e.name - 1); else snprintf(e.name, sizeof e.name, "Effect %d", r.spell);
                strncpy(e.icon, bad ? bad : good, 16);
                e.item = bad ? kDebuffIcons[r.spell].item : isItemAbility(r.spell);
                use = true;
            }
        }
        if (use && r.perm && !g_cfg.showPermanent) { use = false; why = "permanent hidden"; }
        if (dump) logf("  eff[%d] %08x type=0x%02x sub=0x%04x spell=%d iconRow=%d dur=%.1f %s rem=%.1fs -> %s%s%s%s",
                       i, r.eff, r.type, r.sub, r.spell, r.iconRow, r.dur, r.perm ? "PERM" : "temp", r.rem / 1000.0,
                       use ? (e.harmful ? "HARMFUL '" : "BENEFICIAL '") : "skip: ", use ? e.name : why, use ? "'" : "",
                       (r.type == EffTypeIcon && r.iconRow > 0 && r.iconRow < kEffIconRowCount) ? (kEffIconRows[r.iconRow].good == 1 ? " [icon good=1]" : kEffIconRows[r.iconRow].good == 0 ? " [icon good=0]" : " [icon good=-]") : "");
        if (!use) continue;
        e.perm = r.perm; e.remMs = r.rem;
        int k = 0; for (; k < nt; ++k) if (tmp[k].key == e.key) break;
        if (k < nt) {   // keep the longest remaining (permanent beats everything)
            bool longer = (e.perm && !tmp[k].perm) || (!tmp[k].perm && !e.perm && e.remMs > tmp[k].remMs);
            if (longer) { tmp[k].perm = e.perm; tmp[k].remMs = e.remMs; }
            continue;
        }
        if (nt < MaxEnt - 2) tmp[nt++] = e;
    }
    // sections: Harmful first, then Beneficial; inside ascending remaining, unknown after timed, permanent last
    auto rank = [](const Entry& e) { return e.perm ? 2 : (e.remMs < 0 ? 1 : 0); };
    auto after = [&](const Entry& a, const Entry& b) {   // true when a sorts after b
        if (rank(a) != rank(b)) return rank(a) > rank(b);
        if (rank(a) == 0 && a.remMs != b.remMs) return a.remMs > b.remMs;
        return strcmp(a.name, b.name) > 0;
    };
    for (int i = 1; i < nt; ++i) for (int j = i; j > 0 && after(tmp[j - 1], tmp[j]); --j) { Entry t = tmp[j - 1]; tmp[j - 1] = tmp[j]; tmp[j] = t; }
    int n = 0; unsigned sig = 2166136261u;
    for (int pass = 0; pass < 2; ++pass) {
        bool harm = pass == 0; bool any = false;
        for (int i = 0; i < nt; ++i) if (tmp[i].harmful == harm) { any = true; break; }
        if (!any) continue;
        Entry sec = {}; sec.section = true; sec.harmful = harm; sec.gap = n > 0; out[n++] = sec;
        for (int i = 0; i < nt; ++i) if (tmp[i].harmful == harm) { out[n++] = tmp[i]; sig = (sig ^ (unsigned)tmp[i].key) * 16777619u; }
        sig = (sig ^ (harm ? 0xB0u : 0x60u)) * 16777619u;
    }
    s->sig = sig;
    return n;
}

// ---- whole-entry scrolling (= character_page_mockup.fits / scroll_state) ----
float entryH(const Entry& e, bool atTop) { return e.section ? SecH + 2 + ((e.gap && !atTop) ? SecGap : 0) : RowH; }
int fits(const Entry* e, int n, int first) {
    int k = 0; float acc = 0;
    while (first + k < n) { float h = entryH(e[first + k], k == 0); if (acc + h > Win) break; acc += h; ++k; }
    return k;
}
int lastFirstOf(const Entry* e, int n) {
    int lf = n - 1;
    while (lf > 0 && fits(e, n, lf - 1) == n - (lf - 1)) --lf;
    return lf < 0 ? 0 : lf;
}

// fitted name: cut with "..." to the label width (font texels drawn at sy px per texel, see gen_effect_name_table.py)
void fitName(Slot* s, const char* name, float maxPx, char* out, int n) {
    float k = s->sy * g_cfg.textScale;
    auto width = [&](const char* t, int len) { float w = 0; for (int i = 0; i < len; ++i) w += kFontTexW[(unsigned char)t[i]]; return w * k; };
    int len = (int)strlen(name);
    if (width(name, len) <= maxPx) { snprintf(out, n, "%s", name); return; }
    float dots = width("...", 3);
    while (len > 0 && width(name, len) + dots > maxPx) --len;
    while (len > 0 && name[len - 1] == ' ') --len;
    snprintf(out, n, "%.*s...", len, name);
}

void hideAll(Slot* s) { for (int t = 0; t < NumTags; ++t) hide(s, t); s->over = false; g_panelRect[2] = 0; }

void layout(Slot* s) {
    const Entry* e = s->ent; int n = s->nEnt;
    for (int t = T_TIPBG; t <= T_TIP; ++t) hide(s, t);   // tooltips deferred
    if (g_cfg.stage < 1 || n == 0) {
        for (int t = T_SECG; t < T_MORE; ++t) hide(s, t);
        for (int t = T_SBTRK; t <= T_SBDN; ++t) hide(s, t);
        char msg[64];
        int rows = 0; for (int i = 0; i < n; ++i) rows += !e[i].section;
        if (n == 0) snprintf(msg, sizeof msg, "No active effects");
        else snprintf(msg, sizeof msg, "%d active effect%s (preview)", rows, rows == 1 ? "" : "s");
        place(s, T_MORE, X0, PanelY + PanelH / 2 - RowH / 2, InnerW, RowH);
        setText(s, T_MORE, msg);
        s->over = false; g_panelRect[2] = 0;
        return;
    }
    hide(s, T_MORE);
    bool over = fits(e, n, 0) < n;
    int lf = over ? lastFirstOf(e, n) : 0;
    if (s->first > lf) s->first = lf;
    if (s->first < 0) s->first = 0;
    int cnt = fits(e, n, s->first);
    float rowW = InnerW - (over ? SbW + SbGap : 0);
    float y = Top; int r = 0; bool secShown[2] = {false, false};
    for (int k = 0; k < cnt; ++k) {
        const Entry& x = e[s->first + k];
        if (x.section) {
            if (x.gap && k > 0) y += SecGap;
            int t = x.harmful ? T_SECB : T_SECG; secShown[x.harmful] = true;
            place(s, t, X0, y, rowW, SecH);
            y += SecH + 2;
            continue;
        }
        if (r >= MaxRows) break;
        place(s, T_I0 + r, X0, y + (RowH - IconH) / 2, IconW, IconH);
        setFill(s, T_I0 + r, x.icon[0] ? x.icon : g_cfg.fallbackIcon, White);
        if (x.item && g_cfg.discs) { place(s, T_D0 + r, X0, y + (RowH - IconH) / 2, IconW, IconH); setFill(s, T_D0 + r, DiscResref, DiscTeal); }
        else hide(s, T_D0 + r);
        float nw = rowW - (IconW + 4) - TimeW - 2;
        place(s, T_N0 + r, X0 + IconW + 4, y, nw, RowH);
        char nm[64]; fitName(s, x.name, (ux(s, X0 + IconW + 4 + nw) - ux(s, X0 + IconW + 4)) * 0.98f, nm, sizeof nm);
        setText(s, T_N0 + r, nm);
        place(s, T_T0 + r, X0 + rowW - TimeW, y, TimeW, RowH);
        char tm[16]; fmtTime(x, tm, sizeof tm); setText(s, T_T0 + r, tm);
        bool warn = g_cfg.colorTimes && (x.harmful || (!x.perm && x.remMs >= 0 && x.remMs < g_cfg.soonS * 1000.0));
        setTextColor(s, T_T0 + r, warn ? 2 : 1, warn ? Orange : Cream);
        y += RowH; ++r;
    }
    for (; r < MaxRows; ++r) { hide(s, T_D0 + r); hide(s, T_I0 + r); hide(s, T_N0 + r); hide(s, T_T0 + r); }
    if (!secShown[0]) hide(s, T_SECG);
    if (!secShown[1]) hide(s, T_SECB);
    s->over = over; s->shownN = cnt; s->lastFirst = lf;
    if (!over) { for (int t = T_SBTRK; t <= T_SBDN; ++t) hide(s, t); g_panelRect[2] = 0; return; }
    float sbx = X0 + InnerW - SbW;
    float trkY = Top + SbArrowH + 2, trkH = Win - 2 * SbArrowH - 4;
    float th = fmaxf(SbThumbMin, roundf(trkH * cnt / n));
    float ty = trkY + roundf((trkH - th) * s->first / (lf > 0 ? lf : 1));
    place(s, T_SBTRK, sbx, trkY, SbW, trkH);
    place(s, T_SBTHM, sbx + 2, ty, SbW - 4, th);
    place(s, T_SBUP, sbx, Top, SbW, SbArrowH);
    place(s, T_SBDN, sbx, Top + Win - SbArrowH, SbW, SbArrowH);
    const float Dim[3] = {0.35f, 0.35f, 0.35f};   // arrows dim at the ends (border tint; skipped if the offset check failed)
    setFill(s, T_SBUP, "uibit_uparrowg", s->first > 0 ? White : Dim);
    setFill(s, T_SBDN, "efx_dnarrowg", s->first < lf ? White : Dim);
    ctlRect(s->ctl[T_SBUP], s->rUp); ctlRect(s->ctl[T_SBDN], s->rDn); ctlRect(s->ctl[T_SBTRK], s->rTrk); ctlRect(s->ctl[T_SBTHM], s->rThm);
    g_panelRect[0] = ux(s, PanelX); g_panelRect[1] = uy(s, PanelY); g_panelRect[3] = uy(s, PanelY + PanelH) - g_panelRect[1];
    g_panelRect[2] = ux(s, PanelX + PanelW) - g_panelRect[0];
}

bool inRect(const float* r, float x, float y) { return r[2] > 0 && x >= r[0] && x < r[0] + r[2] && y >= r[1] && y < r[1] + r[3]; }
bool scrollTo(Slot* s, int f, const char* why) {
    if (f > s->lastFirst) f = s->lastFirst;
    if (f < 0) f = 0;
    if (f == s->first) return false;
    dlogf("scroll: %d -> %d (%s)", s->first, f, why);
    s->first = f; return true;
}
// returns true when the scroll position changed
bool tickInput(Slot* s) {
    bool changed = false;
    LONG wheel = InterlockedExchange(&g_wheelAccum, 0);
    if (wheel && s->over) { int steps = -(int)(wheel / 120); if (!steps) steps = wheel > 0 ? -1 : 1; changed |= scrollTo(s, s->first + steps, "wheel"); }
    bool l = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (!s->over) { s->dragging = false; s->heldPart = 0; s->wasL = l; return changed; }
    float cx = 0, cy = 0; bool haveCur = cursorEngine(&cx, &cy);
    bool fg = gameWindow() && GetForegroundWindow() == g_hwnd;
    DWORD t = GetTickCount();
    if (l && !s->wasL && haveCur && fg) {
        if (inRect(s->rUp, cx, cy)) { s->heldPart = -1; s->heldNext = t + 400; changed |= scrollTo(s, s->first - 1, "up arrow"); }
        else if (inRect(s->rDn, cx, cy)) { s->heldPart = 1; s->heldNext = t + 400; changed |= scrollTo(s, s->first + 1, "down arrow"); }
        else if (inRect(s->rThm, cx, cy)) { s->dragging = true; s->dragOff = cy - s->rThm[1]; logf("scroll: thumb drag start at y=%.0f first=%d", cy, s->first); }
        else if (inRect(s->rTrk, cx, cy)) {
            int page = s->shownN > 1 ? s->shownN - 1 : 1;
            changed |= scrollTo(s, cy < s->rThm[1] ? s->first - page : s->first + page, "track page");
        }
    }
    if (!l) { if (s->dragging) logf("scroll: thumb drag end first=%d", s->first); s->dragging = false; s->heldPart = 0; }
    if (l && s->heldPart && t >= s->heldNext) { s->heldNext = t + 100; changed |= scrollTo(s, s->first + s->heldPart, "arrow held"); }
    if (s->dragging && haveCur && s->rTrk[3] > s->rThm[3]) {
        float frac = (cy - s->dragOff - s->rTrk[1]) / (s->rTrk[3] - s->rThm[3]);
        frac = fminf(fmaxf(frac, 0.0f), 1.0f);
        changed |= scrollTo(s, (int)lroundf(frac * s->lastFirst), "thumb drag");
    }
    s->wasL = l;
    return changed;
}

LARGE_INTEGER g_qpcFreq, g_qpcLast;

} // namespace

extern "C" void __cdecl BindCharEffects(int panel) {
    saferead::beginScope();
    int ctlCount = -1; readAt<int>(panel, 0x28, &ctlCount);
    logf("H1 BindCharEffects panel=%08x controls=%d enabled=%d stage=%d", panel, ctlCount, g_cfg.enabled, g_cfg.stage);
    if (!panel) { logf("H1: null panel, abort"); return; }
    if (!g_cfg.enabled) { logf("H1: disabled by INI, controls not bound (right third stays empty)"); return; }
    Slot* s = claimSlot(panel);
    *s = Slot();
    s->panel = panel; s->seq = ++g_seq; s->bindTick = GetTickCount();
    static const char* const fixed[] = {"LBL_EFX_BACK", "LBL_EFX_HEADER", "LBL_EFX_SECG", "LBL_EFX_SECB"};
    static const char* const tail[] = {"LBL_EFX_MORE", "LBL_EFX_SBTRK", "LBL_EFX_SBTHM", "LBL_EFX_SBUP", "LBL_EFX_SBDN", "LBL_EFX_TIPBG", "LBL_EFX_TIPT", "LBL_EFX_TIP"};
    int bound = 0;
    for (int t = 0; t < NumTags; ++t) {
        char tag[20];
        if (t < T_D0) snprintf(tag, sizeof tag, "%s", fixed[t]);
        else if (t < T_MORE) snprintf(tag, sizeof tag, "LBL_EFX_%c%02d", "DINT"[(t - T_D0) / MaxRows], (t - T_D0) % MaxRows);
        else snprintf(tag, sizeof tag, "%s", tail[t - T_MORE]);
        s->ctl[t] = bindLabel(panel, tag, FirstId + t);
        bound += s->ctl[t] != nullptr;
    }
    int newCount = -1; readAt<int>(panel, 0x28, &newCount);
    logf("H1: bound %d/%d labels, controls now %d (was %d)", bound, (int)NumTags, newCount, ctlCount);
    float br[4];
    if (!s->ctl[T_BACK] || !ctlRect(s->ctl[T_BACK], br)) { logf("H1: LBL_EFX_BACK missing (is character_p.gui patched? scripts/mirror_character_page.py), panel unusable"); s->panel = panel; s->bound = false; return; }
    s->bx = br[0]; s->by = br[1]; s->sx = br[2] / PanelW; s->sy = br[3] / PanelH;
    logf("H1: BACK px [%.0f,%.0f,%.0f,%.0f] -> unit scale %.4f x %.4f", br[0], br[1], br[2], br[3], s->sx, s->sy);
    // colour offsets: accept only when the GUI's own colours read back where expected
    {
        float v[3] = {}, w[3] = {};
        bool tOk = s->ctl[T_T0] && textObjOk(s->ctl[T_T0]) && readVec3(reinterpret_cast<int>(s->ctl[T_T0]) + LabelTextOff + TextStringSubOff + TextColorOff, v) && near3(v, Cream);
        bool nOk = s->ctl[T_N0] && textObjOk(s->ctl[T_N0]) && readVec3(reinterpret_cast<int>(s->ctl[T_N0]) + LabelTextOff + TextStringSubOff + TextColorOff, w) && near3(w, TealText);
        s->canTextColor = tOk && nOk;
        logf("H1: text colour check T00=(%.3f,%.3f,%.3f) N00=(%.3f,%.3f,%.3f) -> runtime time colours %s", v[0], v[1], v[2], w[0], w[1], w[2], s->canTextColor ? "ON" : "OFF");
        float a[3] = {}, b[3] = {};
        bool dOk = s->ctl[T_D0] && readVec3(reinterpret_cast<int>(s->ctl[T_D0]) + LabelBorderOff + BorderColorOff, a) && near3(a, White);
        bool sOk = s->ctl[T_SECG] && readVec3(reinterpret_cast<int>(s->ctl[T_SECG]) + LabelBorderOff + BorderColorOff, b) && near3(b, TealFrame);
        s->canBorderColor = dOk && sOk;
        logf("H1: border colour check D00=(%.3f,%.3f,%.3f) SECG=(%.3f,%.3f,%.3f) -> disc/arrow tint %s", a[0], a[1], a[2], b[0], b[1], b[2], s->canBorderColor ? "ON" : "OFF");
        for (int t = 0; t < NumTags; ++t) s->col[t] = 1;   // GUI text colour = the cached "cream" state for times
    }
    for (int t = 0; t < NumTags; ++t) if (t != T_BACK && t != T_HEADER) hide(s, t);
    setCtlVisible(s->ctl[T_BACK], true); setCtlVisible(s->ctl[T_HEADER], true);
    s->bound = true;
    layout(s);   // empty state until the first tick polls the creature
    logf("H1: slot %d ready (panel=%08x seq=%u)", static_cast<int>(s - g_slots), panel, s->seq);
}

extern "C" void __cdecl CharEffectsTick(int panel) {
    saferead::beginScope();
    static int calls = 0;
    if (!g_qpcFreq.QuadPart) { QueryPerformanceFrequency(&g_qpcFreq); QueryPerformanceCounter(&g_qpcLast); }
    LARGE_INTEGER now; QueryPerformanceCounter(&now);
    float dt = (float)((double)(now.QuadPart - g_qpcLast.QuadPart) / (double)g_qpcFreq.QuadPart);
    g_qpcLast = now;
    g_lastTickMs = GetTickCount();
    Slot* s = findSlot(panel);
    if (calls < 5) { ++calls; logf("H3 tick #%d panel=%08x slot=%s", calls, panel, s ? "bound" : "none"); }
    if (!s) return;
    if (dt > 0.5f) logf("H3: tick gap %.2fs (page reopened)", dt);
    maybeReloadIni();
    if (!g_cfg.enabled) { hideAll(s); whyLog(1, "H3: disabled by INI, panel hidden"); return; }
    setCtlVisible(s->ctl[T_BACK], true); setCtlVisible(s->ctl[T_HEADER], true);
    bool scrolled = g_cfg.stage >= 1 && tickInput(s);
    DWORD t = GetTickCount();
    bool poll = t - s->lastPoll >= (DWORD)g_cfg.pollMs || dt > 0.5f;
    if (!poll) { if (scrolled) layout(s); return; }
    s->lastPoll = t;
    int client = leaderClient();
    if (!client) { whyLog(2, "H3: no party leader (client creature null)"); s->nEnt = 0; layout(s); return; }
    if (!clientValid(client)) { int vt = 0; readAt<int>(client, 0, &vt); whyLog(3, "H3: leader %08x vtable %08x != %08x, skipped", client, vt, ClientCreatureVftable); return; }
    int server = reinterpret_cast<int(__thiscall*)(int)>(FnClientToServer)(client);
    saferead::beginScope();   // engine code ran: drop the readable-region cache
    if (!server) { whyLog(4, "H3: leader %08x has no server creature", client); s->nEnt = 0; layout(s); return; }
    bool changedCreature = client != s->client || server != s->server;
    if (changedCreature) {
        int shown = 0; readAt<int>(panel, PanelShownOff, &shown);
        logf("H3: creature client=%08x server=%08x (panel+0x4b70=%08x%s) -> scroll reset", client, server, shown, shown == client ? "" : " differs: SetStats not run yet");
        s->client = client; s->server = server; s->first = 0;
    }
    unsigned oldSig = s->sig; int oldN = s->nEnt;
    int n = collect(s, server, s->ent, false);
    if (n < 0) { whyLog(5, "H3: effect list of %08x unreadable", server); s->nEnt = 0; layout(s); return; }
    s->nEnt = n;
    if (changedCreature || s->sig != oldSig || n != oldN) {   // structure changed: dump once with reasons
        int arr = 0, cnt = 0; readAt<int>(server, EffArrOff, &arr); readAt<int>(server, EffCntOff, &cnt);
        logf("H3: list changed: %d entries (sig %08x -> %08x), %d raw effects", n, oldSig, s->sig, cnt);
        collect(s, server, s->ent, true);
        for (int i = 0; i < n; ++i) { char tm[16]; fmtTime(s->ent[i], tm, sizeof tm); logf("  entry %d: %s%s%s icon=%s item=%d %s", i, s->ent[i].section ? "[SECTION] " : "", s->ent[i].section ? (s->ent[i].harmful ? "HARMFUL" : "BENEFICIAL") : s->ent[i].name, "", s->ent[i].icon, s->ent[i].item, s->ent[i].section ? "" : tm); }
    }
    layout(s);
    if (t - s->bindTick < 120000 && t - s->lastBeat >= 5000) {
        s->lastBeat = t;
        logf("H3 beat: client=%08x server=%08x entries=%d first=%d over=%d stage=%d", client, server, s->nEnt, s->first, s->over, g_cfg.stage);
    }
}

extern "C" void __cdecl CharEffectsDestroy(int panel) {
    Slot* s = findSlot(panel);
    logf("H4 CharEffectsDestroy panel=%08x slot=%s", panel, s ? "found" : "none");
    if (!s) return;
    g_panelRect[2] = 0;
    s->bound = false; s->panel = 0;   // controls are engine-owned (InitControl addToList=1): never freed here
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_DETACH && g_oldProc && g_hwnd && IsWindow(g_hwnd) &&
        reinterpret_cast<WNDPROC>(GetWindowLongPtrA(g_hwnd, GWLP_WNDPROC)) == wheelProc) {
        SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_oldProc)); g_oldProc = nullptr;   // never leave a dangling window proc
    }
    if (reason == DLL_PROCESS_ATTACH) { logf("chareffects 0.1.0-probe loaded"); loadIni(); saferead::enableRing(g_debug); }
    return TRUE;
}
