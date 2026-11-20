// npc-world (Steam Aspyr build 6A522E71...): neutral owner of the three world detours that the NPC mods share, and provider of
// "k2.npcworld" v1 (NpcWorld.h). Design: project design doc 40 section 12. Registry, one-visit state,
// POP-STABLE latch, tag list and spawn gating live in NwCore.h (host-tested); this file only reads the engine and calls the native
// CreateObject sequence that k1-spawn-probe 0.2.0 introduced (doc 38 sections 13/14).
//
// Hooks (cdecl detours, original bytes still run; sites and stolen bytes copied from k1_spawn_probe_src):
//   0x0055A460 NW_OnLoad       CSWSModule::LoadModuleInProgress entry, ECX = module: a new area load starts (resets the visit, picks the salt)
//   0x00568BD0 NW_OnAddToArea  creature AddToArea entry, ECX = creature, [esp+4] = area pointer value (runs before the engine's own code)
//   0x00533830 NW_OnTick       CServerExoAppInternal::MainLoop entry, ECX = server internal, every tick
// Every engine read goes through saferead (site ids 0x7300+); a field that cannot be read is reported as 0 / "".
// Opt-out: npc_world.ini [world] enabled=0 (hooks stay passive: no events, no salt, no spawns); spawn_cap=N (default 4, max 32);
// verbose=1 logs one line per AddToArea (DEBUG).
// Logging: kpatch.log patch "npc-world": channel "core" = init, registrations, WARNs; "load" = one line per load; "add" = per creature
// add (DEBUG); "stable" = the latch; "spawn" = every spawn step (logged BEFORE it runs, so a crash leaves the last step).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../Common/kpatch_api.h"
#include "../Common/KpLog.h"
#include "../Common/SafeRead.h"
#include "../Common/GameAddr.h"
#include "../Common/NpcWorld.h"
#include "NwCore.h"

KP_LOG_PATCH("npc-world");

namespace {

KP_CHANNEL(chCore, "core");
KP_CHANNEL(chLoad, "load");
KP_CHANNEL(chAdd, "add");
KP_CHANNEL(chStable, "stable");
KP_CHANNEL(chSpawn, "spawn");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("npc-world_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

constexpr const char* kVersion = "0.1.2-dev";
constexpr const char* kIniName = ".\\npc_world.ini";

// saferead site ids: 0x7300-0x73FF belong to this mod
enum Site : unsigned { S_Creature = 0x7301, S_Area, S_TagPtr, S_TagStr, S_Resref, S_Float, S_Oid };

// Engine functions of the CreateObject creature branch (Steam exe 7751F7D8..., doc 38 sec 13/14). Calling conventions per REA; the
// operator new (gameaddr::New, cdecl size) is UNVERIFIED (MSVC operator new is always cdecl) until the NB0 live check.
constexpr uintptr_t kAddrCreatureCtor = 0x00561F30;  // thiscall (p, id, 0), retn 8
constexpr uintptr_t kAddrResrefInit = 0x00710840;    // thiscall (CResRef*, const char*), retn 4
constexpr uintptr_t kAddrLoadTemplate = 0x00572A70;  // thiscall (p, CResRef*, 0), retn 8
constexpr uintptr_t kAddrSetOrient = 0x00543F10;     // thiscall (p, float vec3*), retn 4
constexpr uintptr_t kAddrAddToArea = 0x00568BD0;     // thiscall (p, area*, x, y, z, 1, 1), retn 0x18
constexpr unsigned kCreatureStructSize = 0x1220;
constexpr unsigned kPlaceholderId = 0x7f000000u;

using NewFn = void*(__cdecl*)(unsigned);
using CtorFn = void*(__thiscall*)(void*, unsigned, int);
using ResrefFn = void*(__thiscall*)(void*, const char*);
using LoadFn = int(__thiscall*)(void*, void*, int);
using OrientFn = void(__thiscall*)(void*, const float*);
using AddFn = void(__thiscall*)(void*, void*, float, float, float, int, int);
using DtorFn = void*(__thiscall*)(void*, int);

nw::World g_world;
bool g_inited = false;

uint32_t nowUs(void*) {
    static LARGE_INTEGER freq = [] { LARGE_INTEGER f; if (!QueryPerformanceFrequency(&f) || f.QuadPart <= 0) f.QuadPart = 1000000; return f; }();
    LARGE_INTEGER c; QueryPerformanceCounter(&c);
    const unsigned long long q = (unsigned long long)c.QuadPart, f = (unsigned long long)freq.QuadPart;
    return (uint32_t)((q / f) * 1000000ull + (q % f) * 1000000ull / f);
}

const char* kindName(int32_t k) {
    switch (k) {
        case K2NW_AREA_LOAD: return "AREA_LOAD";
        case K2NW_CREATURE_ADDED: return "CREATURE_ADDED";
        case K2NW_AREA_STABLE: return "AREA_STABLE";
        case K2NW_TICK: return "TICK";
    }
    return "?";
}
void onSlow(void*, const nw::Entry& e, uint32_t us, int32_t kind) {
    KP_LOGV(chCore, KPLOG_WARN, "slow consumer owner=%s kind=%s took %u.%03u ms (limit %u ms): npcworld callbacks run on the engine thread and must not block",
            e.owner ? e.owner : "?", kindName(kind), (unsigned)(us / 1000), (unsigned)(us % 1000), (unsigned)(nw::SlowThresholdUs / 1000));
}

// ---- engine reads ---------------------------------------------------------------------------------------------------------------
// Copies the NUL-terminated string at p into out (at most cap-1 bytes, each byte probed). Non-printable bytes become '.'.
// False with out = "" when p is 0 or its first byte is unreadable. A string cut short by an unreadable byte is kept.
bool readCStr(unsigned site, uintptr_t p, char* out, int cap, int* nonPrintable = nullptr) {
    out[0] = 0;
    if (nonPrintable) *nonPrintable = 0;
    if (p == 0 || cap < 2) return false;
    if (!saferead::probeRead(site, reinterpret_cast<const void*>(p), 1)) return false;
    int n = 0;
    while (n < cap - 1) {
        if (!saferead::probeRead(site, reinterpret_cast<const void*>(p + n), 1)) break;
        const unsigned char ch = *reinterpret_cast<const unsigned char*>(p + n);
        if (ch == 0) break;
        if (ch < 0x20 || ch >= 0x7f) { if (nonPrintable) ++*nonPrintable; }
        out[n++] = (ch >= 0x20 && ch < 0x7f) ? static_cast<char>(ch) : '.';
    }
    out[n] = 0;
    return true;
}

// The provider's memory reader for nw::readCreatureTag: every byte is probed through saferead.
struct DllMem : nw::MemReader {
    bool u32(uintptr_t addr, unsigned* v) override { return saferead::readAt(S_TagPtr, addr, 0, v); }
    bool cstr(uintptr_t p, char* out, int cap, int* nonPrintable) override { return readCStr(S_TagStr, p, out, cap, nonPrintable); }
};

// 16-byte resref at p (not necessarily NUL-terminated) -> out (17 bytes). Printable ASCII prefix only; "" when unreadable.
void readResref16(uintptr_t p, char* out) {
    out[0] = 0;
    if (!p || !saferead::probeRead(S_Resref, reinterpret_cast<const void*>(p), 16)) return;
    const unsigned char* rb = reinterpret_cast<const unsigned char*>(p);
    int n = 0;
    while (n < 16 && rb[n] >= 0x20 && rb[n] < 0x7f) { out[n] = static_cast<char>(rb[n]); ++n; }
    out[n] = 0;
}

void logStable(const nw::World& w);

void ensureInit() {
    if (g_inited) return;
    g_inited = true;
    g_world.enabled = GetPrivateProfileIntA("world", "enabled", 1, kIniName) != 0;
    int cap = static_cast<int>(GetPrivateProfileIntA("world", "spawn_cap", nw::DefaultSpawnCap, kIniName));
    if (cap < 0) cap = 0;
    if (cap > nw::HardSpawnCap) cap = nw::HardSpawnCap;
    g_world.spawnCap = cap;
    const bool verbose = GetPrivateProfileIntA("world", "verbose", 0, kIniName) != 0;
    if (!KpLog_HasCore()) KpLog_SetFallbackLevel(verbose ? KPLOG_DEBUG : KPLOG_INFO);
    g_world.disp.timing.now = nowUs;
    g_world.disp.timing.slow = onSlow;
    g_world.preStable = logStable;
    KP_LOG(chCore, "npc-world %s loaded (events+spawn %s, spawn_cap=%d, verbose=%d)", kVersion, g_world.enabled ? "enabled" : "disabled by ini",
           (int)g_world.spawnCap, verbose ? 1 : 0);
}

void logStable(const nw::World& w) {
    KP_LOG(chStable, "STABLE area=%s load=%d visit=%d tags=%d pop=%d%s", w.area(), (int)w.loadSeq(), (int)w.visitSeq(), w.seenTags(), (int)w.popEngine(),
           w.seenOverflow() ? " (tag table overflowed)" : "");
}

// ---- native spawn (the CreateObject creature branch, minus the optional walkmesh snap and appear message) --------------------------
// Every step is logged BEFORE it runs. Returns the new object id, or 0 when a step failed.
uint32_t engineSpawn(void*, void* area, const char* resref, float x, float y, float z, float facing) {
    KP_LOG(chSpawn, "begin resref=%s area=%08lx at %.1f,%.1f,%.1f", resref, (unsigned long)reinterpret_cast<uintptr_t>(area), x, y, z);
    KP_LOG(chSpawn, "step new");
    void* p = reinterpret_cast<NewFn>(gameaddr::New)(kCreatureStructSize);
    if (!p) { KP_LOGV(chSpawn, KPLOG_WARN, "fail: new returned null"); return 0; }
    KP_LOG(chSpawn, "step ctor p=%08lx", (unsigned long)reinterpret_cast<uintptr_t>(p));
    reinterpret_cast<CtorFn>(kAddrCreatureCtor)(p, kPlaceholderId, 0);
    alignas(8) char cres[32];
    memset(cres, 0, sizeof cres);
    KP_LOG(chSpawn, "step resref");
    reinterpret_cast<ResrefFn>(kAddrResrefInit)(cres, resref);
    KP_LOG(chSpawn, "step load");
    const int loaded = reinterpret_cast<LoadFn>(kAddrLoadTemplate)(p, cres, 0) & 0xFF;
    if (!loaded) {
        KP_LOGV(chSpawn, KPLOG_WARN, "fail: load template %s; destroying", resref);
        void** vtbl = *reinterpret_cast<void***>(p);
        reinterpret_cast<DtorFn>(vtbl[0])(p, 1);
        return 0;
    }
    const nw::Vec3 o = nw::orientationFromFacing(facing);
    const float vec[3] = {o.x, o.y, o.z};
    KP_LOG(chSpawn, "step orient");
    reinterpret_cast<OrientFn>(kAddrSetOrient)(p, vec);
    KP_LOG(chSpawn, "step add");
    reinterpret_cast<AddFn>(kAddrAddToArea)(p, area, x, y, z, 1, 1);   // re-enters NW_OnAddToArea (origin LIB)
    unsigned id = 0;
    saferead::readAt(S_Oid, reinterpret_cast<uintptr_t>(p), 4, &id);
    KP_LOG(chSpawn, "ok resref=%s id=%08x", resref, id);
    return id ? id : kPlaceholderId;
}

// ---- the interface --------------------------------------------------------------------------------------------------------------
int32_t KPATCH_CALL apiRegister(int32_t mask, K2NwCallback fn, void* user, const char* owner) {
    const int32_t t = g_world.disp.add(mask, fn, user, owner);
    KP_LOGV(chCore, t > 0 ? KPLOG_INFO : KPLOG_WARN, "register mask=%d owner=%s -> %d", mask, owner ? owner : "?", t);
    return t;
}
void KPATCH_CALL apiUnregister(int32_t token) {
    const bool ok = g_world.disp.remove(token);
    KP_LOG(chCore, "unregister %d -> %d", token, ok ? 1 : 0);
}
uint32_t KPATCH_CALL apiVisitSalt() { return g_world.visitSalt(); }
int32_t KPATCH_CALL apiVisitSeq() { return g_world.visitSeq(); }
int32_t KPATCH_CALL apiCurrentArea(char* buf, uint32_t cap) { return g_world.currentArea(buf, cap); }
int32_t KPATCH_CALL apiTagPresent(const char* tag) { return g_world.tagPresent(tag) ? 1 : 0; }
uint32_t KPATCH_CALL apiSpawn(const char* resref, const char* tag, float x, float y, float z, float facing) {
    saferead::beginScope();
    uint32_t oid = 0;
    const nw::SpawnResult r = g_world.spawn(resref, tag, x, y, z, facing, &oid);
    if (r != nw::SpawnResult::Ok)
        KP_LOGV(chSpawn, KPLOG_WARN, "refused resref=%s tag=%s reason=%d", resref ? resref : "?", tag ? tag : "", (int)r);
    return oid;
}
int32_t KPATCH_CALL apiSpawnedCount() { return g_world.spawnedCount(); }
int32_t KPATCH_CALL apiSpawnCap() { return g_world.effectiveCap(); }

const K2NpcWorldApi g_api = { sizeof(K2NpcWorldApi), apiRegister, apiUnregister, apiVisitSalt, apiVisitSeq, apiCurrentArea, apiTagPresent,
                              apiSpawn, apiSpawnedCount, apiSpawnCap };

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) return;
    api->provide(K2NW_IFACE, K2NW_VERSION, &g_api);
    KpLog_Init(api);
    g_world.eng.spawn = engineSpawn;
    ensureInit();
    KP_LOG(chCore, "provided %s v%d", K2NW_IFACE, K2NW_VERSION);
}

// H2 0x0055A460: LoadModuleInProgress entry (ECX = module).
__declspec(dllexport) void __cdecl NW_OnLoad(void* module) {
    saferead::beginScope();   // engine code between hook calls can free memory: drop the cached readable regions
    g_world.eng.spawn = engineSpawn;
    ensureInit();
    g_world.onLoad(GetTickCount() ^ (nowUs(nullptr) * 2654435761u));
    KP_LOG(chLoad, "LOAD n=%d module=%08lx salt=%08x enabled=%d", (int)g_world.loadSeq(), (unsigned long)reinterpret_cast<uintptr_t>(module),
           (unsigned)g_world.visitSalt(), g_world.enabled ? 1 : 0);
}

// H3 0x00568BD0: creature AddToArea entry (ECX = creature; [esp+4] = area pointer value).
__declspec(dllexport) void __cdecl NW_OnAddToArea(void* creature, void* areaArg) {
    saferead::beginScope();
    g_world.eng.spawn = engineSpawn;
    ensureInit();
    if (!g_world.enabled) return;
    const uintptr_t c = reinterpret_cast<uintptr_t>(creature);
    const uintptr_t a = reinterpret_cast<uintptr_t>(areaArg);

    nw::CreatureInfo ci;
    ci.creature = creature;
    ci.areaPtr = areaArg;
    unsigned id = 0;
    if (saferead::readAt(S_Creature, c, 4, &id)) ci.oid = id;
    saferead::readAt(S_Float, c, 0x94, &ci.x);
    saferead::readAt(S_Float, c, 0x98, &ci.y);
    saferead::readAt(S_Float, c, 0x9c, &ci.z);

    char tagStr[nw::TagChars + 1];
    DllMem mem;
    const nw::TagVia via = nw::readCreatureTag(mem, c, tagStr);
    ci.tag = tagStr;

    char areaRes[17];
    readResref16(a ? a + 0x110 : 0, areaRes);
    ci.areaRes = areaRes;

    KP_LOGV(chAdd, KPLOG_DEBUG, "ADD c=%08lx id=%08x tag=\"%s\" via=%s area=%s pos=%.1f,%.1f,%.1f origin=%s", (unsigned long)c, id, tagStr,
            via == nw::TagVia18 ? "18" : via == nw::TagVia24 ? "24" : "none", areaRes, ci.x, ci.y, ci.z, g_world.spawning() ? "lib" : "engine");
    const bool wasStable = g_world.stable();
    g_world.onCreatureAdded(ci);
    if (wasStable && !g_world.stable())
        KP_LOG(chStable, "area change without load event, gate re-armed area=%s", areaRes);
}

// H1 0x00533830: MainLoop entry (ECX = server internal), every tick.
__declspec(dllexport) void __cdecl NW_OnTick(void* server) {
    (void)server;
    if (!g_inited || !g_world.enabled) return;
    saferead::beginScope();
    g_world.onTick();
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinst);
    return TRUE;
}
