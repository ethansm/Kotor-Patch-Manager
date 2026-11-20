/* NpcWorld.h -- the "k2.npcworld" v1 registry interface (provider: the npc-world patch) + a C++ consumer helper.
 *
 * Why: area load, creature AddToArea and the per-frame server tick are interesting to several NPC mods (appearance diversity, extra
 * walkers, animation, scripted encounters), but KPM rejects two patches that detour one address. npc-world owns the three detours
 * (CSWSModule::LoadModuleInProgress, creature AddToArea, CServerExoAppInternal::MainLoop; addresses in npc_world_src's hooks toml),
 * tracks one area visit, and exposes events plus a few services. Design: patch_manager_mods/40_npc_behaviour_variety.md section 12.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 *
 * Events (a bit mask when registering, exactly one bit in an event):
 *   K2NW_AREA_LOAD       a module/area load started (H2). visit_salt is already the NEW visit's salt; area is "" (not known yet).
 *   K2NW_CREATURE_ADDED  a creature is being added to an area (H3, runs before the engine's own AddToArea code). oid/tag/pos are read
 *                        when readable (oid 0 / tag "" otherwise). origin tells engine adds from the library's own Spawn().
 *   K2NW_AREA_STABLE     once per load, after the area saw 30 ticks with no engine creature add (the "population is stable" latch,
 *                        7/7 loads in area-hook-probe 0.2.0). Safe moment to Spawn(). visit_seq counts these per process.
 *   K2NW_TICK            every MainLoop tick (opt-in, hot: keep the callback trivial; most consumers want AREA_STABLE).
 *
 * Visit salt: a nonzero uint32 chosen at AREA_LOAD and constant for the whole visit (until the next AREA_LOAD). VisitSalt() is valid
 * from AREA_LOAD on, so a SetAppearance hook that runs during the load can already read it. 0 = no load seen yet.
 *
 * Threading / cost: callbacks run on the engine thread inside engine code. A callback MUST be cheap and MUST NOT block or throw. The
 * provider times every callback and logs one WARN when one takes longer than 5 ms; it keeps dispatching. Registering / unregistering
 * from inside a callback is allowed: a consumer registered during a dispatch is first called for the NEXT event, a consumer
 * unregistered during a dispatch is not called afterwards. Spawn() may be called from a callback (it re-enters H3 internally; those
 * adds are delivered with origin K2NW_ORIGIN_LIB, never counted as population).
 *
 * Pointers in events (area_ptr, creature) are valid only during the callback; copy what you need. Key persistent state on tag, not
 * on oid (a spawned creature's oid changes every load; doc 40 P4).
 */
#ifndef K2_NPCWORLD_H
#define K2_NPCWORLD_H

#include "kpatch_api.h"
#include <stddef.h>
#include <stdint.h>

#define K2NW_IFACE   "k2.npcworld"
#define K2NW_VERSION 1

#define K2NW_AREA_LOAD       1
#define K2NW_CREATURE_ADDED  2
#define K2NW_AREA_STABLE     4
#define K2NW_TICK            8
#define K2NW_ALL             15

#define K2NW_ORIGIN_ENGINE 0
#define K2NW_ORIGIN_LIB    1

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2NwEvent {
    uint32_t struct_size;
    int32_t kind;          /* exactly one K2NW_* bit */
    int32_t load_seq;      /* 1-based count of AREA_LOAD in this process */
    int32_t visit_seq;     /* count of AREA_STABLE so far in this process (including this one for AREA_STABLE) */
    uint32_t visit_salt;   /* the current visit's salt, 0 before the first AREA_LOAD */
    uint32_t tick;         /* MainLoop tick counter since process start (wraps) */
    const char* area;      /* current area resref, "" when unknown; never NULL */
    void* area_ptr;        /* engine area object, NULL when unknown */
    void* creature;        /* K2NW_CREATURE_ADDED only: the creature object, else NULL */
    uint32_t oid;          /* K2NW_CREATURE_ADDED only: object id when readable, else 0 */
    const char* tag;       /* K2NW_CREATURE_ADDED only: creature tag when readable, else ""; never NULL */
    float x, y, z;         /* K2NW_CREATURE_ADDED only: position at entry (0 if unreadable) */
    int32_t origin;        /* K2NW_CREATURE_ADDED only: K2NW_ORIGIN_* */
} K2NwEvent;

typedef void (KPATCH_CALL *K2NwCallback)(const K2NwEvent* ev, void* user);

typedef struct K2NpcWorldApi {
    uint32_t struct_size;
    /* Call fn for every event whose kind is in kindMask (K2NW_*). owner = your manifest id (logged). Returns a token > 0,
       or <= 0 (no valid kind bit, null fn, table full). */
    int32_t (KPATCH_CALL *Register)(int32_t kindMask, K2NwCallback fn, void* user, const char* owner);
    void    (KPATCH_CALL *Unregister)(int32_t token);
    uint32_t (KPATCH_CALL *VisitSalt)(void);   /* see above; 0 = none yet */
    int32_t (KPATCH_CALL *VisitSeq)(void);     /* number of AREA_STABLE so far */
    /* snprintf-style: current area resref into buf, returns its full length (>= cap = truncated), 0 = unknown (buf[0] = 0). */
    int32_t (KPATCH_CALL *CurrentArea)(char* buf, uint32_t cap);
    /* 1 when a creature with this tag (case-insensitive) was seen entering the current area, or when the seen-tag table overflowed
       (unknown counts as present so nobody double-spawns); 0 otherwise. NULL/empty tag -> 0. */
    int32_t (KPATCH_CALL *TagPresent)(const char* tag);
    /* Spawn one creature from a UTC template through the engine's CreateObject path (new, ctor, LoadFromTemplate, orient, AddToArea).
       tag = the template's tag, used only for the duplicate check and the cap (pass the UTC's tag). Returns the new object id, or 0
       when refused: provider disabled, area unknown or not stable yet, resref empty, tag already present, spawn cap reached, or the
       template failed to load. Never call before AREA_STABLE of the current visit. */
    uint32_t (KPATCH_CALL *Spawn)(const char* resref, const char* tag, float x, float y, float z, float facingRadians);
    int32_t (KPATCH_CALL *SpawnedCount)(void); /* successful Spawn() calls in the current visit */
    int32_t (KPATCH_CALL *SpawnCap)(void);     /* max Spawn() successes per visit (npc_world.ini [world] spawn_cap, default 4) */
} K2NpcWorldApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */

// ---- C++ consumer helper ------------------------------------------------------------------------------------------------------
// In KPatchInit:   g_nw.init(api, "my-patch-id"); g_nw.add(K2NW_AREA_STABLE, onStable); g_nw.connect();
// In your hooks:   g_nw.poll();   (retries connect() if the provider was not there at init: config order is not a contract)
//                  uint32_t salt = g_nw.visitSalt();
// Without a provider (not installed, or a patcher that predates KPatchInit) every call is a safe no-op: no callback ever fires, getters
// return 0 and spawn() returns 0. Check ev->struct_size with K2NW_EVENT_HAS before reading a field newer than the oldest provider you
// support.
namespace k2nw {

constexpr int MaxRegs = 8;

inline bool eventHas(const K2NwEvent* ev, size_t fieldEnd) { return ev && ev->struct_size >= fieldEnd; }
#define K2NW_EVENT_HAS(ev, field) ::k2nw::eventHas((ev), offsetof(K2NwEvent, field) + sizeof((ev)->field))

struct Client {
    struct Reg { int32_t mask; K2NwCallback fn; void* user; int32_t token; };
    const KPatchApi* api = nullptr;
    const K2NpcWorldApi* nw = nullptr;
    const char* owner = "";
    Reg regs[MaxRegs] = {};
    int nRegs = 0;
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;   // optional: log the outcome of each connect attempt that changes state
    bool reportedMissing = false;

    void init(const KPatchApi* a, const char* id) { api = a; owner = id ? id : ""; }
    bool add(int32_t kindMask, K2NwCallback fn, void* user = nullptr) {
        if (nRegs >= MaxRegs || !fn || (kindMask & K2NW_ALL) == 0) return false;
        regs[nRegs++] = { kindMask, fn, user, 0 };
        return true;
    }
    // The table must reach the last v1 field, or it is treated as absent.
    static bool usable(const K2NpcWorldApi* p) {
        return p && p->struct_size >= offsetof(K2NpcWorldApi, SpawnCap) + sizeof(p->SpawnCap)
                 && p->Register && p->Unregister && p->VisitSalt && p->VisitSeq && p->CurrentArea && p->TagPresent
                 && p->Spawn && p->SpawnedCount && p->SpawnCap;
    }
    // require + register everything once. Idempotent; true when connected.
    bool connect() {
        if (connected) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2NpcWorldApi* p = static_cast<const K2NpcWorldApi*>(api->require(K2NW_IFACE, K2NW_VERSION));
        if (!usable(p)) {
            if (!reportedMissing && onConnect) onConnect(false, p ? "k2.npcworld v1 table too small" : "k2.npcworld v1 not provided (npc-world missing or loaded later)");
            reportedMissing = true;
            return false;
        }
        nw = p;
        int ok = 0;
        for (int i = 0; i < nRegs; ++i) { regs[i].token = nw->Register(regs[i].mask, regs[i].fn, regs[i].user, owner); if (regs[i].token > 0) ++ok; }
        connected = true;
        if (onConnect) onConnect(ok == nRegs, ok == nRegs ? "registered" : "some registrations refused");
        return true;
    }
    // Cheap retry from a hook: one require every 256 calls until connected.
    void poll() { if (!connected && (pollCount++ & 255) == 0) connect(); }
    // Unregisters everything that was accepted and forgets the provider (the next connect() registers again).
    void release() {
        if (!connected || !nw) return;
        for (int i = 0; i < nRegs; ++i) { if (regs[i].token > 0) nw->Unregister(regs[i].token); regs[i].token = 0; }
        nw = nullptr; connected = false;
    }

    // ---- service wrappers: safe no-ops without a provider ----
    uint32_t visitSalt() const { return nw ? nw->VisitSalt() : 0u; }
    int32_t visitSeq() const { return nw ? nw->VisitSeq() : 0; }
    int32_t currentArea(char* buf, uint32_t cap) const {
        if (!nw) { if (buf && cap) buf[0] = 0; return 0; }
        return nw->CurrentArea(buf, cap);
    }
    bool tagPresent(const char* tag) const { return nw && nw->TagPresent(tag) != 0; }
    uint32_t spawn(const char* resref, const char* tag, float x, float y, float z, float facing) const {
        return nw ? nw->Spawn(resref, tag, x, y, z, facing) : 0u;
    }
    int32_t spawnedCount() const { return nw ? nw->SpawnedCount() : 0; }
    int32_t spawnCap() const { return nw ? nw->SpawnCap() : 0; }
};

} // namespace k2nw
#endif /* __cplusplus */

#endif /* K2_NPCWORLD_H */
