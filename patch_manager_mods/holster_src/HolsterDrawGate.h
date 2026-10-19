// HolsterDrawGate.h -- pure draw-gate decision of the holstered-weapons mod (doc 42, HW-UX3 Wave C1; no Win32, host-testable; tests in holster_drawgate_test.cpp).
// holster.ini place_mode=drawgate keeps the engine's native rhand/lhand attach (no code patch) and lets HolsterPlace (hook H7) write the hip pose only
// while the owner creature is NOT drawn. place_mode=repoint is the old behaviour (global attach-node repoint, the hip pose is always written).
//   shouldWriteHip   true = write the hip pose, false = leave the hand pose the engine wrote (weapon drawn / vanilla)
//   nodeAccepted     which attach node name HolsterPlace treats as its target (repoint: bone_name; drawgate: rhand, and lhand when enabled)
//   GateState        per placement-cache entry: owner + hip/drawn state; gateUpdate reports a change so the entry's resolved placement is dropped
//   isWeaponModel    drawgate target filter: the Gob's model name starts with "w_" (weapon models); anything else on a hand node is left alone
//   handFromNode     which hand a placement entry is: drawgate = the attach node name (lhand => left); repoint = the old sticky ring resolve
//   holdActive       toggle-key draw hold: owner matches the hold owner and holdUntil (ms tick, wrap-safe) is still in the future
//   poseBaseEqual    tolerance compare of two world poses (mountPose(pelvis_g) base vs the old pelvis attach-node base)
// Failure mode rule: anything unreadable (owner null, combat flag unreadable) is treated as DRAWN, so the weapon is never stuck on the hip.
#pragma once
#include <stdint.h>
#include <math.h>
#include "HolsterQuat.h"
#include "HolsterHand.h"

namespace hs {

enum PlaceMode : int { PlaceRepoint = 0, PlaceDrawgate = 1 };

inline bool ieq(const char* a, const char* b) {
    if (!a || !b) return false;
    for (;; ++a, ++b) {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z') x = static_cast<char>(x - 'A' + 'a');
        if (y >= 'A' && y <= 'Z') y = static_cast<char>(y - 'A' + 'a');
        if (x != y) return false;
        if (x == 0) return true;
    }
}

// "drawgate" | "repoint" (case-insensitive). Anything else leaves `out` untouched and returns false.
inline bool placeModeParse(const char* s, PlaceMode& out) {
    if (ieq(s, "drawgate")) { out = PlaceDrawgate; return true; }
    if (ieq(s, "repoint")) { out = PlaceRepoint; return true; }
    return false;
}
inline const char* placeModeName(PlaceMode m) { return m == PlaceDrawgate ? "drawgate" : "repoint"; }

// True = write the hip pose. Repoint mode: always (old behaviour). Drawgate: only with a readable owner, combat flag 0 and no flourish window.
inline bool shouldWriteHip(bool drawgate, bool ownerReadable, uint32_t combatFlag, bool flourishOpen) {
    if (!drawgate) return true;
    return ownerReadable && combatFlag == 0 && !flourishOpen;
}

// Toggle-key draw hold (holster toggle key). True while `owner` is the creature the hold was set for and the hold deadline is still ahead of `now`.
// All tick math is wrap-safe (signed 32-bit difference). holdOwner 0 = no hold. Does not affect shouldWriteHip; the caller ORs it in as "drawn".
inline bool holdActive(uintptr_t owner, uintptr_t holdOwner, uint32_t holdUntil, uint32_t now) {
    return holdOwner != 0 && owner == holdOwner && static_cast<int32_t>(holdUntil - now) > 0;
}

// Attach-node name HolsterPlace accepts. Repoint: the configured bone_name (the engine attached the weapon there). Drawgate: the native hand nodes,
// "lhand" only when leftOn (bone_test_left=1).
inline bool nodeAccepted(bool drawgate, const char* nodeName, const char* boneName, bool leftOn) {
    if (!nodeName || !nodeName[0]) return false;
    if (!drawgate) return ieq(nodeName, boneName);
    return ieq(nodeName, "rhand") || (leftOn && ieq(nodeName, "lhand"));
}

// Drawgate target filter (HW-UX3 C1 review B1): only weapon Gobs are placed. Weapon models are named "w_..." (live: w_Lngswrd_001, w_SonicPstl_002);
// case-insensitive prefix. Null / empty / unreadable name is NOT a weapon (fail-safe: the object stays where the engine put it).
inline bool isWeaponModel(const char* modelName) {
    if (!modelName) return false;
    if (modelName[0] != 'w' && modelName[0] != 'W') return false;
    return modelName[1] == '_';   // short-circuit: never reads past the terminator
}

// Left flag of a placement entry (C1 review B2). Drawgate: the attach node name is authoritative (lhand => left, anything else => right); the
// hand ring and the previous flag are ignored. Repoint (both hands share one node): the old sticky resolveLeft(prev, ringLookup), unchanged.
inline bool handFromNode(bool drawgate, const char* nodeName, int ringLookup, bool prevLeft) {
    if (drawgate) return ieq(nodeName, "lhand");
    return resolveLeft(prevLeft, ringLookup);
}

// Per placement-cache entry. hip: -1 = not evaluated yet, 0 = drawn (weapon in hand), 1 = hip pose written.
struct GateState { uintptr_t owner; int hip; };
inline GateState gateInit() { return GateState{0, -1}; }

// Records the current (owner, hip) and returns true when it differs from the stored one (first evaluation included): the caller then drops the
// entry's resolved placement (seq = -1) and logs the transition.
inline bool gateUpdate(GateState& s, uintptr_t owner, bool hip) {
    const int h = hip ? 1 : 0;
    if (s.hip == h && s.owner == owner) return false;
    s.owner = owner;
    s.hip = h;
    return true;
}

// Same world position (metres) and orientation (degrees, q and -q equal) within tolerance.
inline bool poseBaseEqual(const Pose& a, const Pose& b, float posTol, float angTolDeg) {
    for (int i = 0; i < 3; ++i)
        if (!(fabsf(a.pos[i] - b.pos[i]) <= posTol)) return false;
    float d = a.q.w * b.q.w + a.q.x * b.q.x + a.q.y * b.q.y + a.q.z * b.q.z;
    d = fabsf(d);
    if (d > 1.0f) d = 1.0f;
    const float ang = 2.0f * acosf(d) * 57.29578f;
    return ang <= angTolDeg;
}

}  // namespace hs
