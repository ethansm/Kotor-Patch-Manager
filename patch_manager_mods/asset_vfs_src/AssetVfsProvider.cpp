#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
    S_2Da = 0x7430
};

typedef uint32_t (__stdcall *AilFileOpenCb)(const char* filename, uint32_t* file_handle);
typedef void (__stdcall *AilFileCloseCb)(uint32_t file_handle);
typedef int32_t (__stdcall *AilFileSeekCb)(uint32_t file_handle, int32_t offset, uint32_t type);
typedef uint32_t (__stdcall *AilFileReadCb)(uint32_t file_handle, void* buffer, uint32_t bytes);
typedef void (__stdcall *AilSetFileCallbacksFn)(AilFileOpenCb, AilFileCloseCb, AilFileSeekCb, AilFileReadCb);

uint32_t __stdcall MilesFileOpen(const char* filename, uint32_t* file_handle) {
    if (!filename || !file_handle) return 0;
    if (g_state.parseVfsStreamSpec(filename, file_handle)) {
        return 1;
    }
    HANDLE h = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        *file_handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(h));
        return 1;
    }
    return 0;
}

void __stdcall MilesFileClose(uint32_t handle) {
    if (g_state.cursorClose(handle)) {
        return;
    }
    if (vfsreg::MilesCursorTable::hasVfsTag(handle)) {
        return; // stale / double close of one of our cursor handles: never hand it to CloseHandle
    }
    if (handle != 0 && handle != static_cast<uint32_t>(reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE))) {
        CloseHandle(reinterpret_cast<HANDLE>(static_cast<uintptr_t>(handle)));
    }
}

int32_t __stdcall MilesFileSeek(uint32_t handle, int32_t offset, uint32_t type) {
    if (g_state.isVfsHandle(handle)) {
        return g_state.cursorSeek(handle, offset, type);
    } else if (handle != 0 && handle != static_cast<uint32_t>(reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE))) {
        DWORD moveMethod = FILE_BEGIN;
        if (type == 1) moveMethod = FILE_CURRENT;
        else if (type == 2) moveMethod = FILE_END;
        return static_cast<int32_t>(SetFilePointer(reinterpret_cast<HANDLE>(static_cast<uintptr_t>(handle)), offset, nullptr, moveMethod));
    }
    return -1;
}

uint32_t __stdcall MilesFileRead(uint32_t handle, void* buffer, uint32_t bytes) {
    if (g_state.isVfsHandle(handle)) {
        return g_state.cursorRead(handle, buffer, bytes);
    } else if (handle != 0 && handle != static_cast<uint32_t>(reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE))) {
        DWORD bytesRead = 0;
        if (ReadFile(reinterpret_cast<HANDLE>(static_cast<uintptr_t>(handle)), buffer, bytes, &bytesRead, nullptr)) {
            return bytesRead;
        }
    }
    return 0;
}

void ensureMilesCallbacks() {
    static bool installed = false;
    if (installed) return;
    HMODULE hMss = GetModuleHandleA("Mss32.dll");
    if (!hMss) {
        hMss = LoadLibraryA("Mss32.dll");
    }
    if (hMss) {
        auto setCb = reinterpret_cast<AilSetFileCallbacksFn>(GetProcAddress(hMss, "_AIL_set_file_callbacks@16"));
        if (setCb) {
            setCb(MilesFileOpen, MilesFileClose, MilesFileSeek, MilesFileRead);
            installed = true;
            KP_LOG(chAudio, "installed Miles file callbacks");
        }
    }
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
    }
}

const char* ownerOrUnknown(const char* owner) {
    return (owner && owner[0]) ? owner : "?";
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
    (void)owner;
    return g_state.addTwoDaCells(cells, count);
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
    return g_state.chars.registerDef(def, owner);
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
    if (!entry) return 0;
    return g_nw.spawn(entry->default_utc, entry->default_tag, x, y, z, facingRadians);
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

    int enabledIni = GetPrivateProfileIntA("vfs", "enabled", 1, ".\\asset_vfs.ini");
    if (enabledIni == 0) {
        g_state.enabled = false;
    }

    g_nw.init(api, "asset-vfs");
    g_nw.connect();

    KP_LOG(chCore, "provided %s v%d and %s v%d (enabled=%d)",
           K2_ASSETVFS_IFACE, K2_ASSETVFS_VERSION,
           K2_CHARLIB_IFACE, K2_CHARLIB_VERSION,
           g_state.enabled ? 1 : 0);
}

__declspec(dllexport) int __cdecl VFS_OnGetKeyEntryMiss(uintptr_t ebp) {
    if (!ebp) return 0;
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
    if (!g_state.enabled || !ebp) return 0;
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

    KP_LOG(ch2Da, "SPLICE %s +%dB", resref, static_cast<int>(newSize) - static_cast<int>(oldSize));
}

__declspec(dllexport) int __cdecl VFS_OnOpenStream(uintptr_t ebp) {
    if (!g_state.enabled || !ebp) return 0;
    saferead::beginScope();

    uintptr_t thisPtr = 0;
    if (!saferead::readAt(S_OpenStream, ebp, -0x21C, &thisPtr) || !thisPtr) {
        return 0;
    }

    uint16_t streamKind = 0;
    if (!saferead::readAt(S_OpenStream, thisPtr, 0x148, &streamKind)) {
        return 0;
    }

    char candidate[260] = {0};
    char specBuf[64] = {0};
    bool matched = false;

    if (readExoString(thisPtr + 0x28, candidate, sizeof(candidate))) {
        if (g_state.resolveStreamSpec(candidate, streamKind, specBuf, sizeof(specBuf))) {
            matched = true;
        }
    }

    if (!matched) {
        for (int i = 0; i < 32; ++i) {
            uintptr_t candidateExo = thisPtr + 0x30 + i * 8;
            if (readExoString(candidateExo, candidate, sizeof(candidate))) {
                if (g_state.resolveStreamSpec(candidate, streamKind, specBuf, sizeof(specBuf))) {
                    matched = true;
                    break;
                }
            }
        }
    }

    if (!matched) {
        return 0;
    }

    if (std::strncmp(specBuf, "#*VFS_", 6) == 0) {
        ensureMilesCallbacks();
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
