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
// Build 2 (still 0.1.0-probe, DLL swap): INI fake_rows=N (synthetic entries for overflow/scrollbar tests), one LAYOUT log
// line per change of (list, first, overflow), INPUT press log (hit part, capped at 40).
// Build 3 (user screenshot 19:09): more vertical room (pad/row/section spacing), section band 4 units wider than the rows
// on the left with the title inset by a leading space, measured text widths (1.98 px/texel, not sy), times right-aligned by
// placement (the engine drew the GUI's align 19 left-aligned), names get the room up to the time.
// Build 4 (user requests 2026-10-02): STAT column (LBL_EFX_Sxx, dialogfont10x10) between name and time with what the
// effect does, decoded from the live effect ints (decodeEffect, research R3 + R3b), short forms joined with ", " and cut
// with "..."; long names/stats marquee after marquee_delay_ms without input; hover tooltips (name + time, every stat line,
// the spells.2da description). GUI = 82 controls (S block inserted before MORE: ids of MORE..TIP moved by 14).
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
       T_S0 = T_T0 + MaxRows, T_MORE = T_S0 + MaxRows, T_SBTRK, T_SBTHM, T_SBUP, T_SBDN, T_TIPBG, T_TIPT, T_TIP, NumTags };
static_assert(NumTags == 82, "tag count must match mirror_character_page.NEW_TAGS");

// ---- layout (GUI units, 800x600; = scripts/character_page_mockup.py) ----
constexpr float PanelX = 536, PanelY = 134, PanelW = 216, PanelH = 336;
constexpr float PadX = 8, PadTop = 8, RowH = 24, SecH = 20, SecPad = 4, SecGap = 8, IconW = 10, IconH = 18;
constexpr float SecOut = 4, TimePadR = 2, NameGap = 4;   // band overhang left of the rows; time inset from the row's right; name-time gap
// rendered dialogfont16x16 width = texels * sy * TextK (screenshot 19:09: 1.98 px/texel at sy 2.6667, all three names)
constexpr float TextK = 0.75f;
// dialogfont10x10 (stat column, tooltip body): same 0.01 world units per texel as 16x16 (TXI texturewidth 2.56 on a
// 256 texel sheet, research R3b R-c), so the same factor; line pitch = kFont10LineH texels (calibrate live, tip_line_scale)
constexpr float TextK10 = 0.75f;
constexpr float StatGap = 4, NameMinW = 40;   // stat-time gap; the name keeps at least this much when the stat is wide
constexpr float TipInX = 8, TipTitleY = 6, TipTitleH = 20, TipBodyY = 28, TipPadB = 8;   // tooltip insets (= mirror script)
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
    int enabled = 1, stage = 1, pollMs = 250, showPermanent = 1, wheelScroll = 1, tooltips = 1;
    int statColumn = 1, marquee = 1, marqueeDelayMs = 2500, marqueeStepMs = 160, marqueeHoldMs = 1200;
    int tooltipDelayMs = 350, tooltipDesc = 1;
    float statMaxW = 70, tipLineScale = 1.0f;
    int skipEquipped = 1, iconSource = 1, colorTimes = 1, discs = 1, soonS = 30;
    int fakeRows = 0;   // debug aid: N synthetic entries appended to the real list (overflow / scrollbar / sort tests)
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
        {"screen_w", nullptr, &c.screenW}, {"screen_h", nullptr, &c.screenH}, {"fake_rows", &c.fakeRows, nullptr},
        {"stat_column", &c.statColumn, nullptr}, {"stat_max_w", nullptr, &c.statMaxW}, {"marquee", &c.marquee, nullptr},
        {"marquee_delay_ms", &c.marqueeDelayMs, nullptr}, {"marquee_step_ms", &c.marqueeStepMs, nullptr},
        {"marquee_hold_ms", &c.marqueeHoldMs, nullptr}, {"tooltip_delay_ms", &c.tooltipDelayMs, nullptr},
        {"tooltip_desc", &c.tooltipDesc, nullptr}, {"tip_line_scale", nullptr, &c.tipLineScale}};
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
    if (c.fakeRows < 0) c.fakeRows = 0;
    if (c.fakeRows > 60) c.fakeRows = 60;
    if (c.textScale < 0.5f || c.textScale > 2.0f) c.textScale = 1.0f;
    if (c.statMaxW < 10 || c.statMaxW > 150) c.statMaxW = 70;
    if (c.marqueeDelayMs < 200) c.marqueeDelayMs = 200;
    if (c.marqueeStepMs < 30) c.marqueeStepMs = 30;
    if (c.marqueeHoldMs < 0) c.marqueeHoldMs = 0;
    if (c.tooltipDelayMs < 0) c.tooltipDelayMs = 0;
    if (c.tipLineScale < 0.5f || c.tipLineScale > 2.0f) c.tipLineScale = 1.0f;
    if (c.screenW < 100 || c.screenH < 100) { c.screenW = 3840; c.screenH = 1600; c.screenExplicit = false; }
    g_cfg = c; ++g_cfgGen;
    logf("INI loaded: %d keys ok, %d rejected, debug=%d gen=%u enabled=%d stage=%d poll_ms=%d fake_rows=%d stat=%d/%.0f marquee=%d (%d/%d/%d) tooltips=%d (%d ms, desc=%d)",
         ok, bad, g_debug, g_cfgGen, c.enabled, c.stage, c.pollMs, c.fakeRows, c.statColumn, c.statMaxW, c.marquee,
         c.marqueeDelayMs, c.marqueeStepMs, c.marqueeHoldMs, c.tooltips, c.tooltipDelayMs, c.tooltipDesc);
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
constexpr int MaxLines = 8, LineLen = 96;   // long lines are word-wrapped in the tooltip, never cut
struct Entry {
    bool section, harmful, gap, item, perm;
    int key;              // dedup key: 0x10000 + effecticon row, or spells.2da row
    int spell;            // spells.2da row of the effect that made the entry (-1 none): stat sibling match + description
    double remMs;         // < 0 = unknown
    char name[64]; char icon[17];
    char stat[96];        // stat column: short forms joined with ", " (cut with "..." at layout time)
    char lines[MaxLines][LineLen]; int nLines;   // tooltip: one long form per stat
    const char* desc;     // tooltip description (kSpellDesc) or a fixed text, nullptr = none
};
constexpr int MaxEnt = 96;
struct Slot {
    int panel = 0; bool bound = false; unsigned seq = 0; DWORD bindTick = 0;
    unsigned char* ctl[NumTags] = {};
    char text[NumTags][100] = {}; char tipText[1024] = {}; void* fill[NumTags] = {}; int col[NumTags] = {}; int rect[NumTags][4] = {};
    float bx = 0, by = 0, sx = 0, sy = 0;     // unit -> px from the bound LBL_EFX_BACK extent
    bool canTextColor = false, canBorderColor = false;
    int client = 0, server = 0; unsigned sig = 0; DWORD lastPoll = 0, lastBeat = 0;
    Entry ent[MaxEnt]; int nEnt = 0; int first = 0;
    // input
    bool wasL = false, dragging = false; float dragOff = 0; int heldPart = 0; DWORD heldNext = 0;
    // last layout (engine px) for hit tests
    bool over = false; int shownN = 0, lastFirst = 0; float rUp[4] = {}, rDn[4] = {}, rTrk[4] = {}, rThm[4] = {};
    unsigned layKey = 0;   // LAYOUT log: (list sig, first, over, count, cfg gen) of the last logged layout
    // placed rows (units) for the hover hit test: y, entry index; rowW of the last layout
    int nPlaced = 0; float rowY[MaxRows] = {}; int rowEnt[MaxRows] = {}; float lastRowW = 0;
    // marquee: input tracking + one shared offset (characters dropped from the front of every cut text)
    float lastCx = -1, lastCy = -1; DWORD lastInputMs = 0; bool mqActive = false; int mqOff = 0, mqMax = 0, mqPhase = 0;
    DWORD mqNext = 0; int mqRows = 0;   // mqPhase 0 scrolling, 1 hold at the end, 2 hold at the start; mqMax/mqRows from layout
    // tooltip
    int hoverKey = -1, hoverRow = -1; DWORD hoverSince = 0; bool tipShown = false; int tipKey = -1;
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
    char* cache = t == T_TIP ? s->tipText : s->text[t]; int cap = t == T_TIP ? (int)sizeof s->tipText : (int)sizeof s->text[t];
    unsigned char* c = s->ctl[t]; if (!c || !strcmp(cache, txt)) return;
    if (!textObjOk(c)) { whyLog(100 + t, "label %d: text object bad, cannot set \"%s\"", t, txt); return; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(exo, txt);
    reinterpret_cast<void(__thiscall*)(void*, void*)>(FnTextSet)(c + LabelTextOff + TextStringSubOff, exo);
    reinterpret_cast<CExoDtor>(FnExoDtor)(exo);
    dlogf("label %d: text \"%.80s\" -> \"%.80s\"", t, cache, txt);
    strncpy(cache, txt, cap - 1); cache[cap - 1] = 0;
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
// ---- stat decoder (research R3 + R3b): effect type + ints[] -> short form (stat column) + long form (tooltip line) ----
// Decrease types store positive magnitudes; the sign comes from the type. Effects of one entry that differ only in the
// damage type (e.g. Energy Resistance = 5 DamageResistance effects) merge into one atom with the OR of the type masks.
enum AtomKind { A_NONE, A_ABIL, A_AC, A_ATT, A_DMG, A_RES, A_DR, A_IMMD, A_SAVE, A_SKILL, A_THP, A_REGEN, A_SPEED, A_IMM,
                A_FR, A_DEFL, A_FSHIELD, A_STATE, A_TFP, A_CONCEAL, A_DSHIELD, A_POISON, A_INVIS, A_ATTACKS, A_FPREG, A_VPREG };
struct Atom { int kind, a, b, mask, limit; };
const char* const kAbilShort[6] = {"Str", "Dex", "Con", "Int", "Wis", "Cha"};
const char* const kAbilLong[6] = {"Strength", "Dexterity", "Constitution", "Intelligence", "Wisdom", "Charisma"};
const char* const kSkillShort[8] = {"Comp", "Demo", "Stealth", "Aware", "Persuade", "Repair", "Security", "Treat Inj"};
const char* const kSkillLong[8] = {"Computer Use", "Demolitions", "Stealth", "Awareness", "Persuade", "Repair", "Security", "Treat Injury"};
// damage flag bit i (nwscript DAMAGE_TYPE_*, = iprp_damagetype row i): 4096 is the in-game "Energy" (blaster)
const char* const kDmgShort[13] = {"Bludg", "Pierce", "Slash", "Univ", "Acid", "Cold", "LS", "Elec", "Fire", "DS", "Sonic", "Ion", "Energy"};
const char* const kDmgLong[13] = {"Bludgeoning", "Piercing", "Slashing", "Universal", "Acid", "Cold", "Light Side", "Electrical",
                                  "Fire", "Dark Side", "Sonic", "Ion", "Energy"};
const char* const kSaveShort[4] = {"Saves", "Fort", "Ref", "Will"};
const char* const kSaveLong[4] = {"All saves", "Fortitude", "Reflex", "Will"};
const char* const kSaveType[19] = {nullptr, "acid", "sneak attack", "cold", "death", "disease", "Light Side", "electrical", "fear",
                                   "fire", "mind-affecting", "Dark Side", "poison", "sonic", "traps", "Force powers", "ion",
                                   "blaster", "paralysis"};
const char* const kImmunity[34] = {"none", "Mind", "Poison", "Disease", "Fear", "Traps", "Paralysis", "Blindness", "Deafness",
                                   "Slow", "Entangle", "Silence", "Stun", "Sleep", "Charm", "Dominate", "Confusion", "Curse",
                                   "Daze", "Ability loss", "Attack loss", "Damage loss", "Immunity loss", "Defense loss",
                                   "Slow", "Save loss", "FR loss", "Skill loss", "Knockdown", "Neg. level", "Sneak attack",
                                   "Critical hits", "Death", "Droid confusion"};
const char* const kAcType[5] = {"dodge", "natural", "armour", "shield", "deflection"};
const char* stateName(int st) {
    switch (st) {
    case 1: return "Confused"; case 2: return "Frightened"; case 3: return "Droid stun"; case 4: return "Stunned";
    case 5: return "Paralyzed"; case 6: return "Asleep"; case 7: return "Choked"; case 8: return "Horrified";
    case 10: return "Whirlwind"; case 15: return "Crushed"; case 16: return "Droid confused"; case 18: return "Mind trick";
    case 19: return "Droid scrambled"; default: return nullptr;
    }
}
// damage-type mask: one name, a known group, two names, or "N types"; long = every name
void maskName(int mask, bool lng, char* out, int n) {
    mask &= 0x1FFF;
    int bits = 0; for (int i = 0; i < 13; ++i) bits += (mask >> i) & 1;
    out[0] = 0;
    if (!bits) { snprintf(out, n, "%s", lng ? "no" : "?"); return; }
    if (!lng) {
        if (mask == 0x1FFF) { snprintf(out, n, "All"); return; }
        if (mask == 7) { snprintf(out, n, "Phys"); return; }
        if (mask == (4096 | 128)) { snprintf(out, n, "Energy"); return; }               // blaster + electrical (energy shields)
        if (mask == (4096 | 128 | 32 | 256 | 1024)) { snprintf(out, n, "Elem"); return; }   // Energy Resistance: + cold, fire, sonic
        if (bits > 2) { snprintf(out, n, "%d types", bits); return; }   // "<first> +N" read like a bonus
    } else if (mask == 0x1FFF) { snprintf(out, n, "all"); return; }
    int k = 0, len = 0;
    for (int i = 0; i < 13; ++i) {
        if (!((mask >> i) & 1)) continue;
        int w = snprintf(out + len, n - len, "%s%s", k ? (lng ? ", " : "/") : "", lng ? kDmgLong[i] : kDmgShort[i]);
        if (w < 0 || w >= n - len) return;
        len += w; ++k;
    }
}
bool atomOf(int type, const int* v, int n, Atom* at) {
    auto I = [&](int i) { return i < n ? v[i] : 0; };
    Atom x = {A_NONE, 0, 0, 0, 0};
    int sg = 1;
    switch (type) {
    case 0x25: sg = -1; /* fallthrough */
    case 0x24: if (I(0) < 0 || I(0) > 5) return false; x = {A_ABIL, sg * I(1), I(0), 0, 0}; break;
    case 0x31: sg = -1; /* fallthrough */
    case 0x30: x = {A_AC, sg * I(1), I(0), 0, 0}; break;
    case 0x0b: sg = -1; /* fallthrough */
    case 0x0a: x = {A_ATT, sg * I(0), I(1), 0, 0}; break;
    case 0x0e: sg = -1; /* fallthrough */
    case 0x0d: x = {A_DMG, sg * I(0), 0, I(1), 0}; break;   // a = DAMAGE_BONUS_* enum 1..10
    case 0x02: x = {A_RES, I(1), 0, I(0), I(2)}; break;
    case 0x0c: x = {A_DR, I(0), I(1), 0, I(2)}; break;
    case 0x11: sg = -1; /* fallthrough */
    case 0x10: x = {A_IMMD, sg * I(1), 0, I(0), 0}; break;
    case 0x1b: sg = -1; /* fallthrough */
    case 0x1a: x = {A_SAVE, sg * I(0), I(1), 0, I(2)}; break;   // limit = save type (0 all)
    case 0x38: sg = -1; /* fallthrough */
    case 0x37: if (I(0) < 0 || I(0) > 7) return false; x = {A_SKILL, sg * I(1), I(0), 0, 0}; break;
    case 0x0f: x = {A_THP, I(0), 0, 0, 0}; break;
    case 0x07: x = {A_REGEN, I(0), I(1), 0, 0}; break;
    case 0x1d: sg = -1; /* fallthrough */
    case 0x1c: x = {A_SPEED, sg * I(0), 0, 0, 0}; break;
    case 0x16: x = {A_IMM, 0, I(0), 0, 0}; break;
    case 0x22: sg = -1; /* fallthrough */
    case 0x21: x = {A_FR, sg * I(0), 0, 0, 0}; break;
    case 0x5d: sg = -1; /* fallthrough */
    case 0x5c: x = {A_DEFL, sg * I(0), 0, 0, 0}; break;
    case 0x6b: if (I(0) < 0 || I(0) >= kForceShieldCount) return false; x = {A_FSHIELD, kForceShield[I(0)].amount, I(0), kForceShield[I(0)].flags, 0}; break;
    case 0x08: if (!stateName(I(0))) return false; x = {A_STATE, 0, I(0), 0, 0}; break;
    case 0x5b: x = {A_TFP, I(0), 0, 0, 0}; break;
    case 0x4c: x = {A_CONCEAL, I(0), 0, 0, 0}; break;
    case 0x3d: x = {A_DSHIELD, I(0), I(1), I(2), 0}; break;
    case 0x23: x = {A_POISON, 0, I(0), 0, 0}; break;
    case 0x2f: x = {A_INVIS, 0, I(0), 0, 0}; break;
    case 0x2c: x = {A_ATTACKS, I(0), 0, 0, 0}; break;
    case 0x70: x = {A_FPREG, I(0), 0, 0, 0}; break;
    case 0x71: x = {A_VPREG, I(0), 0, 0, 0}; break;
    default: return false;   // 0x1e visual, 0x43 icon, 0x44 innate, link, heal/damage instants, ...: nothing to show
    }
    *at = x; return true;
}
const char* dmgBonus(int e, char* buf, int n) {   // DAMAGE_BONUS_* 1..5 = +1..+5, 6..10 = 1d4 1d6 1d8 1d10 2d6
    static const char* const dice[5] = {"1d4", "1d6", "1d8", "1d10", "2d6"};
    int m = e < 0 ? -e : e;
    if (m >= 6 && m <= 10) snprintf(buf, n, "%c%s", e < 0 ? '-' : '+', dice[m - 6]); else snprintf(buf, n, "%+d", e);
    return buf;
}
void fmtAtom(const Atom& x, char* sh, int shn, char* lg, int lgn) {
    char m[64], m2[160], d[16];
    sh[0] = lg[0] = 0;
    switch (x.kind) {
    case A_ABIL: snprintf(sh, shn, "%+d %s", x.a, kAbilShort[x.b]); snprintf(lg, lgn, "%+d %s", x.a, kAbilLong[x.b]); break;
    case A_AC: snprintf(sh, shn, "Def %+d", x.a); snprintf(lg, lgn, "Defense %+d%s%s%s", x.a, x.b > 0 && x.b < 5 ? " (" : "", x.b > 0 && x.b < 5 ? kAcType[x.b] : "", x.b > 0 && x.b < 5 ? ")" : ""); break;
    case A_ATT: snprintf(sh, shn, "Att %+d", x.a); snprintf(lg, lgn, "Attack %+d%s", x.a, x.b == 1 ? " (main hand)" : x.b == 2 ? " (off hand)" : ""); break;
    case A_DMG: maskName(x.mask, true, m2, sizeof m2); dmgBonus(x.a, d, sizeof d);
        snprintf(sh, shn, "Dmg %s", d); snprintf(lg, lgn, "Damage %s %s", d, m2); break;
    case A_RES: maskName(x.mask, false, m, sizeof m); maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Res %d %s", x.a, m);
        if (x.limit > 0) snprintf(lg, lgn, "Resist %d %s (up to %d)", x.a, m2, x.limit); else snprintf(lg, lgn, "Resist %d %s", x.a, m2); break;
    case A_DR: snprintf(sh, shn, "DR %d", x.a);
        if (x.limit > 0) snprintf(lg, lgn, "Damage reduction %d (up to %d)", x.a, x.limit); else snprintf(lg, lgn, "Damage reduction %d", x.a); break;
    case A_IMMD: maskName(x.mask, false, m, sizeof m); maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Imm %d%% %s", x.a, m); snprintf(lg, lgn, "%+d%% immunity: %s", x.a, m2); break;
    case A_SAVE: { int w = x.b >= 0 && x.b < 4 ? x.b : 0; const char* vs = x.limit > 0 && x.limit < 19 ? kSaveType[x.limit] : nullptr;
        snprintf(sh, shn, "%s %+d", kSaveShort[w], x.a);
        snprintf(lg, lgn, "%s %+d%s%s", kSaveLong[w], x.a, vs ? " vs " : "", vs ? vs : ""); break; }
    case A_SKILL: snprintf(sh, shn, "%s %+d", kSkillShort[x.b], x.a); snprintf(lg, lgn, "%s %+d", kSkillLong[x.b], x.a); break;
    case A_THP: snprintf(sh, shn, "+%d VP", x.a); snprintf(lg, lgn, "+%d temporary Vitality", x.a); break;
    case A_REGEN: { int sec = x.b > 0 ? (x.b + 500) / 1000 : 0;
        snprintf(sh, shn, "Regen %d/%ds", x.a, sec); snprintf(lg, lgn, "Regenerates %d Vitality every %d s", x.a, sec); break; }
    case A_SPEED: snprintf(sh, shn, "Speed %+d%%", x.a); snprintf(lg, lgn, "Movement speed %+d%%", x.a); break;
    case A_IMM: { const char* nm = x.b >= 0 && x.b < 34 ? kImmunity[x.b] : "?";
        snprintf(sh, shn, "Imm: %s", nm); snprintf(lg, lgn, "Immune: %s", nm); break; }
    case A_FR: snprintf(sh, shn, "FR %+d", x.a); snprintf(lg, lgn, "Force resistance %+d", x.a); break;
    case A_DEFL: snprintf(sh, shn, "Defl %+d", x.a); snprintf(lg, lgn, "Blaster deflection %+d", x.a); break;
    case A_FSHIELD: maskName(x.mask, false, m, sizeof m); maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Shield %d %s", x.a, m); snprintf(lg, lgn, "Absorbs %d: %s", x.a, m2); break;
    case A_STATE: snprintf(sh, shn, "%s", stateName(x.b)); snprintf(lg, lgn, "%s", stateName(x.b)); break;
    case A_TFP: snprintf(sh, shn, "+%d FP", x.a); snprintf(lg, lgn, "+%d temporary Force points", x.a); break;
    case A_CONCEAL: snprintf(sh, shn, "Conceal %d%%", x.a); snprintf(lg, lgn, "%d%% concealment", x.a); break;
    case A_DSHIELD: maskName(x.mask, true, m2, sizeof m2);
        snprintf(sh, shn, "Dmg shield %d", x.a); snprintf(lg, lgn, "Damage shield %d %s", x.a, m2); break;
    case A_POISON: snprintf(sh, shn, "Poisoned"); snprintf(lg, lgn, "Poisoned"); break;
    case A_INVIS: snprintf(sh, shn, "Invisible"); snprintf(lg, lgn, "Invisible"); break;
    case A_ATTACKS: snprintf(sh, shn, "%+d attacks", x.a); snprintf(lg, lgn, "%+d attack(s) per round", x.a); break;
    case A_FPREG: snprintf(sh, shn, "FP regen %+d%%", x.a); snprintf(lg, lgn, "Force point regeneration %+d%%", x.a); break;
    case A_VPREG: snprintf(sh, shn, "VP regen %+d%%", x.a); snprintf(lg, lgn, "Vitality regeneration %+d%%", x.a); break;
    default: break;
    }
}
// per entry: atoms in effect-array order, merged by (kind, a, b, limit) with OR-ed masks; formatted when collect ends
constexpr int MaxAtoms = 12;
struct AtomSet { Atom at[MaxAtoms]; int n; };
void addAtom(AtomSet& set, const Atom& x) {
    for (int i = 0; i < set.n; ++i) {
        Atom& y = set.at[i];
        if (y.kind == x.kind && y.a == x.a && y.b == x.b && y.limit == x.limit) { y.mask |= x.mask; return; }
    }
    if (set.n < MaxAtoms) set.at[set.n++] = x;
}
void formatStats(Entry& e, const AtomSet& set) {
    bool shield = false; for (int i = 0; i < set.n; ++i) shield |= set.at[i].kind == A_FSHIELD;
    e.stat[0] = 0; e.nLines = 0; int len = 0;
    for (int i = 0; i < set.n; ++i) {
        const Atom& x = set.at[i];
        if (shield && x.kind == A_RES && x.limit > 0) continue;   // a force shield's own absorb pieces: the Shield line says it
        char sh[48], lg[LineLen];
        fmtAtom(x, sh, sizeof sh, lg, sizeof lg);
        if (!sh[0]) continue;
        bool dup = false; for (int k = 0; k < e.nLines; ++k) if (!strcmp(e.lines[k], lg)) { dup = true; break; }
        if (dup) continue;
        if (e.nLines < MaxLines) { snprintf(e.lines[e.nLines], LineLen, "%s", lg); ++e.nLines; }
        int w = snprintf(e.stat + len, sizeof e.stat - len, "%s%s", len ? ", " : "", sh);
        if (w > 0 && w < (int)sizeof e.stat - len) len += w; else { e.stat[len] = 0; }
    }
}
bool readInts(int eff, int* v, int* n) {   // ints[] = *(eff+0x34), count *(eff+0x30), first 8
    int cnt = 0, ip = 0; *n = 0;
    if (!readAt<int>(eff, EffNIntsOff, &cnt) || cnt <= 0 || !readAt<int>(eff, EffIntsOff, &ip) || !ip) return false;
    if (cnt > 8) cnt = 8;
    for (int i = 0; i < cnt; ++i) if (!readAt<int>(ip, 4 * i, &v[i])) return false;
    *n = cnt; return true;
}
void fmtInts(const int* v, int n, char* out, int cap) {
    int len = snprintf(out, cap, "[");
    for (int i = 0; i < n && len < cap - 1; ++i) { int w = snprintf(out + len, cap - len, "%s%d", i ? "," : "", v[i]); if (w > 0) len += w; }
    if (len < cap - 1) snprintf(out + len, cap - len, "]");
}
// DECODE log: once per (type, ints) signature (+ once per unknown type id), capped per session
unsigned g_decSeen[256]; int g_decSeenN = 0, g_decLogs = 0; bool g_unkSeen[256];
void logDecode(int type, const int* v, int n) {
    if (g_decLogs >= 200) return;
    char iv[100]; fmtInts(v, n, iv, sizeof iv);
    Atom a;
    if (!atomOf(type, v, n, &a)) {
        if (type >= 0 && type < 256 && !g_unkSeen[type]) { g_unkSeen[type] = true; ++g_decLogs; logf("DECODE: type=0x%02x ints=%s -> (no stat shown)", type, iv); }
        return;
    }
    unsigned h = 2166136261u; h = (h ^ (unsigned)type) * 16777619u;
    for (int i = 0; i < n; ++i) h = (h ^ (unsigned)v[i]) * 16777619u;
    for (int i = 0; i < g_decSeenN; ++i) if (g_decSeen[i] == h) return;
    if (g_decSeenN < 256) g_decSeen[g_decSeenN++] = h;
    char sh[48], lg[LineLen]; fmtAtom(a, sh, sizeof sh, lg, sizeof lg);
    ++g_decLogs; logf("DECODE: type=0x%02x ints=%s -> '%s' / '%s'", type, iv, sh, lg);
}
// fake_rows test aid: alternating harmful/beneficial, growing names, 5 s .. 2 h plus one permanent, every third an item;
// times are frozen. Entry 4 uses a resref with no texture to show what FnLoadImage returns for a missing image.
int addFakeRows(Entry* out, int nt, int n) {
    static const char* const tails[] = {"", " short", " with a long name", " with a much longer name that will not fit"};
    static const double secs[] = {5, 20, 45, 90, 300, 900, 3600, 7200};
    int row = 0;
    for (int i = 0; i < n && nt < MaxEnt - 2; ++i) {
        Entry e = {};
        e.key = 0x20000 + i; e.harmful = i % 2 == 0; e.item = i % 3 == 2;
        snprintf(e.name, sizeof e.name, "Test effect %02d%s", i + 1, tails[i % 4]);
        e.perm = n > 1 && i == n - 1; e.remMs = e.perm ? -1 : secs[i % 8] * 1000.0 + i * 1000.0;
        for (int tries = 0; tries < kEffIconRowCount; ++tries) { row = row % (kEffIconRowCount - 1) + 1; if (kEffIconRows[row].icon) break; }
        snprintf(e.icon, sizeof e.icon, "%s", i == 4 ? "chfx_no_texture" : (kEffIconRows[row].icon ? kEffIconRows[row].icon : ""));
        // stat column tests: a long joined string (cut / marquee), a short one, none
        static const int16_t fakeInts[][4] = {{0x24, 1, 2, 0}, {0x02, 4096, 20, 0}, {0x16, 2, 0, 0}, {0x30, 0, 2, 0}, {0x1c, 50, 0, 0}};
        AtomSet set = {}; e.spell = -1;
        if (i % 3 == 0) for (auto& fi : fakeInts) { int v[3] = {fi[1], fi[2], fi[3]}; Atom a; if (atomOf(fi[0], v, 3, &a)) addAtom(set, a); }
        else if (i % 3 == 1) { int v[2] = {2 + i % 4, 0}; Atom a; if (atomOf(0x0a, v, 2, &a)) addAtom(set, a); }
        formatStats(e, set);
        e.desc = "Test entry";
        out[nt++] = e;
    }
    return nt;
}
// Collect + classify + dedup + sort; returns a signature of the structure (keys/order) for change logging.
int collect(Slot* s, int server, Entry* out, bool dump) {
    int arr = 0, count = 0;
    if (!readAt<int>(server, EffArrOff, &arr) || !readAt<int>(server, EffCntOff, &count) || count < 0 || count > MaxEffects) return -1;
    if ((!arr || !count) && !g_cfg.fakeRows) return 0;
    if (!arr) count = 0;
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
    static Entry tmp[MaxEnt]; int nt = 0;   // ~70 KB: static, not on the engine thread's stack
    static int rawKey[MaxEffects];
    for (int i = 0; i < nr; ++i) {
        const Raw& r = raw[i];
        rawKey[i] = -1;
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
                e.key = 0x10000 + r.iconRow; e.spell = r.spell; e.harmful = harm; strncpy(e.name, row.name, sizeof e.name - 1);
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
                e.key = r.spell; e.spell = r.spell; e.harmful = bad != nullptr;
                const char* nm = kSpellRows[r.spell].name;
                if (nm) strncpy(e.name, nm, sizeof e.name - 1); else snprintf(e.name, sizeof e.name, "Effect %d", r.spell);
                strncpy(e.icon, bad ? bad : good, 16);
                e.item = bad ? kDebuffIcons[r.spell].item : isItemAbility(r.spell);
                use = true;
            }
        }
        if (use && r.perm && !g_cfg.showPermanent) { use = false; why = "permanent hidden"; }
        char iv[100] = "[]"; if (dump) { int v[8], n = 0; if (readInts(r.eff, v, &n)) fmtInts(v, n, iv, sizeof iv); }
        if (dump) logf("  eff[%d] %08x type=0x%02x sub=0x%04x spell=%d iconRow=%d ints=%s dur=%.1f %s rem=%.1fs -> %s%s%s%s",
                       i, r.eff, r.type, r.sub, r.spell, r.iconRow, iv, r.dur, r.perm ? "PERM" : "temp", r.rem / 1000.0,
                       use ? (e.harmful ? "HARMFUL '" : "BENEFICIAL '") : "skip: ", use ? e.name : why, use ? "'" : "",
                       (r.type == EffTypeIcon && r.iconRow > 0 && r.iconRow < kEffIconRowCount) ? (kEffIconRows[r.iconRow].good == 1 ? " [icon good=1]" : kEffIconRows[r.iconRow].good == 0 ? " [icon good=0]" : " [icon good=-]") : "");
        if (!use) continue;
        rawKey[i] = e.key;
        e.perm = r.perm; e.remMs = r.rem;
        int k = 0; for (; k < nt; ++k) if (tmp[k].key == e.key) break;
        if (k < nt) {   // keep the longest remaining (permanent beats everything)
            bool longer = (e.perm && !tmp[k].perm) || (!tmp[k].perm && !e.perm && e.remMs > tmp[k].remMs);
            if (longer) { tmp[k].perm = e.perm; tmp[k].remMs = e.remMs; }
            continue;
        }
        if (nt < MaxEnt - 2) tmp[nt++] = e;
    }
    // stats: every non-equipped effect with a spell joins the entry of its spell (icon entries carry the spell of their
    // EffectIcon effect); a spell merged into another entry by icon row is found through a used raw effect of that spell
    static AtomSet sets[MaxEnt];
    for (int k = 0; k < nt; ++k) sets[k].n = 0;
    for (int j = 0; j < nr; ++j) {
        const Raw& r = raw[j];
        if (r.spell < 0 || (g_cfg.skipEquipped && (r.sub & 7) == 3) || r.type == EffTypeIcon) continue;
        int v[8], n = 0; if (!readInts(r.eff, v, &n)) continue;
        if (dump) logDecode(r.type, v, n);
        Atom a; if (!atomOf(r.type, v, n, &a)) continue;
        int k = 0; for (; k < nt; ++k) if (tmp[k].spell == r.spell) break;
        if (k == nt) {
            int key = -1; for (int i = 0; i < nr && key < 0; ++i) if (rawKey[i] >= 0 && raw[i].spell == r.spell) key = rawKey[i];
            for (k = 0; k < nt; ++k) if (key >= 0 && tmp[k].key == key) break;
        }
        if (k < nt) addAtom(sets[k], a);
    }
    for (int k = 0; k < nt; ++k) {
        formatStats(tmp[k], sets[k]);
        const char* d = tmp[k].spell >= 0 && tmp[k].spell < kSpellRowCount ? kSpellDesc[tmp[k].spell] : nullptr;
        tmp[k].desc = d && d[0] ? d : nullptr;
    }
    nt = addFakeRows(tmp, nt, g_cfg.fakeRows);
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
        for (int i = 0; i < nt; ++i) if (tmp[i].harmful == harm) {
            out[n++] = tmp[i]; sig = (sig ^ (unsigned)tmp[i].key) * 16777619u;
            for (const char* p = tmp[i].stat; *p; ++p) sig = (sig ^ (unsigned char)*p) * 16777619u;   // a stat change relogs
        }
        sig = (sig ^ (harm ? 0xB0u : 0x60u)) * 16777619u;
    }
    s->sig = sig;
    return n;
}

// ---- whole-entry scrolling (= character_page_mockup.fits / scroll_state) ----
float entryH(const Entry& e, bool atTop) { return e.section ? SecH + SecPad + ((e.gap && !atTop) ? SecGap : 0) : RowH; }
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

// text width in engine px (kFontTexW / kFont10TexW from gen_effect_name_table.py; TextK measured; text_scale = INI fine-tune)
enum Font { F16, F10 };
float textPx(Slot* s, const char* t, int len, Font f = F16) {
    const unsigned char* tw = f == F10 ? kFont10TexW : kFontTexW;
    float w = 0; for (int i = 0; i < len; ++i) w += tw[(unsigned char)t[i]];
    return w * s->sy * (f == F10 ? TextK10 : TextK) * g_cfg.textScale;
}
// fitted text: cut with "..." to the label width; returns true when it had to cut
bool fitText(Slot* s, const char* name, float maxPx, char* out, int n, Font f = F16) {
    int len = (int)strlen(name);
    if (textPx(s, name, len, f) <= maxPx) { snprintf(out, n, "%s", name); return false; }
    float dots = textPx(s, "...", 3, f);
    while (len > 0 && textPx(s, name, len, f) + dots > maxPx) --len;
    while (len > 0 && name[len - 1] == ' ') --len;
    snprintf(out, n, "%.*s...", len, name);
    return true;
}
// marquee: the cut text with `off` characters dropped from the front (no leading "..."; a trailing one while more
// remains); *maxOff = the offset at which the rest fits (0 when the whole text fits)
void marqueeText(Slot* s, const char* full, float maxPx, int off, char* out, int n, int* maxOff, Font f = F16) {
    int len = (int)strlen(full), m = 0;
    while (m < len && textPx(s, full + m, len - m, f) > maxPx) ++m;
    *maxOff = m;
    if (off > m) off = m;
    while (off > 0 && off < len && full[off] == ' ') ++off;   // never start on a space
    fitText(s, full + off, maxPx, out, n, f);
}

// one LAYOUT line per change of (list, first, overflow, INI generation): scroll state, px widths, fitted names and times
void logLayout(Slot* s, int cnt, int lf, bool over, float rowW, const char* rows) {
    unsigned key = 2166136261u;
    const unsigned parts[] = {s->sig, (unsigned)s->first, (unsigned)over, (unsigned)s->nEnt, g_cfgGen};
    for (unsigned v : parts) key = (key ^ v) * 16777619u;
    if (key == s->layKey) return;
    s->layKey = key;
    logf("LAYOUT: n=%d first=%d cnt=%d lastFirst=%d over=%d rowW=%dpx thumb=[%.0f,%.0f,%.0f,%.0f] track=[%.0f,%.0f,%.0f,%.0f] rows:%s",
         s->nEnt, s->first, cnt, lf, over, ux(s, X0 + rowW) - ux(s, X0), s->rThm[0], s->rThm[1], s->rThm[2], s->rThm[3],
         s->rTrk[0], s->rTrk[1], s->rTrk[2], s->rTrk[3], rows[0] ? rows : " (none)");
}

void hideAll(Slot* s) { for (int t = 0; t < NumTags; ++t) hide(s, t); s->over = false; s->tipShown = false; s->nPlaced = 0; g_panelRect[2] = 0; }

void layout(Slot* s) {
    const Entry* e = s->ent; int n = s->nEnt;
    if (g_cfg.stage < 1 || n == 0) {
        for (int t = T_TIPBG; t <= T_TIP; ++t) hide(s, t);
        s->nPlaced = 0; s->tipShown = false; s->mqMax = 0; s->mqRows = 0;
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
    float y = Top; int r = 0; bool secShown[2] = {false, false}; int mqMax = 0, mqRows = 0;
    char rows[2048] = ""; int rl = 0;
    auto addRow = [&](const char* fmt, ...) {
        if (rl >= (int)sizeof rows - 1) return;
        va_list ap; va_start(ap, fmt); int w = vsnprintf(rows + rl, sizeof rows - rl, fmt, ap); va_end(ap);
        if (w > 0) rl += w;
    };
    for (int k = 0; k < cnt; ++k) {
        const Entry& x = e[s->first + k];
        if (x.section) {
            if (x.gap && k > 0) y += SecGap;
            int t = x.harmful ? T_SECB : T_SECG; secShown[x.harmful] = true;
            place(s, t, X0 - SecOut, y, rowW + SecOut + (over ? 0 : SecOut), SecH);   // no right overhang next to the scrollbar
            setText(s, t, x.harmful ? " HARMFUL" : " BENEFICIAL");   // leading space = title inset (the label draws flush left)
            addRow(" |%s%s|", x.harmful ? "HARMFUL" : "BENEFICIAL", (x.gap && k == 0) ? "(gap dropped)" : "");
            y += SecH + SecPad;
            continue;
        }
        if (r >= MaxRows) break;
        place(s, T_I0 + r, X0, y + (RowH - IconH) / 2, IconW, IconH);
        setFill(s, T_I0 + r, x.icon[0] ? x.icon : g_cfg.fallbackIcon, White);
        if (x.item && g_cfg.discs) { place(s, T_D0 + r, X0, y + (RowH - IconH) / 2, IconW, IconH); setFill(s, T_D0 + r, DiscResref, DiscTeal); }
        else hide(s, T_D0 + r);
        // time right-aligned by placement: label = text width + 1 unit slack, so left- and right-aligned drawing both end at tr
        char tm[16]; fmtTime(x, tm, sizeof tm);
        float tr = X0 + rowW - TimePadR, tx = tr - textPx(s, tm, (int)strlen(tm)) / s->sx - 1;
        place(s, T_T0 + r, tx, y, tr - tx, RowH);
        setText(s, T_T0 + r, tm);
        float nx = X0 + IconW + 4, nr = tx - NameGap;   // name right edge: the stat's left edge when there is one
        char st[64] = "";
        if (g_cfg.statColumn && x.stat[0]) {
            float cap = fminf(g_cfg.statMaxW, tx - StatGap - NameGap - NameMinW - nx);
            float sw = fminf(textPx(s, x.stat, (int)strlen(x.stat), F10) / s->sx + 1, cap);
            if (sw >= 12) {
                float sxl = tx - StatGap - sw;
                place(s, T_S0 + r, sxl, y, sw, RowH);
                int mo = 0; marqueeText(s, x.stat, (ux(s, sxl + sw) - ux(s, sxl)) * 0.98f, s->mqOff, st, sizeof st, &mo, F10);
                if (mo > mqMax) mqMax = mo;
                mqRows += mo > 0;
                setText(s, T_S0 + r, st);
                nr = sxl - NameGap;
            } else hide(s, T_S0 + r);
        } else hide(s, T_S0 + r);
        float nw = nr - nx;
        place(s, T_N0 + r, nx, y, nw, RowH);
        s->rowY[r] = y; s->rowEnt[r] = s->first + k;
        char nm[64]; int mo = 0;
        marqueeText(s, x.name, (ux(s, nx + nw) - ux(s, nx)) * 0.98f, s->mqOff, nm, sizeof nm, &mo);
        if (mo > mqMax) mqMax = mo;
        mqRows += mo > 0;
        setText(s, T_N0 + r, nm);
        addRow(" [%d] \"%s\" {%s} %s%s", r, nm, st, tm, x.item ? " disc" : "");
        bool warn = g_cfg.colorTimes && (x.harmful || (!x.perm && x.remMs >= 0 && x.remMs < g_cfg.soonS * 1000.0));
        setTextColor(s, T_T0 + r, warn ? 2 : 1, warn ? Orange : Cream);
        y += RowH; ++r;
    }
    s->nPlaced = r; s->lastRowW = rowW; s->mqMax = mqMax; s->mqRows = mqRows;
    for (; r < MaxRows; ++r) { hide(s, T_D0 + r); hide(s, T_I0 + r); hide(s, T_N0 + r); hide(s, T_T0 + r); hide(s, T_S0 + r); }
    if (!secShown[0]) hide(s, T_SECG);
    if (!secShown[1]) hide(s, T_SECB);
    s->over = over; s->shownN = cnt; s->lastFirst = lf;
    if (!over) {
        for (int t = T_SBTRK; t <= T_SBDN; ++t) hide(s, t);
        g_panelRect[2] = 0;
        memset(s->rUp, 0, sizeof s->rUp); memset(s->rDn, 0, sizeof s->rDn); memset(s->rTrk, 0, sizeof s->rTrk); memset(s->rThm, 0, sizeof s->rThm);
        logLayout(s, cnt, lf, over, rowW, rows);
        return;
    }
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
    logLayout(s, cnt, lf, over, rowW, rows);
}

bool inRect(const float* r, float x, float y) { return r[2] > 0 && x >= r[0] && x < r[0] + r[2] && y >= r[1] && y < r[1] + r[3]; }
int g_pressLogs = 0; constexpr int MaxPressLogs = 40;
// INPUT log: every left press inside the panel with the scrollbar part it hits (same order as the handler below)
void logPress(Slot* s, float cx, float cy, bool fg) {
    if (g_pressLogs >= MaxPressLogs) return;
    float pr[4] = {(float)ux(s, PanelX), (float)uy(s, PanelY), (float)(ux(s, PanelX + PanelW) - ux(s, PanelX)), (float)(uy(s, PanelY + PanelH) - uy(s, PanelY))};
    if (!inRect(pr, cx, cy)) return;
    const char* part = !s->over ? "none" : inRect(s->rUp, cx, cy) ? "up" : inRect(s->rDn, cx, cy) ? "down" : inRect(s->rThm, cx, cy) ? "thumb" : inRect(s->rTrk, cx, cy) ? "track" : "none";
    logf("INPUT: press #%d at (%.0f,%.0f) part=%s over=%d first=%d/%d fg=%d", ++g_pressLogs, cx, cy, part, s->over, s->first, s->lastFirst, fg);
}
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
    if (!s->over) {
        float px = 0, py = 0;
        if (l && !s->wasL && g_pressLogs < MaxPressLogs && cursorEngine(&px, &py)) logPress(s, px, py, GetForegroundWindow() == g_hwnd);
        s->dragging = false; s->heldPart = 0; s->wasL = l; return changed;
    }
    float cx = 0, cy = 0; bool haveCur = cursorEngine(&cx, &cy);
    bool fg = gameWindow() && GetForegroundWindow() == g_hwnd;
    if (l && !s->wasL && haveCur) logPress(s, cx, cy, fg);
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

// ---- idle tracking + marquee (names and stats that were cut scroll after marquee_delay_ms without input) ----
// input = cursor moved > 2 px, wheel, left button, scroll, list change; any input snaps every row back to the "..." form
bool trackInput(Slot* s, DWORD t, bool haveCur, float cx, float cy, bool other) {
    bool input = other || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (haveCur && (fabsf(cx - s->lastCx) > 2 || fabsf(cy - s->lastCy) > 2)) { input = true; s->lastCx = cx; s->lastCy = cy; }
    if (!input) return false;
    s->lastInputMs = t;
    if (!s->mqActive && !s->mqOff) return false;
    if (s->mqActive) logf("MARQUEE: reset (input)");
    s->mqActive = false; s->mqOff = 0; s->mqPhase = 0;
    return true;   // relayout with offset 0
}
// returns true when the offset changed (relayout). Cycle: scroll 1 char per step -> hold at the end -> snap back -> hold.
bool marqueeStep(Slot* s, DWORD t) {
    if (!g_cfg.marquee || g_cfg.stage < 1 || s->mqMax <= 0) {
        bool had = s->mqOff != 0;
        if (s->mqActive) logf("MARQUEE: stop (%s)", !g_cfg.marquee ? "marquee=0" : "nothing cut");
        s->mqActive = false; s->mqOff = 0; s->mqPhase = 0;
        return had;
    }
    if (!s->mqActive) {
        if (t - s->lastInputMs < (DWORD)g_cfg.marqueeDelayMs) return false;
        s->mqActive = true; s->mqPhase = 0; s->mqOff = 0; s->mqNext = t + g_cfg.marqueeStepMs;
        logf("MARQUEE: start rows=%d max=%d chars (idle %lu ms)", s->mqRows, s->mqMax, (unsigned long)(t - s->lastInputMs));
        return false;
    }
    if ((LONG)(t - s->mqNext) < 0) return false;
    switch (s->mqPhase) {
    case 0:
        if (s->mqOff < s->mqMax) ++s->mqOff;
        if (s->mqOff >= s->mqMax) { s->mqPhase = 1; s->mqNext = t + g_cfg.marqueeHoldMs; }
        else s->mqNext = t + g_cfg.marqueeStepMs;
        return true;
    case 1: s->mqOff = 0; s->mqPhase = 2; s->mqNext = t + g_cfg.marqueeHoldMs; return true;
    default: s->mqPhase = 0; s->mqNext = t + g_cfg.marqueeStepMs; return false;
    }
}

// ---- hover tooltip: title = name - time (dialogfont16x16), body = every stat line + the description (10x10, wrapped) ----
void hideTip(Slot* s, const char* why) {
    if (s->tipShown) logf("TIP: hide (%s)", why);
    if (s->tipShown) for (int t = T_TIPBG; t <= T_TIP; ++t) hide(s, t);
    s->tipShown = false; s->tipKey = -1;
}
// greedy word wrap of src into out (appends, '\n' between lines); returns the number of lines added
int wrapAppend(Slot* s, const char* src, float maxPx, char* out, int* len, int cap) {
    int lines = 0; char cur[256] = ""; int cl = 0;
    auto emit = [&]() {
        int w = snprintf(out + *len, cap - *len, "%s%s", *len ? "\n" : "", cur);
        if (w > 0 && w < cap - *len) { *len += w; ++lines; } else out[*len] = 0;   // no clipped tail on overflow
        cur[0] = 0; cl = 0;
    };
    const char* p = src;
    while (*p) {
        while (*p == ' ') ++p;
        if (!*p) break;
        const char* q = p; while (*q && *q != ' ') ++q;
        int wl = (int)(q - p); if (wl > 120) wl = 120;
        char cand[256]; snprintf(cand, sizeof cand, "%s%s%.*s", cur, cl ? " " : "", wl, p);
        if (cl && textPx(s, cand, (int)strlen(cand), F10) > maxPx) { emit(); snprintf(cur, sizeof cur, "%.*s", wl, p); }
        else snprintf(cur, sizeof cur, "%s", cand);
        cl = (int)strlen(cur); p = q;
    }
    if (cl) emit();
    return lines;
}
void showTip(Slot* s, int row) {
    if (!s->ctl[T_TIPBG] || !s->ctl[T_TIPT] || !s->ctl[T_TIP] || row < 0 || row >= s->nPlaced) return;
    const Entry& x = s->ent[s->rowEnt[row]];
    float tw = PanelW - 8, inner = tw - 2 * TipInX, tx = PanelX + 4;
    float innerPx = (ux(s, tx + TipInX + inner) - ux(s, tx + TipInX)) * 0.98f;
    char tm[16], full[96], title[100]; fmtTime(x, tm, sizeof tm);
    snprintf(full, sizeof full, "%s - %s", x.name, tm);
    fitText(s, full, innerPx, title, sizeof title);
    static char body[1024]; int len = 0, lines = 0; body[0] = 0;
    for (int i = 0; i < x.nLines; ++i) lines += wrapAppend(s, x.lines[i], innerPx, body, &len, sizeof body);
    if (x.desc && g_cfg.tooltipDesc) {
        if (len && len < (int)sizeof body - 2) { body[len++] = '\n'; body[len] = 0; ++lines; }   // blank line before the text
        lines += wrapAppend(s, x.desc, innerPx, body, &len, sizeof body);
    }
    if (!len) { snprintf(body, sizeof body, "No details"); lines = 1; }
    float lineU = kFont10LineH * TextK10 * g_cfg.textScale * g_cfg.tipLineScale;   // texels -> units (px = units * sy)
    float h = fminf(TipBodyY + lines * lineU + TipPadB, PanelH);
    float y = s->rowY[row] + RowH;
    bool flip = y + h > PanelY + PanelH;
    if (flip) y = s->rowY[row] - h;
    if (y < PanelY) y = PanelY;
    place(s, T_TIPBG, tx, y, tw, h);
    place(s, T_TIPT, tx + TipInX, y + TipTitleY, inner, TipTitleH);
    place(s, T_TIP, tx + TipInX, y + TipBodyY, inner, h - TipBodyY - TipPadB / 2);
    setText(s, T_TIPT, title); setText(s, T_TIP, body);
    if (!s->tipShown || s->tipKey != x.key)
        logf("TIP: show row=%d '%s' lines=%d stat='%s' flip=%d rect=[%d,%d,%d,%d] lineU=%.2f", row, x.name, lines, x.stat, flip,
             s->rect[T_TIPBG][0], s->rect[T_TIPBG][1], s->rect[T_TIPBG][2], s->rect[T_TIPBG][3], lineU);
    s->tipShown = true; s->tipKey = x.key;
}
// hover hit test on the placed rows (sections, gaps and the scrollbar do not count); refresh = re-show (time update)
void tickTooltip(Slot* s, DWORD t, bool haveCur, float cx, float cy, bool refresh) {
    if (!g_cfg.tooltips || g_cfg.stage < 1 || !s->nPlaced || s->sx <= 0 || s->sy <= 0) { s->hoverRow = -1; s->hoverKey = -1; hideTip(s, "off"); return; }
    int row = -1;
    if (haveCur) {
        float u = PanelX + (cx - s->bx) / s->sx, v = PanelY + (cy - s->by) / s->sy;
        for (int r = 0; r < s->nPlaced; ++r)
            if (u >= X0 && u < X0 + s->lastRowW && v >= s->rowY[r] && v < s->rowY[r] + RowH) { row = r; break; }
    }
    int key = row >= 0 ? s->ent[s->rowEnt[row]].key : -1;
    if (key != s->hoverKey || row != s->hoverRow) {
        if (s->tipShown) hideTip(s, row < 0 ? "leave" : "other row");
        s->hoverKey = key; s->hoverRow = row; s->hoverSince = t;
    }
    if (row < 0) return;
    if (!s->tipShown ? t - s->hoverSince >= (DWORD)g_cfg.tooltipDelayMs : refresh) showTip(s, row);
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
        else if (t < T_MORE) snprintf(tag, sizeof tag, "LBL_EFX_%c%02d", "DINTS"[(t - T_D0) / MaxRows], (t - T_D0) % MaxRows);
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
    s->lastInputMs = GetTickCount();
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
    bool wheelPending = g_wheelAccum != 0;
    bool scrolled = g_cfg.stage >= 1 && tickInput(s);
    DWORD t = GetTickCount();
    float cx = 0, cy = 0; bool haveCur = cursorEngine(&cx, &cy);
    bool relayout = scrolled;
    if (scrolled) hideTip(s, "scroll");
    relayout |= trackInput(s, t, haveCur, cx, cy, wheelPending || scrolled || dt > 0.5f);
    relayout |= marqueeStep(s, t);
    bool poll = t - s->lastPoll >= (DWORD)g_cfg.pollMs || dt > 0.5f;
    if (!poll) { if (relayout) layout(s); tickTooltip(s, t, haveCur, cx, cy, false); return; }
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
        hideTip(s, "list changed");
        if (trackInput(s, t, false, 0, 0, true)) {}   // a new list restarts the idle clock (marquee back to offset 0)
        for (int i = 0; i < n; ++i) { char tm[16]; fmtTime(s->ent[i], tm, sizeof tm); logf("  entry %d: %s%s%s icon=%s item=%d %s", i, s->ent[i].section ? "[SECTION] " : "", s->ent[i].section ? (s->ent[i].harmful ? "HARMFUL" : "BENEFICIAL") : s->ent[i].name, "", s->ent[i].icon, s->ent[i].item, s->ent[i].section ? "" : tm); }
    }
    layout(s);
    tickTooltip(s, t, haveCur, cx, cy, true);
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
    if (reason == DLL_PROCESS_ATTACH) { logf("chareffects 0.1.0-probe build 4 loaded"); loadIni(); saferead::enableRing(g_debug); }
    return TRUE;
}
