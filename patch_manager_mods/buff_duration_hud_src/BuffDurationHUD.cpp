// Buff Duration HUD (Steam Aspyr build 6A522E71...). Replaces the engine's plain
// "buff arrow" glyph over each party portrait with the buff's own icon and shrinks it
// as the effect runs out. Research trail and every address below:
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

#include "EffectIconTable.inc"

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
constexpr int HudScaleFn = 0x00479ab0;        // cdecl, returns float in st0
constexpr int FlushGuiBatch = 0x0047ea60;     // cdecl(float), flushes batched GUI quads
constexpr int PushClipRect = 0x00479460;      // cdecl(x,y,w,h,color*,flag,float alpha) -> nonzero if pushed
constexpr int PopClipRect = 0x004799c0;
const void* const ClipNoFillColor = reinterpret_cast<const void*>(0x00a1ba3c); // what the row itself passes
constexpr float ColumnWidthFraction = 0.25f;  // icon size as a fraction of card width (tuned live)
constexpr float MinIconSize = 12.0f, MaxIconSize = 36.0f;
constexpr float DimAlpha = 0.35f;

// ---- diagnostics: buffhud_log.txt in the game dir, capped, off the hot path when full ----
int g_logLines = 0;
constexpr int MaxLogLines = 6000;
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    FILE* f = fopen("buffhud_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

// hex dump of `words` 32-bit words at addr (readable-checked), 8 words per line
void dumpMem(const char* tag, int addr, int words) {
    if (addr == 0) { logf("  dump %s @0: null", tag); return; }
    for (int i = 0; i < words; i += 8) {
        char line[200]; int p = 0;
        for (int j = i; j < i + 8 && j < words; ++j) {
            int v;
            if (IsBadReadPtr(reinterpret_cast<const void*>(addr + j * 4), 4)) { p += snprintf(line + p, sizeof(line) - p, " ????????"); }
            else { v = *reinterpret_cast<const int*>(addr + j * 4); p += snprintf(line + p, sizeof(line) - p, " %08x", v); }
        }
        logf("  dump %s +%03x:%s", tag, i * 4, line);
    }
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

struct Buff { const char* resref; float ratio; };

// distinct tracked spell ids on the creature, in application order
int collectBuffs(int creature, int day, int ms, bool haveTime, Buff* out, int maxOut) {
    int arr = 0, count = 0;
    if (!readAt(creature, Creature_EffectArrayOffset, &arr) || !readAt(creature, Creature_EffectCountOffset, &count)) return 0;
    if (arr == 0 || count <= 0 || count > MaxEffects) return 0;
    int seen[MaxEffects]; int nSeen = 0; int n = 0;
    for (int i = 0; i < count && n < maxOut; ++i) {
        int eff, spell;
        if (!readAt(arr, i * 4, &eff) || !readAt(eff, Effect_SpellIdOffset, &spell)) continue;
        if (spell < 0 || spell >= kEffectIconTableSize || !kEffectIcons[spell]) continue;
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
        out[n++] = { kEffectIcons[spell], ratio };
    }
    return n;
}

// state saved by hook 2 for hook 3
struct Saved { bool active; int border; int origImage; int width; int height; } g_saved;

} // namespace

extern "C" {

void __cdecl SnapshotPortraitSlot(int slotPtr, int creaturePtr) {
    static int n = 0; if (n < 60) { ++n; logf("snapshot slot=%08x creature=%08x", slotPtr, creaturePtr); }
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) { e.creaturePtr = creaturePtr; return; }
    for (auto& e : g_slots) if (e.slotPtr == 0) { e = { slotPtr, creaturePtr }; return; }
}

// Hook 2 (per native arrow, buff row): the icon column is drawn by DrawBuffColumn, so hide
// the first `tracked` native arrows (CSWGuiBorder::Draw returns at once when +0xc/+0x10 == 0).
void __cdecl RetargetBuffIcon(int slotPtr, int index, int iconWidget, int whichRow) {
    g_saved.active = false;
    if (whichRow == 0 || iconWidget == 0 || index < 0) return;
    int creature = creatureForSlot(slotPtr);
    static int callNo = 0; ++callNo;
    bool logIt = callNo <= 40;
    if (!creature) { if (logIt) logf("arrow slot=%08x idx=%d: NO creature in side table", slotPtr, index); return; }
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16);
    if (logIt) logf("arrow slot=%08x idx=%d tracked=%d -> %s", slotPtr, index, n, index < n ? "suppress" : "keep");
    if (index >= n) return;                     // untracked engine buff: leave its arrow

    int border = iconWidget + Label_TextBorderOffset;
    int w = 0, h = 0, bvt = 0;
    if (!readAt(border, 0, &bvt) || bvt != 0x009875bc || !readAt(border, Border_WidthOffset, &w) || !readAt(border, Border_HeightOffset, &h)) return;
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
    if (whichRow == 0 || iconWidget == 0) return;
    static int callNo = 0; ++callNo;
    int creature = creatureForSlot(slotPtr);
    if (!creature) return;
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16);

    float scale = 1.0f;
    { float s = reinterpret_cast<float(__cdecl*)()>(HudScaleFn)(); if (s > 0.1f && s < 10.0f) scale = s; }
    int size = int(cardW * ColumnWidthFraction);
    int minSize = int(MinIconSize * scale), maxSize = int(MaxIconSize * scale);
    if (size < minSize) size = minSize;
    if (size > maxSize) size = maxSize;
    int gap = int(scale + 0.5f); if (gap < 1) gap = 1;
    int fit = (cardH - gap) / (size + gap); if (fit < 1) fit = 1;
    int drawN = n < fit ? n : fit;

    static int lastSig[8]; static int sigSlot[8]; static int nSig = 0;
    int si = -1; for (int k = 0; k < nSig; ++k) if (sigSlot[k] == slotPtr) si = k;
    if (si < 0 && nSig < 8) { si = nSig++; sigSlot[si] = slotPtr; lastSig[si] = -1; }
    int sig = (n << 16) ^ (size << 8) ^ (cardW << 20) ^ cardH ^ isLarge;
    bool logIt = callNo <= 30 || (si >= 0 && lastSig[si] != sig);
    if (si >= 0) lastSig[si] = sig;
    if (logIt) logf("COLUMN#%d slot=%08x large=%d card=%dx%d scale=%.3f tracked=%d fit=%d draw=%d size=%d gap=%d",
                    callNo, slotPtr, isLarge, cardW, cardH, scale, n, fit, drawN, size, gap);
    if (drawN <= 0) return;

    int border = iconWidget + Label_TextBorderOffset;
    int rotBits = 0; readAt(border, 0x20, &rotBits);
    const float* color = reinterpret_cast<const float*>(border + 0x28);
    float baseAlpha = 1.0f; readAt(border, 0x24, &baseAlpha);
    float flushArg = 0.0f; readAt(0x009f5f60, 0, &flushArg);
    auto flush = reinterpret_cast<void(__cdecl*)(float)>(FlushGuiBatch);
    auto pushClip = reinterpret_cast<int(__cdecl*)(int, int, int, int, const void*, int, float)>(PushClipRect);
    auto popClip = reinterpret_cast<void(__cdecl*)()>(PopClipRect);

    flush(flushArg);
    for (int i = 0; i < drawN; ++i) {
        void* image = imageFor(buffs[i].resref);
        if (!image) { if (logIt) logf("  icon[%d] %s: no image", i, buffs[i].resref); continue; }
        auto draw = reinterpret_cast<void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float)>(
            (*reinterpret_cast<void***>(image))[6]);
        int ix = gap, iy = gap + i * (size + gap);
        float ratio = buffs[i].ratio;
        int bright = int(size * ratio + 0.5f);
        if (bright > size) bright = size;
        if (logIt) logf("  icon[%d] %s rect=(%d,%d,%d,%d) ratio=%.3f bright=%d dimAlpha=%.2f", i, buffs[i].resref, ix, iy, size, size, ratio, bright, DimAlpha);
        draw(image, ix, iy, size, size, 0, rotBits, color, baseAlpha * DimAlpha);          // elapsed part: dim
        if (bright > 0) {                                                                    // remaining part, bottom-anchored
            int y0 = iy + size - bright;
            int ok = pushClip(ix, y0, size, bright, ClipNoFillColor, 0, 1.0f);
            if (ok) {
                draw(image, 0, -(size - bright), size, size, 0, rotBits, color, baseAlpha);
                popClip();
            } else if (logIt) logf("  icon[%d] clip push refused", i);
        }
    }
    flush(flushArg);
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) logf("buffhud loaded");
    return TRUE;
}
