// DiversityAnimEngine.h -- NB2 engine side (DLL only, not host-tested): client oid -> server creature -> client creature -> body Gob -> PlayAnimation.
// Every pointer is re-resolved from the oid at use time (nothing is cached across ticks), so a freed object resolves to 0 instead of dangling.
// Evidence: .claude/research/npc_variety/nb2_recon.md (Steam exe 6a522e71) and doc 40 section 15.
//   server object   = thiscall 0x0051C0A0 (this = [[0xA1B4A4]+8], oid) RET 4        (companion-ai uses the same call)
//   client creature = fastcall 0x0054A840 (ECX = server object): key = oid|0x80000000 -> 0x0073F4D0, result cached at server+0x264. UNVERIFIED
//                     live for ordinary creatures (the recon found no writer for the high-bit buckets), hence the probe stages below.
//   vtable check    = [client] == 0x0099EE14 (CSWCCreature)
//   body Gob        = holder = [client+0x68]; holder->vt[2](0xFF)                    (equip3d partGob, proven there for preview creatures)
//   play            = body->vt[6](name, speed, flags, 0.0f) = Gob::PlayAnimation (equip3d R4a, PartPlaySlot 0x18)
#pragma once

#include "DiversityAnim.h"
#include <stdint.h>

namespace NpcDiversity {
namespace AnimEngine {

constexpr uintptr_t kGlobalApp = 0x00A1B4A4;       // CAppManager*: +4 client, +8 server
constexpr uintptr_t kFnServerObject = 0x0051C0A0;
constexpr uintptr_t kFnClientObject = 0x0054A840;
constexpr uint32_t kVtCreatureClient = 0x0099EE14;

constexpr int kOffType = 0x08, kOffAction = 0x84, kOffQueue = 0x100, kOffCombat = 0x520, kOffDialog = 0x54;
constexpr int kOffInventory = 0x1150, kInvRightHand = 0x14;   // nb2_weapon_recon.md: CSWInventory* at server+0x1150, slot mask 0x10 -> inv+0x14
constexpr int kOffParty = 0x11AC, kOffIsPc = 0x10E8, kOffClientHolder = 0x68, kOffClientCache = 0x264;
constexpr int kHolderGetSlot = 2, kBodyPlaySlot = 6;
constexpr int kPartBody = 0xFF;

// How far one resolve got. The probe logs this per creature once.
enum Stage : int {
    StNone = 0, StServer = 1, StType = 2, StClient = 3, StVtable = 4, StHolder = 5, StBody = 6,
};

struct Chain {
    int stage = StNone;
    uintptr_t server = 0, client = 0, holder = 0, body = 0;
    Anim::ServerFacts facts = {};
    uint32_t serverOid = 0;      // the oid read back from the server object (+0x04)
    uint32_t clientVt = 0;
    uint32_t cache = 0;          // server+0x264 after the GetClientObject call
    uint32_t inv = 0;            // server+0x1150 (CSWInventory*)
    uint32_t qd[3] = {0, 0, 0};  // first dwords at the action list head (logged to find an emptiness test)
};

inline const char* StageName(int s) {
    static const char* n[] = {"none", "server", "type", "client", "vtable", "holder", "body"};
    return (s >= 0 && s <= StBody) ? n[s] : "?";
}

}  // namespace AnimEngine
}  // namespace NpcDiversity

#ifdef NPCDIV_ANIM_ENGINE_IMPL   // defined once, by the dispatcher, after SafeRead.h

namespace NpcDiversity {
namespace AnimEngine {

constexpr unsigned kSite = 12;   // SafeRead site id (dispatcher enum S_ANIM)

template <typename T> inline bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(kSite, base, off, out); }

// Walks the chain as far as it goes and fills facts. Never throws, never writes.
inline void Resolve(uint32_t oid, Chain* c) {
    *c = Chain();
    uint32_t app = 0, server = 0;
    if (!oid || !rd(kGlobalApp, 0, &app) || !app || !rd(app, 8, &server) || !server) return;
    using FnObj = uintptr_t(__thiscall*)(uintptr_t, uint32_t);
    // Run A (10-18): the client oid (bit 31 set, e.g. 0x8000000B) found no server object. GetClientObject keys on serverOid|0x80000000,
    // so the server oid is the client oid with bit 31 cleared.
    c->server = reinterpret_cast<FnObj>(kFnServerObject)(server, oid & 0x7FFFFFFFu);
    if (!c->server) return;
    c->stage = StServer;
    uint8_t type = 0;
    rd(c->server, 0x04, &c->serverOid);
    if (!rd(c->server, kOffType, &type)) return;
    c->facts.type = type;
    if (type != 5) return;
    c->stage = StType;
    c->facts.action = 0;
    rd(c->server, kOffParty, &c->facts.party);
    rd(c->server, kOffIsPc, &c->facts.isPc);
    rd(c->server, kOffAction, &c->facts.action);
    rd(c->server, kOffQueue, &c->facts.queueHead);
    if (c->facts.queueHead) for (int i = 0; i < 3; ++i) rd(c->facts.queueHead, i * 4, &c->qd[i]);
    rd(c->server, kOffCombat, &c->facts.combat);
    rd(c->server, kOffDialog, &c->facts.dialog);
    // Right-hand weapon: read the slot's object id straight from the inventory (no GetItemInSlot call, nothing written).
    c->facts.rightHand = 0;
    if (rd(c->server, kOffInventory, &c->inv) && c->inv) rd(c->inv, kInvRightHand, &c->facts.rightHand);

    using FnClient = uintptr_t(__fastcall*)(uintptr_t);
    c->client = reinterpret_cast<FnClient>(kFnClientObject)(c->server);
    rd(c->server, kOffClientCache, &c->cache);
    if (!c->client) return;
    c->stage = StClient;
    if (!rd(c->client, 0, &c->clientVt) || c->clientVt != kVtCreatureClient) return;
    c->stage = StVtable;
    uint32_t holder = 0;
    if (!rd(c->client, kOffClientHolder, &holder) || !holder) return;
    c->holder = holder;
    c->stage = StHolder;
    uint32_t hvt = 0, getFn = 0;
    if (!rd(holder, 0, &hvt) || !rd(hvt, kHolderGetSlot * 4, &getFn) || !getFn) return;
    using FnPart = uintptr_t(__thiscall*)(uintptr_t, int);
    c->body = reinterpret_cast<FnPart>(getFn)(holder, kPartBody);
    if (!c->body) return;
    c->stage = StBody;
}

// Plays name on the body Gob. flags: 0 loops until replaced, 1 = engine "once" (fades out), 2 = no cross-fade (Mod 10 notes).
inline bool Play(const Chain& c, const char* name, float speed, unsigned flags) {
    if (c.stage < StBody || !c.body) return false;
    uint32_t bvt = 0, fn = 0;
    if (!rd(c.body, 0, &bvt) || !rd(bvt, kBodyPlaySlot * 4, &fn) || !fn) return false;
    using FnPlay = void(__thiscall*)(uintptr_t, const char*, float, unsigned, float);
    reinterpret_cast<FnPlay>(fn)(c.body, name, speed, flags, 0.0f);
    return true;
}

}  // namespace AnimEngine
}  // namespace NpcDiversity

#endif  // NPCDIV_ANIM_ENGINE_IMPL
