// Buff Duration HUD (Steam Aspyr build 6A522E71...). Replaces the engine's plain
// "buff arrow" glyph over each party portrait with the buff's own icon and shrinks it
// as the effect runs out. Research trail and every address below:
// patch_manager_mods/02_buff_duration_hud.md (local-setup/KOTOR-II worktree).
//
// Hooks (all inside CSWGuiMainInterface, cdecl detours, original bytes still run):
//   1a/1b  0x0074f25a / 0x0074f8ab  UpdatePortraits: remember slotPtr -> creature
//   2      0x00745a84  PortraitIconRowDraw, just before the icon widget's Draw():
//                      swap the widget's "TEXT" border FILL image and shrink it
//   3      0x00745a97  right after that Draw() (buff row only): put both back
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
constexpr float MinWipeRatio = 0.15f;

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

void __cdecl RetargetBuffIcon(int slotPtr, int index, int iconWidget, int whichRow) {
    g_saved.active = false;
    if (whichRow == 0 || iconWidget == 0 || index < 0) return;
    int creature = creatureForSlot(slotPtr);
    static int lastSig[8][16]; int si = 0;
    for (int k = 0; k < 4; ++k) if (g_slots[k].slotPtr == slotPtr) si = k;
    int sig = creature ^ (whichRow << 3);
    bool logIt = index < 16 && lastSig[si][index] != sig; if (index < 16) lastSig[si][index] = sig;
    static int callNo = 0; ++callNo;
    if (callNo <= 60) logIt = true;
    if (callNo <= 4) {
        logf("CALL#%d slot=%08x idx=%d widget=%08x whichRow=%d creature=%08x", callNo, slotPtr, index, iconWidget, whichRow, creature);
        int vt = 0; readAt(iconWidget, 0, &vt);
        logf("  widget vtable=%08x (CSWGuiLabel=009878bc)", vt);
        dumpMem("widget", iconWidget, 0x1e0 / 4);
        dumpMem("border@+60", iconWidget + 0x60, 0x78 / 4);
        dumpMem("border@+d8", iconWidget + 0xd8, 0x78 / 4);
    }
    if (!creature) { if (logIt) logf("row slot=%08x idx=%d: NO creature in side table", slotPtr, index); return; }
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16);
    if (logIt) {
        int arr = 0, cnt = 0; readAt(creature, Creature_EffectArrayOffset, &arr); readAt(creature, Creature_EffectCountOffset, &cnt);
        char ids[400] = {}; int p = 0;
        for (int i = 0; i < cnt && i < 60 && p < 380; ++i) { int e = 0, sp = -2; readAt(arr, i * 4, &e); readAt(e, Effect_SpellIdOffset, &sp); p += snprintf(ids + p, 400 - p, "%d ", sp); }
        logf("row slot=%08x idx=%d creature=%08x effects=%d tracked=%d time=%d(%d,%d) spellIds: %s", slotPtr, index, creature, cnt, n, haveTime, day, ms, ids);
    }
    if (logIt) for (int k = 0; k < n; ++k) logf("  buff[%d] resref=%s ratio=%.3f", k, buffs[k].resref, buffs[k].ratio);
    if (index >= n) return;                     // leave the engine glyph for extra arrows

    int border = iconWidget + Label_TextBorderOffset;
    int orig = 0, w = 0, h = 0, bvt = 0;
    bool okBvt = readAt(border, 0, &bvt);
    bool okOrig = readAt(border, Border_FillImageOffset, &orig);
    bool okW = readAt(border, Border_WidthOffset, &w), okH = readAt(border, Border_HeightOffset, &h);
    if (logIt) {
        logf("  reads widget=%08x border=%08x vt=%d(%08x, want 009875bc) fill=%d(%08x) w=%d(%d) h=%d(%d)", iconWidget, border, okBvt, bvt, okOrig, orig, okW, w, okH, h);
        int ivt = 0; readAt(orig, 0, &ivt);
        logf("  orig fill image vtable=%08x (CAurGUIImageInternal=0098c1fc)", ivt);
        dumpMem("orig fill image", orig, 16);
    }
    if (!okBvt || bvt != 0x009875bc || !okOrig || orig == 0 || !okW || !okH) return;
    void* image = imageFor(buffs[index].resref);
    if (logIt) { int nvt = 0; readAt(reinterpret_cast<int>(image), 0, &nvt);
        logf("  idx=%d resref=%s ratio=%.2f newimage=%p (vtable %08x) border=%08x orig=%08x w=%d h=%d", index, buffs[index].resref, buffs[index].ratio, image, nvt, border, orig, w, h); }
    if (!image) return;

    g_saved = { true, border, orig, w, h };
    *reinterpret_cast<int*>(border + Border_FillImageOffset) = reinterpret_cast<int>(image);
    *reinterpret_cast<int*>(border + Border_WidthOffset) = int(w * buffs[index].ratio);
    *reinterpret_cast<int*>(border + Border_HeightOffset) = int(h * buffs[index].ratio);
}

void __cdecl RestoreBuffIcon() {
    static int rn = 0; if (rn < 40) { ++rn; logf("RESTORE#%d active=%d", rn, g_saved.active); }
    if (!g_saved.active) return;
    g_saved.active = false;
    *reinterpret_cast<int*>(g_saved.border + Border_FillImageOffset) = g_saved.origImage;
    *reinterpret_cast<int*>(g_saved.border + Border_WidthOffset) = g_saved.width;
    *reinterpret_cast<int*>(g_saved.border + Border_HeightOffset) = g_saved.height;
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) logf("buffhud loaded");
    return TRUE;
}
