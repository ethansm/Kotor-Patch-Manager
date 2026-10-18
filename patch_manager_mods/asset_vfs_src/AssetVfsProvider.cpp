#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <atomic>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "NpcWorld.h"
#include "AssetVfsCore.h"

KP_LOG_PATCH("asset-vfs");

namespace {

KP_CHANNEL(chCore, "core");
KP_CHANNEL(chRes, "res");
KP_CHANNEL(ch2Da, "2da");
KP_CHANNEL(chAudio, "audio");
KP_CHANNEL(chChar, "char");

const bool g_fallbackSet = (KpLog_SetFallbackFile("asset-vfs_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

assetvfs::EngineState g_state;
k2nw::Client g_nw;

enum Site : unsigned {
    S_ExoRead = 0x7401,
    S_OpenStream = 0x7404,
    S_GetKeyEntry = 0x7410,
    S_Service = 0x7420,
    S_Release = 0x7425,
    S_2Da = 0x7430
};

// Mss32.dll exports. _AIL_file_callbacks reads the four current file callbacks into four out-pointers;
// _AIL_set_file_callbacks is their only writer (four NULLs restore Miles' defaults).
typedef void (__stdcall *AilGetFileCallbacksFn)(assetvfs::AilOpenCb*, assetvfs::AilCloseCb*,
                                                assetvfs::AilSeekCb*, assetvfs::AilReadCb*);
typedef void (__stdcall *AilSetFileCallbacksFn)(assetvfs::AilOpenCb, assetvfs::AilCloseCb,
                                                assetvfs::AilSeekCb, assetvfs::AilReadCb);

AilGetFileCallbacksFn g_ailGet = nullptr;
AilSetFileCallbacksFn g_ailSet = nullptr;
assetvfs::MilesChain g_miles;

uint32_t __stdcall MilesFileOpen(const char* filename, uint32_t* file_handle) {
    return g_miles.open(filename, file_handle);
}

void __stdcall MilesFileClose(uint32_t handle) {
    g_miles.close(handle);
}

int32_t __stdcall MilesFileSeek(uint32_t handle, int32_t offset, uint32_t type) {
    return g_miles.seek(handle, offset, type);
}

uint32_t __stdcall MilesFileRead(uint32_t handle, void* buffer, uint32_t bytes) {
    return g_miles.read(handle, buffer, bytes);
}

void milesGetCallbacks(assetvfs::MilesCallbackSet* out) {
    if (g_ailGet && out) {
        g_ailGet(&out->open, &out->close, &out->seek, &out->read);
    }
}

void milesSetCallbacks(const assetvfs::MilesCallbackSet& set) {
    if (g_ailSet) {
        g_ailSet(set.open, set.close, set.seek, set.read);
    }
}

void onMilesNotice(int code) {
    if (code == assetvfs::kMilesNoticeChanged) {
        KP_WARN("Miles file callbacks were replaced by another module; chaining to the new set and re-installing ours");
    } else if (code == assetvfs::kMilesNoticeNoDefaults) {
        KP_ERR("Miles file callbacks are NULL even after resetting to defaults; VFS streams disabled (engine path)");
    }
}

void initMilesChain() {
    assetvfs::MilesCallbackSet ours;
    ours.open = MilesFileOpen;
    ours.close = MilesFileClose;
    ours.seek = MilesFileSeek;
    ours.read = MilesFileRead;
    g_miles.bind(&g_state, ours, milesGetCallbacks, milesSetCallbacks, onMilesNotice);
}

// Resolves the Mss32 exports once. False = Miles unavailable (the stream is left to the engine).
bool resolveMilesExports() {
    if (g_ailGet && g_ailSet) return true;
    HMODULE hMss = GetModuleHandleA("Mss32.dll");
    if (!hMss) {
        hMss = LoadLibraryA("Mss32.dll");
    }
    if (!hMss) return false;
    g_ailGet = reinterpret_cast<AilGetFileCallbacksFn>(GetProcAddress(hMss, "_AIL_file_callbacks@16"));
    g_ailSet = reinterpret_cast<AilSetFileCallbacksFn>(GetProcAddress(hMss, "_AIL_set_file_callbacks@16"));
    if (!g_ailGet || !g_ailSet) {
        g_ailGet = nullptr;
        g_ailSet = nullptr;
        return false;
    }
    return true;
}

// Called on every VFS stream match: chains/installs our callbacks. False = do not rewrite the stream.
bool ensureMilesChain() {
    static bool logged = false;
    if (!resolveMilesExports()) {
        static bool warnedExports = false;
        if (!warnedExports) {
            warnedExports = true;
            KP_ERR("Mss32.dll file-callback exports not found; VFS streams disabled (engine path)");
        }
        return false;
    }
    if (!g_miles.ensure()) return false;
    if (!logged) {
        logged = true;
        KP_LOG(chAudio, "installed Miles file callbacks (chained to the previous set)");
    }
    return true;
}

uint8_t* engineAlloc(uint32_t size) {
    return static_cast<uint8_t*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size));
}

int engineCallOnServiced(void* pRes) {
    if (!pRes) return 0;
    void*** pVtbl = reinterpret_cast<void***>(pRes);
    if (!pVtbl || !*pVtbl) return 0;
    void** vtbl = *pVtbl;
    using ServicedFn = int(__thiscall*)(void*);
    ServicedFn fn = reinterpret_cast<ServicedFn>(vtbl[4]);
    if (fn) {
        return fn(pRes);
    }
    return 1;
}

bool readExoString(uintptr_t exoAddr, char* out, size_t cap) {
    if (!exoAddr || !out || cap == 0) return false;
    out[0] = '\0';
    uintptr_t strPtr = 0;
    uint32_t len = 0;
    if (!saferead::readAt(S_ExoRead, exoAddr, 0, &strPtr)) return false;
    if (!saferead::readAt(S_ExoRead, exoAddr, 4, &len)) return false;
    if (!strPtr || len == 0) return false;
    if (len >= cap) len = static_cast<uint32_t>(cap - 1);
    if (!saferead::probeRead(S_ExoRead, reinterpret_cast<const void*>(strPtr), len)) return false;
    memcpy(out, reinterpret_cast<const void*>(strPtr), len);
    out[len] = '\0';
    return true;
}

// Pool-full / served-reject notices raised by the core (called with the VFS lock released).
void onVfsNotice(int code, const char* what, const char* owner) {
    if (code == assetvfs::kNoticeServedReject) {
        KP_WARN("REGISTER rejected: '%s' already served to the engine (owner=%s)", what, owner);
    } else if (code == assetvfs::kNoticePoolFull) {
        KP_WARN("REGISTER failed: %s asset pool full (owner=%s); further failures in this pool are silent", what, owner);
    } else if (code == assetvfs::kNoticeRewriteIgnored) {
        KP_WARN("stream filter K2VFS_STREAM_REWRITE is disabled; treated as PASS (owner=%s)", owner);
    } else if (code == assetvfs::kNoticeStemConflict) {
        KP_WARN("stream filter MEMORY claim rejected: stem already owned by a different path '%s' (owner=%s)", what, owner);
    } else if (code == assetvfs::kNoticeOpenFail) {
        static std::atomic<uint32_t> fails{0};
        const uint32_t n = fails.fetch_add(1, std::memory_order_relaxed);
        if (n < 20) { // first 20 only; the reason is "cursor-full" or "bad-spec"
            KP_WARN("OPEN failed: '%s' (%s)", what, owner);
        }
    } else if (code == assetvfs::kNoticeTwoDaFull) {
        KP_WARN("REGISTER 2DA cells: table full, %s cell(s) dropped (owner=%s); those rows will be incomplete", what, owner);
    } else if (code == assetvfs::kNoticeServedKept) {
        KP_WARN("unregistered owner=%s: %s served VFS record(s) keep their data copy (engine still references them)", owner, what);
    }
}

const char* ownerOrUnknown(const char* owner) {
    return (owner && owner[0]) ? owner : "?";
}

// `res` channel lines. The core only calls these when the log gate (prefix test + budget) allows the line,
// so no formatting happens for filtered names.
void onMissEvent(const char* name, uint16_t resType, int outcome) {
    const char* what = outcome == assetvfs::kMissServe ? "serve"
                     : (outcome == assetvfs::kMissObserve ? "observe" : "pass");
    KP_LOG(chRes, "MISS %s %u %s", name, static_cast<unsigned>(resType), what);
}

void onServeEvent(const char* name, uint16_t resType, uint32_t size, int ret) {
    KP_LOG(chRes, "SERVE %s %u %u ret=%d", name, static_cast<unsigned>(resType), static_cast<unsigned>(size), ret);
}

// Returns 1 when registered, 0 otherwise (invalid entry, already served, or pool full).
int32_t KPATCH_CALL apiRegisterAsset(const K2VfsAssetEntry* entry, const char* owner) {
    if (!entry) return 0;
    int res = g_state.registerAsset(entry->resref, entry->res_type, entry->data, entry->size,
                                    entry->flags, false, ownerOrUnknown(owner));
    if (res < 0) {
        KP_WARN("RegisterAsset '%s' type=%u failed rc=%d (owner=%s)",
                entry->resref ? entry->resref : "(null)", static_cast<unsigned>(entry->res_type), res, ownerOrUnknown(owner));
    }
    return res >= 0 ? 1 : 0;
}

// Returns the number registered; callers must compare against `count` (count != returned = partial).
int32_t KPATCH_CALL apiRegisterAssets(const K2VfsAssetEntry* entries, uint32_t count, const char* owner) {
    if (!entries || count == 0) return 0;
    int32_t okCount = 0;
    for (uint32_t i = 0; i < count; ++i) {
        if (g_state.registerAsset(entries[i].resref, entries[i].res_type, entries[i].data, entries[i].size,
                                  entries[i].flags, false, ownerOrUnknown(owner)) >= 0) {
            okCount++;
        }
    }
    if (okCount != static_cast<int32_t>(count)) {
        KP_WARN("RegisterAssets: only %d of %u registered (owner=%s)",
                static_cast<int>(okCount), static_cast<unsigned>(count), ownerOrUnknown(owner));
    }
    return okCount;
}

int32_t KPATCH_CALL apiRegister2DaCells(const K2Vfs2DaCell* cells, uint32_t count, const char* owner) {
    return g_state.addTwoDaCells(cells, count, owner);
}

int32_t KPATCH_CALL apiHasAsset(const char* resref, uint16_t res_type, uint16_t flags) {
    if (!resref || !resref[0]) return 0;
    return g_state.hasAsset(resref, res_type, flags) ? 1 : 0;
}

int32_t KPATCH_CALL apiRegisterResManMissHandler(K2VfsResManMissFn fn, void* user, const char* owner) {
    return g_state.registerMissHandler(fn, user, owner);
}

void KPATCH_CALL apiUnregisterResManMissHandler(int32_t token) {
    g_state.unregisterMissHandler(token);
}

int32_t KPATCH_CALL apiRegisterStreamFilter(K2VfsStreamFilterFn fn, void* user, const char* owner) {
    return g_state.registerStreamFilter(fn, user, owner);
}

void KPATCH_CALL apiUnregisterStreamFilter(int32_t token) {
    g_state.unregisterStreamFilter(token);
}

const K2AssetVfsApi g_vfsApi = {
    sizeof(K2AssetVfsApi),
    apiRegisterAsset,
    apiRegisterAssets,
    apiRegister2DaCells,
    apiHasAsset,
    apiRegisterResManMissHandler,
    apiUnregisterResManMissHandler,
    apiRegisterStreamFilter,
    apiUnregisterStreamFilter
};

int32_t KPATCH_CALL apiRegisterCharacter(const K2CharacterDefinition* def, const char* owner) {
    const int32_t rc = g_state.chars.registerDef(def, owner);
    KP_LOG(chChar, "REGISTER %s owner=%s rc=%d",
           (def && def->character_id) ? def->character_id : "(null)", ownerOrUnknown(owner), static_cast<int>(rc));
    return rc;
}

int32_t KPATCH_CALL apiHasCharacter(const char* character_id) {
    return g_state.chars.has(character_id);
}

int32_t KPATCH_CALL apiGetCharacter(const char* character_id, K2CharacterDefinition* out_def) {
    return g_state.chars.get(character_id, out_def);
}

int32_t KPATCH_CALL apiIsCharacterAppearance(uint32_t appearance_id) {
    return g_state.chars.isAppearance(appearance_id);
}

uint32_t KPATCH_CALL apiSpawn(const char* character_id, const char* area_resref, float x, float y, float z, float facingRadians) {
    (void)area_resref;
    g_nw.poll();
    const k2char::Catalog::Entry* entry = g_state.chars.getEntry(character_id);
    if (!entry) {
        KP_WARN("SPAWN %s: not registered", character_id ? character_id : "(null)");
        return 0;
    }
    const uint32_t oid = g_nw.spawn(entry->default_utc, entry->default_tag, x, y, z, facingRadians);
    KP_LOG(chChar, "SPAWN %s utc=%s tag=%s -> oid=0x%X", character_id, entry->default_utc, entry->default_tag,
           static_cast<unsigned>(oid));
    return oid;
}

const K2CharacterLibApi g_charApi = {
    sizeof(K2CharacterLibApi),
    apiRegisterCharacter,
    apiHasCharacter,
    apiGetCharacter,
    apiIsCharacterAppearance,
    apiSpawn
};

// H1 uses skip_original_bytes = true: the stolen `MOV DWORD [ecx],0` (zero *ppTable) does NOT run,
// so every path returning 0 must zero *ppTable itself, through this one exit helper.
// (The native tail at 0x712457 reached on 0 still zeroes *ppEntry and EAX.)
int missExit(void** ppTable, int rc) {
    if (rc == 0 && ppTable) {
        *ppTable = nullptr;
    }
    return rc;
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) return;

    api->provide(K2_ASSETVFS_IFACE, K2_ASSETVFS_VERSION, &g_vfsApi);
    api->provide(K2_CHARLIB_IFACE, K2_CHARLIB_VERSION, &g_charApi);
    KpLog_Init(api);
    g_state.noticeFn = onVfsNotice;
    initMilesChain();

    // [vfs] mode=serve|observe|off (default observe), legacy enabled=0 = off, log_prefix=a,b (empty = counters
    // only), log_all=1 logs every name. Parsed by the engine-free core (host-tested).
    char modeBuf[32] = {0};
    char prefixBuf[160] = {0};
    GetPrivateProfileStringA("vfs", "mode", "", modeBuf, sizeof(modeBuf), ".\\asset_vfs.ini");
    GetPrivateProfileStringA("vfs", "log_prefix", assetvfs::defaultLogPrefixes(), prefixBuf, sizeof(prefixBuf),
                             ".\\asset_vfs.ini");
    const int enabledIni = GetPrivateProfileIntA("vfs", "enabled", 1, ".\\asset_vfs.ini");
    const int logAllIni = GetPrivateProfileIntA("vfs", "log_all", 0, ".\\asset_vfs.ini");
    const assetvfs::VfsConfig cfg = assetvfs::VfsConfig::fromIni(modeBuf, enabledIni, prefixBuf, logAllIni);
    g_state.applyConfig(cfg);
    g_state.missEventFn = onMissEvent;
    g_state.serveEventFn = onServeEvent;

    g_nw.init(api, "asset-vfs");
    g_nw.connect();

    KP_LOG(chCore, "provided %s v%d and %s v%d (mode=%s enabled=%d log_prefixes=%u log_all=%d)",
           K2_ASSETVFS_IFACE, K2_ASSETVFS_VERSION,
           K2_CHARLIB_IFACE, K2_CHARLIB_VERSION,
           assetvfs::modeName(cfg.mode), g_state.enabled ? 1 : 0,
           static_cast<unsigned>(cfg.nPrefixes), cfg.logAll ? 1 : 0);
    if (cfg.modeInvalid) {
        KP_WARN("asset_vfs.ini [vfs] mode='%s' is not serve|observe|off; using observe", modeBuf);
    }
    if (cfg.mode == assetvfs::kModeObserve) {
        KP_LOG(chCore, "OBSERVE mode: hooks log what they would do and leave engine state untouched (set mode=serve to serve)");
    }
}

__declspec(dllexport) int __cdecl VFS_OnGetKeyEntryMiss(uintptr_t ebp) {
    if (!ebp) return 0;

    // Counter summary every 30000 H1 calls (call-count based: KP_LOG_EVERY is time based).
    const uint32_t h1n = g_state.counters.h1Calls.fetch_add(1, std::memory_order_relaxed) + 1;
    if ((h1n % 30000u) == 0) {
        KP_LOG(chCore, "COUNTERS h1=%u misses=%u served=%u observe=%u streams=%u splices=%u",
               static_cast<unsigned>(h1n),
               static_cast<unsigned>(g_state.counters.misses.load(std::memory_order_relaxed)),
               static_cast<unsigned>(g_state.counters.served.load(std::memory_order_relaxed)),
               static_cast<unsigned>(g_state.counters.observe.load(std::memory_order_relaxed)),
               static_cast<unsigned>(g_state.counters.streams.load(std::memory_order_relaxed)),
               static_cast<unsigned>(g_state.counters.splices.load(std::memory_order_relaxed)));
    }

    // Zero-handler fast path (every ordinary resource lookup that misses the key table lands here): when
    // no miss handler is active and the static registry holds no ResMan record of this type, nothing can
    // be served, so zero *ppTable and return 0 with PLAIN reads - [ebp+0xC] resType and [ebp+0x10] ppTable
    // are the engine's own live frame (the native code writes *ppTable unconditionally anyway), so no
    // SafeRead and no lock are needed.
    {
        const uint32_t fastType = *reinterpret_cast<const uint32_t*>(ebp + 0x0C);
        void** fastTable = *reinterpret_cast<void***>(ebp + 0x10);
        if (fastTable && g_state.missCannotHit(static_cast<uint16_t>(fastType & 0xFFFF))) {
            *fastTable = nullptr;
            return 0;
        }
    }

    saferead::beginScope();

    // If ppTable itself cannot be read or written nothing can be zeroed: return 0 and let the native
    // tail run (it zeroes ppEntry and EAX; there is no other table write we could safely make).
    void** ppTable = nullptr;
    if (!saferead::readAt(S_GetKeyEntry, ebp, 0x10, &ppTable) || !ppTable) return 0;
    if (!saferead::probeWrite(S_GetKeyEntry, ppTable, sizeof(void*))) return 0;

    if (!g_state.enabled) return missExit(ppTable, 0);

    uintptr_t resRefPtr = 0;
    uint32_t resType32 = 0;
    void** ppEntry = nullptr;

    if (!saferead::readAt(S_GetKeyEntry, ebp, 0x08, &resRefPtr)) return missExit(ppTable, 0);
    if (!saferead::readAt(S_GetKeyEntry, ebp, 0x0C, &resType32)) return missExit(ppTable, 0);
    if (!saferead::readAt(S_GetKeyEntry, ebp, 0x14, &ppEntry)) return missExit(ppTable, 0);

    if (!resRefPtr || !ppEntry) return missExit(ppTable, 0);
    if (!saferead::probeRead(S_GetKeyEntry, reinterpret_cast<const void*>(resRefPtr), 16)) return missExit(ppTable, 0);

    const char* rawResRef16 = reinterpret_cast<const char*>(resRefPtr);
    uint16_t resType = static_cast<uint16_t>(resType32 & 0xFFFF);

    // Core zeroes *ppTable on a miss as well; missExit keeps the contract explicit at the hook boundary.
    return missExit(ppTable, g_state.handleGetKeyEntryMiss(rawResRef16, resType, ppTable, ppEntry));
}

__declspec(dllexport) int __cdecl VFS_OnServiceFromImage(void* thisPtr, uintptr_t ebp) {
    (void)thisPtr;
    if (!g_state.enabled || g_state.observe || !ebp) return 0; // observe: never touch the CRes
    saferead::beginScope();

    void* pRes = nullptr;
    if (!saferead::readAt(S_Service, ebp, 0x08, &pRes) || !pRes) {
        return 0;
    }

    uint32_t packedId = 0;
    if (!saferead::readAt(S_Service, reinterpret_cast<uintptr_t>(pRes), static_cast<int>(assetvfs::kResOffPackedId), &packedId)) {
        return 0;
    }
    if (!vfsreg::isVfsPackedId(packedId)) {
        return 0;
    }

    return g_state.handleServiceFromImage(reinterpret_cast<uint8_t*>(pRes), engineAlloc, engineCallOnServiced);
}

// H5: CExoResMan::Release at the archive-source release block (hooks toml H5). Returns 1 for a VFS resource so the
// engine skips the FUN_00712d70 lookup that would return NULL for it (NULL read on the first served UTC).
__declspec(dllexport) int __cdecl VFS_OnResRelease(uintptr_t ebp) {
    if (!ebp) return 0;
    saferead::beginScope();
    void* pRes = nullptr;
    if (!saferead::readAt(S_Release, ebp, 0x08, &pRes) || !pRes) return 0;
    uint32_t packedId = 0;
    if (!saferead::readAt(S_Release, reinterpret_cast<uintptr_t>(pRes), static_cast<int>(assetvfs::kResOffPackedId), &packedId)) {
        return 0;
    }
    return assetvfs::skipEngineSourceRelease(packedId) ? 1 : 0;
}

__declspec(dllexport) void __cdecl VFS_On2DaServiced(void* pRes2Da) {
    if (!g_state.enabled || !pRes2Da) return;
    saferead::beginScope();

    uintptr_t res = reinterpret_cast<uintptr_t>(pRes2Da);

    // A 2DA served from the VFS (packedId 0x40FC....) is the provider's own data: never splice it.
    uint32_t packedId = 0;
    if (!saferead::readAt(S_2Da, res, static_cast<int>(assetvfs::kResOffPackedId), &packedId)) {
        return;
    }
    if (vfsreg::isVfsPackedId(packedId)) {
        return;
    }

    uint32_t loaded = 0;
    if (!saferead::readAt(S_2Da, res, static_cast<int>(assetvfs::kRes2DaOffLoaded), &loaded) || loaded != 0) {
        return;
    }

    uint8_t* oldData = nullptr;
    uint32_t oldSize = 0;
    uint8_t* keyEntry = nullptr;

    if (!saferead::readAt(S_2Da, res, static_cast<int>(assetvfs::kResOffData), &oldData) || !oldData) {
        return;
    }
    if (!saferead::readAt(S_2Da, res, static_cast<int>(assetvfs::kResOffSize), &oldSize) || oldSize == 0) {
        return;
    }
    if (!saferead::readAt(S_2Da, res, static_cast<int>(assetvfs::kResOffKeyEntry), &keyEntry) || !keyEntry) {
        return;
    }

    char resref[17] = {0};
    if (!saferead::probeRead(S_2Da, keyEntry, 16)) {
        return;
    }
    memcpy(resref, keyEntry, 16);

    if (g_state.observe) {
        // Observe: report what would be spliced, build nothing, leave the table untouched.
        const uint32_t rows = g_state.countTwoDaRows(resref);
        if (rows > 0) {
            g_state.counters.observe.fetch_add(1, std::memory_order_relaxed);
            KP_LOG(ch2Da, "OBSERVE %s rows=%u", resref, static_cast<unsigned>(rows));
        }
        return;
    }

    uint32_t newSize = g_state.buildTwoDaSpliced(resref, oldData, oldSize, nullptr, 0);
    if (newSize == 0) {
        return;
    }

    uint8_t* tempBuf = static_cast<uint8_t*>(HeapAlloc(GetProcessHeap(), 0, newSize));
    if (!tempBuf) {
        return;
    }

    uint32_t written = g_state.buildTwoDaSpliced(resref, oldData, oldSize, tempBuf, newSize);
    if (written == 0) {
        HeapFree(GetProcessHeap(), 0, tempBuf);
        return;
    }

    void* resMan = *reinterpret_cast<void**>(gameaddr::ResManGlobal);
    if (!resMan) {
        HeapFree(GetProcessHeap(), 0, tempBuf);
        return;
    }

    using FreeDataFn = void(__thiscall*)(void*, void*);
    using MallocFn = void(__thiscall*)(void*, void*);

    reinterpret_cast<FreeDataFn>(gameaddr::ResManFreeData)(resMan, pRes2Da);
    *reinterpret_cast<uint32_t*>(res + assetvfs::kResOffSize) = written;
    reinterpret_cast<MallocFn>(gameaddr::ResManMalloc)(resMan, pRes2Da);

    void* newData = *reinterpret_cast<void**>(res + assetvfs::kResOffData);
    if (!newData) {
        // Old data is already freed; the engine's OnResourceServiced will now see a null buffer and return 0.
        KP_ERR("SPLICE %s: ResManMalloc(%u B) returned no data; table left unspliced", resref, static_cast<unsigned>(written));
        HeapFree(GetProcessHeap(), 0, tempBuf);
        return;
    }
    memcpy(newData, tempBuf, written);
    HeapFree(GetProcessHeap(), 0, tempBuf);

    g_state.counters.splices.fetch_add(1, std::memory_order_relaxed);
    KP_LOG(ch2Da, "SPLICE %s +%d %u->%u", resref, static_cast<int>(newSize) - static_cast<int>(oldSize),
           static_cast<unsigned>(oldSize), static_cast<unsigned>(newSize));
}

__declspec(dllexport) int __cdecl VFS_OnOpenStream(uintptr_t ebp) {
    if (!g_state.enabled || !ebp) return 0;
    saferead::beginScope();

    // Once: the dword behind the InitializeSource stream gate (no writer; expected non-zero).
    static bool gateProbed = false;
    if (!gateProbed) {
        gateProbed = true;
        uint32_t gateVal = 0;
        if (saferead::readAt(S_OpenStream, assetvfs::kStreamGateFlag, 0, &gateVal)) {
            if (gateVal == 0) {
                KP_WARN("A10768=%u (stream gate flag is 0: the VFS stream branch is dead)", static_cast<unsigned>(gateVal));
            } else {
                KP_LOG(chAudio, "A10768=%u", static_cast<unsigned>(gateVal));
            }
        } else {
            KP_WARN("A10768 unreadable");
        }
    }

    uintptr_t thisPtr = 0;
    if (!saferead::readAt(S_OpenStream, ebp, -0x21C, &thisPtr) || !thisPtr) {
        return 0;
    }

    uint16_t streamKind = 0;
    if (!saferead::readAt(S_OpenStream, thisPtr, 0x148, &streamKind)) {
        return 0;
    }

    char candidate[260] = {0};
    char specBuf[260] = {0};
    bool matched = false;

    // Only the primary candidate (this+0x28) may reach the stream filters; the fallback candidates are
    // matched against the static registry only. Plain-disk specs are never claimed here.
    if (readExoString(thisPtr + 0x28, candidate, sizeof(candidate))) {
        if (g_state.resolveStreamSpec(candidate, streamKind, specBuf, sizeof(specBuf) - 1, true)) {
            matched = true;
        }
    }

    if (!matched) {
        for (int i = 0; i < 32; ++i) {
            uintptr_t candidateExo = thisPtr + 0x30 + i * 8;
            if (readExoString(candidateExo, candidate, sizeof(candidate))) {
                if (g_state.resolveStreamSpec(candidate, streamKind, specBuf, sizeof(specBuf) - 1, false)) {
                    matched = true;
                    break;
                }
            }
        }
    }
    specBuf[sizeof(specBuf) - 1] = '\0';

    if (!matched) {
        return 0;
    }

    g_state.counters.streams.fetch_add(1, std::memory_order_relaxed);
    char stem[40];
    assetvfs::streamStem(candidate, stem, sizeof(stem));
    const bool logIt = g_state.takeLog(stem);

    if (g_state.observe) {
        // Observe: spec was resolved for the log only; no Miles callback install, this+0x28 untouched.
        if (logIt) {
            KP_LOG(chAudio, "STREAM %s kind=%u spec=observe", stem, static_cast<unsigned>(streamKind));
        }
        return 0;
    }

    // Chain our Miles file callbacks in front of the current set; if that is impossible the stream
    // stays on the engine's normal path.
    if (!ensureMilesChain()) {
        if (logIt) {
            KP_WARN("STREAM %s kind=%u: Miles chain unavailable, left to the engine", stem, static_cast<unsigned>(streamKind));
        }
        return 0;
    }
    if (logIt) {
        KP_LOG(chAudio, "STREAM %s kind=%u spec=%s", stem, static_cast<unsigned>(streamKind), specBuf);
    }

    uint8_t tempExo[8] = {0};
    using ExoCtorFn = void*(__thiscall*)(void*, const char*);
    using ExoAssignFn = void*(__thiscall*)(void*, void*);
    using ExoDtorFn = void*(__thiscall*)(void*);

    reinterpret_cast<ExoCtorFn>(gameaddr::ExoCtor)(tempExo, specBuf);
    reinterpret_cast<ExoAssignFn>(gameaddr::ExoStringAssign)(reinterpret_cast<void*>(thisPtr + 0x28), tempExo);
    reinterpret_cast<ExoDtorFn>(gameaddr::ExoDtor)(tempExo);

    return 1;
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_state.lock.init(); // before any API/hook can run; never initialised lazily
        DisableThreadLibraryCalls(hinst);
    }
    return TRUE;
}
