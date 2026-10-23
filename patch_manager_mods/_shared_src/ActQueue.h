// ActQueue.h -- guarded reader + pure helpers for one SERVER creature's action queue and combat round (extracted from queue_hud_src/QueueHud.cpp gatherFor).
// Engine layout (queue-hud RE.md): queue = inline CExoLinkedList at cr+0x100 -> internal -> head node (node next @4, data @8; action type dword
// @data+0, param0 @data+0x38); round object at cr+0x10DC with pending-node list ptr @+0xB00, current node ptr @+0xB18, and the wait flags @+0xAB8 / @+0xAA8.
// Round node fields: sub-type byte @+0x10, spell id @+0x24, feat word @+0x5C, item id @+0x64.
// Pure part (types + has/head/starved/useQueued) has no engine dependency. read() goes through saferead only; no engine calls.
#pragma once
#include <stdint.h>
#include "SafeRead.h"
#include "GameAddr.h"

namespace actq {

// ---- engine offsets (named; no raw engine function addresses) ----
constexpr int CrQueue = 0x100, CrRound = 0x10DC;                        // server creature
constexpr int RdList = 0xB00, RdCur = 0xB18, RdAA8 = 0xAA8, RdAB8 = 0xAB8;   // round object
constexpr int NodeNext = 4, NodeData = 8;                                // CExoLinkedList node
constexpr int ActType = 0, ActParam0 = 0x38;                             // queued action
constexpr int RnSub = 0x10, RnSpell = 0x24, RnFeat = 0x5C, RnItem = 0x64;   // round node
constexpr int kMaxEntries = 16, kMaxRound = 4;
constexpr int CrRunning = 0x20C;   // creature: node in flight inside RunActions (R8 rotate gate: must be 0)
constexpr int Cr11AC = 0x11AC;     // creature: R8 rotate gate (must be nonzero); meaning UNVERIFIED, logged
constexpr int RdA84 = 0xA84;       // round object: AIActionCombat returns 1 vs 2 on it (R8 Q4); named only
constexpr int PauseInternal = 0x10078;   // byte [[[[AppGlobal]+8]+4]+0x10078] & 2 = player pause

// Action types the helpers know about.
constexpr int T_NONE = -1;
constexpr int T_FOLLOW_LEADER = 0x3D;  // FollowLeader: a follower's permanent resting action; its handler 0x5BF7F0 re-adds itself at the head (doc 29 D5)
constexpr int T_USE_ITEM = 0x3F;       // item use / cast-spell action appended by AddItemCastSpellActions (route A, doc 29 D5)

struct Entry { int type = -1; uint32_t param0 = 0; };   // type -1 = unreadable (keeps its index, never matches)
struct RNode { uint8_t subType = 0; uint32_t spellId = 0, itemId = 0, featId = 0, ptr = 0; };

struct Snapshot {
    uintptr_t creature = 0, round = 0;
    Entry e[kMaxEntries]; uintptr_t eData[kMaxEntries] = {}; int ne = 0;
    RNode rn[kMaxRound]; int nrn = 0;
    RNode cur; bool haveCur = false;
    int ab8 = 0, aa8 = 0;
};

// ---- pure helpers ----
inline bool hasType(const Snapshot& s, int type) {
    for (int i = 0; i < s.ne && i < kMaxEntries; ++i) if (s.e[i].type == type) return true;
    return false;
}
inline int headType(const Snapshot& s) { return s.ne > 0 ? s.e[0].type : -1; }
// Index of the first entry of `type` (-1 = none).
inline int firstIndex(const Snapshot& s, int type) {
    for (int i = 0; i < s.ne && i < kMaxEntries; ++i) if (s.e[i].type == type) return i;
    return -1;
}
// An item use is really pending: a round node (or the current node) carries an item id. A bare 0x3F with an empty round list is a
// harmless leftover that stays behind 0x3D forever (R8 Q3), so it does NOT count.
inline bool useQueued(const Snapshot& s) {
    for (int i = 0; i < s.nrn && i < kMaxRound; ++i) if (s.rn[i].itemId != 0) return true;
    return s.haveCur && s.cur.itemId != 0;
}
// RQ13: a cast/use is stalled behind FollowLeader: first 0x3F at index 1..3 with only 0x3D before it (same shape as starved()) AND a pending
// ROUND-LIST node (rn[], never cur: cur goes stale, see QueueHudLogic.h curReal/kStaleCurMs) has sub-type 9 (cast) or 10 (item). Covers queue-hud's
// server-route Force power (0x3F + node 9, itemId 0). An executing cast (0x3F at the head) and a bare leftover 0x3F do NOT count.
inline bool castQueued(const Snapshot& s) {
    const int k = firstIndex(s, T_USE_ITEM);
    if (k < 1 || k > 3) return false;
    for (int i = 0; i < k; ++i) if (s.e[i].type != T_FOLLOW_LEADER) return false;
    for (int i = 0; i < s.nrn && i < kMaxRound; ++i) if (s.rn[i].subType == 9 || s.rn[i].subType == 10) return true;
    return false;
}
// Starved (R8 rotate rule): a use is queued, the first 0x3F sits at index 1..3 and every node before it is 0x3D
// (a MoveToPoint / 0x40 ahead of it means the follow driver is mid-expansion: not starved, wait).
inline bool starved(const Snapshot& s) {
    if (!useQueued(s)) return false;
    int k = firstIndex(s, T_USE_ITEM);
    if (k < 1 || k > 3) return false;
    for (int i = 0; i < k; ++i) if (s.e[i].type != T_FOLLOW_LEADER) return false;
    return true;
}

// Player pause: byte [[[[AppGlobal]+8]+4]+0x10078] & 2 (0x51C720 / 0x538CD0). Unreadable chain = not paused.
inline bool playerPaused(unsigned site) {
    uint32_t app = 0, srv = 0, internal = 0;
    uint8_t b = 0;
    if (!saferead::readAt(site, gameaddr::AppGlobal, 0, &app) || !app) return false;
    if (!saferead::readAt(site + 1, app, 8, &srv) || !srv) return false;
    if (!saferead::readAt(site + 2, srv, 4, &internal) || !internal) return false;
    return saferead::readAt(site + 3, internal, PauseInternal, &b) && (b & 2) != 0;
}

// ---- guarded reader ----
// Site ids: queue reads use siteQueue+0..5, round reads siteRound+0..3 and +8..12 (QueueHud passes its SiteQueue / SiteRound).
inline bool readRNode(unsigned siteRound, uintptr_t n, RNode* out) {
    uint8_t sub = 0; uint32_t spell = 0, item = 0; uint16_t feat = 0;
    if (!n || !saferead::readAt(siteRound + 4, n, RnSub, &sub)) return false;
    saferead::readAt(siteRound + 5, n, RnSpell, &spell); saferead::readAt(siteRound + 6, n, RnItem, &item); saferead::readAt(siteRound + 7, n, RnFeat, &feat);
    out->subType = sub; out->spellId = spell; out->itemId = item; out->featId = feat; out->ptr = static_cast<uint32_t>(n);
    return true;
}

// false = no creature. Everything else degrades to an empty queue / round.
inline bool read(uintptr_t serverCr, Snapshot* out, unsigned siteQueue, unsigned siteRound) {
    if (!out) return false;
    Snapshot& s = *out;
    s = Snapshot();
    s.creature = serverCr;
    if (!s.creature) return false;
    auto rd = [](unsigned site, uintptr_t base, int off, auto* v) { return saferead::readAt(site, base, off, v); };
    uint32_t internal = 0, node = 0;
    if (rd(siteQueue, s.creature, CrQueue, &internal) && internal && rd(siteQueue + 1, internal, 0, &node)) {
        for (int i = 0; node && i < kMaxEntries; ++i) {
            uint32_t data = 0, next = 0, type = 0, p0 = 0;
            rd(siteQueue + 2, node, NodeData, &data);
            rd(siteQueue + 3, node, NodeNext, &next);
            Entry& e = s.e[s.ne];
            e.type = -1; e.param0 = 0; s.eData[s.ne] = 0;   // an unreadable entry keeps its index but never qualifies
            if (data && rd(siteQueue + 4, data, ActType, &type)) {
                e.type = static_cast<int>(type);
                rd(siteQueue + 5, data, ActParam0, &p0);
                e.param0 = p0;
                s.eData[s.ne] = data;
            }
            ++s.ne;
            node = next;
        }
    }
    uint32_t round = 0;
    rd(siteRound, s.creature, CrRound, &round);
    s.round = round;
    if (round) {
        uint32_t list = 0, curPtr = 0;
        if (rd(siteRound + 1, round, RdList, &list) && list && rd(siteRound + 2, list, 0, &internal) && internal && rd(siteRound + 3, internal, 0, &node)) {
            for (int i = 0; node && i < kMaxRound; ++i) {
                uint32_t data = 0, next = 0;
                rd(siteRound + 8, node, NodeData, &data);
                rd(siteRound + 9, node, NodeNext, &next);
                if (data && readRNode(siteRound, data, &s.rn[s.nrn])) ++s.nrn;
                node = next;
            }
        }
        if (rd(siteRound + 10, round, RdCur, &curPtr) && curPtr && readRNode(siteRound, curPtr, &s.cur)) s.haveCur = true;
        rd(siteRound + 11, round, RdAB8, &s.ab8);
        rd(siteRound + 12, round, RdAA8, &s.aa8);
    }
    return true;
}

}  // namespace actq
