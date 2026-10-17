// Preview3dLogic.h -- pure logic of the k2.preview3d provider (equipment-3d-character preview): the viewer token table and the
// idle-name choice. The camera maths and drag state come from the shared ViewDrag.h (also used by holster-editor); equip3d keeps its
// own eq3d copy in Equip3dLogic.h, which is NOT included here. No engine calls, no Win32 types, no KpLog: the provider .cpp keeps all
// engine access and logging.
#pragma once
#include <math.h>
#include <stdint.h>
#include "ViewDrag.h"   // camera maths and drag state (p3d::fovForZoom, yawVec, zoomStep, Drag, dragMove ...), shared with holster-editor

namespace p3d {

// ---------------------------------------------------------------- viewer tokens
// A fixed table of live viewers. A token is never 0 and is never handed out while another live viewer holds it. Tokens are not
// reused right after a removal: the counter runs on, so a stale token from a destroyed view cannot alias a new one.
constexpr int kMaxViewers = 4;

struct Viewers {
    uint32_t ids[kMaxViewers];   // live tokens; 0 = free slot
    uint32_t nextId;             // next counter value to try (never 0 after init)
};

inline void viewersInit(Viewers* v) {
    for (int i = 0; i < kMaxViewers; ++i) v->ids[i] = 0;
    v->nextId = 1;
}

// Index of the live slot holding `token`, or -1 (token 0 is never live).
inline int viewersIndex(const Viewers* v, uint32_t token) {
    if (token == 0) return -1;
    for (int i = 0; i < kMaxViewers; ++i)
        if (v->ids[i] == token) return i;
    return -1;
}

inline int viewersCount(const Viewers* v) {
    int n = 0;
    for (int i = 0; i < kMaxViewers; ++i)
        if (v->ids[i] != 0) ++n;
    return n;
}

// New token, or 0 when every slot is in use. The counter skips 0 on wrap and skips any value still held by a live viewer.
inline uint32_t viewersAdd(Viewers* v) {
    int slot = -1;
    for (int i = 0; i < kMaxViewers && slot < 0; ++i)
        if (v->ids[i] == 0) slot = i;
    if (slot < 0) return 0;
    for (int tries = 0; tries <= kMaxViewers; ++tries) {   // at most kMaxViewers live ids, so one extra try always finds a free value
        uint32_t t = v->nextId;
        v->nextId = (t == 0xFFFFFFFFu) ? 1u : t + 1u;
        if (t != 0 && viewersIndex(v, t) < 0) {
            v->ids[slot] = t;
            return t;
        }
    }
    return 0;
}

// True when `token` was live and is now removed.
inline bool viewersRemove(Viewers* v, uint32_t token) {
    int i = viewersIndex(v, token);
    if (i < 0) return false;
    v->ids[i] = 0;
    return true;
}

// ---------------------------------------------------------------- idle
// Appearance rows that are droids: they idle on "pause1" whatever the alignment.
inline bool isDroidRow(int r) { return r == 2 || r == 57 || r == 231 || r == 232 || r == 453 || r == 3 || r == 538 || r == 539; }

// Idle animation name: droids "pause1", else by alignment: < 40 "evil", < 60 "neutral", otherwise "good".
inline const char* idleFor(int appearanceRow, short align) {
    return isDroidRow(appearanceRow) ? "pause1" : (align < 40 ? "evil" : (align < 60 ? "neutral" : "good"));
}

}  // namespace p3d
