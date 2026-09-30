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
struct SlotEntry { int slotPtr; int creaturePtr; };
SlotEntry g_slots[4];

int creatureForSlot(int slotPtr) {
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) return e.creaturePtr;
    return 0;
}

// ---- helpers -------------------------------------------------------------------------
template <typename T> bool readAt(int base, int off, T* out) {
    if (base == 0 || IsBadReadPtr(reinterpret_cast<const void*>(base + off), sizeof(T))) return false;
    *out = *reinterpret_cast<const T*>(base + off);
    return true;
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
constexpr int MaxSegments = 3;
constexpr float ShieldWidthFraction = 0.55f;          // of the health bar rect width
constexpr float ShieldLighten = 0.40f;                // mix toward white so a red shield reads differently from the red health bar
const float DividerTint[3] = { 0.93f, 0.97f, 1.0f };   // bright seam between shield and health
const char* const ShieldBarTexture = "uibit_bar_sh_p";     // curved outer-left, outline-free right edge (scripts/gen_shield_bar_art.py)
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

int DrawShieldPools(int creature, int vx, int vy, int vw, int vh, float scale, int slotPtr = 0) {
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
    int W = int(ShieldWidthFraction * vw + 0.5f), H = vh;
    int seamX = vx + int(0.445f * vw + 0.5f);                  // visible left edge of the (squared) health bar
    int x = seamX - int(0.773f * W + 0.5f), y = vy;
    long total = 0; for (int i = 0; i < n; ++i) total += pools[i].max;
    if (total < 1) return 0;
    float flushArg = 0.0f; readAt(0x009f5f60, 0, &flushArg);
    auto flush = reinterpret_cast<void(__cdecl*)(float)>(FlushGuiBatch);
    auto pushClip = reinterpret_cast<int(__cdecl*)(int, int, int, int, const void*, int, float)>(PushClipRect);
    auto popClip = reinterpret_cast<void(__cdecl*)()>(PopClipRect);
    auto draw = reinterpret_cast<void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float)>(
        (*reinterpret_cast<void***>(image))[6]);
    flush(flushArg);
    draw(image, x, y, W, H, 0, 0, TrackTint, 1.0f);            // empty track, full rect
    int baseUp = 0;                                            // px of bar height already used, from the bottom
    for (int i = 0; i < n; ++i) {
        int segH = int(double(H) * pools[i].max / total + 0.5);
        int fillH = int(double(H) * pools[i].remaining / total + 0.5);
        if (fillH > segH) fillH = segH;
        if (fillH > 0) {
            int y0 = y + H - baseUp - fillH;
            int pushed = pushClip(x, y0, W, fillH, ClipNoFillColor, 0, 1.0f);
            { static int lastPush[8]; static int pushSlot[8]; static int nPush = 0;
              int pi = -1; for (int k = 0; k < nPush; ++k) if (pushSlot[k] == slotPtr) pi = k;
              if (pi < 0 && nPush < 8) { pi = nPush++; pushSlot[pi] = slotPtr; lastPush[pi] = -1; }
              int psig = pushed ? 1 : 0;
              if (pi >= 0 && lastPush[pi] != psig) { lastPush[pi] = psig; probeLog("FILL slot=%08x seg=%d pushClip=%d rect=(%d,%d,%d,%d) vit=(%d,%d,%d,%d) n=%d", slotPtr, i, pushed, x, y0, W, fillH, vx, vy, vw, vh, n); } }
            if (pushed) {
                float lit[3];
                for (int c = 0; c < 3; ++c) lit[c] = pools[i].rgb[c] + (1.0f - pools[i].rgb[c]) * ShieldLighten;
                draw(image, 0, -(y0 - y), W, H, 0, 0, lit, 1.0f);
                popClip();
            }
        }
        baseUp += segH;
    }
    int dw = vw / 30 < 2 ? 2 : vw / 30;                        // divider: 3 px on the leader card, 2 px on small cards
    int dh = int(H * 0.94f);
    if (pushClip(seamX - dw / 2, y + (H - dh) / 2, dw, dh, DividerTint, 0, 0.9f)) popClip();
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
    g_gl.ok = g_gl.getError && g_gl.getIntegerv && g_gl.getBooleanv && g_gl.isEnabled;
    probeLog("GLINIT ok=%d readPixels=%d", g_gl.ok, g_gl.readPixels != nullptr);
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
} // namespace

extern "C" {

void __cdecl SnapshotPortraitSlot(int slotPtr, int creaturePtr) {
    static int n = 0; if (n < 60) { ++n; logf("snapshot slot=%08x creature=%08x", slotPtr, creaturePtr); }
    if (g_probe && creaturePtr) probeEffects(slotPtr, creaturePtr);
    if (creaturePtr) { Pool tmp[8]; swapHealthBar(slotPtr, !g_noSwap && collectPools(creaturePtr, tmp, 8) > 0); }
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) { e.creaturePtr = creaturePtr; return; }
    for (auto& e : g_slots) if (e.slotPtr == 0) { e = { slotPtr, creaturePtr }; return; }
    static int evict = 0; g_slots[evict++ & 3] = { slotPtr, creaturePtr };   // HUD rebuilt (area load): replace a stale entry
}

// Hook 2 (per native arrow, buff row): the icon column is drawn by DrawBuffColumn, so hide
// the first `tracked` native arrows (CSWGuiBorder::Draw returns at once when +0xc/+0x10 == 0).
void __cdecl RetargetBuffIcon(int slotPtr, int index, int iconWidget, int whichRow) {
    g_saved.active = false;
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
    if (g_saved.active && g_saved.border == border) { w = g_saved.width; h = g_saved.height; }  // debuff row has no restore hook
    g_saved = { true, border, 0, w, h };
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
void __cdecl DrawBuffColumn(int slotPtr, int iconWidget, int isLarge, int whichRow, int cardW, int cardH) {
    if (g_noBuff) return;
    RestoreBuffIcon();                          // debuff arrows are hidden without a restore hook
    if (iconWidget == 0) return;
    const bool debuff = whichRow == 0;
    static int callNo = 0; ++callNo;
    int creature = creatureForSlot(slotPtr);
    if (!creature) return;
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

    int border = iconWidget + Label_TextBorderOffset;
    int rotBits = 0; readAt(border, 0x20, &rotBits);
    const float* color = reinterpret_cast<const float*>(border + 0x28);
    float baseAlpha = 1.0f; readAt(border, 0x24, &baseAlpha);
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
                if (bright < size) {                                                             // 1px boundary line
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

// Probe draw site: FUN_00745770 (per-slot icon-row gate) after its prologue, ECX = slotPtr,
// [ebp+0xc] = isFirst (leader). Runs once per slot per frame, in full-HUD coordinates,
// after the panel controls drew and independent of the engine's arrow counts.
void __cdecl ShieldProbeSlot(int slotPtr, int isLarge) {
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
        static bool f9Prev = false; bool f9 = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;
        if (f9 && !f9Prev) { static int mark = 0; probeLog("MARK %d scene=%d", ++mark, g_probe ? sceneBrightness() : -2); glDump("MARK"); }
        f9Prev = f9;
    }
    int creature = creatureForSlot(slotPtr);
    int flags = 0, vitVt = 0, forceVt = 0, vit[4] = {}, force[4] = {}, card[4] = {};
    readAt(slotPtr, 0x590, &flags); readAt(slotPtr, 0x690, &vitVt); readAt(slotPtr, 0x7fc, &forceVt);
    for (int k = 0; k < 4; ++k) {
        readAt(slotPtr, 0x694 + 4 * k, &vit[k]); readAt(slotPtr, 0x800 + 4 * k, &force[k]); readAt(slotPtr, 0x54c + 4 * k, &card[k]);
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
      if (menu) return; }
    if (g_noDraw) return;
    static bool prevAny = false; static int dumpFrames = 0;
    bool any = false;
    for (auto& e : g_slots) { Pool t[8]; if (e.slotPtr && e.creaturePtr && collectPools(e.creaturePtr, t, 8) > 0) any = true; }
    if (any != prevAny) { prevAny = any; dumpFrames = 4; probeLog("SHIELD_ANY %d", any); }
    if (dumpFrames > 0) glDump("PRE");
    struct PostDump { int* d; ~PostDump() { if (*d > 0) { glDump("POST"); --*d; } } } postDump{ &dumpFrames };
    for (auto& e : g_slots) {
        if (!e.slotPtr || !e.creaturePtr) continue;
        int v[4] = {};
        for (int k = 0; k < 4; ++k) readAt(e.slotPtr, 0x694 + 4 * k, &v[k]);
        if (v[2] < 1 || v[2] > 400 || v[3] < 1 || v[3] > 400) continue;
        int n = DrawShieldPools(e.creaturePtr, v[0], v[1], v[2], v[3], scale, e.slotPtr);
        int effCount = 0; readAt(e.creaturePtr, Creature_EffectCountOffset, &effCount);
        unsigned char arrowBuff = 0, arrowDebuff = 0; readAt(e.slotPtr, 0xddc, &arrowBuff); readAt(e.slotPtr, 0xddd, &arrowDebuff);
        static int lastDraw[8]; static int drawSlot[8]; static int nDraw = 0;
        int di = -1; for (int k = 0; k < nDraw; ++k) if (drawSlot[k] == e.slotPtr) di = k;
        if (di < 0 && nDraw < 8) { di = nDraw++; drawSlot[di] = e.slotPtr; lastDraw[di] = -12345; }
        int dsig = e.creaturePtr ^ (n << 24) ^ (effCount << 16) ^ (arrowBuff << 8) ^ (arrowDebuff << 4);
        if (di >= 0 && lastDraw[di] != dsig) {
            lastDraw[di] = dsig;
            probeLog("DRAW slot=%08x creature=%08x pools=%d effects=%d engineArrows buff=%d debuff=%d (drawn from leader call)", e.slotPtr, e.creaturePtr, n, effCount, arrowBuff, arrowDebuff);
        }
    }
}

// Portrait hover tooltip (FUN_00744c20 builds the text into the CExoString at charButton+0x1d4, once per hover / refresh flag).
// Hooked at its RET path 0x00744EEC; [ebp-0x48] = charButton, *(charButton+0x1d0) = the portrait slot pointer. The engine's
// cdecl "sprintf into CExoString" (FUN_00734270) formats into a scratch buffer before touching the destination, so the old
// text can be passed back in as an argument. The tooltip panel sizes itself from the text (FUN_0075ff50).
void __cdecl AppendShieldToTooltip(int charButton) {
    int slot = 0, oldText = 0;
    if (!readAt(charButton, 0x1d0, &slot) || !slot || !readAt(charButton, 0x1d4, &oldText) || !oldText) return;
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
        logf("buffhud loaded");
    }
    return TRUE;
}
