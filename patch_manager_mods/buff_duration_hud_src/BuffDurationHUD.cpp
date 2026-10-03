// Buff Duration HUD (Steam Aspyr build 6A522E71...). Replaces the engine's plain
// buff/debuff arrow glyphs over each party portrait with the effect's own icons: buffs in a
// column down the card's left edge, debuffs down the right edge, item abilities on a round
// teal disc, each draining top-down (dim = elapsed, full = remaining) as the effect runs out. Research trail and every address below:
// patch_manager_mods/02_buff_duration_hud.md (local-setup/KOTOR-II worktree).
//
// Hooks (all inside CSWGuiMainInterface, cdecl detours, original bytes still run):
//   1a/1b  0x0074f25a / 0x0074f8ab  UpdatePortraits: remember slotPtr -> creature
//   2      0x00745a84  PortraitIconRowDraw, before each native arrow Draw(): hide the arrow
//   3      0x00745a97  right after that Draw() (buff row only): un-hide it
//   5      0x00745aec  row exit (once per call): draw our icon column, dimmed + bright slice
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "EffectIconTable.inc"
#include "DebuffIconTable.inc"
#include "ShieldTable.inc"
#include "../_shared/SafeRead.h"   // VirtualQuery-based probing: IsBadReadPtr = a real Wine page fault + SEH dispatch per bad pointer

namespace {

// CGameEffect (non-virtual). +0x1c = spells.2da row of the spell/item ability that made it.
constexpr int Effect_DurationOffset = 0xC;    // float, total seconds
constexpr int Effect_ExpiryDayOffset = 0x10;  // int
constexpr int Effect_ExpiryTimeOffset = 0x14; // int, ms of day
constexpr int Effect_SpellIdOffset = 0x1c;
// creature (CSWSObject) active-effect list
constexpr int Creature_EffectArrayOffset = 0x148; // CGameEffect**
constexpr int Creature_EffectCountOffset = 0x14c; // int
// The icon widget is a CSWGuiLabel with two embedded children: +0x60 CSWGuiBorder ("BORDER",
// draws the arrow glyph as its fill image) and +0xd8 CSWGuiText (font text, unused here).
// Verified by live memory dump: border vtable 009875bc, fill image at border+0x74 (what
// CSWGuiBorder::Draw 0x415300 reads), size at +0xc/+0x10.
constexpr int Label_TextBorderOffset = 0x60;
constexpr int Border_WidthOffset = 0xc;
constexpr int Border_HeightOffset = 0x10;
constexpr int Border_FillImageOffset = 0x74;      // bound FILL image object (direct field)

constexpr int WorldTimerSingletonGlobal = 0x00a1b4a4;
constexpr int WorldTimerGetCurrentTime = 0x0051ad20; // thiscall(timer, int* day, uint* ms), RET 8
constexpr int LoadFillResource = 0x0047eb60;         // cdecl(resref buf) -> CAurGUIImageInternal*
constexpr int MsPerDay = 86400000;
constexpr int MaxEffects = 512;
constexpr int MaxPoolImages = 128;
constexpr float MinWipeRatio = 0.0f;
constexpr int CExoStringSprintf = 0x00734270;  // cdecl (CExoString* dst, const char* fmt, ...)
constexpr int HudScaleFn = 0x00479ab0;        // cdecl, returns float in st0
constexpr int FlushGuiBatch = 0x0047ea60;     // cdecl(float), flushes batched GUI quads
constexpr int PushClipRect = 0x00479460;      // cdecl(x,y,w,h,color*,flag,float alpha) -> nonzero if pushed
constexpr int PopClipRect = 0x004799c0;
const void* const ClipNoFillColor = reinterpret_cast<const void*>(0x00a1ba3c); // what the row itself passes
constexpr float SmallCardFraction = 0.19f, LargeCardFraction = 0.22f; // preferred icon size / card width
constexpr float MinIconSize = 5.0f, MaxIconSize = 24.0f;  // x HUD scale (=10 / 48 px at scale 2)
constexpr int MaxColumns = 3;
constexpr int RightInset = 0;                 // debuff column: extra px in from the card's right edge
const float LineColor[3] = { 0.0f, 0.16f, 0.14f };  // thin dark-teal edge between remaining and elapsed
constexpr float LineAlpha = 0.95f;
constexpr float DimAlpha = 0.35f;
constexpr float BgAlpha = 0.80f;              // backing disc, full-strength part
const float BgColor[3] = { 0.035f, 0.415f, 0.34f };  // the HUD border teal (sampled 9,106,87)
const char* const CircleResref = "buffhud_circle";  // white disc TGA shipped in override/

// spells.2da rows that are ITEM abilities (stims, shots, shields, cloak); force powers get no backing
bool isItemAbility(int spell) {
    return (spell >= 69 && spell <= 77) || (spell >= 99 && spell <= 108) || (spell >= 110 && spell <= 115) || spell == 129 || spell == 132 || spell == 257;
}

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

// ---- hook 1: slot -> creature side table -------------------------------------------
struct SlotEntry { int slotPtr; int creaturePtr; DWORD tick; };   // tick = when SnapshotPortraitSlot last refreshed it
SlotEntry g_slots[4];

SlotEntry g_menuSlot;   // the menu top bar's portrait slot (menu+0x2350) -> selected creature, refreshed every menu frame

// UpdatePortraits refreshes every live HUD slot each frame, so an entry older than this is stale (area load / HUD rebuilt): its
// creature pointer may be freed memory.
constexpr DWORD SlotFreshMs = 500;
inline bool slotFresh(const SlotEntry& e) { return GetTickCount() - e.tick <= SlotFreshMs; }

int creatureForSlot(int slotPtr) {
    if (g_menuSlot.slotPtr == slotPtr) return g_menuSlot.creaturePtr;
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) return slotFresh(e) ? e.creaturePtr : 0;
    return 0;
}

// ---- helpers -------------------------------------------------------------------------
template <typename T> bool readAt(int base, int off, T* out) {
    return saferead::readAt(1, static_cast<uintptr_t>(base), off, out);
}

bool currentTime(int* day, int* ms) {
    int g, a, b;
    if (!readAt(WorldTimerSingletonGlobal, 0, &g) || !readAt(g, 8, &a) || !readAt(a, 4, &b)) return false;
    int timer;                                  // FUN_0051c370: *(*(a+4)+0x10048)
    if (!readAt(b, 0x10048, &timer) || timer == 0) return false;
    using GetTime = void(__thiscall*)(void*, int*, int*);
    reinterpret_cast<GetTime>(WorldTimerGetCurrentTime)(reinterpret_cast<void*>(timer), day, ms);
    return true;
}

// One engine image object per icon resref, created lazily and never released: the
// widget's own image is put back after every Draw, so the engine never sees ours.
struct PoolEntry { char resref[17]; void* image; };
PoolEntry g_pool[MaxPoolImages];
int g_poolCount = 0;

void* imageFor(const char* resref) {
    for (int i = 0; i < g_poolCount; ++i)
        if (strcmp(g_pool[i].resref, resref) == 0) return g_pool[i].image;
    if (g_poolCount >= MaxPoolImages) return nullptr;
    char buf[20] = {};                          // same shape FUN_00414840 builds: 16 chars + NUL
    strncpy(buf, resref, 16);
    void* image = reinterpret_cast<void*(__cdecl*)(const char*)>(LoadFillResource)(buf);
    PoolEntry& e = g_pool[g_poolCount++];       // cache misses too (image may be null)
    memset(&e, 0, sizeof(e));
    strncpy(e.resref, resref, 16);
    e.image = image;
    return image;
}

struct Buff { const char* resref; float ratio; bool item; };

// distinct tracked spell ids on the creature, in application order
int collectBuffs(int creature, int day, int ms, bool haveTime, Buff* out, int maxOut, bool debuff = false) {
    int arr = 0, count = 0;
    if (!readAt(creature, Creature_EffectArrayOffset, &arr) || !readAt(creature, Creature_EffectCountOffset, &count)) return 0;
    if (arr == 0 || count <= 0 || count > MaxEffects) return 0;
    int seen[MaxEffects]; int nSeen = 0; int n = 0;
    for (int i = 0; i < count && n < maxOut; ++i) {
        int eff, spell;
        if (!readAt(arr, i * 4, &eff) || !readAt(eff, Effect_SpellIdOffset, &spell)) continue;
        const char* resref = nullptr; bool item = false;
        if (spell >= 0 && spell < kEffectIconTableSize) {
            if (debuff) { resref = kDebuffIcons[spell].resref; item = kDebuffIcons[spell].item; }
            else { resref = kEffectIcons[spell]; item = isItemAbility(spell); }
        }
        if (!resref) continue;
        bool dup = false;
        for (int k = 0; k < nSeen; ++k) if (seen[k] == spell) { dup = true; break; }
        if (dup) continue;
        seen[nSeen++] = spell;
        float ratio = 1.0f;
        float duration; int eDay, eMs;
        if (haveTime && readAt(eff, Effect_DurationOffset, &duration) && duration > 0.0f &&
            readAt(eff, Effect_ExpiryDayOffset, &eDay) && readAt(eff, Effect_ExpiryTimeOffset, &eMs)) {
            double remaining = double(eDay - day) * MsPerDay + double(eMs - ms);
            ratio = float(remaining / (double(duration) * 1000.0));
            if (ratio > 1.0f) ratio = 1.0f;
            if (ratio < MinWipeRatio) ratio = MinWipeRatio;
        }
        out[n++] = { resref, ratio, item };
    }
    return n;
}

// state saved by hook 2 for hook 3
struct Saved { bool active; int border; int origImage; int width; int height; } g_saved;


// ---- Shield Bar PROBE (1.1.0-probe): OFF unless "shieldprobe.txt" exists in the game dir.
// Writes shieldprobe_log.txt: effect APPLY/CHANGE/REMOVE per creature (S1/S2/S4) and the
// portrait slot rects + test rects drawn at the new every-frame draw site (R-D/S6/S7). ----
bool g_probe = false;
int g_probeLines = 0, g_frame = 0, g_firstSlot = 0;
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

struct EffRec { int n34; int eff; unsigned short type, sub; int spell, creator; float dur; int ints[8]; bool seen; };
constexpr int ProbeMaxEff = 128, ProbeMaxCreatures = 8;
struct CreatureRec { int creature; unsigned lastTick; bool primed; int n; EffRec e[ProbeMaxEff]; };
CreatureRec g_cr[ProbeMaxCreatures];

void probeDump(int eff, char* out, int cap) {
    int len = 0;
    for (int row = 0; row < 12; ++row) {
        len += snprintf(out + len, cap - len, "\n    +%02x:", row * 16);
        for (int k = 0; k < 16; ++k) {
            unsigned char b = 0;
            if (!readAt(eff, row * 16 + k, &b)) { len += snprintf(out + len, cap - len, " .."); continue; }
            len += snprintf(out + len, cap - len, " %02x", b);
        }
    }
}

void probeEffects(int slotPtr, int creature) {
    CreatureRec* cr = nullptr;
    for (auto& c : g_cr) if (c.creature == creature) { cr = &c; break; }
    if (!cr) for (auto& c : g_cr) if (c.creature == 0) { cr = &c; c.creature = creature; break; }
    if (!cr) return;
    unsigned now = GetTickCount();
    if (cr->lastTick == now) return;            // the mirror slot repeats slot 0's creature
    cr->lastTick = now;
    int arr = 0, count = 0;
    if (!readAt(creature, Creature_EffectArrayOffset, &arr) || !readAt(creature, Creature_EffectCountOffset, &count)) return;
    if (arr == 0 || count < 0 || count > MaxEffects) return;
    for (int i = 0; i < cr->n; ++i) cr->e[i].seen = false;
    for (int i = 0; i < count; ++i) {
        int eff;
        if (!readAt(arr, i * 4, &eff) || eff == 0) continue;
        EffRec cur = {}; cur.eff = eff;
        readAt(eff, 0x8, &cur.type); readAt(eff, 0xa, &cur.sub); readAt(eff, 0x1c, &cur.spell);
        readAt(eff, 0x18, &cur.creator); readAt(eff, 0xc, &cur.dur);
        readAt(eff, 0x30, &cur.n34);                // +0x30 = count, +0x34 = int* array (probe 2 log: dump shows 08 00 00 00 | ptr)
        { int ip = 0; if (readAt(eff, 0x34, &ip) && ip) for (int k = 0; k < 8 && k < (cur.n34 > 0 ? cur.n34 : 8); ++k) readAt(ip, 4 * k, &cur.ints[k]); }
        EffRec* old = nullptr;
        for (int k = 0; k < cr->n; ++k) if (cr->e[k].eff == eff) { old = &cr->e[k]; break; }
        if (!old) {
            if (cr->n >= ProbeMaxEff) continue;
            char dump[1400]; probeDump(eff, dump, sizeof(dump));
            { int ip = 0, len = (int)strlen(dump); if (readAt(eff, 0x34, &ip) && ip) { len += snprintf(dump + len, sizeof(dump) - len, "\n    arr@%08x:", ip); for (int k = 0; k < 16; ++k) { int v = 0; if (!readAt(ip, 4 * k, &v)) break; len += snprintf(dump + len, sizeof(dump) - len, " %d", v); } } }
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


// ---- Shield bar drawing (probe2): the vitality bar's own curved texture, tinted per pool ----
constexpr float BarShiftFraction = 0.328f;            // opaque x-span of uibit_bar_vp_p (0.445..0.773) = 0.328 of the rect
const float TrackTint[3] = { 0.051f, 0.349f, 0.271f }; // the engine's empty-track teal
constexpr bool SquareHealthEdge = false;              // true: swap the health bar to a squared left edge (shield joins it with a seam)
constexpr bool DrawSeam = false;                      // user (10-01): keep the health bar's own curved left highlight, no seam between
constexpr int MaxSegments = 3;
constexpr float ShieldWidthFraction = 0.55f;          // of the health bar rect width
// Fill = the shield's VFX hue with its saturation cut (HSV, hue and value kept): red -> salmon, blue -> pale blue. Saturation is
// scaled by ShieldSatScale but never below ShieldMinSat (so already-pale hues do not wash out to white) and never raised.
constexpr float ShieldSatScale = 0.50f, ShieldMinSat = 0.30f;
const float SeamTint[3] = { 0.0f, 0.16f, 0.14f };           // dark-teal groove between shield and health
const char* const ShieldBarTexture = "uibit_bar_sh_p";     // curved outer-left, outline-free right edge (scripts/gen_shield_bar_art.py)
const char* const CornerTexture = "uibit_bar_sc_p";          // wedge that fills the card-frame corner notch (drawn on the health rect)
const char* const HealthSquareTexture = "uibit_bar_vp_sq"; // health bar with a squared, outline-free left edge
constexpr int Slot_VitBarOffset = 0x690, Bar_Border1Offset = 0x6c, Bar_Border2Offset = 0xe4, BorderVtable = 0x009875bc;
constexpr int Eff_Pool = 0x02, Eff_Visual = 0x1e;
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
    if (!readAt(creature, Creature_EffectArrayOffset, &arr) || !readAt(creature, Creature_EffectCountOffset, &count)) return 0;
    if (arr == 0 || count <= 0 || count > MaxEffects) return 0;
    ++g_maxStamp;
    int n = 0;
    for (int i = 0; i < count && n < maxOut; ++i) {
        int eff = 0; unsigned short type = 0, sub = 0;
        if (!readAt(arr, i * 4, &eff) || !eff || !readAt(eff, 0x8, &type) || !readAt(eff, 0xa, &sub)) continue;
        if (type != Eff_Pool || !(sub & 8)) continue;
        int ip = 0, cnt = 0, spell = 0;
        if (!readAt(eff, 0x30, &cnt) || cnt < 3 || !readAt(eff, 0x34, &ip) || !ip || !readAt(eff, Effect_SpellIdOffset, &spell)) continue;
        int flags = 0, remaining = 0;
        if (!readAt(ip, 0, &flags) || !readAt(ip, 8, &remaining) || remaining <= 0) continue;
        int vfx = 0;
        for (int k = 0; k < count; ++k) {
            int e2 = 0; unsigned short t2 = 0; int sp2 = 0, ip2 = 0;
            if (!readAt(arr, k * 4, &e2) || !e2 || !readAt(e2, 0x8, &t2) || t2 != Eff_Visual) continue;
            if (!readAt(e2, Effect_SpellIdOffset, &sp2) || sp2 != spell || !readAt(e2, 0x34, &ip2) || !ip2) continue;
            readAt(ip2, 0, &vfx); break;
        }
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
        len += snprintf(out + len, cap - len, "%s%d/%d", i ? " + " : "", pools[i].remaining, pools[i].max);
    return n;
}

// While a creature has a shield, the health bar's two borders (track + fill) draw a squared-left texture so the shield
// bar and the health bar read as one bar. Swapped just before the engine draws (UpdatePortraits snapshot), restored
// right after (our slot hook), so the engine never keeps our image pointer.
struct SwapRec { int slot; int b[2]; int orig[2]; bool active; };
SwapRec g_swap[4];
void swapHealthBar(int slotPtr, bool on) {
    SwapRec* r = nullptr;
    for (auto& e : g_swap) if (e.slot == slotPtr) { r = &e; break; }
    if (!r) { if (!on) return; for (auto& e : g_swap) if (e.slot == 0) { r = &e; e.slot = slotPtr; break; } }
    if (!r || r->active == on) return;
    if (on) {
        void* img = imageFor(HealthSquareTexture);
        if (!img) return;
        int bar = slotPtr + Slot_VitBarOffset, bs[2] = { bar + Bar_Border1Offset, bar + Bar_Border2Offset }, vt[2], cur[2];
        for (int k = 0; k < 2; ++k)
            if (!readAt(bs[k], 0, &vt[k]) || vt[k] != BorderVtable || !readAt(bs[k], Border_FillImageOffset, &cur[k]) || !cur[k]) return;
        for (int k = 0; k < 2; ++k) { r->b[k] = bs[k]; r->orig[k] = cur[k]; *reinterpret_cast<void**>(bs[k] + Border_FillImageOffset) = img; }
        r->active = true;
    } else {
        for (int k = 0; k < 2; ++k) *reinterpret_cast<int*>(r->b[k] + Border_FillImageOffset) = r->orig[k];
        r->active = false;
    }
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
    Pool pools[8];
    int n = collectPools(creature, pools, 8);
    if (n <= 0) return 0;
    void* image = imageFor(ShieldBarTexture);
    if (!image) return -1;
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
    float flushArg = 0.0f; readAt(0x009f5f60, 0, &flushArg);
    auto flush = reinterpret_cast<void(__cdecl*)(float)>(FlushGuiBatch);
    auto pushClip = reinterpret_cast<int(__cdecl*)(int, int, int, int, const void*, int, float)>(PushClipRect);
    auto popClip = reinterpret_cast<void(__cdecl*)()>(PopClipRect);
    auto draw = reinterpret_cast<void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float)>(
        (*reinterpret_cast<void***>(image))[6]);
    flush(flushArg);
    // Everything is clipped to [x, seamX): the art's body now extends past its visible edge, so the right edge is pixel-exact on the seam
    // (a hard clip edge, no texture-filter fringe = no hairline between shield, corner wedge and health bar).
    if (pushClip(x, y, V, H, ClipNoFillColor, 0, 1.0f)) { draw(image, 0, 0, W, H, 0, 0, TrackTint, 1.0f); popClip(); }   // empty track: the engine's neutral teal
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
            int pushed = pushClip(x, y0, V, fillH, ClipNoFillColor, 0, 1.0f);
            { static int lastPush[8]; static int pushSlot[8]; static int nPush = 0;
              int pi = -1; for (int k = 0; k < nPush; ++k) if (pushSlot[k] == slotPtr) pi = k;
              if (pi < 0 && nPush < 8) { pi = nPush++; pushSlot[pi] = slotPtr; lastPush[pi] = -1; }
              int psig = pushed ? 1 : 0;
              if (pi >= 0 && lastPush[pi] != psig) { lastPush[pi] = psig; probeLog("FILL slot=%08x seg=%d pushClip=%d rect=(%d,%d,%d,%d) vit=(%d,%d,%d,%d) n=%d", slotPtr, i, pushed, x, y0, W, fillH, vx, vy, vw, vh, n); } }
            if (pushed) {
                draw(image, 0, -(y0 - y), W, H, 0, 0, lit, 1.0f);
                popClip();
            }
            }
        }
        baseUp += segH;
    }
    // Seam between shield and health: a thin DARK line (the card border's own deep teal), not a bright bar, so the two bars read
    // as one gauge with a groove between them. 2 px on the leader card, 1 px on small cards, full bar height.
    int dw = vw / 50 < 1 ? 1 : vw / 50;
    // Corner fillers: the card frame's rounded corner leaves a dark notch above/below the health bar's outline, right of the shield's
    // right edge. uibit_bar_sc_p (scripts/gen_shield_bar_art.py) is drawn on the HEALTH rect and holds only the wedge between x=0.445*vw and
    // the health art's curved outline, so it extends the shield's top/bottom edge into the corner without touching a health pixel.
    if (void* corner = imageFor(CornerTexture)) {
        auto cdraw = reinterpret_cast<void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float)>((*reinterpret_cast<void***>(corner))[6]);
        float top[3], bot[3];
        bool topFull = pools[n - 1].remaining >= pools[n - 1].max, botFull = pools[0].remaining > 0;
        lightenFill(pools[n - 1].rgb, top); lightenFill(pools[0].rgb, bot);
        const float* tc = topFull ? top : TrackTint; const float* bc = botFull ? bot : TrackTint;
        if (noClip) cdraw(corner, vx, vy, vw, vh, 0, 0, bc, 1.0f);
        else {
            int half = vh / 2;
            if (pushClip(vx, vy, vw, half, ClipNoFillColor, 0, 1.0f)) { cdraw(corner, 0, 0, vw, vh, 0, 0, tc, 1.0f); popClip(); }
            if (pushClip(vx, vy + half, vw, vh - half, ClipNoFillColor, 0, 1.0f)) { cdraw(corner, 0, -half, vw, vh, 0, 0, bc, 1.0f); popClip(); }
        }
    }
    if (DrawSeam && !noClip && pushClip(seamX - dw, y, dw, H, SeamTint, 0, 0.85f)) popClip();
    flush(flushArg);
    return n;
}


// ---- blackout diagnostics (flag files, polled every 30 frames; work mid-session, no restart):
//   shield_nodraw.txt  -> skip the shield bar draws        shield_noswap.txt -> skip the health-bar texture swap
//   buffhud_off.txt    -> skip the Buff HUD icon column (engine arrows draw natively)
// F9 = write a MARK line with a full GL state dump + scene brightness sample. While shields turn on/off the GL state is dumped
// before/after our draws for 4 frames (PRE/POST) and the scene brightness is sampled every 30 frames (SCENE dark=0/1). ----
bool g_noDraw = false, g_noSwap = false, g_noBuff = false;
bool fileExists(const char* n) { return GetFileAttributesA(n) != INVALID_FILE_ATTRIBUTES; }
struct GlFns {
    bool tried = false, ok = false;
    int (WINAPI* getError)();
    void (WINAPI* getIntegerv)(unsigned, int*);
    void (WINAPI* getBooleanv)(unsigned, unsigned char*);
    unsigned char (WINAPI* isEnabled)(unsigned);
    void (WINAPI* readPixels)(int, int, int, int, unsigned, unsigned, void*);
    void (WINAPI* enable)(unsigned);
    void (WINAPI* disable)(unsigned);
} g_gl;
void glInit() {
    if (g_gl.tried) return;
    g_gl.tried = true;
    HMODULE m = GetModuleHandleA("opengl32.dll");
    if (!m) return;
    g_gl.getError = reinterpret_cast<decltype(g_gl.getError)>(GetProcAddress(m, "glGetError"));
    g_gl.getIntegerv = reinterpret_cast<decltype(g_gl.getIntegerv)>(GetProcAddress(m, "glGetIntegerv"));
    g_gl.getBooleanv = reinterpret_cast<decltype(g_gl.getBooleanv)>(GetProcAddress(m, "glGetBooleanv"));
    g_gl.isEnabled = reinterpret_cast<decltype(g_gl.isEnabled)>(GetProcAddress(m, "glIsEnabled"));
    g_gl.readPixels = reinterpret_cast<decltype(g_gl.readPixels)>(GetProcAddress(m, "glReadPixels"));
    g_gl.enable = reinterpret_cast<decltype(g_gl.enable)>(GetProcAddress(m, "glEnable"));
    g_gl.disable = reinterpret_cast<decltype(g_gl.disable)>(GetProcAddress(m, "glDisable"));
    g_gl.ok = g_gl.getError && g_gl.getIntegerv && g_gl.getBooleanv && g_gl.isEnabled;
    probeLog("GLINIT ok=%d readPixels=%d enable/disable=%d", g_gl.ok, g_gl.readPixels != nullptr, g_gl.enable && g_gl.disable);
}
void glDump(const char* tag) {
    if (!g_probe) return;
    glInit();
    if (!g_gl.ok) return;
    int vp[4] = {}, sc[4] = {}, prog = 0, fbo = 0, tex = 0, mm = 0, mvd = 0, prd = 0, bsrc = 0, bdst = 0, drawbuf = 0;
    unsigned char dm = 0, cm[4] = {};
    g_gl.getIntegerv(0x0BA2, vp); g_gl.getIntegerv(0x0C10, sc);
    g_gl.getIntegerv(0x8B8D, &prog); g_gl.getIntegerv(0x8CA6, &fbo); g_gl.getIntegerv(0x8069, &tex);
    g_gl.getIntegerv(0x0BA0, &mm); g_gl.getIntegerv(0x0BA3, &mvd); g_gl.getIntegerv(0x0BA4, &prd);
    g_gl.getIntegerv(0x0BE1, &bsrc); g_gl.getIntegerv(0x0BE0, &bdst); g_gl.getIntegerv(0x0C01, &drawbuf);
    g_gl.getBooleanv(0x0B72, &dm); g_gl.getBooleanv(0x0C23, cm);
    int clipDepth = 0; readAt(0x00a309c0, 0, &clipDepth);
    probeLog("GL %s vp=(%d,%d,%d,%d) scissor=(%d,%d,%d,%d) en[scis=%d sten=%d depth=%d light=%d blend=%d fog=%d tex2d=%d cull=%d alpha=%d vprog=%d fprog=%d] depthmask=%d colormask=%d%d%d%d prog=%d fbo=%d tex=%d drawbuf=%x blend=%x/%x mm=%x mvdepth=%d prdepth=%d clipDepth=%d",
             tag, vp[0], vp[1], vp[2], vp[3], sc[0], sc[1], sc[2], sc[3],
             g_gl.isEnabled(0x0C11), g_gl.isEnabled(0x0B90), g_gl.isEnabled(0x0B71), g_gl.isEnabled(0x0B50), g_gl.isEnabled(0x0BE2),
             g_gl.isEnabled(0x0B60), g_gl.isEnabled(0x0DE1), g_gl.isEnabled(0x0B44), g_gl.isEnabled(0x0BC0), g_gl.isEnabled(0x8620), g_gl.isEnabled(0x8804),
             dm, cm[0], cm[1], cm[2], cm[3], prog, fbo, tex, drawbuf, bsrc, bdst, mm, mvd, prd, clipDepth);
    for (int i = 0; i < 4; ++i) { int e = g_gl.getError(); if (!e) break; probeLog("GL %s glGetError=0x%x", tag, e); }
}
// average brightness (0..255) of a 5x5 grid over the 3D scene area; -1 when unavailable
int sceneBrightness() {
    glInit();
    if (!g_gl.readPixels) return -1;
    int w = 0, h = 0; readAt(0x009f42a4, 0, &w); readAt(0x009f42a8, 0, &h);
    if (w < 100 || h < 100) return -1;
    long sum = 0; int cnt = 0;
    for (int iy = 0; iy < 5; ++iy) for (int ix = 0; ix < 5; ++ix) {
        unsigned char px[16] = {};
        g_gl.readPixels(w * (15 + 8 * ix) / 100, h * (25 + 9 * iy) / 100, 1, 1, 0x1908, 0x1401, px);
        sum += px[0] + px[1] + px[2]; ++cnt;
    }
    while (g_gl.getError()) {}
    return int(sum / (3 * cnt));
}

// Our GUI draws leave GL_LIGHTING enabled (probe 02:34: PRE light=0 -> POST light=1; the engine's colour-fill clip / batch flush
// toggle it). Snapshot the on/off capabilities before our draws and put them back afterwards, every frame, probe flag or not.
struct CapGuard {
    static constexpr unsigned Caps[9] = { 0x0C11, 0x0B90, 0x0B71, 0x0B50, 0x0BE2, 0x0B60, 0x0DE1, 0x0B44, 0x0BC0 };
    unsigned char before[9] = {}; bool armed = false;
    CapGuard() {
        glInit();
        if (!g_gl.ok || !g_gl.enable || !g_gl.disable) return;
        for (int i = 0; i < 9; ++i) before[i] = g_gl.isEnabled(Caps[i]);
        armed = true;
    }
    ~CapGuard() {
        if (!armed) return;
        for (int i = 0; i < 9; ++i) {
            unsigned char now = g_gl.isEnabled(Caps[i]);
            if (now != before[i]) {
                static int logged = 0;
                if (logged < 20) { ++logged; probeLog("CAPFIX cap=%x was=%d now=%d -> restored", Caps[i], before[i], now); }
                (before[i] ? g_gl.enable : g_gl.disable)(Caps[i]);
            }
        }
    }
};
constexpr unsigned CapGuard::Caps[9];
} // namespace

extern "C" {

void __cdecl SnapshotPortraitSlot(int slotPtr, int creaturePtr) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
    static int n = 0; if (n < 60) { ++n; logf("snapshot slot=%08x creature=%08x", slotPtr, creaturePtr); }
    if (g_probe && creaturePtr) probeEffects(slotPtr, creaturePtr);
    if (creaturePtr) { Pool tmp[8]; swapHealthBar(slotPtr, SquareHealthEdge && !g_noSwap && collectPools(creaturePtr, tmp, 8) > 0); }
    const DWORD now = GetTickCount();
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) { e.creaturePtr = creaturePtr; e.tick = now; return; }
    for (auto& e : g_slots) if (e.slotPtr == 0) { e = { slotPtr, creaturePtr, now }; return; }
    static int evict = 0; g_slots[evict++ & 3] = { slotPtr, creaturePtr, now };   // HUD rebuilt (area load): replace a stale entry
}

// Hook 2 (per native arrow, buff row): the icon column is drawn by DrawBuffColumn, so hide
// the first `tracked` native arrows (CSWGuiBorder::Draw returns at once when +0xc/+0x10 == 0).
void __cdecl RestoreBuffIcon();
void __cdecl RetargetBuffIcon(int slotPtr, int index, int iconWidget, int whichRow) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
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
    if (logIt) logf("arrow row=%d slot=%08x idx=%d tracked=%d -> %s", whichRow, slotPtr, index, n, index < n ? "suppress" : "keep");
    if (index >= n) return;                     // untracked engine buff: leave its arrow

    int border = iconWidget + Label_TextBorderOffset;
    int w = 0, h = 0, bvt = 0;
    if (!readAt(border, 0, &bvt) || bvt != 0x009875bc || !readAt(border, Border_WidthOffset, &w) || !readAt(border, Border_HeightOffset, &h)) return;
    g_saved ={ true, border, 0, w, h };
    *reinterpret_cast<int*>(border + Border_WidthOffset) = 0;
    *reinterpret_cast<int*>(border + Border_HeightOffset) = 0;
}

void __cdecl RestoreBuffIcon() {
    if (!g_saved.active) return;
    g_saved.active = false;
    *reinterpret_cast<int*>(g_saved.border + Border_WidthOffset) = g_saved.width;
    *reinterpret_cast<int*>(g_saved.border + Border_HeightOffset) = g_saved.height;
}

// Hook 5 (once per row call, after the native loop, still inside the card's viewport whose
// origin is the card's top-left): draw our own icon column down the card's left edge.
// Core icon-column drawer: origin = the card's top-left (the caller is inside a clip/viewport at the card). iconWidget may be 0 (menu
// companion portraits have no icon widget): then the draw uses the stock defaults (white tint, alpha 1, no rotation).
static void drawColumnCore(int slotPtr, int creature, int iconWidget, int isLarge, int whichRow, int cardW, int cardH) {
    const bool debuff = whichRow == 0;
    static int callNo = 0; ++callNo;
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16], other[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16, debuff);
    int nOther = collectBuffs(creature, day, ms, haveTime, other, 16, !debuff);
    int nSize = n > nOther ? n : nOther;        // both columns share one icon size

    float scale = 1.0f;
    { float s = reinterpret_cast<float(__cdecl*)()>(HudScaleFn)(); if (s > 0.1f && s < 10.0f) scale = s; }
    int gap = int(scale + 0.5f); if (gap < 1) gap = 1;
    // preferred size (smaller on the little companion cards so the face stays visible), then
    // shrink so all n fit down the column; below MinIconSize add another column instead.
    int size = int(cardW * (isLarge ? LargeCardFraction : SmallCardFraction));
    int maxSize = int(MaxIconSize * scale), minSize = int(MinIconSize * scale);
    if (size > maxSize) size = maxSize;
    if (size < minSize) size = minSize;
    int cols = 1;
    for (;; ++cols) {
        int perCol = (nSize + cols - 1) / cols; if (perCol < 1) perCol = 1;
        int fitSize = (cardH - gap) / perCol - gap;
        if (fitSize < size) size = fitSize > 0 ? fitSize : 1;
        if (size >= minSize || cols >= MaxColumns) break;
        size = int(cardW * (isLarge ? LargeCardFraction : SmallCardFraction));
        if (size > maxSize) size = maxSize;
    }
    int perCol = (cardH - gap) / (size + gap); if (perCol < 1) perCol = 1;
    int fit = perCol * cols;
    int drawN = n < fit ? n : fit;

    static int lastSig[8]; static int sigSlot[8]; static int nSig = 0;
    int si = -1; for (int k = 0; k < nSig; ++k) if (sigSlot[k] == slotPtr) si = k;
    if (si < 0 && nSig < 8) { si = nSig++; sigSlot[si] = slotPtr; lastSig[si] = -1; }
    int sig = (n << 16) ^ (debuff << 4) ^ (nOther << 12) ^ (size << 8) ^ (cardW << 20) ^ cardH ^ isLarge;
    bool logIt = callNo <= 30 || (si >= 0 && lastSig[si] != sig);
    if (si >= 0) lastSig[si] = sig;
    if (logIt) logf("COLUMN#%d %s slot=%08x large=%d card=%dx%d scale=%.3f tracked=%d cols=%d fit=%d draw=%d size=%d gap=%d",
                    callNo, debuff ? "DEBUFF" : "buff", slotPtr, isLarge, cardW, cardH, scale, n, cols, fit, drawN, size, gap);
    if (drawN <= 0) return;

    CapGuard capGuard;                          // same GL_LIGHTING leak as DrawShieldBar: flush/pushClip toggle caps, put them back on exit
    int border = iconWidget ? iconWidget + Label_TextBorderOffset : 0;
    static const float whiteTint[3] = { 1.0f, 1.0f, 1.0f };
    int rotBits = 0; if (border) readAt(border, 0x20, &rotBits);
    const float* color = border ? reinterpret_cast<const float*>(border + 0x28) : whiteTint;
    float baseAlpha = 1.0f; if (border) readAt(border, 0x24, &baseAlpha);
    float flushArg = 0.0f; readAt(0x009f5f60, 0, &flushArg);
    auto flush = reinterpret_cast<void(__cdecl*)(float)>(FlushGuiBatch);
    auto pushClip = reinterpret_cast<int(__cdecl*)(int, int, int, int, const void*, int, float)>(PushClipRect);
    auto popClip = reinterpret_cast<void(__cdecl*)()>(PopClipRect);

    auto draw2 = [&](void* img, int x, int y, int w, int h, const float* tint, float alpha) {
        reinterpret_cast<void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float)>(
            (*reinterpret_cast<void***>(img))[6])(img, x, y, w, h, 0, rotBits, tint, alpha);
    };
    flush(flushArg);
    for (int i = 0; i < drawN; ++i) {
        void* image = imageFor(buffs[i].resref);
        if (!image) { if (logIt) logf("  icon[%d] %s: no image", i, buffs[i].resref); continue; }
        auto draw = reinterpret_cast<void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float)>(
            (*reinterpret_cast<void***>(image))[6]);
        int colIdx = i / perCol;
        int ix = debuff ? cardW - gap - size - colIdx * (size + gap) - RightInset : gap + colIdx * (size + gap), iy = gap + (i % perCol) * (size + gap);
        float ratio = buffs[i].ratio;
        int bright = int(size * ratio + 0.5f);
        if (bright > size) bright = size;
        if (logIt) logf("  icon[%d] %s rect=(%d,%d,%d,%d) ratio=%.3f bright=%d dimAlpha=%.2f", i, buffs[i].resref, ix, iy, size, size, ratio, bright, DimAlpha);
        void* disc = buffs[i].item ? imageFor(CircleResref) : nullptr;   // round backing for item abilities only
        // elapsed part (whole square, dim): backing disc + icon
        if (disc) draw2(disc, ix, iy, size, size, BgColor, BgAlpha * DimAlpha);
        draw(image, ix, iy, size, size, 0, rotBits, color, baseAlpha * DimAlpha);
        if (bright > 0) {                                                               // remaining part, bottom-anchored
            int y0 = iy + size - bright;
            int ok = pushClip(ix, y0, size, bright, ClipNoFillColor, 0, 1.0f);
            if (ok) {
                if (disc) draw2(disc, 0, -(size - bright), size, size, BgColor, BgAlpha);
                draw(image, 0, -(size - bright), size, size, 0, rotBits, color, baseAlpha);
                popClip();
                if (disc && bright < size) {                                                     // 1px boundary line (round item icons only)
                    int lx = ix, lw = size;
                    if (disc) {                                     // item icons: only inside the round backing
                        float r = size * 0.5f - 0.5f, dy = (y0 + 0.5f) - (iy + size * 0.5f);
                        float chord2 = r * r - dy * dy;
                        lw = chord2 > 0.0f ? int(2.0f * sqrtf(chord2) + 0.5f) : 0;
                        lx = ix + (size - lw) / 2;
                    }
                    if (lw > 0 && pushClip(lx, y0, lw, 1, LineColor, 0, LineAlpha)) popClip();
                }
            } else if (logIt) logf("  icon[%d] clip push refused", i);
        }
    }
    flush(flushArg);
}

void __cdecl DrawBuffColumn(int slotPtr, int iconWidget, int isLarge, int whichRow, int cardW, int cardH) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
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
struct ArrowRaw { int slot; unsigned char buff, debuff, nBuff, nDebuff; };
ArrowRaw g_arrowRaw[8];

ArrowRaw* arrowRawFor(int slotPtr) {
    for (auto& r : g_arrowRaw) if (r.slot == slotPtr) return &r;
    for (auto& r : g_arrowRaw) if (r.slot == 0) { r = {}; r.slot = slotPtr; return &r; }
    return nullptr;
}

void ForceArrowCounters(int slotPtr) {
    if (g_noBuff) return;
    int gate = 0;
    if (!readAt(slotPtr, 0x590, &gate) || !(gate & 2)) return;              // engine's own "row visible" bit (also excludes the creature-less mirror slot)
    unsigned char rawBuff = 0, rawDebuff = 0;
    if (!readAt(slotPtr, 0xddc, &rawBuff) || !readAt(slotPtr, 0xddd, &rawDebuff)) return;
    int creature = creatureForSlot(slotPtr);
    if (!creature) return;
    ArrowRaw* raw = arrowRawFor(slotPtr);
    int overlay[3] = {};                                                    // FUN_00744f00: mask&2 -> +0x1b8, mask&1 -> +0x300, mask&4 -> +0x448 (bit 1 = shown)
    readAt(slotPtr, 0x1b8, &overlay[0]); readAt(slotPtr, 0x300, &overlay[1]); readAt(slotPtr, 0x448, &overlay[2]);
    bool hidden = ((overlay[0] | overlay[1] | overlay[2]) & 2) != 0;        // engine deliberately hid the arrows (dead/disabled): keep that
    int nBuff = 0, nDebuff = 0;
    if (!hidden) {
        int day = 0, ms = 0;
        bool haveTime = currentTime(&day, &ms);
        Buff tmp[16];
        nBuff = collectBuffs(creature, day, ms, haveTime, tmp, 16, false);
        nDebuff = collectBuffs(creature, day, ms, haveTime, tmp, 16, true);
    }
    unsigned char newBuff = rawBuff, newDebuff = rawDebuff;
    if (nBuff > 0 && rawBuff == 0) newBuff = 1;                             // never lower an engine value
    if (nDebuff > 0 && rawDebuff == 0) newDebuff = 1;
    if (newBuff != rawBuff) *reinterpret_cast<unsigned char*>(slotPtr + 0xddc) = newBuff;
    if (newDebuff != rawDebuff) *reinterpret_cast<unsigned char*>(slotPtr + 0xddd) = newDebuff;
    if (raw) {
        unsigned char sig[4] = { rawBuff, rawDebuff, (unsigned char)nBuff, (unsigned char)nDebuff };
        bool changed = memcmp(sig, &raw->buff, 4) != 0;
        raw->buff = rawBuff; raw->debuff = rawDebuff; raw->nBuff = (unsigned char)nBuff; raw->nDebuff = (unsigned char)nDebuff;
        if (changed) {
            logf("ARROWS slot=%08x creature=%08x raw=(%d,%d) tracked=(%d,%d) -> counters=(%d,%d) overlay=(%08x,%08x,%08x)%s",
                 slotPtr, creature, rawBuff, rawDebuff, nBuff, nDebuff, newBuff, newDebuff, overlay[0], overlay[1], overlay[2], hidden ? " HIDDEN" : "");
            probeLog("ARROWS slot=%08x creature=%08x raw=(%d,%d) tracked=(%d,%d) -> counters=(%d,%d) overlay=(%08x,%08x,%08x) gate590=%08x%s",
                     slotPtr, creature, rawBuff, rawDebuff, nBuff, nDebuff, newBuff, newDebuff, overlay[0], overlay[1], overlay[2], gate, hidden ? " HIDDEN" : "");
            // Phase 0: the engine's own count comes from the icon cache at creature+0xFFC (data) / +0x1000 (count).
            int cacheData = 0, cacheCount = 0;
            readAt(creature, 0xFFC, &cacheData); readAt(creature, 0x1000, &cacheCount);
            char ent[256]; int len = 0; ent[0] = 0;
            for (int k = 0; k < 6 && k < cacheCount && cacheData; ++k) {
                int e = 0; short id = 0; int f14 = 0, f1c = 0;
                if (!readAt(cacheData, k * 4, &e) || !e) continue;
                readAt(e, 0, &id); readAt(e, 0x14, &f14); readAt(e, 0x1c, &f1c);
                len += snprintf(ent + len, sizeof(ent) - len, " [%d id=%d f14=%d f1c=%d]", k, id, f14, f1c);
            }
            probeLog("ICONCACHE slot=%08x creature=%08x count=%d%s", slotPtr, creature, cacheCount, ent);
        }
    }
}

// Menu screens (character/inventory/equip/abilities...): CSWGuiInGameMenu's draw (vtable slot 14, 0x756b70) draws its panel (0x40ec90 =
// BeginGui2D 0x4792c0, push the panel rect as clip/viewport, controls, pop, EndGui2D 0x4793f0), THEN calls 0x745770 with ECX = its portrait
// slot (menu+0x2350). At that point the 2D batch is closed (clip stack index 0xFFFF, ortho matrix popped, viewport garbage), which is why
// drawing there showed nothing. So we reopen the batch exactly as the panel does: BeginGui2D, push the panel rect (0x410020), draw in
// panel-local coordinates (the slot's PB_VIT rect, the LBL_CHAR2/3 companion portraits), pop, EndGui2D.
// The selected creature = caller's local [callerEbp-0x38] (= FUN_0077d800(handle), the object the HUD also uses; [-0x28] is the handle).
// Companions = roster positions 1 and 2 (the menu sets LBL_CHAR2/3 portraits from FUN_007e5da0(roster, 1/2)).
constexpr int Menu_SlotOffset = 0x2350, Menu_Char2Offset = 0x3134, Menu_Char3Offset = 0x36DC;
constexpr int BeginGui2D = 0x004792c0, EndGui2D = 0x004793f0, PanelRectFn = 0x00410020;
constexpr int RosterAccessor = 0x0073fb90, RosterMember = 0x007e5da0, MemberCreature = 0x0077d800;

int rosterCreature(int idx) {
    int app = 0, internal = 0;
    if (!readAt(0x00a1b4a4, 0, &app) || !readAt(app, 4, &internal) || !internal) return 0;
    using F0 = int(__thiscall*)(int);
    using F1 = int(__thiscall*)(int, int);
    int roster = reinterpret_cast<F0>(RosterAccessor)(internal);
    if (!roster) return 0;
    int handle = reinterpret_cast<F1>(RosterMember)(roster, idx);
    if (!handle) return 0;
    int c = reinterpret_cast<F0>(MemberCreature)(handle);
    int cnt = 0, arr = 0;
    if (!c || !readAt(c, Creature_EffectCountOffset, &cnt) || !readAt(c, Creature_EffectArrayOffset, &arr) || cnt < 0 || cnt > MaxEffects) return 0;
    return c;
}

// ---- Menu companion portraits: health / force / shield bars + room for them -----------------------------------------------------
// The menu's LBL_CHAR2/3 portraits are plain images. We draw the HUD small card's bars around them (same art + tints as PB_VIT/PB_FORCE:
// uibit_bar_vp_p red over teal, uibit_bar_fp_p cyan over teal; geometry copied from the HUD small card: vit = card.x-40 .. +47 wide, force =
// card.x+77 .. +47 wide, both 96 tall from card.y-5, on an 85 px card). Values the way UpdatePortraits gets them (0x74ee90): HP = creature
// vtable+0x9c(0, max) with max = vtable+0x98(1, creature[0x43d]); FP = stats(+0x1198) shorts +0x130 + +0x132, max = FUN_0057eca0(creature).
constexpr int MenuCharGroup[2][4] = { { 0xc4d, 0xc9f, 0xcf1, 0xd43 }, { 0xdb7, 0xe09, 0xe5b, 0xead } };   // dword index of LBL_CHARn, LEVELUPn, BACKn, BTN_CHANGEn
const float HealthFillRed[3] = { 1.0f, 0.0f, 0.0f }, ForceFillCyan[3] = { 0.0f, 0.8f, 0.898f };
const char* const HealthBarTexture = "uibit_bar_vp_p";
const char* const ForceBarTexture = "uibit_bar_fp_p";

bool readVitals(int creature, int* hp, int* hpMax, int* fp, int* fpMax) {
    int vt = 0, stats = 0;
    if (!readAt(creature, 0, &vt) || !vt || !readAt(creature, 0x1198, &stats) || !stats) return false;
    int fnMax = 0, fnCur = 0; short s130 = 0, s132 = 0;
    if (!readAt(vt, 0x98, &fnMax) || !readAt(vt, 0x9c, &fnCur) || !fnMax || !fnCur || !readAt(stats, 0x130, &s130) || !readAt(stats, 0x132, &s132)) return false;
    // CSWSCreature vtable slots 38/39 (0x57e8c0 / 0x5413c0) each take ONE stack arg (RET 4): a 2-arg call unbalances the stack (crash 10-02)
    *hpMax = reinterpret_cast<short(__thiscall*)(int, int)>(fnMax)(creature, 1);
    *hp = reinterpret_cast<short(__thiscall*)(int, int)>(fnCur)(creature, 0);
    *fp = int(s130) + int(s132);
    *fpMax = int(reinterpret_cast<unsigned(__fastcall*)(int)>(0x0057eca0)(creature));
    return true;
}

// one vertical gauge: teal track over the whole rect, then the colour fill from the bottom (cur/max of the height), as the stock bars
void drawGauge(const char* texture, int x, int y, int w, int h, const float* fill, int cur, int mx) {
    void* img = imageFor(texture);
    if (!img || w < 4 || h < 4) return;
    if (cur < 0) cur = 0; if (cur > mx) cur = mx;
    auto draw = reinterpret_cast<void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float)>((*reinterpret_cast<void***>(img))[6]);
    if (mx < 1) { draw(img, x, y, w, h, 0, 0, TrackTint, 1.0f); return; }     // no Force (droid): empty teal track, as the active portrait shows
    auto pushClip = reinterpret_cast<int(__cdecl*)(int, int, int, int, const void*, int, float)>(PushClipRect);
    auto popClip = reinterpret_cast<void(__cdecl*)()>(PopClipRect);
    draw(img, x, y, w, h, 0, 0, TrackTint, 1.0f);
    int fh = int(double(h) * cur / mx + 0.5), y0 = y + h - fh;
    if (fh > 0 && pushClip(x, y0, w, fh, ClipNoFillColor, 0, 1.0f)) { draw(img, 0, -(y0 - y), w, h, 0, 0, fill, 1.0f); popClip(); }
}

// Spread the two companion portrait groups apart (runtime write of the controls' x, so no .gui edit and the hit areas move too).
// Shifts are remembered per panel; a layout we did not write (menu rebuilt) is adopted as the new original.
struct ShiftRec { int panel; int orig[2][4], shifted[2][4]; };
ShiftRec g_shift[8];
void spreadCompanions(int panel, int w) {
    ShiftRec* rec = nullptr;
    for (auto& r : g_shift) if (r.panel == panel) { rec = &r; break; }
    if (!rec) { static int next = 0; rec = &g_shift[next++ & 7]; *rec = {}; rec->panel = panel; }
    const int dx[2] = { int(0.30f * w + 0.5f), int(1.00f * w + 0.5f) };
    for (int g = 0; g < 2; ++g) for (int i = 0; i < 4; ++i) {
        int ctl = panel + MenuCharGroup[g][i] * 4, rc[4] = {};
        for (int q = 0; q < 4; ++q) readAt(ctl, 4 + 4 * q, &rc[q]);
        if (rc[2] < 1 || rc[3] < 1) continue;
        int cur = rc[0];
        if (cur != rec->orig[g][i] && cur != rec->shifted[g][i]) { rec->orig[g][i] = cur; rec->shifted[g][i] = cur + dx[g]; }
        if (cur == rec->shifted[g][i]) continue;
        // engine SetRect (vtable slot 1, RET 4: moves the rect + border/text copies, so the drawn portrait follows; raw int writes only moved the hit area)
        int vt = 0, fn = 0;
        if (!readAt(ctl, 0, &vt) || !vt || !readAt(vt, 4, &fn) || !fn) continue;
        int nr[4] = { rec->shifted[g][i], rc[1], rc[2], rc[3] };
        reinterpret_cast<void(__thiscall*)(int, int*)>(fn)(ctl, nr);
        probeLog("MENUMOVE g=%d i=%d x %d -> %d", g, i, cur, nr[0]);
    }
}

void DrawMenuOverlay(int slotPtr, int callerEbp, float scale) {
    int panel = slotPtr - Menu_SlotOffset;
    int creature = 0, v[4] = {};
    readAt(callerEbp, -0x38, &creature);
    for (int k = 0; k < 4; ++k) readAt(slotPtr, 0x694 + 4 * k, &v[k]);
    int pools = 0;
    if (creature && v[2] >= 1 && v[2] <= 2000 && v[3] >= 1 && v[3] <= 2000) { Pool t[8]; pools = collectPools(creature, t, 8); }
    int comp[2] = { rosterCreature(1), rosterCreature(2) };
    static int lastSig = -1; int sig = slotPtr ^ creature ^ (pools << 28) ^ v[0] ^ (v[1] << 4) ^ (v[2] << 8) ^ (v[3] << 12) ^ comp[0] ^ (comp[1] << 3);
    if (sig != lastSig) { lastSig = sig; probeLog("MENUSLOT slot=%08x callerEbp=%08x creature=%08x pools=%d vit=(%d,%d,%d,%d) companions=%08x,%08x", slotPtr, callerEbp, creature, pools, v[0], v[1], v[2], v[3], comp[0], comp[1]); }
    int pr[4] = {};
    reinterpret_cast<void(__thiscall*)(int, int*)>(PanelRectFn)(panel, pr);
    if (pr[2] < 1 || pr[3] < 1 || pr[2] > 8192 || pr[3] > 8192) { probeLog("MENUDRAW bad panel rect %d,%d,%d,%d", pr[0], pr[1], pr[2], pr[3]); return; }
    CapGuard capGuard;
    auto pushClip = reinterpret_cast<int(__cdecl*)(int, int, int, int, const void*, int, float)>(PushClipRect);
    auto popClip = reinterpret_cast<void(__cdecl*)()>(PopClipRect);
    reinterpret_cast<void(__cdecl*)()>(BeginGui2D)();
    int shown = 0, cols = 0;
    if (pushClip(pr[0], pr[1], pr[2], pr[3], ClipNoFillColor, 0, 1.0f)) {
        if (pools > 0) shown = DrawShieldPools(creature, v[0], v[1], v[2], v[3], scale, slotPtr, false);
        if (g_probe) { int q[2][4] = {}; for (int k = 0; k < 2; ++k) for (int i = 0; i < 4; ++i) readAt(panel + (k ? Menu_Char3Offset : Menu_Char2Offset), 4 + 4 * i, &q[k][i]);
            static int lastQ = -1; int sq = q[0][0] * 3 ^ q[1][0] * 7 ^ q[0][1] ^ q[1][2];
            static int pc = 0; if (sq != lastQ || (++pc % 120) == 0) { lastQ = sq; probeLog("MENUPRE c2=(%d,%d,%d,%d) c3=(%d,%d,%d,%d) panel=%08x", q[0][0], q[0][1], q[0][2], q[0][3], q[1][0], q[1][1], q[1][2], q[1][3], panel); } }
        { int w2 = 0; if (readAt(panel + Menu_Char2Offset, 0xc, &w2) && w2 >= 8 && w2 <= 2000) spreadCompanions(panel, w2); }
        for (int k = 0; k < 2; ++k) {
            int ctrl = panel + (k ? Menu_Char3Offset : Menu_Char2Offset), fl = 0, r[4] = {};
            if (!comp[k] || !readAt(ctrl, 0x48, &fl) || !(fl & 2)) continue;
            for (int q = 0; q < 4; ++q) readAt(ctrl, 4 + 4 * q, &r[q]);
            if (r[2] < 1 || r[3] < 1 || r[2] > 2000 || r[3] > 2000) continue;
            { // bars around the portrait (HUD small-card geometry scaled to this card), shield beside the health bar
              int hp = 0, hpMax = 0, fp = 0, fpMax = 0;
              if (readVitals(comp[k], &hp, &hpMax, &fp, &fpMax)) {
                  float sf = (r[3] + 4) / 96.0f;                       // bars about as tall as the portrait (visible art = 97% of the draw height)
                  int bw = int(47 * sf + 0.5f), bh = int(96 * sf + 0.5f), by = r[1] - 2;
                  // art columns: health 57..97 of 128, force 29..69: put the visible health edge flush against the portrait's left edge and the
                  // visible force edge flush against its right edge (the 40/77 HUD constants left a ~5 px gap on the 96 px menu cards)
                  int vx = r[0] - int(97.0f / 128.0f * bw + 0.5f), bwF = int(bw * 1.2f + 0.5f), fx = r[0] + r[2] - int(29.0f / 128.0f * bwF + 0.5f);   // force bar 20% wider (read skinny next to health)
                  drawGauge(HealthBarTexture, vx, by, bw, bh, HealthFillRed, hp, hpMax);
                  drawGauge(ForceBarTexture, fx, by, bwF, bh, ForceFillCyan, fp, fpMax);
                  DrawShieldPools(comp[k], vx, by, bw, bh, scale, ctrl, false);
                  static int lastV[2] = { -1, -1 }; int vs = hp * 7 + hpMax * 13 + fp * 17 + fpMax * 19;
                  if (lastV[k] != vs) { lastV[k] = vs; probeLog("MENUCOMP k=%d creature=%08x hp=%d/%d fp=%d/%d card=(%d,%d,%d,%d) vit=(%d,%d,%d,%d)", k, comp[k], hp, hpMax, fp, fpMax, r[0], r[1], r[2], r[3], vx, by, bw, bh); }
              }
            }
            if (g_noBuff) continue;
            if (pushClip(r[0], r[1], r[2], r[3], ClipNoFillColor, 0, 1.0f)) {
                drawColumnCore(ctrl, comp[k], 0, 0, 1, r[2], r[3]);     // buffs: left edge
                drawColumnCore(ctrl, comp[k], 0, 0, 0, r[2], r[3]);     // debuffs: right edge
                popClip(); ++cols;
            }
        }
        popClip();
    }
    reinterpret_cast<void(__cdecl*)()>(EndGui2D)();
    static int lastShown = -2, lastCols = -2;
    if (shown != lastShown || cols != lastCols) { lastShown = shown; lastCols = cols; probeLog("MENUDRAW panel=(%d,%d,%d,%d) bar=%d companionPortraits=%d", pr[0], pr[1], pr[2], pr[3], shown, cols); }
}

// Probe draw site: FUN_00745770 (per-slot icon-row gate) after its prologue, ECX = slotPtr,
// [ebp+0xc] = isFirst (leader). Runs once per slot per frame, in full-HUD coordinates,
// after the panel controls drew and independent of the engine's arrow counts.
void __cdecl DrawShieldBar(int slotPtr, int isLarge, int callerEbp) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
    { int cd = 0; readAt(0x00a309c0, 0, &cd);                       // menu frame: map the menu portrait slot to its creature so the Buff HUD
      if ((cd & 0xffff) != 0 && isLarge) {                             // hooks (which run later in 0x745770) and the arrow-counter bump know it
          int c = 0; readAt(callerEbp, -0x38, &c);
          int cnt = 0, arr = 0;
          bool ok = c && readAt(c, Creature_EffectCountOffset, &cnt) && readAt(c, Creature_EffectArrayOffset, &arr) && cnt >= 0 && cnt <= MaxEffects;
          g_menuSlot = ok ? SlotEntry{ slotPtr, c, GetTickCount() } : SlotEntry{ 0, 0, 0 };
      } else if (g_menuSlot.slotPtr == slotPtr) g_menuSlot = { 0, 0, 0 }; }
    ForceArrowCounters(slotPtr);                // before FUN_00745770 reads +0xddc/+0xddd; must run for every slot, not just the leader
    swapHealthBar(slotPtr, false);              // engine has drawn: put the health bar's own texture back
    if (isLarge) {
        ++g_frame;
        if (g_frame % 30 == 0) {
            g_noDraw = fileExists("shield_nodraw.txt"); g_noSwap = fileExists("shield_noswap.txt"); g_noBuff = fileExists("buffhud_off.txt");
            static int lastFlags = -1; int fl = g_noDraw | (g_noSwap << 1) | (g_noBuff << 2);
            if (fl != lastFlags) { lastFlags = fl; probeLog("FLAGS nodraw=%d noswap=%d nobuff=%d", g_noDraw, g_noSwap, g_noBuff); }
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
    int creature = creatureForSlot(slotPtr);
    int flags = 0, vitVt = 0, forceVt = 0, vit[4] = {}, force[4] = {}, card[4] = {};
    readAt(slotPtr, 0x590, &flags); readAt(slotPtr, 0x690, &vitVt); readAt(slotPtr, 0x7fc, &forceVt);
    for (int k = 0; k < 4; ++k) {
        readAt(slotPtr, 0x694 + 4 * k, &vit[k]); readAt(slotPtr, 0x800 + 4 * k, &force[k]); readAt(slotPtr, 0x54c + 4 * k, &card[k]);
    }
    if (isLarge && !g_noDraw && force[2] >= 8 && force[2] <= 2000 && card[2] >= 8) {
        // leader force bar: its art leaves a few px between the portrait and the visible bar (health is flush): slide it left by that gap
        // (art columns 29..69 of 128 => visible left edge = x + 0.227*w). Engine SetRect = progress-bar vtable slot 1 (0x41b9e0, RET 4), once.
        int gap = int(force[0] + 29.0f / 128.0f * force[2] + 0.5f) - (card[0] + card[2]);
        if (gap > 0 && gap < 12) {
            int vt = 0, fn = 0, fo = slotPtr + 0x7fc;
            if (readAt(fo, 0, &vt) && vt && readAt(vt, 4, &fn) && fn) {
                int nr[4] = { force[0] - gap, force[1], force[2], force[3] };
                reinterpret_cast<void(__thiscall*)(int, int*)>(fn)(fo, nr);
                probeLog("FORCEMOVE slot=%08x x %d -> %d (gap %d)", slotPtr, force[0], nr[0], gap);
                force[0] = nr[0];
            }
        }
    }
    float scale = 1.0f;
    { float s = reinterpret_cast<float(__cdecl*)()>(HudScaleFn)(); if (s > 0.1f && s < 10.0f) scale = s; }
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
    // All shield bars are drawn from the LEADER's call (the first slot call of the frame). Drawing them per slot failed on
    // the later slots whenever an earlier slot had already drawn its Buff HUD icon row (leaked GUI clip/batch state): the
    // leader and, at most, one small card showed a bar (session 4c logs). At the leader's call the state is still clean.
    if (!isLarge) return;
    // Menu frames (settings etc.): the engine's GUI clip stack reads 0xFFFF and the 3D viewport is 0x0 (probe 02:34 logs). The HUD
    // hook still runs there; drawing then produced a black bar. Only draw in the normal HUD state (clip stack empty).
    { int cd = 0; readAt(0x00a309c0, 0, &cd); bool menu = (cd & 0xffff) != 0;
      static bool lastMenu = false;
      if (menu != lastMenu) { lastMenu = menu; probeLog("MENUSTATE skip=%d clipDepth=%x", menu, cd); }
      if (menu) { if (!g_noDraw) DrawMenuOverlay(slotPtr, callerEbp, scale); return; } }
    if (g_noDraw) return;
    CapGuard capGuard;
    static bool prevAny = false; static int dumpFrames = 0;
    bool any = false;
    for (auto& e : g_slots) { Pool t[8]; if (e.slotPtr && e.creaturePtr && slotFresh(e) && collectPools(e.creaturePtr, t, 8) > 0) any = true; }
    if (any != prevAny) { prevAny = any; dumpFrames = 4; probeLog("SHIELD_ANY %d", any); }
    if (dumpFrames > 0) glDump("PRE");
    struct PostDump { int* d; ~PostDump() { if (*d > 0) { glDump("POST"); --*d; } } } postDump{ &dumpFrames };
    for (auto& e : g_slots) {
        if (!e.slotPtr || !e.creaturePtr || !slotFresh(e)) continue;
        int v[4] = {};
        for (int k = 0; k < 4; ++k) readAt(e.slotPtr, 0x694 + 4 * k, &v[k]);
        if (v[2] < 1 || v[2] > 400 || v[3] < 1 || v[3] > 400) continue;
        int n = DrawShieldPools(e.creaturePtr, v[0], v[1], v[2], v[3], scale, e.slotPtr);
        int effCount = 0; readAt(e.creaturePtr, Creature_EffectCountOffset, &effCount);
        unsigned char arrowBuff = 0, arrowDebuff = 0; readAt(e.slotPtr, 0xddc, &arrowBuff); readAt(e.slotPtr, 0xddd, &arrowDebuff);
        static int lastDraw[8]; static int drawSlot[8]; static int nDraw = 0;
        int di = -1; for (int k = 0; k < nDraw; ++k) if (drawSlot[k] == e.slotPtr) di = k;
        if (di < 0 && nDraw < 8) { di = nDraw++; drawSlot[di] = e.slotPtr; lastDraw[di] = -12345; }
        ArrowRaw* rw = arrowRawFor(e.slotPtr);
        int rawB = rw ? rw->buff : 0, rawD = rw ? rw->debuff : 0, trB = rw ? rw->nBuff : 0, trD = rw ? rw->nDebuff : 0;
        int dsig = e.creaturePtr ^ (n << 24) ^ (effCount << 16) ^ (arrowBuff << 8) ^ (arrowDebuff << 4) ^ (rawB << 2) ^ (rawD << 1) ^ (trB << 12) ^ (trD << 10);
        if (di >= 0 && lastDraw[di] != dsig) {
            lastDraw[di] = dsig;
            probeLog("DRAW slot=%08x creature=%08x pools=%d effects=%d engineArrows buff=%d debuff=%d raw=(%d,%d) tracked=(%d,%d) (drawn from leader call)", e.slotPtr, e.creaturePtr, n, effCount, arrowBuff, arrowDebuff, rawB, rawD, trB, trD);
        }
    }
}

// Portrait hover tooltip (FUN_00744c20 builds the text into the CExoString at charButton+0x1d4, once per hover / refresh flag).
// Hooked at its RET path 0x00744EEC; [ebp-0x48] = charButton, *(charButton+0x1d0) = the portrait slot pointer. The engine's
// cdecl "sprintf into CExoString" (FUN_00734270) formats into a scratch buffer before touching the destination, so the old
// text can be passed back in as an argument. The tooltip panel sizes itself from the text (FUN_0075ff50).
void __cdecl AppendShieldToTooltip(int charButton) {
    saferead::beginScope();                     // engine code between hook calls can free memory: drop the cached readable regions
    int slot = 0, oldText = 0;
    if (!readAt(charButton, 0x1d0, &slot) || !slot || !readAt(charButton, 0x1d4, &oldText) || !oldText) return;
    // Defensive: the builder re-formats the text before this RET path, so the line is normally absent here; never append twice.
    if (strstr(reinterpret_cast<const char*>(oldText), "Shield:")) return;
    int creature = creatureForSlot(slot);
    char line[96];
    int n = creature ? describePools(creature, line, sizeof(line)) : 0;
    probeLog("TIP btn=%08x slot=%08x creature=%08x pools=%d line='%s'", charButton, slot, creature, n, line);
    if (n <= 0) return;
    using CExoSprintf = void(__cdecl*)(void*, const char*, ...);
    reinterpret_cast<CExoSprintf>(CExoStringSprintf)(reinterpret_cast<void*>(charButton + 0x1d4), "%s\n%s", reinterpret_cast<const char*>(oldText), line);
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_debugLog = GetFileAttributesA("buffhud_debug.txt") != INVALID_FILE_ATTRIBUTES;
        g_probe = GetFileAttributesA("shieldprobe.txt") != INVALID_FILE_ATTRIBUTES;
        saferead::enableRing(g_probe);
        logf("buffhud loaded");
    }
    return TRUE;
}
