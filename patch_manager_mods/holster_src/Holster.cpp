// Holster.cpp -- holster 0.1.0-dev (Steam build): P0 LOG-ONLY probe for the holstered-weapons mod (patch_manager_mods/42_holstered_weapons.md).
// Plan: .claude/plans/holstered_weapons.md. Handlers are plain extern "C" void __cdecl functions; the detour wrappers in
// kotor2-steam-aspyr.hooks.toml call them at function entry with the game arguments BEFORE the stolen bytes run.
// Hard rules for P0 (exceptions, both default off: the bone_test code patch, and HolsterPlace's 28-byte pose write with probe=1 bone_test=1 place=1): no other writes to game memory or registers, no engine calls, reads only through saferead, and every handler returns at
// once while its channel is off (holster.ini: probe=1 AND ch_<name>=1). All channels share one line budget (max_lines).
//
// Gates this probe answers (doc 42):
//   G2  ATTACH lines: obj/tag/creature plus creature+0x68 (body object) and the hand handles +0x278/+0x27C before and after
//   G3  COMB lines: does 0x7682B0 fire for NPCs and party members alike, old/new state per client creature
//   G4  ANIM lines: animation ids resolved/sent around combat entry (0x2712/0x2713 = suspected wield-transition ids)
//   G5  SWAP lines: the swap executor entry and the first dwords of its object
//   G1  GOB lines: per-update samples of the Gob that re-copies the attach-node transform (+0xA4 vec3 / +0xB0 quat)
// Pure logic (classify, mounts, state machine) lives in HolsterLogic.h and is not used by the probe yet.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "SafeRead.h"
#include "KpLog.h"
#include "K2Addr.h"
#include "HolsterLogic.h"
#include "HolsterBonePatch.h"
#include "HolsterQuat.h"
#include "HolsterTune.h"
#include "FileWatch.h"

KP_LOG_PATCH("holster");

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chComb, "comb");
KP_CHANNEL(chAttach, "attach");
KP_CHANNEL(chSwap, "swap");
KP_CHANNEL(chAnim, "anim");
KP_CHANNEL(chGob, "gob");
KP_CHANNEL(chPlace, "place");

// Fallback sink for an old core or a failed KpLog_Init. Stores the path and level only.
const bool g_fallbackSet = (KpLog_SetFallbackFile("holster_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// Config. Written once in KPatchInit (before the handlers do anything); defaults are silent.
static bool g_probe = false;
static bool g_onComb = false, g_onAttach = false, g_onSwap = false, g_onAnim = false, g_onGob = false, g_onPlace = false;   // probe=1 AND ch_<name>=1 (g_onPlace gates PLACE logging only)
static bool g_boneTest = false, g_boneLeft = false;   // bone_test=1 (needs probe=1), bone_test_left=1
static char g_boneName[32] = "pelvis_g";              // attach node name; its ADDRESS becomes the push immediate, so it must stay static
static LONG g_maxLines = 3000;
static int g_gobEvery = 600;            // GOB lines: first 24 updates, then every Nth (the handler runs per Gob per frame)
static uint32_t g_placeGobvt = 0;       // place_gobvt: required Gob vtable (0 = no check); boot-time only
static volatile LONG g_lines = 0;
static volatile LONG g_gobCalls = 0;
static volatile LONG g_placeCalls = 0;   // PLACE lines: first 24 placements, then every gob_every-th
static volatile LONG g_animPerId[0x100] = {};   // ANIMRES lines per id 0x27xx: ids resolve every frame (run 2: 0x2711 x3976, 0x2718 x1900), cap each at 20

// saferead site tags (failure ring only).
static const unsigned kSiteComb = 0xE70010u, kSiteAttach = 0xE70020u, kSiteSwap = 0xE70030u, kSiteAnim = 0xE70040u, kSiteGob = 0xE70050u, kSitePlace = 0xE70060u;

static inline unsigned U(const void* p) { return static_cast<unsigned>(reinterpret_cast<uintptr_t>(p)); }

static bool takeLine() {
    LONG n = InterlockedIncrement(&g_lines);
    if (n <= g_maxLines) return true;
    if (n == g_maxLines + 1) KP_LOG(chMain, "cap reached max_lines=%ld: further lines dropped", static_cast<long>(g_maxLines));
    return false;
}

static char* trimIni(char* s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ++s;
    size_t n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' || s[n - 1] == '\n')) s[--n] = 0;
    return s;
}

// Strict on: only 1 / on / true / yes switch a key on. Anything else (including a blank value) is off.
static bool flagValue(const char* v) {
    return !_stricmp(v, "1") || !_stricmp(v, "on") || !_stricmp(v, "true") || !_stricmp(v, "yes");
}

static_assert(sizeof(hs::Pose) == 28, "Gob+0xA4..+0xBF is {pos[3], q.w,q.x,q.y,q.z}");

// Live-tunable placement (place, place_rot, place_off). g_tuneW is the writer's master copy; g_tunePub is the reader copy published
// under a sequence counter (odd = write in progress). Readers retry up to 3 times and otherwise keep their previous local copy.
static hs::TuneCfg g_tuneW, g_tunePub;
static volatile LONG g_tuneSeq = 0;
static filewatch::ReloadPoll g_poll;
static volatile LONG g_reloadBusy = 0;

static void publishTune(const hs::TuneCfg& t) {
    g_tuneW = t;
    InterlockedIncrement(&g_tuneSeq);   // odd
    MemoryBarrier();
    memcpy(static_cast<void*>(&g_tunePub), &t, sizeof t);
    MemoryBarrier();
    InterlockedIncrement(&g_tuneSeq);   // even
}

static void loadTune(hs::TuneCfg& dst) {
    for (int i = 0; i < 3; ++i) {
        const LONG s1 = g_tuneSeq;
        MemoryBarrier();
        if (s1 & 1) continue;
        hs::TuneCfg tmp;
        memcpy(&tmp, static_cast<const void*>(&g_tunePub), sizeof tmp);
        MemoryBarrier();
        if (g_tuneSeq == s1) { dst = tmp; return; }
    }
}

static void readConfig() {
    hs::TuneCfg tune;
    bool probe = false, boneTest = false, boneLeft = false, comb = false, attach = false, swap = false, anim = false, gob = false, place = false;
    FILE* f = fopen("holster.ini", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char* p = trimIni(line);
            if (*p == 0 || *p == ';' || *p == '#' || *p == '[') continue;
            char* eq = strchr(p, '=');
            if (!eq) continue;
            *eq = 0;
            char* key = trimIni(p);
            char* val = trimIni(eq + 1);
            if (!_stricmp(key, "probe")) probe = flagValue(val);
            else if (!_stricmp(key, "bone_test")) boneTest = flagValue(val);
            else if (!_stricmp(key, "bone_test_left")) boneLeft = flagValue(val);
            else if (!_stricmp(key, "bone_name")) {
                // node names are plain identifiers; anything else (or empty) keeps the default
                size_t n = strlen(val);
                bool ok = n > 0 && n < sizeof(g_boneName);
                for (size_t i = 0; ok && i < n; ++i) ok = isalnum(static_cast<unsigned char>(val[i])) || val[i] == '_';
                if (ok) memcpy(g_boneName, val, n + 1);
            }
            else if (!_stricmp(key, "ch_comb")) comb = flagValue(val);
            else if (!_stricmp(key, "ch_attach")) attach = flagValue(val);
            else if (!_stricmp(key, "ch_swap")) swap = flagValue(val);
            else if (!_stricmp(key, "ch_anim")) anim = flagValue(val);
            else if (!_stricmp(key, "ch_gob")) gob = flagValue(val);
            else if (!_stricmp(key, "ch_place")) place = flagValue(val);
            else if (!_stricmp(key, "place_gobvt")) {
                char* end = nullptr;
                const unsigned long v = strtoul(val, &end, 16);   // accepts a 0x prefix; blank or junk = no check
                g_placeGobvt = (end == val) ? 0u : static_cast<uint32_t>(v);
            }
            else if (!_stricmp(key, "place") || !_stricmp(key, "place_rot") || !_stricmp(key, "place_off")) (void)hs::applyTuneKey(tune, key, val);
            else if (!_stricmp(key, "max_lines") || !_stricmp(key, "gob_every")) {
                char* end = nullptr;
                unsigned long v = strtoul(val, &end, 10);
                if (end == val) continue;
                if (!_stricmp(key, "max_lines") && v >= 1 && v <= 1000000ul) g_maxLines = static_cast<LONG>(v);
                if (!_stricmp(key, "gob_every") && v >= 1 && v <= 1000000ul) g_gobEvery = static_cast<int>(v);
            }
        }
        fclose(f);
    }
    g_probe = probe;
    g_boneTest = probe && boneTest;
    g_boneLeft = g_boneTest && boneLeft;
    g_onComb = probe && comb;
    g_onAttach = probe && attach;
    g_onSwap = probe && swap;
    g_onAnim = probe && anim;
    g_onGob = probe && gob;
    g_onPlace = probe && place;
    publishTune(tune);
}

static void addName(char* buf, size_t cap, bool on, const char* name) {
    if (!on) return;
    size_t n = strlen(buf);
    snprintf(buf + n, cap - n, "%s%s", n ? "," : "", name);
}

// Reads a u32 at base+off, 0 when unreadable (the probe prints rd-ok as a separate bit only where it matters).
static uint32_t rdU32(unsigned site, uintptr_t base, int off) {
    uint32_t v = 0;
    (void)saferead::readAt(site, base, off, &v);
    return v;
}
// Like rdU32, but reports whether the read succeeded (*out is 0 when it did not). Only the COMB old-state read uses it.
static bool rdU32Ok(unsigned site, uintptr_t base, int off, uint32_t* out) {
    uint32_t v = 0;
    const bool ok = saferead::readAt(site, base, off, &v);
    *out = v;
    return ok;
}
static float rdF(unsigned site, uintptr_t base, int off) {
    float v = 0.f;
    (void)saferead::readAt(site, base, off, &v);
    return v;
}

// Reads a NUL-terminated printable ASCII tag (at most cap-1 chars). "?" when unreadable, "" when null.
static void readTag(unsigned site, const char* p, char* out, int cap) {
    out[0] = 0;
    if (!p) return;
    int n = 0;
    for (; n < cap - 1; ++n) {
        char c = 0;
        if (!saferead::readAt(site, reinterpret_cast<uintptr_t>(p), n, &c)) { snprintf(out, cap, "?"); return; }
        if (c == 0) break;
        out[n] = (c >= 0x20 && c <= 0x7E) ? c : '.';
    }
    out[n] = 0;
}

// ---- handlers: called from the detour wrappers, before the stolen bytes run. Read-only. ----

// 0x007682B0 client combat flag setter. ECX = client creature, [esp+4] = new state. Logs EVERY call (G3: party only, or NPCs too).
extern "C" void __cdecl HolsterCombat(void* creature, int newState) {
    if (!g_onComb) return;
    saferead::beginScope();
    if (!KP_ENABLED(chComb, KPLOG_INFO) || !takeLine()) return;
    const uintptr_t c = reinterpret_cast<uintptr_t>(creature);
    const int off = k2addr::off(&K2AddrTable::Creature_ClientCombat);
    uint32_t raw = 0;
    const bool readOk = off != 0 && rdU32Ok(kSiteComb, c, off, &raw);
    const hs::CombFields f = hs::combFields(readOk, raw, newState);
    char oldS[12] = "?", edgeS[4] = "?";
    if (f.oldKnown) { snprintf(oldS, sizeof oldS, "%d", f.old); snprintf(edgeS, sizeof edgeS, "%d", f.edge); }
    KP_LOG(chComb, "COMB creature=%x old=%s new=%d edge=%s tick=%lu", U(creature), oldS, newState, edgeS,
           static_cast<unsigned long>(GetTickCount()));
}

// 0x007637E0 in-world weapon attach. [esp+4] = obj (u32), [esp+8] = tag (char*), [esp+0xC] = creature (int*).
// Logs the creature's body object and the hand handles the call is about to replace.
extern "C" void __cdecl HolsterAttach(uint32_t obj, const char* tag, int* creature) {
    if (!g_onAttach) return;
    saferead::beginScope();
    if (!KP_ENABLED(chAttach, KPLOG_INFO) || !takeLine()) return;
    char t[48];
    readTag(kSiteAttach, tag, t, sizeof(t));
    const uintptr_t c = reinterpret_cast<uintptr_t>(creature);
    const int oBody = k2addr::off(&K2AddrTable::Creature_BodyObj), oR = k2addr::off(&K2AddrTable::Creature_HandleR),
              oL = k2addr::off(&K2AddrTable::Creature_HandleL);
    KP_LOG(chAttach, "ATTACH obj=%x tag='%s' creature=%x body=%x handleR=%x handleL=%x tick=%lu", static_cast<unsigned>(obj), t, U(creature),
           oBody ? static_cast<unsigned>(rdU32(kSiteAttach, c, oBody)) : 0u, oR ? static_cast<unsigned>(rdU32(kSiteAttach, c, oR)) : 0u,
           oL ? static_cast<unsigned>(rdU32(kSiteAttach, c, oL)) : 0u, static_cast<unsigned long>(GetTickCount()));
}

// 0x005DF920 swap executor. ECX = executor object. Dumps its first 8 dwords (G5: which fields carry creature / slots).
extern "C" void __cdecl HolsterSwap(void* exec) {
    if (!g_onSwap) return;
    saferead::beginScope();
    if (!KP_ENABLED(chSwap, KPLOG_INFO) || !takeLine()) return;
    const uintptr_t e = reinterpret_cast<uintptr_t>(exec);
    KP_LOG(chSwap, "SWAP exec=%x d0=%x d1=%x d2=%x d3=%x d4=%x d5=%x d6=%x d7=%x tick=%lu", U(exec), static_cast<unsigned>(rdU32(kSiteSwap, e, 0)),
           static_cast<unsigned>(rdU32(kSiteSwap, e, 4)), static_cast<unsigned>(rdU32(kSiteSwap, e, 8)), static_cast<unsigned>(rdU32(kSiteSwap, e, 12)),
           static_cast<unsigned>(rdU32(kSiteSwap, e, 16)), static_cast<unsigned>(rdU32(kSiteSwap, e, 20)), static_cast<unsigned>(rdU32(kSiteSwap, e, 24)),
           static_cast<unsigned>(rdU32(kSiteSwap, e, 28)), static_cast<unsigned long>(GetTickCount()));
}

// 0x00863650 anim-id resolver (ECX = object, [esp+4] = u16 anim id). (H5 0x005644B0 was removed: A3 showed it is an action enqueue, not an anim sender.)
// Only ids in the hold/ready/wield-transition family 0x2710..0x27FF are logged; the resolver runs for every animation in the game.
static bool animInteresting(uint32_t id) { return (id & 0xFFFFu) >= 0x2710u && (id & 0xFFFFu) <= 0x27FFu; }

extern "C" void __cdecl HolsterAnimResolve(void* self, uint32_t animId) {
    if (!g_onAnim || !animInteresting(animId)) return;
    if (InterlockedIncrement(&g_animPerId[(animId & 0xFFFFu) - 0x2700u]) > 20) return;
    saferead::beginScope();
    if (!KP_ENABLED(chAnim, KPLOG_INFO) || !takeLine()) return;
    const uintptr_t s = reinterpret_cast<uintptr_t>(self);
    uint8_t b[5] = {0, 0, 0, 0, 0};
    static const int offs[5] = {0xC5, 0xC6, 0xCC, 0xD4, 0xE8};
    for (int i = 0; i < 5; ++i) (void)saferead::readAt(kSiteAnim, s, offs[i], &b[i]);
    KP_LOG(chAnim, "ANIMRES self=%x id=%x style=%u gate=%u cc=%u d4=%u e8=%u tick=%lu", U(self), static_cast<unsigned>(animId & 0xFFFFu), b[0], b[1], b[2],
           b[3], b[4], static_cast<unsigned long>(GetTickCount()));
}

// Live reload of the tune keys (place, place_rot, place_off) when holster.ini changes. Throttled to 500 ms by ReloadPoll; a failed open keeps
// the last good config. Runs on the GobUpdate caller's thread; the busy flag keeps two threads from publishing at once.
static void maybeReload() {
    if (!filewatch::changed(g_poll, "holster.ini", GetTickCount())) return;
    if (InterlockedCompareExchange(&g_reloadBusy, 1, 0) != 0) return;
    FILE* f = fopen("holster.ini", "r");
    if (f) {
        hs::TuneCfg t = g_tuneW;
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char* p = trimIni(line);
            if (*p == 0 || *p == ';' || *p == '#' || *p == '[') continue;
            char* eq = strchr(p, '=');
            if (!eq) continue;
            *eq = 0;
            char* key = trimIni(p);
            char* val = trimIni(eq + 1);
            if (!_stricmp(key, "place") || !_stricmp(key, "place_rot") || !_stricmp(key, "place_off")) (void)hs::applyTuneKey(t, key, val);
        }
        fclose(f);
        publishTune(t);
        KP_LOG(chMain, "RELOAD place=%d rot=(%.1f,%.1f,%.1f) off=(%.3f,%.3f,%.3f)", t.place ? 1 : 0, t.rot[0], t.rot[1], t.rot[2], t.off[0], t.off[1], t.off[2]);
    }
    InterlockedExchange(&g_reloadBusy, 0);
}

// 0x005047B0 behavior update handler (UpdateDependentPosition). C1 recon (.claude/research/holster_c1_postcopy.md): ECX is the CAurBehaviorAttach
// object, NOT the Gob. Child Gob = [this+0x10], attach node N = [this+0x18], name-lookup parent = [this+0x14]. The handler calls
// N->vtbl[+0xC](gob+0xA4, gob+0xB0), which re-copies the node transform into the child. Runs per delivered event: count every call, log the
// first 24 and then every gob_every-th. Logs the child WORLD transform (C3: 0x457510 composes the parent chain and the owner Gob) (+0xA4 vec3, +0xB0 quat w-first) of the real Gob.
extern "C" void __cdecl HolsterGobUpdate(void* beh) {
    if (g_probe) {
        static volatile LONG s_tid = 0;
        if (InterlockedCompareExchange(&s_tid, 1, 0) == 0) KP_LOG(chMain, "TID gobupdate=%lu", static_cast<unsigned long>(GetCurrentThreadId()));
        maybeReload();
    }
    if (!g_onGob) return;
    const LONG n = InterlockedIncrement(&g_gobCalls);
    if (n > 24 && (n % g_gobEvery) != 0) return;
    saferead::beginScope();
    if (!KP_ENABLED(chGob, KPLOG_INFO) || !takeLine()) return;
    const uintptr_t b = reinterpret_cast<uintptr_t>(beh);
    const uintptr_t g = static_cast<uintptr_t>(rdU32(kSiteGob, b, 0x10));
    KP_LOG(chGob, "GOB n=%ld gob=%x parent18=%x pos=(%.3f,%.3f,%.3f) quat=(%.3f,%.3f,%.3f,%.3f) beh=%x par14=%x node=%x", static_cast<long>(n),
           static_cast<unsigned>(g), static_cast<unsigned>(g ? rdU32(kSiteGob, g, 0x18) : 0u), g ? rdF(kSiteGob, g, 0xA4) : 0.f,
           g ? rdF(kSiteGob, g, 0xA8) : 0.f, g ? rdF(kSiteGob, g, 0xAC) : 0.f, g ? rdF(kSiteGob, g, 0xB0) : 0.f, g ? rdF(kSiteGob, g, 0xB4) : 0.f,
           g ? rdF(kSiteGob, g, 0xB8) : 0.f, g ? rdF(kSiteGob, g, 0xBC) : 0.f, U(beh), static_cast<unsigned>(rdU32(kSiteGob, b, 0x14)),
           static_cast<unsigned>(rdU32(kSiteGob, b, 0x18)));
}

// Node name of an attach node, no engine call: H = [N+4]; index = [H+4]; owner = [H+8]; t = [owner+0x4C] & 0x7F; t==2 -> table = [owner+0xB8];
// t==5 -> table = [[owner+0x84]+0xB8]; name = (char*)[table + index*4]. False when any step is unreadable or t is another type.
static bool nodeIsTarget(uintptr_t node) {
    const uintptr_t h = rdU32(kSitePlace, node, 0x04);
    if (!h) return false;
    const uint32_t index = rdU32(kSitePlace, h, 0x04);
    const uintptr_t owner = rdU32(kSitePlace, h, 0x08);
    if (!owner || index > 0xFFFFu) return false;
    const uint32_t t = rdU32(kSitePlace, owner, 0x4C) & 0x7Fu;
    uintptr_t table = 0;
    if (t == 2) table = rdU32(kSitePlace, owner, 0xB8);
    else if (t == 5) { const uintptr_t inner = rdU32(kSitePlace, owner, 0x84); table = inner ? rdU32(kSitePlace, inner, 0xB8) : 0; }
    if (!table) return false;
    const uintptr_t namePtr = rdU32(kSitePlace, table, static_cast<int>(index * 4u));
    if (!namePtr) return false;
    char name[32];
    readTag(kSitePlace, reinterpret_cast<const char*>(namePtr), name, sizeof name);
    return _stricmp(name, g_boneName) == 0;
}

// gameaddr::GobUpdateDependent + 0x44 (H7 site, see hooks.toml H7): runs for EVERY attached behavior right after the engine wrote node N's WORLD pose into gob+0xA4 (vec3) / gob+0xB0 (quat w-first).
// For the target node (g_boneName) it composes the live-tuned placement onto that pose and writes it back. Needs probe=1 AND bone_test=1 AND place=1;
// ch_place only gates the PLACE log lines.
extern "C" void __cdecl HolsterPlace(void* gob, void* node, void* beh) {
    if (!g_probe) return;
    static volatile LONG s_tid = 0;
    if (InterlockedCompareExchange(&s_tid, 1, 0) == 0) KP_LOG(chMain, "TID place=%lu", static_cast<unsigned long>(GetCurrentThreadId()));
    if (!g_boneTest) return;
    static hs::TuneCfg s_tune;
    loadTune(s_tune);
    if (!s_tune.place || !gob || !node) return;
    saferead::beginScope();
    // Verdict cache keyed on node + gob + the node's name handle [N+4], so a freed and reused address pair is re-resolved.
    struct Ent { uintptr_t node, gob, handle; bool isTarget; };
    static Ent s_cache[128];
    static unsigned s_next = 0;
    const uintptr_t g = reinterpret_cast<uintptr_t>(gob), n = reinterpret_cast<uintptr_t>(node);
    const uintptr_t h = rdU32(kSitePlace, n, 0x04);
    if (!h) return;
    bool isTarget = false, hit = false;
    for (const Ent& e : s_cache)
        if (e.node == n && e.gob == g && e.handle == h) { isTarget = e.isTarget; hit = true; break; }
    if (!hit) {
        isTarget = nodeIsTarget(n);
        s_cache[s_next] = {n, g, h, isTarget};
        s_next = (s_next + 1) % 128u;
    }
    if (!isTarget) return;
    const uint32_t vt = rdU32(kSitePlace, g, 0);
    if (g_placeGobvt != 0 && vt != g_placeGobvt) return;
    hs::Pose in;
    if (!saferead::readAt(kSitePlace, g, 0xA4, &in) || !hs::poseSane(in)) return;
    hs::Placement pl;
    for (int i = 0; i < 3; ++i) { pl.pos[i] = s_tune.off[i]; pl.rotDeg[i] = s_tune.rot[i]; }
    const hs::Pose out = hs::applyPlacement(in, pl);
    if (!hs::poseSane(out)) return;
    if (!saferead::probeWrite(kSitePlace, reinterpret_cast<void*>(g + 0xA4), sizeof(hs::Pose))) return;
    memcpy(reinterpret_cast<void*>(g + 0xA4), &out, sizeof(hs::Pose));
    if (!g_onPlace) return;
    const LONG c = InterlockedIncrement(&g_placeCalls);
    if (c > 24 && (c % g_gobEvery) != 0) return;
    if (!KP_ENABLED(chPlace, KPLOG_INFO) || !takeLine()) return;
    KP_LOG(chPlace, "PLACE n=%ld gob=%x node=%x beh=%x gobvt=%x pos0=(%.3f,%.3f,%.3f) quat0=(%.3f,%.3f,%.3f,%.3f) pos1=(%.3f,%.3f,%.3f) quat1=(%.3f,%.3f,%.3f,%.3f)",
           static_cast<long>(c), U(gob), U(node), U(beh), static_cast<unsigned>(vt), in.pos[0], in.pos[1], in.pos[2], in.q.w, in.q.x, in.q.y, in.q.z,
           out.pos[0], out.pos[1], out.pos[2], out.q.w, out.q.x, out.q.y, out.q.z);
}

// bone_test: repoint the rhand (and with bone_test_left the lhand) attach-node literal at g_boneName. Default OFF.
static void applyBoneTest() {
    if (!g_boneTest) return;
    if (!k2addr::get()) { KP_LOG(chMain, "BONETEST refused: unknown exe (no address table)"); return; }
    const uint32_t ptr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(g_boneName));
    struct Site { const char* tag; uintptr_t at; const uint8_t* orig; bool on; };
    const Site sites[2] = {{"rhand", hs::kRhandImm, hs::kRhandOrig, true}, {"lhand", hs::kLhandImm, hs::kLhandOrig, g_boneLeft}};
    for (const Site& s : sites) {
        if (!s.on) continue;
        uint8_t seen[5] = {0, 0, 0, 0, 0};
        const int rc = hs::patchNodeLiteral(s.at, s.orig, ptr, seen);
        if (rc == 1) KP_LOG(chMain, "BONETEST patched %s -> %s", s.tag, g_boneName);
        else KP_LOG(chMain, "BONETEST refused %s: rc=%d seen=%02X %02X %02X %02X %02X", s.tag, rc, seen[0], seen[1], seen[2], seen[3], seen[4]);
    }
}

// Written out literally (create-patch's export scan reads the source text) and listed in exports.def.
extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    readConfig();
    KpLog_Init(api);
    g_poll.throttleMs = 500;
    filewatch::accept(g_poll, "holster.ini");
    char names[96] = "";
    addName(names, sizeof(names), g_onComb, "comb");
    addName(names, sizeof(names), g_onAttach, "attach");
    addName(names, sizeof(names), g_onSwap, "swap");
    addName(names, sizeof(names), g_onAnim, "anim");
    addName(names, sizeof(names), g_onGob, "gob");
    addName(names, sizeof(names), g_onPlace, "place");
    KP_LOG(chMain, "holster 0.1.0-dev loaded (P0 probe, log-only): probe=%d channels=%s max_lines=%ld gob_every=%d table=%s", g_probe ? 1 : 0,
           names[0] ? names : "none", static_cast<long>(g_maxLines), g_gobEvery, k2addr::get() ? "ok" : "none");
    applyBoneTest();
}

// No I/O here (loader lock). The config is read in KPatchInit.
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
