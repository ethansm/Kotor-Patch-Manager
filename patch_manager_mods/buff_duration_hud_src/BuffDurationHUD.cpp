// Buff Duration HUD (Steam Aspyr build 6A522E71...). Replaces the engine's plain
// buff/debuff arrow glyphs over each party portrait with the effect's own icons: buffs in a
// column down the card's left edge, debuffs down the right edge, item abilities on a round
// teal disc, each draining top-down (dim = elapsed, full = remaining) as the effect runs out. Research trail and every address below:
// patch_manager_mods/02_buff_duration_hud.md (local-setup/KOTOR-II worktree).
//
// v2.0.0 (doc 21 W4): the shield bar is its own patch (shield-bar) and the shared portrait hooks (UpdatePortraits slot table, the
// per-slot draw gate 0x745774) belong to portrait-slots ("k2.portraitslots" v1, PortraitSlots.h). This DLL registers there:
//   PRE_ENGINE_READ prio 200  ForceArrowCounters (every slot, before the engine reads slot+0xddc/+0xddd)
//   FRAME_BEGIN     prio 200  kill switch buffhud_off.txt (every 30 frames)
//   MENU_OVERLAY    prio 200  companion portrait icon columns (after the shield bars)
// Without portrait-slots every hook finds no creature and the engine arrows draw natively.
// Hooks (all inside CSWGuiMainInterface, cdecl detours, original bytes still run):
//   2      0x00745a84  PortraitIconRowDraw, before each native arrow Draw(): hide the arrow
//   3      0x00745a97  right after that Draw() (buff row only): un-hide it
//   5      0x00745aec  row exit (once per call): draw our icon column, dimmed + bright slice
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "BuffHudLogic.h"          // bh:: pure logic: appendf (bounded log formatting, audit 20a F7), arrow counter bump, column geometry
#include "../_shared/SafeRead.h"    // VirtualQuery-based probing: IsBadReadPtr = a real Wine page fault + SEH dispatch per bad pointer
#include "../_shared/K2Addr.h"     // per-exe address table: every engine offset below is read from k2addr::get() (0 = unknown -> feature off)
#include "../_shared/EffectIcons.h" // effect-list walk + decode (Effects.h) and the buff/debuff icon tables (doc 21 W2)
#include "../_shared/Gui2D.h"       // 2D batch / clip / flush / image draw + CapGuard
#include "../_shared/GuiKit.h"      // imageFor: one pooled engine image per resref
#include "../_shared/PortraitSlots.h" // k2.portraitslots v1 consumer (slot -> creature, phase callbacks)

namespace {

using k2fx::Buff;
using k2fx::collectBuffs;
using k2fx::currentTime;
using guikit::imageFor;
using gui2d::CapGuard;
// The icon widget is a CSWGuiLabel with two embedded children: a CSWGuiBorder ("BORDER", draws the arrow glyph as its fill image) and a
// CSWGuiText (font text, unused here). Verified by live memory dump (Steam): border vtable BorderVtable, size at Border_Width/Border_Height.
// All offsets are K2AddrTable fields (Label_Border, Border_Width, Border_Height, Border_Rot, Border_Alpha, Border_Tint, Ctl_*, Slot_*).

// icon size / column constants (SmallCardFraction, LargeCardFraction, MinIconSize, MaxIconSize, MaxColumns, RightInset) live in BuffHudLogic.h
const float LineColor[3] = { 0.0f, 0.16f, 0.14f };  // thin dark-teal edge between remaining and elapsed
constexpr float LineAlpha = 0.95f;
constexpr float DimAlpha = 0.35f;
constexpr float BgAlpha = 0.80f;              // backing disc, full-strength part
const float BgColor[3] = { 0.035f, 0.415f, 0.34f };  // the HUD border teal (sampled 9,106,87)
const char* const CircleResref = "buffhud_circle";  // white disc TGA shipped in override/

// ---- diagnostics: OFF by default. Create an empty "buffhud_debug.txt" in the game dir to turn
// on buffhud_log.txt (capped, first-N + change-detected lines from the hooks below). ----
bool g_debugLog = false;
int g_logLines = 0;
constexpr int MaxLogLines = 6000;
void logf(const char* fmt, ...) {
    if (!g_debugLog || g_logLines >= MaxLogLines) return;
    FILE* f = fopen("buffhud_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

// ---- portrait-slots consumer -------------------------------------------------------------
k2ps::Client g_ps;
const char* g_connectNote = nullptr;   // set in KPatchInit (no file I/O allowed there), logged on the first frame
bool g_connectOk = false;
void onConnect(bool ok, const char* why) { g_connectOk = ok; g_connectNote = why; }
int creatureForSlot(int slotPtr) { return g_ps.creatureForSlot(slotPtr); }

// ---- helpers -------------------------------------------------------------------------
template <typename T> bool readAt(int base, int off, T* out) {
    return saferead::readAt(1, static_cast<uintptr_t>(base), off, out);
}

// state saved by hook 2 for hook 3
struct Saved { bool active; int border; int origImage; int width; int height; int widthOff, heightOff; } g_saved;

// ---- VIS diagnostics (visibility trace): `VIS <what> shown=%d why=%s rect=[x,y,w,h]`, written through logf (the mod's main channel,
// so it needs buffhud_debug.txt like every other line here), ONLY when (shown, why-category, count, rect) changes for that item.
// At most VisMaxLines lines per process, then one `VIS cap reached` and silence. Pure logging: no effect on hooks or drawing.
enum VisCat { VisNone = 0, VisNoSlot, VisGateClosed, VisKill, VisNoTable, VisBadCard, VisNoBuffs, VisNoFit, VisDrawn };
constexpr int VisMaxLines = 64;
int g_visLines = 0;
void visLine(const char* fmt, ...) {
    if (!g_debugLog || g_visLines > VisMaxLines) return;
    if (g_visLines == VisMaxLines) { ++g_visLines; logf("VIS cap reached"); return; }
    ++g_visLines;
    char buf[256];
    va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    buf[sizeof(buf) - 1] = 0;
    logf("%s", buf);
}
struct VisSlot {
    int slotPtr, cat, count, rect[4];
    int drawn[2], tracked[2], rc[2][4];       // per column (0 = buff, 1 = debuff): icons drawn / tracked effects / bounding rect
};
VisSlot g_vis[4];                              // K2PS_MAX_SLOTS entries, claimed in first-seen order: slot0, slot1, ...
int visIndex(int slotPtr) {
    if (slotPtr == 0) return -1;
    for (int i = 0; i < 4; ++i) if (g_vis[i].slotPtr == slotPtr) return i;
    for (int i = 0; i < 4; ++i) if (g_vis[i].slotPtr == 0) { g_vis[i] = {}; g_vis[i].slotPtr = slotPtr; return i; }
    return -1;
}
void visSet(int idx, int cat, int count, const int* rect, int flags) {
    VisSlot& v = g_vis[idx];
    static const int zero[4] = {};
    if (!rect) rect = zero;
    if (v.cat == cat && v.count == count && memcmp(v.rect, rect, sizeof(v.rect)) == 0) return;
    v.cat = cat; v.count = count; memcpy(v.rect, rect, sizeof(v.rect));
    char why[48];
    switch (cat) {
        case VisNoSlot:     snprintf(why, sizeof(why), "no-slot"); break;
        case VisGateClosed: snprintf(why, sizeof(why), "gate-closed(flags=%08x)", flags); break;
        case VisKill:       snprintf(why, sizeof(why), "killswitch"); break;
        case VisNoTable:    snprintf(why, sizeof(why), "no-table"); break;
        case VisBadCard:    snprintf(why, sizeof(why), "bad-card"); break;
        case VisNoBuffs:    snprintf(why, sizeof(why), "no-buffs"); break;
        case VisNoFit:      snprintf(why, sizeof(why), "no-fit tracked=%d", count); break;
        default:            snprintf(why, sizeof(why), "drawn count=%d", count); break;
    }
    visLine("VIS slot%d shown=%d why=%s rect=[%d,%d,%d,%d]", idx, cat == VisDrawn ? 1 : 0, why, rect[0], rect[1], rect[2], rect[3]);
}
// Rejections decided before any column is laid out (kill switch, no table, closed gate, no creature).
void visReject(int slotPtr, int cat, int flags) {
    int i = visIndex(slotPtr);
    if (i >= 0) visSet(i, cat, 0, nullptr, flags);
}
// One column of a HUD slot was laid out (drawN icons of `tracked` effects, bounding rect x,y,w,h or null). Merged with the other column.
void visColumn(int slotPtr, bool debuff, int tracked, int drawN, const int* rect) {
    int i = visIndex(slotPtr);
    if (i < 0) return;
    VisSlot& v = g_vis[i];
    const int c = debuff ? 1 : 0;
    v.drawn[c] = drawN; v.tracked[c] = tracked;
    for (int q = 0; q < 4; ++q) v.rc[c][q] = (drawN > 0 && rect) ? rect[q] : 0;
    int l = 0, t = 0, r = 0, b = 0; bool any = false;
    for (int k = 0; k < 2; ++k) {
        if (v.drawn[k] <= 0) continue;
        int x0 = v.rc[k][0], y0 = v.rc[k][1], x1 = x0 + v.rc[k][2], y1 = y0 + v.rc[k][3];
        if (!any) { l = x0; t = y0; r = x1; b = y1; any = true; }
        else { if (x0 < l) l = x0; if (y0 < t) t = y0; if (x1 > r) r = x1; if (y1 > b) b = y1; }
    }
    const int total = v.drawn[0] + v.drawn[1], trk = v.tracked[0] + v.tracked[1];
    const int rc[4] = { l, t, r - l, b - t };
    if (total > 0) visSet(i, VisDrawn, total, rc, 0);
    else visSet(i, trk > 0 ? VisNoFit : VisNoBuffs, trk, nullptr, 0);
}


// ---- shared probe log: OFF unless "shieldprobe.txt" exists in the game dir. ARROWS / ICONCACHE / GLINIT / CAPFIX lines go to
// shieldprobe_log.txt, the same file the shield-bar patch writes (one game thread: lines interleave in call order). ----
bool g_probe = false;
int g_probeLines = 0, g_frame = 0;   // g_frame = the portrait-slots frame counter, copied in every phase callback
constexpr int MaxProbeLines = 20000;
void probeLog(const char* fmt, ...) {
    if (!g_probe || g_probeLines >= MaxProbeLines) return;
    FILE* f = fopen("shieldprobe_log.txt", "a");
    if (!f) return;
    ++g_probeLines;
    fprintf(f, "[t=%lu f=%d] ", GetTickCount(), g_frame);
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

// A feature runs only when the exe's address table has every field it needs (K2Addr.h; null table / 0 = unknown). The first feature
// that finds it unusable logs ONE line per session, from the hook call (KPatchInit must not do file I/O).
bool g_tableNoted = false;
bool featureOk(bool ok, const char* feature) {
    if (!ok && !g_tableNoted) logf("k2addr: %s disabled (no address table for this exe, or a needed field is 0)", feature);
    return k2addr::featureOk(ok, feature, probeLog, &g_tableNoted);   // shared: sets g_tableNoted, writes the K2ADDR probe line once
}

// ---- kill switch (polled every 30 frames, works mid-session): buffhud_off.txt -> skip the icon columns (engine arrows draw natively).
bool g_noBuff = false;
bool fileExists(const char* n) { return GetFileAttributesA(n) != INVALID_FILE_ATTRIBUTES; }
// GL function table + CapGuard live in Gui2D.h; GLINIT / CAPFIX lines come back through gui2d::g_hooks (set in DllMain).
void onGlInit(const gui2d::GlFns& g) {
    probeLog("GLINIT ok=%d readPixels=%d enable/disable=%d", g.ok, g.readPixels != nullptr, g.enable && g.disable);
}
void onCapFix(unsigned cap, int was, int now) { probeLog("CAPFIX cap=%x was=%d now=%d -> restored", cap, was, now); }
} // namespace

extern "C" {

// Hook 2 (per native arrow, buff row): the icon column is drawn by DrawBuffColumn, so hide
// the first `tracked` native arrows (CSWGuiBorder::Draw returns at once when +0xc/+0x10 == 0).
void __cdecl RestoreBuffIcon();
void __cdecl RetargetBuffIcon(int slotPtr, int index, int iconWidget, int whichRow) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
    g_ps.poll();                                // portrait-slots loaded after us (config order): register late
    // The debuff row has no per-arrow restore hook and all its arrows share one widget: un-hide the previous arrow's
    // widget first, so a kept (untracked) arrow after the suppressed ones draws at its real size and the save below
    // records the real size, not the 0x0 left by the previous suppression.
    RestoreBuffIcon();
    if (g_noBuff) return;
    if (iconWidget == 0 || index < 0) return;
    int creature = creatureForSlot(slotPtr);
    static int callNo = 0; ++callNo;
    bool logIt = callNo <= 40;
    if (!creature) { if (logIt) logf("arrow slot=%08x idx=%d: NO creature in side table", slotPtr, index); return; }
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16, whichRow == 0);
    if (logIt) logf("arrow row=%d slot=%08x idx=%d tracked=%d -> %s", whichRow, slotPtr, index, n, bh::arrowSuppressed(index, n) ? "suppress" : "keep");
    if (!bh::arrowSuppressed(index, n)) return; // untracked engine buff: leave its arrow

    const K2AddrTable* T = k2addr::get();
    if (!featureOk(bh::arrowHideFieldsOk(T), "arrow suppression")) return;
    int border = iconWidget + T->Label_Border;
    int w = 0, h = 0, bvt = 0;
    if (!readAt(border, 0, &bvt) || bvt != static_cast<int>(T->BorderVtable) || !readAt(border, T->Border_Width, &w) || !readAt(border, T->Border_Height, &h)) {
        if (logIt) logf("arrow suppression: border %08x unreadable or vtable %08x != %08x", border, bvt, (unsigned)T->BorderVtable);
        return;
    }
    g_saved ={ true, border, 0, w, h, T->Border_Width, T->Border_Height };
    *reinterpret_cast<int*>(border + T->Border_Width) = 0;
    *reinterpret_cast<int*>(border + T->Border_Height) = 0;
}

void __cdecl RestoreBuffIcon() {
    if (!g_saved.active) return;
    g_saved.active = false;
    *reinterpret_cast<int*>(g_saved.border + g_saved.widthOff) = g_saved.width;
    *reinterpret_cast<int*>(g_saved.border + g_saved.heightOff) = g_saved.height;
}

// Hook 5 (once per row call, after the native loop, still inside the card's viewport whose
// origin is the card's top-left): draw our own icon column down the card's left edge.
// Core icon-column drawer: origin = the card's top-left (the caller is inside a clip/viewport at the card). iconWidget may be 0 (menu
// companion portraits have no icon widget): then the draw uses the stock defaults (white tint, alpha 1, no rotation).
static void drawColumnCore(int slotPtr, int creature, int iconWidget, int isLarge, int whichRow, int cardW, int cardH) {
    const bool debuff = whichRow == 0;
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(bh::columnFieldsOk(T), "icon columns")) { if (iconWidget) visReject(slotPtr, VisNoTable, 0); return; }
    static int callNo = 0; ++callNo;
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16], other[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16, debuff);
    int nOther = collectBuffs(creature, day, ms, haveTime, other, 16, !debuff);

    if (cardW < 1 || cardH < 1) { if (iconWidget) visReject(slotPtr, VisBadCard, 0); return; }         // degenerate card (F23): nothing sensible to lay out
    float scale = gui2d::hudScale();
    // both columns share one icon size; preferred size shrunk so all fit down the column, else another column (bh::columnLayout)
    const bh::ColumnLayout lay = bh::columnLayout(n, nOther, isLarge, cardW, cardH, scale);
    const int gap = lay.gap, size = lay.size, cols = lay.cols, fit = lay.fit, drawN = lay.drawN;

    static bh::ColumnLogGate colGate;           // keyed by (slot, debuff): the two columns of a slot alternate and must not thrash one cache entry
    int sig = bh::columnSig(n, debuff, nOther, size, cardW, cardH, isLarge);
    const bool sigChanged = colGate.shouldLog(slotPtr, debuff ? 1 : 0, sig);   // always evaluated so the table stays current during the first 30 calls
    bool logIt = callNo <= 30 || sigChanged;
    if (logIt) logf("COLUMN#%d %s slot=%08x large=%d card=%dx%d scale=%.3f tracked=%d cols=%d fit=%d draw=%d size=%d gap=%d",
                    callNo, debuff ? "DEBUFF" : "buff", slotPtr, isLarge, cardW, cardH, scale, n, cols, fit, drawN, size, gap);
    if (iconWidget) {                           // HUD slot (menu companion columns have iconWidget 0 and are traced in onMenuOverlay)
        int vr[4] = {}, *vrp = nullptr;
        if (drawN > 0 && lay.perCol > 0) {
            const int colsUsed = (drawN - 1) / lay.perCol + 1, rows = drawN < lay.perCol ? drawN : lay.perCol;
            const bh::IconPos p0 = bh::iconPos(lay, 0, debuff, cardW), pc = bh::iconPos(lay, (colsUsed - 1) * lay.perCol, debuff, cardW);
            const bh::IconPos pr = bh::iconPos(lay, rows - 1, debuff, cardW);
            const int x0 = p0.x < pc.x ? p0.x : pc.x, x1 = (p0.x > pc.x ? p0.x : pc.x) + size;
            vr[0] = x0; vr[1] = p0.y; vr[2] = x1 - x0; vr[3] = pr.y + size - p0.y; vrp = vr;
        }
        visColumn(slotPtr, debuff, n, drawN, vrp);
    }
    if (drawN <= 0) return;

    CapGuard capGuard;                          // same GL_LIGHTING leak as the shield bar: flush/pushClip toggle caps, put them back on exit
    int border = iconWidget ? iconWidget + T->Label_Border : 0;
    static const float whiteTint[3] = { 1.0f, 1.0f, 1.0f };
    int rotBits = 0; if (border) readAt(border, T->Border_Rot, &rotBits);
    const float* color = border ? reinterpret_cast<const float*>(border + T->Border_Tint) : whiteTint;
    float baseAlpha = 1.0f; if (border) readAt(border, T->Border_Alpha, &baseAlpha);
    const float flushArg = gui2d::flushArg();

    auto draw2 = [&](void* img, int x, int y, int w, int h, const float* tint, float alpha) {
        gui2d::drawImage(img, x, y, w, h, rotBits, tint, alpha);       // image vtable slot ImageVt_Draw, a5 = 0
    };
    gui2d::flush(flushArg);
    for (int i = 0; i < drawN; ++i) {
        void* image = imageFor(buffs[i].resref);
        if (!image) { if (logIt) logf("  icon[%d] %s: no image", i, buffs[i].resref); continue; }
        const bh::IconPos pos = bh::iconPos(lay, i, debuff, cardW);
        int ix = pos.x, iy = pos.y;
        float ratio = buffs[i].ratio;
        int bright = bh::brightPx(size, ratio);
        if (logIt) logf("  icon[%d] %s rect=(%d,%d,%d,%d) ratio=%.3f bright=%d dimAlpha=%.2f", i, buffs[i].resref, ix, iy, size, size, ratio, bright, DimAlpha);
        void* disc = buffs[i].item ? imageFor(CircleResref) : nullptr;   // round backing for item abilities only
        // elapsed part (whole square, dim): backing disc + icon
        if (disc) draw2(disc, ix, iy, size, size, BgColor, BgAlpha * DimAlpha);
        draw2(image, ix, iy, size, size, color, baseAlpha * DimAlpha);
        if (bright > 0) {                                                               // remaining part, bottom-anchored
            int y0 = iy + size - bright;
            int ok = gui2d::pushClip(ix, y0, size, bright);
            if (ok) {
                if (disc) draw2(disc, 0, -(size - bright), size, size, BgColor, BgAlpha);
                draw2(image, 0, -(size - bright), size, size, color, baseAlpha);
                gui2d::popClip();
                if (disc && bright < size) {                                                     // 1px boundary line (round item icons only)
                    // item icons: only inside the round backing
                    const bh::DiscLine dl = bh::discLine(ix, iy, size, y0);
                    int lx = dl.x, lw = dl.w;
                    if (lw > 0 && gui2d::pushClip(lx, y0, lw, 1, LineColor, LineAlpha)) gui2d::popClip();
                }
            } else if (logIt) logf("  icon[%d] clip push refused", i);
        }
    }
    gui2d::flush(flushArg);
}

void __cdecl DrawBuffColumn(int slotPtr, int iconWidget, int isLarge, int whichRow, int cardW, int cardH) {
    static bool visFirst = true; if (visFirst) { visFirst = false; visLine("VIS hud column first call"); }
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
    g_ps.poll();                                // portrait-slots loaded after us (config order): register late
    RestoreBuffIcon();                          // debuff arrows are hidden without a restore hook (also when the kill switch flips mid-row)
    if (g_noBuff) return;
    if (iconWidget == 0) return;
    int creature = creatureForSlot(slotPtr);
    if (!creature) return;
    drawColumnCore(slotPtr, creature, iconWidget, isLarge, whichRow, cardW, cardH);
}

// ---- engine arrow counter bump ----------------------------------------------------------
// The engine only runs the row draw (FUN_007457e0, where hook 5 draws our column) when its own arrow counter
// slot+0xddc (buff) / +0xddd (debuff) is non-zero. FUN_00744f00 fills them from a client-side effect-icon cache at
// creature+0xFFC, NOT from the +0x148 effect list we read, so a creature can hold tracked effects and still be at 0
// (doc 02 "Engine arrow counter root cause"). FUN_00745770 reads the counters right after hook 6, so raise them
// here: native code then draws that many identical arrows and hooks 2/3/5 hide/replace them as usual.
using bh::ArrowRaw;
ArrowRaw g_arrowRaw[8];

ArrowRaw* arrowRawFor(int slotPtr) { return bh::arrowRawFor(g_arrowRaw, 8, slotPtr); }

// DIAG DBGMISS (debuff triage, doc 02): the engine flags a debuff (raw debuff byte > 0) but the spell-id walk tracks none. Dump every effect on
// the creature (spell id incl. -1, type, subtype, duration, creator, icon resolution) once per creature per change of its effect list, so the
// spell id and source behind an unmapped debuff (power vs grenade vs mine vs script effect) lands in buffhud_log.txt. Remove after triage.
struct MissSeen { int creature; int sig; };
MissSeen g_missSeen[16];
int g_missN = 0, g_missLogged = 0;
void logUntrackedDebuff(int creature, int rawDebuff) {
    int arr = 0, count = 0;
    const bool ok = k2fx::readEffectList(static_cast<uintptr_t>(creature), &arr, &count);
    int sig = count;
    for (int i = 0; ok && arr && i < count && i < k2fx::MaxEffects; ++i) {
        int eff = 0, spell = -1;
        if (readAt(arr, i * 4, &eff) && eff && readAt(eff, k2fx::Effect_SpellId, &spell)) sig = sig * 31 + spell;
    }
    int slot = -1;
    for (int k = 0; k < g_missN; ++k) if (g_missSeen[k].creature == creature) slot = k;
    if (slot >= 0 && g_missSeen[slot].sig == sig) return;
    if (slot < 0) { if (g_missN >= 16) return; slot = g_missN++; g_missSeen[slot].creature = creature; }
    if (g_missLogged >= 40) return;
    g_missSeen[slot].sig = sig;
    ++g_missLogged;
    logf("DBGMISS creature=%08x raw=%d listOk=%d count=%d", creature, rawDebuff, ok ? 1 : 0, count);
    for (int i = 0, lines = 0; ok && arr && i < count && i < k2fx::MaxEffects && lines < 12; ++i) {
        int eff = 0, spell = -1, creator = 0; unsigned short type = 0, sub = 0; float dur = 0.0f; bool item = false;
        if (!readAt(arr, i * 4, &eff) || !eff) continue;
        readAt(eff, k2fx::Effect_SpellId, &spell); readAt(eff, k2fx::Effect_Type, &type); readAt(eff, k2fx::Effect_Subtype, &sub);
        readAt(eff, k2fx::Effect_Duration, &dur); readAt(eff, k2fx::Effect_Creator, &creator);
        const char* dRes = k2fx::iconForSpell(spell, true, &item);
        const char* bRes = k2fx::iconForSpell(spell, false, &item);
        logf("DBGMISS   i=%d spell=%d type=%d sub=%d dur=%.1f creator=%08x debuffIcon=%s buffIcon=%s", i, spell, type, sub, dur, creator, dRes ? dRes : "-", bRes ? bRes : "-");
        ++lines;
    }
}

void ForceArrowCounters(int slotPtr) {
    if (g_noBuff) { visReject(slotPtr, VisKill, 0); return; }
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(bh::arrowFieldsOk(T), "arrow counters")) { visReject(slotPtr, VisNoTable, 0); return; }
    int gate = 0;
    if (!readAt(slotPtr, T->Slot_Flags, &gate) || !bh::arrowGateOpen(gate)) { visReject(slotPtr, VisGateClosed, gate); return; }              // engine's own "row visible" bit (also excludes the creature-less mirror slot)
    unsigned char rawBuff = 0, rawDebuff = 0;
    if (!readAt(slotPtr, T->Slot_ArrowBuff, &rawBuff) || !readAt(slotPtr, T->Slot_ArrowDebuff, &rawDebuff)) return;
    int creature = creatureForSlot(slotPtr);
    if (!creature) { visReject(slotPtr, VisNoSlot, 0); return; }
    ArrowRaw* raw = arrowRawFor(slotPtr);
    int overlay[3] = {};                                                    // FUN_00744f00: mask&2 -> +0x1b8, mask&1 -> +0x300, mask&4 -> +0x448 (bit 1 = shown)
    readAt(slotPtr, T->Slot_Overlay0, &overlay[0]); readAt(slotPtr, T->Slot_Overlay1, &overlay[1]); readAt(slotPtr, T->Slot_Overlay2, &overlay[2]);
    bool hidden = bh::arrowsHidden(overlay[0], overlay[1], overlay[2]);      // engine deliberately hid the arrows (dead/disabled): keep that
    int nBuff = 0, nDebuff = 0;
    if (!hidden) {
        int day = 0, ms = 0;
        bool haveTime = currentTime(&day, &ms);
        Buff tmp[16];
        nBuff = collectBuffs(creature, day, ms, haveTime, tmp, 16, false);
        nDebuff = collectBuffs(creature, day, ms, haveTime, tmp, 16, true);
        if (g_debugLog && rawDebuff > 0 && nDebuff == 0) logUntrackedDebuff(creature, rawDebuff);
    }
    {   // gate open again after a rejection: nothing tracked means no column call will follow, so close the stale state here
        const int vi = visIndex(slotPtr);
        if (vi >= 0 && g_vis[vi].cat != VisNone && g_vis[vi].cat < VisNoBuffs && nBuff + nDebuff == 0) visSet(vi, VisNoBuffs, 0, nullptr, 0);
    }
    const bh::ArrowCounters ctr = bh::bumpArrowCounters(rawBuff, rawDebuff, nBuff, nDebuff, hidden);   // never lowers an engine value
    unsigned char newBuff = (unsigned char)ctr.buff, newDebuff = (unsigned char)ctr.debuff;
    if (newBuff != rawBuff) *reinterpret_cast<unsigned char*>(slotPtr + T->Slot_ArrowBuff) = newBuff;
    if (newDebuff != rawDebuff) *reinterpret_cast<unsigned char*>(slotPtr + T->Slot_ArrowDebuff) = newDebuff;
    if (raw) {
        bool changed = bh::arrowRawUpdate(raw, rawBuff, rawDebuff, nBuff, nDebuff);
        if (changed) {
            logf("ARROWS slot=%08x creature=%08x raw=(%d,%d) tracked=(%d,%d) -> counters=(%d,%d) overlay=(%08x,%08x,%08x)%s",
                 slotPtr, creature, rawBuff, rawDebuff, nBuff, nDebuff, newBuff, newDebuff, overlay[0], overlay[1], overlay[2], hidden ? " HIDDEN" : "");
            probeLog("ARROWS slot=%08x creature=%08x raw=(%d,%d) tracked=(%d,%d) -> counters=(%d,%d) overlay=(%08x,%08x,%08x) gate590=%08x%s",
                     slotPtr, creature, rawBuff, rawDebuff, nBuff, nDebuff, newBuff, newDebuff, overlay[0], overlay[1], overlay[2], gate, hidden ? " HIDDEN" : "");
            // Phase 0: the engine's own count comes from the creature's icon cache (Creature_IconCacheData / Creature_IconCacheCount).
            // Diagnostic only: skipped when the table does not know those offsets.
            int cacheData = 0, cacheCount = 0;
            const bool haveCache = bh::iconCacheFieldsOk(T);
            if (haveCache) { readAt(creature, T->Creature_IconCacheData, &cacheData); readAt(creature, T->Creature_IconCacheCount, &cacheCount); }
            char ent[256]; int len = 0; ent[0] = 0;
            for (int k = 0; k < 6 && k < cacheCount && cacheData; ++k) {
                int e = 0; short id = 0; int f14 = 0, f1c = 0;
                if (!readAt(cacheData, k * 4, &e) || !e) continue;
                readAt(e, 0, &id); readAt(e, T->IconEnt_F14, &f14); readAt(e, T->IconEnt_F1c, &f1c);
                len = bh::appendf(ent, (int)sizeof(ent), len, " [%d id=%d f14=%d f1c=%d]", k, id, f14, f1c);
            }
            probeLog("ICONCACHE slot=%08x creature=%08x count=%d%s", slotPtr, creature, cacheCount, ent);
        }
    }
}

// ---- phase callbacks ---------------------------------------------------------------------------------------------------------------
void flushConnectNote() {
    if (!g_connectNote) return;
    probeLog("PORTRAITSLOTS %s: %s", g_connectOk ? "ok" : "MISSING", g_connectNote);
    logf("portrait-slots %s: %s", g_connectOk ? "ok" : "MISSING", g_connectNote);
    g_connectNote = nullptr;
}

static void KPATCH_CALL onPreEngineRead(const K2PsContext* c, void*) {
    g_frame = int(c->frame);
    ForceArrowCounters(c->slot);                // before FUN_00745770 reads +0xddc/+0xddd; every slot, not just the leader
}

static void KPATCH_CALL onFrameBegin(const K2PsContext* c, void*) {
    g_frame = int(c->frame);
    flushConnectNote();
    if (g_frame % 30 == 0) {
        g_noBuff = fileExists("buffhud_off.txt");
        static int lastFlags = -1; int fl = g_noBuff;
        if (fl != lastFlags) { lastFlags = fl; probeLog("FLAGS nobuff=%d", g_noBuff); }
    }
}

// Menu screens: the companion portraits (LBL_CHAR2/3, plain images without icon widgets) get the icon columns. The provider has reopened
// the 2D batch and pushed the panel clip (panel-local coordinates); the shield bar (prio 100) has already moved and decorated them.
static void KPATCH_CALL onMenuOverlay(const K2PsContext* c, void*) {
    g_frame = int(c->frame);
    static bool visFirst = true; if (visFirst) { visFirst = false; visLine("VIS overlay first call"); }
    if (g_noBuff) return;
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(bh::menuFieldsOk(T), "menu companion columns")) return;
    const int rectOff[4] = { T->Ctl_X, T->Ctl_Y, T->Ctl_W, T->Ctl_H };      // control extent: x, y, w, h
    int cols = 0;
    static int visSt[2] = { -1, -1 }, visRect[2][4] = {};
    for (int k = 0; k < 2; ++k) {
        int comp = c->companions[k], ctrl = c->companionCtrl[k], fl = 0, r[4] = {};
        int st = 0; const char* why = "no-slot";
        if (comp) {
            if (!readAt(ctrl, T->Ctl_Flags, &fl) || !bh::menuPortraitVisible(fl)) { st = 1; why = "portrait-hidden"; }
            else {
                for (int q = 0; q < 4; ++q) readAt(ctrl, rectOff[q], &r[q]);
                if (!bh::menuRectOk(r[2], r[3])) { st = 2; why = "bad-rect"; }
                else if (gui2d::pushClip(r[0], r[1], r[2], r[3])) {
                    drawColumnCore(ctrl, comp, 0, 0, 1, r[2], r[3]);     // buffs: left edge
                    drawColumnCore(ctrl, comp, 0, 0, 0, r[2], r[3]);     // debuffs: right edge
                    gui2d::popClip(); ++cols;
                    st = 3; why = "drawn";
                } else { st = 4; why = "clip-refused"; }
            }
        }
        const int vr[4] = { st >= 2 ? r[0] : 0, st >= 2 ? r[1] : 0, st >= 2 ? r[2] : 0, st >= 2 ? r[3] : 0 };
        if (st != visSt[k] || memcmp(vr, visRect[k], sizeof(vr)) != 0) {
            visSt[k] = st; memcpy(visRect[k], vr, sizeof(vr));
            visLine("VIS menu%d shown=%d why=%s rect=[%d,%d,%d,%d]", k, st == 3 ? 1 : 0, why, vr[0], vr[1], vr[2], vr[3]);
        }
    }
    static int lastCols = -2;
    if (cols != lastCols) { lastCols = cols; probeLog("MENUCOLS companionPortraits=%d", cols); }
}

void __cdecl KPatchInit(const KPatchApi* api) {
    g_ps.init(api, "buff-duration-hud");
    g_ps.onConnect = onConnect;
    g_ps.add(K2PS_PRE_ENGINE_READ, K2PS_PRIO_BUFF, onPreEngineRead);
    g_ps.add(K2PS_FRAME_BEGIN, K2PS_PRIO_BUFF, onFrameBegin);
    g_ps.add(K2PS_MENU_OVERLAY, K2PS_PRIO_BUFF, onMenuOverlay);
    g_ps.connect();
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_debugLog = GetFileAttributesA("buffhud_debug.txt") != INVALID_FILE_ATTRIBUTES;
        g_probe = GetFileAttributesA("shieldprobe.txt") != INVALID_FILE_ATTRIBUTES;
        saferead::enableRing(g_probe);
        gui2d::g_hooks.onGlInit = onGlInit;
        gui2d::g_hooks.onCapFix = onCapFix;
        logf("buffhud loaded");
    }
    return TRUE;
}
