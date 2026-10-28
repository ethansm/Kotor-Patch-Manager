// Character Active Effects Panel 0.5.1-dev (Steam Aspyr build 6A522E71...). Fills the right third of the character page
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
// 0.5.1-dev (doc 51 ST6b): header hint tooltip, Force Mastery tooltip (marker effects 0x9960C8/CC), Level row + class lines on the Stats tab and Experience tip.
// 0.5.0-dev (doc 51 ST6): hover tooltips on the vanilla character page rows (attributes, vitality, Force, Defense, saves, alignment, experience) from VanillaTips.h, drawn with the panel tooltip controls (ini vanilla_tips=0 disables); no hook or GUI change.
// 0.4.1-dev (doc 51 ST5b): Influence row on the Stats tab for party NPCs (party table +0x11C, slot via object id then tag; ini show_influence).
// 0.4.0-dev (doc 51 ST5a): Stats tab. Click the panel header to switch Effects <-> Stats (ini stats_tab=0 hides the toggle); the Stats rows come from StatsRows.h (srows::build) fed by gatherStats; no hook or GUI change.
// 0.3.0-probe (doc 51 ST4): log only, no UI change. STATS block (deflection/defense/Force DC/FP cost/resist via engine getters + CombatStats.h) on each creature change; DEFLECT hook H-C5 at 006e1c60+ compares the engine's blaster-deflection total with cstats::deflection (ini deflect_check).
// 0.2.0-probe (doc 32 MG5): KpLog (core file character-effects-panel_log.txt, old core fallback chareffects_log.txt), IniKit primitives, PanelSlots, GuiKit wheel/screen/cursor/bindByTag, Party; lazy init from KPatchInit/first hook.
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
#include "EffectIcons.h"   // k2fx: icon tables + collectBuffs (Buff HUD)
#include "ChareffDecode.h"      // effect stat decoder + pixel-width text layer (pure, host-tested; includes EffectNameTable.inc)
#include "GameAddr.h"
#include "CombatStats.h"       // cstats:: pure stat math (doc 51)
#include "CombatStatsAddr.h"   // cstats_addr:: engine getters + offsets (doc 51 ST4 probe)
#include "StatsRows.h"         // srows:: Stats tab row builder (doc 51 ST5a, pure, host-tested)
#include "VanillaTips.h"        // vtips:: vanilla character page hover table + tooltip builder (doc 51 ST6, pure, host-tested)
#include "GuiInput.h"
#include "GuiKit.h"   // guikit:: setRect/getExtent/imageFor/setText/setTextColor/setFill/setBorderTint/bindByTag/wheel/screen/cursor + detail::textObjOk
#include "PanelSlots.h"   // Common: panel -> slot table
#include "Party.h"        // Common: party table / member / server creature
#include "SafeRead.h"  // VirtualQuery-based probing: IsBadReadPtr faults are a storm under Wine (DB lesson 234)

KP_LOG_PATCH("character-effects-panel");

namespace {

using namespace cfx;   // ChareffDecode.h: Entry, Atom, atomOf, formatStats, Font, TextK ...

// ---- engine addresses (Steam Aspyr) ----
constexpr int BorderVtable = 0x009875BC, TextVtable = 0x009876B4;
constexpr int LabelBorderOff = 0x60, BorderColorOff = 0x28;
constexpr int LabelTextOff = 0xD8, TextRendererOff = 0x14, TextStringSubOff = 0x18, TextColorOff = 0x2c;   // colour at text+0x18+0x2c
constexpr int CtlFlagsOff = 0x48, CtlIdOff = 0x54;   // flags bit1 visible, 0x20 click-through
constexpr int ClientCreatureVftable = static_cast<int>(gameaddr::ClientCreatureVtable);
constexpr int PanelShownOff = 0x4b70;       // SetStats' cache of the shown client creature (logged for comparison only)
constexpr int EffArrOff = 0x148, EffCntOff = 0x14c;
constexpr int EffTypeOff = 0x8, EffSubOff = 0xA, EffDurOff = 0xC, EffDayOff = 0x10, EffMsOff = 0x14, EffSpellOff = 0x1c;
constexpr int EffNIntsOff = 0x30, EffIntsOff = 0x34;
constexpr int EffTypeIcon = 0x43;
constexpr int WorldTimerGetCurrentTime = static_cast<int>(gameaddr::WorldTimerGetCurrentTime);   // thiscall(timer, int* day, int* ms)
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
// dialogfont10x10 (stat column, tooltip body): same 0.01 world units per texel as 16x16 (TXI texturewidth 2.56 on a
// 256 texel sheet, research R3b R-c), so the same factor; line pitch = kFont10LineH texels (calibrate live, tip_line_scale)
// build 4 screenshot 15:09: "Shield 20 3" = 82 texels drew 224 px (2.73 px/texel, 16x16 names 1.99) -> ~1.03; INI text_k10
constexpr float StatGap = 6, ColGap = 8;   // stat-time gap; name-stat gap (name-time gap without a stat = NameGap)
constexpr float TipInX = 8, TipTitleY = 6, TipTitleH = 20, TipBodyY = 28, TipPadB = 8;   // tooltip insets (= mirror script)
constexpr float SbW = 10, SbArrowH = 18, SbGap = 3, SbThumbMin = 16;
constexpr float X0 = PanelX + PadX, InnerW = PanelW - 2 * PadX, Top = PanelY + PadTop, Win = PanelH - 2 * PadTop;

const float Cream[3] = {0.8f, 0.8f, 0.69804f}, Orange[3] = {0.94902f, 0.65882f, 0.30196f};
const float TealText[3] = {0.10196f, 0.69804f, 0.54902f}, TealFrame[3] = {0.05098f, 0.34902f, 0.27059f};
const float DiscTeal[3] = {0.035f, 0.415f, 0.34f}, White[3] = {1, 1, 1};   // disc = the Buff HUD backing colour
const char* const DiscResref = "buffhud_circle";

// ---- diagnostics: KpLog (core file character-effects-panel_log.txt; old core: fallback chareffects_log.txt at INFO), capped; kplog.ini [character-effects-panel] level=debug enables detail ----
KP_CHANNEL(chMain, "main");
KP_CHANNEL(chDbg, "debug");
// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("chareffects_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);
int g_logLines = 0; constexpr int MaxLogLines = 20000;
bool g_debug = false; int g_dbgLines = 0; constexpr int MaxDebugLines = 10000;
char g_loadLine[160];   // formatted by DllMain, emitted by ensureInit (the logger is only known after KPatchInit)
void vlog(::kplog::Channel& ch, int level, const char* fmt, va_list ap) {
    char buf[2048]; vsnprintf(buf, sizeof buf, fmt, ap);
    KP_LOGV(ch, level, "%s", buf);
}
void logf(const char* fmt, ...) { if (g_logLines >= MaxLogLines) return; ++g_logLines; va_list ap; va_start(ap, fmt); vlog(chMain, KPLOG_INFO, fmt, ap); va_end(ap); }
void logw(const char* fmt, ...) { va_list ap; va_start(ap, fmt); vlog(chMain, KPLOG_WARN, fmt, ap); va_end(ap); }
void loge(const char* fmt, ...) { va_list ap; va_start(ap, fmt); vlog(chMain, KPLOG_ERR, fmt, ap); va_end(ap); }
void dlogf(const char* fmt, ...) { if (!g_debug || g_dbgLines >= MaxDebugLines) return; ++g_dbgLines; va_list ap; va_start(ap, fmt); vlog(chDbg, KPLOG_INFO, fmt, ap); va_end(ap); }
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
    int lvl = (code <= 1) ? KPLOG_INFO : KPLOG_WARN;
    va_list ap; va_start(ap, fmt); vlog(chMain, lvl, fmt, ap); va_end(ap);
}

// ---- configuration: $GAME/chareffects.ini (missing/garbage = defaults; every key and rejected line logged) ----
struct Cfg {
    int enabled = 1, stage = 1, pollMs = 250, showPermanent = 1, wheelScroll = 1, tooltips = 1;
    int statColumn = 1, marquee = 1, marqueeDelayMs = 2500, marqueeStepMs = 160, marqueeHoldMs = 1200;
    int tooltipDelayMs = 350, tooltipDesc = 1, deflectCheck = 1, statsTab = 1, showInfluence = 0, vanillaTips = 1;   // statsTab 0 = no Effects/Stats header toggle; showInfluence 1 = Influence row (default 0: spoiler)
    float statMaxW = 0, statMinFrac = 0.4f, textK10 = 1.03f, tipLineScale = 1.0f;   // stat_max_w 0 = no cap
    int skipEquipped = 1, iconSource = 1, colorTimes = 1, discs = 1, soonS = 30;
    int fakeRows = 0;   // debug aid: N synthetic entries appended to the real list (overflow / scrollbar / sort tests)
    float textScale = 1.0f, screenW = 3840, screenH = 1600; bool screenExplicit = false;
    char fallbackIcon[17] = "ip_mind";
};
Cfg g_cfg; unsigned g_cfgGen = 0;
ULONGLONG g_iniStamp = 0; DWORD g_iniLastCheck = 0;
using inikit::trim;
void syncWheelCfg() { guikit::g_wheel.enabled = g_cfg.wheelScroll != 0; guikit::g_wheel.iniW = (int)g_cfg.screenW; guikit::g_wheel.iniH = (int)g_cfg.screenH; }   // after every loadIni and each tick
void loadIni() {
    Cfg c;
    g_debug = ::kplog::Enabled(chDbg, KPLOG_DEBUG);
    WIN32_FILE_ATTRIBUTE_DATA fa;
    g_iniStamp = GetFileAttributesExA("chareffects.ini", GetFileExInfoStandard, &fa)
        ? (((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime) : 0;
    FILE* f = fopen("chareffects.ini", "r");
    if (!f) { logf("INI: chareffects.ini not found, using defaults (stage=%d debug=%d)", c.stage, g_debug); g_cfg = c; ++g_cfgGen; syncWheelCfg(); return; }
    char line[300]; int ln = 0, ok = 0, bad = 0;
    struct { const char* k; int* i; float* fl; } keys[] = {
        {"enabled", &c.enabled, nullptr}, {"stage", &c.stage, nullptr}, {"poll_ms", &c.pollMs, nullptr},
        {"show_permanent", &c.showPermanent, nullptr}, {"wheel_scroll", &c.wheelScroll, nullptr}, {"tooltips", &c.tooltips, nullptr},
        {"skip_equipped", &c.skipEquipped, nullptr}, {"icon_source", &c.iconSource, nullptr}, {"color_times", &c.colorTimes, nullptr},
        {"discs", &c.discs, nullptr}, {"soon_s", &c.soonS, nullptr}, {"text_scale", nullptr, &c.textScale},
        {"screen_w", nullptr, &c.screenW}, {"screen_h", nullptr, &c.screenH}, {"fake_rows", &c.fakeRows, nullptr},
        {"stat_column", &c.statColumn, nullptr}, {"stat_max_w", nullptr, &c.statMaxW}, {"stat_min_frac", nullptr, &c.statMinFrac}, {"text_k10", nullptr, &c.textK10}, {"marquee", &c.marquee, nullptr},
        {"marquee_delay_ms", &c.marqueeDelayMs, nullptr}, {"marquee_step_ms", &c.marqueeStepMs, nullptr},
        {"marquee_hold_ms", &c.marqueeHoldMs, nullptr}, {"tooltip_delay_ms", &c.tooltipDelayMs, nullptr},
        {"tooltip_desc", &c.tooltipDesc, nullptr}, {"deflect_check", &c.deflectCheck, nullptr}, {"stats_tab", &c.statsTab, nullptr}, {"show_influence", &c.showInfluence, nullptr}, {"vanilla_tips", &c.vanillaTips, nullptr}, {"tip_line_scale", nullptr, &c.tipLineScale}};
    while (fgets(line, sizeof(line), f)) {
        ++ln; trim(line);
        char* k = nullptr; char* v = nullptr;
        inikit::IniLine sp = inikit::splitIniLine(line, &k, &v);
        if (sp == inikit::IniLineSkip) continue;
        if (sp == inikit::IniLineNoEquals) { logf("INI line %d rejected (no '='): %s", ln, line); ++bad; continue; }
        bool known = false, good = false;
        if (!strcmp(k, "fallback_icon")) { known = true; good = v[0] && strlen(v) < sizeof(c.fallbackIcon); if (good) strcpy(c.fallbackIcon, v); }
        for (auto& e : keys) if (!strcmp(k, e.k)) {
            known = true; float a; good = inikit::parseFloats(v, &a, 1);
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
    if (c.statMaxW < 0 || c.statMaxW > 200) c.statMaxW = 0;
    if (c.statMinFrac < 0.1f || c.statMinFrac > 0.9f) c.statMinFrac = 0.4f;
    if (c.textK10 < 0.4f || c.textK10 > 2.0f) c.textK10 = 1.03f;
    if (c.marqueeDelayMs < 200) c.marqueeDelayMs = 200;
    if (c.marqueeStepMs < 30) c.marqueeStepMs = 30;
    if (c.marqueeHoldMs < 0) c.marqueeHoldMs = 0;
    if (c.tooltipDelayMs < 0) c.tooltipDelayMs = 0;
    if (c.tipLineScale < 0.5f || c.tipLineScale > 2.0f) c.tipLineScale = 1.0f;
    if (c.screenW < 100 || c.screenH < 100) { c.screenW = 3840; c.screenH = 1600; c.screenExplicit = false; }
    g_cfg = c; ++g_cfgGen; syncWheelCfg();
    logf("INI loaded: %d keys ok, %d rejected, debug=%d gen=%u enabled=%d stage=%d poll_ms=%d fake_rows=%d stat=%d/%.0f marquee=%d (%d/%d/%d) tooltips=%d (%d ms, desc=%d) stats_tab=%d influence=%d vanilla_tips=%d",
         ok, bad, g_debug, g_cfgGen, c.enabled, c.stage, c.pollMs, c.fakeRows, c.statColumn, c.statMaxW, c.marquee,
         c.marqueeDelayMs, c.marqueeStepMs, c.marqueeHoldMs, c.tooltips, c.tooltipDelayMs, c.tooltipDesc, c.statsTab, c.showInfluence, c.vanillaTips);
}
void maybeReloadIni() {
    DWORD now = GetTickCount();
    if (now - g_iniLastCheck < 1000) return;
    g_iniLastCheck = now;
    WIN32_FILE_ATTRIBUTE_DATA fa; ULONGLONG st = 0;
    if (GetFileAttributesExA("chareffects.ini", GetFileExInfoStandard, &fa))
        st = ((ULONGLONG)fa.ftLastWriteTime.dwHighDateTime << 32) | fa.ftLastWriteTime.dwLowDateTime;
    bool dbg = ::kplog::Enabled(chDbg, KPLOG_DEBUG);
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
void syncScreen() {   // engine framebuffer size (GuiKit reads the K2Addr ScreenW/ScreenH globals) unless the INI sets it
    int w = (int)g_cfg.screenW, h = (int)g_cfg.screenH;
    if (guikit::syncScreen(g_cfg.screenExplicit ? w : 0, g_cfg.screenExplicit ? h : 0, &w, &h)) { g_cfg.screenW = (float)w; g_cfg.screenH = (float)h; }
}
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
    logf("INPUT: hwnd=%p client=%ldx%ld (engine %.0fx%.0f)", g_hwnd, r.right, r.bottom, g_cfg.screenW, g_cfg.screenH);
    if (g_hwnd && g_cfg.wheelScroll && !guikit::g_wheelHwnd) {
        if (!g_wheelCfgSet) { g_wheelCfgSet = true; guikit::g_wheel.scoped = false; guikit::g_wheel.onEvent = wheelEvent; }
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

// ---- label plumbing (Mod 5) ----
unsigned char* bindLabel(int panel, const char* tagName, int expectId) {
    guikit::BindInfo bi;
    guikit::bindByTag(static_cast<uintptr_t>(static_cast<unsigned>(panel)), tagName, expectId, 0, &bi);
    saferead::beginScope();   // bindByTag does not drop the readable-region cache itself
    if (bi.why == guikit::CF_Alloc) { logw("H1 %s: allocation failed", tagName); return nullptr; }
    if (bi.why == guikit::CF_Vtable) { logw("H1 %s: ctor left vtable %08x (expected %08x), not binding", tagName, bi.badVtable, (unsigned)gameaddr::LabelVtable); return nullptr; }
    logf("H1 %s: ctl=%p id=%d (expect %d)%s extent=[%d,%d,%d,%d] flags=%08x -> %08x", tagName, (void*)bi.ctl, bi.id, expectId,
         bi.id == expectId ? "" : " MISMATCH", bi.ext[0], bi.ext[1], bi.ext[2], bi.ext[3], bi.flagsBefore, bi.flagsAfter);
    return bi.id == expectId ? reinterpret_cast<unsigned char*>(bi.ctl) : nullptr;
}
void* imageFor(const char* resref) {   // one engine image per resref, never released (Buff HUD / Mod 5 pattern); pool = guikit::imageFor
    bool loaded = false;
    void* im = guikit::imageFor(resref, &loaded);
    if (loaded) {
        char buf[20] = {}; strncpy(buf, resref, 16);
        for (char* p = buf; *p; ++p) *p = (char)tolower((unsigned char)*p);
        logf("image '%s' -> %p", buf, im);
    }
    return im;
}
void setCtlVisible(unsigned char* c, bool vis) {
    int fl = 0; if (!c || !readAt<int>(reinterpret_cast<int>(c), CtlFlagsOff, &fl)) return;
    int nf = vis ? (fl | 2) : (fl & ~2);
    if (nf != fl) *reinterpret_cast<int*>(c + CtlFlagsOff) = nf;
}
bool ctlRect(unsigned char* c, float* r) {   // engine-space rect from a label's extent (+4..+0x10, top-left origin)
    int e[4];
    if (!guikit::getExtent(reinterpret_cast<uintptr_t>(c), &e[0], &e[1], &e[2], &e[3])) return false;
    for (int k = 0; k < 4; ++k) r[k] = (float)e[k];
    return e[2] > 0 && e[3] > 0;
}
void setCtlRect(unsigned char* c, int x, int y, int w, int h) { guikit::setRect(reinterpret_cast<uintptr_t>(c), x, y, w, h); }
bool textObjOk(unsigned char* c) { return guikit::detail::textObjOk(reinterpret_cast<uintptr_t>(c)); }
bool readVec3(int addr, float* v) { for (int k = 0; k < 3; ++k) if (!readAt<float>(addr, 4 * k, &v[k])) return false; return true; }
bool near3(const float* a, const float* b) { for (int k = 0; k < 3; ++k) if (fabsf(a[k] - b[k]) > 0.01f) return false; return true; }

// ---- per-panel slot ----
constexpr int MaxEnt = 96;
int g_lastTab = 0;   // tab of the last click (0 Effects, 1 Stats): new Slot binds start here
struct Slot {
    int panel = 0; bool bound = false; unsigned seq = 0; DWORD bindTick = 0;
    unsigned char* ctl[NumTags] = {};
    char text[NumTags][100] = {}; char tipText[1024] = {}; void* fill[NumTags] = {}; int col[NumTags] = {}; int rect[NumTags][4] = {};
    float bx = 0, by = 0, sx = 0, sy = 0;     // unit -> px from the bound LBL_EFX_BACK extent
    bool canTextColor = false, canBorderColor = false;
    int client = 0, server = 0; unsigned sig = 0; DWORD lastPoll = 0, lastBeat = 0;
    Entry ent[MaxEnt]; int nEnt = 0; int first = 0;
    int tab = 0;   // 0 Effects, 1 Stats
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
    Entry vtipEnt; bool vtipValid = false;   // vanilla-row tooltip (doc 51 ST6): text built at show time, kept for refresh
};
panelslots::PanelSlots<Slot, 4> g_slots;
unsigned g_seq = 0;
uintptr_t slotKey(int panel) { return static_cast<uintptr_t>(static_cast<unsigned>(panel)); }
Slot* findSlot(int panel) { return g_slots.find(slotKey(panel)); }
// Array position of a slot in forEach order (index order; used only for the "slot %d ready" log).
int slotIndex(const Slot* s) {
    int idx = -1, i = 0;
    g_slots.forEach([&](Slot& e) { if (&e == s) idx = i; ++i; });
    return idx;
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
    guikit::setText(reinterpret_cast<uintptr_t>(c), txt);   // ExoCtor / TextSet / ExoDtor
    dlogf("label %d: text \"%.80s\" -> \"%.80s\"", t, cache, txt);
    strncpy(cache, txt, cap - 1); cache[cap - 1] = 0;
}
void setTextColor(Slot* s, int t, int which, const float* rgb) {   // which: cache key (1 cream, 2 orange)
    unsigned char* c = s->ctl[t]; if (!c || !s->canTextColor || s->col[t] == which || !textObjOk(c)) return;
    guikit::setTextColor(reinterpret_cast<uintptr_t>(c), rgb[0], rgb[1], rgb[2]);
    s->col[t] = which;
}
void setFill(Slot* s, int t, const char* resref, const float* tint) {
    unsigned char* c = s->ctl[t]; if (!c) return;
    int border = reinterpret_cast<int>(c) + LabelBorderOff, bvt = 0;
    if (!readAt<int>(border, 0, &bvt) || bvt != BorderVtable) { whyLog(200 + t, "label %d: border vtable %08x != %08x, no fill", t, bvt, BorderVtable); return; }
    void* im = resref ? imageFor(resref) : nullptr;
    if (resref && !im) im = imageFor(g_cfg.fallbackIcon);
    if (s->fill[t] != im) { guikit::setFill(reinterpret_cast<uintptr_t>(c), im); s->fill[t] = im; }
    if (tint && s->canBorderColor) { float col[3]; if (guikit::getBorderTint(reinterpret_cast<uintptr_t>(c), col) && !near3(col, tint)) guikit::setBorderTint(reinterpret_cast<uintptr_t>(c), tint); }
}

// ---- effect data ----
int leaderClient() {   // what SetStats 0x84e6f0 shows: PartyGetAt(party table, 0)
    uintptr_t t = party::table();
    int n = party::partyCount(t); if (n <= 0 || n > 64) return 0;   // 0.1.0's range check (Party.h only tests i < count)
    return static_cast<int>(party::member(t, 0));
}
bool clientValid(int c) { int vt = 0; return readAt<int>(c, 0, &vt) && vt == ClientCreatureVftable; }

struct Raw { int eff, type, sub, spell, iconRow; float dur; double rem; bool perm; };
void fmtTime(const Entry& e, char* out, int n) {
    if (e.val[0]) { snprintf(out, n, "%s", e.val); return; }   // Stats tab value text
    if (e.perm) { snprintf(out, n, "Permanent"); return; }
    if (e.remMs < 0) { snprintf(out, n, "--"); return; }
    long secs = (long)ceil(e.remMs / 1000.0);
    if (secs >= 3600) snprintf(out, n, "%ld:%02ld:%02ld", secs / 3600, secs / 60 % 60, secs % 60);
    else snprintf(out, n, "%ld:%02ld", secs / 60, secs % 60);
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
// times are frozen. Entry 4 uses a resref with no texture to show what the engine image loader returns for a missing image.
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
    int day = 0, ms = 0; bool haveTime = k2fx::currentTime(&day, &ms);
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
                e.item = bad ? kDebuffIcons[r.spell].item : k2fx::isItemAbility(r.spell);
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

// text width in engine px: see TextFit.h via ChareffDecode.h (kFontTexW / kFont10TexW in FontWidths.inc from gen_effect_name_table.py; TextK measured; text_scale = INI fine-tune)
// Slot*/g_cfg wrappers over the pure text layer in ChareffDecode.h (cfx::textPx/fitText/marqueeText/wrapAppend)
cfx::TextCtx textCtx(const Slot* s) { return {s->sy, g_cfg.textK10, g_cfg.textScale}; }
float textPx(Slot* s, const char* t, int len, Font f = F16) { return cfx::textPx(textCtx(s), t, len, f); }
bool fitText(Slot* s, const char* name, float maxPx, char* out, int n, Font f = F16) { return cfx::fitText(textCtx(s), name, maxPx, out, n, f); }
void marqueeText(Slot* s, const char* full, float maxPx, int off, char* out, int n, int* maxOff, Font f = F16) {
    cfx::marqueeText(textCtx(s), full, maxPx, off, out, n, maxOff, f);
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

void hideAll(Slot* s) { for (int t = 0; t < NumTags; ++t) hide(s, t); s->over = false; s->tipShown = false; s->nPlaced = 0; guikit::g_wheel.setRect(0, 0, 0, 0, 0); }

// Header text: "Effects | Stats" / "Stats | Effects" (active tab first); stats_tab=0 keeps the GUI's "Active Effects".
void updateHeader(Slot* s) {
    if (g_cfg.statsTab) setText(s, T_HEADER, s->tab == 1 ? "Stats | Effects" : "Effects | Stats");
    else if (s->text[T_HEADER][0]) setText(s, T_HEADER, "Active Effects");
}

void layout(Slot* s) {
    const Entry* e = s->ent; int n = s->nEnt;
    const bool st = s->tab == 1;   // Stats tab: section entries use the D control as the band and the N control as the title
    updateHeader(s);
    if (g_cfg.stage < 1 || n == 0) {
        for (int t = T_TIPBG; t <= T_TIP; ++t) hide(s, t);
        s->nPlaced = 0; s->tipShown = false; s->mqMax = 0; s->mqRows = 0;
        for (int t = T_SECG; t < T_MORE; ++t) hide(s, t);
        for (int t = T_SBTRK; t <= T_SBDN; ++t) hide(s, t);
        char msg[64];
        int rows = 0; for (int i = 0; i < n; ++i) rows += !e[i].section;
        if (n == 0) snprintf(msg, sizeof msg, st ? "No stats" : "No active effects");
        else snprintf(msg, sizeof msg, "%d active effect%s (preview)", rows, rows == 1 ? "" : "s");
        place(s, T_MORE, X0, PanelY + PanelH / 2 - RowH / 2, InnerW, RowH);
        setText(s, T_MORE, msg);
        s->over = false; guikit::g_wheel.setRect(0, 0, 0, 0, 0);
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
    // columns sized from the content of the visible rows (build 4b): time column = widest time; stat and name want their
    // widest text; when both do not fit, the stat column keeps at least stat_min_frac of the room and the name the rest.
    // The stat column is shared (left-aligned at one x); a row without a stat lets its name run up to the time column.
    // need = text + 3 units and the fit below = column - 2 units, both in float (build 4c): a column equal to its need
    // never cuts (4b lost ~1 px to ux() rounding and cut 'Speed +199%' in a 86/86 column).
    float timeCol = 0, statNeed = 0, nameNeed = 0;
    for (int k = 0, rr = 0; k < cnt && rr < MaxRows; ++k) {
        const Entry& x = e[s->first + k];
        if (x.section) { if (st) ++rr; continue; }   // Stats tab: a section takes a row slot too
        ++rr;
        char tm[24]; fmtTime(x, tm, sizeof tm);
        timeCol = fmaxf(timeCol, textPx(s, tm, (int)strlen(tm)) / s->sx + 1);
        if (g_cfg.statColumn && x.stat[0]) statNeed = fmaxf(statNeed, textPx(s, x.stat, (int)strlen(x.stat), F10) / s->sx + 3);
        nameNeed = fmaxf(nameNeed, textPx(s, x.name, (int)strlen(x.name)) / s->sx + 3);
    }
    const float nx = st ? X0 : X0 + IconW + 4, tr = X0 + rowW - TimePadR;
    float room = tr - timeCol - nx - (statNeed > 0 ? StatGap + ColGap : NameGap), statCol = 0;
    if (statNeed > 0) {
        statCol = fminf(statNeed, fmaxf(room - nameNeed, fminf(statNeed, room * g_cfg.statMinFrac)));
        if (g_cfg.statMaxW > 0) statCol = fminf(statCol, g_cfg.statMaxW);
        if (statCol < 12) statCol = 0;
    }
    const float statX = tr - timeCol - StatGap - statCol;
    addRow(" cols[name %.0f/%.0f stat %.0f/%.0f time %.0f]", (statCol > 0 ? statX - ColGap : tr - timeCol - NameGap) - nx, nameNeed, statCol, statNeed, timeCol);
    for (int k = 0; k < cnt; ++k) {
        const Entry& x = e[s->first + k];
        if (x.section && st) {   // Stats tab section: band in D (full width), title in N (cream), I/T/S hidden
            if (r >= MaxRows) break;
            place(s, T_D0 + r, X0 - SecOut, y, rowW + SecOut + (over ? 0 : SecOut), SecH);
            setFill(s, T_D0 + r, "uibit_fill_2wt", TealFrame);
            hide(s, T_I0 + r); hide(s, T_T0 + r); hide(s, T_S0 + r);
            place(s, T_N0 + r, X0 - SecOut, y, rowW + SecOut, SecH);
            char title[72]; snprintf(title, sizeof title, " %s", x.name);   // leading space = title inset
            setText(s, T_N0 + r, title);
            setTextColor(s, T_N0 + r, 3, Cream);
            s->rowY[r] = -1000; s->rowEnt[r] = s->first + k;   // never hit by the hover test
            addRow(" |%s|", x.name);
            y += SecH + SecPad; ++r;
            continue;
        }
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
        if (st) { hide(s, T_I0 + r); hide(s, T_D0 + r); }   // Stats value row: no icon, no fallback icon, no disc
        else {
        place(s, T_I0 + r, X0, y + (RowH - IconH) / 2, IconW, IconH);
        setFill(s, T_I0 + r, x.icon[0] ? x.icon : g_cfg.fallbackIcon, White);
        if (x.item && g_cfg.discs) { place(s, T_D0 + r, X0, y + (RowH - IconH) / 2, IconW, IconH); setFill(s, T_D0 + r, DiscResref, DiscTeal); }
        else hide(s, T_D0 + r);
        }
        // time right-aligned by placement: label = text width + 1 unit slack, so left- and right-aligned drawing both end at tr
        char tm[24]; fmtTime(x, tm, sizeof tm);
        float tx = tr - textPx(s, tm, (int)strlen(tm)) / s->sx - 1;
        place(s, T_T0 + r, tx, y, tr - tx, RowH);
        setText(s, T_T0 + r, tm);
        float nr = tr - timeCol - NameGap;   // name right edge: the stat column when this row has a stat
        char st[64] = "";
        if (statCol > 0 && x.stat[0]) {
            place(s, T_S0 + r, statX, y, statCol, RowH);
            // fit 2 units inside the label: a text wider than its label is word-wrapped by the engine (15:09 screenshot)
            int mo = 0; marqueeText(s, x.stat, (statCol - 2) * s->sx, s->mqOff, st, sizeof st, &mo, F10);
            if (mo > mqMax) mqMax = mo;
            mqRows += mo > 0;
            setText(s, T_S0 + r, st);
            nr = statX - ColGap;
        } else hide(s, T_S0 + r);
        float nw = nr - nx;
        place(s, T_N0 + r, nx, y, nw, RowH);
        s->rowY[r] = y; s->rowEnt[r] = s->first + k;
        char nm[64]; int mo = 0;
        marqueeText(s, x.name, (nw - 2) * s->sx, s->mqOff, nm, sizeof nm, &mo);
        if (mo > mqMax) mqMax = mo;
        mqRows += mo > 0;
        setText(s, T_N0 + r, nm);
        if (s->col[T_N0 + r] == 3) setTextColor(s, T_N0 + r, 4, TealText);   // back from a Stats section title (cream) to the GUI's teal
        addRow(" [%d] \"%s\" {%s} %s%s", r, nm, st, tm, x.item ? " disc" : "");
        bool warn = g_cfg.colorTimes && (x.harmful || (!x.perm && x.remMs >= 0 && x.remMs < g_cfg.soonS * 1000.0));
        setTextColor(s, T_T0 + r, warn ? 2 : 1, warn ? Orange : Cream);
        y += RowH; ++r;
    }
    s->nPlaced = r; s->lastRowW = rowW; s->mqMax = mqMax; s->mqRows = mqRows;
    for (; r < MaxRows; ++r) { hide(s, T_D0 + r); hide(s, T_I0 + r); hide(s, T_N0 + r); hide(s, T_T0 + r); hide(s, T_S0 + r); }
    if (!secShown[0] || st) hide(s, T_SECG);
    if (!secShown[1] || st) hide(s, T_SECB);
    s->over = over; s->shownN = cnt; s->lastFirst = lf;
    if (!over) {
        for (int t = T_SBTRK; t <= T_SBDN; ++t) hide(s, t);
        guikit::g_wheel.setRect(0, 0, 0, 0, 0);
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
    {
        int pr0 = ux(s, PanelX), pr1 = uy(s, PanelY);
        guikit::g_wheel.setRect(0, pr0, pr1, ux(s, PanelX + PanelW) - pr0, uy(s, PanelY + PanelH) - pr1);
    }
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
void hideTip(Slot* s, const char* why);
bool tickInput(Slot* s) {
    bool changed = false;
    LONG wheel = guikit::g_wheel.take();
    if (wheel && s->over) { int steps = -(int)(wheel / 120); if (!steps) steps = wheel > 0 ? -1 : 1; changed |= scrollTo(s, s->first + steps, "wheel"); }
    bool l = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (g_cfg.statsTab && l && !s->wasL) {   // header click toggles Effects <-> Stats (before the !over early return: over = list overflows)
        float hx = 0, hy = 0, hr[4];
        if (cursorEngine(&hx, &hy) && GetForegroundWindow() == g_hwnd && ctlRect(s->ctl[T_HEADER], hr) && inRect(hr, hx, hy)) {
            s->tab ^= 1; g_lastTab = s->tab; s->first = 0; s->lastPoll = 0;   // lastPoll = 0: the next tick polls at once
            hideTip(s, "tab");
            logf("TAB -> %s", s->tab == 1 ? "stats" : "effects");
            s->dragging = false; s->heldPart = 0; s->wasL = l;
            return true;
        }
    }
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
    if (s->hoverRow <= -2) { s->hoverRow = -1; s->hoverKey = -1; s->vtipValid = false; }   // vanilla hover state: re-delay after a forced hide
}
int wrapAppend(Slot* s, const char* src, float maxPx, char* out, int* len, int cap) { return cfx::wrapAppend(textCtx(s), src, maxPx, out, len, cap); }
// Draws one tooltip. Panel rows: tx = PanelX + 4, tip below the row (rowTop/rowH), flipped above past the panel bottom.
// Vanilla rows (vanilla = true): tx = left edge chosen by the caller, rowTop = rect top, placed at the same top, clamped to the 800x600 screen.
void showTipAt(Slot* s, const Entry& x, float tx, float rowTop, float rowH, const char* logWhat, bool vanilla) {
    if (!s->ctl[T_TIPBG] || !s->ctl[T_TIPT] || !s->ctl[T_TIP]) return;
    float tw = PanelW - 8, inner = tw - 2 * TipInX;
    float innerPx = (ux(s, tx + TipInX + inner) - ux(s, tx + TipInX)) * 0.98f;
    char tm[24], full[96], title[100]; fmtTime(x, tm, sizeof tm);
    if (x.val[0]) snprintf(full, sizeof full, "%s %s", x.name, x.val);   // Stats tab: "Defense 12"
    else if (vanilla) snprintf(full, sizeof full, "%s", x.name);
    else snprintf(full, sizeof full, "%s - %s", x.name, tm);
    fitText(s, full, innerPx, title, sizeof title);
    static char body[1024]; int len = 0, lines = 0; body[0] = 0;
    for (int i = 0; i < x.nLines; ++i) lines += wrapAppend(s, x.lines[i], innerPx, body, &len, sizeof body);
    if (x.desc && g_cfg.tooltipDesc) {
        if (len && len < (int)sizeof body - 2) { body[len++] = '\n'; body[len] = 0; ++lines; }   // blank line before the text
        lines += wrapAppend(s, x.desc, innerPx, body, &len, sizeof body);
    }
    if (!len) { snprintf(body, sizeof body, "No details"); lines = 1; }
    float lineU = kFont10LineH * g_cfg.textK10 * g_cfg.textScale * g_cfg.tipLineScale;   // texels -> units (px = units * sy)
    float h = fminf(TipBodyY + lines * lineU + TipPadB, PanelH);
    float y; bool flip = false;
    if (vanilla) {
        y = rowTop;
        if (y + h > 600) y = 600 - h;
        if (y < 0) y = 0;
    } else {
        y = rowTop + rowH;
        flip = y + h > PanelY + PanelH;
        if (flip) y = rowTop - h;
        if (y < PanelY) y = PanelY;
    }
    place(s, T_TIPBG, tx, y, tw, h);
    place(s, T_TIPT, tx + TipInX, y + TipTitleY, inner, TipTitleH);
    place(s, T_TIP, tx + TipInX, y + TipBodyY, inner, h - TipBodyY - TipPadB / 2);
    setText(s, T_TIPT, title); setText(s, T_TIP, body);
    if (!s->tipShown || s->tipKey != x.key) {
        if (logWhat) logf("TIP: %s %s lines=%d rect=[%d,%d,%d,%d] lineU=%.2f", logWhat, x.name, lines,
                          s->rect[T_TIPBG][0], s->rect[T_TIPBG][1], s->rect[T_TIPBG][2], s->rect[T_TIPBG][3], lineU);
        else logf("TIP: show '%s' lines=%d stat='%s' flip=%d rect=[%d,%d,%d,%d] lineU=%.2f", x.name, lines, x.stat, flip,
                  s->rect[T_TIPBG][0], s->rect[T_TIPBG][1], s->rect[T_TIPBG][2], s->rect[T_TIPBG][3], lineU);
    }
    s->tipShown = true; s->tipKey = x.key;
}
void showTip(Slot* s, int row) {
    if (row < 0 || row >= s->nPlaced) return;
    showTipAt(s, s->ent[s->rowEnt[row]], PanelX + 4, s->rowY[row], RowH, nullptr, false);
}
// Force Mastery marker effect on the creature: 0 none, 1 light, 2 dark (plain reads, no engine call).
int masteryOf(int server) {
    using namespace cstats_addr;
    int lt = 0, dk = 0, arr = 0, cnt = 0;
    if (!server) return 0;
    readAt<int>(static_cast<int>(EffMasteryLight), 0, &lt); readAt<int>(static_cast<int>(EffMasteryDark), 0, &dk);
    if ((!lt && !dk) || !readAt<int>(server, Creature_Effects, &arr) || !readAt<int>(server, Creature_EffectCount, &cnt) || !arr || cnt <= 0 || cnt > MaxEffects) return 0;
    for (int i = 0; i < cnt; ++i) {
        int eff = 0; uint16_t ty = 0;
        if (!readAt<int>(arr, 4 * i, &eff) || !eff || !readAt<uint16_t>(eff, Effect_Type, &ty)) continue;
        if (lt && ty == lt) return 1;
        if (dk && ty == dk) return 2;
    }
    return 0;
}
// Vanilla row tooltip: left = right of the rect (+6), else left of it; top = rect top. fresh = build the text now (gatherStats), else re-show the cached one.
bool gatherStats(int server, srows::StatsIn* in);
void showVanillaTip(Slot* s, int vrow, bool fresh) {
    if (vrow < 0 || vrow >= vtips::V_COUNT) return;
    if (fresh) {
        srows::StatsIn in;
        bool have = s->server && gatherStats(s->server, &in);
        saferead::beginScope();   // engine getters ran
        if (!vtips::build(vrow, in, have, &s->vtipEnt)) return;
        s->vtipEnt.key = 0x40000 + vrow;
        s->vtipValid = true;
    } else if (!s->vtipValid) return;
    const vtips::Rect& r = vtips::kRects[vrow];
    float tw = PanelW - 8, left = r.x + r.w + 6;
    if (left + tw > 800) left = r.x - 6 - tw;
    showTipAt(s, s->vtipEnt, left, r.y, r.h, "vanilla", true);
}
// hover hit test on the placed rows (sections, gaps and the scrollbar do not count); refresh = re-show (time update)
void tickTooltip(Slot* s, DWORD t, bool haveCur, float cx, float cy, bool refresh) {
    if (!g_cfg.tooltips || g_cfg.stage < 1 || s->sx <= 0 || s->sy <= 0) { s->hoverRow = -1; s->hoverKey = -1; hideTip(s, "off"); return; }
    int row = -1, vrow = -1;
    if (haveCur) {
        float u = PanelX + (cx - s->bx) / s->sx, v = PanelY + (cy - s->by) / s->sy;
        for (int r = 0; r < s->nPlaced; ++r)
            if (u >= X0 && u < X0 + s->lastRowW && v >= s->rowY[r] && v < s->rowY[r] + RowH) { row = r; break; }
        if (row < 0 && g_cfg.vanillaTips) vrow = vtips::hit(u, v);   // vanilla character page rows (doc 51 ST6); the effects list may be empty
        if (vrow == vtips::V_HEADER && !g_cfg.statsTab) vrow = -1;   // no toggle, no hint
        if (vrow == vtips::V_MASTERY && !masteryOf(s->server)) vrow = -1;   // LBL_FORCEMASTERY is hidden without a mastery effect
    }
    int key = row >= 0 ? s->ent[s->rowEnt[row]].key : vrow >= 0 ? 0x40000 + vrow : -1;
    int hrow = row >= 0 ? row : vrow >= 0 ? -2 - vrow : -1;
    if (key != s->hoverKey || hrow != s->hoverRow) {
        if (s->tipShown) hideTip(s, hrow == -1 ? "leave" : "other row");
        s->hoverKey = key; s->hoverRow = hrow; s->hoverSince = t; s->vtipValid = false;
    }
    if (row < 0 && vrow < 0) return;
    if (vrow >= 0) {
        if (!s->tipShown) { if (t - s->hoverSince >= (DWORD)g_cfg.tooltipDelayMs) showVanillaTip(s, vrow, true); }
        else if (refresh) showVanillaTip(s, vrow, false);
        return;
    }
    if (!s->tipShown ? t - s->hoverSince >= (DWORD)g_cfg.tooltipDelayMs : refresh) showTip(s, row);
}

LARGE_INTEGER g_qpcFreq, g_qpcLast;

// Lazy init: load line + first loadIni() + ring. Runs from KPatchInit, else from the first hook (old core without KPatchInit).
volatile LONG g_inited = 0;
void ensureInit() {
    if (g_inited) return;                                 // cheap fast path
    if (InterlockedExchange(&g_inited, 1)) return;
    logf("%s", g_loadLine);
    loadIni();
    saferead::enableRing(g_debug);
}

// ---- doc 51 ST4: STATS / DEFLECT probe (log only) ----
void* vp(int a) { return reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<unsigned>(a))); }
// Fills DeflectIn from the engine getters (every memory read via readAt). Returns false (with *why) if the stats pointer is unusable.
bool gatherDeflect(int server, cstats::DeflectIn* in, int* statsOut, const char** why) {
    using namespace cstats_addr;
    *in = cstats::DeflectIn{}; *statsOut = 0;
    if (!server) { *why = "null creature"; return false; }
    int stats = 0;
    if (!readAt<int>(server, Creature_Stats, &stats) || !stats) { *why = "null stats pointer"; return false; }
    uint8_t probe = 0;
    if (!readAt<uint8_t>(stats, Stats_ClassCount, &probe)) { *why = "stats unreadable"; return false; }
    *statsOut = stats;
    void* st = vp(stats);
    in->bab = static_cast<int>(reinterpret_cast<GetBaseAttackBonusFn>(GetBaseAttackBonus)(st));
    in->hasAJD = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatAJD) != 0;
    in->hasMJD = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatMJD) != 0;
    in->hasForceRedirection = reinterpret_cast<HasSpellFn>(HasSpell)(st, 0, cstats::kSpellForceRedirection, 0) != 0;
    in->hasWMDeflect = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatWMDeflect) != 0;
    in->saber = reinterpret_cast<IsWieldingLightsaberFn>(IsWieldingLightsaber)(vp(server)) != 0;
    // Weapon Master: the engine only looks at the first 2 class slots
    uint8_t nCls = probe;
    for (int i = 0; i < 2 && i < nCls; ++i) {
        uint8_t id = 0;
        if (!readAt<uint8_t>(stats, static_cast<int>(Stats_ClassId0 + Stats_ClassStride * i), &id)) break;
        if (id == cstats::kClassWeaponMaster) {
            in->isWeaponMaster = true;
            in->wmLevel = reinterpret_cast<GetClassLevelFn>(GetClassLevel)(st, static_cast<uint8_t>(i), 0);
            break;
        }
    }
    int32_t form = 0; readAt<int32_t>(server, Creature_Form, &form); in->form = form;
    // effect walk: sum of deflection increase - decrease effects
    int incT = 0, decT = 0, arr = 0, cnt = 0;
    readAt<int>(static_cast<int>(EffDeflectInc), 0, &incT); readAt<int>(static_cast<int>(EffDeflectDec), 0, &decT);
    if (readAt<int>(server, Creature_Effects, &arr) && readAt<int>(server, Creature_EffectCount, &cnt) && arr && cnt > 0 && cnt <= MaxEffects && (incT || decT)) {
        int delta = 0;
        for (int i = 0; i < cnt; ++i) {
            int eff = 0; uint16_t ty = 0;
            if (!readAt<int>(arr, 4 * i, &eff) || !eff || !readAt<uint16_t>(eff, Effect_Type, &ty)) continue;
            if (incT && ty == incT) delta += reinterpret_cast<EffectGetIntegerFn>(EffectGetInteger)(vp(eff), 1);
            else if (decT && ty == decT) delta -= reinterpret_cast<EffectGetIntegerFn>(EffectGetInteger)(vp(eff), 1);
        }
        in->effectDelta = delta;
    }
    return true;
}
void fmtParts(const cstats::Breakdown& b, char* buf, size_t cap) {
    size_t n = 0; buf[0] = 0;
    for (int i = 0; i < b.count && n + 1 < cap; ++i) {
        int w = snprintf(buf + n, cap - n, "%s%s %+d", i ? ", " : "", b.parts[i].label, b.parts[i].value);
        if (w < 0 || static_cast<size_t>(w) >= cap - n) break;
        n += static_cast<size_t>(w);
    }
}
// 3 log lines per creature change.
void logStats(int server) {
    using namespace cstats_addr;
    cstats::DeflectIn in; int stats = 0; const char* why = "";
    if (!gatherDeflect(server, &in, &stats, &why)) { logf("STATS server=%08x skipped: %s", server, why); return; }
    void* st = vp(stats);
    cstats::Breakdown d = cstats::deflection(in);
    char dp[400]; fmtParts(d, dp, sizeof dp);
    logf("STATS server=%08x deflect=%d [%s] form=%d saber=%d wm=%d/%d", server, d.total, dp, in.form, in.saber ? 1 : 0, in.isWeaponMaster ? 1 : 0, in.wmLevel);
    int defense = reinterpret_cast<GetArmorClassFn>(GetArmorClass)(vp(server));
    int dex = reinterpret_cast<GetDEXModFn>(GetDEXMod)(st, 1);
    uint8_t stance = 0xFF; readAt<uint8_t>(server, Creature_StanceFlag, &stance);
    logf("STATS defense=%d dex=%d stance=%u", defense, dex, (unsigned)stance);
    cstats::ForceDCIn fi = {};
    fi.level = reinterpret_cast<GetLevelFn>(GetLevel)(st, 1);
    int8_t cha = 0, wis = 0; readAt<int8_t>(stats, Stats_ChaMod, &cha); readAt<int8_t>(stats, Stats_WisMod, &wis);
    fi.chaMod = cha; fi.wisMod = wis;
    fi.ff1 = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatFF1) != 0;
    fi.ff2 = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatFF2) != 0;
    fi.ff3 = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatFF3) != 0;
    cstats::Breakdown fd = cstats::forceDC(fi);
    char fp[400]; fmtParts(fd, fp, sizeof fp);
    int16_t ge = 0; readAt<int16_t>(stats, Stats_GoodEvil, &ge);
    cstats::CostMult cm = cstats::fpCostMult(ge);
    int fr = reinterpret_cast<ForceResistFn>(ForceResist)(st);
    logf("STATS forceDC=%d [%s] goodEvil=%d light=x%.2f dark=x%.2f forceResist=%d", fd.total, fp, (int)ge, cm.light, cm.dark, fr);
}

// Short damage-resist text from the creature's effect list: A_RES / A_DR atoms (no absorb limit) via ChareffDecode, ", "-joined.
void gatherResist(int server, char* out, size_t cap) {
    using namespace cstats_addr;
    out[0] = 0;
    int arr = 0, cnt = 0;
    if (!readAt<int>(server, Creature_Effects, &arr) || !readAt<int>(server, Creature_EffectCount, &cnt) || !arr || cnt <= 0 || cnt > MaxEffects) return;
    AtomSet set = {};
    for (int i = 0; i < cnt; ++i) {
        int eff = 0, v[8], nv = 0; uint16_t ty = 0; Atom a;
        if (!readAt<int>(arr, 4 * i, &eff) || !eff || !readAt<uint16_t>(eff, Effect_Type, &ty) || !readInts(eff, v, &nv) || !atomOf(ty, v, nv, &a)) continue;
        if ((a.kind == A_RES || a.kind == A_DR) && a.limit <= 0) addAtom(set, a);
    }
    size_t len = 0;
    for (int i = 0; i < set.n; ++i) {
        char sh[48], lg[LineLen]; fmtAtom(set.at[i], sh, sizeof sh, lg, sizeof lg);
        if (!sh[0]) continue;
        int w = snprintf(out + len, cap - len, "%s%s", len ? ", " : "", sh);
        if (w < 0 || static_cast<size_t>(w) >= cap - len) { out[len] = 0; break; }
        len += static_cast<size_t>(w);
    }
}
// Party NPC slot (0..11) of a server creature, -1 = not a party NPC. Same order as the engine: object id scan, then tag.
int npcSlot(int server, int table) {
    using namespace cstats_addr;
    uint32_t id = 0;
    if (readAt<uint32_t>(server, Creature_ObjectId, &id) && id != 0x7F000000u)
        for (int i = 0; i < 12; ++i) { uint32_t v = 0; if (readAt<uint32_t>(table, static_cast<int>(PT_NpcObjectIds + 4 * i), &v) && v == id) return i; }
    int txt = 0, len = 0; char tag[17] = {};
    if (!readAt<int>(server, Creature_Tag, &txt) || !txt || !readAt<int>(server, Creature_Tag + 4, &len) || len <= 0 || len > 16) return -1;
    for (int k = 0; k < len; ++k) if (!readAt<char>(txt, k, &tag[k])) return -1;
    for (int i = 0; i < 12; ++i) if (!_stricmp(tag, srows::kNpcTags[i])) return i;
    return -1;
}
// Influence of the shown creature (doc 51 ST5b): plain reads, no engine call.
void gatherInfluence(int server, srows::StatsIn* in) {
    using namespace cstats_addr;
    int app = 0, sv = 0, inner = 0;
    if (!readAt<int>(static_cast<int>(AppGlobal), 0, &app) || !app || !readAt<int>(app, App_Server, &sv) || !sv ||
        !readAt<int>(sv, Server_Internal, &inner) || !inner) return;
    int table = inner + static_cast<int>(PartyTableOff);
    int slot = npcSlot(server, table); if (slot < 0) return;
    int v = 0; if (!readAt<int>(table, static_cast<int>(PT_Influence + 4 * slot), &v)) return;
    in->haveInfluence = true; in->npcSlot = slot; in->influence = v;
}
// Fills the Stats tab input from the engine getters (same calls as logStats). Never calls GetAttackModifierVersus / GetCriticalHitRoll.
bool gatherStats(int server, srows::StatsIn* in) {
    using namespace cstats_addr;
    *in = srows::StatsIn{};
    int stats = 0; const char* why = "";
    if (!gatherDeflect(server, &in->defl, &stats, &why)) return false;
    void* st = vp(stats);
    int8_t u0 = 0, u1 = 0; readAt<int8_t>(stats, Stats_Ungated0, &u0); readAt<int8_t>(stats, Stats_Ungated1, &u1);
    in->ungated = static_cast<int8_t>(u0 + u1);
    int ci = 0;
    if (readAt<int>(stats, Stats_CombatInfo, &ci) && ci) {
        int8_t am = 0, ao = 0; uint8_t tm = 0, mm = 0, to = 0, mo = 0; int has = 0;
        if (readAt<int8_t>(ci, CI_AtkMain, &am) && readAt<int8_t>(ci, CI_AtkOff, &ao) && readAt<uint8_t>(ci, CI_CritThreatMain, &tm) &&
            readAt<uint8_t>(ci, CI_CritMultMain, &mm) && readAt<int>(ci, CI_HasOff, &has) && readAt<uint8_t>(ci, CI_CritThreatOff, &to) &&
            readAt<uint8_t>(ci, CI_CritMultOff, &mo)) {
            in->haveCombat = true; in->atkMain = am; in->atkOff = ao; in->critThreatMain = tm; in->critMultMain = mm;
            in->hasOff = has != 0; in->critThreatOff = to; in->critMultOff = mo;
        }
    }
    in->defense = reinterpret_cast<GetArmorClassFn>(GetArmorClass)(vp(server));
    in->dexMod = reinterpret_cast<GetDEXModFn>(GetDEXMod)(st, 1);
    in->stance = 0xFF; readAt<uint8_t>(server, Creature_StanceFlag, &in->stance);
    in->fdc = cstats::ForceDCIn{};
    in->fdc.level = reinterpret_cast<GetLevelFn>(GetLevel)(st, 1);
    int8_t cha = 0, wis = 0; readAt<int8_t>(stats, Stats_ChaMod, &cha); readAt<int8_t>(stats, Stats_WisMod, &wis);
    in->fdc.chaMod = cha; in->fdc.wisMod = wis;
    in->fdc.ff1 = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatFF1) != 0;
    in->fdc.ff2 = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatFF2) != 0;
    in->fdc.ff3 = reinterpret_cast<HasFeatFn>(HasFeat)(st, cstats::kFeatFF3) != 0;
    int16_t ge = 0; readAt<int16_t>(stats, Stats_GoodEvil, &ge); in->goodEvil = ge;
    in->forceResist = reinterpret_cast<ForceResistFn>(ForceResist)(st);
    uint8_t nc = 0; readAt<uint8_t>(stats, Stats_ClassCount, &nc);   // class slots for the Level row and the Mastery line (last slot)
    for (int i = 0; i < nc && in->nClass < 3; ++i) {
        uint8_t id = 0;
        if (!readAt<uint8_t>(stats, static_cast<int>(Stats_ClassId0 + Stats_ClassStride * i), &id)) break;
        in->classId[in->nClass] = id;
        in->classLvl[in->nClass++] = reinterpret_cast<GetClassLevelFn>(GetClassLevel)(st, static_cast<uint8_t>(i), 0);
    }
    in->mastery = masteryOf(server);
    gatherResist(server, in->resist, sizeof in->resist);
    if (g_cfg.showInfluence) gatherInfluence(server, in);
    return true;
}

} // namespace

extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    guikit::initGuiInput(api, "character-effects-panel");
    ensureInit();
}

extern "C" void __cdecl BindCharEffects(int panel) {
    ensureInit();
    saferead::beginScope();
    int ctlCount = -1; readAt<int>(panel, 0x28, &ctlCount);
    logf("H1 BindCharEffects panel=%08x controls=%d enabled=%d stage=%d", panel, ctlCount, g_cfg.enabled, g_cfg.stage);
    if (!panel) { logf("H1: null panel, abort"); return; }
    if (!g_cfg.enabled) { logf("H1: disabled by INI, controls not bound (right third stays empty)"); return; }
    g_slots.onEvict = [](Slot& old, uintptr_t, unsigned seq) { if (old.bound) logf("slot: evicting panel=%08x seq=%u", old.panel, seq); };
    bool fresh = false;
    Slot* s = g_slots.claim(slotKey(panel), ++g_seq, &fresh);
    if (!fresh && s->mqActive) logf("MARQUEE: reset (rebind)");
    *s = Slot();
    s->panel = panel; s->seq = g_seq; s->bindTick = GetTickCount();
    s->tab = (g_cfg.statsTab && g_cfg.stage >= 1) ? g_lastTab : 0;
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
    if (!s->ctl[T_BACK] || !ctlRect(s->ctl[T_BACK], br)) { logw("H1: LBL_EFX_BACK missing (is character_p.gui patched? scripts/mirror_character_page.py), panel unusable"); s->panel = panel; s->bound = false; return; }
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
    logf("H1: slot %d ready (panel=%08x seq=%u)", slotIndex(s), panel, s->seq);
}

extern "C" void __cdecl CharEffectsTick(int panel) {
    ensureInit();
    saferead::beginScope();
    static int calls = 0;
    if (!g_qpcFreq.QuadPart) { QueryPerformanceFrequency(&g_qpcFreq); QueryPerformanceCounter(&g_qpcLast); }
    LARGE_INTEGER now; QueryPerformanceCounter(&now);
    float dt = (float)((double)(now.QuadPart - g_qpcLast.QuadPart) / (double)g_qpcFreq.QuadPart);
    g_qpcLast = now;
    syncScreen();
    syncWheelCfg();
    guikit::g_wheel.touch(GetTickCount());
    Slot* s = findSlot(panel); if (s && !s->bound) s = nullptr;
    if (calls < 5) { ++calls; logf("H3 tick #%d panel=%08x slot=%s", calls, panel, s ? "bound" : "none"); }
    if (!s) return;
    if (dt > 0.5f) logf("H3: tick gap %.2fs (page reopened)", dt);
    maybeReloadIni();
    if (!g_cfg.enabled) { hideAll(s); whyLog(1, "H3: disabled by INI, panel hidden"); return; }
    setCtlVisible(s->ctl[T_BACK], true); setCtlVisible(s->ctl[T_HEADER], true);
    {   // ini stats_tab=0 / stage 0 (also on hot reload): tab forced to Effects
        int want = (g_cfg.statsTab && g_cfg.stage >= 1) ? s->tab : 0;
        if (want != s->tab) { s->tab = want; s->first = 0; s->lastPoll = 0; hideTip(s, "tab"); }
    }
    bool wheelPending = guikit::g_wheel.accum != 0;
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
    int server = static_cast<int>(party::serverCreature(static_cast<uintptr_t>(static_cast<unsigned>(client))));
    saferead::beginScope();   // engine code ran: drop the readable-region cache
    if (!server) { whyLog(4, "H3: leader %08x has no server creature", client); s->nEnt = 0; layout(s); return; }
    bool changedCreature = client != s->client || server != s->server;
    if (changedCreature) {
        int shown = 0; readAt<int>(panel, PanelShownOff, &shown);
        logf("H3: creature client=%08x server=%08x (panel+0x4b70=%08x%s) -> scroll reset", client, server, shown, shown == client ? "" : " differs: SetStats not run yet");
        s->client = client; s->server = server; s->first = 0;
        logStats(server);
    }
    unsigned oldSig = s->sig; int oldN = s->nEnt;
    int n = 0;
    if (s->tab == 1) {   // Stats tab: rows from StatsRows.h; signature = FNV over name + value
        srows::StatsIn in;
        if (gatherStats(server, &in)) n = srows::build(in, s->ent, MaxEnt);
        saferead::beginScope();   // engine getters ran
        unsigned h = 2166136261u;
        for (int i = 0; i < n; ++i) {
            s->ent[i].key = 0x30000 + i;
            for (const char* p = s->ent[i].name; *p; ++p) h = (h ^ (unsigned char)*p) * 16777619u;
            h = (h ^ 0x1Fu) * 16777619u;
            for (const char* p = s->ent[i].val; *p; ++p) h = (h ^ (unsigned char)*p) * 16777619u;
            h = (h ^ 0x1Eu) * 16777619u;
        }
        s->sig = h;
    } else {
        n = collect(s, server, s->ent, false);
        if (n < 0) { whyLog(5, "H3: effect list of %08x unreadable", server); s->nEnt = 0; layout(s); return; }
    }
    s->nEnt = n;
    if (s->tab == 1) {
        if (changedCreature || s->sig != oldSig || n != oldN) {
            dlogf("H3: stats changed: %d rows (sig %08x -> %08x)", n, oldSig, s->sig);
            hideTip(s, "list changed");
            trackInput(s, t, false, 0, 0, true);
        }
    } else if (changedCreature || s->sig != oldSig || n != oldN) {   // structure changed: dump once with reasons
        int arr = 0, cnt = 0; readAt<int>(server, EffArrOff, &arr); readAt<int>(server, EffCntOff, &cnt);
        logf("H3: list changed: %d entries (sig %08x -> %08x), %d raw effects", n, oldSig, s->sig, cnt);
        collect(s, server, s->ent, true);
        hideTip(s, "list changed");
        if (trackInput(s, t, false, 0, 0, true)) {}   // a new list restarts the idle clock (marquee back to offset 0)
        for (int i = 0; i < n; ++i) { char tm[24]; fmtTime(s->ent[i], tm, sizeof tm); logf("  entry %d: %s%s%s icon=%s item=%d %s", i, s->ent[i].section ? "[SECTION] " : "", s->ent[i].section ? (s->ent[i].harmful ? "HARMFUL" : "BENEFICIAL") : s->ent[i].name, "", s->ent[i].icon, s->ent[i].item, s->ent[i].section ? "" : tm); }
    }
    layout(s);
    tickTooltip(s, t, haveCur, cx, cy, true);
    if (t - s->bindTick < 120000 && t - s->lastBeat >= 5000) {
        s->lastBeat = t;
        logf("H3 beat: client=%08x server=%08x entries=%d first=%d over=%d stage=%d", client, server, s->nEnt, s->first, s->over, g_cfg.stage);
    }
}

// H-C5: ResolveDefensiveEffects 006e1c60 after rec+0x168 = total. Log only: compares the engine total with cstats::deflection.
// Runs inside combat for every blaster bolt: no allocation, no UI, logf only for the first 30 calls then mismatches.
extern "C" void __cdecl CharEffectsDeflectCheck(int rec, int total, int defender, int effDelta, int attacker) {
    using namespace cstats_addr;
    ensureInit();
    static unsigned calls = 0, mismatches = 0, loggedMis = 0; static bool capLogged = false;
    ++calls;
    if (!g_cfg.deflectCheck) return;
    constexpr unsigned kFirstLogged = 30, kMaxMisLogged = 100, kMaxCalls = 5000;
    if (capLogged) return;
    if (loggedMis >= kMaxMisLogged || calls > kMaxCalls) {
        capLogged = true;
        logf("DEFLECT: log cap reached (%u calls, %u mismatches)", calls, mismatches);
        return;
    }
    saferead::beginScope();   // engine code ran: drop the readable-region cache
    cstats::DeflectIn in; int stats = 0; const char* why = "";
    if (!gatherDeflect(defender, &in, &stats, &why)) {
        if (calls <= kFirstLogged) logf("DEFLECT #%u def=%08x skipped: %s", calls, defender, why);
        return;
    }
    uint8_t roll = 0, psFeat = 0, u0 = 0, u1 = 0; int8_t ps = 0;
    readAt<uint8_t>(rec, Rec_Roll, &roll); readAt<uint8_t>(rec, Rec_PSFeat, &psFeat); readAt<int8_t>(rec, Rec_PS, &ps);
    readAt<uint8_t>(stats, Stats_Ungated0, &u0); readAt<uint8_t>(stats, Stats_Ungated1, &u1);
    int ungated = static_cast<int8_t>(static_cast<uint8_t>(u0 + u1));
    // Precise Shot is the ATTACKER's feat (engine: HasFeat on [ebp-0xb8]->stats, 0xF4..0xF0 = -10..-2, highest only).
    // rec+0x17A/B are written only when a tier applies, so they can be stale: recompute, log rec as a cross-check.
    int psTerm = 0, attStats = 0;
    if (attacker && readAt<int>(attacker, Creature_Stats, &attStats) && attStats) {
        for (uint16_t f = 0xF4; f >= 0xF0; --f)
            if (reinterpret_cast<HasFeatFn>(HasFeat)(vp(attStats), f)) { psTerm = -2 * (f - 0xEF); break; }
    }
    int psRec = (psFeat >= 0xF0 && psFeat <= 0xF4) ? static_cast<int>(ps) : 0;
    cstats::Breakdown d = cstats::deflection(in);
    int ours = static_cast<int>(roll) + d.total + ungated + psTerm;
    bool match = ours == total;
    if (!match) ++mismatches;
    if (calls <= kFirstLogged || !match) {
        if (!match && calls > kFirstLogged) ++loggedMis;
        logf("DEFLECT #%u def=%08x att=%08x roll=%d engine=%d ours=%d diff=%d effEngine=%d effOurs=%d ps=%d psRec=%d form=%d %s",
             calls, defender, attacker, (int)roll, total, ours, total - ours, effDelta, in.effectDelta, psTerm, psRec, (int)in.form, match ? "MATCH" : "MISMATCH");
    }
}

extern "C" void __cdecl CharEffectsDestroy(int panel) {
    ensureInit();
    Slot* s = findSlot(panel); if (s && !s->bound) s = nullptr;
    logf("H4 CharEffectsDestroy panel=%08x slot=%s", panel, s ? "found" : "none");
    if (!s) return;
    guikit::g_wheel.setRect(0, 0, 0, 0, 0);
    s->bound = false; s->panel = 0;   // controls are engine-owned (InitControl addToList=1): never freed here
    g_slots.release(slotKey(panel));
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_DETACH) guikit::wheelUninstall();   // never leave a dangling window proc: restores if on top, else stays chained but inert
    if (reason == DLL_PROCESS_ATTACH) { snprintf(g_loadLine, sizeof g_loadLine, "chareffects 0.5.1-dev build 10 loaded (KpLog + IniKit + PanelSlots + GuiKit wheel + Party + STATS/DEFLECT probe + Stats tab + influence + vanilla tips) tick=%lu", (unsigned long)GetTickCount()); }
    return TRUE;
}
