// npc-world-probe 0.1.0-probe (Steam Aspyr build 6A522E71...): the V2a client of the npc-world library (k2.npcworld@1, NpcWorld.h).
// It owns NO hook: area load, creature add, stable-area and tick events and the native Spawn() service all come from npc-world, which
// replaces k1-spawn-probe (same three detour sites). Stage logic is in ProbeCore.h (host-tested); this file only reads the creature
// fields it logs and routes the log.
//
// Ini ".\npc_world_probe.ini" [probe] (all keys optional; see ProbeCore.h for the stage list and keys). Without the file the probe runs
// stages "t26,persist": the old k1-spawn-probe roster (c_drdmouse01 + k1p_native_npc in 204tel) with persistence evidence.
// Logging: kpatch.log patch "npc-world-probe" (kplog.ini), channels core/stage/persist/frame/spawn; fallback file npc-world-probe_log.txt.
// A DLL-only patch has no hook to retry the provider lookup from, so npc-world MUST load first (manifest requires = ["npc-world"]).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../Common/kpatch_api.h"
#include "../Common/KpLog.h"
#include "../Common/SafeRead.h"
#include "../Common/NpcWorld.h"
#include "ProbeCore.h"

KP_LOG_PATCH("npc-world-probe");

namespace {

KP_CHANNEL(chCore, "core");
KP_CHANNEL(chStage, "stage");
KP_CHANNEL(chPersist, "persist");
KP_CHANNEL(chFrame, "frame");
KP_CHANNEL(chSpawn, "spawn");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("npc-world-probe_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

constexpr const char* kVersion = "0.1.0-probe";
constexpr const char* kIniName = ".\\npc_world_probe.ini";

// saferead site ids: 0x7400-0x74FF belong to this mod
enum Site : unsigned { S_Fields = 0x7401 };

static_assert(nwp::EV_LOAD == K2NW_AREA_LOAD && nwp::EV_ADDED == K2NW_CREATURE_ADDED && nwp::EV_STABLE == K2NW_AREA_STABLE && nwp::EV_TICK == K2NW_TICK,
              "ProbeCore event kinds must equal the k2.npcworld kinds");

k2nw::Client g_nw;
nwp::Probe g_probe;
bool g_haveProvider = false;

class DllEnv : public nwp::Env {
public:
    void log(nwp::Chan ch, int level, const char* msg) override {
        const int lv = level >= 1 ? KPLOG_WARN : KPLOG_INFO;
        switch (ch) {
            case nwp::CH_CORE:    KP_LOGV(chCore, lv, "%s", msg); break;
            case nwp::CH_STAGE:   KP_LOGV(chStage, lv, "%s", msg); break;
            case nwp::CH_PERSIST: KP_LOGV(chPersist, lv, "%s", msg); break;
            case nwp::CH_FRAME:   KP_LOGV(chFrame, lv, "%s", msg); break;
            case nwp::CH_SPAWN:   KP_LOGV(chSpawn, lv, "%s", msg); break;
        }
    }
    bool tagPresent(const char* tag) override { return g_nw.tagPresent(tag); }
    uint32_t spawn(const nwp::Row& r) override { return g_nw.spawn(r.resref, r.tag, r.x, r.y, r.z, r.facing); }
    int spawned() override { return g_nw.spawnedCount(); }
    int cap() override { return g_nw.spawnCap(); }
    uint32_t nowUs() override {
        static LARGE_INTEGER freq = [] { LARGE_INTEGER f; if (!QueryPerformanceFrequency(&f) || f.QuadPart <= 0) f.QuadPart = 1000000; return f; }();
        LARGE_INTEGER c; QueryPerformanceCounter(&c);
        const unsigned long long q = (unsigned long long)c.QuadPart, f = (unsigned long long)freq.QuadPart;
        return (uint32_t)((q / f) * 1000000ull + (q % f) * 1000000ull / f);
    }
};
DllEnv g_env;

void KPATCH_CALL onEvent(const K2NwEvent* ev, void*) {
    if (!ev) return;
    nwp::Ev e;
    e.kind = ev->kind;
    e.loadSeq = ev->load_seq;
    e.visitSeq = ev->visit_seq;
    e.salt = ev->visit_salt;
    e.area = ev->area ? ev->area : "";
    if (ev->kind == K2NW_CREATURE_ADDED) {
        e.tag = (K2NW_EVENT_HAS(ev, tag) && ev->tag) ? ev->tag : "";
        e.oid = ev->oid;
        e.x = ev->x; e.y = ev->y; e.z = ev->z;
        e.origin = ev->origin;
        if (ev->creature) {
            saferead::beginScope();   // this DLL has its own readable-region cache
            const uintptr_t c = reinterpret_cast<uintptr_t>(ev->creature);
            unsigned a = 0, b = 0, id = 0;
            e.fieldsOk = saferead::readAt(S_Fields, c, 0x11ac, &a) && saferead::readAt(S_Fields, c, 0x11fc, &b) && saferead::readAt(S_Fields, c, 0x90, &id);
            if (e.fieldsOk) { e.f11ac = a; e.f11fc = b; e.areaId = id; }
        }
    }
    switch (ev->kind) {
        case K2NW_AREA_LOAD:      g_probe.onLoad(e); break;
        case K2NW_CREATURE_ADDED: g_probe.onAdded(e); break;
        case K2NW_AREA_STABLE:    g_probe.onStable(e); break;
        case K2NW_TICK:           g_probe.onTick(e); break;
    }
}

void onConnect(bool ok, const char* why) {
    g_haveProvider = ok;
    KP_LOGV(chCore, ok ? KPLOG_INFO : KPLOG_WARN, "npc-world connect: %s", why ? why : "?");
}

void loadConfig(nwp::Config& cfg) {
    char sec[4096];
    const DWORD n = GetPrivateProfileSectionA("probe", sec, sizeof sec, kIniName);
    if (n == 0) return;
    for (const char* p = sec; *p; p += strlen(p) + 1) {
        const char* eq = strchr(p, '=');
        if (!eq || eq == p) { ++cfg.badKeys; continue; }
        char key[48], val[160];
        const size_t kl = static_cast<size_t>(eq - p);
        if (kl >= sizeof key || strlen(eq + 1) >= sizeof val) { ++cfg.badKeys; continue; }
        memcpy(key, p, kl); key[kl] = 0;
        strcpy(val, eq + 1);
        // trim the value
        size_t vl = strlen(val);
        while (vl && (val[vl - 1] == ' ' || val[vl - 1] == '\t' || val[vl - 1] == '\r')) val[--vl] = 0;
        const char* v = val;
        while (*v == ' ' || *v == '\t') ++v;
        if (!cfg.set(key, v)) KP_LOGV(chCore, KPLOG_WARN, "ini: ignored %s=%s", key, v);
    }
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    if (!KpLog_HasCore()) KpLog_SetFallbackLevel(KPLOG_INFO);
    nwp::Config cfg;
    loadConfig(cfg);
    g_probe.configure(cfg, &g_env);
    KP_LOG(chCore, "npc-world-probe %s loaded: stages=%u (t26=%d pool=%d capacity=%d persist=%d frame=%d) pool_rows=%d capacity_n=%d bad_ini_keys=%d", kVersion,
           cfg.stages, (cfg.stages & nwp::ST_T26) != 0, (cfg.stages & nwp::ST_POOL) != 0, (cfg.stages & nwp::ST_CAPACITY) != 0,
           (cfg.stages & nwp::ST_PERSIST) != 0, (cfg.stages & nwp::ST_FRAME) != 0, cfg.nPool, cfg.capN, cfg.badKeys);
    g_nw.init(api, "npc-world-probe");
    g_nw.onConnect = onConnect;
    g_nw.add(g_probe.mask(), onEvent);
    g_nw.connect();
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinst);
    return TRUE;
}
