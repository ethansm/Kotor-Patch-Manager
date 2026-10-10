// Shield Bar (Steam Aspyr build 6A522E71...). Mod 4: a shield absorb bar beside each party portrait's health bar (segment per
// shield, coloured like the shield's in-world VFX, draining as it absorbs), the same bars + health/force gauges for the companion
// portraits on the in-game menu screens, and "Shield: cur/max" in the native portrait hover tooltip.
// Split out of buff-duration-hud v1.1.0 (commit 17b508d) in doc 21 W4: the shared portrait hooks now belong to the portrait-slots
// patch ("k2.portraitslots" v1, PortraitSlots.h); this DLL registers phase callbacks there and owns only hook 7.
// Research trail: patch_manager_mods/08_shield_bar.md; split design: 21_portrait_slots_and_split.md (local-setup/KOTOR-II worktree).
//
//   hook 7  0x00744EEC  portrait tooltip builder RET path: append the shield line
//   phases  SNAPSHOT (probe), FRAME_BEGIN (kill switch, F9 mark, scene probe), SLOT (leader force-bar slide FORCEMOVE + SLOT log),
//           MENU_OVERLAY prio 100 (top-bar bar, companion gauges + bars, portrait spacing), HUD_DRAW prio 100 (all HUD bars)
// Without portrait-slots (or with a patcher that has no registry) only the tooltip hook runs and it finds no creature: inert.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "ShieldTable.inc"
#include "ShieldLogic.h"     // sb::appendf (bounded log formatting)
#include "SafeRead.h"
#include "K2Addr.h"
#include "Effects.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "PortraitSlots.h"


namespace {

using guikit::imageFor;
using gui2d::CapGuard;
constexpr int MaxEffects = k2fx::MaxEffects;
// Every engine address / struct offset below comes from the exe's address table (K2Addr.h, k2addr::get()): CExoStringFormat,
// GetMaxForcePoints, Slot_*, Menu_*, Creature_*, Stats_*, Tip_*, Ctl_*, Effect_*. A null table or a 0 field = that feature does nothing.

template <typename T> bool readAt(int base, int off, T* out) {
    return saferead::readAt(1, static_cast<uintptr_t>(base), off, out);
}
// control extent (x, y, w, h) at the table's Ctl_X / Ctl_Y / Ctl_W / Ctl_H; false = offsets unknown (r untouched)
bool readCtlRect(int ctl, int* r) {
    const K2AddrTable* T = k2addr::get();
    if (!T || !T->Ctl_X || !T->Ctl_Y || !T->Ctl_W || !T->Ctl_H) return false;
    const int o[4] = { T->Ctl_X, T->Ctl_Y, T->Ctl_W, T->Ctl_H };
    for (int q = 0; q < 4; ++q) readAt(ctl, o[q], &r[q]);
    return true;
}

k2ps::Client g_ps;
const char* g_connectNote = nullptr;   // set in KPatchInit (no file I/O allowed there), written to the probe log on the first frame
bool g_connectOk = false;
void onConnect(bool ok, const char* why) { g_connectOk = ok; g_connectNote = why; }

// ---- Shield Bar PROBE (1.1.0-probe): OFF unless "shieldprobe.txt" exists in the game dir.
// Writes shieldprobe_log.txt: effect APPLY/CHANGE/REMOVE per creature (S1/S2/S4) and the
// portrait slot rects + test rects drawn at the new every-frame draw site (R-D/S6/S7). ----
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

// A feature runs only when the exe's address table has every field it needs (K2Addr.h; null table / 0 = unknown). The first feature that
// finds it unusable writes ONE probe-log line per session, from the hook call (KPatchInit must not do file I/O).
bool g_tableNoted = false;
bool featureOk(bool ok, const char* feature) {
    return k2addr::featureOk(ok, feature, probeLog, &g_tableNoted);
}

struct EffRec { int n34; int eff; unsigned short type, sub; int spell, creator; float dur; int ints[8]; bool seen; };
constexpr int ProbeMaxEff = 128, ProbeMaxCreatures = 8;
struct CreatureRec { int creature; unsigned lastTick; bool primed; int n; EffRec e[ProbeMaxEff]; };
CreatureRec g_cr[ProbeMaxCreatures];

void probeDump(int eff, char* out, int cap) {
    (void)eff;
    if (cap > 0) out[0] = 0;
}

void probeEffects(int slotPtr, int creature) {
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(sb::effectProbeFieldsOk(T), "effect probe")) return;
    CreatureRec* cr = nullptr;
    for (auto& c : g_cr) if (c.creature == creature) { cr = &c; break; }
    if (!cr) for (auto& c : g_cr) if (c.creature == 0) { cr = &c; c.creature = creature; break; }
    if (!cr) return;
    unsigned now = GetTickCount();
    if (cr->lastTick == now) return;            // the mirror slot repeats slot 0's creature
    cr->lastTick = now;
    int arr = 0, count = 0;
    if (!readAt(creature, T->Creature_EffectArray, &arr) || !readAt(creature, T->Creature_EffectCount, &count)) return;
    if (arr == 0 || count < 0 || count > MaxEffects) return;
    for (int i = 0; i < cr->n; ++i) cr->e[i].seen = false;
    for (int i = 0; i < count; ++i) {
        int eff;
        if (!readAt(arr, i * 4, &eff) || eff == 0) continue;
        EffRec cur = {}; cur.eff = eff;
        readAt(eff, T->Effect_Type, &cur.type); readAt(eff, T->Effect_Subtype, &cur.sub); readAt(eff, T->Effect_SpellId, &cur.spell);
        readAt(eff, T->Effect_Creator, &cur.creator); readAt(eff, T->Effect_Duration, &cur.dur);
        readAt(eff, T->Effect_IntCount, &cur.n34);  // Effect_IntCount = count, Effect_Ints = int* array (probe 2 log: dump shows 08 00 00 00 | ptr)
        { int ip = 0; if (readAt(eff, T->Effect_Ints, &ip) && ip) for (int k = 0; k < 8 && k < (cur.n34 > 0 ? cur.n34 : 8); ++k) readAt(ip, 4 * k, &cur.ints[k]); }
        EffRec* old = nullptr;
        for (int k = 0; k < cr->n; ++k) if (cr->e[k].eff == eff) { old = &cr->e[k]; break; }
        if (!old) {
            if (cr->n >= ProbeMaxEff) continue;
            char dump[1400]; probeDump(eff, dump, sizeof(dump));
            { int ip = 0, len = (int)strlen(dump); if (readAt(eff, T->Effect_Ints, &ip) && ip) { len = sb::appendf(dump, (int)sizeof(dump), len, "\n    arr@%08x:", ip); for (int k = 0; k < 16; ++k) { int v = 0; if (!readAt(ip, 4 * k, &v)) break; len = sb::appendf(dump, (int)sizeof(dump), len, " %d", v); } } }
            probeLog("APPLY%s n34=%d c=%08x slot=%08x e=%08x type=0x%04x sub=0x%04x spell=%d creator=%08x dur=%.2f ints=[%d,%d,%d,%d,%d,%d,%d,%d]%s",
                     cr->primed ? "" : "(pre)", cur.n34, creature, slotPtr, eff, cur.type, cur.sub, cur.spell, cur.creator, cur.dur,
                     cur.ints[0], cur.ints[1], cur.ints[2], cur.ints[3], cur.ints[4], cur.ints[5], cur.ints[6], cur.ints[7], dump);
            cur.seen = true; cr->e[cr->n++] = cur;
        } else {
            bool chg = old->type != cur.type || old->sub != cur.sub || memcmp(old->ints, cur.ints, sizeof(cur.ints)) != 0;
            if (chg)
                probeLog("CHANGE c=%08x e=%08x spell=%d type 0x%04x->0x%04x sub 0x%04x->0x%04x ints [%d,%d,%d,%d,%d,%d,%d,%d]->[%d,%d,%d,%d,%d,%d,%d,%d]",
                         creature, eff, cur.spell, old->type, cur.type, old->sub, cur.sub,
                         old->ints[0], old->ints[1], old->ints[2], old->ints[3], old->ints[4], old->ints[5], old->ints[6], old->ints[7],
                         cur.ints[0], cur.ints[1], cur.ints[2], cur.ints[3], cur.ints[4], cur.ints[5], cur.ints[6], cur.ints[7]);
            *old = cur; old->seen = true;
        }
    }
    for (int k = 0; k < cr->n;) {
        if (!cr->e[k].seen) {
            probeLog("REMOVE c=%08x e=%08x type=0x%04x sub=0x%04x spell=%d lastints=[%d,%d,%d,%d,%d,%d,%d,%d]", creature, cr->e[k].eff,
                     cr->e[k].type, cr->e[k].sub, cr->e[k].spell, cr->e[k].ints[0], cr->e[k].ints[1], cr->e[k].ints[2], cr->e[k].ints[3],
                     cr->e[k].ints[4], cr->e[k].ints[5], cr->e[k].ints[6], cr->e[k].ints[7]);
            cr->e[k] = cr->e[--cr->n];
        } else ++k;
    }
    cr->primed = true;
}



void flushConnectNote() {
    if (!g_connectNote) return;
    probeLog("PORTRAITSLOTS %s: %s", g_connectOk ? "ok" : "MISSING", g_connectNote);
    g_connectNote = nullptr;
}

// ---- Shield bar drawing (probe2): the vitality bar's own curved texture, tinted per pool ----
constexpr float BarShiftFraction = 0.328f;            // opaque x-span of uibit_bar_vp_p (0.445..0.773) = 0.328 of the rect
const float TrackTint[3] = { 0.051f, 0.349f, 0.271f }; // the engine's empty-track teal
constexpr bool DrawSeam = false;                      // user (10-01): keep the health bar's own curved left highlight, no seam between
constexpr int MaxSegments = 3;
constexpr float ShieldWidthFraction = 0.55f;          // of the health bar rect width
// Fill = the shield's VFX hue with its saturation cut (HSV, hue and value kept): red -> salmon, blue -> pale blue. Saturation is
// scaled by ShieldSatScale but never below ShieldMinSat (so already-pale hues do not wash out to white) and never raised.
constexpr float ShieldSatScale = 0.50f, ShieldMinSat = 0.30f;
const float SeamTint[3] = { 0.0f, 0.16f, 0.14f };           // dark-teal groove between shield and health
const char* const ShieldBarTexture = "uibit_bar_sh_p";     // curved outer-left, outline-free right edge (scripts/gen_shield_bar_art.py)
const char* const CornerTexture = "uibit_bar_sc_p";          // wedge that fills the card-frame corner notch (drawn on the health rect)
#define HEX(r, g, b) { r / 255.0f, g / 255.0f, b / 255.0f }
struct Pool { int eff; int remaining; int max; const float* rgb; };
struct MaxRec { int creature, eff, max, stamp, lastLogged, spell, last; };
MaxRec g_max[64];
int g_maxStamp = 0;

// colour of the shield's own VFX row (ShieldTable.inc, generated); unknown VFX (droid rows): by coverage
const float* shieldColor(int vfx, int damageFlags) {
    for (const auto& r : kShieldRows) if (r.vfx == vfx) return r.rgb;
    static const float red[3] = HEX(0xFF, 0x0A, 0x0A), blue[3] = HEX(0x4C, 0xAC, 0xFF);
    return (damageFlags & (2048 | 1024)) ? blue : red;    // ion/sonic coverage = enviro blue, else energy red
}

// type-0x02 sub&8 effects = shield pools (remaining = ints[2]); colour key = sibling type-0x1e ints[0]
int collectPools(int creature, Pool* out, int maxOut) {
    int arr = 0, count = 0;
    if (!k2fx::effectListNonEmpty(creature, &arr, &count)) return 0;
    ++g_maxStamp;
    k2fx::PoolRaw raw[16];
    int nRaw = k2fx::scanShieldPools(creature, raw, maxOut < 16 ? maxOut : 16);
    int n = 0;
    for (int i = 0; i < nRaw; ++i) {
        const int eff = raw[i].eff, spell = raw[i].spell, flags = raw[i].flags, remaining = raw[i].remaining, vfx = raw[i].vfx;
        MaxRec* m = nullptr; MaxRec* freeSlot = nullptr;
        for (auto& r : g_max) { if (r.creature == creature && r.eff == eff) { m = &r; break; } if (!freeSlot && r.stamp < g_maxStamp - 300) freeSlot = &r; }
        if (!m && freeSlot) { m = freeSlot; *m = { creature, eff, remaining, 0, -1, spell, remaining }; }
        int mx = remaining;
        if (m) {                                               // effect objects get reused: a pool never refills, so a bigger
            if (m->spell != spell || remaining > m->last) m->max = remaining;   // value or another spell = a NEW shield
            m->spell = spell; m->last = remaining; m->stamp = g_maxStamp; mx = m->max;
        }
        out[n++] = { eff, remaining, mx, shieldColor(vfx, flags) };
        if (m && m->lastLogged != remaining) {
            m->lastLogged = remaining;
            probeLog("POOL c=%08x e=%08x spell=%d vfx=%d flags=%d remaining=%d max=%d", creature, eff, spell, vfx, flags, remaining, mx);
        }
    }
    return n;
}


// "Shield: 32/40" (several pools: "Shield: 32/40 + 10/25") for the portrait hover tooltip; returns the pool count, out = "" when none
int describePools(int creature, char* out, int cap) {
    if (cap <= 0) return 0;
    out[0] = 0;
    Pool pools[8];
    int n = collectPools(creature, pools, 8);
    if (n <= 0) return 0;
    int len = snprintf(out, cap, "Shield: ");
    for (int i = 0; i < n && len > 0 && len < cap; ++i)
        len = sb::appendf(out, cap, len, "%s%d/%d", i ? " + " : "", pools[i].remaining, pools[i].max);
    return n;
}

void lightenFill(const float* rgb, float* out) {
    float mx = rgb[0] > rgb[1] ? (rgb[0] > rgb[2] ? rgb[0] : rgb[2]) : (rgb[1] > rgb[2] ? rgb[1] : rgb[2]);
    float mn = rgb[0] < rgb[1] ? (rgb[0] < rgb[2] ? rgb[0] : rgb[2]) : (rgb[1] < rgb[2] ? rgb[1] : rgb[2]);
    float sat = mx > 0.0f ? (mx - mn) / mx : 0.0f;
    float target = sat * ShieldSatScale; if (target < ShieldMinSat) target = ShieldMinSat; if (target > sat) target = sat;
    float k = sat > 0.0001f ? target / sat : 1.0f;            // saturation scale; value (max channel) unchanged
    for (int c = 0; c < 3; ++c) out[c] = mx - (mx - rgb[c]) * k;
}

int DrawShieldPools(int creature, int vx, int vy, int vw, int vh, float scale, int slotPtr = 0, bool noClip = false) {
    if (!featureOk(sb::drawFieldsOk(k2addr::get()), "shield bar draw")) return 0;
    Pool pools[8];
    int n = collectPools(creature, pools, 8);
    if (n <= 0) return 0;
    void* image = imageFor(ShieldBarTexture);
    if (!image) {
        static bool s_warnLogged = false;
        if (!s_warnLogged) { s_warnLogged = true; probeLog("DRAW: shield bar texture '%s' unresolved, draw skipped", ShieldBarTexture); }
        return -1;
    }
    if (n > MaxSegments) {                                     // merge extras into the last segment
        for (int i = MaxSegments; i < n; ++i) { pools[MaxSegments - 1].remaining += pools[i].remaining; pools[MaxSegments - 1].max += pools[i].max; }
        n = MaxSegments;
    }
    (void)scale;
    // Narrower than the health bar (user: "too wide / too far right"): the art is drawn at ShieldWidthFraction of the health
    // rect's width (curve squashed a little) and placed so its visible right edge sits exactly on the health bar's visible left edge.
    // Small (companion) cards: the bar is 1 px shorter top and bottom, otherwise the pale outline reads taller than the red health bar.
    int inset = vw < 70 ? 1 : 0;
    int W = int(ShieldWidthFraction * vw + 0.5f), H = vh - 2 * inset;
    int seamX = vx + int(0.445f * vw + 0.5f);                  // visible left edge of the (squared) health bar
    // The art's visible right edge is at 98/128 = 0.766 of its width. Pick an integer visible width V, then the draw width that puts that
    // edge on the seam to within half a pixel: no full-pixel gap (the black hairline against the menu's black background) and no
    // 1 px overlap (which double-blends over the health outline's antialiased edge and over the corner wedge = a bright/dark line).
    int V = int(0.766f * W + 0.5f); W = int(V / 0.766f + 0.5f);
    int x = seamX - V, y = vy + inset;
    long total = 0; for (int i = 0; i < n; ++i) total += pools[i].max;
    if (total < 1) return 0;
    const float flushArg = gui2d::flushArg();
    auto draw = [](void* i, int dx, int dy, int dw, int dh, int, int, const float* rgb, float a) { gui2d::drawImage(i, dx, dy, dw, dh, 0, rgb, a); };   // image vtable ImageVt_Draw (a5 = rot = 0)
    gui2d::flush(flushArg);
    // Everything is clipped to [x, seamX): the art's body now extends past its visible edge, so the right edge is pixel-exact on the seam
    // (a hard clip edge, no texture-filter fringe = no hairline between shield, corner wedge and health bar).
    if (gui2d::pushClip(x, y, V, H)) { draw(image, 0, 0, W, H, 0, 0, TrackTint, 1.0f); gui2d::popClip(); }   // empty track: the engine's neutral teal
    int baseUp = 0;                                            // px of bar height already used, from the bottom
    for (int i = 0; i < n; ++i) {
        int segH = int(double(H) * pools[i].max / total + 0.5);
        int fillH = int(double(H) * pools[i].remaining / total + 0.5);
        if (fillH > segH) fillH = segH;
        if (fillH > 0) {
            int y0 = y + H - baseUp - fillH;
            float lit[3]; lightenFill(pools[i].rgb, lit);
            if (noClip) {                                          // menu frames: the engine clip stack reads 0xFFFF there and pushClip
                draw(image, x, y0, W, fillH, 0, 0, lit, 1.0f);     // misbehaves (black menu), so draw the art squashed to the fill height
            } else {
            int pushed = gui2d::pushClip(x, y0, V, fillH);
            { static int lastPush[8]; static int pushSlot[8]; static int nPush = 0;
              int pi = -1; for (int k = 0; k < nPush; ++k) if (pushSlot[k] == slotPtr) pi = k;
              if (pi < 0 && nPush < 8) { pi = nPush++; pushSlot[pi] = slotPtr; lastPush[pi] = -1; }
              int psig = pushed ? 1 : 0;
              if (pi >= 0 && lastPush[pi] != psig) { lastPush[pi] = psig; probeLog("FILL slot=%08x seg=%d pushClip=%d rect=(%d,%d,%d,%d) vit=(%d,%d,%d,%d) n=%d", slotPtr, i, pushed, x, y0, W, fillH, vx, vy, vw, vh, n); } }
            if (pushed) {
                draw(image, 0, -(y0 - y), W, H, 0, 0, lit, 1.0f);
                gui2d::popClip();
            }
            }
        }
        baseUp += segH;
    }
    // Seam between shield and health (disabled: DrawSeam is false, line 151): when enabled, a thin DARK line (the card border's own deep teal),
    // not a bright bar, so the two bars read as one gauge with a groove between them. 2 px on the leader card, 1 px on small cards, full bar height.
    int dw = vw / 50 < 1 ? 1 : vw / 50;
    // Corner fillers: the card frame's rounded corner leaves a dark notch above/below the health bar's outline, right of the shield's
    // right edge. uibit_bar_sc_p (scripts/gen_shield_bar_art.py) is drawn on the HEALTH rect and holds only the wedge between x=0.445*vw and
    // the health art's curved outline, so it extends the shield's top/bottom edge into the corner without touching a health pixel.
    if (void* corner = imageFor(CornerTexture)) {
        auto cdraw = draw;
        float top[3], bot[3];
        bool topFull = pools[n - 1].remaining >= pools[n - 1].max, botFull = pools[0].remaining > 0;
        lightenFill(pools[n - 1].rgb, top); lightenFill(pools[0].rgb, bot);
        const float* tc = topFull ? top : TrackTint; const float* bc = botFull ? bot : TrackTint;
        if (noClip) cdraw(corner, vx, vy, vw, vh, 0, 0, bc, 1.0f);
        else {
            int half = vh / 2;
            if (gui2d::pushClip(vx, vy, vw, half)) { cdraw(corner, 0, 0, vw, vh, 0, 0, tc, 1.0f); gui2d::popClip(); }
            if (gui2d::pushClip(vx, vy + half, vw, vh - half)) { cdraw(corner, 0, -half, vw, vh, 0, 0, bc, 1.0f); gui2d::popClip(); }
        }
    }
    if (DrawSeam && !noClip && gui2d::pushClip(seamX - dw, y, dw, H, SeamTint, 0.85f)) gui2d::popClip();
    gui2d::flush(flushArg);
    return n;
}


// ---- blackout diagnostics (flag files, polled every 30 frames; work mid-session, no restart):
//   shield_nodraw.txt  -> skip the shield bar draws (the Buff HUD's buffhud_off.txt lives in buff-duration-hud)
// F9 = write a MARK line with a full GL state dump + scene brightness sample. While shields turn on/off the GL state is dumped
// before/after our draws for 4 frames (PRE/POST) and the scene brightness is sampled every 30 frames (SCENE dark=0/1). ----
bool g_noDraw = false;
bool fileExists(const char* n) { return GetFileAttributesA(n) != INVALID_FILE_ATTRIBUTES; }
// GL function table + CapGuard live in Gui2D.h; GLINIT / CAPFIX lines come back through gui2d::g_hooks (set in DllMain).
void onGlInit(const gui2d::GlFns& g) {
    probeLog("GLINIT ok=%d readPixels=%d enable/disable=%d", g.ok, g.readPixels != nullptr, g.enable && g.disable);
}
void onCapFix(unsigned cap, int was, int now) { probeLog("CAPFIX cap=%x was=%d now=%d -> restored", cap, was, now); }
void glDump(const char* tag) {
    if (!g_probe) return;
    const gui2d::GlFns& g_gl = gui2d::gl();
    if (!g_gl.ok) return;
    int vp[4] = {}, sc[4] = {}, prog = 0, fbo = 0, tex = 0, mm = 0, mvd = 0, prd = 0, bsrc = 0, bdst = 0, drawbuf = 0;
    unsigned char dm = 0, cm[4] = {};
    g_gl.getIntegerv(0x0BA2, vp); g_gl.getIntegerv(0x0C10, sc);
    g_gl.getIntegerv(0x8B8D, &prog); g_gl.getIntegerv(0x8CA6, &fbo); g_gl.getIntegerv(0x8069, &tex);
    g_gl.getIntegerv(0x0BA0, &mm); g_gl.getIntegerv(0x0BA3, &mvd); g_gl.getIntegerv(0x0BA4, &prd);
    g_gl.getIntegerv(0x0BE1, &bsrc); g_gl.getIntegerv(0x0BE0, &bdst); g_gl.getIntegerv(0x0C01, &drawbuf);
    g_gl.getBooleanv(0x0B72, &dm); g_gl.getBooleanv(0x0C23, cm);
    int clipDepth = gui2d::clipDepthRaw();
    probeLog("GL %s vp=(%d,%d,%d,%d) scissor=(%d,%d,%d,%d) en[scis=%d sten=%d depth=%d light=%d blend=%d fog=%d tex2d=%d cull=%d alpha=%d vprog=%d fprog=%d] depthmask=%d colormask=%d%d%d%d prog=%d fbo=%d tex=%d drawbuf=%x blend=%x/%x mm=%x mvdepth=%d prdepth=%d clipDepth=%d",
             tag, vp[0], vp[1], vp[2], vp[3], sc[0], sc[1], sc[2], sc[3],
             g_gl.isEnabled(0x0C11), g_gl.isEnabled(0x0B90), g_gl.isEnabled(0x0B71), g_gl.isEnabled(0x0B50), g_gl.isEnabled(0x0BE2),
             g_gl.isEnabled(0x0B60), g_gl.isEnabled(0x0DE1), g_gl.isEnabled(0x0B44), g_gl.isEnabled(0x0BC0), g_gl.isEnabled(0x8620), g_gl.isEnabled(0x8804),
             dm, cm[0], cm[1], cm[2], cm[3], prog, fbo, tex, drawbuf, bsrc, bdst, mm, mvd, prd, clipDepth);
    for (int i = 0; i < 4; ++i) { int e = g_gl.getError(); if (!e) break; probeLog("GL %s glGetError=0x%x", tag, e); }
}
// average brightness (0..255) of a 5x5 grid over the 3D scene area; -1 when unavailable
int sceneBrightness() {
    // GPU readback probe disabled to prevent PCIe pipeline stalls
    return -1;
}


// ---- Menu companion portraits: health / force / shield bars + room for them -----------------------------------------------------
// The menu's LBL_CHAR2/3 portraits are plain images. We draw the HUD small card's bars around them (same art + tints as PB_VIT/PB_FORCE:
// uibit_bar_vp_p red over teal, uibit_bar_fp_p cyan over teal; geometry copied from the HUD small card: vit = card.x-40 .. +47 wide, force =
// card.x+77 .. +47 wide, both 96 tall from card.y-5, on an 85 px card). Values the way UpdatePortraits gets them (0x74ee90): HP = creature
// vtable+0x9c(0, max) with max = vtable+0x98(1, creature[0x43d]); FP = stats(+0x1198) shorts +0x130 + +0x132, max = FUN_0057eca0(creature).
// Menu control groups (LBL_CHARn, LEVELUPn, BACKn, BTN_CHANGEn for n = 2, 3) are K2AddrTable Menu_Char2 .. Menu_Change3 (byte offsets from the panel).
const float HealthFillRed[3] = { 1.0f, 0.0f, 0.0f }, ForceFillCyan[3] = { 0.0f, 0.8f, 0.898f };
const char* const HealthBarTexture = "uibit_bar_vp_p";
const char* const ForceBarTexture = "uibit_bar_fp_p";

bool readVitals(int creature, int* hp, int* hpMax, int* fp, int* fpMax) {
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(sb::vitalsFieldsOk(T), "companion vitals")) return false;
    int vt = 0, stats = 0;
    if (!readAt(creature, 0, &vt) || !vt || !readAt(creature, T->Creature_Stats, &stats) || !stats) return false;   // creature+0 = vtable (C++ ABI)
    int fnMax = 0, fnCur = 0; short sForceA = 0, sForceB = 0;
    if (!readAt(vt, T->Creature_VtMaxHp, &fnMax) || !readAt(vt, T->Creature_VtCurHp, &fnCur) || !fnMax || !fnCur || !readAt(stats, T->Stats_ForceA, &sForceA) || !readAt(stats, T->Stats_ForceB, &sForceB)) return false;
    // CSWSCreature vtable slots 38/39 (0x57e8c0 / 0x5413c0) each take ONE stack arg (RET 4): a 2-arg call unbalances the stack (crash 10-02)
    *hpMax = reinterpret_cast<short(__thiscall*)(int, int)>(fnMax)(creature, 1);
    *hp = reinterpret_cast<short(__thiscall*)(int, int)>(fnCur)(creature, 0);
    *fp = int(sForceA) + int(sForceB);
    *fpMax = int(reinterpret_cast<unsigned(__fastcall*)(int)>(T->GetMaxForcePoints)(creature));
    return true;
}

// one vertical gauge: teal track over the whole rect, then the colour fill from the bottom (cur/max of the height), as the stock bars
void drawGauge(const char* texture, int x, int y, int w, int h, const float* fill, int cur, int mx) {
    if (!featureOk(sb::drawFieldsOk(k2addr::get()), "gauge draw")) return;
    void* img = imageFor(texture);
    if (!img || w < 4 || h < 4) return;
    if (cur < 0) cur = 0; if (cur > mx) cur = mx;
    auto draw = [](void* i, int dx, int dy, int dw, int dh, const float* rgb, float a) { gui2d::drawImage(i, dx, dy, dw, dh, 0, rgb, a); };   // image vtable ImageVt_Draw
    if (mx < 1) { draw(img, x, y, w, h, TrackTint, 1.0f); return; }     // no Force (droid): empty teal track, as the active portrait shows
    draw(img, x, y, w, h, TrackTint, 1.0f);
    gui2d::drawVFill(img, {x, y, w, h}, cur, mx, fill, 1.0f);
}

// Spread the two companion portrait groups apart (runtime write of the controls' x, so no .gui edit and the hit areas move too).
// Group 2 moves 0.30w; group 3 is placed so the gap between the companions equals the gap between the leader's force bar and
// companion 1 (sb::secondCompanionX; leader force rect lfx/lfw, 0 = unknown -> fallback 1.00w).
// Group 2 and 3 also move down by dy so their card bottoms meet the leader card bottom (sb::leaderCardBottom from the leader vit
// rect vy/vh; sb::companionDy caps the nudge at 8 px).
// Shifts are remembered per panel; a layout we did not write (menu rebuilt) is adopted as the new original (x and y separately).
struct ShiftRec { int panel; int orig[2][4], shifted[2][4], origY[2][4], shiftedY[2][4]; };
ShiftRec g_shift[8];
void spreadCompanions(int panel, int w, int lfx, int lfw, int vy, int vh) {
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(sb::menuFieldsOk(T), "companion spacing")) return;
    const int grp[2][4] = { { T->Menu_Char2, T->Menu_Level2, T->Menu_Back2, T->Menu_Change2 },    // LBL_CHARn, LEVELUPn, BACKn, BTN_CHANGEn
                            { T->Menu_Char3, T->Menu_Level3, T->Menu_Back3, T->Menu_Change3 } };
    ShiftRec* rec = nullptr;
    for (auto& r : g_shift) if (r.panel == panel) { rec = &r; break; }
    if (!rec) { static int next = 0; rec = &g_shift[next++ & 7]; *rec = {}; rec->panel = panel; }
    int rc[2][4][4] = {};
    for (int g = 0; g < 2; ++g) for (int i = 0; i < 4; ++i) {
        readCtlRect(panel + grp[g][i], rc[g][i]);
        if (rc[g][i][2] < 1 || rc[g][i][3] < 1) continue;
        int cx = rc[g][i][0], cy = rc[g][i][1];
        if (cx != rec->orig[g][i] && cx != rec->shifted[g][i]) rec->orig[g][i] = cx;
        if (cy != rec->origY[g][i] && cy != rec->shiftedY[g][i]) rec->origY[g][i] = cy;
    }
    int dx[2] = { int(0.30f * w + 0.5f), int(1.00f * w + 0.5f) };
    const int* c2 = rc[0][0];
    if (lfw >= 8 && lfw <= 2000 && c2[3] >= 8 && c2[3] <= 2000) {
        int c3x = sb::secondCompanionX(lfx, lfw, rec->orig[0][0] + dx[0], c2[1], w, c2[3]);
        int d = c3x - rec->orig[1][0];
        if (d > 0 && d < 3 * w) dx[1] = d;
    }
    int dy = 0;
    if (vh >= 8 && vh <= 2000 && c2[3] >= 8 && c2[3] <= 2000) dy = sb::companionDy(sb::leaderCardBottom(vy, vh), rec->origY[0][0], c2[3]);
    for (int g = 0; g < 2; ++g) for (int i = 0; i < 4; ++i) {
        int ctl = panel + grp[g][i], cur = rc[g][i][0], curY = rc[g][i][1];
        if (rc[g][i][2] < 1 || rc[g][i][3] < 1) continue;
        rec->shifted[g][i] = rec->orig[g][i] + dx[g];
        rec->shiftedY[g][i] = rec->origY[g][i] + dy;
        if (cur == rec->shifted[g][i] && curY == rec->shiftedY[g][i]) continue;
        // engine SetRect (vtable slot 1, RET 4: moves the rect + border/text copies, so the drawn portrait follows; raw int writes only moved the hit area)
        int vt = 0, fn = 0;
        if (!readAt(ctl, 0, &vt) || !vt || !readAt(vt, T->Ctl_VtSetRect, &fn) || !fn) continue;
        int nr[4] = { rec->shifted[g][i], rec->shiftedY[g][i], rc[g][i][2], rc[g][i][3] };
        reinterpret_cast<void(__thiscall*)(int, int*)>(fn)(ctl, nr);
        probeLog("MENUMOVE g=%d i=%d x %d -> %d (dx %d, leader force x=%d w=%d) y %d -> %d (dy %d)", g, i, cur, nr[0], dx[g], lfx, lfw, curY, nr[1], dy);
    }
}


// MENUSTATE transitions (the leader call either draws the HUD or the menu overlay)
void noteMenuState(bool menu) {
    static bool lastMenu = false;
    if (menu != lastMenu) { lastMenu = menu; probeLog("MENUSTATE skip=%d clipDepth=%x", menu, gui2d::clipDepthRaw()); }
}

void KPATCH_CALL onSnapshot(const K2PsContext* c, void*) {
    if (g_probe && c->creature) probeEffects(c->slot, c->creature);
}

// Leader call, once per frame: kill switch every 30 frames, scene-brightness probe, F9 mark.
void KPATCH_CALL onFrameBegin(const K2PsContext* c, void*) {
    g_frame = int(c->frame);
    flushConnectNote();
    if (g_frame % 30 == 0) {
        g_noDraw = fileExists("shield_nodraw.txt");
        static int lastFlags = -1; int fl = g_noDraw;
        if (fl != lastFlags) { lastFlags = fl; probeLog("FLAGS nodraw=%d", g_noDraw); }
        if (g_probe) {
            int b = sceneBrightness();
            static int lastDark = -1; int dark = (b >= 0 && b < 6) ? 1 : 0;
            if (b >= 0 && dark != lastDark) { lastDark = dark; probeLog("SCENE avg=%d dark=%d", b, dark); glDump(dark ? "DARK" : "BRIGHT"); }
            if (g_frame % 600 == 0) probeLog("HB scene=%d", b);
        }
    }
    static bool f9Prev = false; bool f9 = g_probe && (GetAsyncKeyState(VK_F9) & 0x8000) != 0;
    if (f9 && !f9Prev) { static int mark = 0; probeLog("MARK %d scene=%d", ++mark, g_probe ? sceneBrightness() : -2); glDump("MARK"); { char rb[3000]; saferead::dump(rb, sizeof(rb)); probeLog("%s", rb); } }
    f9Prev = f9;
}

// Every slot: leader force-bar slide (FORCEMOVE) + SLOT geometry log.
void KPATCH_CALL onSlot(const K2PsContext* c, void*) {
    g_frame = int(c->frame);
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(sb::slotFieldsOk(T), "slot geometry / force-bar slide")) return;
    const int slotPtr = c->slot, isLarge = c->isLarge, creature = c->creature;
    int flags = 0, vitVt = 0, forceVt = 0, vit[4] = {}, force[4] = {}, card[4] = {};
    readAt(slotPtr, T->Slot_Flags, &flags); readAt(slotPtr, T->Slot_VitCtl, &vitVt); readAt(slotPtr, T->Slot_ForceCtl, &forceVt);
    for (int k = 0; k < 4; ++k) {
        readAt(slotPtr, T->Slot_VitRect + 4 * k, &vit[k]); readAt(slotPtr, T->Slot_ForceRect + 4 * k, &force[k]); readAt(slotPtr, T->Slot_CardRect + 4 * k, &card[k]);
    }
    if (isLarge && !g_noDraw && force[2] >= 8 && force[2] <= 2000 && card[2] >= 8) {
        // leader force bar: its art leaves a few px between the portrait and the visible bar (health is flush): slide it left by that gap
        // (art columns 29..69 of 128 => visible left edge = x + 0.227*w). Engine SetRect = progress-bar vtable slot 1 (0x41b9e0, RET 4), once.
        int gap = int(force[0] + 29.0f / 128.0f * force[2] + 0.5f) - (card[0] + card[2]);
        if (gap > 0 && gap < 12) {
            int vt = 0, fn = 0, fo = slotPtr + T->Slot_ForceCtl;
            if (readAt(fo, 0, &vt) && vt && readAt(vt, T->Ctl_VtSetRect, &fn) && fn) {
                int nr[4] = { force[0] - gap, force[1], force[2], force[3] };
                reinterpret_cast<void(__thiscall*)(int, int*)>(fn)(fo, nr);
                probeLog("FORCEMOVE slot=%08x x %d -> %d (gap %d)", slotPtr, force[0], nr[0], gap);
                force[0] = nr[0];
            }
        }
    }
    float scale = gui2d::hudScale();
    int sig = slotPtr ^ creature ^ flags ^ isLarge ^ (vit[0] * 3) ^ (vit[1] * 5) ^ (vit[2] * 7) ^ (vit[3] * 11) ^ (force[0] * 13) ^ (card[0] * 17) ^ (card[2] * 19) ^ int(scale * 100);
    static int lastSig[8]; static int sigSlot[8]; static int nSig = 0;
    int si = -1; for (int k = 0; k < nSig; ++k) if (sigSlot[k] == slotPtr) si = k;
    if (si < 0 && nSig < 8) { si = nSig++; sigSlot[si] = slotPtr; lastSig[si] = 0; }
    if (si >= 0 && lastSig[si] != sig) {
        lastSig[si] = sig;
        probeLog("SLOT %08x creature=%08x large=%d flags590=%08x vt690=%08x vt7fc=%08x scale=%.3f vit=(%d,%d,%d,%d) force=(%d,%d,%d,%d) card=(%d,%d,%d,%d)",
                 slotPtr, creature, isLarge, flags, vitVt, forceVt, scale, vit[0], vit[1], vit[2], vit[3], force[0], force[1], force[2], force[3],
                 card[0], card[1], card[2], card[3]);
    }
}

// Menu frame (the provider has reopened the 2D batch and pushed the panel clip): the top-bar portrait's shield bar, the companion
// portraits' health / force / shield bars, and the companion portrait spacing.
void KPATCH_CALL onMenuOverlay(const K2PsContext* c, void*) {
    g_frame = int(c->frame);
    noteMenuState(true);
    if (g_noDraw) return;
    const K2AddrTable* T = k2addr::get();
    const bool haveVit = featureOk(sb::vitRectOk(T), "menu shield bar");          // top-bar portrait's health-bar rect
    const bool haveMenu = featureOk(sb::menuFieldsOk(T), "menu companion bars");   // companion controls: gauges, bars, spacing
    if (!haveVit && !haveMenu) return;
    const int slotPtr = c->slot, panel = c->panel, creature = c->menuCreature;
    const float scale = gui2d::hudScale();
    int v[4] = {};
    if (haveVit) for (int k = 0; k < 4; ++k) readAt(slotPtr, T->Slot_VitRect + 4 * k, &v[k]);
    int pools = 0;
    if (creature && v[2] >= 1 && v[2] <= 2000 && v[3] >= 1 && v[3] <= 2000) { Pool t[8]; pools = collectPools(creature, t, 8); }
    const int comp[2] = { c->companions[0], c->companions[1] };
    static int lastSig = -1; int sig = slotPtr ^ creature ^ (pools << 28) ^ v[0] ^ (v[1] << 4) ^ (v[2] << 8) ^ (v[3] << 12) ^ comp[0] ^ (comp[1] << 3);
    if (sig != lastSig) { lastSig = sig; probeLog("MENUSLOT slot=%08x callerEbp=%08x creature=%08x pools=%d vit=(%d,%d,%d,%d) companions=%08x,%08x", slotPtr, c->callerEbp, creature, pools, v[0], v[1], v[2], v[3], comp[0], comp[1]); }
    CapGuard capGuard;
    int shown = 0, bars = 0;
    if (pools > 0) shown = DrawShieldPools(creature, v[0], v[1], v[2], v[3], scale, slotPtr, false);
    if (g_probe && haveMenu) { int q[2][4] = {}; for (int k = 0; k < 2; ++k) readCtlRect(c->companionCtrl[k], q[k]);
        static int lastQ = -1; int sq = q[0][0] * 3 ^ q[1][0] * 7 ^ q[0][1] ^ q[1][2];
        static int pc = 0; if (sq != lastQ || (++pc % 120) == 0) { lastQ = sq; probeLog("MENUPRE c2=(%d,%d,%d,%d) c3=(%d,%d,%d,%d) panel=%08x", q[0][0], q[0][1], q[0][2], q[0][3], q[1][0], q[1][1], q[1][2], q[1][3], panel); } }
    if (haveMenu) {
        int w2 = 0, lf[4] = {};   // leader force rect (after the SLOT-phase FORCEMOVE slide): companion 3 is spaced from it
        if (T->Slot_ForceRect) for (int k = 0; k < 4; ++k) readAt(slotPtr, T->Slot_ForceRect + 4 * k, &lf[k]);
        if (readAt(c->companionCtrl[0], T->Ctl_W, &w2) && w2 >= 8 && w2 <= 2000) spreadCompanions(panel, w2, lf[0], lf[2], v[1], v[3]);
    }
    for (int k = 0; haveMenu && k < 2; ++k) {
        int ctrl = c->companionCtrl[k], fl = 0, r[4] = {};
        if (!comp[k] || !readAt(ctrl, T->Ctl_Flags, &fl) || !(fl & 2)) continue;
        readCtlRect(ctrl, r);
        if (r[2] < 1 || r[3] < 1 || r[2] > 2000 || r[3] > 2000) continue;
        int hp = 0, hpMax = 0, fp = 0, fpMax = 0;
        if (!readVitals(comp[k], &hp, &hpMax, &fp, &fpMax)) continue;
        // bars around the portrait: visible health edge on the card's left edge, force edge on its right edge, art rows flush with the
        // card's top / bottom (sb::menuBars, art columns measured from the override TGAs); shield beside the health bar
        const sb::MenuBars mb = sb::menuBars(r[0], r[1], r[2], r[3]);
        const int vx = mb.vx, fx = mb.fx, by = mb.by, bw = mb.bw, bwF = mb.bwF, bh = mb.bh;
        drawGauge(HealthBarTexture, vx, by, bw, bh, HealthFillRed, hp, hpMax);
        drawGauge(ForceBarTexture, fx, by, bwF, bh, ForceFillCyan, fp, fpMax);
        DrawShieldPools(comp[k], vx, by, bw, bh, scale, ctrl, false);
        ++bars;
        static int lastV[2] = { -1, -1 }; int vs = hp * 7 + hpMax * 13 + fp * 17 + fpMax * 19;
        if (lastV[k] != vs) { lastV[k] = vs; probeLog("MENUCOMP k=%d creature=%08x hp=%d/%d fp=%d/%d card=(%d,%d,%d,%d) vit=(%d,%d,%d,%d) force=(%d,%d,%d,%d)", k, comp[k], hp, hpMax, fp, fpMax, r[0], r[1], r[2], r[3], vx, by, bw, bh, fx, by, bwF, bh); }
    }
    static int lastShown = -2, lastBars = -2;
    if (shown != lastShown || bars != lastBars) { lastShown = shown; lastBars = bars; probeLog("MENUDRAW panel=(%d,%d,%d,%d) bar=%d companionBars=%d", c->panelRect[0], c->panelRect[1], c->panelRect[2], c->panelRect[3], shown, bars); }
}

// HUD frame, leader call: every card's shield bar. All bars are drawn from the LEADER's call: per-slot draws failed on the later slots
// whenever an earlier slot had already drawn its Buff HUD icon row (leaked GUI clip/batch state, session 4c logs).
void KPATCH_CALL onHudDraw(const K2PsContext* c, void*) {
    g_frame = int(c->frame);
    noteMenuState(false);
    if (g_noDraw) return;
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(sb::vitRectOk(T), "HUD shield bars")) return;
    const int effCountOff = T->Creature_EffectCount, arrowBuffOff = T->Slot_ArrowBuff, arrowDebuffOff = T->Slot_ArrowDebuff;   // log-only reads: 0 = skip
    const float scale = gui2d::hudScale();
    CapGuard capGuard;
    static bool prevAny = false; static int dumpFrames = 0;
    bool any = false;
    for (int i = 0; i < K2PS_MAX_SLOTS; ++i) { int32_t s = 0, cr = 0; Pool t[8]; if (g_ps.slotAt(i, &s, &cr) && collectPools(cr, t, 8) > 0) any = true; }
    if (any != prevAny) { prevAny = any; dumpFrames = 4; probeLog("SHIELD_ANY %d", any); }
    if (dumpFrames > 0) glDump("PRE");
    struct PostDump { int* d; ~PostDump() { if (*d > 0) { glDump("POST"); --*d; } } } postDump{ &dumpFrames };
    for (int i = 0; i < K2PS_MAX_SLOTS; ++i) {
        int32_t slot = 0, cr = 0;
        if (!g_ps.slotAt(i, &slot, &cr)) continue;
        int v[4] = {};
        for (int k = 0; k < 4; ++k) readAt(slot, T->Slot_VitRect + 4 * k, &v[k]);
        if (v[2] < 1 || v[2] > 400 || v[3] < 1 || v[3] > 400) continue;
        int n = DrawShieldPools(cr, v[0], v[1], v[2], v[3], scale, slot);
        int effCount = 0; if (effCountOff) readAt(cr, effCountOff, &effCount);
        unsigned char arrowBuff = 0, arrowDebuff = 0;
        if (arrowBuffOff) readAt(slot, arrowBuffOff, &arrowBuff);
        if (arrowDebuffOff) readAt(slot, arrowDebuffOff, &arrowDebuff);
        static int lastDraw[8]; static int drawSlot[8]; static int nDraw = 0;
        int di = -1; for (int k = 0; k < nDraw; ++k) if (drawSlot[k] == slot) di = k;
        if (di < 0 && nDraw < 8) { di = nDraw++; drawSlot[di] = slot; lastDraw[di] = -12345; }
        int dsig = cr ^ (n << 24) ^ (effCount << 16) ^ (arrowBuff << 8) ^ (arrowDebuff << 4);
        if (di >= 0 && lastDraw[di] != dsig) {
            lastDraw[di] = dsig;
            probeLog("DRAW slot=%08x creature=%08x pools=%d effects=%d engineArrows buff=%d debuff=%d (drawn from leader call)", slot, cr, n, effCount, arrowBuff, arrowDebuff);
        }
    }
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    g_ps.init(api, "shield-bar");
    g_ps.onConnect = onConnect;
    g_ps.add(K2PS_SNAPSHOT, K2PS_PRIO_SHIELD, onSnapshot);
    g_ps.add(K2PS_FRAME_BEGIN, K2PS_PRIO_SHIELD, onFrameBegin);
    g_ps.add(K2PS_SLOT, K2PS_PRIO_SHIELD, onSlot);
    g_ps.add(K2PS_MENU_OVERLAY, K2PS_PRIO_SHIELD, onMenuOverlay);
    g_ps.add(K2PS_HUD_DRAW, K2PS_PRIO_SHIELD, onHudDraw);
    g_ps.connect();
}

// Portrait hover tooltip (FUN_00744c20 builds the text into the CExoString at charButton+0x1d4, once per hover / refresh flag).
// Hooked at its RET path 0x00744EEC; [ebp-0x48] = charButton, *(charButton+0x1d0) = the portrait slot pointer. The engine's
// cdecl "sprintf into CExoString" (FUN_00734270) formats into a scratch buffer before touching the destination, so the old
// text can be passed back in as an argument. The tooltip panel sizes itself from the text (FUN_0075ff50).
void __cdecl AppendShieldToTooltip(int charButton) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
    g_ps.poll();
    flushConnectNote();
    const K2AddrTable* T = k2addr::get();
    if (!featureOk(sb::tooltipFieldsOk(T), "tooltip shield line")) return;
    int slot = 0, oldText = 0;
    if (!readAt(charButton, T->Tip_Slot, &slot) || !slot || !readAt(charButton, T->Tip_Text, &oldText) || !oldText) return;
    // Defensive: the builder re-formats the text before this RET path, so the line is normally absent here; never append twice.
    if (strstr(reinterpret_cast<const char*>(oldText), "Shield:")) return;
    int creature = g_ps.creatureForSlot(slot);
    char line[96];
    int n = creature ? describePools(creature, line, sizeof(line)) : 0;
    probeLog("TIP btn=%08x slot=%08x creature=%08x pools=%d line='%s'", charButton, slot, creature, n, line);
    if (n <= 0) return;
    using CExoSprintf = void(__cdecl*)(void*, const char*, ...);
    reinterpret_cast<CExoSprintf>(T->CExoStringFormat)(reinterpret_cast<void*>(charButton + T->Tip_Text), "%s\n%s", reinterpret_cast<const char*>(oldText), line);
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_probe = GetFileAttributesA("shieldprobe.txt") != INVALID_FILE_ATTRIBUTES;
        saferead::enableRing(g_probe);
        gui2d::g_hooks.onGlInit = onGlInit;
        gui2d::g_hooks.onCapFix = onCapFix;
    }
    return TRUE;
}
