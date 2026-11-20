/* CharacterProvider.h -- ABI for modular character providers and asset VFS.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints, floats, and function pointers only,
 * KPATCH_CALL (__cdecl on x86), structs packed to 8, structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 *
 * Interfaces defined here:
 *   1. "k2.assetvfs" v1 (K2_ASSETVFS_IFACE, K2_ASSETVFS_VERSION 1)
 *      Allows runtime in-memory registration of game assets (CExoResMan / Miles stream)
 *      and dynamic 2DA row/cell patches.
 *   2. "k2.characterlib" v1 (K2_CHARLIB_IFACE, K2_CHARLIB_VERSION 1)
 *      Allows registering ported character definitions and spawning them into the
 *      active area via npc-world.
 *
 * Under C++ (#ifdef __cplusplus), namespace k2char provides:
 *   - Client: safe consumer wrapper with lazy poll() connection and inert defaults.
 *   - Catalog: pure in-memory character catalog for up to kMaxCharacters (64) entries.
 */
#ifndef K2_CHARACTER_PROVIDER_H
#define K2_CHARACTER_PROVIDER_H

#if __has_include("kpatch_api.h")
#include "kpatch_api.h"
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef KPATCH_CALL
#if defined(_MSC_VER) || defined(__MINGW32__)
#define KPATCH_CALL __cdecl
#elif defined(__i386__)
#define KPATCH_CALL __attribute__((cdecl))
#else
#define KPATCH_CALL
#endif
#endif

#ifndef KPATCH_API_H
#define KPATCH_API_H
#pragma pack(push, 8)
typedef struct KPatchApi {
    uint32_t struct_size;
    uint32_t abi_version;
    int32_t (KPATCH_CALL *provide)(const char* name, uint32_t version, const void* iface);
    const void* (KPATCH_CALL *require)(const char* name, uint32_t version);
} KPatchApi;
#pragma pack(pop)
#endif

/* Interface 1: k2.assetvfs v1 */
#define K2_ASSETVFS_IFACE   "k2.assetvfs"
#define K2_ASSETVFS_VERSION 1

#define K2VFS_FLAG_RESMAN 0
#define K2VFS_FLAG_STREAM 1

/* Stream filter return codes */
#define K2VFS_STREAM_PASS    0 /* Not handled; pass to next filter or disk */
#define K2VFS_STREAM_MEMORY  1 /* Filter populated *out_data and *out_size (> 0); stream from memory */
#define K2VFS_STREAM_REWRITE 2 /* DISABLED: treated as K2VFS_STREAM_PASS (one WARN per owner) */

/* Interface 2: k2.characterlib v1 */
#define K2_CHARLIB_IFACE   "k2.characterlib"
#define K2_CHARLIB_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2VfsAssetEntry {
    const char* resref;
    uint16_t res_type;
    uint16_t flags;
    const void* data;
    uint32_t size;
} K2VfsAssetEntry;

typedef struct K2Vfs2DaCell {
    const char* table_name;
    uint32_t row_index;
    int32_t template_row;
    const char* column_name;
    const char* value;
} K2Vfs2DaCell;

/* Called on CExoResMan::GetKeyEntry (0x0071244E) miss when the static VFS registry does not have
 * (resref, res_type). resref is lowercase null-terminated (up to 16 chars).
 * If the callback fills *out_entry (data != NULL, size > 0) and returns 1, asset-vfs registers
 * that asset into the VFS registry and immediately services it via ServiceFromImage (0x00713E83).
 * Return 0 to pass to the next handler / vanilla miss.
 *
 * Contract:
 *   - The data is COPIED by asset-vfs before the callback's claim is registered: *out_entry->data
 *     need only stay valid until the callback returns. The copy is owned by the handler's token and
 *     freed (unless already served to the engine) by UnregisterResManMissHandler.
 *   - Called on the engine's main thread, WITHOUT any asset-vfs lock held, so the callback may call
 *     back into the k2.assetvfs API. A re-entrant GetKeyEntry miss raised from inside a callback is
 *     answered as a plain miss without calling any handler.
 *   - Handlers are asked in registration slot order; the first one to claim (return 1 with data) wins.
 *   - A (resref, res_type) every handler declined is remembered in a bounded negative cache and not
 *     asked again until ANY RegisterAsset / RegisterAssets / RegisterResManMissHandler call. A handler
 *     whose answer can change must therefore re-register (or register an asset) when it does. */
typedef int32_t (KPATCH_CALL *K2VfsResManMissFn)(
    const char* resref,
    uint16_t res_type,
    K2VfsAssetEntry* out_entry,
    void* user);

/* Called on StreamingSource::OpenStream (0x0070CB76) when resolving the stream's primary candidate path
 * (the string at this+0x28, e.g. "HD0:STREAMVOICE\\kvjv0001.wav") with stream_kind (+0x148).
 * Filters are called ONLY for stream_kind 4 and 8 and ONLY for that primary candidate (the fallback
 * candidates at this+0x30.. are resolved against the static registry only), on the engine's main
 * thread, with no asset-vfs lock held.
 * Return K2VFS_STREAM_MEMORY (1) and set *out_data / *out_size (> 0) to stream an in-memory
 * audio buffer (58-byte VO, 470-byte SFX, or plain RIFF/MP3) via asset-vfs's Miles cursor table.
 *   - The buffer is COPIED before the callback's claim is registered; it need only stay valid until
 *     the callback returns. The copy is owned by the filter's token and freed (unless already served)
 *     by UnregisterStreamFilter.
 *   - The record is keyed by the full normalised candidate path (lowercase, '/' -> '\\'). A MEMORY claim
 *     on a file stem already owned by a DIFFERENT candidate path is rejected (one WARN) and treated as PASS.
 *   - Filters are asked in registration slot order; the first to claim wins.
 * K2VFS_STREAM_REWRITE (2) is DISABLED: it is treated as K2VFS_STREAM_PASS (one WARN per owner), because
 * a rewritten path could not be told apart from a disk spec by the Miles file callbacks.
 * Return K2VFS_STREAM_PASS (0) to pass through. */
typedef int32_t (KPATCH_CALL *K2VfsStreamFilterFn)(
    const char* candidate_path,
    uint16_t stream_kind,
    const void** out_data,
    uint32_t* out_size,
    char* out_resolved_path,
    uint32_t path_cap,
    void* user);

typedef struct K2AssetVfsApi {
    uint32_t struct_size;
    int32_t (KPATCH_CALL *RegisterAsset)(const K2VfsAssetEntry* entry, const char* owner);
    int32_t (KPATCH_CALL *RegisterAssets)(const K2VfsAssetEntry* entries, uint32_t count, const char* owner);
    int32_t (KPATCH_CALL *Register2DaCells)(const K2Vfs2DaCell* cells, uint32_t count, const char* owner);
    int32_t (KPATCH_CALL *HasAsset)(const char* resref, uint16_t res_type, uint16_t flags);
    int32_t (KPATCH_CALL *RegisterResManMissHandler)(K2VfsResManMissFn fn, void* user, const char* owner);
    void    (KPATCH_CALL *UnregisterResManMissHandler)(int32_t token);
    int32_t (KPATCH_CALL *RegisterStreamFilter)(K2VfsStreamFilterFn fn, void* user, const char* owner);
    void    (KPATCH_CALL *UnregisterStreamFilter)(int32_t token);
} K2AssetVfsApi;

typedef struct K2CharacterDefinition {
    uint32_t struct_size;
    const char* character_id;   /* e.g. "juhani", "carth" */
    const char* display_name;   /* e.g. "Juhani" */
    const char* default_tag;    /* e.g. "k1p_juhani" */
    const char* default_utc;    /* e.g. "k1p_juhani" */
    const char* default_dlg;    /* e.g. "kvjdlg01" */
    uint32_t appearance_id;     /* Virtual 2DA appearance row (e.g. 818) */
    uint32_t head_id;           /* Virtual 2DA heads row (e.g. 249; 250 is Carth's) */
    uint32_t soundset_id;       /* Virtual soundset row (e.g. 70) */
} K2CharacterDefinition;

typedef struct K2CharacterLibApi {
    uint32_t struct_size;
    int32_t (KPATCH_CALL *RegisterCharacter)(const K2CharacterDefinition* def, const char* owner);
    int32_t (KPATCH_CALL *HasCharacter)(const char* character_id);
    int32_t (KPATCH_CALL *GetCharacter)(const char* character_id, K2CharacterDefinition* out_def);
    int32_t (KPATCH_CALL *IsCharacterAppearance)(uint32_t appearance_id); /* 1 if appearance_id belongs to a registered character, 0 otherwise */
    uint32_t (KPATCH_CALL *Spawn)(const char* character_id, const char* area_resref, float x, float y, float z, float facingRadians);
} K2CharacterLibApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Layout assertions */
#if defined(__cplusplus)
#define K2CHAR_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define K2CHAR_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef K2CHAR_STATIC_ASSERT
K2CHAR_STATIC_ASSERT(offsetof(K2AssetVfsApi, RegisterAsset) == sizeof(void*), "K2AssetVfsApi.RegisterAsset offset");
K2CHAR_STATIC_ASSERT(offsetof(K2AssetVfsApi, HasAsset) == 4 * sizeof(void*), "K2AssetVfsApi.HasAsset offset");
K2CHAR_STATIC_ASSERT(offsetof(K2AssetVfsApi, UnregisterStreamFilter) == 8 * sizeof(void*), "K2AssetVfsApi.UnregisterStreamFilter offset");
K2CHAR_STATIC_ASSERT(sizeof(K2AssetVfsApi) == 9 * sizeof(void*), "K2AssetVfsApi size");

K2CHAR_STATIC_ASSERT(offsetof(K2CharacterLibApi, RegisterCharacter) == sizeof(void*), "K2CharacterLibApi.RegisterCharacter offset");
K2CHAR_STATIC_ASSERT(offsetof(K2CharacterLibApi, Spawn) == 5 * sizeof(void*), "K2CharacterLibApi.Spawn offset");
K2CHAR_STATIC_ASSERT(sizeof(K2CharacterLibApi) == 6 * sizeof(void*), "K2CharacterLibApi size");
#undef K2CHAR_STATIC_ASSERT
#endif

#ifdef __cplusplus

#include <cstdio>
#include <cstring>

namespace k2char {

inline bool strEqualNoCase(const char* a, const char* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    while (*a && *b) {
        char ca = *a++;
        char cb = *b++;
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb + ('a' - 'A'));
        if (ca != cb) return false;
    }
    return *a == *b;
}

constexpr uint32_t kMaxCharacters = 64;

/* In-memory catalog of registered character definitions */
struct Catalog {
    struct Entry {
        char character_id[32];
        char display_name[64];
        char default_tag[32];
        char default_utc[32];
        char default_dlg[32];
        uint32_t appearance_id;
        uint32_t head_id;
        uint32_t soundset_id;
        char owner[32];
    };

    Entry entries[kMaxCharacters] = {};
    uint32_t count = 0;

    void clear() {
        count = 0;
        std::memset(entries, 0, sizeof(entries));
    }

    uint32_t size() const { return count; }

    int32_t registerDef(const K2CharacterDefinition* def, const char* owner = nullptr, bool allowUpdate = true) {
        if (!def) return 0;
        if (def->struct_size < offsetof(K2CharacterDefinition, soundset_id) + sizeof(def->soundset_id)) {
            return 0;
        }
        if (!def->character_id || !def->character_id[0]) {
            return 0;
        }

        // Duplicate check (case-insensitive)
        for (uint32_t i = 0; i < count; ++i) {
            if (strEqualNoCase(entries[i].character_id, def->character_id)) {
                if (!allowUpdate) return 0;
                std::snprintf(entries[i].character_id, sizeof(entries[i].character_id), "%s", def->character_id);
                std::snprintf(entries[i].display_name, sizeof(entries[i].display_name), "%s", def->display_name ? def->display_name : "");
                std::snprintf(entries[i].default_tag, sizeof(entries[i].default_tag), "%s", def->default_tag ? def->default_tag : "");
                std::snprintf(entries[i].default_utc, sizeof(entries[i].default_utc), "%s", def->default_utc ? def->default_utc : "");
                std::snprintf(entries[i].default_dlg, sizeof(entries[i].default_dlg), "%s", def->default_dlg ? def->default_dlg : "");
                entries[i].appearance_id = def->appearance_id;
                entries[i].head_id = def->head_id;
                entries[i].soundset_id = def->soundset_id;
                if (owner) {
                    std::snprintf(entries[i].owner, sizeof(entries[i].owner), "%s", owner);
                }
                return 1;
            }
        }

        if (count >= kMaxCharacters) return 0;

        Entry& e = entries[count++];
        std::snprintf(e.character_id, sizeof(e.character_id), "%s", def->character_id);
        std::snprintf(e.display_name, sizeof(e.display_name), "%s", def->display_name ? def->display_name : "");
        std::snprintf(e.default_tag, sizeof(e.default_tag), "%s", def->default_tag ? def->default_tag : "");
        std::snprintf(e.default_utc, sizeof(e.default_utc), "%s", def->default_utc ? def->default_utc : "");
        std::snprintf(e.default_dlg, sizeof(e.default_dlg), "%s", def->default_dlg ? def->default_dlg : "");
        e.appearance_id = def->appearance_id;
        e.head_id = def->head_id;
        e.soundset_id = def->soundset_id;
        if (owner) {
            std::snprintf(e.owner, sizeof(e.owner), "%s", owner);
        } else {
            e.owner[0] = '\0';
        }
        return 1;
    }

    int32_t has(const char* character_id) const {
        if (!character_id || !*character_id) return 0;
        for (uint32_t i = 0; i < count; ++i) {
            if (strEqualNoCase(entries[i].character_id, character_id)) return 1;
        }
        return 0;
    }

    int32_t get(const char* character_id, K2CharacterDefinition* out_def) const {
        if (!character_id || !*character_id || !out_def) return 0;
        for (uint32_t i = 0; i < count; ++i) {
            if (strEqualNoCase(entries[i].character_id, character_id)) {
                K2CharacterDefinition d = {};
                d.struct_size = sizeof(K2CharacterDefinition);
                d.character_id = entries[i].character_id;
                d.display_name = entries[i].display_name;
                d.default_tag = entries[i].default_tag;
                d.default_utc = entries[i].default_utc;
                d.default_dlg = entries[i].default_dlg;
                d.appearance_id = entries[i].appearance_id;
                d.head_id = entries[i].head_id;
                d.soundset_id = entries[i].soundset_id;

                uint32_t copySize = sizeof(K2CharacterDefinition);
                if (out_def->struct_size > 0 && out_def->struct_size < sizeof(K2CharacterDefinition)) {
                    copySize = out_def->struct_size;
                }
                std::memcpy(out_def, &d, copySize);
                out_def->struct_size = copySize;
                return 1;
            }
        }
        return 0;
    }

    int32_t isAppearance(uint32_t appearance_id) const {
        if (appearance_id == 0) return 0;
        for (uint32_t i = 0; i < count; ++i) {
            if (entries[i].appearance_id == appearance_id) return 1;
        }
        return 0;
    }

    const Entry* getEntry(const char* character_id) const {
        if (!character_id || !*character_id) return nullptr;
        for (uint32_t i = 0; i < count; ++i) {
            if (strEqualNoCase(entries[i].character_id, character_id)) return &entries[i];
        }
        return nullptr;
    }

    const Entry* findByAppearance(uint32_t appearance_id) const {
        if (appearance_id == 0) return nullptr;
        for (uint32_t i = 0; i < count; ++i) {
            if (entries[i].appearance_id == appearance_id) return &entries[i];
        }
        return nullptr;
    }
};

/* C++ Consumer Client Helper (modeled on k2nw::Client in NpcWorld.h) */
struct Client {
    const KPatchApi* api = nullptr;
    const K2CharacterLibApi* lib = nullptr;
    const K2AssetVfsApi* vfs = nullptr;
    const char* owner = "";
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;
    bool reportedMissing = false;

    void init(const KPatchApi* a, const char* id) {
        api = a;
        owner = id ? id : "";
    }

    static bool usable(const K2CharacterLibApi* p) {
        return p && p->struct_size >= offsetof(K2CharacterLibApi, Spawn) + sizeof(p->Spawn)
                 && p->RegisterCharacter && p->HasCharacter && p->GetCharacter
                 && p->IsCharacterAppearance && p->Spawn;
    }

    static bool usableVfs(const K2AssetVfsApi* v) {
        return v && v->struct_size >= offsetof(K2AssetVfsApi, HasAsset) + sizeof(v->HasAsset)
                 && v->RegisterAsset && v->RegisterAssets && v->Register2DaCells && v->HasAsset;
    }

    static bool usableVfsHooks(const K2AssetVfsApi* v) {
        return usableVfs(v) && v->struct_size >= offsetof(K2AssetVfsApi, UnregisterStreamFilter) + sizeof(v->UnregisterStreamFilter)
                 && v->RegisterResManMissHandler && v->UnregisterResManMissHandler
                 && v->RegisterStreamFilter && v->UnregisterStreamFilter;
    }

    bool connect() {
        if (connected && vfs) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi");
            reportedMissing = true;
            return false;
        }
        if (!lib) {
            const K2CharacterLibApi* p = static_cast<const K2CharacterLibApi*>(api->require(K2_CHARLIB_IFACE, K2_CHARLIB_VERSION));
            if (usable(p)) {
                lib = p;
            } else if (p) {
                if (!reportedMissing && onConnect) onConnect(false, "k2.characterlib v1 table too small or incomplete");
            }
        }
        if (!vfs) {
            const K2AssetVfsApi* v = static_cast<const K2AssetVfsApi*>(api->require(K2_ASSETVFS_IFACE, K2_ASSETVFS_VERSION));
            if (usableVfs(v)) {
                vfs = v;
            }
        }
        connected = (lib != nullptr);
        if (!connected) {
            if (!reportedMissing && onConnect) onConnect(false, "k2.characterlib v1 not provided");
            reportedMissing = true;
            return false;
        }
        if (onConnect) onConnect(true, vfs ? "connected with vfs" : "connected");
        return true;
    }

    void poll() {
        if (!connected && (pollCount++ & 255) == 0) {
            connect();
        }
    }

    void release() {
        lib = nullptr;
        vfs = nullptr;
        connected = false;
    }

    // ---- Service wrappers: safe no-ops without a provider ----
    int32_t registerCharacter(const K2CharacterDefinition* def) const {
        return lib ? lib->RegisterCharacter(def, owner) : 0;
    }

    int32_t hasCharacter(const char* character_id) const {
        return lib ? lib->HasCharacter(character_id) : 0;
    }

    int32_t getCharacter(const char* character_id, K2CharacterDefinition* out_def) const {
        return lib ? lib->GetCharacter(character_id, out_def) : 0;
    }

    int32_t isCharacterAppearance(uint32_t appId) const {
        return lib ? lib->IsCharacterAppearance(appId) : 0;
    }

    uint32_t spawn(const char* character_id, const char* area_resref, float x, float y, float z, float facingRadians) const {
        return lib ? lib->Spawn(character_id, area_resref, x, y, z, facingRadians) : 0u;
    }

    int32_t registerAsset(const K2VfsAssetEntry* entry) const {
        return vfs ? vfs->RegisterAsset(entry, owner) : 0;
    }

    int32_t registerAssets(const K2VfsAssetEntry* entries, uint32_t count) const {
        return vfs ? vfs->RegisterAssets(entries, count, owner) : 0;
    }

    int32_t register2DaCells(const K2Vfs2DaCell* cells, uint32_t count) const {
        return vfs ? vfs->Register2DaCells(cells, count, owner) : 0;
    }

    int32_t hasAsset(const char* resref, uint16_t res_type, uint16_t flags) const {
        return vfs ? vfs->HasAsset(resref, res_type, flags) : 0;
    }

    int32_t registerResManMissHandler(K2VfsResManMissFn fn, void* user = nullptr) const {
        return (vfs && usableVfsHooks(vfs)) ? vfs->RegisterResManMissHandler(fn, user, owner) : 0;
    }

    void unregisterResManMissHandler(int32_t token) const {
        if (vfs && usableVfsHooks(vfs) && token > 0) {
            vfs->UnregisterResManMissHandler(token);
        }
    }

    int32_t registerStreamFilter(K2VfsStreamFilterFn fn, void* user = nullptr) const {
        return (vfs && usableVfsHooks(vfs)) ? vfs->RegisterStreamFilter(fn, user, owner) : 0;
    }

    void unregisterStreamFilter(int32_t token) const {
        if (vfs && usableVfsHooks(vfs) && token > 0) {
            vfs->UnregisterStreamFilter(token);
        }
    }
};

} // namespace k2char

#endif /* __cplusplus */

#endif /* K2_CHARACTER_PROVIDER_H */
