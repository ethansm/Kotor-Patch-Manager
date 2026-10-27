// CombatStatsAddr.h - engine read paths for the combat-stats page (doc 51 ST1).
// Mod-local copy: do not edit _shared_src/GameAddr.h; promote later.
// Column: Steam (Aspyr swkotor2.exe). All 8 functions verified in the Steam DB
// (doc 51 ST1, patch_manager_mods/51_combat_stats_page.md).
// Do NOT use GetAttacksPerRound 0x006A7170: stub returning 1.
// Constants and function-pointer typedefs only: no calls, no hooks, no windows.h.
#pragma once
#include <stdint.h>

#if defined(_MSC_VER) || defined(__i386__)
#define CSTATS_THISCALL __thiscall
#define CSTATS_FASTCALL __fastcall
#else
#define CSTATS_THISCALL
#define CSTATS_FASTCALL
#endif

namespace cstats_addr {

// Engine functions (Steam)
constexpr uintptr_t HasFeat              = 0x006B83F0;  // 0x006B83F0 thiscall(stats, uint16 featId) -> int, RET 4
constexpr uintptr_t HasSpell             = 0x006BD9C0;  // 0x006BD9C0 thiscall(stats, uint8 list=0, uint32 spellId, int check=0) -> int, RET 12
constexpr uintptr_t GetBaseAttackBonus   = 0x006A6F80;  // 0x006A6F80 fastcall(stats) -> uint32
constexpr uintptr_t IsWieldingLightsaber = 0x005763E0;  // 0x005763E0 fastcall(serverCreature) -> int (reads creature+0x1150, slot 0x10)
constexpr uintptr_t GetLevel             = 0x006A4E00;  // 0x006A4E00 thiscall(stats, int flag) -> uint8, RET 4 (Force DC uses flag 1)
constexpr uintptr_t GetDEXMod            = 0x006B5DB0;  // 0x006B5DB0 thiscall(stats, int flag) -> int, RET 4 (Defense adds flag 1)
constexpr uintptr_t GetArmorClass        = 0x0057E530;  // 0x0057E530 fastcall(serverCreature) -> int16 (Defense total, non-versus)
constexpr uintptr_t ForceResist          = 0x006BAD10;  // 0x006BAD10 fastcall(stats) -> int (max(0, stats+0x12B - stats+0x12C))

// Offsets (Creature_* from creature object, Stats_* from stats object)
constexpr uint32_t Creature_Stats        = 0x1198;  // creature -> stats object (the 'stats' arg of the functions above)
constexpr uint32_t Creature_Form         = 0x1110;  // int, spells.2da row 258..268, 0 = none
constexpr uint32_t Creature_StanceFlag   = 0x110C;  // byte: 0 Total Defense, 1 Meditative, 0xFF none
constexpr uint32_t Creature_Effects      = 0x148;   // effect array pointer
constexpr uint32_t Creature_EffectCount  = 0x14C;   // effect count
constexpr uint32_t Stats_GoodEvil        = 0x18A;   // int16 0..100
constexpr uint32_t Stats_WisMod          = 0xF6;    // stats offset
constexpr uint32_t Stats_ChaMod          = 0xF8;    // stats offset

// Effect type globals: read the int stored at this address at runtime, it is the effect type id
constexpr uintptr_t EffDeflectInc = 0x00996088;  // read the int stored at this address at runtime, it is the effect type id
constexpr uintptr_t EffDeflectDec = 0x0099608C;  // read the int stored at this address at runtime, it is the effect type id

// Function-pointer typedefs (thiscall: first arg is ECX; fastcall: first arg is ECX, takes one pointer here)
typedef int      (CSTATS_THISCALL* HasFeatFn)(void* stats, uint16_t featId);
typedef int      (CSTATS_THISCALL* HasSpellFn)(void* stats, uint8_t list, uint32_t spellId, int check);
typedef uint32_t (CSTATS_FASTCALL* GetBaseAttackBonusFn)(void* stats);
typedef int      (CSTATS_FASTCALL* IsWieldingLightsaberFn)(void* serverCreature);
typedef uint8_t  (CSTATS_THISCALL* GetLevelFn)(void* stats, int flag);
typedef int      (CSTATS_THISCALL* GetDEXModFn)(void* stats, int flag);
typedef int16_t  (CSTATS_FASTCALL* GetArmorClassFn)(void* serverCreature);
typedef int      (CSTATS_FASTCALL* ForceResistFn)(void* stats);

}  // namespace cstats_addr
