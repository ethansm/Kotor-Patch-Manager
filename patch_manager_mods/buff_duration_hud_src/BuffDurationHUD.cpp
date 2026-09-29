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
// The icon widget is a CSWGuiLabel; its "TEXT" child (CSWGuiBorder) draws the glyph
constexpr int Label_TextBorderOffset = 0xd8;
constexpr int Border_WidthOffset = 0xc;
constexpr int Border_HeightOffset = 0x10;
constexpr int Border_ResourceBlockOffset = 0x54;  // *(border+0x54) = block of bound resources
constexpr int Block_FillImageOffset = 0x74;       // bound FILL image object

constexpr int WorldTimerSingletonGlobal = 0x00a1b4a4;
constexpr int WorldTimerGetCurrentTime = 0x0051ad20; // thiscall(timer, int* day, uint* ms), RET 8
constexpr int LoadFillResource = 0x0047eb60;         // cdecl(resref buf) -> CAurGUIImageInternal*
constexpr int MsPerDay = 86400000;
constexpr int MaxEffects = 512;
constexpr int MaxPoolImages = 128;
constexpr float MinWipeRatio = 0.15f;

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
struct Saved { bool active; int border; int block; int origImage; int width; int height; } g_saved;

} // namespace

extern "C" {

void __cdecl SnapshotPortraitSlot(int slotPtr, int creaturePtr) {
    for (auto& e : g_slots) if (e.slotPtr == slotPtr) { e.creaturePtr = creaturePtr; return; }
    for (auto& e : g_slots) if (e.slotPtr == 0) { e = { slotPtr, creaturePtr }; return; }
}

void __cdecl RetargetBuffIcon(int slotPtr, int index, int iconWidget, int whichRow) {
    g_saved.active = false;
    if (whichRow == 0 || iconWidget == 0 || index < 0) return;
    int creature = creatureForSlot(slotPtr);
    if (!creature) return;
    int day = 0, ms = 0;
    bool haveTime = currentTime(&day, &ms);
    Buff buffs[16];
    int n = collectBuffs(creature, day, ms, haveTime, buffs, 16);
    if (index >= n) return;                     // leave the engine glyph for extra arrows

    int border = iconWidget + Label_TextBorderOffset;
    int block, orig, w, h;
    if (!readAt(border, Border_ResourceBlockOffset, &block) || block == 0 ||
        !readAt(block, Block_FillImageOffset, &orig) ||
        !readAt(border, Border_WidthOffset, &w) || !readAt(border, Border_HeightOffset, &h)) return;
    void* image = imageFor(buffs[index].resref);
    if (!image) return;

    g_saved = { true, border, block, orig, w, h };
    *reinterpret_cast<int*>(block + Block_FillImageOffset) = reinterpret_cast<int>(image);
    *reinterpret_cast<int*>(border + Border_WidthOffset) = int(w * buffs[index].ratio);
    *reinterpret_cast<int*>(border + Border_HeightOffset) = int(h * buffs[index].ratio);
}

void __cdecl RestoreBuffIcon() {
    if (!g_saved.active) return;
    g_saved.active = false;
    *reinterpret_cast<int*>(g_saved.block + Block_FillImageOffset) = g_saved.origImage;
    *reinterpret_cast<int*>(g_saved.border + Border_WidthOffset) = g_saved.width;
    *reinterpret_cast<int*>(g_saved.border + Border_HeightOffset) = g_saved.height;
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
