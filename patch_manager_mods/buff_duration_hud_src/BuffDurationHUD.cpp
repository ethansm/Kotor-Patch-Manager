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
    if (ok) return true;
    if (!g_tableNoted) {
        g_tableNoted = true;
        logf("k2addr: %s disabled (no address table for this exe, or a needed field is 0)", feature);
        probeLog("K2ADDR %s disabled (no address table for this exe, or a needed field is 0)", feature);
    }
    return false;
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
    if (!readAt(border, 0, &bvt) || bvt != static_cast<int>(T->BorderVtable) || !readAt(border, T->Border_Width, &w) || !readAt(border, T->Border_Height, &h)) return;
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
    if (!featureOk(bh::columnFieldsOk(T), "icon columns")) return;
    static int callNo = 0; ++callNo;
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16], other[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16, debuff);
    int nOther = collectBuffs(creature, day, ms, haveTime, other, 16, !debuff);

    float scale = gui2d::hudScale();
    // both columns share one icon size; preferred size shrunk so all fit down the column, else another column (bh::columnLayout)
    const bh::ColumnLayout lay = bh::columnLayout(n, nOther, isLarge, cardW, cardH, scale);
    const int gap = lay.gap, size = lay.size, cols = lay.cols, fit = lay.fit, drawN = lay.drawN;

    static int lastSig[8]; static int sigSlot[8]; static int nSig = 0;
    int si = -1; for (int k = 0; k < nSig; ++k) if (sigSlot[k] == slotPtr) si = k;
    if (si < 0 && nSig < 8) { si = nSig++; sigSlot[si] = slotPtr; lastSig[si] = -1; }
    int sig = bh::columnSig(n, debuff, nOther, size, cardW, cardH, isLarge);
    bool logIt = callNo <= 30 || (si >= 0 && lastSig[si] != sig);
    if (si >= 0) lastSig[si] = sig;
    if (logIt) logf("COLUMN#%d %s slot=%08x large=%d card=%dx%d scale=%.3f tracked=%d cols=%d fit=%d draw=%d size=%d gap=%d",
                    callNo, debuff ? "DEBUFF" : "buff", slotPtr, isLarge, cardW, cardH, scale, n, cols, fit, drawN, size, gap);
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
                    int lx = ix, lw = size;
                    if (disc) {                                     // item icons: only inside the round backing
                        const bh::DiscLine dl = bh::discLine(ix, iy, size, y0);
                        lw = dl.w;
                        lx = dl.x;
                    }
                    if (lw > 0 && gui2d::pushClip(lx, y0, lw, 1, LineColor, LineAlpha)) gui2d::popClip();
                }
            } else if (logIt) logf("  icon[%d] clip push refused", i);
        }
    }
    gui2d::flush(flushArg);
}

void __cdecl DrawBuffColumn(int slotPtr, int iconWidget, int isLarge, int whichRow, int cardW, int cardH) {
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

void ForceArrowCounters(int slotPtr) {
    if (g_noBuff) return;
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(bh::arrowFieldsOk(T), "arrow counters")) return;
    int gate = 0;
    if (!readAt(slotPtr, T->Slot_Flags, &gate) || !bh::arrowGateOpen(gate)) return;              // engine's own "row visible" bit (also excludes the creature-less mirror slot)
    unsigned char rawBuff = 0, rawDebuff = 0;
    if (!readAt(slotPtr, T->Slot_ArrowBuff, &rawBuff) || !readAt(slotPtr, T->Slot_ArrowDebuff, &rawDebuff)) return;
    int creature = creatureForSlot(slotPtr);
    if (!creature) return;
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
    if (g_noBuff) return;
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(bh::menuFieldsOk(T), "menu companion columns")) return;
    const int rectOff[4] = { T->Ctl_X, T->Ctl_Y, T->Ctl_W, T->Ctl_H };      // control extent: x, y, w, h
    int cols = 0;
    for (int k = 0; k < 2; ++k) {
        int comp = c->companions[k], ctrl = c->companionCtrl[k], fl = 0, r[4] = {};
        if (!comp || !readAt(ctrl, T->Ctl_Flags, &fl) || !bh::menuPortraitVisible(fl)) continue;
        for (int q = 0; q < 4; ++q) readAt(ctrl, rectOff[q], &r[q]);
        if (!bh::menuRectOk(r[2], r[3])) continue;
        if (gui2d::pushClip(r[0], r[1], r[2], r[3])) {
            drawColumnCore(ctrl, comp, 0, 0, 1, r[2], r[3]);     // buffs: left edge
            drawColumnCore(ctrl, comp, 0, 0, 0, r[2], r[3]);     // debuffs: right edge
            gui2d::popClip(); ++cols;
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
