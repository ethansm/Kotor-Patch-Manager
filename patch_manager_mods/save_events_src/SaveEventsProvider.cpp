// save-events (Steam Aspyr build 6A522E71...): neutral owner of the five save / autosave / load / new-game detours that Action Hotbar
// 1.2.1 used for its per-save INI, and provider of "k2.saveevents" v1 (SaveEvents.h). It writes nothing to the engine, does no file
// I/O except the kpatch log, and never blocks; consumers register a kind mask + callback through the registry.
// Design, API and migration: patch_manager_mods/22_hotbar_shared_services.md sections 4.1 / 4.1a. Registry, folder parsing and the
// slow-consumer guard live in SeDispatch.h (host-tested, se_registry_test.cpp).
//
// Hooks (cdecl detours, original bytes still run; the sites, stolen bytes and parameter are copied from the hotbar's hooks toml):
//   0x00534527 SE_OnSaved           after SaveGameInProgress in the main loop: folder = CExoString at R+0x18, R = *(*(AppGlobal)+0x14)
//   0x0053C5AA SE_OnAutosaved       end of DoPCAutosave: folder "000001 - AUTOSAVE"
//   0x0052FD1E SE_OnLoadRequest     LoadGame after the party table / globals: [ebp-0x14] = char* "SAVES:NNNNNN - Name"
//   0x008529E0 SE_OnAutosaveSlotLoad LoadAutosaveSlot entry: folder "000001 - AUTOSAVE"
//   0x008FACA0 SE_OnNewGame         chargen "Play" commit: no folder
// Save root: the "SAVES:" alias is saves/ unless the Steam Cloud byte at 0x00A7EA88 is set, then the CExoString at 0x00A7EABC
// ("cloudsaves/<steam id>/"). Every engine read goes through saferead (site ids 0x7200+); a hook that cannot read what it needs
// logs and dispatches nothing.
// Logging: kpatch.log patch "save-events": channel "evt" = one INFO line per event (kind, folder, cloud, consumers called);
// channel "core" = registrations, slow-consumer WARNs, rejected folders.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "SaveEvents.h"
#include "SeDispatch.h"

KP_LOG_PATCH("save-events");

namespace {

KP_CHANNEL(chEvt, "evt");
KP_CHANNEL(chCore, "core");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("save-events_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// saferead site ids: 0x7200-0x72FF belong to this mod
enum Site : unsigned { S_Req = 0x7201, S_Folder, S_Cloud, S_Str };

constexpr int SaveReq_Holder = 0x14;           // *(AppGlobal)+0x14 = server save/load request R
constexpr int SaveReq_Folder = 0x18;           // R+0x18 = CExoString "SAVES:NNNNNN - Name"
constexpr uintptr_t CloudSavesFlag = 0x00A7EA88, CloudSavesDir = 0x00A7EABC;
const char* const AutosaveFolder = "000001 - AUTOSAVE";

se::Hub g_hub;

// ---- slow-consumer guard clock: microseconds from QueryPerformanceCounter (wraps at 2^32, the dispatcher subtracts) ----------------
uint32_t nowUs(void*) {
    static LARGE_INTEGER freq = [] { LARGE_INTEGER f; if (!QueryPerformanceFrequency(&f) || f.QuadPart <= 0) f.QuadPart = 1000000; return f; }();
    LARGE_INTEGER c; QueryPerformanceCounter(&c);
    const unsigned long long q = (unsigned long long)c.QuadPart, f = (unsigned long long)freq.QuadPart;
    return (uint32_t)((q / f) * 1000000ull + (q % f) * 1000000ull / f);
}
void onSlow(void*, const se::Entry& e, uint32_t us, int32_t kind);

const char* kindName(int32_t k) {
    switch (k) {
        case K2SE_SAVED: return "SAVED";
        case K2SE_AUTOSAVED: return "AUTOSAVED";
        case K2SE_LOAD_REQUEST: return "LOAD_REQUEST";
        case K2SE_NEW_GAME: return "NEW_GAME";
    }
    return "?";
}
void onSlow(void*, const se::Entry& e, uint32_t us, int32_t kind) {
    KP_LOGV(chCore, KPLOG_WARN, "slow consumer owner=%s kind=%s took %u.%03u ms (limit %u ms): save callbacks run on the engine thread and must not block",
            e.owner ? e.owner : "?", kindName(kind), (unsigned)(us / 1000), (unsigned)(us % 1000), (unsigned)(se::SlowThresholdUs / 1000));
}

// ---- the interface --------------------------------------------------------------------------------------------------------------
int32_t KPATCH_CALL apiRegister(int32_t mask, K2SaveCallback fn, void* user, const char* owner) {
    int32_t t = g_hub.disp.add(mask, fn, user, owner);
    KP_LOGV(chCore, t > 0 ? KPLOG_INFO : KPLOG_WARN, "register mask=%d owner=%s -> %d", mask, owner ? owner : "?", t);
    return t;
}
void KPATCH_CALL apiUnregister(int32_t token) {
    bool ok = g_hub.disp.remove(token);
    KP_LOG(chCore, "unregister %d -> %d", token, ok ? 1 : 0);
}
int32_t KPATCH_CALL apiCurrentSaveFolder(char* buf, uint32_t cap) { return g_hub.copyCurrent(buf, cap); }

const K2SaveEventsApi g_api = { sizeof(K2SaveEventsApi), apiRegister, apiUnregister, apiCurrentSaveFolder };

// ---- engine reads ---------------------------------------------------------------------------------------------------------------
// NUL-terminated char* -> out; false when unreadable, empty, or not terminated within cap bytes.
bool readCStr(unsigned site, const char* p, char* out, int cap) {
    if (!p) return false;
    for (int i = 0; i < cap - 1; ++i) {
        if (!saferead::probeRead(site, p + i, 1)) { out[0] = 0; return false; }
        out[i] = p[i];
        if (!out[i]) return i > 0;
    }
    out[0] = 0;
    return false;
}

// raw engine folder ("SAVES:NNNNNN - Name" / "NNNNNN - Name") -> dispatch one event; logs the outcome.
void emitFolder(int32_t kind, const char* raw) {
    unsigned char flag = 0;
    saferead::readAt(S_Cloud, CloudSavesFlag, 0, &flag);
    char rootBuf[160]; rootBuf[0] = 0;
    if (flag) saferead::readExo(S_Folder, CloudSavesDir, rootBuf, sizeof rootBuf, 150);
    int32_t cloud = 0;
    const std::string abs = se::absFolder(flag != 0, rootBuf, raw ? raw : "", &cloud);
    if (abs.empty()) {
        KP_LOGV(chCore, KPLOG_WARN, "%s: save folder not recognised ('%.80s'), not dispatched", kindName(kind), raw ? raw : "?");
        return;
    }
    const int n = g_hub.emit(kind, abs.c_str(), cloud);
    KP_LOG(chEvt, "%s folder='%.150s' cloud=%d consumers=%d", kindName(kind), abs.c_str(), (int)cloud, n);
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) return;
    api->provide(K2SE_IFACE, K2SE_VERSION, &g_api);
    KpLog_Init(api);
    g_hub.disp.timing.now = nowUs;
    g_hub.disp.timing.slow = onSlow;
    KP_LOG(chCore, "provided %s v%d", K2SE_IFACE, K2SE_VERSION);
}

// S1 0x00534527: main loop right after SaveGameInProgress (the only writer of menu saves, quicksaves and timer autosaves).
__declspec(dllexport) void __cdecl SE_OnSaved() {
    saferead::beginScope();   // engine code between hook calls can free memory: drop the cached readable regions
    int app = 0, req = 0;
    char raw[128]; raw[0] = 0;
    if (saferead::readAt(S_Req, gameaddr::AppGlobal, 0, &app) && app) saferead::readAt(S_Req, static_cast<uintptr_t>(app), SaveReq_Holder, &req);
    if (req) saferead::readExo(S_Folder, static_cast<uintptr_t>(req) + SaveReq_Folder, raw, sizeof raw, 120);
    emitFolder(K2SE_SAVED, raw);
}
// S2 0x0053C5AA: end of DoPCAutosave (module-entry autosave; always 000001 - AUTOSAVE, bypasses S1).
__declspec(dllexport) void __cdecl SE_OnAutosaved() { saferead::beginScope(); emitFolder(K2SE_AUTOSAVED, AutosaveFolder); }
// L1 0x0052FD1E: LoadGame after the save's party table and globals loaded; folder = "SAVES:NNNNNN - Name".
__declspec(dllexport) void __cdecl SE_OnLoadRequest(const char* folder) {
    saferead::beginScope();
    char raw[200];
    if (!readCStr(S_Str, folder, raw, sizeof raw)) {
        KP_LOGV(chCore, KPLOG_WARN, "LOAD_REQUEST: folder pointer %p unreadable or implausible, not dispatched", (const void*)folder);
        return;
    }
    emitFolder(K2SE_LOAD_REQUEST, raw);
}
// L1b 0x008529E0: LoadAutosaveSlot (PCAUTOSAVE entries; bypasses LoadGame).
__declspec(dllexport) void __cdecl SE_OnAutosaveSlotLoad() { saferead::beginScope(); emitFolder(K2SE_LOAD_REQUEST, AutosaveFolder); }
// NG 0x008FACA0: chargen "Play" commit (only caller CSWGuiMainCharGen::BeginPlayGame 0x00907340), before the server is re-created.
__declspec(dllexport) void __cdecl SE_OnNewGame() {
    saferead::beginScope();
    const int n = g_hub.emit(K2SE_NEW_GAME, nullptr, 0);
    KP_LOG(chEvt, "NEW_GAME consumers=%d", n);
}

} // extern "C"
