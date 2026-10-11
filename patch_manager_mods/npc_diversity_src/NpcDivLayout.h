// NpcDivLayout.h -- engine layout constants for the NPC Diversity Dispatcher hooks.
// Confidence tags follow .claude/review/npc_div/A1_engine_hooks.md:
//   VERIFIED   read or written in the disassembly
//   INFERRED   plausible from usage, not proven
//   UNVERIFIED not confirmed; a runtime dump (CRE0 probe) must settle it
#pragma once

#include <cstddef>
#include <cstdint>

namespace NpcDiversity {
namespace layout {

// Engine appearance descriptor (pDesc). Packed so the field offsets match the engine's byte layout.
#pragma pack(push, 1)
struct AppearanceDesc {
    char subType;         // +0x00 letter index source, clamped 1..14 by GetBType (VERIFIED, A1 f5/f6)
    uint8_t textureVar;   // +0x01 texture variation, max(1, byte) (VERIFIED, A1 f6)
    uint8_t pad02[0x12];  // +0x02..+0x13 not decoded
    int32_t descMode14;   // +0x14 zero/nonzero switch between default and EVIL column families, NOT an armor class (VERIFIED, A1 f5); GUI paths write 1
    uint16_t appearance;  // +0x18 2DA row index (VERIFIED, A1 f6)
    char headOverride;    // +0x1A not decoded
    uint8_t gender;       // +0x1B 0=male, 1=female (UNVERIFIED: GetBType does not read it, A1 f6)
    uint8_t phenotype;    // +0x1C not decoded
};
#pragma pack(pop)

constexpr size_t kDescOffTextureVar = 0x01;
constexpr size_t kDescOffMode14 = 0x14;
constexpr size_t kDescOffAppearance = 0x18;
constexpr size_t kDescOffHeadOverride = 0x1A;
constexpr size_t kDescOffGender = 0x1B;
constexpr size_t kDescOffPhenotype = 0x1C;

static_assert(offsetof(AppearanceDesc, textureVar) == kDescOffTextureVar, "AppearanceDesc::textureVar offset");
static_assert(offsetof(AppearanceDesc, descMode14) == kDescOffMode14, "AppearanceDesc::descMode14 offset");
static_assert(offsetof(AppearanceDesc, appearance) == kDescOffAppearance, "AppearanceDesc::appearance offset");
static_assert(offsetof(AppearanceDesc, headOverride) == kDescOffHeadOverride, "AppearanceDesc::headOverride offset");
static_assert(offsetof(AppearanceDesc, gender) == kDescOffGender, "AppearanceDesc::gender offset");
static_assert(offsetof(AppearanceDesc, phenotype) == kDescOffPhenotype, "AppearanceDesc::phenotype offset");
static_assert(sizeof(AppearanceDesc) >= 0x1D, "AppearanceDesc must cover +0x00..+0x1C");

// Creature (CSWCCreature / CSWCObject) offsets, relative to the creature `this`.
constexpr size_t kCreOffObjectId = 0x04;     // INFERRED: compared with the current object id (A1 f11)
constexpr size_t kCreOffTagPtr = 0x18;       // UNVERIFIED: A1 f11 found no tag reference here; CRE0 dump settles it
constexpr size_t kCreOffTagLen = 0x1C;       // UNVERIFIED: same as kCreOffTagPtr
constexpr size_t kCreOffPosX = 0x24;         // +0x24 read VERIFIED (A1 f11); vec3 meaning INFERRED
constexpr size_t kCreOffPosY = 0x28;         // INFERRED vec3 component
constexpr size_t kCreOffPosZ = 0x2C;         // INFERRED vec3 component
constexpr size_t kCreOffAppCounter = 0x328;  // UNVERIFIED: pending-appearance counter per A1 f12; writer not decoded

// Hook 1 (0x85E601) stack frame, offsets from EBP at hook entry (A1 f1).
constexpr size_t kFrameOffPDesc = 0x08;       // [EBP+0x08] = pDesc (VERIFIED)
constexpr size_t kFrameOffOutTexture = 0x10;  // [EBP+0x10] = optional out texture resref (VERIFIED)

// 16-byte engine resref buffer. The engine copies 16 bytes (A1 f2); a full 16-char name has no NUL.
struct ResRef {
    char value[16];
};
static_assert(sizeof(ResRef) == 16, "ResRef must be 16 bytes");

} // namespace layout
} // namespace NpcDiversity
