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
constexpr uintptr_t GetClassLevel        = 0x006AD2F0;  // 0x006AD2F0 thiscall(stats, uint8 classIdx, int flag) -> uint8, RET 8 (CSWSCreatureStats::GetClassLevel; engine passes flag 0)
constexpr uintptr_t EffectGetInteger     = 0x005E4E60;  // 0x005E4E60 thiscall(effect, int idx) -> int, RET 4 (CGameEffect::GetInteger)

// Offsets (Creature_* from creature object, Stats_* from stats object)
constexpr uint32_t Creature_Stats        = 0x1198;  // creature -> stats object (the 'stats' arg of the functions above)
constexpr uint32_t Creature_Form         = 0x1110;  // int, spells.2da row 258..268, 0 = none
constexpr uint32_t Creature_StanceFlag   = 0x110C;  // byte: 0 Total Defense, 1 Meditative, 0xFF none
constexpr uint32_t Creature_Effects      = 0x148;   // effect array pointer
constexpr uint32_t Creature_EffectCount  = 0x14C;   // effect count
constexpr uint32_t Stats_GoodEvil        = 0x18A;   // int16 0..100
constexpr uint32_t Stats_WisMod          = 0xF6;    // stats offset
constexpr uint32_t Stats_ChaMod          = 0xF8;    // stats offset
constexpr uint32_t Stats_ClassCount      = 0x8D;    // byte: number of classes (engine loop reads the first 2)
constexpr uint32_t Stats_ClassId0        = 0xAB;    // byte: class id of slot 0
constexpr uint32_t Stats_ClassStride     = 0x28;    // bytes between class slots
constexpr uint32_t Effect_Type           = 0x8;     // uint16 effect type id (effect object offset)
constexpr uint32_t Stats_Ungated0        = 0x178;   // int8 terms summed into deflection: (int8)(stats+0x179 + stats+0x178)
constexpr uint32_t Stats_Ungated1        = 0x179;

// Combat information block (doc 51 ST5a): stats+0x11C -> pointer, filled by UpdateCombatInformation 0x006A7190.
// Byte +0 is the GetAttacksPerRound stub (always 1): do not use. Attack bytes are int8 (signed).
// NEVER call GetAttackModifierVersus / GetCriticalHitRoll: read these bytes instead.
constexpr uint32_t Stats_CombatInfo      = 0x11C;
constexpr uint32_t CI_AtkMain            = 0x1;     // int8 main-hand attack bonus
constexpr uint32_t CI_AtkOff             = 0x3;     // int8 off-hand attack bonus
constexpr uint32_t CI_CritThreatMain     = 0xA;     // byte
constexpr uint32_t CI_CritMultMain       = 0xB;     // byte
constexpr uint32_t CI_HasOff             = 0xC;     // dword, nonzero when an off-hand weapon is wielded
constexpr uint32_t CI_CritThreatOff      = 0x10;    // byte
constexpr uint32_t CI_CritMultOff        = 0x11;    // byte

// Deflection hook: CSWSCreature::ResolveDefensiveEffects 0x006E1C60, hook site 0x006E2351 (after rec+0x168 = total; ECX = attack record).
// Attack record (rec) offsets. Bytes other than 0x158/0x168 are written only when the term applies (may be stale from an earlier attack).
constexpr uintptr_t DeflectHookSite = 0x006E2351;
constexpr uint32_t Rec_Roll      = 0x158;  // u8 d20
constexpr uint32_t Rec_JDFeat    = 0x160;  // u8
constexpr uint32_t Rec_JD        = 0x161;  // i8
constexpr uint32_t Rec_EffDelta  = 0x162;  // i8
constexpr uint32_t Rec_BAB       = 0x164;  // u8
constexpr uint32_t Rec_Ungated   = 0x165;  // i8
constexpr uint32_t Rec_Total     = 0x168;  // i32
constexpr uint32_t Rec_FR        = 0x16C;  // i8
constexpr uint32_t Rec_Form      = 0x170;  // i32
constexpr uint32_t Rec_PSFeat    = 0x17A;  // u8 0xF0..0xF4 (attacker Precise Shot tier)
constexpr uint32_t Rec_PS        = 0x17B;  // i8 -2..-10
constexpr uint32_t Rec_ThreshA   = 0xC1;   // u8
constexpr uint32_t Rec_ThreshB   = 0xC2;   // i8

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
typedef uint8_t  (CSTATS_THISCALL* GetClassLevelFn)(void* stats, uint8_t classIdx, int flag);
typedef int      (CSTATS_THISCALL* EffectGetIntegerFn)(void* effect, int idx);

}  // namespace cstats_addr
